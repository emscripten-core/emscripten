// Copyright 2015 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <assert.h>
#include <atomic>

int main(void)
{
    std::atomic<int64_t> a(0x0F0F0F0F0F0F0F0FULL);
    int64_t old = a.fetch_add(0x1020304150607001ULL, std::memory_order_relaxed);
    printf("old=%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F0FULL);
    printf("added=%llx\n", a.load(std::memory_order_relaxed));
    assert(a.load(std::memory_order_relaxed) == 0x1F2F3F505F6F7F10ULL);

    a.store(0x0F0F0F0F0F0F0F0FULL);
    old = a.fetch_sub(0x0102030405060708ULL, std::memory_order_relaxed);
    printf("old=%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F0FULL);
    printf("subbed=%llx\n", a.load(std::memory_order_relaxed));
    assert(a.load(std::memory_order_relaxed) == 0x0E0D0C0B0A090807ULL);

    old = a.fetch_and(0x0770077007700070ULL, std::memory_order_relaxed);
    printf("old=%llx\n", old);
    assert(old == 0x0E0D0C0B0A090807ULL);
    printf("anded=%llx\n", a.load(std::memory_order_relaxed));
    assert(a.load(std::memory_order_relaxed) == 0x0600040002000000ULL);

    old = a.fetch_or(0x3003300330033000ULL, std::memory_order_relaxed);
    printf("old=%llx\n", old);
    assert(old == 0x0600040002000000ULL);
    printf("orred=%llx\n", a.load(std::memory_order_relaxed));
    assert(a.load(std::memory_order_relaxed) == 0x3603340332033000ULL);

    old = a.fetch_xor(0xFF00FF0000FF00FFULL, std::memory_order_relaxed);
    printf("old=%llx\n", old);
    assert(old == 0x3603340332033000ULL);
    printf("xorred=%llx\n", a.load(std::memory_order_relaxed));
    assert(a.load(std::memory_order_relaxed) == 0xC903CB0332FC30FFULL);

    std::atomic_thread_fence(std::memory_order_acquire);

    std::atomic<int64_t> b(0x0F0F0F0F0F0F0F0FULL);
    old = std::atomic_fetch_add<int64_t>(&b, 0x1020304150607001ULL);
    printf("old=%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F0FULL);
    printf("added2=%llx\n", b.load(std::memory_order_relaxed));
    assert(b.load(std::memory_order_relaxed) == 0x1F2F3F505F6F7F10ULL);

    b.store(0x0F0F0F0F0F0F0F0FULL);
    old = std::atomic_fetch_sub<int64_t>(&b, 0x0102030405060708ULL);
    printf("old=%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F0FULL);
    printf("subbed2=%llx\n", b.load(std::memory_order_relaxed));
    assert(b.load(std::memory_order_relaxed) == 0x0E0D0C0B0A090807ULL);

    std::atomic<int64_t> c(0x0F0F0F0F0F0F0F0FULL);
    old = std::atomic_fetch_add_explicit<int64_t>(&c, 0x1020304150607001ULL, std::memory_order_relaxed);
    printf("old=%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F0FULL);
    printf("added3=%llx\n", c.load(std::memory_order_relaxed));
    assert(c.load(std::memory_order_relaxed) == 0x1F2F3F505F6F7F10ULL);

    c.store(0x0F0F0F0F0F0F0F0FULL);
    old = std::atomic_fetch_sub_explicit<int64_t>(&c, 0x0102030405060708ULL, std::memory_order_relaxed);
    printf("old=%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F0FULL);
    printf("subbed3=%llx\n", c.load(std::memory_order_relaxed));
    assert(c.load(std::memory_order_relaxed) == 0x0E0D0C0B0A090807ULL);

    return 0;
}
