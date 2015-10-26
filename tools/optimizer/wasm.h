//
// WebAssembly representation and processing library
//

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

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
    return ret;
  }

  ~Arena() {
    for (char* chunk : chunks) {
      delete[] chunk;
    }
  }
};

std::ostream &doIndent(std::ostream &o, unsigned indent) {
  for (unsigned i = 0; i < indent; i++) {
    o << "  ";
  }
  return o;
}
void incIndent(std::ostream &o, unsigned& indent) {
  o << '\n';
  indent++;    
}
void decIndent(std::ostream &o, unsigned& indent) {
  indent--;
  doIndent(o, indent);
  o << ')';
}

// Basics

typedef cashew::IString Name;

// A 'var' in the spec.
class Var {
  enum {
    MAX_NUM = 1000000 // less than this, a num, higher, a string; 0 = null
  };
  union {
    unsigned num; // numeric ID
    Name str;     // string
  };
public:
  Var() : num(0) {}
  Var(unsigned num) : num(num) {
    assert(num > 0 && num < MAX_NUM);
  }
  Var(Name str) : str(str) {
    assert(num > MAX_NUM);
  }

  bool is() {
    return num != 0;
  }

  std::ostream& print(std::ostream &o) {
    if (num < MAX_NUM) {
      o << num;
    } else {
      o << str.str;
    }
    return o;
  }
};

// Types

enum BasicType {
  none,
  i32,
  i64,
  f32,
  f64
};

std::ostream& print(std::ostream &o, BasicType type) {
  switch (type) {
    case BasicType::none: o << "none";
    case BasicType::i32: o << "i32";
    case BasicType::i64: o << "i64";
    case BasicType::f32: o << "f32";
    case BasicType::f64: o << "f64";
  }
  return o;
}

struct Literal {
  BasicType type;
  union {
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;
  };

  std::ostream& print(std::ostream &o) {
    switch (type) {
      case none: abort();
      case BasicType::i32: o << i32;
      case BasicType::i64: o << i64;
      case BasicType::f32: o << f32;
      case BasicType::f64: o << f64;
    }
    return o;
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
  virtual std::ostream& print(std::ostream &o, unsigned indent) = 0;
};

std::ostream& printFullLine(std::ostream &o, unsigned indent, Expression *expression) {
  doIndent(o, indent);
  expression->print(o, indent);
  o << '\n';
}

typedef std::vector<Expression*> ExpressionList; // TODO: optimize  

class Nop : public Expression {
  std::ostream& print(std::ostream &o, unsigned indent) override {
    o << "nop";
    return o;
  }
};

class Block : public Expression {
public:
  Var var;
  ExpressionList list;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    o << "(block";
    if (var.is()) {
      o << " ";
      var.print(o);
    }
    incIndent(o, indent);
    for (auto expression : list) {
      printFullLine(o, indent, expression);
    }
    decIndent(o, indent);
    return o;
  }
};

class If : public Expression {
public:
  Expression *condition, *ifTrue, *ifFalse;

  std::ostream& print(std::ostream &o, unsigned indent) override {
    o << "(if";
    incIndent(o, indent);
    printFullLine(o, indent, condition);
    printFullLine(o, indent, ifTrue);
    if (ifFalse) printFullLine(o, indent, ifFalse);
    decIndent(o, indent);
    return o;
  }
};

class Loop : public Expression {
public:
  Var out, in;
  Expression *body;
};

class Label : public Expression {
public:
  Var var;
};

class Break : public Expression {
public:
  Var var;
  Expression *condition;
};

class Switch : public Expression {
public:
  struct Case {
    Literal value;
    Expression *body;
    bool fallthru;
  };

  Var var;
  Expression *value;
  std::vector<Case> cases;
  Expression *default_;
};

class Call : public Expression {
public:
  Var target;
  ExpressionList operands;
};

class CallImport : public Call {
};

class CallIndirect : public Expression {
public:
  Var type;
  Expression *target;
  ExpressionList operands;
};

class GetLocal : public Expression {
public:
  Var id;
};

class SetLocal : public Expression {
public:
  Var id;
  Expression *value;
};

class Load : public Expression {
public:
  unsigned bytes;
  bool signed_;
  int offset;
  unsigned align;
  Expression *ptr;
};

class Store : public Expression {
public:
  unsigned bytes;
  int offset;
  unsigned align;
  Expression *ptr, *value;
};

class Const : public Expression {
public:
  Literal value;
};

class Unary : public Expression {
public:
  UnaryOp op;
  Expression *value;
};

class Binary : public Expression {
public:
  BinaryOp op;
  Expression *left, *right;
};

class Compare : public Expression {
public:
  RelationalOp op;
  Expression *left, *right;
};

class Convert : public Expression {
public:
  ConvertOp op;
  Expression *value;
};

class Host : public Expression {
public:
  HostOp op;
  ExpressionList operands;
};

// Globals

struct NameType {
  Name name;
  BasicType type;
  NameType(Name name, BasicType type) : name(name), type(type) {}
};

class GeneralType {
public:
  GeneralType() : basic(BasicType::none) {}
  GeneralType(BasicType basic) : basic(basic) {}

  BasicType basic; // if none, then custom/function, and other params matter
  Name name;
  BasicType result;
  std::vector<NameType> params;
};

class Function {
public:
  Name name;
  BasicType result;
  std::vector<NameType> params;
  std::vector<NameType> locals;
  Expression *body;

  std::ostream& print(std::ostream &o, unsigned indent) {
    o << "(func " << name.str << " ";
    for (auto& param : params) {
      o << "(param " << param.name.str << " ";
      print(o, param.type) << ") ";
    }
    o << "(result ";
    print(o, result) << ")";
    incIndent(o, indent);
    for (auto& local : locals) {
      doIndent(o, indent);
      o << "(local " << local.name.str << " ";
      print(o, local.type) << ")\n";
    }
    printFullLine(o, indent, body);
    decIndent(o, indent);
    return o;
  }
};

class Import {
public:
  Name name, module, base; // name = module.base
  GeneralType type;
};

class Export {
public:
  Name name;
  Var value;
};

class Table {
public:
  std::vector<Var> vars;
};

class Module {
protected:
  // wasm contents
  std::vector<GeneralType> customTypes;
  std::vector<Import> imports;
  std::vector<Export> exports;
  Table table;
  std::vector<Function*> functions;

  // internals
  std::map<Var, void*> map; // maps var ids/names to things
  unsigned nextVar;

public:
  Module() : nextVar(1) {}

  std::ostream& print(std::ostream &o) {
    unsigned indent = 0;
    o << "(module";
    incIndent(o, indent);
    /*
    for (auto& curr : customTypes) {
      curr.print(o, indent);
    }
    for (auto& curr : imports) {
      curr.print(o, indent);
    }
    for (auto& curr : exports) {
      curr.print(o, indent);
    }
    for (auto& curr : table) {
      curr.print(o, indent);
    }
    */
    for (auto& curr : functions) {
      curr->print(o, indent);
    }
    decIndent(o, indent);
  }
};

} // namespace wasm

