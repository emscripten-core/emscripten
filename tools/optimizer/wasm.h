//
// WebAssembly representation and processing library
//

#ifndef __wasm_h__
#define __wasm_h__

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

#include "simple_ast.h"
#include "colors.h"

namespace wasm {

// Utilities

// Arena allocation for mixed-type data.
struct Arena {
  std::vector<char*> chunks;
  int index; // in last chunk

  template<class T>
  T* alloc() {
    const size_t CHUNK = 10000;
    size_t currSize = (sizeof(T) + 7) & (-8); // same alignment as malloc TODO optimize?
    assert(currSize < CHUNK);
    if (chunks.size() == 0 || index + currSize >= CHUNK) {
      chunks.push_back(new char[CHUNK]);
      index = 0;
    }
    T* ret = (T*)(chunks.back() + index);
    index += currSize;
    new (ret) T();
    return ret;
  }

  void clear() {
    for (char* chunk : chunks) {
      delete[] chunk;
    }
    chunks.clear();
  }

  ~Arena() {
    clear();
  }
};

std::ostream &doIndent(std::ostream &o, unsigned indent) {
  for (unsigned i = 0; i < indent; i++) {
    o << "  ";
  }
  return o;
}
std::ostream &incIndent(std::ostream &o, unsigned& indent) {
  o << '\n';
  indent++;
  return o; 
}
std::ostream &decIndent(std::ostream &o, unsigned& indent) {
  indent--;
  doIndent(o, indent);
  return o << ')';
}

// Basics

struct Name : public cashew::IString {
  Name() : cashew::IString() {}
  Name(const char *str) : cashew::IString(str) {}
  Name(cashew::IString str) : cashew::IString(str) {}

  friend std::ostream& operator<<(std::ostream &o, Name name) {
    assert(name.str);
    return o << '$' << name.str; // reference interpreter requires we prefix all names
  }
};

// Types

enum WasmType {
  none,
  i32,
  i64,
  f32,
  f64
};

const char* printWasmType(WasmType type) {
  switch (type) {
    case WasmType::none: return "none";
    case WasmType::i32: return "i32";
    case WasmType::i64: return "i64";
    case WasmType::f32: return "f32";
    case WasmType::f64: return "f64";
  }
}

unsigned getWasmTypeSize(WasmType type) {
  switch (type) {
    case WasmType::none: abort();
    case WasmType::i32: return 4;
    case WasmType::i64: return 8;
    case WasmType::f32: return 4;
    case WasmType::f64: return 8;
  }
}

bool isFloat(WasmType type) {
  switch (type) {
    case f32:
    case f64: return true;
    default: return false;
  }
}

WasmType getWasmType(unsigned size, bool float_) {
  if (size < 4) return WasmType::i32;
  if (size == 4) return float_ ? WasmType::f32 : WasmType::i32;
  if (size == 8) return float_ ? WasmType::f64 : WasmType::i64;
  abort();
}

std::ostream &prepareMajorColor(std::ostream &o) {
  Colors::red(o);
  Colors::bold(o);
  return o;
}

std::ostream &prepareColor(std::ostream &o) {
  Colors::magenta(o);
  Colors::bold(o);
  return o;
}

std::ostream &prepareMinorColor(std::ostream &o) {
  Colors::orange(o);
  return o;
}

std::ostream &restoreNormalColor(std::ostream &o) {
  Colors::normal(o);
  return o;
}

std::ostream& printText(std::ostream &o, const char *str) {
  o << '"';
  Colors::green(o);
  o << str;
  Colors::normal(o);
  return o << '"';
}

struct Literal {
  WasmType type;
  union {
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;
  };

  Literal() : type(WasmType::none), f64(0) {}
  Literal(int32_t init) : type(WasmType::i32), i32(init) {}
  Literal(int64_t init) : type(WasmType::i64), i64(init) {}
  Literal(float   init) : type(WasmType::f32), f32(init) {}
  Literal(double  init) : type(WasmType::f64), f64(init) {}

  int32_t geti32() { assert(type == WasmType::i32); return i32; }
  int64_t geti64() { assert(type == WasmType::i64); return i64; }
  float   getf32() { assert(type == WasmType::f32); return f32; }
  double  getf64() { assert(type == WasmType::f64); return f64; }

  void printDouble(std::ostream &o, double d) {
    const char *text = JSPrinter::numToString(d);
    // spec interpreter hates floats starting with '.'
    if (text[0] == '.') {
      o << '0';
    } else if (text[0] == '-' && text[1] == '.') {
      o << "-0";
      text++;
    }
    o << text;
  }

  friend std::ostream& operator<<(std::ostream &o, Literal literal) {
    o << '(';
    prepareMinorColor(o) << printWasmType(literal.type) << ".const ";
    switch (literal.type) {
      case none: abort();
      case WasmType::i32: o << literal.i32; break;
      case WasmType::i64: o << literal.i64; break;
      case WasmType::f32: literal.printDouble(o, literal.f32); break;
      case WasmType::f64: literal.printDouble(o, literal.f64); break;
    }
    restoreNormalColor(o);
    return o << ')';
  }
};

// Operators

enum UnaryOp {
  Clz, Ctz, Popcnt, // int
  Neg, Abs, Ceil, Floor, Trunc, Nearest, Sqrt // float
};

enum BinaryOp {
  Add, Sub, Mul, // int or float
  DivS, DivU, RemS, RemU, And, Or, Xor, Shl, ShrU, ShrS, // int
  Div, CopySign, Min, Max // float
};

enum RelationalOp {
  Eq, Ne, // int or float
  LtS, LtU, LeS, LeU, GtS, GtU, GeS, GeU, // int
  Lt, Le, Gt, Ge // float
};

enum ConvertOp {
  ExtendSInt32, ExtendUInt32, WrapInt64, TruncSFloat32, TruncUFloat32, TruncSFloat64, TruncUFloat64, ReinterpretFloat, // int
  ConvertSInt32, ConvertUInt32, ConvertSInt64, ConvertUInt64, PromoteFloat32, DemoteFloat64, ReinterpretInt // float
};

enum HostOp {
  PageSize, MemorySize, GrowMemory, HasFeature
};

// Expressions

class Expression {
public:
  WasmType type; // the type of the expression: its output, not necessarily its input(s)

  virtual std::ostream& print(std::ostream &o, unsigned indent) = 0;

  template<class T>
  bool is() {
    return !!dynamic_cast<T*>(this);
  }
};

std::ostream& printFullLine(std::ostream &o, unsigned indent, Expression *expression) {
  doIndent(o, indent);
  expression->print(o, indent);
  return o << '\n';
}

std::ostream& printOpening(std::ostream &o, const char *str, bool major=false) {
  o << '(';
  major ? prepareMajorColor(o) : prepareColor(o);
  o << str;
  restoreNormalColor(o);
  return o;
}

std::ostream& printMinorOpening(std::ostream &o, const char *str) {
  o << '(';
  prepareMinorColor(o);
  o << str;
  restoreNormalColor(o);
  return o;
}

typedef std::vector<Expression*> ExpressionList; // TODO: optimize  

class Nop : public Expression {
  std::ostream& print(std::ostream &o, unsigned indent) override {
    return printMinorOpening(o, "nop") << ')';
  }
};

class Block : public Expression {
public:
  Name name;
  ExpressionList list;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    printOpening(o, "block");
    if (name.is()) {
      o << ' ' << name;
    }
    incIndent(o, indent);
    for (auto expression : list) {
      printFullLine(o, indent, expression);
    }
    return decIndent(o, indent);
  }
};

class If : public Expression {
public:
  Expression *condition, *ifTrue, *ifFalse;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    printOpening(o, "if");
    incIndent(o, indent);
    printFullLine(o, indent, condition);
    printFullLine(o, indent, ifTrue);
    if (ifFalse) printFullLine(o, indent, ifFalse);
    return decIndent(o, indent);
  }
};

class Loop : public Expression {
public:
  Name out, in;
  Expression *body;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    printOpening(o, "loop");
    if (out.is()) {
      o << ' ' << out;
      if (in.is()) {
        o << ' ' << in;
        }
    }
    incIndent(o, indent);
    printFullLine(o, indent, body);
    return decIndent(o, indent);
  }
};

class Label : public Expression {
public:
  Name name;
  Expression* body;
};

class Break : public Expression {
public:
  Name name;
  Expression *condition, *value;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    printOpening(o, "break ") << name;
    incIndent(o, indent);
    if (condition) printFullLine(o, indent, condition);
    if (value) printFullLine(o, indent, value);
    return decIndent(o, indent);
  }
};

class Switch : public Expression {
public:
  struct Case {
    Literal value;
    Expression *body;
    bool fallthru;
  };

  Name name;
  Expression *value;
  std::vector<Case> cases;
  Expression *default_;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    printOpening(o, "switch ") << name;
    incIndent(o, indent);
    printFullLine(o, indent, value);
    o << "TODO: cases/default\n";
    return decIndent(o, indent);
  }

};

class Call : public Expression {
public:
  Name target;
  ExpressionList operands;

  std::ostream& printBody(std::ostream &o, unsigned indent) {
    o << target;
    if (operands.size() > 0) {
      incIndent(o, indent);
      for (auto operand : operands) {
        printFullLine(o, indent, operand);
      }
      decIndent(o, indent);
    } else {
      o << ')';
    }
    return o;
  }

  std::ostream& print(std::ostream &o, unsigned indent) override {
    printOpening(o, "call ");
    return printBody(o, indent);
  }
};

class CallImport : public Call {
  std::ostream& print(std::ostream &o, unsigned indent) override {
    printOpening(o, "call_import ");
    return printBody(o, indent);
  }
};

class FunctionType {
public:
  Name name;
  WasmType result;
  std::vector<WasmType> params;

  std::ostream& print(std::ostream &o, unsigned indent, bool full=false) {
    if (full) {
      printOpening(o, "type") << ' ' << name << " (func";
    }
    if (params.size() > 0) {
      o << ' ';
      printMinorOpening(o, "param");
      for (auto& param : params) {
        o << ' ' << printWasmType(param);
      }
      o << ')';
    }
    if (result != none) {
      o << ' ';
      printMinorOpening(o, "result ") << printWasmType(result) << ')';
    }
    if (full) {
      o << "))";;
    }
    return o;
  }

  bool operator==(FunctionType& b) {
    if (name != b.name) return false; // XXX
    if (result != b.result) return false;
    if (params.size() != b.params.size()) return false;
    for (size_t i = 0; i < params.size(); i++) {
      if (params[i] != b.params[i]) return false;
    }
    return true;
  }
  bool operator!=(FunctionType& b) {
    return !(*this == b);
  }
};

class CallIndirect : public Expression {
public:
  FunctionType *type;
  Expression *target;
  ExpressionList operands;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    printOpening(o, "call_indirect ") << type->name;
    incIndent(o, indent);
    printFullLine(o, indent, target);
    for (auto operand : operands) {
      printFullLine(o, indent, operand);
    }
    return decIndent(o, indent);
  }
};

class GetLocal : public Expression {
public:
  Name name;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    return printOpening(o, "get_local ") << name << ')';
  }
};

class SetLocal : public Expression {
public:
  Name name;
  Expression *value;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    printOpening(o, "set_local ") << name;
    incIndent(o, indent);
    printFullLine(o, indent, value);
    return decIndent(o, indent);
  }
};

class Load : public Expression {
public:
  unsigned bytes;
  bool signed_;
  bool float_;
  int offset;
  unsigned align;
  Expression *ptr;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    o << '(';
    prepareColor(o) << printWasmType(getWasmType(bytes, float_)) << ".load";
    if (bytes < 4) {
      if (bytes == 1) {
        o << '8';
      } else if (bytes == 2) {
        o << "16";
      } else {
        abort();
      }
      o << (signed_ ? "_s" : "_u");
    }
    restoreNormalColor(o);
    o << " align=" << align;
    assert(!offset);
    incIndent(o, indent);
    printFullLine(o, indent, ptr);
    return decIndent(o, indent);
  }
};

class Store : public Expression {
public:
  unsigned bytes;
  bool float_;
  int offset;
  unsigned align;
  Expression *ptr, *value;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    o << '(';
    prepareColor(o) << printWasmType(getWasmType(bytes, float_)) << ".store";
    if (bytes < 4) {
      if (bytes == 1) {
        o << '8';
      } else if (bytes == 2) {
        o << "16";
      } else {
        abort();
      }
    }
    restoreNormalColor(o);
    o << " align=" << align;
    assert(!offset);
    incIndent(o, indent);
    printFullLine(o, indent, ptr);
    printFullLine(o, indent, value);
    return decIndent(o, indent);
  }
};

class Const : public Expression {
public:
  Literal value;

  Const* set(Literal value_) {
    value = value_;
    type = value.type;
    return this;
  }

  std::ostream& print(std::ostream &o, unsigned indent) override {
    return o << value;
  }
};

class Unary : public Expression {
public:
  UnaryOp op;
  Expression *value;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    o << '(';
    prepareColor(o) << printWasmType(type) << '.';
    switch (op) {
      case Clz: o << "clz"; break;
      case Neg: o << "neg"; break;
      case Floor: o << "floor"; break;
      default: abort();
    }
    incIndent(o, indent);
    printFullLine(o, indent, value);
    return decIndent(o, indent);
  }
};

class Binary : public Expression {
public:
  BinaryOp op;
  Expression *left, *right;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    o << '(';
    prepareColor(o) << printWasmType(type) << '.';
    switch (op) {
      case Add:      o << "add"; break;
      case Sub:      o << "sub"; break;
      case Mul:      o << "mul"; break;
      case DivS:     o << "div_s"; break;
      case DivU:     o << "div_u"; break;
      case RemS:     o << "rem_s"; break;
      case RemU:     o << "rem_u"; break;
      case And:      o << "and"; break;
      case Or:       o << "or"; break;
      case Xor:      o << "xor"; break;
      case Shl:      o << "shl"; break;
      case ShrU:     o << "shr_u"; break;
      case ShrS:     o << "shr_s"; break;
      case Div:      o << "div"; break;
      case CopySign: o << "copysign"; break;
      case Min:      o << "min"; break;
      case Max:      o << "max"; break;
      default: abort();
    }
    restoreNormalColor(o);
    incIndent(o, indent);
    printFullLine(o, indent, left);
    printFullLine(o, indent, right);
    return decIndent(o, indent);
  }
};

class Compare : public Expression {
public:
  RelationalOp op;
  WasmType inputType;
  Expression *left, *right;

  Compare() {
    type = WasmType::i32; // output is always i32
  }

  std::ostream& print(std::ostream &o, unsigned indent) override {
    o << '(';
    prepareColor(o) << printWasmType(inputType) << '.';
    switch (op) {
      case Eq:  o << "eq"; break;
      case Ne:  o << "ne"; break;
      case LtS: o << "lt_s"; break;
      case LtU: o << "lt_u"; break;
      case LeS: o << "le_s"; break;
      case LeU: o << "le_u"; break;
      case GtS: o << "gt_s"; break;
      case GtU: o << "gt_u"; break;
      case GeS: o << "ge_s"; break;
      case GeU: o << "ge_u"; break;
      case Lt:  o << "lt"; break;
      case Le:  o << "le"; break;
      case Gt:  o << "gt"; break;
      case Ge:  o << "ge"; break;
      default: abort();
    }
    restoreNormalColor(o);
    incIndent(o, indent);
    printFullLine(o, indent, left);
    printFullLine(o, indent, right);
    return decIndent(o, indent);
  }
};

class Convert : public Expression {
public:
  ConvertOp op;
  Expression *value;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    o << '(';
    prepareColor(o);
    switch (op) {
      case ConvertUInt32: o << "f64.convert_u/i32"; break;
      case ConvertSInt32: o << "f64.convert_s/i32"; break;
      case TruncSFloat64: o << "i32.trunc_s/f64"; break;
      default: abort();
    }
    restoreNormalColor(o);
    incIndent(o, indent);
    printFullLine(o, indent, value);
    return decIndent(o, indent);
  }
};

class Host : public Expression {
public:
  HostOp op;
  ExpressionList operands;
};

// Globals

struct NameType {
  Name name;
  WasmType type;
  NameType() : name(nullptr), type(none) {}
  NameType(Name name, WasmType type) : name(name), type(type) {}
};

class Function {
public:
  Name name;
  WasmType result;
  std::vector<NameType> params;
  std::vector<NameType> locals;
  Expression *body;

  std::ostream& print(std::ostream &o, unsigned indent) {
    printOpening(o, "func ", true) << name;
    if (params.size() > 0) {
      for (auto& param : params) {
        o << ' ';
        printMinorOpening(o, "param ") << param.name << ' ' << printWasmType(param.type) << ")";
      }
    }
    if (result != none) {
      o << ' ';
      printMinorOpening(o, "result ") << printWasmType(result) << ")";
    }
    incIndent(o, indent);
    for (auto& local : locals) {
      doIndent(o, indent);
      printMinorOpening(o, "local ") << local.name << ' ' << printWasmType(local.type) << ")\n";
    }
    printFullLine(o, indent, body);
    return decIndent(o, indent);
  }
};

class Import {
public:
  Name name, module, base; // name = module.base
  FunctionType type;

  std::ostream& print(std::ostream &o, unsigned indent) {
    printOpening(o, "import ") << name << ' ';
    printText(o, module.str) << ' ';
    printText(o, base.str) << ' ';
    type.print(o, indent);
    return o << ')';
  }
};

class Export {
public:
  Name name;
  Name value;

  std::ostream& print(std::ostream &o, unsigned indent) {
    printOpening(o, "export ");
    return printText(o, name.str) << ' ' << value << ')';
  }
};

class Table {
public:
  std::vector<Name> names;

  std::ostream& print(std::ostream &o, unsigned indent) {
    printOpening(o, "table");
    for (auto name : names) {
      o << ' ' << name;
    }
    return o << ')';
  }
};

class Module {
public:
  // wasm contents
  std::map<Name, FunctionType*> functionTypes;
  std::map<Name, Import> imports;
  std::vector<Export> exports;
  Table table;
  std::vector<Function*> functions;

  Module() {}

  friend std::ostream& operator<<(std::ostream &o, Module module) {
    unsigned indent = 0;
    printOpening(o, "module", true);
    incIndent(o, indent);
    doIndent(o, indent);
    printOpening(o, "memory") << " 16777216)\n"; // XXX
    for (auto& curr : module.functionTypes) {
      doIndent(o, indent);
      curr.second->print(o, indent, true);
      o << '\n';
    }
#if 0
    for (auto& curr : module.imports) {
      doIndent(o, indent);
      curr.second.print(o, indent);
      o << '\n';
    }
#endif
    for (auto& curr : module.exports) {
      doIndent(o, indent);
      curr.print(o, indent);
      o << '\n';
    }
    if (module.table.names.size() > 0) {
      doIndent(o, indent);
      module.table.print(o, indent);
      o << '\n';
    }
    for (auto& curr : module.functions) {
      doIndent(o, indent);
      curr->print(o, indent);
      o << '\n';
    }
    decIndent(o, indent);
    return o << '\n';
  }
};

//
// Simple WebAssembly AST visiting and children-first walking
//

template<class ReturnType>
struct WasmVisitor {
  virtual ReturnType visitBlock(Block *curr) = 0;
  virtual ReturnType visitIf(If *curr) = 0;
  virtual ReturnType visitLoop(Loop *curr) = 0;
  virtual ReturnType visitLabel(Label *curr) = 0;
  virtual ReturnType visitBreak(Break *curr) = 0;
  virtual ReturnType visitSwitch(Switch *curr) = 0;
  virtual ReturnType visitCall(Call *curr) = 0;
  virtual ReturnType visitCallImport(CallImport *curr) = 0;
  virtual ReturnType visitCallIndirect(CallIndirect *curr) = 0;
  virtual ReturnType visitGetLocal(GetLocal *curr) = 0;
  virtual ReturnType visitSetLocal(SetLocal *curr) = 0;
  virtual ReturnType visitLoad(Load *curr) = 0;
  virtual ReturnType visitStore(Store *curr) = 0;
  virtual ReturnType visitConst(Const *curr) = 0;
  virtual ReturnType visitUnary(Unary *curr) = 0;
  virtual ReturnType visitBinary(Binary *curr) = 0;
  virtual ReturnType visitCompare(Compare *curr) = 0;
  virtual ReturnType visitConvert(Convert *curr) = 0;
  virtual ReturnType visitHost(Host *curr) = 0;
  virtual ReturnType visitNop(Nop *curr) = 0;

  ReturnType visit(Expression *curr) {
    assert(curr);
    if (Block *cast = dynamic_cast<Block*>(curr)) return visitBlock(cast);
    if (If *cast = dynamic_cast<If*>(curr)) return visitIf(cast);
    if (Loop *cast = dynamic_cast<Loop*>(curr)) return visitLoop(cast);
    if (Label *cast = dynamic_cast<Label*>(curr)) return visitLabel(cast);
    if (Break *cast = dynamic_cast<Break*>(curr)) return visitBreak(cast);
    if (Switch *cast = dynamic_cast<Switch*>(curr)) return visitSwitch(cast);
    if (Call *cast = dynamic_cast<Call*>(curr)) return visitCall(cast);
    if (CallImport *cast = dynamic_cast<CallImport*>(curr)) return visitCallImport(cast);
    if (CallIndirect *cast = dynamic_cast<CallIndirect*>(curr)) return visitCallIndirect(cast);
    if (GetLocal *cast = dynamic_cast<GetLocal*>(curr)) return visitGetLocal(cast);
    if (SetLocal *cast = dynamic_cast<SetLocal*>(curr)) return visitSetLocal(cast);
    if (Load *cast = dynamic_cast<Load*>(curr)) return visitLoad(cast);
    if (Store *cast = dynamic_cast<Store*>(curr)) return visitStore(cast);
    if (Const *cast = dynamic_cast<Const*>(curr)) return visitConst(cast);
    if (Unary *cast = dynamic_cast<Unary*>(curr)) return visitUnary(cast);
    if (Binary *cast = dynamic_cast<Binary*>(curr)) return visitBinary(cast);
    if (Compare *cast = dynamic_cast<Compare*>(curr)) return visitCompare(cast);
    if (Convert *cast = dynamic_cast<Convert*>(curr)) return visitConvert(cast);
    if (Host *cast = dynamic_cast<Host*>(curr)) return visitHost(cast);
    if (Nop *cast = dynamic_cast<Nop*>(curr)) return visitNop(cast);
    abort();
  }
};

struct WasmWalker : public WasmVisitor<Expression*> {
  wasm::Arena* allocator; // use an existing allocator, or null if no allocations

  WasmWalker() : allocator(nullptr) {}
  WasmWalker(wasm::Arena* allocator) : allocator(allocator) {}

  // Each method receives an AST pointer, and it is replaced with what is returned.
  Expression* visitBlock(Block *curr) override { return curr; };
  Expression* visitIf(If *curr) override { return curr; };
  Expression* visitLoop(Loop *curr) override { return curr; };
  Expression* visitLabel(Label *curr) override { return curr; };
  Expression* visitBreak(Break *curr) override { return curr; };
  Expression* visitSwitch(Switch *curr) override { return curr; };
  Expression* visitCall(Call *curr) override { return curr; };
  Expression* visitCallImport(CallImport *curr) override { return curr; };
  Expression* visitCallIndirect(CallIndirect *curr) override { return curr; };
  Expression* visitGetLocal(GetLocal *curr) override { return curr; };
  Expression* visitSetLocal(SetLocal *curr) override { return curr; };
  Expression* visitLoad(Load *curr) override { return curr; };
  Expression* visitStore(Store *curr) override { return curr; };
  Expression* visitConst(Const *curr) override { return curr; };
  Expression* visitUnary(Unary *curr) override { return curr; };
  Expression* visitBinary(Binary *curr) override { return curr; };
  Expression* visitCompare(Compare *curr) override { return curr; };
  Expression* visitConvert(Convert *curr) override { return curr; };
  Expression* visitHost(Host *curr) override { return curr; };
  Expression* visitNop(Nop *curr) override { return curr; };

  // children-first
  Expression *walk(Expression *curr) {
    if (!curr) return curr;

    if (Block *cast = dynamic_cast<Block*>(curr)) {
      ExpressionList& list = cast->list;
      for (size_t z = 0; z < list.size(); z++) {
        list[z] = walk(list[z]);
      }
    } else if (If *cast = dynamic_cast<If*>(curr)) {
      cast->condition = walk(cast->condition);
      cast->ifTrue = walk(cast->ifTrue);
      cast->ifFalse = walk(cast->ifFalse);
    } else if (Loop *cast = dynamic_cast<Loop*>(curr)) {
      cast->body = walk(cast->body);
    } else if (Label *cast = dynamic_cast<Label*>(curr)) {
    } else if (Break *cast = dynamic_cast<Break*>(curr)) {
      cast->condition = walk(cast->condition);
      cast->value = walk(cast->value);
    } else if (Switch *cast = dynamic_cast<Switch*>(curr)) {
      cast->value = walk(cast->value);
      for (auto& curr : cast->cases) {
        curr.body = walk(curr.body);
      }
      cast->default_ = walk(cast->default_);
    } else if (Call *cast = dynamic_cast<Call*>(curr)) {
      ExpressionList& list = cast->operands;
      for (size_t z = 0; z < list.size(); z++) {
        list[z] = walk(list[z]);
      }
    } else if (CallImport *cast = dynamic_cast<CallImport*>(curr)) {
      ExpressionList& list = cast->operands;
      for (size_t z = 0; z < list.size(); z++) {
        list[z] = walk(list[z]);
      }
    } else if (CallIndirect *cast = dynamic_cast<CallIndirect*>(curr)) {
      cast->target = walk(cast->target);
      ExpressionList& list = cast->operands;
      for (size_t z = 0; z < list.size(); z++) {
        list[z] = walk(list[z]);
      }
    } else if (GetLocal *cast = dynamic_cast<GetLocal*>(curr)) {
    } else if (SetLocal *cast = dynamic_cast<SetLocal*>(curr)) {
      cast->value = walk(cast->value);
    } else if (Load *cast = dynamic_cast<Load*>(curr)) {
      cast->ptr = walk(cast->ptr);
    } else if (Store *cast = dynamic_cast<Store*>(curr)) {
      cast->ptr = walk(cast->ptr);
      cast->value = walk(cast->value);
    } else if (Const *cast = dynamic_cast<Const*>(curr)) {
    } else if (Unary *cast = dynamic_cast<Unary*>(curr)) {
      cast->value = walk(cast->value);
    } else if (Binary *cast = dynamic_cast<Binary*>(curr)) {
      cast->left = walk(cast->left);
      cast->right = walk(cast->right);
    } else if (Compare *cast = dynamic_cast<Compare*>(curr)) {
      cast->left = walk(cast->left);
      cast->right = walk(cast->right);
    } else if (Convert *cast = dynamic_cast<Convert*>(curr)) {
      cast->value = walk(cast->value);
    } else if (Host *cast = dynamic_cast<Host*>(curr)) {
      ExpressionList& list = cast->operands;
      for (size_t z = 0; z < list.size(); z++) {
        list[z] = walk(list[z]);
      }
    } else if (Nop *cast = dynamic_cast<Nop*>(curr)) {
    } else {
      abort();
    }
    return visit(curr);
  }

  void startWalk(Function *func) {
    func->body = walk(func->body);
  }
};

} // namespace wasm

#endif // __wasm_h__

