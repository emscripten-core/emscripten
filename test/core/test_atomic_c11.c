// Copyright 2026 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

//------------------------------------------------------------------------------
//  test C11 atomics
//
//  This is a trasnliteration of the test_atomic_cxx.cpp test into C11 atomics.
//------------------------------------------------------------------------------
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>

#define TEST(TYPE, UNSIGNED_TYPE, MASK0, MASK1, MASK2)                         \
  {                                                                            \
    const TYPE numMemoryOrders = 6;                                            \
    memory_order memoryOrder[] = {                                             \
      memory_order_relaxed,                                                    \
      memory_order_consume,                                                    \
      memory_order_acquire,                                                    \
      memory_order_release,                                                    \
      memory_order_acq_rel,                                                    \
      memory_order_seq_cst,                                                    \
    };                                                                         \
                                                                               \
    _Atomic TYPE atomicDog = 5;                                                \
    printf("is_lock_free: %s\n",                                               \
           atomic_is_lock_free(&atomicDog) ? "true" : "false");                \
    printf("value: %lld\n", (long long)(TYPE)atomicDog);                       \
                                                                               \
    for (TYPE i = 0; i < numMemoryOrders; i++) {                               \
      atomic_store_explicit(&atomicDog, i, memoryOrder[i]);                    \
      printf("store/load %lld: %lld\n",                                        \
             (long long)i,                                                     \
             (long long)atomic_load_explicit(&atomicDog, memoryOrder[i]));     \
    }                                                                          \
                                                                               \
    for (TYPE i = 0; i < numMemoryOrders; i++) {                               \
      TYPE old = atomic_exchange_explicit(&atomicDog, i, memoryOrder[i]);      \
      printf("exchange %lld: old=%lld new=%lld\n",                             \
             (long long)i,                                                     \
             (long long)old,                                                   \
             (long long)(TYPE)atomicDog);                                      \
    }                                                                          \
                                                                               \
    for (TYPE i = 0; i < numMemoryOrders; i++) {                               \
      TYPE expected = i;                                                       \
      bool success = atomic_compare_exchange_weak_explicit(                    \
        &atomicDog, &expected, i + 1, memoryOrder[i], memoryOrder[i]);         \
      i = expected;                                                            \
      printf("compare_exchange_weak %lld: success = %s\n",                     \
             (long long)i,                                                     \
             success ? "true" : "false");                                      \
    }                                                                          \
                                                                               \
    for (TYPE i = 0; i < numMemoryOrders; i++) {                               \
      TYPE expected = i;                                                       \
      bool success = atomic_compare_exchange_strong_explicit(                  \
        &atomicDog, &expected, i + 1, memoryOrder[i], memoryOrder[i]);         \
      i = expected;                                                            \
      printf("compare_exchange_strong %lld: success = %s\n",                   \
             (long long)i,                                                     \
             success ? "true" : "false");                                      \
    }                                                                          \
                                                                               \
    atomicDog = MASK2;                                                         \
    for (TYPE i = 0; i < numMemoryOrders; i++) {                               \
      TYPE old = atomic_fetch_add_explicit(&atomicDog, 1, memoryOrder[i]);     \
      printf("fetch_add %lld: old=%llx new=%llx\n",                            \
             (long long)i,                                                     \
             (long long)old,                                                   \
             (long long)(TYPE)atomicDog);                                      \
    }                                                                          \
                                                                               \
    for (TYPE i = 0; i < numMemoryOrders; i++) {                               \
      TYPE old = atomic_fetch_sub_explicit(&atomicDog, 1, memoryOrder[i]);     \
      printf("fetch_sub %lld: old=%llx new=%llx\n",                            \
             (long long)i,                                                     \
             (long long)old,                                                   \
             (long long)(TYPE)atomicDog);                                      \
    }                                                                          \
                                                                               \
    for (TYPE i = 0; i < numMemoryOrders; i++) {                               \
      atomic_store_explicit(&atomicDog, MASK0, memoryOrder[i]);                \
      TYPE old =                                                               \
        atomic_fetch_and_explicit(&atomicDog, (TYPE)1 << i, memoryOrder[i]);   \
      printf("fetch_and %lld: old=%llx, new=%llx\n",                           \
             (long long)i,                                                     \
             (unsigned long long)(UNSIGNED_TYPE)old,                           \
             (unsigned long long)(UNSIGNED_TYPE)atomicDog);                    \
    }                                                                          \
                                                                               \
    atomicDog = 0;                                                             \
    for (TYPE i = 0; i < numMemoryOrders; i++) {                               \
      TYPE old =                                                               \
        atomic_fetch_or_explicit(&atomicDog, (TYPE)1 << i, memoryOrder[i]);    \
      printf("fetch_or %lld: old=%llx, new=%llx\n",                            \
             (long long)i,                                                     \
             (unsigned long long)(UNSIGNED_TYPE)old,                           \
             (unsigned long long)(UNSIGNED_TYPE)atomicDog);                    \
    }                                                                          \
                                                                               \
    atomicDog = 0;                                                             \
    for (TYPE i = 0; i < numMemoryOrders; i++) {                               \
      TYPE old =                                                               \
        atomic_fetch_xor_explicit(&atomicDog, (TYPE)1 << i, memoryOrder[i]);   \
      printf("fetch_xor %lld: old=%llx, new=%llx\n",                           \
             (long long)i,                                                     \
             (unsigned long long)(UNSIGNED_TYPE)old,                           \
             (unsigned long long)(UNSIGNED_TYPE)atomicDog);                    \
    }                                                                          \
                                                                               \
    atomicDog = 0;                                                             \
    atomicDog++;                                                               \
    printf("operator++: %lld\n", (long long)(TYPE)(atomicDog));                \
    atomicDog--;                                                               \
    printf("operator--: %lld\n", (long long)(TYPE)(atomicDog));                \
                                                                               \
    atomicDog += 10;                                                           \
    printf("operator+=: %lld\n", (long long)(TYPE)(atomicDog));                \
    atomicDog -= 5;                                                            \
    printf("operator-=: %lld\n", (long long)(TYPE)(atomicDog));                \
    atomicDog |= MASK0;                                                        \
    printf("operator|=: %llx\n",                                               \
           (unsigned long long)(UNSIGNED_TYPE)(atomicDog));                    \
    atomicDog &= MASK1;                                                        \
    printf("operator&=: %llx\n",                                               \
           (unsigned long long)(UNSIGNED_TYPE)(atomicDog));                    \
    atomicDog ^= MASK2;                                                        \
    printf("operator^=: %llx\n",                                               \
           (unsigned long long)(UNSIGNED_TYPE)(atomicDog));                    \
  }

typedef struct Pair128 {
  uint64_t m1;
  uint64_t m2;
} Pair128;

int main() {
  // test 8, 16, 32 and 64-bit data types
  printf("\n8 bits\n\n");
  TEST(char, unsigned char, 0xFF, 0xF0, 0x0F);
  printf("\n16 bits\n\n");
  TEST(short, unsigned short, 0xFFFF, 0xF0F0, 0x0F0F);
  printf("\n32 bits\n\n");
  TEST(int, unsigned int, 0xFFFFFFFF, 0xF0F0F0F0, 0x0F0F0F0F);
  printf("\n64 bits\n\n");
  TEST(long long, unsigned long long, 0xFFFFFFFFFFFFFFFF,
       0xF0F0F0F0F0F0F0F0, 0x0F0F0F0F0F0F0F0F);

  printf("\n128 bits\n\n");
  _Atomic Pair128 atomicPair;
  printf("is_lock_free: %s\n", atomic_is_lock_free(&atomicPair) ? "true" : "false");
  atomicPair = (Pair128){1, 2};
  Pair128 newPair = (Pair128){3, 4};
  Pair128 oldPair = atomic_exchange(&atomicPair, newPair);
  printf("exchange: %lld:%lld -> %lld:%lld\n\n", oldPair.m1, oldPair.m2, atomic_load(&atomicPair).m1, atomic_load(&atomicPair).m2);

  // test atomic_flag (should also have memory_orders, but probably doesn't
  // matter to find the missing atomic functions)
  atomic_flag af = ATOMIC_FLAG_INIT;
  atomic_flag_clear(&af);
  bool b = atomic_flag_test_and_set(&af);
  printf("atomic_flag: %s\n", b ? "true" : "false");

  printf("done.\n");
  return 0;
}
