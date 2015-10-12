// vim: set ts=2 sw=2 tw=99 et:

#include "shared.h"
#include "unpack.h"
#include "parser.h"

#include <unordered_map>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <memory>

using namespace std;
using cashew::IString;

namespace asmjs {

class Module;

// =================================================================================================
// Utilities

template <class To, class From>
To
union_cast(From from)
{
  static_assert(sizeof(To) == sizeof(From), "Sizes should match");
  union {
    From from;
    To to;
  } u = { from };
  return u.to;
}

// =================================================================================================
// Output writing

class Out
{
  std::ostream& os_;

  template <class T> void u8(T t) { os_.put(uint8_t(t)); }

public:
  Out(std::ostream& os) : os_(os) {}

  template <class T> void fixed_width(T);
  void code(Stmt s) { u8(s); }
  void code(SwitchCase c) { u8(c); }
  void code(I32 i) { assert(i < I32::Bad); u8(i); }
  void code(F32 f) { assert(f < F32::Bad); u8(f); }
  void code(F64 f) { assert(f < F64::Bad); u8(f); }
  void code(Void v) { assert(v < Void::Bad); u8(v); }
  void code(Expr e) { u8(e.raw_code()); }
  void code(ExportFormat f) { u8(f); }
  void code(Type t) { u8(t); }
  void code(RType t) { u8(t); }
  void code(VarTypes t) { u8(t); }
  inline void code(ExprWithImm, uint8_t);
  inline void code(StmtWithImm, uint8_t);
  inline void code(VarTypesWithImm, uint8_t);
  inline void imm_u32(uint32_t u32);
  inline void imm_s32(int32_t s32);
  void c_str(const char*);
};

template <class T>
void
Out::fixed_width(T t)
{
  union {
    T t;
    uint8_t arr[sizeof(T)];
  } u = { t };
  for (auto u8 : u.arr)
    os_.put(u8);
}

void inline
Out::code(ExprWithImm e, uint8_t imm)
{
  assert(imm < ImmLimit);
  u8(PackOpWithImm(e.raw_code(), imm));
}

void inline
Out::code(StmtWithImm s, uint8_t imm)
{
  assert(imm < ImmLimit);
  u8(PackOpWithImm(uint8_t(s), imm));
}

void inline
Out::code(VarTypesWithImm l, uint8_t imm)
{
  assert(imm < ImmLimit);
  u8(PackOpWithImm(uint8_t(l), imm));
}

void
Out::imm_u32(uint32_t u32)
{
  if (u32)
    for (; true; u32 >>= 7) {
      if (u32 < 0x80) {
        os_.put(u32);
        return;
      }
      os_.put(0x80 | (u32 & 0x7f));
    }
  else
    os_.put(0);
}

void
Out::imm_s32(int32_t s32)
{
  if (s32)
    for (; true; s32 >>= 7) {
      if (-64 <= s32 && s32 < 64) {
        os_.put(s32 & 0x7f);
        return;
      }
      os_.put(0x80 | (s32 & 0x7f));
    }
  else
    os_.put(0);
}

void
Out::c_str(const char* p)
{
  do {
    os_.put(*p);
  } while(*p++);
}

// =================================================================================================
// AST

class AstMemory
{
  vector<unique_ptr<uint8_t[]>> allocs_;
  uint8_t* free_;
  size_t remain_;

public:
  AstMemory() : free_(nullptr), remain_(0) {}

  void* alloc(size_t bytes)
  {
    bytes = (bytes + 7)&-8; // some of our data structures, like DoubleNode, need 8-byte alignment

    if (remain_ < bytes) {
      remain_ = max<size_t>(bytes, 64 * 1024);
      allocs_.emplace_back(new uint8_t[remain_]);
      free_ = allocs_.back().get();
    }

    uint8_t* ret = free_;
    free_ += bytes;
    remain_ -= bytes;
    return ret;
  }

} ast_memory;

struct AstMemoryBase
{
  void* operator new(size_t bytes) { return ast_memory.alloc(bytes); }
};

template <class T>
struct ListElemBase
{
  ListElemBase() : next(nullptr) {}

  T* next;
};

template <class T>
struct List
{
  List() : first(nullptr), last(nullptr) {}

  void append(T& n)
  {
    if (!first) {
      first = last = &n;
    } else {
      last->next = &n;
      last = &n;
    }
  }

  void append_list(List<T>& list)
  {
    for (T* p = list.first; p; p = p->next)
      append(*p);
  }

  uint32_t compute_length() const
  {
    uint32_t len = 0;
    for (T* p = first; p; p = p->next)
      len++;
    return len;
  }

  T* first;
  T* last;
};

struct AstNode : AstMemoryBase, ListElemBase<AstNode>
{
  enum Enum {
    TopLevel, Function,
    Var, Return, Block, If, While, Do, Label, Break, Continue, Switch,
    Double, Int, Name, Prefix, Binary, Ternary, Call, String, Object, New, Dot, Index, Array
  } which;

  template <class T> bool is() const { return which == T::Which; }
  template <class T> T& as() { assert(is<T>()); return *static_cast<T*>(this); }
  template <class T> const T& as() const  { assert(is<T>()); return *static_cast<const T*>(this); }

  AstNode(Enum which) : which(which) {}
};

struct TopLevelNode : AstNode, List<AstNode>
{
  static const Enum Which = TopLevel;
  TopLevelNode() : AstNode(Which) {}
};

struct ArgNode : AstMemoryBase, ListElemBase<ArgNode>
{
  ArgNode(IString name) : name(name) {}

  IString const name;
};

struct FuncNode : AstNode, List<AstNode>, List<ArgNode>
{
  static const Enum Which = Function;
  FuncNode(IString name) : AstNode(Which), name(name) {}

  IString const name;
};

struct VarNameNode : AstMemoryBase, ListElemBase<VarNameNode>
{
  VarNameNode(IString name, AstNode& init) : name(name), init(init) {}

  IString const name;
  AstNode& init;
};

struct VarNode : AstNode, List<VarNameNode>
{
  static const Enum Which = Var;
  VarNode() : AstNode(Which) {}
};

struct ReturnNode : AstNode
{
  static const Enum Which = Return;
  ReturnNode(AstNode* expr) : AstNode(Which), expr(expr) {}

  AstNode* const expr;
};

struct BlockNode : AstNode, List<AstNode>
{
  static const Enum Which = Block;
  BlockNode() : AstNode(Which) {}
  BlockNode(AstNode& a, AstNode& b) : AstNode(Which) { append(a); append(b); }
};

struct IfNode : AstNode
{
  static const Enum Which = If;
  IfNode(AstNode& cond, AstNode& if_true, AstNode* if_false)
  : AstNode(Which)
  , cond(cond)
  , if_true(if_true)
  , if_false(if_false)
  {}

  AstNode& cond;
  AstNode& if_true;
  AstNode* if_false;
};

struct WhileNode : AstNode
{
  static const Enum Which = While;
  WhileNode(AstNode& cond, AstNode& body) : AstNode(Which), cond(cond), body(body) {}

  AstNode& cond;
  AstNode& body;
};

struct DoNode : AstNode
{
  static const Enum Which = Do;
  DoNode(AstNode& body, AstNode& cond) : AstNode(Which), body(body), cond(cond) {}

  AstNode& body;
  AstNode& cond;
};

struct LabelNode : AstNode
{
  static const Enum Which = Label;
  LabelNode(IString str, AstNode& stmt) : AstNode(Which), str(str), stmt(stmt) {}

  const IString str;
  AstNode& stmt;
};

struct BreakNode : AstNode
{
  static const Enum Which = Break;
  BreakNode(IString str) : AstNode(Which), str(str) {}

  const IString str;
};

struct ContinueNode : AstNode
{
  static const Enum Which = Continue;
  ContinueNode(IString str) : AstNode(Which), str(str) {}

  const IString str;
};

struct CaseStmtNode : AstMemoryBase, ListElemBase<CaseStmtNode>
{
  CaseStmtNode(AstNode& stmt) : stmt(stmt) {}

  AstNode& stmt;
};

struct CaseNode : AstMemoryBase, ListElemBase<CaseNode>, List<CaseStmtNode>
{
  CaseNode() : label(nullptr) {}
  CaseNode(AstNode& label) : label(&label) {}

  AstNode* const label;
};

struct SwitchNode : AstNode, List<CaseNode>
{
  static const Enum Which = Switch;
  SwitchNode(AstNode& expr) : AstNode(Which), expr(expr) {}

  AstNode& expr;
};

struct DoubleNode : AstNode
{
  static const Enum Which = Double;
  DoubleNode(double f64) : AstNode(Which), f64(f64) {}

  const double f64;
};

struct IntNode : AstNode
{
  static const Enum Which = Int;
  IntNode(uint32_t u32) : AstNode(Which), u32(u32) {}

  const uint32_t u32;
};

struct NameNode : AstNode
{
  static const Enum Which = Name;
  NameNode(IString str) : AstNode(Which), str(str) {}

  uint32_t index;
  IString const str;

  Expr expr;
  ExprWithImm expr_with_imm;
};

struct PrefixNode : AstNode
{
  static const Enum Which = Prefix;
  PrefixNode(IString op, AstNode& kid)
  : AstNode(Which)
  , op(op)
  , kid(kid)
  {}

  IString const op;
  AstNode& kid;

  Expr expr;
};

struct BinaryNode : AstNode
{
  static const Enum Which = Binary;
  BinaryNode(IString op, AstNode& lhs, AstNode& rhs)
  : AstNode(Which)
  , op(op)
  , lhs(lhs)
  , rhs(rhs)
  , kind(Generic)
  , stmt(Stmt::Bad)
  , stmt_with_imm(StmtWithImm::Bad)
  , comma_lhs_type(RType(-1))
  {}

  IString const op;
  AstNode& lhs;
  AstNode& rhs;

  enum Kind { Bitwise, Assign, Store, Comma, Generic } kind;
  Expr expr;
  Stmt stmt;
  StmtWithImm stmt_with_imm;
  RType comma_lhs_type;
  Expr store_rhs_conv;
};

struct TernaryNode : AstNode
{
  static const Enum Which = Ternary;
  TernaryNode(AstNode& cond, AstNode& lhs, AstNode& rhs)
  : AstNode(Which)
  , cond(cond)
  , lhs(lhs)
  , rhs(rhs)
  {}

  AstNode& cond;
  AstNode& lhs;
  AstNode& rhs;

  Expr expr;
};

struct CallNode : AstNode, List<AstNode>
{
  static const Enum Which = Call;
  CallNode(AstNode& callee)
  : AstNode(Which)
  , import_preindex(UINT32_MAX)
  , callee(callee)
  , kind(Kind(UINT32_MAX))
  , stmt(Stmt::Bad)
  {}

  uint32_t import_preindex;
  AstNode& callee;

  enum Kind { Internal, Indirect, Import, NaryBuiltin, FixedArityBuiltin, Fround } kind;
  Expr expr;
  Stmt stmt;
};

struct StringNode : AstNode
{
  static const Enum Which = String;
  StringNode(IString str) : AstNode(Which), str(str) {}

  IString const str;
};

struct FieldNode : AstMemoryBase, ListElemBase<FieldNode>
{
  FieldNode(IString key, AstNode& value) : key(key), value(value) {}

  IString const key;
  AstNode& value;
};

struct ObjectNode : AstNode, List<FieldNode>
{
  static const Enum Which = Object;
  ObjectNode() : AstNode(Which) {}
};

struct NewNode : AstNode
{
  static const Enum Which = New;
  NewNode(CallNode& call) : AstNode(Which), call(call) {}

  CallNode& call;
};

struct DotNode : AstNode
{
  static const Enum Which = Dot;
  DotNode(AstNode& base, IString name) : AstNode(Which), base(base), name(name) {}

  AstNode& base;
  IString name;
};

struct IndexNode : AstNode
{
  static const Enum Which = Index;
  IndexNode(AstNode& array, AstNode& index)
  : AstNode(Which)
  , array(array)
  , index(&index)
  , constant(false)
  , offset(0)
  {}

  AstNode& array;
  AstNode* index;

  bool constant;
  Expr expr;
  uint32_t offset;
};

struct ArrayNode : AstNode, List<AstNode>
{
  static const Enum Which = Array;
  ArrayNode() : AstNode(Which) {}
};

// =================================================================================================
// asm.js types

class AsmJSType
{
  public:
    enum Enum { Fixnum, Signed, Unsigned, Int, Float, Double, Void };
  private:
    Enum which_;
  public:
    AsmJSType() {}
    AsmJSType(Enum which) : which_(which) {}
    Enum which() const { return which_; }

    Type type() const {
      switch (which_) {
        case Fixnum: case Signed: case Unsigned: case Int:
          return Type::I32;
        case Float:
          return Type::F32;
        case Double:
          return Type::F64;
        case Void:;
      }
      return unreachable<Type>();
    }

    RType rtype() const {
      switch (which_) {
        case Fixnum: case Signed: case Unsigned: case Int:
          return RType::I32;
        case Float:
          return RType::F32;
        case Double:
          return RType::F64;
        case Void:
          return RType::Void;
      }
      return unreachable<RType>();
    }

    bool is_signed() const { return which_ == Fixnum || which_ == Signed; }
    bool is_unsigned() const { return which_ == Fixnum || which_ == Unsigned; }
    bool is_int() const { return type() == Type::I32; }
    bool is_float() const { return which_ == Float; }
    bool is_double() const { return which_ == Double; }
    AsmJSType forget_signedness() const { return type_switch(type(), Int, Float, Double); }
};

class NumLit
{
  AsmJSType asmjs_type_;
  union {
    uint32_t u32_;
    float f32_;
    double f64_;
  } u;

  static bool is_non_float_lit(const AstNode& n)
  {
    if (n.is<IntNode>() || n.is<DoubleNode>())
      return true;
    if (!n.is<PrefixNode>())
      return false;
    const PrefixNode& pre = n.as<PrefixNode>();
    if (!pre.op.equals("-"))
      return false;
    return pre.kid.is<IntNode>() || pre.kid.is<DoubleNode>();
  }

public:
  static bool is(Module& m, const AstNode& n);
  explicit NumLit(Module& m, const AstNode& n);
  explicit NumLit(double f64) : asmjs_type_(AsmJSType::Double) { u.f64_ = f64; }

  AsmJSType asmjs_type() const { return asmjs_type_; }
  Type type() const { return asmjs_type_.type(); }

  uint32_t uint32() const { assert(asmjs_type_.is_int()); return u.u32_; }
  int32_t int32() const { assert(asmjs_type_.is_int()); return u.u32_; }
  float float32() const { assert(asmjs_type_.is_float()); return u.f32_; }
  double float64() const { assert(asmjs_type_.is_double()); return u.f64_; }
};

// =================================================================================================
// Module

class Function
{
  Signature sig_;
  uint32_t sig_index_;
  bool set_ret_type_;
  unordered_map<IString, uint32_t> local_name_to_index_;
  vector<Type> locals_;
  uint32_t num_i32s_;
  uint32_t num_f32s_;
  uint32_t num_f64s_;
  uint32_t f32_temp_;
  uint32_t f64_temp_;
  const AstNode* body_;
  unordered_map<IString, uint32_t> label_to_depth_;

  void add_var(IString name, Type type)
  {
    assert(local_name_to_index_.find(name) == local_name_to_index_.end());
    local_name_to_index_.emplace(name, locals_.size());
    locals_.push_back(type);
  }

public:
  explicit Function()
  : sig_(RType::Void)
  , set_ret_type_(false)
  , num_i32s_(0)
  , num_f32s_(0)
  , num_f64s_(0)
  , f32_temp_(UINT32_MAX)
  , f64_temp_(UINT32_MAX)
  , body_(nullptr)
  {}

  void add_arg(IString name, Type type)
  {
    assert(local_name_to_index_.find(name) == local_name_to_index_.end());
    assert(locals_.size() == sig_.args.size());
    local_name_to_index_.emplace(name, locals_.size());
    locals_.push_back(type);
    sig_.args.push_back(type);
  }

  void start_body(const AstNode* body,
                  const vector<IString>& i32s,
                  const vector<IString>& f32s,
                  const vector<IString>& f64s)
  {
    assert(!body_);
    body_ = body;
    num_i32s_ = i32s.size();
    num_f32s_ = f32s.size();
    num_f64s_ = f64s.size();
    for (IString name : i32s)
      add_var(name, Type::I32);
    for (IString name : f32s)
      add_var(name, Type::F32);
    for (IString name : f64s)
      add_var(name, Type::F64);
  }

  void set_needs_f32_temp()
  {
    if (f32_temp_ != UINT32_MAX)
      return;
    f32_temp_ = sig_.args.size() + num_i32s_ + num_f32s_;
    num_f32s_++;
    locals_.insert(locals_.begin() + f32_temp_, Type::F32);
  }

  void set_needs_f64_temp()
  {
    if (f64_temp_ != UINT32_MAX)
      return;
    f64_temp_ = sig_.args.size() + num_i32s_ + num_f32s_ + num_f64s_;
    num_f64s_++;
    locals_.insert(locals_.begin() + f64_temp_, Type::F64);
  }

  void set_ret_type(RType ret)
  {
    assert(!set_ret_type_ || sig_.ret == ret);
    set_ret_type_ = true;
    sig_.ret = ret;
  }

  void push_label(IString name)
  {
    assert(label_to_depth_.find(name) == label_to_depth_.end());
    label_to_depth_.emplace(name, label_to_depth_.size());
  }

  void pop_label(IString name)
  {
    assert(label_to_depth_.find(name)->second == label_to_depth_.size() - 1);
    label_to_depth_.erase(name);
  }

  uint32_t label_depth(IString name) const
  {
    return label_to_depth_.find(name)->second;
  }

  const Signature& sig() const { return sig_; }
  void set_sig_index(uint32_t sig_index) { sig_index_ = sig_index; }
  uint32_t sig_index() const { return sig_index_; }

  uint32_t num_vars() const { return locals_.size() - sig_.args.size(); }
  uint32_t num_i32_vars() const { return num_i32s_; }
  uint32_t num_f32_vars() const { return num_f32s_; }
  uint32_t num_f64_vars() const { return num_f64s_; }
  uint32_t num_locals() const { return locals_.size(); }
  bool is_local_name(IString name) { return local_name_to_index_.find(name) != local_name_to_index_.end(); }
  uint32_t local_index(IString name) const { return local_name_to_index_.find(name)->second; }
  Type local_type(size_t i) const { return locals_[i]; }
  const AstNode* body() const { return body_; }
  uint32_t f32_temp() const { assert(f32_temp_ != UINT32_MAX); return f32_temp_; }
  uint32_t f64_temp() const { assert(f64_temp_ != UINT32_MAX); return f64_temp_; }
};

enum class PreTypeCode { Neg, Add, Sub, Mul, Eq, NEq, Abs, Ceil, Floor, Sqrt, Comma };

inline Expr
specialize(PreTypeCode pre, Type type)
{
  switch (pre) {
    case PreTypeCode::Neg:   return type_switch(type, I32::Neg,    F32::Neg,    F64::Neg);
    case PreTypeCode::Add:   return type_switch(type, I32::Add,    F32::Add,    F64::Add);
    case PreTypeCode::Sub:   return type_switch(type, I32::Sub,    F32::Sub,    F64::Sub);
    case PreTypeCode::Mul:   return type_switch(type, I32::Mul,    F32::Mul,    F64::Mul);
    case PreTypeCode::Eq:    return type_switch(type, I32::EqI32,  I32::EqF32,  I32::EqF64);
    case PreTypeCode::NEq:   return type_switch(type, I32::NEqI32, I32::NEqF32, I32::NEqF64);
    case PreTypeCode::Abs:   return type_switch(type, I32::Abs,    F32::Abs,    F64::Abs);
    case PreTypeCode::Ceil:  return type_switch(type, I32::Bad,    F32::Ceil,   F64::Ceil);
    case PreTypeCode::Floor: return type_switch(type, I32::Bad,    F32::Floor,  F64::Floor);
    case PreTypeCode::Sqrt:  return type_switch(type, I32::Bad,    F32::Sqrt,   F64::Sqrt);
    case PreTypeCode::Comma: return type_switch(type, I32::Comma,  F32::Comma,  F64::Comma);
  }
  return unreachable<Expr>();
}

enum class PreTypeSignCode { ToF32, Div, Mod, LeTh, LeEq, GrTh, GrEq, Min, Max };

inline Expr
specialize(PreTypeSignCode pre, Signedness si)
{
  switch (pre) {
    case PreTypeSignCode::ToF32: return signedness_switch(si, F32::FromS32,  F32::FromU32);
    case PreTypeSignCode::Div:   return signedness_switch(si, I32::SDiv,     I32::UDiv);
    case PreTypeSignCode::Mod:   return signedness_switch(si, I32::SMod,     I32::UMod);
    case PreTypeSignCode::LeTh:  return signedness_switch(si, I32::SLeThI32, I32::ULeThI32);
    case PreTypeSignCode::LeEq:  return signedness_switch(si, I32::SLeEqI32, I32::ULeEqI32);
    case PreTypeSignCode::GrTh:  return signedness_switch(si, I32::SGrThI32, I32::UGrThI32);
    case PreTypeSignCode::GrEq:  return signedness_switch(si, I32::SGrEqI32, I32::UGrEqI32);
    case PreTypeSignCode::Min:   return signedness_switch(si, I32::SMin,     I32::UMin);
    case PreTypeSignCode::Max:   return signedness_switch(si, I32::SMax,     I32::UMax);
  }
  return unreachable<Expr>();
}

inline Expr
specialize(PreTypeSignCode pre, Type type)
{
  assert(type == Type::F32 || type == Type::F64);
  switch (pre) {
    case PreTypeSignCode::ToF32: return type == Type::F32 ? Expr::Bad() : F32::FromF64;
    case PreTypeSignCode::Div:   return type_switch(type, I32::Bad, F32::Div, F64::Div);
    case PreTypeSignCode::LeTh:  return type_switch(type, I32::Bad, I32::LeThF32, I32::LeThF64);
    case PreTypeSignCode::LeEq:  return type_switch(type, I32::Bad, I32::LeEqF32, I32::LeEqF64);
    case PreTypeSignCode::GrTh:  return type_switch(type, I32::Bad, I32::GrThF32, I32::GrThF64);
    case PreTypeSignCode::GrEq:  return type_switch(type, I32::Bad, I32::GrEqF32, I32::GrEqF64);
    case PreTypeSignCode::Mod:   assert(type == Type::F64); return F64::Mod;
    case PreTypeSignCode::Min:   assert(type == Type::F64); return F64::Min;
    case PreTypeSignCode::Max:   assert(type == Type::F64); return F64::Max;
  }
  return unreachable<Expr>();
}

class StdLibFunc
{
public:
  enum Enum { Mono, Poly, SignedPoly };

private:
  Enum which_;
  union {
    Expr mono_;
    PreTypeCode poly_;
    PreTypeSignCode signed_poly_;
  } u;

public:
  StdLibFunc(Expr e) : which_(Mono), u{} { u.mono_ = e; }
  StdLibFunc(PreTypeCode p) : which_(Poly), u{} { u.poly_ = p; }
  StdLibFunc(PreTypeSignCode p) : which_(SignedPoly), u{} { u.signed_poly_ = p; }

  Enum which() const { return which_; }
  Expr mono() const { assert(which_ == Mono); return u.mono_; }
  PreTypeCode poly() const { assert(which_ == Poly); return u.poly_; }
  PreTypeSignCode signed_poly() const { assert(which_ == SignedPoly); return u.signed_poly_; }
};

struct HeapView
{
  HeapView(Type type, unsigned shift) : type(type), shift(shift), si(Signedness(-1))  {}
  HeapView(unsigned shift, Signedness si) : type(Type::I32), shift(shift), si(si)  {}

  Type type;
  unsigned shift;
  Signedness si;
};

struct FuncImportSignature
{
  FuncImportSignature(uint32_t sig_index) : sig_index(sig_index) {}

  uint32_t sig_index;
  uint32_t func_imp_sig_index;
};

struct FuncImport
{
  FuncImport(IString external) : external(external) {}

  IString external;
  vector<FuncImportSignature> sigs;
};

struct FuncPtrTable
{
  uint32_t sig_index;
  vector<uint32_t> elems;

  FuncPtrTable(uint32_t sig_index, vector<uint32_t>&& elems)
  : sig_index(sig_index)
  , elems(move(elems))
  {}
};

struct Global
{
  Global(Type type, uint32_t index) : type(type), index(index) {}

  Type type;
  uint32_t index;
};

struct Export
{
  Export(IString external, uint32_t internal) : external(external), internal(internal) {}

  IString external;
  uint32_t internal;
};

class Module
{
  Out out_;

  IString stdlib_;
  IString foreign_;
  IString buffer_;
  IString fround_;

  vector<Signature> sigs_;
  unordered_map<Signature, uint32_t, Signature::Hash> sig_to_index_;

  unordered_map<IString, uint32_t> func_to_index_;
  vector<Function> funcs_;

  unordered_map<IString, uint32_t> func_import_to_index_;
  vector<FuncImport> func_imps_;
  uint32_t num_func_imp_sigs_;

  unordered_map<IString, uint32_t> func_ptr_table_to_index_;
  vector<FuncPtrTable> func_ptr_tables_;

  typedef unordered_map<uint32_t, uint32_t> I32LitMap;
  typedef unordered_map<uint32_t, uint32_t> F32LitMap;
  typedef unordered_map<uint64_t, uint32_t> F64LitMap;
  I32LitMap i32_lit_map_;
  F32LitMap f32_lit_map_;
  F64LitMap f64_lit_map_;
  typedef pair<uint32_t, uint32_t> I32Lit;
  typedef pair<uint32_t, uint32_t> F32Lit;
  typedef pair<uint64_t, uint32_t> F64Lit;
  vector<I32Lit> i32s_;
  vector<F32Lit> f32s_;
  vector<F64Lit> f64s_;

  unordered_map<IString, Global> globals_;
  uint32_t num_global_i32_zero_;
  uint32_t num_global_f32_zero_;
  uint32_t num_global_f64_zero_;
  vector<IString> global_i32_imports_;
  vector<IString> global_f32_imports_;
  vector<IString> global_f64_imports_;

  unordered_map<IString, double> stdlib_doubles_;
  unordered_map<IString, StdLibFunc> stdlib_funcs_;
  unordered_map<IString, HeapView> heap_views_;
  vector<Export> exports_;

  bool finished_analysis_;

  void assert_unique_global_name(IString i)
  {
    assert(func_to_index_.find(i) == func_to_index_.end());
    assert(func_import_to_index_.find(i) == func_import_to_index_.end());
    assert(func_ptr_table_to_index_.find(i) == func_ptr_table_to_index_.end());
    assert(stdlib_doubles_.find(i) == stdlib_doubles_.end());
    assert(stdlib_funcs_.find(i) == stdlib_funcs_.end());
    assert(heap_views_.find(i) == heap_views_.end());
  }

  template <class SigT>
  uint32_t get_sig_index(SigT&& sig)
  {
    assert(!finished_analysis_);
    auto iter = sig_to_index_.find(sig);
    if (iter != sig_to_index_.end())
      return iter->second;
    uint32_t sig_index = sigs_.size();
    sigs_.push_back(sig);
    sig_to_index_.emplace(forward<SigT>(sig), sig_index);
    return sig_index;
  }

public:
  Module(ostream& os)
  : out_(os)
  , finished_analysis_(false)
  {}

  // ===============================================================================================
  // First-pass functions

  void stdlib(IString name) { stdlib_ = name; }
  void foreign(IString name) { foreign_ = name; }
  void buffer(IString name) { buffer_ = name; }
  IString stdlib() const { return stdlib_; }
  IString foreign() const { return foreign_; }
  IString buffer() const { return buffer_; }
  IString fround() const { return fround_; }

  void add_heap_view(IString name, HeapView v)
  {
    assert_unique_global_name(name);
    heap_views_.emplace(name, v);
  }

  void set_globals(const vector<IString>& i32_zero,
                   const vector<IString>& f32_zero,
                   const vector<IString>& f64_zero,
                   const vector<pair<IString,IString>>& i32_import,
                   const vector<pair<IString,IString>>& f32_import,
                   const vector<pair<IString,IString>>& f64_import)
  {
    num_global_i32_zero_ = i32_zero.size();
    num_global_f32_zero_ = f32_zero.size();
    num_global_f64_zero_ = f64_zero.size();
    for (auto name : i32_zero)
      globals_.emplace(name, Global(Type::I32, globals_.size()));
    for (auto name : f32_zero)
      globals_.emplace(name, Global(Type::F32, globals_.size()));
    for (auto name : f64_zero)
      globals_.emplace(name, Global(Type::F64, globals_.size()));
    for (auto pair : i32_import) {
      globals_.emplace(pair.first, Global(Type::I32, globals_.size()));
      global_i32_imports_.push_back(pair.second);
    }
    for (auto pair : f32_import) {
      globals_.emplace(pair.first, Global(Type::F32, globals_.size()));
      global_f32_imports_.push_back(pair.second);
    }
    for (auto pair : f64_import) {
      globals_.emplace(pair.first, Global(Type::F64, globals_.size()));
      global_f64_imports_.push_back(pair.second);
    }
  }

  Function &add_func(IString name)
  {
    assert(!finished_analysis_);
    assert_unique_global_name(name);
    uint32_t func_index = funcs_.size();
    func_to_index_.emplace(name, func_index);
    funcs_.emplace_back();
    return funcs_.back();
  }

  void finish_func(Function& f)
  {
    f.set_sig_index(get_sig_index(f.sig()));
  }

  void add_func_import(IString internal, IString external)
  {
    assert(!finished_analysis_);
    assert_unique_global_name(internal);
    func_import_to_index_.emplace(internal, func_imps_.size());
    func_imps_.emplace_back(external);
  };

  uint32_t add_import_sig(IString internal, Signature&& sig)
  {
    assert(!finished_analysis_);
    uint32_t sig_index = get_sig_index(move(sig));
    FuncImport& func_imp = func_imps_[func_import_to_index_.find(internal)->second];
    for (uint32_t pre_index = 0; pre_index < func_imp.sigs.size(); pre_index++)
      if (sig_index == func_imp.sigs[pre_index].sig_index)
        return pre_index;
    uint32_t pre_index = func_imp.sigs.size();
    func_imp.sigs.emplace_back(sig_index);
    return pre_index;
  }

  bool is_import_func(IString name)
  {
    return func_import_to_index_.find(name) != func_import_to_index_.end();
  }

  void add_stdlib_double(IString name, double d)
  {
    assert(!finished_analysis_);
    assert_unique_global_name(name);
    stdlib_doubles_.emplace(name, d);
  }

  bool is_stdlib_double(IString name) { return stdlib_doubles_.find(name) != stdlib_doubles_.end(); }
  double stdlib_double(IString name) { return stdlib_doubles_.find(name)->second; }

  template <class CodeT>
  void add_stdlib_func(IString name, CodeT code)
  {
    assert(!finished_analysis_);
    assert_unique_global_name(name);
    stdlib_funcs_.emplace(name, StdLibFunc(code));
  }

  void add_fround(IString name)
  {
    add_stdlib_func(name, PreTypeSignCode::ToF32);
    fround_ = name;
  }

  bool is_stdlib_func(IString name) { return stdlib_funcs_.find(name) != stdlib_funcs_.end(); }
  const StdLibFunc& stdlib_func(IString name) { return stdlib_funcs_.find(name)->second; }

  void add_func_ptr_table(IString name, vector<uint32_t>&& elems)
  {
    assert(!finished_analysis_);
    assert_unique_global_name(name);
    auto& sig = funcs_[elems[0]].sig();
    for (uint32_t func_index : elems)
      assert(sig == funcs_[func_index].sig());
    uint32_t sig_index = get_sig_index(sig);
    func_ptr_table_to_index_.emplace(name, func_ptr_tables_.size());
    func_ptr_tables_.emplace_back(sig_index, move(elems));
  }

  void add_default_export(IString internal_name)
  {
    assert(!finished_analysis_);
    assert(exports_.empty());
    auto result = func_to_index_.find(internal_name);
    exports_.push_back(Export(IString(), result->second));
  }

  void add_record_export(IString external, IString internal_name)
  {
    assert(!finished_analysis_);
    auto result = func_to_index_.find(internal_name);
    exports_.push_back(Export(external, result->second));
  }

  void add_lit(NumLit lit, unsigned lshift)
  {
    switch (lit.type()) {
      case Type::I32: i32_lit_map_[lit.uint32() << lshift]++; break;
      case Type::F32: assert(lshift == 0); f32_lit_map_[union_cast<uint32_t>(lit.float32())]++; break;
      case Type::F64: assert(lshift == 0); f64_lit_map_[union_cast<uint64_t>(lit.float64())]++; break;
    }
  }

  unsigned imm_len(uint32_t u32)
  {
    if (u32 < ImmLimit)
      return 0;
    if (u32 < 0x80)
      return 1;
    if (u32 < 0x4000)
      return 2;
    if (u32 < 0x200000)
      return 3;
    if (u32 < 0x10000000)
      return 4;
    return 5;
  }

  void process_i32_lit_map(I32LitMap& map, vector<I32Lit>& i32s)
  {
    i32s.assign(map.begin(), map.end());
    sort(i32s.begin(), i32s.end(), [](const I32Lit& lhs, const I32Lit& rhs){ return lhs.second > rhs.second; });
    for (uint32_t pool_index = 0; pool_index < i32s.size();) {
      uint32_t constant = i32s[pool_index].first;
      uint32_t uses = i32s[pool_index].second;
      int savings = imm_len(constant) - imm_len(pool_index);
      if (savings <= 0 || imm_len(constant) >= unsigned(savings) * uses) {
        map.erase(constant);
        i32s.erase(i32s.begin() + pool_index);
      } else {
        pool_index++;
      }
    }
    for (size_t i = 0; i < i32s.size(); i++)
      map[i32s[i].first] = i;
  }

  template <class MapT, class LitT>
  void process_f_lit_map(MapT& map, vector<LitT>& vec)
  {
    vec.assign(map.begin(), map.end());
    sort(vec.begin(), vec.end(), [](const LitT& lhs, const LitT& rhs){ return lhs.second > rhs.second; });
    size_t new_size = vec.size();
    for (; new_size > 0 && vec[new_size - 1].second <= 3; new_size--)
      map.erase(vec[new_size - 1].first);
    vec.resize(new_size);
    for (size_t i = 0; i < vec.size(); i++)
      map[vec[i].first] = i;
  }

  void finish_analysis()
  {
    assert(!finished_analysis_);
    finished_analysis_ = true;

    num_func_imp_sigs_ = 0;
    for (auto& func_imp : func_imps_)
      for (auto& func_imp_sig : func_imp.sigs)
        func_imp_sig.func_imp_sig_index = num_func_imp_sigs_++;

    // Before finish_analysis, the *_lit_maps map literal to use count.
    // After finish_analysis, the *_lit_maps map literal to index in i32s_ (now sorted hottest first).
    process_i32_lit_map(i32_lit_map_, i32s_);
    process_f_lit_map(f32_lit_map_, f32s_);
    process_f_lit_map(f64_lit_map_, f64s_);
  }

  // ===============================================================================================
  // Second-pass functions

  Out& write() { assert(finished_analysis_); return out_; }

  Global global(IString name) const { return globals_.find(name)->second; }
  uint32_t num_global_i32_zero() const { return num_global_i32_zero_; }
  uint32_t num_global_f64_zero() const { return num_global_f64_zero_; }
  uint32_t num_global_f32_zero() const { return num_global_f32_zero_; }
  const vector<IString>& global_i32_imports() const { return global_i32_imports_; }
  const vector<IString>& global_f32_imports() const { return global_f32_imports_; }
  const vector<IString>& global_f64_imports() const { return global_f64_imports_; }

  const vector<Signature>& sigs() { assert(finished_analysis_); return sigs_; }
  const Signature& sig(size_t i) { assert(finished_analysis_); return sigs_[i]; }

  vector<Function>& funcs() { assert(finished_analysis_); return funcs_; }
  uint32_t func_index(IString name) const { return func_to_index_.find(name)->second; }
  Function& func(uint32_t func_index) { return funcs_[func_index]; }

  vector<FuncImport>& func_imps() { assert(finished_analysis_); return func_imps_; }
  FuncImport& func_imp(IString name) { assert(finished_analysis_); return func_imps_[func_import_to_index_.find(name)->second]; }
  uint32_t num_func_imp_sigs() const { assert(finished_analysis_); return num_func_imp_sigs_; }

  const vector<FuncPtrTable>& func_ptr_tables() const { return func_ptr_tables_; }
  uint32_t func_ptr_table_index(IString name) const { return func_ptr_table_to_index_.find(name)->second; }
  const FuncPtrTable& func_ptr_table(size_t i) const { return func_ptr_tables_[i]; }

  HeapView heap_view(IString name) const { return heap_views_.find(name)->second; }
  const vector<Export>& exports() const { assert(finished_analysis_); return exports_; }

  bool lit_has_pool_index(NumLit lit, unsigned lshift, uint32_t *index) const
  {
    assert(finished_analysis_);
    switch (lit.type()) {
      case Type::I32: {
        auto iter = i32_lit_map_.find(lit.uint32() << lshift);
        if (iter == i32_lit_map_.end())
          return false;
        *index = iter->second;
        return true;
      }
      case Type::F32: {
        assert(lshift == 0);
        auto iter = f32_lit_map_.find(union_cast<uint32_t>(lit.float32()));
        if (iter == f32_lit_map_.end())
          return false;
        *index = iter->second;
        return true;
      }
      case Type::F64: {
        assert(lshift == 0);
        auto iter = f64_lit_map_.find(union_cast<uint64_t>(lit.float64()));
        if (iter == f64_lit_map_.end())
          return false;
        *index = iter->second;
        return true;
      }
    }
    return unreachable<bool>();
  }
  const vector<I32Lit>& i32s() const { assert(finished_analysis_); return i32s_; }
  const vector<F32Lit>& f32s() const { assert(finished_analysis_); return f32s_; }
  const vector<F64Lit>& f64s() const { assert(finished_analysis_); return f64s_; }
};

// =================================================================================================
// Numeric literals

/* static */ bool
NumLit::is(Module& m, const AstNode& n)
{
  if (n.is<CallNode>()) {
    const CallNode& call = n.as<CallNode>();
    return call.callee.is<NameNode>() &&
           call.callee.as<NameNode>().str == m.fround() &&
           call.compute_length() == 1 &&
           is_non_float_lit(*call.first);
  }
  return is_non_float_lit(n);
}

NumLit::NumLit(Module& m, const AstNode& n)
{
  assert(is(m, n));

  if (n.is<IntNode>()) {
    u.u32_ = n.as<IntNode>().u32;
    asmjs_type_ = u.u32_ > INT32_MAX ? AsmJSType::Unsigned : AsmJSType::Fixnum;
  } else if (n.is<DoubleNode>()) {
    u.f64_ = n.as<DoubleNode>().f64;
    asmjs_type_ = AsmJSType::Double;
  } else if (n.is<PrefixNode>()) {
    const AstNode& op = n.as<PrefixNode>().kid;
    if (op.is<IntNode>()) {
      u.u32_ = -op.as<IntNode>().u32;
      if (u.u32_ == 0) {
        u.f64_ = -0;
        asmjs_type_ = AsmJSType::Double;
      } else {
        assert(u.u32_ > INT32_MAX);
        asmjs_type_ = AsmJSType::Signed;
      }
    } else {
      u.f64_ = -op.as<DoubleNode>().f64;
      asmjs_type_ = AsmJSType::Double;
    }
  } else {
    const AstNode& arg = *n.as<CallNode>().first;
    if (arg.is<IntNode>())
      u.f32_ = float(double(arg.as<IntNode>().u32));
    else if (arg.is<DoubleNode>())
      u.f32_ = float(arg.as<DoubleNode>().f64);
    else if (arg.as<PrefixNode>().kid.is<IntNode>())
      u.f32_ = -float(double(arg.as<PrefixNode>().kid.as<IntNode>().u32));
    else
      u.f32_ = -float(arg.as<PrefixNode>().kid.as<DoubleNode>().f64);
    asmjs_type_ = AsmJSType::Float;
  }
}

// =================================================================================================
// Analysis (first) pass

void
analyze_heap_ctor(Module& m, const VarNameNode& var)
{
  const CallNode& call = var.init.as<NewNode>().call;
  assert(call.first == call.last);
  assert(call.first->as<NameNode>().str == m.buffer());

  DotNode& dot = call.callee.as<DotNode>();
  assert(dot.base.as<NameNode>().str == m.stdlib());

  if (dot.name.equals("Int8Array"))
    m.add_heap_view(var.name, HeapView(0, Signed));
  else if (dot.name.equals("Uint8Array"))
    m.add_heap_view(var.name, HeapView(0, Unsigned));
  else if (dot.name.equals("Int16Array"))
    m.add_heap_view(var.name, HeapView(1, Signed));
  else if (dot.name.equals("Uint16Array"))
    m.add_heap_view(var.name, HeapView(1, Unsigned));
  else if (dot.name.equals("Int32Array"))
    m.add_heap_view(var.name, HeapView(2, Signed));
  else if (dot.name.equals("Uint32Array"))
    m.add_heap_view(var.name, HeapView(2, Unsigned));
  else if (dot.name.equals("Float32Array"))
    m.add_heap_view(var.name, HeapView(Type::F32, 2));
  else if (dot.name.equals("Float64Array"))
    m.add_heap_view(var.name, HeapView(Type::F64, 3));
  else
    abort();
}

void
analyze_import(Module& m, IString name, DotNode& dot)
{
  if (dot.base.is<DotNode>()) {
    assert(dot.base.as<DotNode>().base.as<NameNode>().str == m.stdlib());
    assert(dot.base.as<DotNode>().name.equals("Math"));
    if (dot.name.equals("imul"))
      m.add_stdlib_func(name, I32::Mul);
    else if (dot.name.equals("clz32"))
      m.add_stdlib_func(name, I32::Clz);
    else if (dot.name.equals("fround"))
      m.add_fround(name);
    else if (dot.name.equals("min"))
      m.add_stdlib_func(name, PreTypeSignCode::Min);
    else if (dot.name.equals("max"))
      m.add_stdlib_func(name, PreTypeSignCode::Max);
    else if (dot.name.equals("abs"))
      m.add_stdlib_func(name, PreTypeCode::Abs);
    else if (dot.name.equals("ceil"))
      m.add_stdlib_func(name, PreTypeCode::Ceil);
    else if (dot.name.equals("floor"))
      m.add_stdlib_func(name, PreTypeCode::Floor);
    else if (dot.name.equals("sqrt"))
      m.add_stdlib_func(name, PreTypeCode::Sqrt);
    else if (dot.name.equals("cos"))
      m.add_stdlib_func(name, F64::Cos);
    else if (dot.name.equals("sin"))
      m.add_stdlib_func(name, F64::Sin);
    else if (dot.name.equals("tan"))
      m.add_stdlib_func(name, F64::Tan);
    else if (dot.name.equals("acos"))
      m.add_stdlib_func(name, F64::ACos);
    else if (dot.name.equals("asin"))
      m.add_stdlib_func(name, F64::ASin);
    else if (dot.name.equals("atan"))
      m.add_stdlib_func(name, F64::ATan);
    else if (dot.name.equals("atan2"))
      m.add_stdlib_func(name, F64::ATan2);
    else if (dot.name.equals("exp"))
      m.add_stdlib_func(name, F64::Exp);
    else if (dot.name.equals("log"))
      m.add_stdlib_func(name, F64::Ln);
    else if (dot.name.equals("pow"))
      m.add_stdlib_func(name, F64::Pow);
    else
      unreachable<void>();
  } else if (dot.base.as<NameNode>().str == m.stdlib()) {
    if (dot.name.equals("NaN"))
      m.add_stdlib_double(name, NAN);
    else if (dot.name.equals("Infinity"))
      m.add_stdlib_double(name, INFINITY);
    else
      unreachable<void>();
  } else if (dot.base.as<NameNode>().str == m.foreign()) {
    m.add_func_import(name, dot.name);
  } else {
    unreachable<void>();
  }
}

AstNode*
analyze_global_definitions(Module& m, AstNode* stmt)
{
  vector<IString> i32_zero;
  vector<IString> f32_zero;
  vector<IString> f64_zero;
  vector<pair<IString,IString>> i32_import;
  vector<pair<IString,IString>> f32_import;
  vector<pair<IString,IString>> f64_import;

  for (; stmt && stmt->is<VarNode>(); stmt = stmt->next) {
    for (VarNameNode* var = stmt->as<VarNode>().first; var; var = var->next) {
      if (NumLit::is(m, var->init)) {
        NumLit lit(m, var->init);
        switch (lit.type()) {
          case Type::I32:
            assert(lit.uint32() == 0);
            i32_zero.push_back(var->name);
            break;
          case Type::F32:
            assert(lit.float32() == 0);
            f32_zero.push_back(var->name);
            break;
          case Type::F64:
            assert(lit.float64() == 0);
            f64_zero.push_back(var->name);
            break;
        }
      } else {
        switch (var->init.which) {
          case AstNode::New:
            analyze_heap_ctor(m, *var);
            break;
          case AstNode::Dot:
            analyze_import(m, var->name, var->init.as<DotNode>());
            break;
          case AstNode::Prefix: {
            auto& dot = var->init.as<PrefixNode>().kid.as<DotNode>();
            assert(dot.base.as<NameNode>().str == m.foreign());
            f64_import.emplace_back(var->name, dot.name);
            break;
          }
          case AstNode::Binary: {
            auto& binary = var->init.as<BinaryNode>();
            assert(binary.op.equals("|"));
            assert(binary.rhs.as<IntNode>().u32 == 0);
            auto& dot = binary.lhs.as<DotNode>();
            assert(dot.base.as<NameNode>().str == m.foreign());
            i32_import.emplace_back(var->name, dot.name);
            break;
          }
          case AstNode::Call: {
            auto& call = var->init.as<CallNode>();
            assert(call.callee.as<NameNode>().str == m.fround());
            assert(call.compute_length() == 1);
            auto& dot = call.first->as<DotNode>();
            assert(dot.base.as<NameNode>().str == m.foreign());
            f32_import.emplace_back(var->name, dot.name);
            break;
          }
          default:
            unreachable<void>();
        }
      }
    }
  }

  m.set_globals(i32_zero, f32_zero, f64_zero, i32_import, f32_import, f64_import);
  return stmt;
}

Type
extract_arg_type(Module& m, const ArgNode& arg, const AstNode& stmt)
{
  const BinaryNode& assign = stmt.as<BinaryNode>();
  assert(assign.op.equals("="));
  assert(assign.lhs.as<NameNode>().str == arg.name);

  AstNode& coercion = assign.rhs;

  if (coercion.is<BinaryNode>()) {
    assert(coercion.as<BinaryNode>().op.equals("|"));
    assert(coercion.as<BinaryNode>().rhs.as<IntNode>().u32 == 0);
    assert(coercion.as<BinaryNode>().lhs.as<NameNode>().str == arg.name);
    return Type::I32;
  }

  if (coercion.is<CallNode>()) {
    assert(coercion.as<CallNode>().callee.as<NameNode>().str == m.fround());
    assert(coercion.as<CallNode>().compute_length() == 1);
    assert(coercion.as<CallNode>().first->as<NameNode>().str == arg.name);
    return Type::F32;
  }

  assert(coercion.as<PrefixNode>().op.equals("+"));
  assert(coercion.as<PrefixNode>().kid.as<NameNode>().str == arg.name);
  return Type::F64;
}

Type
extract_var_init(Module& m, const AstNode& init)
{
  if (init.is<NameNode>())
    return m.global(init.as<NameNode>().str).type;

  NumLit lit(m, init);

  assert((lit.type() == Type::I32 && lit.uint32() == 0) ||
         (lit.type() == Type::F32 && lit.float32() == 0) ||
         (lit.type() == Type::F64 && lit.float64() == 0));

  return lit.type();
}

AsmJSType analyze_expr(Module&, Function&, AstNode&);
RType analyze_call(Module& m, Function& f, CallNode& call, RType ret_type, AstNode* coercion = nullptr);

AsmJSType
analyze_num_lit(Module& m, NumLit lit, unsigned lshift = 0)
{
  m.add_lit(lit, lshift);
  return lit.asmjs_type();
}

AsmJSType
analyze_stdlib_call(Module& m, Function& f, CallNode& call)
{
  IString callee = call.callee.as<NameNode>().str;

  assert(call.expr.is_bad());

  if (callee == m.fround() && call.compute_length() == 1 && call.first->is<CallNode>()) {
    call.kind = CallNode::Fround;
    analyze_call(m, f, call.first->as<CallNode>(), RType::F32, &call);
    return AsmJSType::Float;
  }

  vector<AsmJSType> args;
  for (AstNode* arg = call.first; arg; arg = arg->next)
    args.push_back(analyze_expr(m, f, *arg));

  StdLibFunc stdlib = m.stdlib_func(callee);
  switch (stdlib.which()) {
    case StdLibFunc::Mono:
      call.kind = CallNode::FixedArityBuiltin;
      call.expr = stdlib.mono();
      switch (stdlib.mono().type()) {
        case RType::I32:
          switch (stdlib.mono().i32()) {
            case I32::Mul: return AsmJSType::Signed;
            case I32::Clz: return AsmJSType::Fixnum;
            default: unreachable<void>();
          }
        case RType::F64:
          return AsmJSType::Double;
        default:
          unreachable<void>();
      }
    case StdLibFunc::Poly:
      switch (stdlib.poly()) {
        case PreTypeCode::Ceil:
        case PreTypeCode::Floor:
        case PreTypeCode::Sqrt:
          assert(args.size() == 1);
          assert(args[0].is_double() || args[0].is_float());
          call.kind = CallNode::FixedArityBuiltin;
          call.expr = specialize(stdlib.poly(), args[0].type());
          return args[0].is_float() ? AsmJSType::Float : AsmJSType::Double;
        case PreTypeCode::Abs:
          assert(args.size() == 1);
          assert(args[0].is_signed() || args[0].is_float() || args[0].is_double());
          call.kind = CallNode::FixedArityBuiltin;
          call.expr = specialize(stdlib.poly(), args[0].type());
          return type_switch(args[0].type(), AsmJSType::Unsigned, AsmJSType::Float, AsmJSType::Double);
        default:
          unreachable<void>();
      }
    case StdLibFunc::SignedPoly:
      switch (stdlib.signed_poly()) {
        case PreTypeSignCode::Min:
        case PreTypeSignCode::Max:
          assert(args.size() >= 1);
          call.kind = CallNode::NaryBuiltin;
          for (AsmJSType t : args)
            assert(t.type() == args[0].type() && (t.is_signed() || t.is_double()));
          if (args[0].is_double()) {
            call.expr = specialize(stdlib.signed_poly(), Type::F64);
            return AsmJSType::Double;
          } else {
            call.expr = specialize(stdlib.signed_poly(), Signed);
            return AsmJSType::Signed;
          }
        case PreTypeSignCode::ToF32:
          assert(args.size() == 1);
          call.kind = CallNode::Fround;
          if (args[0].is_signed())
            call.expr = specialize(PreTypeSignCode::ToF32, Signed);
          else if (args[0].is_unsigned())
            call.expr = specialize(PreTypeSignCode::ToF32, Unsigned);
          else
            call.expr = specialize(PreTypeSignCode::ToF32, args[0].type());
          return AsmJSType::Float;
        default:
          unreachable<void>();
      }
  }

  return unreachable<AsmJSType>();
}

void
set_coercion_node_expr(AstNode* coercion, Expr expr)
{
  if (coercion->is<PrefixNode>())
    coercion->as<PrefixNode>().expr = expr;
  else
    coercion->as<CallNode>().expr = expr;
}

RType
analyze_coerced_stdlib_call(Module& m, Function& f, CallNode& call, RType ret_type, AstNode* coercion)
{
  AsmJSType t;
  if (NumLit::is(m, call))
    t = analyze_num_lit(m, NumLit(m, call));
  else
    t = analyze_stdlib_call(m, f, call);

  switch (ret_type) {
    case RType::I32:
      assert(t.is_int());
      break;
    case RType::F32:
      if (t.is_signed())
        set_coercion_node_expr(coercion, F32::FromS32);
      else if (t.is_unsigned())
        set_coercion_node_expr(coercion, F32::FromU32);
      else if (t.is_double())
        set_coercion_node_expr(coercion, F32::FromF64);
      else
        assert(t.is_float());
      break;
    case RType::F64:
      if (t.is_signed())
        set_coercion_node_expr(coercion, F64::FromS32);
      else if (t.is_unsigned())
        set_coercion_node_expr(coercion, F64::FromU32);
      else if (t.is_float())
        set_coercion_node_expr(coercion, F64::FromF32);
      else
        assert(t.is_double());
      break;
    case RType::Void:
      ret_type = t.rtype();
  }

  return ret_type;
}

void
analyze_call_import(Module& m, Function& f, CallNode& call, RType ret_type)
{
  call.kind = CallNode::Import;
  call.expr = type_switch(ret_type, I32::CallImp, F32::Bad, F64::CallImp, Void::CallImp);
  call.stmt = Stmt::CallImp;

  Signature sig(ret_type);
  for (AstNode* arg = call.first; arg; arg = arg->next)
    sig.args.push_back(analyze_expr(m, f, *arg).type());

  call.import_preindex = m.add_import_sig(call.callee.as<NameNode>().str, move(sig));
}

void
analyze_call_internal(Module& m, Function& f, CallNode& call, RType ret_type)
{
  assert(call.callee.is<NameNode>());

  call.kind = CallNode::Internal;
  call.expr = type_switch(ret_type, I32::CallInt, F32::CallInt, F64::CallInt, Void::CallInt);
  call.stmt = Stmt::CallInt;

  for (AstNode* arg = call.first; arg; arg = arg->next)
    analyze_expr(m, f, *arg);
}

void
analyze_call_indirect(Module& m, Function& f, CallNode& call, RType ret_type)
{
  call.kind = CallNode::Indirect;
  call.expr = type_switch(ret_type, I32::CallInd, F32::CallInd, F64::CallInd, Void::CallInd);
  call.stmt = Stmt::CallInd;

  assert(call.callee.is<IndexNode>());
  IndexNode& index = call.callee.as<IndexNode>();
  assert(index.array.is<NameNode>());
  assert(index.index->is<BinaryNode>());
  BinaryNode& mask = index.index->as<BinaryNode>();
  assert(mask.op.equals("&"));
  assert(mask.rhs.is<IntNode>());
  analyze_expr(m, f, mask.lhs);
  for (AstNode* arg = call.first; arg; arg = arg->next)
    analyze_expr(m, f, *arg);
}

RType
analyze_call(Module& m, Function& f, CallNode& call, RType ret_type, AstNode* coercion)
{
  if (call.callee.which == AstNode::Name) {
    IString name = call.callee.as<NameNode>().str;
    if (m.is_stdlib_func(name))
      ret_type = analyze_coerced_stdlib_call(m, f, call, ret_type, coercion);
    else if (m.is_import_func(name))
      analyze_call_import(m, f, call, ret_type);
    else
      analyze_call_internal(m, f, call, ret_type);
  } else if (call.callee.which == AstNode::Index) {
    analyze_call_indirect(m, f, call, ret_type);
  } else {
    unreachable<void>();
  }
  return ret_type;
}

bool
is_double_coerced_call(const PrefixNode& prefix)
{
  return prefix.op.equals("+") &&
         prefix.kid.is<CallNode>();
}

AsmJSType
analyze_to_number(Module& m, Function& f, PrefixNode& prefix)
{
  assert(prefix.expr.is_bad());

  if (is_double_coerced_call(prefix)) {
    analyze_call(m, f, prefix.kid.as<CallNode>(), RType::F64, &prefix);
    return AsmJSType::Double;
  }

  AsmJSType t = analyze_expr(m, f, prefix.kid);
  if (t.is_signed())
    prefix.expr = F64::FromS32;
  else if (t.is_unsigned())
    prefix.expr = F64::FromU32;
  else if (t.is_float())
    prefix.expr = F64::FromF32;
  else
    assert(t.is_double());
  return AsmJSType::Double;
}

AsmJSType
analyze_negate(Module& m, Function& f, PrefixNode& prefix)
{
  AsmJSType t = analyze_expr(m, f, prefix.kid);
  prefix.expr = specialize(PreTypeCode::Neg, t.type());
  return t;
}

AsmJSType
analyze_bitnot(Module& m, Function& f, PrefixNode& prefix)
{
  if (prefix.kid.is<PrefixNode>() && prefix.kid.as<PrefixNode>().op.equals("~")) {
    AsmJSType t = analyze_expr(m, f, prefix.kid.as<PrefixNode>().kid);
    assert(t.is_float() || t.is_double());
    if (t.is_float())
      prefix.expr = I32::FromF32;
    else
      prefix.expr = I32::FromF64;
    return AsmJSType::Signed;
  }

  AsmJSType t = analyze_expr(m, f, prefix.kid);
  assert(t.is_int());
  prefix.expr = I32::BitNot;
  return AsmJSType::Signed;
}

AsmJSType
analyze_not(Module& m, Function& f, PrefixNode& prefix)
{
  AsmJSType t = analyze_expr(m, f, prefix.kid);
  assert(t.is_int());
  prefix.expr = I32::LogicNot;
  return AsmJSType::Int;
}

AsmJSType
analyze_name(Module& m, Function& f, NameNode& name)
{
  if (f.is_local_name(name.str)) {
    name.index = f.local_index(name.str);
    switch (f.local_type(name.index)) {
      case Type::I32: name.expr = I32::GetLoc; name.expr_with_imm = I32WithImm::GetLoc; return AsmJSType::Int;
      case Type::F32: name.expr = F32::GetLoc; name.expr_with_imm = F32WithImm::GetLoc; return AsmJSType::Float;
      case Type::F64: name.expr = F64::GetLoc; name.expr_with_imm = F64WithImm::GetLoc; return AsmJSType::Double;
    }
    return unreachable<AsmJSType>();
  }

  if (m.is_stdlib_double(name.str))
    return analyze_num_lit(m, NumLit(m.stdlib_double(name.str)));

  Global g = m.global(name.str);
  name.index = g.index;
  switch (g.type) {
    case Type::I32: name.expr = I32::GetGlo; return AsmJSType::Int;
    case Type::F32: name.expr = F32::GetGlo; return AsmJSType::Float;
    case Type::F64: name.expr = F64::GetGlo; return AsmJSType::Double;
  }
  return unreachable<AsmJSType>();
}

AsmJSType
analyze_prefix(Module& m, Function& f, PrefixNode& prefix)
{
  if (prefix.op.equals("~"))
    return analyze_bitnot(m, f, prefix);

  assert(strlen(prefix.op.c_str()) == 1);
  switch (prefix.op.c_str()[0]) {
    case '+': return analyze_to_number(m, f, prefix);
    case '-': return analyze_negate(m, f, prefix);
    case '!': return analyze_not(m, f, prefix);
    default:;
  }
  return unreachable<AsmJSType>();
}

AsmJSType
analyze_comma(Module& m, Function& f, BinaryNode& binary)
{
  RType lhs;
  if (binary.lhs.is<CallNode>())
    lhs = analyze_call(m, f, binary.lhs.as<CallNode>(), RType::Void);
  else
    lhs = analyze_expr(m, f, binary.lhs).rtype();
  AsmJSType rhs = analyze_expr(m, f, binary.rhs);
  binary.expr = specialize(PreTypeCode::Comma, rhs.type());
  binary.kind = BinaryNode::Comma;
  binary.comma_lhs_type = lhs;
  return rhs;
}

HeapView
analyze_index(Module& m, Function& f, IndexNode& index)
{
  HeapView hv = m.heap_view(index.array.as<NameNode>().str);

  AsmJSType t;
  if (index.index->is<IntNode>()) {
    index.constant = true;
    t = analyze_num_lit(m, NumLit(m, *index.index), hv.shift);
  } else {
    if (index.index->is<BinaryNode>()) {
      BinaryNode& binary = index.index->as<BinaryNode>();
      if (binary.op.equals(">>") && binary.rhs.is<IntNode>() && binary.rhs.as<IntNode>().u32 == hv.shift)
        index.index = &binary.lhs;
      else
        assert(hv.shift == 0);
    }

    if (index.index->is<BinaryNode>()) {
      BinaryNode& binary = index.index->as<BinaryNode>();
      if (binary.op.equals("+")) {
        if (binary.rhs.is<IntNode>()) {
          index.offset = binary.rhs.as<IntNode>().u32;
          index.index = &binary.lhs;
        } else if (binary.lhs.is<IntNode>()) {
          index.offset = binary.lhs.as<IntNode>().u32;
          index.index = &binary.rhs;
        }
      }
    }

    t = analyze_expr(m, f, *index.index);
  }

  assert(t.is_int());
  return hv;
}

AsmJSType
analyze_load(Module& m, Function& f, IndexNode& index)
{
  HeapView hv = analyze_index(m, f, index);
  switch (hv.type) {
    case Type::I32:
      switch (hv.shift) {
        case 0:
          index.expr = index.offset ? signedness_switch(hv.si, I32::SLoadOff8, I32::ULoadOff8)
                                    : signedness_switch(hv.si, I32::SLoad8, I32::ULoad8);
          break;
        case 1:
          index.expr = index.offset ? signedness_switch(hv.si, I32::SLoadOff16, I32::ULoadOff16)
                                    : signedness_switch(hv.si, I32::SLoad16, I32::ULoad16);
          break;
        case 2:
          index.expr = index.offset ? I32::LoadOff32 : I32::Load32;
          break;
      }
      break;
    case Type::F32:
      index.expr = index.offset ? F32::LoadOff : F32::Load;
      break;
    case Type::F64:
      index.expr = index.offset ? F64::LoadOff : F64::Load;
      break;
  }
  return type_switch(hv.type, AsmJSType::Int, AsmJSType::Float, AsmJSType::Double);
}

AsmJSType
analyze_store(Module& m, Function& f, BinaryNode& binary)
{
  IndexNode& index = binary.lhs.as<IndexNode>();
  HeapView hv = analyze_index(m, f, index);
  AsmJSType rhs = analyze_expr(m, f, binary.rhs);
  switch (hv.type) {
    case Type::I32:
      assert(rhs.is_int());
      switch (hv.shift) {
        case 0:
          binary.expr = index.offset ? I32::StoreOff8 : I32::Store8;
          binary.stmt = index.offset ? Stmt::I32StoreOff8 : Stmt::I32Store8;
          break;
        case 1:
          binary.expr = index.offset ? I32::StoreOff16 : I32::Store16;
          binary.stmt = index.offset ? Stmt::I32StoreOff16 : Stmt::I32Store16;
          break;
        case 2:
          binary.expr = index.offset ? I32::StoreOff32 : I32::Store32;
          binary.stmt = index.offset ? Stmt::I32StoreOff32 : Stmt::I32Store32;
          break;
      }
      break;
    case Type::F32:
      assert(rhs.is_float() || rhs.is_double());
      binary.expr = index.offset ? F32::StoreOff : F32::Store;
      binary.stmt = index.offset ? Stmt::F32StoreOff : Stmt::F32Store;
      if (rhs.is_double()) {
        binary.store_rhs_conv = F32::FromF64;
        f.set_needs_f64_temp();
      }
      break;
    case Type::F64:
      assert(rhs.is_float() || rhs.is_double());
      binary.expr = index.offset ? F64::StoreOff : F64::Store;
      binary.stmt = index.offset ? Stmt::F64StoreOff : Stmt::F64Store;
      if (rhs.is_float()) {
        binary.store_rhs_conv = F64::FromF32;
        f.set_needs_f32_temp();
      }
      break;
  }
  binary.kind = BinaryNode::Store;
  return rhs;
}

AsmJSType
analyze_assign(Module& m, Function& f, BinaryNode& binary)
{
  if (binary.lhs.is<IndexNode>())
    return analyze_store(m, f, binary);

  binary.kind = BinaryNode::Assign;

  NameNode& lhs = binary.lhs.as<NameNode>();
  AsmJSType rhs = analyze_expr(m, f, binary.rhs);

  if (f.is_local_name(lhs.str)) {
    lhs.index = f.local_index(lhs.str);
    assert(rhs.type() == f.local_type(lhs.index));
    binary.expr = type_switch(rhs.type(), I32::SetLoc, F32::SetLoc, F64::SetLoc);
    binary.stmt = Stmt::SetLoc;
    binary.stmt_with_imm = StmtWithImm::SetLoc;
    return rhs;
  }

  Global g = m.global(lhs.str);
  lhs.index = g.index;
  assert(rhs.type() == g.type);
  binary.expr = type_switch(rhs.type(), I32::SetGlo, F32::SetGlo, F64::SetGlo);
  binary.stmt = Stmt::SetGlo;
  binary.stmt_with_imm = StmtWithImm::SetGlo;
  return rhs;
}

AsmJSType
analyze_arith(Module& m, Function& f, BinaryNode& binary, PreTypeCode pre)
{
  AsmJSType lhs = analyze_expr(m, f, binary.lhs);
  AsmJSType rhs = analyze_expr(m, f, binary.rhs);
  assert(lhs.type() == rhs.type());
  binary.expr = specialize(pre, lhs.type());
  return lhs.forget_signedness();
}

Expr
specialize(AsmJSType lhs, AsmJSType rhs, PreTypeSignCode pre)
{
  if (lhs.is_int()) {
    if (lhs.is_signed() && rhs.is_signed())
      return specialize(pre, Signed);
    return specialize(pre, Unsigned);
  }
  return specialize(pre, lhs.type());
}

AsmJSType
analyze_div_mod(Module& m, Function& f, BinaryNode& binary, PreTypeSignCode pre)
{
  AsmJSType lhs = analyze_expr(m, f, binary.lhs);
  AsmJSType rhs = analyze_expr(m, f, binary.rhs);
  assert(lhs.type() == rhs.type());
  binary.expr = specialize(lhs, rhs, pre);
  return lhs.forget_signedness();
}

AsmJSType
analyze_eq(Module& m, Function& f, BinaryNode& binary, PreTypeCode pre)
{
  AsmJSType lhs = analyze_expr(m, f, binary.lhs);
  AsmJSType rhs = analyze_expr(m, f, binary.rhs);
  assert(lhs.type() == rhs.type());
  binary.expr = specialize(pre, lhs.type());
  return AsmJSType::Int;
}

AsmJSType
analyze_comp(Module& m, Function& f, BinaryNode& binary, PreTypeSignCode pre)
{
  AsmJSType lhs = analyze_expr(m, f, binary.lhs);
  AsmJSType rhs = analyze_expr(m, f, binary.rhs);
  assert(lhs.type() == rhs.type());
  binary.expr = specialize(lhs, rhs, pre);
  return AsmJSType::Int;
}

bool
is_int_coerced_call(const BinaryNode& binary)
{
  return binary.op.equals("|") &&
         binary.lhs.is<CallNode>() &&
         binary.rhs.is<IntNode>() &&
         binary.rhs.as<IntNode>().u32 == 0;
}

AsmJSType
analyze_bitwise(Module& m, Function& f, BinaryNode& binary, I32 i32)
{
  binary.kind = BinaryNode::Bitwise;

  if (is_int_coerced_call(binary)) {
    analyze_call(m, f, binary.lhs.as<CallNode>(), RType::I32);
    return AsmJSType::Signed;
  }

  AsmJSType lhs = analyze_expr(m, f, binary.lhs);
  AsmJSType rhs = analyze_expr(m, f, binary.rhs);
  assert(lhs.type() == rhs.type());
  assert(lhs.is_int() && rhs.is_int());
  binary.expr = i32;
  return i32 == I32::LogicRsh ? AsmJSType::Unsigned : AsmJSType::Signed;
}

AsmJSType
analyze_binary(Module& m, Function& f, BinaryNode& binary)
{
  switch (binary.op.c_str()[0]) {
    case ',': return analyze_comma(m, f, binary);
    case '+': return analyze_arith(m, f, binary, PreTypeCode::Add);
    case '-': return analyze_arith(m, f, binary, PreTypeCode::Sub);
    case '*': return analyze_arith(m, f, binary, PreTypeCode::Mul);
    case '/': return analyze_div_mod(m, f, binary, PreTypeSignCode::Div);
    case '%': return analyze_div_mod(m, f, binary, PreTypeSignCode::Mod);
    case '|': return analyze_bitwise(m, f, binary, I32::BitOr);
    case '&': return analyze_bitwise(m, f, binary, I32::BitAnd);
    case '^': return analyze_bitwise(m, f, binary, I32::BitXor);
    case '=':
      switch (binary.op.c_str()[1]) {
        case '\0': return analyze_assign(m, f, binary);
        case '=': return analyze_eq(m, f, binary, PreTypeCode::Eq);
        default: unreachable<void>();
      }
    case '!':
      switch (binary.op.c_str()[1]) {
        case '\0': unreachable<void>();
        case '=': return analyze_eq(m, f, binary, PreTypeCode::NEq);
      }
    case '<':
      switch (binary.op.c_str()[1]) {
        case '\0': return analyze_comp(m, f, binary, PreTypeSignCode::LeTh);
        case '=': return analyze_comp(m, f, binary, PreTypeSignCode::LeEq);
        case '<': return analyze_bitwise(m, f, binary, I32::Lsh);
        default: unreachable<void>();
      }
    case '>':
      switch (binary.op.c_str()[1]) {
        case '\0': return analyze_comp(m, f, binary, PreTypeSignCode::GrTh);
        case '=': return analyze_comp(m, f, binary, PreTypeSignCode::GrEq);
        case '>':
          switch (binary.op.c_str()[2]) {
            case '\0': return analyze_bitwise(m, f, binary, I32::ArithRsh);
            case '>': return analyze_bitwise(m, f, binary, I32::LogicRsh);
            default: unreachable<void>();
          }
      }
    default: unreachable<void>();
  }
  return unreachable<AsmJSType>();
}

AsmJSType
analyze_ternary(Module& m, Function& f, TernaryNode& ternary)
{
  AsmJSType cond = analyze_expr(m, f, ternary.cond);
  assert(cond.is_int());
  AsmJSType lhs = analyze_expr(m, f, ternary.lhs);
  AsmJSType rhs = analyze_expr(m, f, ternary.rhs);
  assert(lhs.type() == rhs.type());
  if (lhs.is_int())
    ternary.expr = I32::Cond;
  else if (lhs.is_float())
    ternary.expr = F32::Cond;
  else if (lhs.is_double())
    ternary.expr = F64::Cond;
  else
    unreachable<void>();
  return lhs.forget_signedness();
}

AsmJSType
analyze_expr(Module& m, Function& f, AstNode& expr)
{
  if (NumLit::is(m, expr))
    return analyze_num_lit(m, NumLit(m, expr));

  switch (expr.which) {
    case AstNode::Name: return analyze_name(m, f, expr.as<NameNode>());
    case AstNode::Prefix: return analyze_prefix(m, f, expr.as<PrefixNode>());
    case AstNode::Binary: return analyze_binary(m, f, expr.as<BinaryNode>());
    case AstNode::Ternary: return analyze_ternary(m, f, expr.as<TernaryNode>());
    case AstNode::Index: return analyze_load(m, f, expr.as<IndexNode>());
    case AstNode::Call: return analyze_stdlib_call(m, f, expr.as<CallNode>());
    default: unreachable<void>();
  }
  return unreachable<AsmJSType>();
}

void
analyze_stmt(Module& m, Function& f, AstNode& stmt)
{
  switch (stmt.which) {
    case AstNode::Call:
      analyze_call(m, f, stmt.as<CallNode>(), RType::Void);
      break;
    case AstNode::Prefix:
      analyze_prefix(m, f, stmt.as<PrefixNode>());
      break;
    case AstNode::Binary:
      analyze_binary(m, f, stmt.as<BinaryNode>());
      break;
    case AstNode::Return:
      if (!stmt.as<ReturnNode>().expr)
        f.set_ret_type(RType::Void);
      else
        f.set_ret_type(to_rtype(analyze_expr(m, f, *stmt.as<ReturnNode>().expr).type()));
      break;
    case AstNode::Block:
      for (AstNode* p = stmt.as<BlockNode>().first; p; p = p->next)
        analyze_stmt(m, f, *p);
      break;
    case AstNode::If:
      analyze_expr(m, f, stmt.as<IfNode>().cond);
      analyze_stmt(m, f, stmt.as<IfNode>().if_true);
      if (stmt.as<IfNode>().if_false)
        analyze_stmt(m, f, *stmt.as<IfNode>().if_false);
      break;
    case AstNode::While:
      analyze_expr(m, f, stmt.as<WhileNode>().cond);
      analyze_stmt(m, f, stmt.as<WhileNode>().body);
      break;
    case AstNode::Do:
      analyze_stmt(m, f, stmt.as<DoNode>().body);
      analyze_expr(m, f, stmt.as<DoNode>().cond);
      break;
    case AstNode::Label:
      analyze_stmt(m, f, stmt.as<LabelNode>().stmt);
      break;
    case AstNode::Break:
    case AstNode::Continue:
      break;
    case AstNode::Switch:
      analyze_expr(m, f, stmt.as<SwitchNode>().expr);
      for (CaseNode* c = stmt.as<SwitchNode>().first; c; c = c->next)
        for (CaseStmtNode* s = c->first; s; s = s->next)
          analyze_stmt(m, f, s->stmt);
      break;
    default: unreachable<void>();
  }
}

void
analyze_function_definition(Module& m, const FuncNode& func)
{
  Function& f = m.add_func(func.name);

  AstNode* body = func.List<AstNode>::first;

  for (const ArgNode* arg = func.List<ArgNode>::first; arg; arg = arg->next, body = body->next)
    f.add_arg(arg->name, extract_arg_type(m, *arg, *body));

  vector<IString> i32s, f32s, f64s;
  for (; body && body->is<VarNode>(); body = body->next) {
    for (VarNameNode* var = body->as<VarNode>().first; var; var = var->next) {
      switch (extract_var_init(m, var->init)) {
        case Type::I32: i32s.push_back(var->name); break;
        case Type::F32: f32s.push_back(var->name); break;
        case Type::F64: f64s.push_back(var->name); break;
      }
    }
  }

  f.start_body(body, i32s, f32s, f64s);

  for (; body; body = body->next)
    analyze_stmt(m, f, *body);

  m.finish_func(f);
}

void
analyze_func_ptr_table(Module& m, const VarNode& var)
{
  assert(var.first == var.last);
  VarNameNode& name = *var.first;

  vector<uint32_t> funcs;
  for (AstNode* elem = name.init.as<ArrayNode>().first; elem; elem = elem->next)
    funcs.push_back(m.func_index(elem->as<NameNode>().str));

  m.add_func_ptr_table(name.name, move(funcs));
}

void
analyze_export_stmt(Module& m, const ReturnNode& export_stmt)
{
  if (export_stmt.expr->is<NameNode>()) {
    m.add_default_export(export_stmt.expr->as<NameNode>().str);
  } else {
    ObjectNode& export_obj = export_stmt.expr->as<ObjectNode>();
    for (FieldNode* cur_field = export_obj.first; cur_field; cur_field = cur_field->next)
      m.add_record_export(cur_field->key, cur_field->value.as<NameNode>().str);
  }
}

void
analyze_module(Module& m, const FuncNode& module)
{
  const ArgNode* arg1 = module.List<ArgNode>::first;
  m.stdlib(arg1->name);
  const ArgNode* arg2 = arg1->next;
  m.foreign(arg2->name);
  const ArgNode* arg3 = arg2->next;
  m.buffer(arg3->name);
  assert(!arg3->next);

  AstNode* stmt = module.List<AstNode>::first;
  assert(stmt->as<StringNode>().str.equals("use asm"));
  stmt = stmt->next;

  stmt = analyze_global_definitions(m, stmt);

  for (; stmt && stmt->is<FuncNode>(); stmt = stmt->next)
    analyze_function_definition(m, stmt->as<FuncNode>());

  for (; stmt && stmt->is<VarNode>(); stmt = stmt->next)
    analyze_func_ptr_table(m, stmt->as<VarNode>());

  analyze_export_stmt(m, stmt->as<ReturnNode>());
  assert(!stmt->next);

  m.finish_analysis();
}

// =================================================================================================
// Write (second) pass

void
write_signature_section(Module& m)
{
  m.write().imm_u32(m.sigs().size());
  for (auto& sig : m.sigs()) {
    m.write().code(sig.ret);
    m.write().imm_u32(sig.args.size());
    for (auto t : sig.args)
      m.write().code(t);
  }
}

void
write_function_import_section(Module& m)
{
  m.write().imm_u32(m.func_imps().size());
  m.write().imm_u32(m.num_func_imp_sigs());
  for (auto& func_imp : m.func_imps()) {
    m.write().c_str(func_imp.external.c_str());
    m.write().imm_u32(func_imp.sigs.size());
    for (auto& func_imp_sig : func_imp.sigs)
      m.write().imm_u32(func_imp_sig.sig_index);
  }
}

void
write_constant_pool_section(Module& m)
{
  m.write().imm_u32(m.i32s().size());
  m.write().imm_u32(m.f32s().size());
  m.write().imm_u32(m.f64s().size());
  for (auto t : m.i32s())
    m.write().imm_u32(t.first);
  for (auto t : m.f32s())
    m.write().fixed_width<uint32_t>(t.first);
  for (auto t : m.f64s())
    m.write().fixed_width<uint64_t>(t.first);
}

void
write_global_section(Module& m)
{
  m.write().imm_u32(m.num_global_i32_zero());
  m.write().imm_u32(m.num_global_f32_zero());
  m.write().imm_u32(m.num_global_f64_zero());
  m.write().imm_u32(m.global_i32_imports().size());
  m.write().imm_u32(m.global_f32_imports().size());
  m.write().imm_u32(m.global_f64_imports().size());
  for (auto i : m.global_i32_imports())
    m.write().c_str(i.c_str());
  for (auto i : m.global_f32_imports())
    m.write().c_str(i.c_str());
  for (auto i : m.global_f64_imports())
    m.write().c_str(i.c_str());
}

void
write_function_declaration_section(Module& m)
{
  m.write().imm_u32(m.funcs().size());
  for (auto& f : m.funcs())
    m.write().imm_u32(f.sig_index());
}

void
write_function_pointer_tables(Module& m)
{
  m.write().imm_u32(m.func_ptr_tables().size());
  for (auto& func_ptr_table : m.func_ptr_tables()) {
    m.write().imm_u32(func_ptr_table.sig_index);
    m.write().imm_u32(func_ptr_table.elems.size());
    for (auto elem : func_ptr_table.elems)
      m.write().imm_u32(elem);
  }
}

enum class Ctx { Expr, Stmt };

void write_expr(Module&, Function&, const AstNode&);
void write_call(Module& m, Function& f, const CallNode& call, Ctx ctx);
void write_stmt(Module&, Function&, const AstNode&);

template <class T, class TWithImm>
void
write_num_lit_pool(Module& m, uint32_t pool_index)
{
  if (pool_index < ImmLimit) {
    m.write().code(TWithImm::LitPool, pool_index);
  } else {
    m.write().code(T::LitPool);
    m.write().imm_u32(pool_index);
  }
}

void
write_num_lit(Module& m, Function& f, NumLit lit, unsigned lshift = 0)
{
  uint32_t pool_index;
  if (m.lit_has_pool_index(lit, lshift, &pool_index)) {
    switch (lit.type()) {
      case Type::I32: write_num_lit_pool<I32, I32WithImm>(m, pool_index); break;
      case Type::F32: write_num_lit_pool<F32, F32WithImm>(m, pool_index); break;
      case Type::F64: write_num_lit_pool<F64, F64WithImm>(m, pool_index); break;
    }
  } else {
    switch (lit.type()) {
      case Type::I32: {
        uint32_t u32 = lit.uint32() << lshift;
        if (u32 < ImmLimit) {
          m.write().code(I32WithImm::LitImm, u32);
        } else {
          m.write().code(I32::LitImm);
          m.write().imm_u32(u32);
        }
        break;
      }
      case Type::F32:
        assert(lshift == 0);
        m.write().code(F32::LitImm);
        m.write().fixed_width<float>(lit.float32());
        break;
      case Type::F64:
        assert(lshift == 0);
        m.write().code(F64::LitImm);
        m.write().fixed_width<double>(lit.float64());
        break;
    }
  }
}

bool
is_num_lit_u32(Module& m, const AstNode& ast, uint32_t u32)
{
  if (!NumLit::is(m, ast))
    return false;
  NumLit lit(m, ast);
  if (lit.type() != Type::I32)
    return false;
  return lit.uint32() == u32;
}

void
write_bitwise(Module& m, Function& f, const BinaryNode& binary, Ctx ctx)
{
  if (is_int_coerced_call(binary)) {
    write_call(m, f, binary.lhs.as<CallNode>(), ctx);
    return;
  }

  assert(ctx == Ctx::Expr);

  bool commutative;
  uint32_t identity = 0;
  switch (binary.expr.i32()) {
    case I32::BitOr: commutative = true; identity = 0; break;
    case I32::BitAnd: commutative = true; identity = UINT32_MAX; break;
    case I32::BitXor: commutative = true; identity = 0; break;
    case I32::Lsh: commutative = false; identity = 0; break;
    case I32::ArithRsh: commutative = false; identity = 0; break;
    case I32::LogicRsh: commutative = false; identity = 0; break;
    default: unreachable<void>();
  }

  if (is_num_lit_u32(m, binary.rhs, identity)) {
    write_expr(m, f, binary.lhs);
    return;
  }

  if (commutative && is_num_lit_u32(m, binary.lhs, identity)) {
    write_expr(m, f, binary.rhs);
    return;
  }

  m.write().code(binary.expr);
  write_expr(m, f, binary.lhs);
  write_expr(m, f, binary.rhs);
}

void
write_index(Module& m, Function& f, const IndexNode& index)
{
  if (index.offset > 0)
    m.write().imm_u32(index.offset);

  if (index.constant)
    write_num_lit(m, f, NumLit(m, *index.index), m.heap_view(index.array.as<NameNode>().str).shift);
  else
    write_expr(m, f, *index.index);
}

void
write_assign(Module& m, Function& f, const BinaryNode& binary, Ctx ctx)
{
  uint32_t index = binary.lhs.as<NameNode>().index;
  if (ctx == Ctx::Expr) {
    m.write().code(binary.expr);
    m.write().imm_u32(index);
  } else {
    if (binary.stmt_with_imm != StmtWithImm::Bad && index < ImmLimit) {
      m.write().code(binary.stmt_with_imm, index);
    } else {
      m.write().code(binary.stmt);
      m.write().imm_u32(index);
    }
  }
  write_expr(m, f, binary.rhs);
}

void
write_store(Module& m, Function& f, const BinaryNode& binary, Ctx ctx)
{
  // The simple case
  if (ctx == Ctx::Stmt || binary.store_rhs_conv.is_bad()) {
    if (ctx == Ctx::Stmt)
      m.write().code(binary.stmt);
    else
      m.write().code(binary.expr);
    write_index(m, f, binary.lhs.as<IndexNode>());
    if (!binary.store_rhs_conv.is_bad())
      m.write().code(binary.store_rhs_conv);
    write_expr(m, f, binary.rhs);
    return;
  }

  // The complex case: the stored value must be converted, but the result of the SetLoc expression
  // must be pre-conversion. Store the pre-conversion value in a temporary local (that we allocated
  // during the analyze phase), convert and store in the lhs of a Comma, and return the
  // pre-conversion value as the rhs of the Comma.

  Type val_type;
  uint32_t temp_local_index;
  if (binary.store_rhs_conv == F64::FromF32) {
    val_type = Type::F32;
    temp_local_index = f.f32_temp();
  } else {
    assert(binary.store_rhs_conv == F32::FromF64);
    val_type = Type::F64;
    temp_local_index = f.f64_temp();
  }

  m.write().code(type_switch(val_type, I32::Bad, F32::Comma, F64::Comma));
  m.write().code(binary.store_rhs_conv.type());

  // comma lhs
  m.write().code(binary.expr);
  write_index(m, f, binary.lhs.as<IndexNode>());
  m.write().code(binary.store_rhs_conv);
  m.write().code(type_switch(val_type, I32::Bad, F32::SetLoc, F64::SetLoc));
  m.write().imm_u32(temp_local_index);
  write_expr(m, f, binary.rhs);

  // comma rhs
  m.write().code(type_switch(val_type, I32::Bad, F32::GetLoc, F64::GetLoc));
  m.write().imm_u32(temp_local_index);
}

void
write_binary(Module& m, Function& f, const BinaryNode& binary, Ctx ctx)
{
  switch (binary.kind) {
    case BinaryNode::Assign:
      write_assign(m, f, binary, ctx);
      break;
    case BinaryNode::Store:
      write_store(m, f, binary, ctx);
      break;
    case BinaryNode::Bitwise:
      write_bitwise(m, f, binary, ctx);
      break;
    case BinaryNode::Comma:
      assert(ctx == Ctx::Expr);
      m.write().code(binary.expr);
      m.write().code(binary.comma_lhs_type);
      write_expr(m, f, binary.lhs);
      write_expr(m, f, binary.rhs);
      break;
    case BinaryNode::Generic:
      assert(ctx == Ctx::Expr);
      m.write().code(binary.expr);
      write_expr(m, f, binary.lhs);
      write_expr(m, f, binary.rhs);
      break;
  }
}

void
write_name(Module& m, Function& f, const NameNode& name)
{
  if (!name.expr_with_imm.is_bad() && name.index < ImmLimit) {
    m.write().code(name.expr_with_imm, name.index);
  } else if (name.expr.is_bad()) {
    write_num_lit(m, f, NumLit(m.stdlib_double(name.str)));
  } else {
    m.write().code(name.expr);
    m.write().imm_u32(name.index);
  }
}

void
write_prefix(Module& m, Function& f, const PrefixNode& prefix, Ctx ctx)
{
  if (is_double_coerced_call(prefix)) {
    if (!prefix.expr.is_bad())
      m.write().code(prefix.expr);
    write_call(m, f, prefix.kid.as<CallNode>(), ctx);
    return;
  }

  assert(ctx == Ctx::Expr);

  if (prefix.expr.is_bad())
    assert(prefix.op.equals("+") || prefix.op.equals("~"));
  else
    m.write().code(prefix.expr);
  write_expr(m, f, prefix.kid);
}

void
write_ternary(Module& m, Function& f, const TernaryNode& ternary)
{
  m.write().code(ternary.expr);
  write_expr(m, f, ternary.cond);
  write_expr(m, f, ternary.lhs);
  write_expr(m, f, ternary.rhs);
}

void
write_call(Module& m, Function& f, const CallNode& call, Ctx ctx)
{
  switch (call.kind) {
    case CallNode::Import: {
      if (ctx == Ctx::Expr)
        m.write().code(call.expr);
      else
        m.write().code(call.stmt);
      auto& func_imp_sig = m.func_imp(call.callee.as<NameNode>().str).sigs[call.import_preindex];
      m.write().imm_u32(func_imp_sig.func_imp_sig_index);
      assert(call.compute_length() == m.sig(func_imp_sig.sig_index).args.size());
      break;
    }
    case CallNode::Internal: {
      if (ctx == Ctx::Expr)
        m.write().code(call.expr);
      else
        m.write().code(call.stmt);
      auto func_index = m.func_index(call.callee.as<NameNode>().str);
      m.write().imm_u32(func_index);
      assert(call.compute_length() == m.func(func_index).sig().args.size());
      break;
    }
    case CallNode::Indirect: {
      if (ctx == Ctx::Expr)
        m.write().code(call.expr);
      else
        m.write().code(call.stmt);
      auto& index = call.callee.as<IndexNode>();
      auto func_ptr_tbl_i = m.func_ptr_table_index(index.array.as<NameNode>().str);
      m.write().imm_u32(func_ptr_tbl_i);
      write_expr(m, f, index.index->as<BinaryNode>().lhs);
      assert(call.compute_length() == m.sig(m.func_ptr_table(func_ptr_tbl_i).sig_index).args.size());
      break;
    }
    case CallNode::NaryBuiltin:
      assert(ctx == Ctx::Expr);
      m.write().code(call.expr);
      m.write().imm_u32(call.compute_length());
      break;
    case CallNode::FixedArityBuiltin:
      assert(ctx == Ctx::Expr);
      m.write().code(call.expr);
      break;
    case CallNode::Fround:
      assert(call.compute_length() == 1);
      if (call.expr.is_bad()) {
        if (call.first->is<CallNode>()) {
          write_call(m, f, call.first->as<CallNode>(), ctx);
          return;
        }
      } else {
        m.write().code(call.expr);
      }
      break;
  }

  for (AstNode* arg = call.first; arg; arg = arg->next)
    write_expr(m, f, *arg);
}

void
write_load(Module& m, Function& f, const IndexNode& index)
{
  m.write().code(index.expr);
  write_index(m, f, index);
}

void
write_expr(Module& m, Function& f, const AstNode& expr)
{
  if (NumLit::is(m, expr)) {
    write_num_lit(m, f, NumLit(m, expr));
  } else {
    switch (expr.which) {
      case AstNode::Binary:
        write_binary(m, f, expr.as<BinaryNode>(), Ctx::Expr);
        break;
      case AstNode::Call:
        write_call(m, f, expr.as<CallNode>(), Ctx::Expr);
        break;
      case AstNode::Prefix:
        write_prefix(m, f, expr.as<PrefixNode>(), Ctx::Expr);
        break;
      case AstNode::Ternary:
        write_ternary(m, f, expr.as<TernaryNode>());
        break;
      case AstNode::Name:
        write_name(m, f, expr.as<NameNode>());
        break;
      case AstNode::Index:
        write_load(m, f, expr.as<IndexNode>());
        break;
      default:
        unreachable<void>();
    }
  }
}

void
write_return(Module& m, Function& f, const ReturnNode& ret)
{
  m.write().code(Stmt::Ret);
  if (ret.expr)
    write_expr(m, f, *ret.expr);
}

void
write_stmt_list(Module& m, Function& f, const AstNode* stmts)
{
  uint32_t num_stmts = 0;
  for (const AstNode* p = stmts; p; p = p->next)
    num_stmts++;

  m.write().imm_u32(num_stmts);
  for (const AstNode* n = stmts; n; n = n->next)
    write_stmt(m, f, *n);
}

void
write_block(Module& m, Function& f, const BlockNode& block)
{
  m.write().code(Stmt::Block);
  write_stmt_list(m, f, block.first);
}

void
write_if(Module& m, Function& f, const IfNode& i)
{
  if (i.if_false) {
    m.write().code(Stmt::IfElse);
    write_expr(m, f, i.cond);
    write_stmt(m, f, i.if_true);
    write_stmt(m, f, *i.if_false);
  } else {
    m.write().code(Stmt::IfThen);
    write_expr(m, f, i.cond);
    write_stmt(m, f, i.if_true);
  }
}

void
write_while(Module& m, Function& f, const WhileNode& w)
{
  m.write().code(Stmt::While);
  write_expr(m, f, w.cond);
  write_stmt(m, f, w.body);
}

void
write_do(Module& m, Function& f, const DoNode& d)
{
  m.write().code(Stmt::Do);
  write_stmt(m, f, d.body);
  write_expr(m, f, d.cond);
}

void
write_label(Module& m, Function& f, const LabelNode& l)
{
  m.write().code(Stmt::Label);
  f.push_label(l.str);
  write_stmt(m, f, l.stmt);
  f.pop_label(l.str);
}

void
write_break(Module& m, Function& f, const BreakNode& b)
{
  if (!b.str) {
    m.write().code(Stmt::Break);
    return;
  }

  m.write().code(Stmt::BreakLabel);
  m.write().imm_u32(f.label_depth(b.str));
}

void
write_continue(Module& m, Function& f, const ContinueNode& c)
{
  if (!c.str) {
    m.write().code(Stmt::Continue);
    return;
  }

  m.write().code(Stmt::ContinueLabel);
  m.write().imm_u32(f.label_depth(c.str));
}

void
write_switch(Module& m, Function& f, const SwitchNode& s)
{
  m.write().code(Stmt::Switch);
  m.write().imm_u32(s.compute_length());
  write_expr(m, f, s.expr);
  bool wrote_default = false;
  for (const CaseNode* c = s.first; c; c = c->next) {
    if (c->label) {
      if (!c->first) {
        m.write().code(SwitchCase::Case0);
        m.write().imm_s32(NumLit(m, *c->label).int32());
      } else if (c->first == c->last) {
        m.write().code(SwitchCase::Case1);
        m.write().imm_s32(NumLit(m, *c->label).int32());
        write_stmt(m, f, c->first->stmt);
      } else {
        m.write().code(SwitchCase::CaseN);
        m.write().imm_s32(NumLit(m, *c->label).int32());
        m.write().imm_u32(c->compute_length());
        for (const CaseStmtNode* s = c->first; s; s = s->next)
          write_stmt(m, f, s->stmt);
      }
    } else {
      assert(!wrote_default);
      wrote_default = true;
      if (!c->first) {
        m.write().code(SwitchCase::Default0);
      } else if (c->first == c->last) {
        m.write().code(SwitchCase::Default1);
        write_stmt(m, f, c->first->stmt);
      } else {
        m.write().code(SwitchCase::DefaultN);
        m.write().imm_u32(c->compute_length());
        for (const CaseStmtNode* s = c->first; s; s = s->next)
          write_stmt(m, f, s->stmt);
      }
    }
  }
}

void
write_stmt(Module& m, Function& f, const AstNode& stmt)
{
  switch (stmt.which) {
    case AstNode::Call: write_call(m, f, stmt.as<CallNode>(), Ctx::Stmt); break;
    case AstNode::Prefix: write_prefix(m, f, stmt.as<PrefixNode>(), Ctx::Stmt); break;
    case AstNode::Binary: write_binary(m, f, stmt.as<BinaryNode>(), Ctx::Stmt); break;
    case AstNode::Return: write_return(m, f, stmt.as<ReturnNode>()); break;
    case AstNode::Block: write_block(m, f, stmt.as<BlockNode>()); break;
    case AstNode::If: write_if(m, f, stmt.as<IfNode>()); break;
    case AstNode::While: write_while(m, f, stmt.as<WhileNode>()); break;
    case AstNode::Do: write_do(m, f, stmt.as<DoNode>()); break;
    case AstNode::Label: write_label(m, f, stmt.as<LabelNode>()); break;
    case AstNode::Break: write_break(m, f, stmt.as<BreakNode>()); break;
    case AstNode::Continue: write_continue(m, f, stmt.as<ContinueNode>()); break;
    case AstNode::Switch: write_switch(m, f, stmt.as<SwitchNode>()); break;
    default: unreachable<void>();
  }
}

void
write_function_definition(Module& m, Function& f)
{
  if (f.num_i32_vars() < ImmLimit && f.num_f32_vars() == 0 && f.num_f64_vars() == 0) {
    m.write().code(VarTypesWithImm::OnlyI32, f.num_i32_vars());
  } else {
    VarTypes vt = (f.num_i32_vars() > 0 ? VarTypes::I32 : VarTypes(0)) |
                  (f.num_f32_vars() > 0 ? VarTypes::F32 : VarTypes(0)) |
                  (f.num_f64_vars() > 0 ? VarTypes::F64 : VarTypes(0));
    m.write().code(vt);
    if (vt & VarTypes::I32)
      m.write().imm_u32(f.num_i32_vars());
    if (vt & VarTypes::F32)
      m.write().imm_u32(f.num_f32_vars());
    if (vt & VarTypes::F64)
      m.write().imm_u32(f.num_f64_vars());
  }

  write_stmt_list(m, f, f.body());
}

void
write_function_definition_section(Module& m)
{
  for (auto& f : m.funcs())
    write_function_definition(m, f);
}

void
write_export_section(Module& m)
{
  if (m.exports().size() == 1 && !m.exports()[0].external) {
    m.write().code(ExportFormat::Default);
    m.write().imm_u32(m.exports().front().internal);
  } else {
    m.write().code(ExportFormat::Record);
    m.write().imm_u32(m.exports().size());
    for (auto& e : m.exports()) {
      m.write().c_str(e.external.c_str());
      m.write().imm_u32(e.internal);
    }
  }
}

void
write_module(Module& m)
{
  m.write().fixed_width<uint32_t>(MagicNumber);

  // Bogus unpacked-size; to be patched in patch_unpacked_size.
  m.write().fixed_width<uint32_t>(UINT32_MAX);

  write_constant_pool_section(m);
  write_signature_section(m);
  write_function_import_section(m);
  write_global_section(m);
  write_function_declaration_section(m);
  write_function_pointer_tables(m);
  write_function_definition_section(m);
  write_export_section(m);
}

// =================================================================================================
// AST building

struct AstNodePtr
{
  AstNode* ptr;
  AstNodePtr(AstNode* ptr = nullptr) : ptr(ptr) {}
  operator AstNode*() const { return ptr; }
};

struct AstBuilder
{
  static AstNodePtr makeToplevel() { return new TopLevelNode(); }
  static AstNodePtr makeFunction(IString name) { return new FuncNode(name); }

  static void appendArgumentToFunction(AstNode* fun, IString name)
  {
    fun->as<FuncNode>().List<ArgNode>::append(*new ArgNode(name));
  }

  static AstNode* stmtify(AstNode* expr)
  {
    if (!expr)
      return nullptr;
    switch (expr->which) {
      case AstNode::New:
      case AstNode::Object:
      case AstNode::Array:
        unreachable<void>();
      case AstNode::TopLevel:
      case AstNode::Function:
      case AstNode::String:
      case AstNode::Var:
      case AstNode::Block:
      case AstNode::Return:
      case AstNode::If:
      case AstNode::While:
      case AstNode::Do:
      case AstNode::Label:
      case AstNode::Break:
      case AstNode::Continue:
      case AstNode::Switch:
      case AstNode::Call:
        return expr;
      case AstNode::Double:
      case AstNode::Int:
      case AstNode::Name:
        return nullptr;
      case AstNode::Dot:
        return stmtify(&expr->as<DotNode>().base);
      case AstNode::Index:
        return stmtify(expr->as<IndexNode>().index);
      case AstNode::Prefix:
        if (!is_double_coerced_call(expr->as<PrefixNode>()))
          return stmtify(&expr->as<PrefixNode>().kid);
        return expr;
      case AstNode::Binary: {
        BinaryNode& binary = expr->as<BinaryNode>();
        if (!binary.op.equals("=") && !is_int_coerced_call(binary)) {
          AstNode* lhs = stmtify(&binary.lhs);
          AstNode* rhs = stmtify(&binary.rhs);
          return lhs && rhs ? new BlockNode(*lhs, *rhs) : lhs ? lhs : rhs;
        }
        return expr;
      }
      case AstNode::Ternary: {
        TernaryNode& ternary = expr->as<TernaryNode>();
        return makeIf(&ternary.cond, &ternary.lhs, &ternary.rhs);
      }
    }
    return nullptr;
  }

  static AstNodePtr makeIf(AstNode* cond, AstNode* if_true, AstNode* if_false)
  {
    assert(cond);
    if_true = stmtify(if_true);
    if_false = stmtify(if_false);
    if (if_true)
      return new IfNode(*cond, *if_true, if_false);
    if (if_false)
      return new IfNode(*new PrefixNode(cashew::L_NOT, *cond), *if_false, nullptr);
    return nullptr;
  }

  static AstNodePtr makeStatement(AstNode* expr) { return stmtify(expr); }
  static AstNodePtr makeBlock() { return new BlockNode(); }
  static AstNodePtr makeReturn(AstNode* expr) { return new ReturnNode(expr); }
  static AstNodePtr makeWhile(AstNode* cond, AstNode* body) { assert(cond); return new WhileNode(*cond, *stmtify(body)); }
  static AstNodePtr makeDo(AstNode* body, AstNode* cond) { assert(cond); return new DoNode(*stmtify(body), *cond); }
  static AstNodePtr makeLabel(IString str, AstNode* stmt) { return new LabelNode(str, *stmtify(stmt)); }
  static AstNodePtr makeBreak(IString str) { return new BreakNode(str); }
  static AstNodePtr makeContinue(IString str) { return new ContinueNode(str); }
  static AstNodePtr makeSwitch(AstNode* expr) { assert(expr); return new SwitchNode(*expr); }

  static void setBlockContent(AstNode* target, AstNode* block) {
    if (target->is<TopLevelNode>())
      target->as<TopLevelNode>().append_list(block->as<BlockNode>());
    else if (target->is<FuncNode>())
      target->as<FuncNode>().List<AstNode>::append_list(block->as<BlockNode>());
    else abort();
  }

  static void appendToBlock(AstNode* block, AstNode* stmt) {
    if (!stmt)
      return;
    block->as<BlockNode>().append(*stmt);
  }

  static void appendCaseToSwitch(AstNode* sw, AstNode* label)
  {
    assert(label);
    sw->as<SwitchNode>().append(*new CaseNode(*label));
  }

  static void appendDefaultToSwitch(AstNode* sw)
  {
    sw->as<SwitchNode>().append(*new CaseNode());
  }

  static void appendCodeToSwitch(AstNode* sw, AstNode* stmt, bool)
  {
    if (!stmt)
      return;
    if (stmt->is<BlockNode>())
      for (AstNode* p = stmt->as<BlockNode>().first; p; p = p->next)
        sw->as<SwitchNode>().last->append(*new CaseStmtNode(*p));
    else
      sw->as<SwitchNode>().last->append(*new CaseStmtNode(*stmtify(stmt)));
  }

  static AstNodePtr makeString(IString str) { return new StringNode(str); }
  static AstNodePtr makeDouble(double f64) { return new DoubleNode(f64); }
  static AstNodePtr makeInt(uint32_t u32) { return new IntNode(u32); }
  static AstNodePtr makeName(IString str) { return new NameNode(str); }
  static AstNodePtr makePrefix(IString op, AstNode* expr) { assert(expr); return new PrefixNode(op, *expr); }
  static AstNodePtr makeBinary(AstNode* lhs, IString op, AstNode* rhs) { assert(lhs && rhs); return new BinaryNode(op, *lhs, *rhs); }
  static AstNodePtr makeConditional(AstNode* cond, AstNode* lhs, AstNode* rhs) { assert(cond && lhs && rhs); return new TernaryNode(*cond, *lhs, *rhs); }
  static AstNodePtr makeNew(AstNode* call) { return new NewNode(call->as<CallNode>()); }
  static AstNodePtr makeDot(AstNode* base, IString name) { assert(base); return new DotNode(*base, name); }
  static AstNodePtr makeDot(AstNode* base, AstNode* name) { assert(base); return new DotNode(*base, name->as<NameNode>().str); }
  static AstNodePtr makeIndexing(AstNode* array, AstNode* index) { assert(array && index); return new IndexNode(*array, *index); }
  static AstNodePtr makeCall(AstNode* callee) { return new CallNode(*callee); }
  static AstNodePtr makeObject() { return new ObjectNode(); }
  static AstNodePtr makeVar(bool is_const) { return new VarNode(); }
  static AstNodePtr makeArray() { return new ArrayNode(); }

  static void appendToCall(AstNode* call, AstNode* arg)
  {
    call->as<CallNode>().append(*arg);
  }

  static void appendToObject(AstNode* object, IString key, AstNode* value)
  {
    assert(value);
    object->as<ObjectNode>().append(*new FieldNode(key, *value));
  }

  static void appendToVar(AstNode* var, IString name, AstNode* init)
  {
    assert(init);
    var->as<VarNode>().append(*new VarNameNode(name, *init));
  }

  static void appendToArray(AstNode* array, AstNode* elem)
  {
    assert(elem);
    array->as<ArrayNode>().append(*elem);
  }
};

const FuncNode&
parse(char* src)
{
  const AstNode* ast = cashew::Parser<AstNodePtr, AstBuilder>().parseToplevel(src);
  const TopLevelNode& top = ast->as<TopLevelNode>();
  if (!top.first || top.first != top.last || !top.first->is<FuncNode>())
    throw runtime_error("there should be exactly one top-level asm.js statement in the asm.js file");

  FuncNode& module_func = top.first->as<FuncNode>();
  List<AstNode>& body = module_func;
  if (!body.first || !body.first->is<StringNode>() || !body.first->as<StringNode>().str.equals("use asm"))
    throw runtime_error("the argument to the top-level call should be an asm.js module");

  return module_func;
}

void
patch_unpacked_size(std::ostream& os, uint32_t unpacked_size)
{
  os.seekp(sizeof(uint32_t));
  Out out(os);
  out.fixed_width<uint32_t>(unpacked_size);
}

void
pack(ostream& os, const FuncNode& module)
{
  Module m(os);
  analyze_module(m, module);
  write_module(m);
}

}  // namespace asmjs

int
main(int argc, char** argv)
try
{
  if (argc != 3 || !argv[1] || !argv[2]) {
    cerr << "Usage: pack-asmjs in.js out.wasm" << endl;
    return -1;
  }

  // Parse the asm.js file.
  ifstream in_stream(argv[1], ios::binary | ios::ate);
  in_stream.exceptions(ios::failbit | ios::badbit);
  vector<char> in_bytes(in_stream.tellg());
  in_stream.seekg(0);
  in_stream.read(in_bytes.data(), in_bytes.size());
  in_bytes.push_back('\0');
  in_stream.close();
  const asmjs::FuncNode& module = asmjs::parse(in_bytes.data());

  // Write out the .asm file (with bogus unpacked-size).
  fstream out_stream(argv[2], ios::in | ios::out | ios::binary | ios::trunc);
  out_stream.exceptions(ios::failbit | ios::badbit);
  asmjs::pack(out_stream, module);

  // Compute unpacked-size (using unpack()) and patch the file.
  vector<uint8_t> out_bytes(out_stream.tellp());
  out_stream.seekg(0);
  out_stream.read((char*)out_bytes.data(), out_bytes.size());
  uint32_t unpacked_size = asmjs::calculate_unpacked_size(out_bytes.data());
  asmjs::patch_unpacked_size(out_stream, unpacked_size);

  return 0;
}
catch(const ios::failure& err)
{
  cerr << "Failed with runtime error: " << err.what() << endl;
  return -1;
}
catch(const runtime_error& err)
{
  cerr << "Failed with runtime error: " << err.what() << endl;
  return -1;
}

