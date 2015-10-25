//
// WebAssembly representation and processing library
//

#include <assert.h>

namespace wasm {

// Vars

// A 'var' in the spec.
class Var {
  enum {
    MAX_NUM = 1000000 // less than this, a num, higher, a string; 0 = null
  }
  union {
    unsigned num;    // numeric ID
    const char *str; // string
  };
public:
  Var() : num(0) {}
  Var(unsigned num) : num(num) {
    assert(num > 0 && num < MAX_NUM);
  }
  Var(const char *str) : str(str) {
    assert(((size_t)str) > MAX_NUM);
  }
};

// Operators

namespace IntOp {
  enum unop { Clz, Ctz, Popcnt }
  enum binop { Add, Sub, Mul, DivS, DivU, RemS, RemU, And, Or, Xor, Shl, ShrU, ShrS }
  enum relop { Eq, Ne, LtS, LtU, LeS, LeU, GtS, GtU, GeS, GeU }
  enum cvt { ExtendSInt32, ExtendUInt32, WrapInt64, TruncSFloat32, TruncUFloat32, TruncSFloat64, TruncUFloat64, ReinterpretFloat }
}

namespace FloatOp {
  enum unop { Neg, Abs, Ceil, Floor, Trunc, Nearest, Sqrt }
  enum binop { Add, Sub, Mul, Div, CopySign, Min, Max }
  enum relop { Eq, Ne, Lt, Le, Gt, Ge }
  enum cvt { ConvertSInt32, ConvertUInt32, ConvertSInt64, ConvertUInt64, PromoteFloat32, DemoteFloat64, ReinterpretInt }
}

namespace HostOp {
  enum hostop { PageSize, MemorySize, GrowMemory, HasFeature }
}

// Expressions

typedef std::vector<Expression*> ExpressionList; // TODO: optimize  

class Expression {
};

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
  bool unsigned;
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
  IntOp::unop op;
  Expression *value;
};

class Binary : public Expression {
  IntOp::binop op;
  Expression *left, *right;
};

class Compare : public Expression {
  IntOp::relop op;
  Expression *left, *right;
};

class Convert : public Expression {
  IntOp::cvt op;
  Expression *value;
};

class Host : public Expression {
  HostOp::hostop op;
  ExpressionList operands;
};

} // namespace wasm

