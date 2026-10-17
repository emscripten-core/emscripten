// Copyright 2026 The Emscripten Authors. All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License. Both these licenses can be
// found in the LICENSE file.

#include <cassert>
#include <cstdio>
#include <initializer_list>
#include <new>

static int handler_calls;

static void retry_handler() {
  if (++handler_calls == 2) {
    throw std::bad_alloc();
  }
}

template<typename Allocate, typename Deallocate>
static void test_new(Allocate allocate, Deallocate deallocate) {
  std::set_new_handler(nullptr);
  void* p = allocate(0, false);
  assert(p != nullptr);
  deallocate(p);

  // Exceed MAXIMUM_MEMORY without exhausting the memory needed to throw.
  constexpr size_t too_large = 128 * 1024 * 1024;
  try {
    allocate(too_large, false);
    assert(false && "operator new must throw on allocation failure");
  } catch (const std::bad_alloc&) {
  }
  assert(allocate(too_large, true) == nullptr);

  // Retry after a returning handler; propagate its exception from throwing new
  // and catch it in nothrow new.
  std::set_new_handler(retry_handler);
  for (bool nothrow : {false, true}) {
    handler_calls = 0;
    try {
      p = allocate(too_large, nothrow);
      assert(nothrow && p == nullptr);
    } catch (const std::bad_alloc&) {
      assert(!nothrow);
    }
    assert(handler_calls == 2);
  }
  std::set_new_handler(nullptr);

  p = allocate(1024, false);
  assert(p != nullptr);
  deallocate(p);
}

int main() {
  test_new([](size_t size, bool nothrow) {
    if (nothrow) return ::operator new(size, std::nothrow);
    return ::operator new(size);
  }, [](void* p) { ::operator delete(p); });

  test_new([](size_t size, bool nothrow) {
    if (nothrow) return ::operator new[](size, std::nothrow);
    return ::operator new[](size);
  }, [](void* p) { ::operator delete[](p); });

  test_new([](size_t size, bool nothrow) {
    if (nothrow) return ::operator new(size, std::align_val_t(64), std::nothrow);
    return ::operator new(size, std::align_val_t(64));
  }, [](void* p) { ::operator delete(p, std::align_val_t(64)); });

  test_new([](size_t size, bool nothrow) {
    if (nothrow) return ::operator new[](size, std::align_val_t(64), std::nothrow);
    return ::operator new[](size, std::align_val_t(64));
  }, [](void* p) { ::operator delete[](p, std::align_val_t(64)); });

  puts("new allocation failures handled");
}
