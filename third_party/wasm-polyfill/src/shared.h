// vim: set ts=2 sw=2 tw=99 et:

#ifndef shared_h_
#define shared_h_

#include <cstdint>
#include <cassert>
#include <vector>

namespace asmjs {

// =================================================================================================
// Magic serialization constants

static const uint32_t MagicNumber = 0x6d736177;

enum class Stmt : uint8_t
{
  SetLoc,
  SetGlo,
  I32Store8,
  I32StoreOff8,
  I32Store16,
  I32StoreOff16,
  I32Store32,
  I32StoreOff32,
  F32Store,
  F32StoreOff,
  F64Store,
  F64StoreOff,
  CallInt,
  CallInd,
  CallImp,
  Ret,
  Block,
  IfThen,
  IfElse,
  While,
  Do,
  Label,
  Break,
  BreakLabel,
  Continue,
  ContinueLabel,
  Switch,

  Bad
};

enum class StmtWithImm : uint8_t
{
  SetLoc,
  SetGlo,
  Reseved1,
  Reseved2,

  Bad
};

enum class SwitchCase : uint8_t
{
  Case0,
  Case1,
  CaseN,
  Default0,
  Default1,
  DefaultN
};

enum class I32 : uint8_t
{
  LitPool,
  LitImm,
  GetLoc,
  GetGlo,
  SetLoc,
  SetGlo,
  SLoad8,
  SLoadOff8,
  ULoad8,
  ULoadOff8,
  SLoad16,
  SLoadOff16,
  ULoad16,
  ULoadOff16,
  Load32,
  LoadOff32,
  Store8,
  StoreOff8,
  Store16,
  StoreOff16,
  Store32,
  StoreOff32,
  CallInt,
  CallInd,
  CallImp,
  Cond,
  Comma,
  FromF32,
  FromF64,
  Neg,
  Add,
  Sub,
  Mul,
  SDiv,
  UDiv,
  SMod,
  UMod,
  BitNot,
  BitOr,
  BitAnd,
  BitXor,
  Lsh,
  ArithRsh,
  LogicRsh,
  Clz,
  LogicNot,
  EqI32,
  EqF32,
  EqF64,
  NEqI32,
  NEqF32,
  NEqF64,
  SLeThI32,
  ULeThI32,
  LeThF32,
  LeThF64,
  SLeEqI32,
  ULeEqI32,
  LeEqF32,
  LeEqF64,
  SGrThI32,
  UGrThI32,
  GrThF32,
  GrThF64,
  SGrEqI32,
  UGrEqI32,
  GrEqF32,
  GrEqF64,
  SMin,
  UMin,
  SMax,
  UMax,
  Abs,

  Bad
};

enum class I32WithImm : uint8_t
{
  LitPool,
  LitImm,
  GetLoc,
  Reserved,

  Bad
};

enum class F32 : uint8_t
{
  LitPool,
  LitImm,
  GetLoc,
  GetGlo,
  SetLoc,
  SetGlo,
  Load,
  LoadOff,
  Store,
  StoreOff,
  CallInt,
  CallInd,
  Cond,
  Comma,
  FromS32,
  FromU32,
  FromF64,
  Neg,
  Add,
  Sub,
  Mul,
  Div,
  Abs,
  Ceil,
  Floor,
  Sqrt,

  Bad
};

enum class F32WithImm : uint8_t
{
  LitPool,
  GetLoc,
  Reserved0,
  Reserved1,

  Bad
};

enum class F64 : uint8_t
{
  LitPool,
  LitImm,
  GetLoc,
  GetGlo,
  SetLoc,
  SetGlo,
  Load,
  LoadOff,
  Store,
  StoreOff,
  CallInt,
  CallInd,
  CallImp,
  Cond,
  Comma,
  FromS32,
  FromU32,
  FromF32,
  Neg,
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Min,
  Max,
  Abs,
  Ceil,
  Floor,
  Sqrt,
  Cos,
  Sin,
  Tan,
  ACos,
  ASin,
  ATan,
  ATan2,
  Exp,
  Ln,
  Pow,

  Bad
};

enum class F64WithImm : uint8_t
{
  LitPool,
  GetLoc,
  Reserved0,
  Reserved1,

  Bad
};

enum class Void : uint8_t
{
  CallInt,
  CallInd,
  CallImp,

  Bad
};

enum class Type : uint8_t
{
  I32,
  F32,
  F64
};

enum class VarTypes : uint8_t
{
  I32 = 0x1,
  F32 = 0x2,
  F64 = 0x4,
};

inline VarTypes operator|(VarTypes lhs, VarTypes rhs) { return VarTypes(uint8_t(lhs) | uint8_t(rhs)); }
inline bool operator&(VarTypes lhs, VarTypes rhs) { return bool(uint8_t(lhs) & uint8_t(rhs)); }

enum class VarTypesWithImm : uint8_t
{
  OnlyI32,
  Reserved0,
  Reserved1,
  Reserved2
};

enum class RType : uint8_t
{
  I32 = uint8_t(Type::I32),
  F32 = uint8_t(Type::F32),
  F64 = uint8_t(Type::F64),
  Void
};

inline RType
to_rtype(Type t)
{
  return RType(t);
}

class Expr
{
  RType type_;
  union U {
    I32 i32_;
    F32 f32_;
    F64 f64_;
    Void v_;
    uint8_t raw_;
  } u;

  static_assert(sizeof(U) == sizeof(uint8_t), "Exact overlay of raw_");

public:
  Expr() : type_(RType(-1)), u{} {}
  static Expr Bad() { return Expr(); }
  Expr(I32 i32) : type_(RType::I32) { assert(i32 < I32::Bad); u.i32_ = i32; }
  Expr(F32 f32) : type_(RType::F32) { assert(f32 < F32::Bad); u.f32_ = f32; }
  Expr(F64 f64) : type_(RType::F64) { assert(f64 < F64::Bad); u.f64_ = f64; }
  Expr(Void v) : type_(RType::Void) { assert(v < Void::Bad); u.v_ = v; }

  bool is_bad() const { return type_ == RType(-1); }
  RType type() const { assert(!is_bad()); return type_; }
  I32 i32() const { assert(type_ == RType::I32); return u.i32_; }
  F32 f32() const { assert(type_ == RType::F32); return u.f32_; }
  F64 f64() const { assert(type_ == RType::F64); return u.f64_; }
  uint8_t raw_code() const { assert(!is_bad()); return u.raw_; }

  bool operator==(Expr rhs) const { return type_ == rhs.type_ && u.raw_ == rhs.u.raw_; }
  bool operator!=(Expr rhs) const { return !(*this == rhs); }
};

static const uint8_t HasImmFlag = 0x80;
static_assert(uint8_t(I32::Bad) <= HasImmFlag, "MSB reserved to distinguish I32 from I32WithImm");
static_assert(uint8_t(F32::Bad) <= HasImmFlag, "MSB reserved to distinguish F32 from F32WithImm");
static_assert(uint8_t(F64::Bad) <= HasImmFlag, "MSB reserved to distinguish F64 from F64WithImm");

static const unsigned OpWithImmBits = 2;
static const uint32_t OpWithImmLimit = 1 << OpWithImmBits;
static_assert(uint8_t(I32WithImm::Bad) <= OpWithImmLimit, "I32WithImm op fits");
static_assert(uint8_t(F32WithImm::Bad) <= OpWithImmLimit, "F32WithImm op fits");
static_assert(uint8_t(F64WithImm::Bad) <= OpWithImmLimit, "F64WithImm op fits");

static const unsigned ImmBits = 5;
static const uint32_t ImmLimit = 1 << ImmBits;
static_assert(1 + OpWithImmBits + ImmBits == 8, "Bits of immediate op should add up to a byte");

static inline uint8_t
PackOpWithImm(uint8_t op, uint8_t imm)
{
  assert(op < OpWithImmLimit);
  assert(imm < ImmLimit);
  return HasImmFlag | (uint8_t(op) << ImmBits) | imm;
}

template <class TWithImm>
static inline void
UnpackOpWithImm(uint8_t byte, TWithImm* op, uint8_t *imm)
{
  assert(byte & HasImmFlag);
  *op = TWithImm((byte >> ImmBits) & (OpWithImmLimit - 1));
  *imm = byte & (ImmLimit - 1);
}

class ExprWithImm
{
  Type type_;
  union U {
    I32WithImm i32_;
    F32WithImm f32_;
    F64WithImm f64_;
    uint8_t raw_;
  } u;

  static_assert(sizeof(U) == sizeof(uint8_t), "Exact overlay of raw_");

public:
  ExprWithImm() : type_(Type(-1)) {}
  ExprWithImm(I32WithImm i32) : type_(Type::I32) { assert(i32 < I32WithImm::Bad); u.i32_ = i32; }
  ExprWithImm(F32WithImm f32) : type_(Type::F32) { assert(f32 < F32WithImm::Bad); u.f32_ = f32; }
  ExprWithImm(F64WithImm f64) : type_(Type::F64) { assert(f64 < F64WithImm::Bad); u.f64_ = f64; }

  bool is_bad() const { return type_ == Type(-1); }
  Type type() const { return type_; }
  I32WithImm i32() const { assert(type_ == Type::I32); return u.i32_; }
  F32WithImm f32() const { assert(type_ == Type::F32); return u.f32_; }
  F64WithImm f64() const { assert(type_ == Type::F64); return u.f64_; }
  uint8_t raw_code() const { assert(!is_bad()); return u.raw_; }
};

enum class ExportFormat : uint8_t
{
  Default,
  Record
};

// =================================================================================================
// Utilities

template <class T>
T inline
unreachable()
{
  assert(false && "unreachable");
  return *(T*)nullptr;
}

template <>
void inline
unreachable<void>()
{
  assert(false && "unreachable");
}

template <class T>
T inline
type_switch(Type type, T i32, T f32, T f64)
{
  switch (type) {
    case Type::I32: return i32;
    case Type::F32: return f32;
    case Type::F64: return f64;
  }
  return unreachable<T>();
}

inline Expr
type_switch(RType type, I32 i32, F32 f32, F64 f64, Void v)
{
  switch (type) {
    case RType::I32: return i32;
    case RType::F32: return f32;
    case RType::F64: return f64;
    case RType::Void: return v;
  }
  return unreachable<Expr>();
}

template <class T>
inline T
type_switch(RType type, T i32, T f32, T f64, T v)
{
  switch (type) {
    case RType::I32: return i32;
    case RType::F32: return f32;
    case RType::F64: return f64;
    case RType::Void: return v;
  }
  return unreachable<T>();
}

inline Expr
type_switch(Type type, I32 i32, F32 f32, F64 f64)
{
  switch (type) {
    case Type::I32: return i32;
    case Type::F32: return f32;
    case Type::F64: return f64;
  }
  return unreachable<Expr>();
}

enum Signedness { Signed, Unsigned };

template <class T>
inline Expr
signedness_switch(Signedness si, T s32, T u32)
{
  switch (si) {
    case Signed: return s32;
    case Unsigned: return u32;
  }
  return unreachable<T>();
}

struct Signature
{
  RType ret;
  std::vector<Type> args;

  Signature() {}
  Signature(RType ret) : ret(ret) {}
  Signature(RType ret, uint32_t num_args) : ret(ret), args(num_args) {}
  Signature(RType ret, std::vector<Type>&& args) : ret(ret), args(move(args)) {}

  bool operator==(const Signature& rhs) const { return ret == rhs.ret && args == rhs.args; }
  bool operator!=(const Signature& rhs) const { return !(*this == rhs); }

  struct Hash {
    bool operator()(const Signature &sig) const {
      static_assert(sizeof(Type) == 1 && sizeof(RType) == 1, "Shift more");
      uint32_t h = uint8_t(sig.ret);
      for (Type t : sig.args)
        h = ((h << 2) | uint8_t(t)) ^ (h >> 30);
      return h;
    }
  };
};

// =================================================================================================

class In
{
  const uint8_t* cur_;

  template <class T> T u8() { return T(*cur_++); }

public:
  In(const uint8_t* beg) : cur_(beg) {}

  template <class T> T fixed_width();
  Stmt stmt() { return u8<Stmt>(); }
  SwitchCase switch_case() { return u8<SwitchCase>(); }
  template<class T, class TWithImm> inline bool code(T*, TWithImm*, uint8_t*);
  Void void_expr() { return u8<Void>(); }
  ExportFormat export_format() { return u8<ExportFormat>(); }
  Type type() { return u8<Type>(); }
  RType rtype() { return u8<RType>(); }
  inline uint32_t imm_u32();
  inline int32_t imm_s32();
  char single_char() { return *cur_++; }

  inline bool if_i32_lit(const std::vector<uint32_t>& i32s, uint32_t* u32);
  inline bool is_next_node_block();
};

template <>
uint32_t inline
In::fixed_width<uint32_t>()
{
  uint32_t u32 = cur_[0] | cur_[1] << 8 | cur_[2] << 16 | cur_[3] << 24;
  cur_ += 4;
  return u32;
}

template <>
float inline
In::fixed_width<float>()
{
  union {
    uint8_t arr[4];
    float f;
  } u = { { cur_[0], cur_[1], cur_[2], cur_[3] } };
  cur_ += 4;
  return u.f;
}

template <>
double inline
In::fixed_width<double>()
{
  union {
    uint8_t arr[8];
    double d;
  } u = { { cur_[0], cur_[1], cur_[2], cur_[3], cur_[4], cur_[5], cur_[6], cur_[7] } };
  cur_ += 8;
  return u.d;
}

template <class T, class TWithImm>
bool
In::code(T* t, TWithImm* t_with_imm, uint8_t* imm)
{
  uint8_t byte = *cur_++;
  if (!(byte & HasImmFlag)) {
    *t = T(byte);
    return true;
  }

  UnpackOpWithImm(byte, t_with_imm, imm);
  return false;
}

uint32_t inline
In::imm_u32()
{
  uint32_t u32 = *cur_++;
  if (u32 < 0x80)
    return u32;

  u32 &= 0x7f;

  for (unsigned shift = 7; true; shift += 7) {
    uint32_t b = *cur_++;
    if (b < 0x80)
      return u32 | (b << shift);
    u32 |= (b & 0x7f) << shift;
  }
}

int32_t inline
In::imm_s32()
{
  uint32_t u32 = *cur_++;
  if (u32 < 0x80)
    return int32_t(u32) << (32-7) >> (32-7);

  u32 &= 0x7f;

  for (unsigned shift = 7; true; shift += 7) {
    uint32_t b = *cur_++;
    if (b < 0x80) {
      u32 |= b << shift;
      int sign_extend = (32-7) - shift;
      if (sign_extend > 0)
        return int32_t(u32) << sign_extend >> sign_extend;
      return int32_t(u32);
    }
    u32 |= (b & 0x7f) << shift;
  }
}

bool inline
In::if_i32_lit(const std::vector<uint32_t>& i32s, uint32_t* u32)
{
  uint8_t byte = *cur_;

  if (byte & HasImmFlag) {
    I32WithImm i32_with_imm;
    uint8_t imm;
    UnpackOpWithImm(byte, &i32_with_imm, &imm);

    if (i32_with_imm == I32WithImm::LitImm) {
      cur_++;
      *u32 = imm;
      return true;
    }

    if (i32_with_imm == I32WithImm::LitPool) {
      cur_++;
      *u32 = i32s[imm];
      return true;
    }

    return false;
  }

  if (I32(byte) == I32::LitImm) {
    cur_++;
    *u32 = imm_u32();
    return true;
  }

  if (I32(byte) == I32::LitPool) {
    cur_++;
    *u32 = i32s[imm_u32()];
    return true;
  }

  return false;
}

bool inline
In::is_next_node_block()
{
  static_assert(uint32_t(Stmt::Block) < 0x80,
                "We can fetch the code in a single byte");
  return Stmt(*cur_) == Stmt::Block;
}

}  // namespace asmjs
#endif // shared_h_
