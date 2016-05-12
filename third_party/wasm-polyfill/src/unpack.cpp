// vim: set ts=2 sw=2 tw=99 et:

#include "unpack.h"

#include "shared.h"

#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cmath>

#ifdef __EMSCRIPTEN__
# include <emscripten.h>
#endif

using namespace std;

// TODO:
//  - Use pool allocator for vector et al to avoid malloc (or at least inline allocation).
//  - include ascii magic number (to ensure we got the right file)
//  - add a built-with-version to easily catch tool mismatches

namespace asmjs {

const uint8_t iden_chars[] = {
  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  '_', '$',
  '0','1','2','3','4','5','6','7','8','9'
};
static const unsigned FirstCharRange = 26 * 2 + 2;
static const unsigned FirstCharRangeMinusDollar = 26 * 2 + 1;
static const unsigned NextCharRange = sizeof(iden_chars);

enum class HotStdLib : unsigned
{
  HeapS8,
  HeapU8,
  HeapS16,
  HeapU16,
  HeapS32,
  HeapU32,
  HeapF32,
  HeapF64,
  IMul,
  FRound,
  Count
};

enum class StdLib : unsigned
{
  stdlib,
  foreign,
  buffer,
  acos,
  asin,
  atan,
  cos,
  sin,
  tan,
  exp,
  log,
  ceil,
  floor,
  sqrt,
  abs,
  min,
  max,
  atan2,
  pow,
  clz32,
  NaN,
  Infinity,
  Count
};

class Utf8Writer
{
  uint8_t* cur_;

#ifdef CHECKED_OUTPUT_SIZE
  uint8_t* limit_;
  vector<uint8_t> bytes_;

  void check_write(int32_t bytes_to_write)
  {
    if (limit_ - cur_ >= bytes_to_write)
      return;

    size_t cur_off = cur_ - bytes_.data();
    bytes_.resize(cur_off + bytes_to_write + 16*1024);
    cur_ = bytes_.data() + cur_off;
    limit_ = bytes_.data() + bytes_.size();
  }
#else
# ifndef NDEBUG
  uint8_t* const begin_;
  const uint32_t unpacked_size_;
# endif

  void inline check_write(size_t bytes_to_write)
  {
    assert((cur_ - begin_) + bytes_to_write <= unpacked_size_);
  }
#endif

  // In the encoded format, variables are named by dense indices. A straight ASCII-decimal encoding
  // would only use 10 of the 64 possible single-byte UTF8 JS identifiers and thus generate longer
  // variable names than necessary. (Since the goal is speed and non-ASCII identifiers often trigger
  // slow paths in JS tokenizers, only consider the ASCII subset of UTF-8.) This function maps
  // indices surjectively onto the enumeration of valid ASCII JS identifiers.
  template <uint32_t FirstCharRange>
  void indexed_name(uint32_t i)
  {
    // Take advantage of power-of-two length of iden_chars to replace slow integer division and
    // modulus operations with fast bitwise operations.
    static_assert(sizeof(iden_chars) == 64, "assumed below");

    if (i < FirstCharRange) {
      check_write(1);
      *cur_++ = iden_chars[i];
      return;
    }
    i -= FirstCharRange;

    if (i < FirstCharRange * 64) {
      check_write(2);
      *cur_++ = iden_chars[i >> 6];
      *cur_++ = iden_chars[i & 0x3f];

      // Append _ if we would otherwise generate one of the two-letter keywords.
      assert(iden_chars[0x205>>6] == 'i' && iden_chars[0x205&0x3f] == 'f');
      assert(iden_chars[0x20d>>6] == 'i' && iden_chars[0x20d&0x3f] == 'n');
      assert(iden_chars[0x0ce>>6] == 'd' && iden_chars[0x0ce&0x3f] == 'o');
      if (FirstCharRange < NextCharRange && (i == 0x205 || i == 0x20d || i == 0x0ce)) {
        check_write(1);
        *cur_++ = '_';
      }
      return;
    }
    i -= FirstCharRange * 64;

    // Instead of trying to catch every >2 letter keyword, just inject a _.
    if (FirstCharRange < NextCharRange) {
      check_write(1);
      *cur_++ = '_';
    }

    uint32_t len = 0;
    do {
      check_write(1);
      *cur_++ = iden_chars[i & 0x3f];
      len++;
      i = i >> 6;
    } while (i >= FirstCharRange * 64);
    check_write(2);
    *cur_++ = iden_chars[i & 0x3f];
    *cur_++ = iden_chars[i >> 6];
    len += 2;
    reverse(cur_ - len, cur_);
  }

public:

#ifdef CHECKED_OUTPUT_SIZE
  Utf8Writer() : cur_(nullptr), limit_(nullptr) {}
  uint32_t finish() { return cur_ - bytes_.data(); }
#else
  Utf8Writer(uint32_t sz, uint8_t* begin)
  : cur_(begin)
#ifndef NDEBUG
  , begin_(begin)
  , unpacked_size_(sz)
#endif
  {}

  void finish()
  {
    assert(unpacked_size_ == cur_ - begin_);
  }
#endif

  bool has_token_ambiguity(char c)
  {
    return c == '+' || c == '-';
  }

  template <unsigned N>
  void ascii(const char (&str)[N])
  {
    if (N > 0 && has_token_ambiguity(str[0]) && cur_[-1] == str[0]) {
      check_write(1);
      *cur_++ = ' ';
    }
    for (unsigned i = 0; i < N-1; i++) {
      check_write(1);
      *cur_++ = str[i];
    }
  }

  void ascii(unsigned char c)
  {
    assert(c < 0x80);
    if (has_token_ambiguity(c) && cur_[-1] == c) {
      check_write(1);
      *cur_++ = ' ';
    }
    check_write(1);
    *cur_++ = c;
  }

  void dynamic_ascii(const char *str)
  {
    while (*str)
      ascii(*str++);
  }

  void uint32(uint32_t u32)
  {
    if (u32 <= 9) {
      check_write(1);
      *cur_++ = '0' + u32;
      return;
    }

    uint32_t len = 0;
    do {
      check_write(1);
      *cur_++ = '0' + u32 % 10;
      len++;
      u32 /= 10;
    } while (u32 > 0);
    reverse(cur_ - len, cur_);
  }

  void int32(int32_t i32)
  {
    if (i32 >= 0) {
      uint32(i32);
    } else {
      ascii('-');
      uint32(-i32);
    }
  }

  void float32(double f)
  {
    name(HotStdLib::FRound);
    ascii('(');
    float64((double)f);
    ascii(')');
  }

  void float64(double d)
  {
    if (std::isnan(d))
        return name(StdLib::NaN);

    if (std::isinf(d))
        return name(StdLib::Infinity);

    // handle negative numbers. to detect -0 (which has -0 == 0), we must
    // inspect the sign bit.
    if (d < 0 || (d == 0 && std::signbit((float)d) == 1)) {
      ascii('-');
      d = -d;
    }

    static const unsigned N = 100;
    char buf[N];
    uint32_t len;

#if __EMSCRIPTEN__
    len = emscripten_print_double(d, buf, 100);
    assert(len < N);
#else
    len = snprintf(buf, N, "%g", d);
#endif

    check_write(len);
    memcpy(cur_, buf, len);
    uint8_t *beg = cur_;
    cur_ += len;

    for (uint8_t* p = beg; p != cur_; ++p) {
      if (*p == '.')
        return;
    }

    for (uint8_t* p = beg; p != cur_; ++p) {
      if (*p == 'e' || *p == 'E') {
        check_write(1);
        for (uint8_t* q = cur_++; q != p; q--)
          *q = *(q - 1);
        *p = '.';
        return;
      }
    }

    check_write(1);
    *cur_++ = '.';
  }

  void name(HotStdLib i)
  {
    assert(i < HotStdLib::Count);
    check_write(1);
    *cur_++ = 'a' + unsigned(i);
  }
  
  void local_name(uint32_t i)
  {
    indexed_name<FirstCharRangeMinusDollar>(i + uint32_t(HotStdLib::Count));
  }

  void name(StdLib i)
  {
    assert(i < StdLib::Count);
    check_write(2);
    *cur_++ = '$';
    *cur_++ = 'a' + unsigned(i);
  }

  void global_name(uint32_t i)
  {
    check_write(1);
    *cur_++ = '$';
    indexed_name<NextCharRange>(i + uint32_t(StdLib::Count));
  }

  void label_name(uint32_t i)
  {
    indexed_name<FirstCharRange>(i);
  }
};

template <>
void
Utf8Writer::ascii(const char (&str)[1])
{
  assert(!str[0]);
}

template <>
void
Utf8Writer::ascii(const char (&str)[2])
{
  if (has_token_ambiguity(str[0]) && cur_[-1] == str[0]) {
    check_write(1);
    *cur_++ = ' ';
  }
  check_write(1);
  *cur_++ = str[0];
  assert(!str[1]);
}

template <>
void
Utf8Writer::ascii(const char (&str)[3])
{
  if (has_token_ambiguity(str[0]) && cur_[-1] == str[0]) {
    check_write(1);
    *cur_++ = ' ';
  }
  check_write(2);
  *cur_++ = str[0];
  *cur_++ = str[1];
  assert(!str[2]);
}

template <>
void
Utf8Writer::ascii(const char (&str)[4])
{
  if (has_token_ambiguity(str[0]) && cur_[-1] == str[0]) {
    check_write(1);
    *cur_++ = ' ';
  }
  check_write(3);
  *cur_++ = str[0];
  *cur_++ = str[1];
  *cur_++ = str[2];
  assert(!str[3]);
}

struct FuncImportSignature
{
  uint32_t sig_index;
  uint32_t func_imp_index;
};

struct FuncPtrTable
{
  uint32_t sig_index;
  vector<uint32_t> elems;
};

uint32_t
cb_name_len(const char* cb_name)
{
  if (!cb_name)
    return 0;
  return strlen(cb_name) + 2;  // cb_name(...)
}

class State
{
  vector<Signature> sigs_;
  vector<uint32_t> i32s_;
  vector<float> f32s_;
  vector<double> f64s_;
  uint32_t num_func_imps_;
  vector<FuncImportSignature> func_imp_sigs_;
  vector<Type> global_types_;
  uint32_t func_name_base_;
  vector<uint32_t> func_sigs_;
  uint32_t func_ptr_table_name_base_;
  vector<FuncPtrTable> func_ptr_tables_;

  uint32_t num_labels_;
  RType cur_ret_;
  vector<Type> cur_local_types_;

public:
  In read;
  Utf8Writer write;

#ifdef CHECKED_OUTPUT_SIZE
  State(const uint8_t* in)
  : num_labels_(0)
  , read(in)
  {
    if (read.fixed_width<uint32_t>() != MagicNumber)
      abort();
    (void)read.fixed_width<uint32_t>();
  }
#else
  State(const uint8_t* in, const char* cb_name, uint32_t out_size, uint8_t* out)
  : num_labels_(0)
  , read(in)
  , write(out_size, out)
  {
    if (read.fixed_width<uint32_t>() != MagicNumber)
      abort();
    if (read.fixed_width<uint32_t>() != out_size - cb_name_len(cb_name))
      abort();
  }
#endif

  void set_sigs(vector<Signature>&& sigs)
  {
    sigs_ = move(sigs);
  }
  void set_pools(vector<uint32_t>&& i32s, vector<float>&& f32s, vector<double>&& f64s)
  {
    i32s_ = move(i32s);
    f32s_ = move(f32s);
    f64s_ = move(f64s);
  }
  void set_func_imps(uint32_t num_func_imps, vector<FuncImportSignature>&& func_imp_sigs)
  {
    num_func_imps_ = num_func_imps;
    func_imp_sigs_ = move(func_imp_sigs);
  }
  void set_global_types(vector<Type>&& global_types)
  {
    global_types_ = move(global_types);
    func_name_base_ = num_func_imps_ + global_types_.size();
  }
  void set_funcs(vector<uint32_t>&& func_sigs)
  {
    func_sigs_ = move(func_sigs);
    func_ptr_table_name_base_ = func_name_base_ + func_sigs_.size();
  }
  void set_func_ptr_tables(vector<FuncPtrTable>&& f)
  {
    func_ptr_tables_ = move(f);
  }

  void write_func_imp_name(uint32_t i)
  {
    write.global_name(i);
  }
  void write_global_name(uint32_t i)
  {
    write.global_name(num_func_imps_ + i);
  }
  void write_func_name(uint32_t i)
  {
    write.global_name(func_name_base_ + i);
  }
  void write_func_ptr_table_name(uint32_t i)
  {
    write.global_name(func_ptr_table_name_base_ + i);
  }

  const vector<Signature>& sigs() const { return sigs_; }
  const Signature& sig(size_t i) const { return sigs_[i]; }
  Type global_type(size_t i) { return global_types_[i]; }
  const FuncImportSignature& func_imp_sig(size_t i) const { return func_imp_sigs_[i]; }
  size_t num_funcs() const { return func_sigs_.size(); }
  const Signature& func_sig(size_t i) { return sigs_[func_sigs_[i]]; }
  size_t num_func_ptr_tables() const { return func_ptr_tables_.size(); }
  const FuncPtrTable& func_ptr_table(size_t i) { return func_ptr_tables_[i]; }
  const vector<uint32_t>& i32s() const { return i32s_; }
  const vector<float>& f32s() const { return f32s_; }
  const vector<double>& f64s() const { return f64s_; }

  void set_cur_ret(RType ret) { cur_ret_ = ret; }
  RType cur_ret() const { return cur_ret_; }
  vector<Type>& cur_local_types() { return cur_local_types_; }
  Type cur_local_type(size_t i) const { return cur_local_types_[i]; }

  uint32_t push_label() { return num_labels_++; }
  void pop_label() { num_labels_--; }
};

void
signature_section(State& s)
{
  uint32_t num_sigs = s.read.imm_u32();
  vector<Signature> sigs(num_sigs);
  for (uint32_t sig_i = 0; sig_i != num_sigs; sig_i++) {
    RType ret = s.read.rtype();
    uint32_t num_args = s.read.imm_u32();
    Signature sig(ret, num_args);
    for (uint32_t arg_i = 0; arg_i < num_args; arg_i++)
      sig.args[arg_i] = s.read.type();
    sigs[sig_i] = move(sig);
  }

  s.set_sigs(move(sigs));
}

void
function_import_section(State& s)
{
  uint32_t num_func_imps = s.read.imm_u32();
  uint32_t num_func_imp_sigs = s.read.imm_u32();

  vector<FuncImportSignature> func_imp_sigs(num_func_imp_sigs);
  FuncImportSignature* func_imp_sig = func_imp_sigs.data();
  for (uint32_t func_imp_i = 0; func_imp_i < num_func_imps; func_imp_i++) {
    s.write.ascii("var ");
    s.write_func_imp_name(func_imp_i);
    s.write.ascii('=');
    s.write.name(StdLib::foreign);
    s.write.ascii('.');
    while (char c = s.read.single_char())
      s.write.ascii(c);
    s.write.ascii(";\n");

    uint32_t num_sigs = s.read.imm_u32();
    for (uint32_t i = 0; i < num_sigs; i++, func_imp_sig++) {
      func_imp_sig->sig_index = s.read.imm_u32();
      func_imp_sig->func_imp_index = func_imp_i;
    }
  }
  assert(func_imp_sig == func_imp_sigs.data() + num_func_imp_sigs);

  s.set_func_imps(num_func_imps, move(func_imp_sigs));
}

void
global_section(State& s)
{
  uint32_t num_i32_zero = s.read.imm_u32();
  uint32_t num_f32_zero = s.read.imm_u32();
  uint32_t num_f64_zero = s.read.imm_u32();
  uint32_t num_i32_import = s.read.imm_u32();
  uint32_t num_f32_import = s.read.imm_u32();
  uint32_t num_f64_import = s.read.imm_u32();

  uint32_t num_global_vars = num_i32_zero +
                             num_f32_zero +
                             num_f64_zero +
                             num_i32_import +
                             num_f32_import +
                             num_f64_import;

  vector<Type> global_types;
  global_types.reserve(num_global_vars);

  for (uint32_t i = 0; i < num_i32_zero; i++) {
    s.write.ascii("var ");
    s.write_global_name(global_types.size());
    s.write.ascii("=0;\n");
    global_types.push_back(Type::I32);
  }
  for (uint32_t i = 0; i < num_f32_zero; i++) {
    s.write.ascii("var ");
    s.write_global_name(global_types.size());
    s.write.ascii('=');
    s.write.name(HotStdLib::FRound);
    s.write.ascii("(0);\n");
    global_types.push_back(Type::F32);
  }
  for (uint32_t i = 0; i < num_f64_zero; i++) {
    s.write.ascii("var ");
    s.write_global_name(global_types.size());
    s.write.ascii("=0.;\n");
    global_types.push_back(Type::F64);
  }
  for (uint32_t i = 0; i < num_i32_import; i++) {
    s.write.ascii("var ");
    s.write_global_name(global_types.size());
    s.write.ascii('=');
    s.write.name(StdLib::foreign);
    s.write.ascii('.');
    while (char c = s.read.single_char())
      s.write.ascii(c);
    s.write.ascii("|0;\n");
    global_types.push_back(Type::I32);
  }
  for (uint32_t i = 0; i < num_f32_import; i++) {
    s.write.ascii("var ");
    s.write_global_name(global_types.size());
    s.write.ascii("=");
    s.write.name(HotStdLib::FRound);
    s.write.ascii('(');
    s.write.name(StdLib::foreign);
    s.write.ascii('.');
    while (char c = s.read.single_char())
      s.write.ascii(c);
    s.write.ascii(");\n");
    global_types.push_back(Type::F32);
  }
  for (uint32_t i = 0; i < num_f64_import; i++) {
    s.write.ascii("var ");
    s.write_global_name(global_types.size());
    s.write.ascii("=+");
    s.write.name(StdLib::foreign);
    s.write.ascii('.');
    while (char c = s.read.single_char())
      s.write.ascii(c);
    s.write.ascii(";\n");
    global_types.push_back(Type::F64);
  }
  assert(global_types.size() == num_global_vars);

  s.set_global_types(move(global_types));
}

void
constant_pool_section(State& s)
{
  uint32_t num_i32s = s.read.imm_u32();
  uint32_t num_f32s = s.read.imm_u32();
  uint32_t num_f64s = s.read.imm_u32();

  vector<uint32_t> i32s(num_i32s);
  for (uint32_t i = 0; i < num_i32s; i++)
    i32s[i] = s.read.imm_u32();

  vector<float> f32s(num_f32s);
  for (uint32_t i = 0; i < num_f32s; i++)
    f32s[i] = s.read.fixed_width<float>();

  vector<double> f64s(num_f64s);
  for (uint32_t i = 0; i < num_f64s; i++)
    f64s[i] = s.read.fixed_width<double>();

  s.set_pools(move(i32s), move(f32s), move(f64s));
}

void
function_declaration_section(State& s)
{
  uint32_t num_funcs = s.read.imm_u32();
  vector<uint32_t> func_sigs(num_funcs);
  for (uint32_t i = 0; i != num_funcs; i++)
    func_sigs[i] = s.read.imm_u32();

  s.set_funcs(move(func_sigs));
}

void
read_function_pointer_tables(State& s)
{
  uint32_t num_func_ptr_tables = s.read.imm_u32();
  vector<FuncPtrTable> func_ptr_tables(num_func_ptr_tables);
  for (uint32_t func_ptr_tbl_i = 0; func_ptr_tbl_i != num_func_ptr_tables; func_ptr_tbl_i++) {
    func_ptr_tables[func_ptr_tbl_i].sig_index = s.read.imm_u32();
    uint32_t num_elems = s.read.imm_u32();
    vector<uint32_t> elems(num_elems);
    for (uint32_t elem_i = 0; elem_i != num_elems; elem_i++)
      elems[elem_i] = s.read.imm_u32();
    func_ptr_tables[func_ptr_tbl_i].elems = move(elems);
  }

  s.set_func_ptr_tables(move(func_ptr_tables));
}

enum class Prec : unsigned {
  Lowest = 0,
  Comma = 2,
  Assign = 4,
  Cond = 6,
  BitOr = 8,
  BitXor = 10,
  BitAnd = 12,
  Eq = 14,
  Comp = 16,
  Shifts = 18,
  AddSub = 20,
  MulDivMod = 22,
  Unary = 24,
  Call = 26,
  Index = 28,
  Highest = 30
};

Prec reject_same(Prec prec) { return prec; }
Prec accept_same(Prec prec) { return Prec(unsigned(prec) - 1); }
bool need_paren(Prec parent, Prec child) { return unsigned(child) <= unsigned(parent); }

enum class Ctx
{
  Default,
  AddSub,
  ToI32,
  FRound,
  ToNumber
};

template <RType T> void expr(State& s, Prec prec, Ctx ctx = Ctx::Default);

void expr_i32(State&, Prec, Ctx);
void expr_f32(State&, Prec, Ctx);
void expr_f64(State&, Prec, Ctx);
void expr_void(State&, Prec, Ctx);
template <> void expr<RType::I32>(State& s, Prec prec, Ctx ctx) { expr_i32(s, prec, ctx); }
template <> void expr<RType::F32>(State& s, Prec prec, Ctx ctx) { expr_f32(s, prec, ctx); }
template <> void expr<RType::F64>(State& s, Prec prec, Ctx ctx) { expr_f64(s, prec, ctx); }
template <> void expr<RType::Void>(State& s, Prec prec, Ctx ctx) { expr_void(s, prec, ctx); }

void
signed_expr(State& s, Prec prec, Signedness si)
{
  uint32_t u32;
  if (s.read.if_i32_lit(s.i32s(), &u32)) {
    if (si == Signed)
      s.write.int32(u32);
    else
      s.write.uint32(u32);
  } else if (si == Signed) {
    if (need_paren(prec, Prec::BitOr)) {
      s.write.ascii('(');
      expr<RType::I32>(s, accept_same(Prec::BitOr), Ctx::ToI32);
      s.write.ascii("|0");
      s.write.ascii(')');
    } else {
      expr<RType::I32>(s, accept_same(Prec::BitOr), Ctx::ToI32);
      s.write.ascii("|0");
    }
  } else {
    if (need_paren(prec, Prec::Shifts)) {
      s.write.ascii('(');
      expr<RType::I32>(s, accept_same(Prec::Shifts), Ctx::ToI32);
      s.write.ascii(">>>0");
      s.write.ascii(')');
    } else {
      expr<RType::I32>(s, accept_same(Prec::Shifts), Ctx::ToI32);
      s.write.ascii(">>>0");
    }
  }
}

void
get_local(State& s)
{
  s.write.local_name(s.read.imm_u32());
}

template <Type T>
void
set_local(State& s, uint32_t imm)
{
  s.write.local_name(imm);
  s.write.ascii('=');
  expr<RType(T)>(s, accept_same(Prec::Assign));
}

template <Type T>
void
set_local(State& s, Prec prec, uint32_t imm)
{
  if (need_paren(prec, Prec::Assign)) {
    s.write.ascii('(');
    set_local<T>(s, imm);
    s.write.ascii(')');
  } else {
    set_local<T>(s, imm);
  }
}

template <Type T>
void
set_local(State& s, Prec prec)
{
  set_local<T>(s, prec, s.read.imm_u32());
}

void
set_local(State& s, Prec prec, uint32_t imm)
{
  switch (s.cur_local_type(imm)) {
    case Type::I32: set_local<Type::I32>(s, prec, imm); break;
    case Type::F32: set_local<Type::F32>(s, prec, imm); break;
    case Type::F64: set_local<Type::F64>(s, prec, imm); break;
    default: unreachable<void>();
  }
}

void
set_local(State& s, Prec prec)
{
  set_local(s, prec, s.read.imm_u32());
}

void
get_global(State& s)
{
  s.write_global_name(s.read.imm_u32());
}

template <Type T>
void
set_global(State& s, uint32_t imm)
{
  s.write_global_name(imm);
  s.write.ascii('=');
  expr<RType(T)>(s, accept_same(Prec::Assign));
}

template <Type T>
void
set_global(State& s, Prec prec, uint32_t imm)
{
  if (need_paren(prec, Prec::Assign)) {
    s.write.ascii('(');
    set_global<T>(s, imm);
    s.write.ascii(')');
  } else {
    set_global<T>(s, imm);
  }
}

template <Type T>
void
set_global(State& s, Prec prec)
{
  set_global<T>(s, prec, s.read.imm_u32());
}

void
set_global(State& s, Prec prec, uint32_t imm)
{
  switch (s.global_type(imm)) {
    case Type::I32: set_global<Type::I32>(s, prec, imm); break;
    case Type::F32: set_global<Type::F32>(s, prec, imm); break;
    case Type::F64: set_global<Type::F64>(s, prec, imm); break;
    default: unreachable<void>();
  }
}

void
set_global(State& s, Prec prec)
{
  set_global(s, prec, s.read.imm_u32());
}

void
index(State& s, HotStdLib name, unsigned shift, bool has_offset)
{
  s.write.name(name);
  s.write.ascii('[');
  uint32_t offset = has_offset ? s.read.imm_u32() : 0;
  uint32_t u32;
  if (s.read.if_i32_lit(s.i32s(), &u32)) {
    u32 += offset;
    if (u32 < 16*1024*1024) {
      s.write.uint32(u32 >> shift);
    } else {
      s.write.uint32(u32);
      s.write.ascii(">>");
      s.write.uint32(shift);
    }
  } else {
    if (offset) {
      expr<RType::I32>(s, accept_same(Prec::AddSub), Ctx::AddSub);
      s.write.ascii('+');
      s.write.uint32(offset);
    } else {
      expr<RType::I32>(s, accept_same(Prec::Shifts), Ctx::ToI32);
    }
    s.write.ascii(">>");
    s.write.uint32(shift);
  }
  s.write.ascii("]");
}

void
load_i32(State& s, Prec prec, Ctx ctx, HotStdLib name, unsigned shift, bool offset)
{
  if (ctx != Ctx::ToI32) {
    if (need_paren(prec, Prec::BitOr)) {
      s.write.ascii('(');
      index(s, name, shift, offset);
      s.write.ascii("|0)");
    } else {
      index(s, name, shift, offset);
      s.write.ascii("|0");
    }
  } else {
    index(s, name, shift, offset);
  }
}

void
load_f32(State& s, Prec prec, Ctx ctx, bool offset)
{
  if (ctx != Ctx::FRound && ctx != Ctx::ToNumber) {
    if (need_paren(prec, Prec::Unary)) {
      s.write.ascii('(');
      s.write.name(HotStdLib::FRound);
      s.write.ascii('(');
      index(s, HotStdLib::HeapF32, 2, offset);
      s.write.ascii("))");
    } else {
      s.write.name(HotStdLib::FRound);
      s.write.ascii('(');
      index(s, HotStdLib::HeapF32, 2, offset);
      s.write.ascii(')');
    }
  } else {
    index(s, HotStdLib::HeapF32, 2, offset);
  }
}

void
load_f64(State& s, Prec prec, Ctx ctx, bool offset)
{
  if (ctx != Ctx::ToNumber) {
    if (need_paren(prec, Prec::Unary)) {
      s.write.ascii("(+");
      index(s, HotStdLib::HeapF64, 3, offset);
      s.write.ascii(')');
    } else {
      s.write.ascii('+');
      index(s, HotStdLib::HeapF64, 3, offset);
    }
  } else {
    index(s, HotStdLib::HeapF64, 3, offset);
  }
}

template <RType T>
void
store(State& s, Prec prec, HotStdLib name, unsigned shift, Ctx child_ctx, bool offset)
{
  if (need_paren(prec, Prec::Assign)) {
    s.write.ascii('(');
    index(s, name, shift, offset);
    s.write.ascii('=');
    expr<T>(s, accept_same(Prec::Assign), child_ctx);
    s.write.ascii(')');
  } else {
    index(s, name, shift, offset);
    s.write.ascii('=');
    expr<T>(s, accept_same(Prec::Assign), child_ctx);
  }
}

void
store_i8(State& s, Prec prec, bool offset)
{
  store<RType::I32>(s, prec, HotStdLib::HeapS8, 0, Ctx::ToI32, offset);
}

void
store_i16(State& s, Prec prec, bool offset)
{
  store<RType::I32>(s, prec, HotStdLib::HeapS16, 1, Ctx::ToI32, offset);
}

void
store_i32(State& s, Prec prec, bool offset)
{
  store<RType::I32>(s, prec, HotStdLib::HeapS32, 2, Ctx::ToI32, offset);
}

void
store_f32(State& s, Prec prec, bool offset)
{
  store<RType::F32>(s, prec, HotStdLib::HeapF32, 2, Ctx::FRound, offset);
}

void
store_f64(State& s, Prec prec, bool offset)
{
  store<RType::F64>(s, prec, HotStdLib::HeapF64, 3, Ctx::ToNumber, offset);
}

void
fround_i32(State& s, Prec prec, Signedness si)
{
  assert(!need_paren(prec, Prec::Call));
  s.write.name(HotStdLib::FRound);
  s.write.ascii('(');
  signed_expr(s, reject_same(Prec::Comma), si);
  s.write.ascii(')');
}

void
fround_f64(State& s, Prec prec)
{
  assert(!need_paren(prec, Prec::Call));
  s.write.name(HotStdLib::FRound);
  s.write.ascii('(');
  expr<RType::F64>(s, reject_same(Prec::Comma));
  s.write.ascii(')');
}

void
pos_i32(State& s, Prec prec, Signedness si)
{
  if (need_paren(prec, Prec::Unary)) {
    s.write.ascii("(+");
    signed_expr(s, accept_same(Prec::Unary), si);
    s.write.ascii(')');
  } else {
    s.write.ascii('+');
    signed_expr(s, accept_same(Prec::Unary), si);
  }
}

void
pos_f32(State& s, Prec prec)
{
  if (need_paren(prec, Prec::Unary)) {
    s.write.ascii('(');
    s.write.ascii('+');
    expr<RType::F32>(s, accept_same(Prec::Unary), Ctx::ToNumber);
    s.write.ascii(')');
  } else {
    s.write.ascii('+');
    expr<RType::F32>(s, accept_same(Prec::Unary), Ctx::ToNumber);
  }
}

enum class Result
{
  NoIsh,
  Intish,
  Floatish
};

template <RType T, unsigned N>
void
prefix(State& s, Prec parent_prec, Ctx parent_ctx, Result r, Ctx child_ctx, const char (&op)[N])
{
  if (r == Result::Intish && parent_ctx != Ctx::ToI32) {
    if (need_paren(parent_prec, Prec::BitOr)) {
      s.write.ascii('(');
      s.write.ascii(op);
      expr<T>(s, accept_same(Prec::Unary), child_ctx);
      s.write.ascii("|0)");
    } else {
      s.write.ascii(op);
      expr<T>(s, accept_same(Prec::Unary), child_ctx);
      s.write.ascii("|0");
    }
  } else if (r == Result::Floatish && parent_ctx != Ctx::FRound) {
    assert(!need_paren(parent_prec, Prec::Call));
    s.write.name(HotStdLib::FRound);
    s.write.ascii('(');
    s.write.ascii(op);
    expr<T>(s, accept_same(Prec::Unary), child_ctx);
    s.write.ascii(')');
  } else {
    if (need_paren(parent_prec, Prec::Unary)) {
      s.write.ascii('(');
      s.write.ascii(op);
      expr<T>(s, accept_same(Prec::Unary), child_ctx);
      s.write.ascii(')');
    } else {
      s.write.ascii(op);
      expr<T>(s, accept_same(Prec::Unary), child_ctx);
    }
  }
}

template <RType T>
void
add_sub(State& s, char op)
{
  expr<T>(s, accept_same(Prec::AddSub), Ctx::AddSub);
  s.write.ascii(op);
  expr<T>(s, reject_same(Prec::AddSub), Ctx::AddSub);
}

template <RType T>
void
add_sub(State& s, Prec prec, Ctx ctx, char op)
{
  if (T == RType::I32 && ctx != Ctx::ToI32 && ctx != Ctx::AddSub) {
    if (need_paren(prec, Prec::BitOr)) {
      s.write.ascii('(');
      add_sub<T>(s, op);
      s.write.ascii("|0)");
    } else {
      add_sub<T>(s, op);
      s.write.ascii("|0");
    }
  } else if (T == RType::F32 && ctx != Ctx::FRound) {
    assert(!need_paren(prec, Prec::Call));
    s.write.name(HotStdLib::FRound);
    s.write.ascii('(');
    add_sub<T>(s, op);
    s.write.ascii(')');
  } else {
    if (need_paren(prec, Prec::AddSub)) {
      s.write.ascii('(');
      add_sub<T>(s, op);
      s.write.ascii(')');
    } else {
      add_sub<T>(s, op);
    }
  }
}

void
mul_i32(State& s, Prec prec)
{
  assert(!need_paren(prec, Prec::Call));
  s.write.name(HotStdLib::IMul);
  s.write.ascii('(');
  expr<RType::I32>(s, reject_same(Prec::Comma));
  s.write.ascii(',');
  expr<RType::I32>(s, reject_same(Prec::Comma));
  s.write.ascii(')');
}

template <RType T>
void
mul_div_mod_f(State& s, char op)
{
  expr<T>(s, accept_same(Prec::MulDivMod));
  s.write.ascii(op);
  expr<T>(s, reject_same(Prec::MulDivMod));
}

void
mul_div_mod_f32(State& s, Prec prec, Ctx ctx, char op)
{
  if (ctx != Ctx::FRound) {
    assert(!need_paren(prec, Prec::Call));
    s.write.name(HotStdLib::FRound);
    s.write.ascii('(');
    mul_div_mod_f<RType::F32>(s, op);
    s.write.ascii(')');
  } else {
    if (need_paren(prec, Prec::MulDivMod)) {
      s.write.ascii('(');
      mul_div_mod_f<RType::F32>(s, op);
      s.write.ascii(')');
    } else {
      mul_div_mod_f<RType::F32>(s, op);
    }
  }
}

void
mul_div_mod_f64(State& s, Prec prec, char op)
{
  if (need_paren(prec, Prec::MulDivMod)) {
    s.write.ascii('(');
    mul_div_mod_f<RType::F64>(s, op);
    s.write.ascii(')');
  } else {
    mul_div_mod_f<RType::F64>(s, op);
  }
}

void
div_mod_i32(State& s, Signedness si, char op)
{
  signed_expr(s, accept_same(Prec::MulDivMod), si);
  s.write.ascii(op);
  signed_expr(s, reject_same(Prec::MulDivMod), si);
}

void
div_mod_i32(State& s, Prec prec, Ctx ctx, Signedness si, char op)
{
  if (ctx != Ctx::FRound) {
    if (need_paren(prec, Prec::BitOr)) {
      s.write.ascii('(');
      div_mod_i32(s, si, op);
      s.write.ascii("|0)");
    } else {
      div_mod_i32(s, si, op);
      s.write.ascii("|0");
    }
  } else {
    if (need_paren(prec, Prec::MulDivMod)) {
      s.write.ascii('(');
      div_mod_i32(s, si, op);
      s.write.ascii(')');
    } else {
      div_mod_i32(s, si, op);
    }
  }
}

template <unsigned N>
void
bitwise(State& s, Prec child_prec, Ctx child_ctx, const char (&op)[N])
{
  expr<RType::I32>(s, accept_same(child_prec), child_ctx);
  s.write.ascii(op);
  expr<RType::I32>(s, reject_same(child_prec), child_ctx);
}

template <unsigned N>
void
bitwise(State& s, Prec parent_prec, Prec child_prec, Ctx child_ctx, const char (&op)[N])
{
  if (need_paren(parent_prec, child_prec)) {
    s.write.ascii('(');
    bitwise(s, child_prec, child_ctx, op);
    s.write.ascii(')');
  } else {
    bitwise(s, child_prec, child_ctx, op);
  }
}

template <unsigned N>
void
rel_i32(State& s, Prec child_prec, Signedness si, const char (&op)[N])
{
  signed_expr(s, accept_same(child_prec), si);
  s.write.ascii(op);
  signed_expr(s, reject_same(child_prec), si);
}

template <unsigned N>
void
rel_i32(State& s, Prec parent_prec, Prec child_prec, Signedness si, const char (&op)[N])
{
  if (need_paren(parent_prec, child_prec)) {
    s.write.ascii('(');
    rel_i32(s, child_prec, si, op);
    s.write.ascii(')');
  } else {
    rel_i32(s, child_prec, si, op);
  }
}

template <RType T, unsigned N>
void
rel_f(State& s, Prec child_prec, const char (&op)[N])
{
  expr<T>(s, accept_same(child_prec));
  s.write.ascii(op);
  expr<T>(s, reject_same(child_prec));
}

template <RType T, unsigned N>
void
rel_f(State& s, Prec parent_prec, Prec child_prec, const char (&op)[N])
{
  if (need_paren(parent_prec, child_prec)) {
    s.write.ascii('(');
    rel_f<T>(s, child_prec, op);
    s.write.ascii(')');
  } else {
    rel_f<T>(s, child_prec, op);
  }
}

template <RType T>
void
comma(State& s)
{
  switch (s.read.rtype()) {
    case RType::I32: expr<RType::I32>(s, accept_same(Prec::Comma)); break;
    case RType::F32: expr<RType::F32>(s, accept_same(Prec::Comma)); break;
    case RType::F64: expr<RType::F64>(s, accept_same(Prec::Comma)); break;
    case RType::Void: expr<RType::Void>(s, accept_same(Prec::Comma)); break;
  }
  s.write.ascii(',');
  expr<T>(s, accept_same(Prec::Comma));
}

template <RType T>
void
comma(State& s, Prec prec)
{
  if (need_paren(prec, Prec::Comma)) {
    s.write.ascii('(');
    comma<T>(s);
    s.write.ascii(')');
  } else {
    comma<T>(s);
  }
}

template <RType T>
void
cond(State& s)
{
  expr<RType::I32>(s, reject_same(Prec::Cond));
  s.write.ascii('?');
  expr<T>(s, accept_same(Prec::Cond));
  s.write.ascii(':');
  expr<T>(s, accept_same(Prec::Cond));
}

template <RType T>
void
cond(State& s, Prec prec)
{
  if (need_paren(prec, Prec::Cond)) {
    s.write.ascii('(');
    cond<T>(s);
    s.write.ascii(')');
  } else {
    cond<T>(s);
  }
}

void
call_args(State& s, const vector<Type>& args)
{
  s.write.ascii('(');
  if (uint32_t num_args = args.size()) {
    for (uint32_t i = 0;;) {
      switch (args[i]) {
        case Type::I32: expr<RType::I32>(s, reject_same(Prec::Comma)); break;
        case Type::F32: expr<RType::F32>(s, reject_same(Prec::Comma)); break;
        case Type::F64: expr<RType::F64>(s, reject_same(Prec::Comma)); break;
      }
      if (++i == num_args)
        break;
      s.write.ascii(',');
    }
  }
  s.write.ascii(')');
}

void
call_internal(State& s, uint32_t func_index)
{
  s.write_func_name(func_index);
  call_args(s, s.func_sig(func_index).args);
}

void
call_internal_void(State& s)
{
  call_internal(s, s.read.imm_u32());
}

void
call_internal_i32(State& s, Prec prec, uint32_t func_index)
{
  assert(!need_paren(prec, Prec::Call));
  if (need_paren(prec, Prec::BitOr)) {
    s.write.ascii('(');
    call_internal(s, func_index);
    s.write.ascii("|0)");
  } else {
    call_internal(s, func_index);
    s.write.ascii("|0");
  }
}

void
call_internal_i32(State& s, Prec prec)
{
  call_internal_i32(s, prec, s.read.imm_u32());
}

void
call_internal_f32(State& s, Prec prec, uint32_t func_index)
{
  assert(!need_paren(prec, Prec::Call));
  s.write.name(HotStdLib::FRound);
  s.write.ascii('(');
  call_internal(s, func_index);
  s.write.ascii(')');
}

void
call_internal_f32(State& s, Prec prec)
{
  call_internal_f32(s, prec, s.read.imm_u32());
}

void
call_internal_f64(State& s, Prec prec, uint32_t func_index)
{
  assert(!need_paren(prec, Prec::Call));
  if (need_paren(prec, Prec::Unary)) {
    s.write.ascii("(+");
    call_internal(s, func_index);
    s.write.ascii(')');
  } else {
    s.write.ascii('+');
    call_internal(s, func_index);
  }
}

void
call_internal_f64(State& s, Prec prec)
{
  call_internal_f64(s, prec, s.read.imm_u32());
}

void
call_internal(State& s, Prec prec)
{
  uint32_t func_index = s.read.imm_u32();
  switch (s.func_sig(func_index).ret) {
    case RType::I32: call_internal_i32(s, prec, func_index); break;
    case RType::F32: call_internal_f32(s, prec, func_index); break;
    case RType::F64: call_internal_f64(s, prec, func_index); break;
    case RType::Void: call_internal(s, func_index); break;
  }
}

void
call_indirect(State& s, uint32_t func_ptr_tbl_i)
{
  const FuncPtrTable& func_ptr_table = s.func_ptr_table(func_ptr_tbl_i);
  s.write_func_ptr_table_name(func_ptr_tbl_i);
  s.write.ascii('[');
  expr<RType::I32>(s, accept_same(Prec::BitAnd), Ctx::ToI32);
  s.write.ascii('&');
  s.write.uint32(func_ptr_table.elems.size() - 1);
  s.write.ascii(']');
  call_args(s, s.sig(func_ptr_table.sig_index).args);
}

void
call_indirect_void(State& s)
{
  call_indirect(s, s.read.imm_u32());
}

void
call_indirect_i32(State& s, Prec prec, uint32_t func_ptr_tbl_i)
{
  assert(!need_paren(prec, Prec::Call));
  if (need_paren(prec, Prec::BitOr)) {
    s.write.ascii('(');
    call_indirect(s, func_ptr_tbl_i);
    s.write.ascii("|0)");
  } else {
    call_indirect(s, func_ptr_tbl_i);
    s.write.ascii("|0");
  }
}

void
call_indirect_i32(State& s, Prec prec)
{
  call_indirect_i32(s, prec, s.read.imm_u32());
}

void
call_indirect_f32(State& s, Prec prec, uint32_t func_ptr_tbl_i)
{
  assert(!need_paren(prec, Prec::Call));
  s.write.name(HotStdLib::FRound);
  s.write.ascii('(');
  call_indirect(s, func_ptr_tbl_i);
  s.write.ascii(')');
}

void
call_indirect_f32(State& s, Prec prec)
{
  call_indirect_f32(s, prec, s.read.imm_u32());
}

void
call_indirect_f64(State& s, Prec prec, uint32_t func_ptr_tbl_i)
{
  assert(!need_paren(prec, Prec::Call));
  if (need_paren(prec, Prec::Unary)) {
    s.write.ascii('(');
    s.write.ascii('+');
    call_indirect(s, func_ptr_tbl_i);
    s.write.ascii(')');
  } else {
    s.write.ascii('+');
    call_indirect(s, func_ptr_tbl_i);
  }
}

void
call_indirect_f64(State& s, Prec prec)
{
  call_indirect_f64(s, prec, s.read.imm_u32());
}

void
call_indirect(State& s, Prec prec)
{
  uint32_t func_ptr_tbl_i = s.read.imm_u32();
  switch (s.sig(s.func_ptr_table(func_ptr_tbl_i).sig_index).ret) {
    case RType::I32: call_indirect_i32(s, prec, func_ptr_tbl_i); break;
    case RType::F32: call_indirect_f32(s, prec, func_ptr_tbl_i); break;
    case RType::F64: call_indirect_f64(s, prec, func_ptr_tbl_i); break;
    case RType::Void: call_indirect(s, func_ptr_tbl_i); break;
  }
}

void
call_import(State& s, uint32_t func_imp_sig_i)
{
  const FuncImportSignature& func_imp_sig = s.func_imp_sig(func_imp_sig_i);
  s.write_func_imp_name(func_imp_sig.func_imp_index);
  s.write.ascii('(');
  auto& args = s.sig(func_imp_sig.sig_index).args;
  if (uint32_t num_args = args.size()) {
    for (uint32_t i = 0;;) {
      switch (args[i]) {
        case Type::I32: signed_expr(s, reject_same(Prec::Comma), Signed); break;
        case Type::F32: expr<RType::F32>(s, reject_same(Prec::Comma)); break;
        case Type::F64: expr<RType::F64>(s, reject_same(Prec::Comma)); break;
      }
      if (++i == num_args)
        break;
      s.write.ascii(',');
    }
  }
  s.write.ascii(')');
}

void
call_import_void(State& s)
{
  call_import(s, s.read.imm_u32());
}

void
call_import_i32(State& s, Prec prec, uint32_t func_imp_sig_i)
{
  if (need_paren(prec, Prec::BitOr)) {
    s.write.ascii('(');
    call_import(s, func_imp_sig_i);
    s.write.ascii("|0)");
  } else {
    call_import(s, func_imp_sig_i);
    s.write.ascii("|0");
  }
}

void
call_import_i32(State& s, Prec prec)
{
  call_import_i32(s, prec, s.read.imm_u32());
}

void
call_import_f64(State& s, Prec prec, uint32_t func_imp_sig_i)
{
  if (need_paren(prec, Prec::Unary)) {
    s.write.ascii("(+");
    call_import(s, func_imp_sig_i);
    s.write.ascii(')');
  } else {
    s.write.ascii('+');
    call_import(s, func_imp_sig_i);
  }
}

void
call_import_f64(State& s, Prec prec)
{
  call_import_f64(s, prec, s.read.imm_u32());
}

void
call_import(State& s, Prec prec)
{
  uint32_t func_imp_sig_i = s.read.imm_u32();
  switch (s.sig(s.func_imp_sig(func_imp_sig_i).sig_index).ret) {
    case RType::I32: call_import_i32(s, prec, func_imp_sig_i); break;
    case RType::F32: unreachable<void>();
    case RType::F64: call_import_f64(s, prec, func_imp_sig_i); break;
    case RType::Void: call_import(s, func_imp_sig_i); break;
  }
}

void
min_max_i32(State& s, Prec prec, Signedness si, StdLib stdlib)
{
  assert(!need_paren(prec, Prec::Call));
  s.write.name(stdlib);
  s.write.ascii('(');
  if (uint32_t num_args = s.read.imm_u32()) {
    for (uint32_t i = 0;;) {
      signed_expr(s, reject_same(Prec::Comma), si);
      if (++i == num_args)
        break;
      s.write.ascii(',');
    }
  }
  s.write.ascii(')');
}

void
min_max_f64(State& s, Prec prec, StdLib stdlib)
{
  assert(!need_paren(prec, Prec::Call));
  s.write.name(stdlib);
  s.write.ascii('(');
  if (uint32_t num_args = s.read.imm_u32()) {
    for (uint32_t i = 0;;) {
      expr<RType::F64>(s, reject_same(Prec::Comma), Ctx::ToNumber);
      if (++i == num_args)
        break;
      s.write.ascii(',');
    }
  }
  s.write.ascii(')');
}

void
abs_i32(State& s, Prec prec, StdLib stdlib)
{
  assert(!need_paren(prec, Prec::Call));
  s.write.name(stdlib);
  s.write.ascii('(');
  signed_expr(s, reject_same(Prec::Comma), Signed);
  s.write.ascii(')');
}

void
clz_i32(State& s, Prec parent_prec, StdLib stdlib)
{
  assert(!need_paren(parent_prec, Prec::Call));
  s.write.name(stdlib);
  s.write.ascii('(');
  expr<RType::I32>(s, reject_same(Prec::Comma), Ctx::ToI32);
  s.write.ascii(')');
}

void
unary_stdlib_f32(State& s, Prec parent_prec, StdLib stdlib)
{
  assert(!need_paren(parent_prec, Prec::Call));
  s.write.name(HotStdLib::FRound);
  s.write.ascii('(');
  s.write.name(stdlib);
  s.write.ascii('(');
  expr<RType::F32>(s, reject_same(Prec::Comma), Ctx::ToNumber);
  s.write.ascii("))");
}

void
unary_stdlib_f64(State& s, Prec parent_prec, StdLib stdlib)
{
  assert(!need_paren(parent_prec, Prec::Call));
  s.write.name(stdlib);
  s.write.ascii('(');
  expr<RType::F64>(s, reject_same(Prec::Comma), Ctx::ToNumber);
  s.write.ascii(')');
}

void
binary_stdlib_f64(State& s, Prec parent_prec, StdLib stdlib)
{
  assert(!need_paren(parent_prec, Prec::Call));
  s.write.name(stdlib);
  s.write.ascii('(');
  expr<RType::F64>(s, reject_same(Prec::Comma), Ctx::ToNumber);
  s.write.ascii(',');
  expr<RType::F64>(s, reject_same(Prec::Comma), Ctx::ToNumber);
  s.write.ascii(')');
}

void
expr_i32(State& s, Prec prec, Ctx ctx)
{
  I32 i32;
  I32WithImm i32_with_imm;
  uint8_t imm;
  if (s.read.code(&i32, &i32_with_imm, &imm)) {
    switch (i32) {
      case I32::LitImm:     s.write.int32(s.read.imm_u32()); break;
      case I32::LitPool:    s.write.int32(s.i32s()[s.read.imm_u32()]); break;
      case I32::GetLoc:     get_local(s); break;
      case I32::GetGlo:     get_global(s); break;
      case I32::SetLoc:     set_local<Type::I32>(s, prec); break;
      case I32::SetGlo:     set_global<Type::I32>(s, prec); break;
      case I32::SLoad8:     load_i32(s, prec, ctx, HotStdLib::HeapS8, 0, false); break;
      case I32::SLoadOff8:  load_i32(s, prec, ctx, HotStdLib::HeapS8, 0, true); break;
      case I32::ULoad8:     load_i32(s, prec, ctx, HotStdLib::HeapU8, 0, false); break;
      case I32::ULoadOff8:  load_i32(s, prec, ctx, HotStdLib::HeapU8, 0, true); break;
      case I32::SLoad16:    load_i32(s, prec, ctx, HotStdLib::HeapS16, 1, false); break;
      case I32::SLoadOff16: load_i32(s, prec, ctx, HotStdLib::HeapS16, 1, true); break;
      case I32::ULoad16:    load_i32(s, prec, ctx, HotStdLib::HeapU16, 1, false); break;
      case I32::ULoadOff16: load_i32(s, prec, ctx, HotStdLib::HeapU16, 1, true); break;
      case I32::Load32:     load_i32(s, prec, ctx, HotStdLib::HeapS32, 2, false); break;
      case I32::LoadOff32:  load_i32(s, prec, ctx, HotStdLib::HeapS32, 2, true); break;
      case I32::Store8:     store_i8(s, prec, false); break;
      case I32::StoreOff8:  store_i8(s, prec, true); break;
      case I32::Store16:    store_i16(s, prec, false); break;
      case I32::StoreOff16: store_i16(s, prec, true); break;
      case I32::Store32:    store_i32(s, prec, false); break;
      case I32::StoreOff32: store_i32(s, prec, true); break;
      case I32::CallInt:    call_internal_i32(s, prec); break;
      case I32::CallInd:    call_indirect_i32(s, prec); break;
      case I32::CallImp:    call_import_i32(s, prec); break;
      case I32::Cond:       cond<RType::I32>(s, prec); break;
      case I32::Comma:      comma<RType::I32>(s, prec); break;
      case I32::FromF32:    prefix<RType::F32>(s, prec, ctx, Result::NoIsh, Ctx::Default, "~~"); break;
      case I32::FromF64:    prefix<RType::F64>(s, prec, ctx, Result::NoIsh, Ctx::Default, "~~"); break;
      case I32::Neg:        prefix<RType::I32>(s, prec, ctx, Result::Intish, Ctx::Default, "-"); break;
      case I32::Add:        add_sub<RType::I32>(s, prec, ctx, '+'); break;
      case I32::Sub:        add_sub<RType::I32>(s, prec, ctx, '-'); break;
      case I32::Mul:        mul_i32(s, prec); break;
      case I32::SDiv:       div_mod_i32(s, prec, ctx, Signed, '/'); break;
      case I32::UDiv:       div_mod_i32(s, prec, ctx, Unsigned, '/'); break;
      case I32::SMod:       div_mod_i32(s, prec, ctx, Signed, '%'); break;
      case I32::UMod:       div_mod_i32(s, prec, ctx, Unsigned, '%'); break;
      case I32::BitNot:     prefix<RType::I32>(s, prec, ctx, Result::NoIsh, Ctx::ToI32, "~"); break;
      case I32::BitOr:      bitwise(s, prec, Prec::BitOr, Ctx::ToI32, "|"); break;
      case I32::BitAnd:     bitwise(s, prec, Prec::BitAnd, Ctx::ToI32, "&"); break;
      case I32::BitXor:     bitwise(s, prec, Prec::BitXor, Ctx::ToI32, "^"); break;
      case I32::Lsh:        bitwise(s, prec, Prec::Shifts, Ctx::ToI32, "<<"); break;
      case I32::ArithRsh:   bitwise(s, prec, Prec::Shifts, Ctx::ToI32, ">>"); break;
      case I32::LogicRsh:   bitwise(s, prec, Prec::Shifts, Ctx::ToI32, ">>>"); break;
      case I32::Clz:        clz_i32(s, prec, StdLib::clz32); break;
      case I32::LogicNot:   prefix<RType::I32>(s, prec, ctx, Result::NoIsh, Ctx::Default, "!"); break;
      case I32::EqI32:      rel_i32(s, prec, Prec::Eq, Signed, "=="); break;
      case I32::EqF32:      rel_f<RType::F32>(s, prec, Prec::Eq, "=="); break;
      case I32::EqF64:      rel_f<RType::F64>(s, prec, Prec::Eq, "=="); break;
      case I32::NEqI32:     rel_i32(s, prec, Prec::Eq, Signed, "!="); break;
      case I32::NEqF32:     rel_f<RType::F32>(s, prec, Prec::Eq, "!="); break;
      case I32::NEqF64:     rel_f<RType::F64>(s, prec, Prec::Eq, "!="); break;
      case I32::SLeThI32:   rel_i32(s, prec, Prec::Comp, Signed, "<"); break;
      case I32::ULeThI32:   rel_i32(s, prec, Prec::Comp, Unsigned, "<"); break;
      case I32::LeThF32:    rel_f<RType::F32>(s, prec, Prec::Comp, "<"); break;
      case I32::LeThF64:    rel_f<RType::F64>(s, prec, Prec::Comp, "<"); break;
      case I32::SLeEqI32:   rel_i32(s, prec, Prec::Comp, Signed, "<="); break;
      case I32::ULeEqI32:   rel_i32(s, prec, Prec::Comp, Unsigned, "<="); break;
      case I32::LeEqF32:    rel_f<RType::F32>(s, prec, Prec::Comp, "<="); break;
      case I32::LeEqF64:    rel_f<RType::F64>(s, prec, Prec::Comp, "<="); break;
      case I32::SGrThI32:   rel_i32(s, prec, Prec::Comp, Signed, ">"); break;
      case I32::UGrThI32:   rel_i32(s, prec, Prec::Comp, Unsigned, ">"); break;
      case I32::GrThF32:    rel_f<RType::F32>(s, prec, Prec::Comp, ">"); break;
      case I32::GrThF64:    rel_f<RType::F64>(s, prec, Prec::Comp, ">"); break;
      case I32::SGrEqI32:   rel_i32(s, prec, Prec::Comp, Signed, ">="); break;
      case I32::UGrEqI32:   rel_i32(s, prec, Prec::Comp, Unsigned, ">="); break;
      case I32::GrEqF32:    rel_f<RType::F32>(s, prec, Prec::Comp, ">="); break;
      case I32::GrEqF64:    rel_f<RType::F64>(s, prec, Prec::Comp, ">="); break;
      case I32::SMin:       min_max_i32(s, prec, Signed, StdLib::min); break;
      case I32::UMin:       min_max_i32(s, prec, Unsigned, StdLib::min); break;
      case I32::SMax:       min_max_i32(s, prec, Signed, StdLib::max); break;
      case I32::UMax:       min_max_i32(s, prec, Unsigned, StdLib::max); break;
      case I32::Abs:        abs_i32(s, prec, StdLib::abs); break;
      default: unreachable<void>();
    }
  } else {
    switch (i32_with_imm) {
      case I32WithImm::LitImm:  s.write.int32(imm); break;
      case I32WithImm::LitPool: s.write.int32(s.i32s()[imm]); break;
      case I32WithImm::GetLoc:  s.write.local_name(imm); break;
      default: unreachable<void>();
    }
  }
}

void
expr_f32(State& s, Prec prec, Ctx ctx)
{
  F32 f32;
  F32WithImm f32_with_imm;
  uint8_t imm;
  if (s.read.code(&f32, &f32_with_imm, &imm)) {
    switch (f32) {
      case F32::LitImm:   s.write.float32(s.read.fixed_width<float>()); break;
      case F32::LitPool:  s.write.float32(s.f32s()[s.read.imm_u32()]); break;
      case F32::GetLoc:   get_local(s); break;
      case F32::GetGlo:   get_global(s); break;
      case F32::SetLoc:   set_local<Type::F32>(s, prec); break;
      case F32::SetGlo:   set_global<Type::F32>(s, prec); break;
      case F32::Load:     load_f32(s, prec, ctx, false); break;
      case F32::LoadOff:  load_f32(s, prec, ctx, true); break;
      case F32::Store:    store_f32(s, prec, false); break;
      case F32::StoreOff: store_f32(s, prec, true); break;
      case F32::CallInt:  call_internal_f32(s, prec); break;
      case F32::CallInd:  call_indirect_f32(s, prec); break;
      case F32::Cond:     cond<RType::F32>(s, prec); break;
      case F32::Comma:    comma<RType::F32>(s, prec); break;
      case F32::FromS32:  fround_i32(s, prec, Signed); break;
      case F32::FromU32:  fround_i32(s, prec, Unsigned); break;
      case F32::FromF64:  fround_f64(s, prec); break;
      case F32::Neg:      prefix<RType::F32>(s, prec, ctx, Result::Floatish, Ctx::Default, "-"); break;
      case F32::Add:      add_sub<RType::F32>(s, prec, ctx, '+'); break;
      case F32::Sub:      add_sub<RType::F32>(s, prec, ctx, '-'); break;
      case F32::Mul:      mul_div_mod_f32(s, prec, ctx, '*'); break;
      case F32::Div:      mul_div_mod_f32(s, prec, ctx, '/'); break;
      case F32::Abs:      unary_stdlib_f32(s, prec, StdLib::abs); break;
      case F32::Ceil:     unary_stdlib_f32(s, prec, StdLib::ceil); break;
      case F32::Floor:    unary_stdlib_f32(s, prec, StdLib::floor); break;
      case F32::Sqrt:     unary_stdlib_f32(s, prec, StdLib::sqrt); break;
      default: unreachable<void>();
    }
  } else {
    switch (f32_with_imm) {
      case F32WithImm::LitPool: s.write.float32(s.f32s()[imm]); break;
      case F32WithImm::GetLoc:  s.write.local_name(imm); break;
      default: unreachable<void>();
    }
  }
}

void
expr_f64(State& s, Prec prec, Ctx ctx)
{
  F64 f64;
  F64WithImm f64_with_imm;
  uint8_t imm;
  if (s.read.code(&f64, &f64_with_imm, &imm)) {
    switch (f64) {
      case F64::LitImm:   s.write.float64(s.read.fixed_width<double>()); break;
      case F64::LitPool:  s.write.float64(s.f64s()[s.read.imm_u32()]); break;
      case F64::GetLoc:   get_local(s); break;
      case F64::GetGlo:   get_global(s); break;
      case F64::SetLoc:   set_local<Type::F64>(s, prec); break;
      case F64::SetGlo:   set_global<Type::F64>(s, prec); break;
      case F64::Load:     load_f64(s, prec, ctx, false); break;
      case F64::LoadOff:  load_f64(s, prec, ctx, true); break;
      case F64::Store:    store_f64(s, prec, false); break;
      case F64::StoreOff: store_f64(s, prec, true); break;
      case F64::CallInt:  call_internal_f64(s, prec); break;
      case F64::CallInd:  call_indirect_f64(s, prec); break;
      case F64::CallImp:  call_import_f64(s, prec); break;
      case F64::Cond:     cond<RType::F64>(s, prec); break;
      case F64::Comma:    comma<RType::F64>(s, prec); break;
      case F64::FromS32:  pos_i32(s, prec, Signed); break;
      case F64::FromU32:  pos_i32(s, prec, Unsigned); break;
      case F64::FromF32:  pos_f32(s, prec); break;
      case F64::Neg:      prefix<RType::F64>(s, prec, ctx, Result::NoIsh, Ctx::Default, "-"); break;
      case F64::Add:      add_sub<RType::F64>(s, prec, ctx, '+'); break;
      case F64::Sub:      add_sub<RType::F64>(s, prec, ctx, '-'); break;
      case F64::Mul:      mul_div_mod_f64(s, prec, '*'); break;
      case F64::Div:      mul_div_mod_f64(s, prec, '/'); break;
      case F64::Mod:      mul_div_mod_f64(s, prec, '%'); break;
      case F64::Min:      min_max_f64(s, prec, StdLib::min); break;
      case F64::Max:      min_max_f64(s, prec, StdLib::max); break;
      case F64::Abs:      unary_stdlib_f64(s, prec, StdLib::abs); break;
      case F64::Ceil:     unary_stdlib_f64(s, prec, StdLib::ceil); break;
      case F64::Floor:    unary_stdlib_f64(s, prec, StdLib::floor); break;
      case F64::Sqrt:     unary_stdlib_f64(s, prec, StdLib::sqrt); break;
      case F64::Cos:      unary_stdlib_f64(s, prec, StdLib::cos); break;
      case F64::Sin:      unary_stdlib_f64(s, prec, StdLib::sin); break;
      case F64::Tan:      unary_stdlib_f64(s, prec, StdLib::tan); break;
      case F64::ACos:     unary_stdlib_f64(s, prec, StdLib::acos); break;
      case F64::ASin:     unary_stdlib_f64(s, prec, StdLib::asin); break;
      case F64::ATan:     unary_stdlib_f64(s, prec, StdLib::atan); break;
      case F64::ATan2:    binary_stdlib_f64(s, prec, StdLib::atan2); break;
      case F64::Exp:      unary_stdlib_f64(s, prec, StdLib::exp); break;
      case F64::Ln:       unary_stdlib_f64(s, prec, StdLib::log); break;
      case F64::Pow:      binary_stdlib_f64(s, prec, StdLib::pow); break;
      default: unreachable<void>();
    }
  } else {
    switch (f64_with_imm) {
      case F64WithImm::LitPool: s.write.float64(s.f64s()[imm]); break;
      case F64WithImm::GetLoc:  s.write.local_name(imm); break;
      default: unreachable<void>();
    }
  }
}

void
expr_void(State& s, Prec prec, Ctx ctx)
{
  assert(!need_paren(prec, Prec::Call));
  switch (s.read.void_expr()) {
    case Void::CallInt: call_internal_void(s); break;
    case Void::CallInd: call_indirect_void(s); break;
    case Void::CallImp: call_import_void(s); break;
    default: unreachable<void>();
  }
}

void stmt(State& s);

void
return_stmt(State& s)
{
  s.write.ascii("return");
  switch (s.cur_ret()) {
    case RType::I32: s.write.ascii(' '); signed_expr(s, Prec::Lowest, Signed); break;
    case RType::F32: s.write.ascii(' '); expr<RType::F32>(s, Prec::Lowest); break;
    case RType::F64: s.write.ascii(' '); expr<RType::F64>(s, Prec::Lowest); break;
    case RType::Void: break;
  }
  s.write.ascii(";\n");
}

void
stmt_list(State& s)
{
  uint32_t num_stmts = s.read.imm_u32();
  for (uint32_t i = 0; i < num_stmts; i++)
    stmt(s);
}

void
block_stmt(State& s)
{
  s.write.ascii("{\n");
  stmt_list(s);
  s.write.ascii("}\n");
}

void
if_stmt(State& s)
{
  s.write.ascii("if(");
  expr<RType::I32>(s, Prec::Lowest);
  s.write.ascii(')');
  stmt(s);
}


void
if_else_stmt(State& s)
{
  s.write.ascii("if(");
  expr<RType::I32>(s, Prec::Lowest);
  s.write.ascii(')');
  if (s.read.is_next_node_block()) {
    stmt(s);
  } else {
    s.write.ascii("{\n");
    stmt(s);
    s.write.ascii("}\n");
  }
  s.write.ascii("else ");
  stmt(s);
}

void
while_stmt(State& s)
{
  s.write.ascii("while(");
  expr<RType::I32>(s, Prec::Lowest);
  s.write.ascii(')');
  stmt(s);
}

void
do_stmt(State& s)
{
  s.write.ascii("do");
  if (s.read.is_next_node_block()) {
    stmt(s);
  } else {
    s.write.ascii("{\n");
    stmt(s);
    s.write.ascii("}\n");
  }
  s.write.ascii("while(");
  expr<RType::I32>(s, Prec::Lowest);
  s.write.ascii(");\n");
}

void
label_stmt(State& s)
{
  uint32_t label_index = s.push_label();
  s.write.label_name(label_index);
  s.write.ascii(':');
  stmt(s);
  s.pop_label();
}

void
break_stmt(State& s)
{
  s.write.ascii("break ");
  s.write.label_name(s.read.imm_u32());
  s.write.ascii(";\n");
}

void
continue_stmt(State& s)
{
  s.write.ascii("continue ");
  s.write.label_name(s.read.imm_u32());
  s.write.ascii(";\n");
}

void
switch_stmt(State& s)
{
  s.write.ascii("switch(");
  uint32_t num_cases = s.read.imm_u32();
  signed_expr(s, Prec::Lowest, Signed);
  s.write.ascii("){\n");

  for (uint32_t i = 0; i < num_cases; i++)
    switch (s.read.switch_case()) {
      case SwitchCase::Case0:
        s.write.ascii("case ");
        s.write.int32(s.read.imm_s32());
        s.write.ascii(":\n");
        break;
      case SwitchCase::Case1:
        s.write.ascii("case ");
        s.write.int32(s.read.imm_s32());
        s.write.ascii(":\n");
        stmt(s);
        break;
      case SwitchCase::CaseN:
        s.write.ascii("case ");
        s.write.int32(s.read.imm_s32());
        s.write.ascii(":\n");
        stmt_list(s);
        break;
      case SwitchCase::Default0:
        s.write.ascii("default:\n");
        break;
      case SwitchCase::Default1:
        s.write.ascii("default:\n");
        stmt(s);
        break;
      case SwitchCase::DefaultN:
        s.write.ascii("default:\n");
        stmt_list(s);
        break;
      default: abort();
    }

  s.write.ascii("}\n");
}

void
stmt(State& s)
{
  Stmt stmt;
  StmtWithImm stmt_with_imm;
  uint8_t imm;
  if (s.read.code(&stmt, &stmt_with_imm, &imm)) {
    switch (stmt) {
      case Stmt::SetLoc: set_local(s, Prec::Lowest); s.write.ascii(";\n"); break;
      case Stmt::SetGlo: set_global(s, Prec::Lowest); s.write.ascii(";\n"); break;
      case Stmt::I32Store8: store_i8(s, Prec::Lowest, false); s.write.ascii(";\n"); break;
      case Stmt::I32StoreOff8: store_i8(s, Prec::Lowest, true); s.write.ascii(";\n"); break;
      case Stmt::I32Store16: store_i16(s, Prec::Lowest, false); s.write.ascii(";\n"); break;
      case Stmt::I32StoreOff16: store_i16(s, Prec::Lowest, true); s.write.ascii(";\n"); break;
      case Stmt::I32Store32: store_i32(s, Prec::Lowest, false); s.write.ascii(";\n"); break;
      case Stmt::I32StoreOff32: store_i32(s, Prec::Lowest, true); s.write.ascii(";\n"); break;
      case Stmt::F32Store: store_f32(s, Prec::Lowest, false); s.write.ascii(";\n"); break;
      case Stmt::F32StoreOff: store_f32(s, Prec::Lowest, true); s.write.ascii(";\n"); break;
      case Stmt::F64Store: store_f64(s, Prec::Lowest, false); s.write.ascii(";\n"); break;
      case Stmt::F64StoreOff: store_f64(s, Prec::Lowest, true); s.write.ascii(";\n"); break;
      case Stmt::CallInt: call_internal(s, Prec::Lowest); s.write.ascii(";\n"); break;
      case Stmt::CallInd: call_indirect(s, Prec::Lowest); s.write.ascii(";\n"); break;
      case Stmt::CallImp: call_import(s, Prec::Lowest); s.write.ascii(";\n"); break;
      case Stmt::Ret: return_stmt(s); return;
      case Stmt::Block: block_stmt(s); return;
      case Stmt::IfThen: if_stmt(s); return;
      case Stmt::IfElse: if_else_stmt(s); return;
      case Stmt::While: while_stmt(s); return;
      case Stmt::Do: do_stmt(s); return;
      case Stmt::Label: label_stmt(s); return;
      case Stmt::Break: s.write.ascii("break;\n"); return;
      case Stmt::Continue: s.write.ascii("continue;\n"); return;
      case Stmt::BreakLabel: break_stmt(s); return;
      case Stmt::ContinueLabel: continue_stmt(s); return;
      case Stmt::Switch: switch_stmt(s); return;
      default: unreachable<void>();
    }
  } else {
    switch (stmt_with_imm) {
      case StmtWithImm::SetLoc: set_local(s, Prec::Lowest, imm); s.write.ascii(";\n"); break;
      case StmtWithImm::SetGlo: set_global(s, Prec::Lowest, imm); s.write.ascii(";\n"); break;
      default: unreachable<void>();
    }
  }
}

void
args(State& s, const Signature& sig)
{
  size_t num_args = sig.args.size();
  if (num_args > 0) {
    for (uint32_t arg_index = 0;;) {
      s.write.local_name(arg_index);
      if (++arg_index == num_args)
        break;
      else
        s.write.ascii(',');
    }
  }
  s.write.ascii("){\n");
  if (num_args > 0) {
    for (uint32_t arg_index = 0; arg_index < num_args; arg_index++) {
      Type type = sig.args[arg_index];
      s.write.local_name(arg_index);
      s.write.ascii('=');
      s.cur_local_types().push_back(type);
      switch (type) {
        case Type::I32:
          s.write.local_name(arg_index);
          s.write.ascii("|0");
          break;
        case Type::F32:
          s.write.name(HotStdLib::FRound);
          s.write.ascii('(');
          s.write.local_name(arg_index);
          s.write.ascii(')');
          break;
        case Type::F64:
          s.write.ascii('+');
          s.write.local_name(arg_index);
          break;
        default: abort();
      }
      s.write.ascii(';');
    }
    s.write.ascii('\n');
  }
}

void
vars(State& s)
{
  uint32_t num_i32_vars = 0;
  uint32_t num_f32_vars = 0;
  uint32_t num_f64_vars = 0;

  VarTypes var_types;
  VarTypesWithImm var_types_with_imm;
  uint8_t imm;
  if (s.read.code(&var_types, &var_types_with_imm, &imm)) {
    if (var_types & VarTypes::I32)
      num_i32_vars = s.read.imm_u32();
    if (var_types & VarTypes::F32)
      num_f32_vars = s.read.imm_u32();
    if (var_types & VarTypes::F64)
      num_f64_vars = s.read.imm_u32();
  } else {
    num_i32_vars = imm;
  }

  uint32_t num_vars = num_i32_vars + num_f32_vars + num_f64_vars;
  if (num_vars > 0) {
    s.write.ascii("var ");
    uint32_t local_index = s.cur_local_types().size();
    uint32_t num_locals = local_index + num_vars;
    for (uint32_t i = 0; i < num_i32_vars; i++) {
      s.write.local_name(local_index++);
      s.cur_local_types().push_back(Type::I32);
      s.write.ascii("=0");
      if (local_index != num_locals)
        s.write.ascii(',');
    }
    for (uint32_t i = 0; i < num_f32_vars; i++) {
      s.write.local_name(local_index++);
      s.cur_local_types().push_back(Type::F32);
      s.write.ascii('=');
      s.write.name(HotStdLib::FRound);
      s.write.ascii("(0)");
      if (local_index != num_locals)
        s.write.ascii(',');
    }
    for (uint32_t i = 0; i < num_f64_vars; i++) {
      s.write.local_name(local_index++);
      s.cur_local_types().push_back(Type::F64);
      s.write.ascii("=0.");
      if (local_index != num_locals)
        s.write.ascii(',');
    }
    s.write.ascii(";\n");
  }
}

void
function_definition(State& s, size_t func_index)
{
  s.write.ascii("function ");
  s.write_func_name(func_index);
  s.write.ascii('(');

  const Signature& sig = s.func_sig(func_index);
  s.set_cur_ret(sig.ret);
  s.cur_local_types().clear();
  args(s, sig);
  vars(s);

  stmt_list(s);

  s.write.ascii("}\n");
}

void
function_definition_section(State& s)
{
  uint32_t num_funcs = s.num_funcs();
  for (uint32_t func_index = 0; func_index < num_funcs; func_index++)
    function_definition(s, func_index);
}

void
write_function_pointer_tables(State& s)
{
  for (uint32_t func_ptr_tbl_i = 0; func_ptr_tbl_i < s.num_func_ptr_tables(); func_ptr_tbl_i++) {
    s.write.ascii("var ");
    s.write_func_ptr_table_name(func_ptr_tbl_i);
    s.write.ascii("=[");
    auto& func_ptr_table = s.func_ptr_table(func_ptr_tbl_i);
    for (uint32_t elem_i = 0, num_elems = func_ptr_table.elems.size();;) {
      s.write_func_name(func_ptr_table.elems[elem_i]);
      if (++elem_i == num_elems)
        break;
      s.write.ascii(',');
    }
    s.write.ascii("];\n");
  }
}

void
export_section(State& s)
{
  switch (s.read.export_format()) {
    case ExportFormat::Default:
      s.write.ascii("return ");
      s.write_func_name(s.read.imm_u32());
      s.write.ascii(";\n");
      break;
    case ExportFormat::Record:
      s.write.ascii("return {");
      if (uint32_t num_exports = s.read.imm_u32()) {
        for (uint32_t export_index = 0;;) {
          while (char c = s.read.single_char())
            s.write.ascii(c);
          s.write.ascii(':');
          s.write_func_name(s.read.imm_u32());
          if (++export_index == num_exports)
            break;
          else
            s.write.ascii(',');
        }
      }
      s.write.ascii("};\n");
      break;
    default:
      abort();
  }
}

template <unsigned N>
void
import_stdlib(State& s, StdLib global, const char (&import)[N])
{
  s.write.ascii("var ");
  s.write.name(global);
  s.write.ascii('=');
  s.write.name(StdLib::stdlib);
  s.write.ascii('.');
  s.write.ascii(import);
  s.write.ascii(";\n");
}

template <unsigned N>
void
import_heap(State& s, HotStdLib global, const char (&import)[N])
{
  s.write.ascii("var ");
  s.write.name(global);
  s.write.ascii("=new ");
  s.write.name(StdLib::stdlib);
  s.write.ascii(".");
  s.write.ascii(import);
  s.write.ascii("(");
  s.write.name(StdLib::buffer);
  s.write.ascii(");\n");
}

template <unsigned N>
void
import_hot_stdlib(State& s, HotStdLib global, const char (&import)[N])
{
  s.write.ascii("var ");
  s.write.name(global);
  s.write.ascii('=');
  s.write.name(StdLib::stdlib);
  s.write.ascii(".");
  s.write.ascii(import);
  s.write.ascii(";\n");
}

void
unpack(State& s, const char* cb_name)
{
  if (cb_name) {
    s.write.dynamic_ascii(cb_name);
    s.write.ascii('(');
  }
  s.write.ascii("function asmModule(");
  s.write.name(StdLib::stdlib);
  s.write.ascii(',');
  s.write.name(StdLib::foreign);
  s.write.ascii(',');
  s.write.name(StdLib::buffer);
  s.write.ascii("){'use asm';\n");
  import_heap(s, HotStdLib::HeapS8, "Int8Array");
  import_heap(s, HotStdLib::HeapU8, "Uint8Array");
  import_heap(s, HotStdLib::HeapS16, "Int16Array");
  import_heap(s, HotStdLib::HeapU16, "Uint16Array");
  import_heap(s, HotStdLib::HeapS32, "Int32Array");
  import_heap(s, HotStdLib::HeapU32, "Uint32Array");
  import_heap(s, HotStdLib::HeapF32, "Float32Array");
  import_heap(s, HotStdLib::HeapF64, "Float64Array");
  import_hot_stdlib(s, HotStdLib::IMul, "Math.imul");
  import_hot_stdlib(s, HotStdLib::FRound, "Math.fround");
  import_stdlib(s, StdLib::acos, "Math.acos");
  import_stdlib(s, StdLib::asin, "Math.asin");
  import_stdlib(s, StdLib::atan, "Math.atan");
  import_stdlib(s, StdLib::cos, "Math.cos");
  import_stdlib(s, StdLib::sin, "Math.sin");
  import_stdlib(s, StdLib::tan, "Math.tan");
  import_stdlib(s, StdLib::exp, "Math.exp");
  import_stdlib(s, StdLib::log, "Math.log");
  import_stdlib(s, StdLib::ceil, "Math.ceil");
  import_stdlib(s, StdLib::floor, "Math.floor");
  import_stdlib(s, StdLib::sqrt, "Math.sqrt");
  import_stdlib(s, StdLib::abs, "Math.abs");
  import_stdlib(s, StdLib::min, "Math.min");
  import_stdlib(s, StdLib::max, "Math.max");
  import_stdlib(s, StdLib::atan2, "Math.atan2");
  import_stdlib(s, StdLib::pow, "Math.pow");
  import_stdlib(s, StdLib::clz32, "Math.clz32");
  import_stdlib(s, StdLib::NaN, "NaN");
  import_stdlib(s, StdLib::Infinity, "Infinity");

  constant_pool_section(s);
  signature_section(s);
  function_import_section(s);
  global_section(s);
  function_declaration_section(s);
  read_function_pointer_tables(s);
  function_definition_section(s);
  write_function_pointer_tables(s);
  export_section(s);

  s.write.ascii('}');
  if (cb_name)
    s.write.ascii(')');
  s.write.ascii('\n');
}

}  // namespace asmjs

#ifdef CHECKED_OUTPUT_SIZE

uint32_t
asmjs::calculate_unpacked_size(const uint8_t* packed)
{
  State s(packed);
  unpack(s, nullptr);
  return s.write.finish();
}

#else

bool
asmjs::has_magic_number(const uint8_t* packed)
{
  In in(packed);
  return in.fixed_width<uint32_t>() == MagicNumber;
}

uint32_t
asmjs::unpacked_size(const uint8_t* packed, const char* cb_name)
{
  In in(packed);
  if (in.fixed_width<uint32_t>() != MagicNumber)
    abort();
  return in.fixed_width<uint32_t>() + cb_name_len(cb_name);
}

void
asmjs::unpack(const uint8_t* packed, const char* cb_name, uint32_t unpacked_size, uint8_t* unpacked)
{
  State s(packed, cb_name, unpacked_size, unpacked);
  unpack(s, cb_name);
  s.write.finish();
}

#endif

#if defined(__EMSCRIPTEN__) && !defined(CHECKED_OUTPUT_SIZE)
extern "C" {

bool EMSCRIPTEN_KEEPALIVE
asmjs_has_magic_number(const uint8_t* packed)
{
  return asmjs::has_magic_number(packed);
}

uint32_t EMSCRIPTEN_KEEPALIVE
asmjs_unpacked_size(const uint8_t* packed, const char* cb_name)
{
  return asmjs::unpacked_size(packed, cb_name);
}

void EMSCRIPTEN_KEEPALIVE
asmjs_unpack(const uint8_t* packed, const char* cb_name, uint32_t unpacked_size, uint8_t* unpacked)
{
  asmjs::unpack(packed, cb_name, unpacked_size, unpacked);
}

// Temporary workaround until Emscripten has no-exception-handling libstdc++ to avoid pulling in
// all of iostream/locales/string.
void
_ZNKSt3__120__vector_base_commonILb1EE20__throw_length_errorEv()
{}

}  // extern "C"
#endif
