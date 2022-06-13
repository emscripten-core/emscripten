/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <emscripten/val.h>

#include <array>
#include <assert.h>
#include <string>
#include <type_traits>
#include <vector>

extern "C" {
void ValHelper_JS(emscripten::EM_VAL h, const void* ptr, int size);
#ifdef __PERF_VAL_HELPER
void ValHelperPerf_AuditBufFull(int c, int n, emscripten::EM_VAL h);
#endif
}

// ValHelper caches changes and commit them bulkly to the binding val.
//
// Why is fast?
//  1) saving context switches
//  2) good locality
//  3) no extra copies
//
// Usage:
//          ValHelper vh(OBJECT);
//          vh.set("k1", 1);
//          vh.set("k3", "hello");
//          vh.set("k2", std::vector<float>{1,2,3,4,5,6,7,8,9});
//          vh.finalize();
//
// Caveats:
//  1) For sub-scoped objects set/added, preferring finalize inside, or use
//     std::move if only few ones(1 or 2).
//          ValHelper vh(ARRAY);
//          if (condition) {
//              std::string temp;
//              generate_string(&temp);
//              vh.add(std::move(temp));
//          }
//  2) When you passed ValHelper as argument, always do finalize() before
//     leaving the scope as cached pointers may be stale immediately.
//
namespace emscripten {

enum class TYPE : uint8_t {
    NONE = 0,
    INT32,
    UINT32,
    FLOAT32,
    FLOAT64,
    BOOL,
    STRING,
    U8STRING,
    EMVAL,
    ARRAY,
    MAX
};

enum EmValType {
    OBJECT = 0,  // Default
    ARRAY,
};

template<typename T>
struct map { static constexpr TYPE type = TYPE::NONE; };
template<> struct map<int> { static constexpr TYPE type = TYPE::INT32; };
template<> struct map<uint32_t> { static constexpr TYPE type = TYPE::UINT32; };
template<> struct map<float> { static constexpr TYPE type = TYPE::FLOAT32; };
template<> struct map<double> { static constexpr TYPE type = TYPE::FLOAT64; };
template<> struct map<bool> { static constexpr TYPE type = TYPE::BOOL; };

constexpr bool is_supported_array_type(TYPE t) {
  return (t == TYPE::INT32 || t == TYPE::UINT32 || t == TYPE::FLOAT32 ||
          t == TYPE::FLOAT64 || t == TYPE::BOOL);
}

template<typename>
struct is_std_vector : std::false_type {};

template<typename T, typename... TS>
struct is_std_vector<std::vector<T, TS...>> : std::true_type {};

template<typename>
struct is_std_array : std::false_type {};

template<typename T, size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
constexpr bool is_supported_rvalue_type() {
  return std::is_same<T, std::string>::value ||
         std::is_same<T, emscripten::val>::value ||
         is_std_vector<T>::value ||
         is_std_array<T>::value;
}

// We expect lvalue reference or literals.  For rvalues other than explicitly
// supported (which is cached or commited instantly), it fails the compiling.
template <typename V>
static constexpr bool check_arg_pointer_wont_dangle() {
  static_assert(
      (is_supported_rvalue_type<typename std::remove_cv<
           typename std::remove_reference<V>::type>::type>() ||
       std::is_lvalue_reference<V>::value || !std::is_class<V>::value),
      "Expect lvalue or literal, but NO rvalue.");
  return true;
}

namespace internal {

template<typename T, typename VH>
void AddArraySpan(const T* start, size_t n, VH* h, bool commit_now) {
  if constexpr (is_supported_array_type(map<T>::type)) {
    // much more efficient way.
    h->add_array(start, n, map<T>::type);
    if (commit_now)
      h->finalize();
  } else {
    VH tp(ARRAY);
    tp.add(start, start + n);
    h->add(tp);  // always commit
  }
}

template<typename T, typename VH>
void ConcatArraySpan(const T* start, size_t n, VH* h, bool commit_now) {
  if constexpr (is_supported_array_type(map<T>::type)) {
    // much more efficient way.
    h->concat_array(start, n, map<T>::type);
  } else {
    h->add(start, start + n);
  }
  if (commit_now) h->finalize();
}

}  // namespace internal

using emscripten::val;
template <size_t N = 20 /*BUFF_SIZE*/ >
class ValHelper {
 public:
  ValHelper() : ValHelper(OBJECT) {}
  ~ValHelper() = default;

  // disallow copy & move operations.
  ValHelper(const ValHelper&) = delete;
  ValHelper& operator=(const ValHelper&) = delete;

  explicit ValHelper(const val& v) : val_(v) {}
  explicit ValHelper(EmValType t)
    : val_(t == ARRAY ? val::array() : val::object()) {}

  template <typename V>
  void set(const val& k, V&& v) {
    if constexpr (check_arg_pointer_wont_dangle<V>()) {}

    cursor_->key = (const char*)k.as_handle();
    cursor_->key_flag = FLAG_VAL_KEY;
    add(std::forward<V>(v));
  }

  template <typename V>
  void set(const char* k, V&& v) {
    if constexpr (check_arg_pointer_wont_dangle<V>()) {}

    cursor_->key = k;
    cursor_->key_flag = 0;
    add(std::forward<V>(v));
  }

  void add(int v) {
    cursor_->type = TYPE::INT32;
    cursor_->value.w[0].i = v;
    advance_and_may_finalize();
  }
  void add(size_t v) {
    return add(static_cast<unsigned int>(v));
  }
  void add(unsigned int v) {
    cursor_->type = TYPE::UINT32;
    cursor_->value.w[0].u = v;
    advance_and_may_finalize();
  }
  void add(float v) {
    cursor_->type = TYPE::FLOAT32;
    cursor_->value.w[0].f = v;
    advance_and_may_finalize();
  }
  void add(double v) {
    cursor_->type = TYPE::FLOAT64;
    cursor_->value.d = v;
    advance_and_may_finalize();
  }
  void add(bool v) {
    cursor_->type = TYPE::BOOL;
    cursor_->value.b = v;
    advance_and_may_finalize();
  }

  void add(const char* v) {
    cursor_->type = TYPE::STRING;
    cursor_->value.w[0].s = v;
    advance_and_may_finalize();
  }
  void add_u8(const char* v) {
    cursor_->type = TYPE::U8STRING;
    cursor_->value.w[0].s = v;
    advance_and_may_finalize();
  }
  void add(const std::string& v) {
    add_u8(v.c_str());
  }
  void add(std::string&& v) {
    auto cached = std::make_unique<std::string>(std::move(v));
    add_u8(cached->data());
    if (!cached_strings_.capacity()) cached_strings_.reserve(4);
    cached_strings_.push_back(std::move(cached));
  }

  // accept |ValHelper| but only lvalue reference
  template<size_t SZ>
  void add(ValHelper<SZ>& vh) {
    add(vh.toval());
  }
  void add(const val& v) {
    cursor_->type = TYPE::EMVAL;
    cursor_->value.w[0].v = v.as_handle();
    advance_and_may_finalize();
  }
  void add(val&& v) {
    if (!cached_vals_.capacity()) cached_vals_.reserve(4);
    cached_vals_.push_back(std::move(v));  // efficient op
    add(cached_vals_.back());
  }

  template <typename Iter>
  void add(Iter begin, Iter end) {
    for (auto it = begin; it != end; ++it)
      add(*it);
  }

  // Using explicit type
  void add_array(const void* a, uint16_t n, TYPE t = TYPE::INT32) {
    cursor_->type = TYPE::ARRAY;
    cursor_->value.w[0].addr = a;
    cursor_->value.w[1].item.t = (uint8_t)t;
    cursor_->value.w[1].item.f = 0;
    cursor_->value.w[1].item.n = n;
    advance_and_may_finalize();
  }

  template <typename T>
  void add(const std::vector<T>& v) {
    internal::AddArraySpan(v.data(), v.size(), this, false);
  }
  template <typename T>
  void add(std::vector<T>&& v) {
    internal::AddArraySpan(v.data(), v.size(), this, true);
  }
  template <typename T, size_t SIZE>
  void add(const std::array<T, SIZE>& a) {
    internal::AddArraySpan(a.data(), a.size(), this, false);
  }
  template <typename T, size_t SIZE>
  void add(std::array<T, SIZE>&& a) {
    internal::AddArraySpan(a.data(), a.size(), this, true);
  }
  template <typename T, size_t SIZE>
  void add(const T(&a)[SIZE]) {
    internal::AddArraySpan(a, SIZE, this, false);
  }

  void concat_array(const void* a, uint16_t n, TYPE t = TYPE::INT32) {
    cursor_->type = TYPE::ARRAY;
    cursor_->value.w[0].addr = a;
    cursor_->value.w[1].item.t = (uint8_t)t;
    cursor_->value.w[1].item.f = FLAG_CONCAT;
    cursor_->value.w[1].item.n = n;
    advance_and_may_finalize();
  }

  // Use only for ARRAY type
  template <typename T>
  void concat(const std::vector<T>& v) {
    internal::ConcatArraySpan(v.data(), v.size(), this, false);
  }
  template <typename T>
  void concat(std::vector<T>&& v) {
    internal::ConcatArraySpan(v.data(), v.size(), this, true);
  }
  template <typename T, size_t SIZE>
  void concat(const std::array<T, SIZE>& a) {
    internal::ConcatArraySpan(a.data(), a.size(), this, false);
  }
  template <typename T, size_t SIZE>
  void concat(std::array<T, SIZE>&& a) {
    internal::ConcatArraySpan(a.data(), a.size(), this, true);
  }
  template <typename T, size_t SIZE>
  void concat(const T(&a)[SIZE]) {
    internal::ConcatArraySpan(a, SIZE, this, false);
  }

  void finalize() {
    ValHelper_JS(val_.as_handle(), buffer(), size());
    reset();
  }

  size_t size() const { return cursor_ - items_.data(); }
  bool empty() const { return cursor_ == items_.data(); }

  const val& get_val() const { return val_; }

  val toval() {
    if (!empty()) finalize();
    return val_;
  }

  void reset_val() {
    assert(empty());
    val_ = val_.isArray() ? val::array() : val::object();
  }

  void attach(const val& v) {
    assert(empty());
    val_ = v;
  }

 private:
  const void* buffer() const { return items_.data(); }

  void reset() {
    cursor_ = items_.data();
    cached_vals_.clear();
    cached_strings_.clear();
  }

  void advance_and_may_finalize() {
    cursor_++;
    if (size() >= N) {
      finalize();

#ifdef __PERF_VAL_HELPER
      static uint32_t count = 0;
      ValHelperPerf_AuditBufFull(++count, N, val_.as_handle());
#endif
    }
  }

  enum { FLAG_CONCAT = 1, FLAG_VAL_KEY = 2 };

  // |Entry| is 16B in length, keep it a POD.
  struct Entry {
    TYPE type;  // uint8_t
    uint8_t key_flag;  // key is emscripten::val

    const char* key;  // key string for OBJECT
    union {
      union {
        int32_t i;
        uint32_t u;
        float f;
        const char* s;
        emscripten::EM_VAL v;
        const void* addr;
        struct {
          uint8_t t;  // TYPE still
          uint8_t f;  // FLAG_CONCAT
          uint16_t n;
        } item;
      } w[2];
      double d;
      uint64_t u;  // not now
      bool b;
    } value;
  };

  static_assert(alignof(Entry) == 8, "Entry must be 8B aligned");
  static_assert(sizeof(Entry) == 16, "Entry must be 16B in length");

  // Use fixed-length array instead of vector, as it's used as a buffer.
  // We will `flush` the buffer on the fly when it got full.
  std::array<Entry, N> items_;

  // Current slot to use
  Entry* cursor_ = items_.data();

  // Binding val
  val val_ = val::null();

  // Transient storage for passed temporary/rvalue arguments.
  std::vector<val> cached_vals_;

  // In most case we don't use cached buffer, so std::array seems not worthy.
  std::vector<std::unique_ptr<std::string>> cached_strings_;
};

}  // namespace emscripten
