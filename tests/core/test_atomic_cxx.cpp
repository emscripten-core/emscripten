// Copyright 2014 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

//------------------------------------------------------------------------------
//  test C++11 atomics
//  compile native version with:
//  clang -Wno-format test_atomic_cxx.cpp
//  compile emscripten version with:
//  emcc -Wno-format test_atomic_cxx.cpp
//------------------------------------------------------------------------------
#include <atomic>
#include <cstdio>

template<typename TYPE, typename UNSIGNED_TYPE> void test(TYPE mask0, TYPE mask1, TYPE mask2) {
    typedef TYPE dog;
    
    const TYPE numMemoryOrders = 6;
    std::memory_order memoryOrder[numMemoryOrders] = {
        std::memory_order_relaxed,
        std::memory_order_consume,
        std::memory_order_acquire,
        std::memory_order_release,
        std::memory_order_acq_rel,
        std::memory_order_seq_cst,
    };

    // test atomic<int>
    std::atomic<dog> atomicDog(5);
    if (sizeof(TYPE) < 8) {
      printf("atomic<int>.is_lock_free(): %s\n", atomicDog.is_lock_free() ? "true" : "false");
    } else {
      printf("atomic<int>.is_lock_free(): %s\n", atomicDog.is_lock_free() == IS_64BIT_LOCK_FREE ? "ok" : "bad :(");
    }
    printf("atomic<int> value: %lld\n", (long long)TYPE(atomicDog));

    // test store/load
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        atomicDog.store(i, memoryOrder[i]);
        printf("store/load %lld: %lld\n", (long long)i, (long long)atomicDog.load(memoryOrder[i]));
    }

    // test exchange
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        TYPE old = atomicDog.exchange(i, memoryOrder[i]);
        printf("exchange %lld: old=%lld new=%lld\n", (long long)i, (long long)old, (long long)TYPE(atomicDog));
    }

    // compare_exchange_weak
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        bool success = atomicDog.compare_exchange_weak(i, i + 1, memoryOrder[i], memoryOrder[i]);
        printf("compare_exchange_weak %lld: success = %s\n", (long long)i, success ? "true" : "false");
    }

    // compare_exchange_strong
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        bool success = atomicDog.compare_exchange_strong(i, i + 1, memoryOrder[i], memoryOrder[i]);
        printf("compare_exchange_strong %lld: success = %s\n", (long long)i, success ? "true" : "false");
    }

    // fetch_add
    atomicDog = mask2;
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        TYPE old = atomicDog.fetch_add(1, memoryOrder[i]);
        printf("fetch_add %lld: old=%llx new=%llx\n", (long long)i, (long long)old, (long long)TYPE(atomicDog));
    }

    // fetch_sub
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        TYPE old = atomicDog.fetch_sub(1, memoryOrder[i]);
        printf("fetch_sub %lld: old=%llx new=%llx\n", (long long)i, (long long)old, (long long)TYPE(atomicDog));
    }

    // fetch_and
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        atomicDog.store(mask0, memoryOrder[i]);
        TYPE old = atomicDog.fetch_and((1<<i), memoryOrder[i]);
        printf("fetch_and %lld: old=%llx, new=%llx\n", (long long)i, (unsigned long long)UNSIGNED_TYPE(old), (unsigned long long)UNSIGNED_TYPE(atomicDog));
    }

    // fetch_or
    atomicDog = 0;
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        TYPE old = atomicDog.fetch_or((1<<i), memoryOrder[i]);
        printf("fetch_or %lld: old=%llx, new=%llx\n", (long long)i, (unsigned long long)UNSIGNED_TYPE(old), (unsigned long long)UNSIGNED_TYPE(atomicDog));
    }

    // fetch_xor
    atomicDog = 0;
    for (int i = 0; i < numMemoryOrders; i++) {
        int old = atomicDog.fetch_xor((1<<i), memoryOrder[i]);
        printf("fetch_xor %lld: old=%llx, new=%llx\n", (long long)i, (unsigned long long)UNSIGNED_TYPE(old), (unsigned long long)UNSIGNED_TYPE(atomicDog));
    }

    // operator++, --
    atomicDog = 0;
    atomicDog++;
    printf("operator++: %lld\n", (long long)TYPE(atomicDog));
    atomicDog--;
    printf("operator--: %lld\n", (long long)TYPE(atomicDog));

    // operator +=, -=, &=, |=, ^=
    atomicDog += 10;
    printf("operator+=: %lld\n", (long long)TYPE(atomicDog));
    atomicDog -= 5;
    printf("operator-=: %lld\n", (long long)TYPE(atomicDog));
    atomicDog |= mask0;
    printf("operator|=: %llx\n", (unsigned long long)UNSIGNED_TYPE(atomicDog));
    atomicDog &= mask1;
    printf("operator&=: %llx\n", (unsigned long long)UNSIGNED_TYPE(atomicDog));
    atomicDog ^= mask2;
    printf("operator^=: %llx\n", (unsigned long long)UNSIGNED_TYPE(atomicDog));

}

int main() {

    // test 8, 16, 32 and 64-bit data types
    printf("\n8 bits\n\n");
    test<char, unsigned char>(0xFF, 0xF0, 0x0F);
    printf("\n16 bits\n\n");
    test<short, unsigned short>(0xFFFF, 0xF0F0, 0x0F0F);
    printf("\n32 bits\n\n");
    test<int, unsigned int>(0xFFFFFFFF, 0xF0F0F0F0, 0x0F0F0F0F);
    printf("\n64 bits\n\n");
    test<long long, unsigned long long>(0xFFFFFFFFFFFFFFFF, 0xF0F0F0F0F0F0F0F0, 0x0F0F0F0F0F0F0F0F);

    // test atomic_flag (should also have memory_orders, but probably doesn't matter 
    // to find the missing atomic functions)
    std::atomic_flag af;
    af.clear();
    bool b = af.test_and_set();
    printf("atomic_flag: %s\n", b ? "true" : "false");

    printf("done.\n");
    return 0;
}

