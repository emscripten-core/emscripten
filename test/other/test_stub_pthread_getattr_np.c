#define _GNU_SOURCE
#include "pthread.h"
#include "stdio.h"
#include "assert.h"

int main() {
    pthread_attr_t attr;
    size_t stack_size, guard_size;
    void *stack_addr;

    assert(pthread_getattr_np(pthread_self(), &attr) == 0);
    assert(pthread_attr_getguardsize(&attr, &guard_size) == 0);
    assert(pthread_attr_getstack(&attr, &stack_addr, &stack_size) == 0);
    assert(pthread_attr_destroy(&attr) == 0);
    printf("stack_addr: %p, stack_size: %zu, guard_size: %zu\n", stack_addr, stack_size, guard_size);
    return 0;
}
