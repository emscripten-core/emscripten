/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Runtime side of the JSPI lifecycle hooks. The __jspi_enter/exit/suspend/
 * resume exports are called by the wrappers that binaryen's --jspi-hooks pass
 * places around every promising export and suspending import; see
 * <emscripten/jspi.h> for the model.
 */

#include <emscripten/console.h>
#include <emscripten/heap.h>
#include <emscripten/jspi.h>
#include <stdlib.h>

typedef struct {
  jspi_hook fn;
  uint32_t mask;
} jspi_registration;

static _Thread_local jspi_registration hooks[JSPI_MAX_HOOKS];
static _Thread_local uint32_t hook_count;

// One record per live fiber, holding each registration's token for it. The
// record's address is the token the wrappers carry between the events of a
// pair, and the current record follows the same protocol a stack pointer
// would: whoever entered or last resumed the fiber is remembered, made
// current again when the fiber leaves at SUSPEND (the import may return to
// that code synchronously), refreshed at RESUME and made current at EXIT.
typedef struct jspi_fiber {
  struct jspi_fiber* host;
  void* tokens[JSPI_MAX_HOOKS];
} jspi_fiber;

// Records for the first fibers alive at once come from a per-thread pool (a
// free record links to the next through `host`); further ones are
// heap-allocated.
#define POOL_FIBERS 64
static _Thread_local jspi_fiber pool[POOL_FIBERS];
static _Thread_local jspi_fiber* free_list;
static _Thread_local uint32_t pool_used;

static jspi_fiber* alloc_fiber(void) {
  jspi_fiber* f = free_list;
  if (f) {
    free_list = f->host;
  } else if (pool_used < POOL_FIBERS) {
    f = &pool[pool_used++];
  } else {
    f = emscripten_builtin_malloc(sizeof(jspi_fiber));
    if (!f) {
      emscripten_err("JSPI: out of memory");
      abort();
    }
  }
  *f = (jspi_fiber){0};
  return f;
}

static void free_fiber(jspi_fiber* f) {
  if (f >= pool && f < pool + POOL_FIBERS) {
    f->host = free_list;
    free_list = f;
  } else {
    emscripten_builtin_free(f);
  }
}

// The current fiber lives in a wasm global (per instance, hence per thread);
// NULL outside any fiber.
#ifdef __wasm64__
#define PTR "i64"
#else
#define PTR "i32"
#endif
__asm__(".globaltype __jspi_cur_fiber, " PTR "\n"
        ".globl __jspi_cur_fiber\n"
        "__jspi_cur_fiber:\n");

static jspi_fiber* get_cur_fiber(void) {
  jspi_fiber* f;
  __asm__ volatile("global.get __jspi_cur_fiber\n"
                   "local.set %0"
                   : "=r"(f));
  return f;
}

static void set_cur_fiber(jspi_fiber* f) {
  __asm__ volatile("local.get %0\n"
                   "global.set __jspi_cur_fiber"
                   :
                   : "r"(f));
}

__attribute__((noinline)) static void
dispatch(jspi_event event, jspi_fiber* f, int error) {
  for (uint32_t i = 0; i < hook_count; i++) {
    jspi_registration* r = &hooks[i];
    if (r->mask & event) {
      f->tokens[i] = r->fn(event, f->tokens[i], error);
    }
  }
}

// The exports the wrappers call around promising exports (enter/exit) and
// suspending imports (suspend/resume). The token the "before" hook returns
// (the fiber) comes back to the "after" hook, along with whether the wrapped
// call threw.
uint64_t __jspi_enter(void) {
  jspi_fiber* f = alloc_fiber();
  f->host = get_cur_fiber();
  set_cur_fiber(f);
  dispatch(JSPI_ENTER, f, 0);
  return (uintptr_t)f;
}

void __jspi_exit(uint64_t token, int error) {
  jspi_fiber* f = (jspi_fiber*)(uintptr_t)token;
  dispatch(JSPI_EXIT, f, error);
  set_cur_fiber(f->host);
  free_fiber(f);
}

uint64_t __jspi_suspend(void) {
  jspi_fiber* f = get_cur_fiber();
  // Without a fiber the import is about to fail with a SuspendError.
  if (f) {
    dispatch(JSPI_SUSPEND, f, 0);
    set_cur_fiber(f->host);
  }
  return (uintptr_t)f;
}

void __jspi_resume(uint64_t token, int error) {
  jspi_fiber* f = (jspi_fiber*)(uintptr_t)token;
  if (f) {
    f->host = get_cur_fiber();
    set_cur_fiber(f);
    dispatch(JSPI_RESUME, f, error);
  }
}

int jspi_register(jspi_hook fn, uint32_t mask) {
  if (hook_count == JSPI_MAX_HOOKS) {
    return -2;
  }
  hooks[hook_count++] = (jspi_registration){fn, mask};
  return 0;
}
