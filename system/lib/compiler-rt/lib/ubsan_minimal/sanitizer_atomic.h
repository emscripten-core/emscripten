/**
 * This file is based on sanitizer_atomic.h, sanitizer_atomic_clang.h, and
 * sanitizer_atomic_clang_other.h from compiler-rt.
 * Last changed revision: 351636.
 * Last changed date: 2019-01-19.
 *
 * The files are combined into one to avoid depending on many unneeded headers
 * in compiler-rt/lib/sanitizer_common.
 */

//===-- sanitizer_atomic.h --------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a part of ThreadSanitizer/AddressSanitizer runtime.
//
//===----------------------------------------------------------------------===//

#ifndef SANITIZER_ATOMIC_H
#define SANITIZER_ATOMIC_H

#include <cinttypes>

#ifndef INLINE
#define INLINE inline
#endif
#define ALIGNED(x) __attribute__((aligned(x)))
#define DCHECK(a)

namespace __sanitizer {

enum memory_order {
  memory_order_relaxed = 1 << 0,
  memory_order_seq_cst = 1 << 5
};

struct atomic_uint8_t {
  typedef std::uint8_t Type;
  volatile Type val_dont_use;
};

struct atomic_uint16_t {
  typedef std::uint16_t Type;
  volatile Type val_dont_use;
};

struct atomic_sint32_t {
  typedef std::int32_t Type;
  volatile Type val_dont_use;
};

struct atomic_uint32_t {
  typedef std::uint32_t Type;
  volatile Type val_dont_use;
};

struct atomic_uint64_t {
  typedef std::uint64_t Type;
  // On 32-bit platforms u64 is not necessary aligned on 8 bytes.
  volatile ALIGNED(8) Type val_dont_use;
};

struct atomic_uintptr_t {
  typedef std::uintptr_t Type;
  volatile Type val_dont_use;
};

typedef std::uintptr_t uptr;

template<typename T>
INLINE typename T::Type atomic_load(
    const volatile T *a, memory_order mo) {
  DCHECK(mo & memory_order_relaxed);
  DCHECK(!((uptr)a % sizeof(*a)));
  typename T::Type v;

  if (sizeof(*a) < 8 || sizeof(void*) == 8) {
    // Assume that aligned loads are atomic.
    v = a->val_dont_use;
  } else {
    // 64-bit load on 32-bit platform.
    // Gross, but simple and reliable.
    // Assume that it is not in read-only memory.
    v = __sync_fetch_and_add(
        const_cast<typename T::Type volatile *>(&a->val_dont_use), 0);
  }
  return v;
}

template<typename T>
INLINE void atomic_store(volatile T *a, typename T::Type v, memory_order mo) {
  DCHECK(mo & memory_order_relaxed);
  DCHECK(!((uptr)a % sizeof(*a)));

  if (sizeof(*a) < 8 || sizeof(void*) == 8) {
    // Assume that aligned loads are atomic.
    a->val_dont_use = v;
  } else {
    // 64-bit store on 32-bit platform.
    // Gross, but simple and reliable.
    typename T::Type cmp = a->val_dont_use;
    typename T::Type cur;
    for (;;) {
      cur = __sync_val_compare_and_swap(&a->val_dont_use, cmp, v);
      if (cur == cmp || cur == v)
        break;
      cmp = cur;
    }
  }
}

// We would like to just use compiler builtin atomic operations
// for loads and stores, but they are mostly broken in clang:
// - they lead to vastly inefficient code generation
// (http://llvm.org/bugs/show_bug.cgi?id=17281)
// - 64-bit atomic operations are not implemented on x86_32
// (http://llvm.org/bugs/show_bug.cgi?id=15034)
// - they are not implemented on ARM
// error: undefined reference to '__atomic_load_4'

// See http://www.cl.cam.ac.uk/~pes20/cpp/cpp0xmappings.html
// for mappings of the memory model to different processors.

template <typename T>
INLINE bool atomic_compare_exchange_strong(volatile T *a, typename T::Type *cmp,
                                           typename T::Type xchg,
                                           memory_order mo) {
  typedef typename T::Type Type;
  Type cmpv = *cmp;
  Type prev;
  prev = __sync_val_compare_and_swap(&a->val_dont_use, cmpv, xchg);
  if (prev == cmpv) return true;
  *cmp = prev;
  return false;
}

// Clutter-reducing helpers.

template<typename T>
INLINE typename T::Type atomic_load_relaxed(const volatile T *a) {
  return atomic_load(a, memory_order_relaxed);
}

template<typename T>
INLINE void atomic_store_relaxed(volatile T *a, typename T::Type v) {
  atomic_store(a, v, memory_order_relaxed);
}

}  // namespace __sanitizer

#endif  // SANITIZER_ATOMIC_H
