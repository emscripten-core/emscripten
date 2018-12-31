#include <stdio.h>
#include <assert.h>
#include <atomic>

int main(void)
{
    std::atomic<int64_t> a(0x0F0F0F0F0F0F0F0FULL);
    int64_t old = a.fetch_add(1, std::memory_order_relaxed);
    printf("%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F0FULL);
    assert(a.load(std::memory_order_relaxed) == 0x0F0F0F0F0F0F0F10ULL);

    old = a.fetch_sub(1, std::memory_order_relaxed);
    printf("%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F10ULL);
    assert(a.load(std::memory_order_relaxed) == 0x0F0F0F0F0F0F0F0FULL);

    old = a.fetch_and(0x0770077007700070ULL, std::memory_order_relaxed);
    printf("%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F0FULL);
    assert(a.load(std::memory_order_relaxed) == 0x0700070007000000ULL);

    old = a.fetch_or(0x3003300330033000ULL, std::memory_order_relaxed);
    printf("%llx\n", old);
    assert(old == 0x0700070007000000ULL);
    assert(a.load(std::memory_order_relaxed) == 0x3703370337033000ULL);

    old = a.fetch_xor(0xFF00FF0000FF00FFULL, std::memory_order_relaxed);
    printf("%llx\n", old);
    assert(old == 0x3703370337033000ULL);
    assert(a.load(std::memory_order_relaxed) == 0xC803C80337FC30FFULL);

    std::atomic_thread_fence(std::memory_order_acquire);

    std::atomic<int64_t> b(0x0F0F0F0F0F0F0F0FULL);
    old = std::atomic_fetch_add<int64_t>(&b, 1);
    printf("%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F0FULL);
    assert(b.load(std::memory_order_relaxed) == 0x0F0F0F0F0F0F0F10ULL);

    old = std::atomic_fetch_sub<int64_t>(&b, 1);
    printf("%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F10ULL);
    assert(b.load(std::memory_order_relaxed) == 0x0F0F0F0F0F0F0F0FULL);

    std::atomic<int64_t> c(0x0F0F0F0F0F0F0F0FULL);
    old = std::atomic_fetch_add_explicit<int64_t>(&c, 1, std::memory_order_relaxed);
    printf("%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F0FULL);
    assert(c.load(std::memory_order_relaxed) == 0x0F0F0F0F0F0F0F10ULL);

    old = std::atomic_fetch_sub_explicit<int64_t>(&c, 1, std::memory_order_relaxed);
    printf("%llx\n", old);
    assert(old == 0x0F0F0F0F0F0F0F10ULL);
    assert(c.load(std::memory_order_relaxed) == 0x0F0F0F0F0F0F0F0FULL);

#ifdef REPORT_RESULT
    int result = 0;
    REPORT_RESULT();
#endif

    return 0;
}
