//
// WebAssembly representation and processing library
//

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

namespace wasm {

// Basics

typedef const char *Name;

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
    assert(((size_t)str) > MAX_NUM);
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

struct Literal {
  BasicType type;
  union value {
    int32_t i32;
    int64_t i64;
    float f32;
    double f64;
  };
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
};

typedef std::vector<Expression*> ExpressionList; // TODO: optimize  

class Nop : public Expression {
};

class Block : public Expression {
  Var var;
  ExpressionList list;
};

class If : public Expression {
  Expression *condition, *ifTrue, *ifFalse;
};

class Loop : public Expression {
  Var out, in;
  Expression *body;
};

class Label : public Expression {
  Var var;
};

class Break : public Expression {
  Var var;
  Expression *condition;
};

class Switch : public Expression {
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
  Var target;
  ExpressionList operands;
};

class CallImport : public Call {
};

class CallIndirect : public Expression {
  Var type;
  Expression *target;
  ExpressionList operands;
};

class GetLocal : public Expression {
  Var id;
};

class SetLocal : public Expression {
  Var id;
  Expression *value;
};

class Load : public Expression {
  unsigned bytes;
  bool signed_;
  int offset;
  unsigned align;
  Expression *ptr;
};

class Store : public Expression {
  unsigned bytes;
  int offset;
  unsigned align;
  Expression *ptr, *value;
};

class Const : public Expression {
  Literal value;
};

class Unary : public Expression {
  UnaryOp op;
  Expression *value;
};

class Binary : public Expression {
  BinaryOp op;
  Expression *left, *right;
};

class Compare : public Expression {
  RelationalOp op;
  Expression *left, *right;
};

class Convert : public Expression {
  ConvertOp op;
  Expression *value;
};

class Host : public Expression {
  HostOp op;
  ExpressionList operands;
};

// Globals

struct NameType {
  Name name;
  BasicType type;
};

class CustomType {
  NameType self;
  std::vector<NameType> params;
};

class Function {
  NameType self;
  std::vector<NameType> params;
  std::vector<NameType> locals;
};

class Import {
  Name name;
  CustomType type;
};

class Export {
  Name name;
  Var value;
};

class Table {
  std::vector<Var> vars;
};

class Module {
  std::map<Var, void*> map; // maps var ids/names to things

  std::vector<CustomType> customTypes;
  std::vector<Function> functions;
  std::vector<Import> imports;
  std::vector<Export> exports;
  Table table;
};

} // namespace wasm

