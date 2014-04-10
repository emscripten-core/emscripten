//------------------------------------------------------------------------------
//  test C++11 atomics
//  compile native version with:
//  clang -std=c++11 -Wno-format atomic_test.cpp 
//  compile emscripten version with:
//  emcc -std=c++11 -Wno-format atomic_test.cpp
//------------------------------------------------------------------------------
#include <atomic>
#include <cstdio>

template<typename TYPE> void test(TYPE mask0, TYPE mask1, TYPE mask2) {
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
    printf("atomic<int>.is_lock_free(): %s", atomicDog.is_lock_free() ? "true" : "false");
    printf("atomic<int> value: %lld\n", TYPE(atomicDog));

    // test store/load
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        atomicDog.store(i, memoryOrder[i]);
        printf("store/load %lld: %lld\n", i, atomicDog.load(memoryOrder[i]));
    }

    // test exchange
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        TYPE old = atomicDog.exchange(i, memoryOrder[i]);
        printf("exchange %lld: old=%lld new=%lld\n", i, old, TYPE(atomicDog));
    }

    // compare_exchange_weak
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        bool success = atomicDog.compare_exchange_weak(i, i + 1, memoryOrder[i], memoryOrder[i]);
        printf("compare_exchange_weak %ld: success = %s\n", i, success ? "true" : "false");
    }

    // compare_exchange_strong
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        bool success = atomicDog.compare_exchange_strong(i, i + 1, memoryOrder[i], memoryOrder[i]);
        printf("compare_exchange_strong %ld: success = %s\n", i, success ? "true" : "false");
    }

    // fetch_add
    atomicDog = 0;
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        TYPE old = atomicDog.fetch_add(1, memoryOrder[i]);
        printf("fetch_add %ld: old=%d new=%ld\n", i, old, TYPE(atomicDog));
    }

    // fetch_sub
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        TYPE old = atomicDog.fetch_sub(1, memoryOrder[i]);
        printf("fetch_sub %ld: old=%d new=%ld\n", i, old, TYPE(atomicDog));
    }

    // fetch_and
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        atomicDog.store(mask0, memoryOrder[i]);
        TYPE old = atomicDog.fetch_and((1<<i), memoryOrder[i]);
        printf("fetch_and %ld: old=%lx, new=%lx\n", i, old, TYPE(atomicDog));
    }

    // fetch_or
    atomicDog = 0;
    for (TYPE i = 0; i < numMemoryOrders; i++) {
        TYPE old = atomicDog.fetch_or((1<<i), memoryOrder[i]);
        printf("fetch_or %ld: old=%lx, new=%lx\n", i, old, TYPE(atomicDog));
    }

    // fetch_xor
    atomicDog = 0;
    for (int i = 0; i < numMemoryOrders; i++) {
        int old = atomicDog.fetch_xor((1<<i), memoryOrder[i]);
        printf("fetch_xor %ld: old=%llx, new=%lx\n", i, old, TYPE(atomicDog));
    }

    // operator++, --
    atomicDog = 0;
    atomicDog++;
    printf("operator++: %ld\n", TYPE(atomicDog));
    atomicDog--;
    printf("operator--: %ld\n", TYPE(atomicDog));

    // operator +=, -=, &=, |=, ^=
    atomicDog += 10;
    printf("operator+=: %ld\n", TYPE(atomicDog));
    atomicDog -= 5;
    printf("operator-=: %ld\n", TYPE(atomicDog));
    atomicDog |= mask0;
    printf("operator|=: %lx\n", TYPE(atomicDog));
    atomicDog &= mask1;
    printf("operator|=: %lx\n", TYPE(atomicDog));
    atomicDog ^= mask2;
    printf("operator^=: %lx\n", TYPE(atomicDog));

}

int main() {

    // test 8, 16, 32 and 64-bit data types
    test<char>(0xFF, 0xF0, 0x0F);
    test<short>(0xFFFF, 0xF0F0, 0x0F0F);
    test<int>(0xFFFFFFFF, 0xF0F0F0F0, 0x0F0F0F0F);
    test<long long>(0xFFFFFFFFFFFFFFFF, 0xF0F0F0F0F0F0F0F0, 0x0F0F0F0F0F0F0F0F);

    // test atomic_flag (should also have memory_orders, but probably doesn't matter 
    // to find the missing atomic functions)
    std::atomic_flag af;
    af.clear();
    bool b = af.test_and_set();
    printf("atomic_flag: %s\n", b ? "true" : "false");

    printf("done.\n");
    return 0;
}

