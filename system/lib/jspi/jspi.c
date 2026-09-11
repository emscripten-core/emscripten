/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Runtime side of the JSPI lifecycle hooks. The __jspi_enter/exit/suspend/
 * resume exports (shims in jspi_ops.S calling __jspi_hook_impl here) are
 * called by the wrappers that binaryen's --jspi-hooks pass places around every
 * promising export and suspending import; see <emscripten/jspi.h> for the
 * model. With REENTRANT_JSPI it also gives each fiber its own shadow stack.
 */

#include <emscripten/console.h>
#include <emscripten/heap.h>
#include <emscripten/jspi.h>
#include <emscripten/stack.h>
#include <stdlib.h>

typedef struct {
  jspi_hook fn;
  uint32_t mask;
} jspi_registration;

static _Thread_local jspi_registration hooks[JSPI_MAX_HOOKS];
static _Thread_local uint32_t hook_count;

// One record per live fiber, holding each registration's token for it. The
// record's address is the token the wrappers carry between the events of a
// pair, and the current record follows the same protocol the stack pointer
// does: whoever entered or last resumed the fiber is remembered, made current
// again when the fiber leaves at SUSPEND (the import may return to that code
// synchronously), refreshed at RESUME and made current at EXIT.
//
// With REENTRANT_JSPI each fiber also runs on its own shadow stack, allocated
// at ENTER and released at EXIT, so any number of fibers can be suspended at
// once with their frames intact and imports may write through pointers into a
// suspended fiber's frames. The stack pointer (and the stack limits, for
// STACK_OVERFLOW_CHECK and emscripten_stack_get_*) are switched at the four
// events; see site/source/docs/porting/asyncify.rst.
typedef struct jspi_fiber {
  struct jspi_fiber* host;
  void* tokens[JSPI_MAX_HOOKS];
#if REENTRANT_JSPI
  uintptr_t sp;      // fiber stack pointer while suspended
  uintptr_t host_sp; // where the code that entered or resumed us was
  uintptr_t host_base;
  uintptr_t host_end;
  void* stack;
#endif
} jspi_fiber;

// Records for the first fibers alive at once come from a per-thread pool (a
// free record links to the next through `host`; static storage, which a fiber
// stack overflow into the heap cannot reach); further ones are heap-allocated.
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

// The current fiber lives in a wasm global (per instance, hence per thread);
// NULL outside any fiber.
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

#if REENTRANT_JSPI
// Released fiber stacks kept for reuse.
#define STACK_POOL 4
static _Thread_local void* stack_pool[STACK_POOL];
static _Thread_local int stack_pool_count;
static _Thread_local size_t stack_size;
static _Thread_local size_t guard_size;

size_t __jspi_fiber_stack_size(void);
size_t __jspi_fiber_stack_guard(void);
int __jspi_stack_checked(void);

// The stack limits to install along with the returned stack pointer, and
// whether the STACK_OVERFLOW_CHECK=2 bounds also need updating. The shim in
// jspi_ops.S defines these globals and applies them after the impl returned.
__asm__(".globaltype __jspi_pending_base, " PTR "\n"
        ".globaltype __jspi_pending_end, " PTR "\n"
        ".globaltype __jspi_stack_check, i32\n");

static void set_pending_limits(uintptr_t base, uintptr_t end) {
  __asm__ volatile("local.get %0\n"
                   "global.set __jspi_pending_base\n"
                   "local.get %1\n"
                   "global.set __jspi_pending_end"
                   :
                   : "r"(base), "r"(end));
}

// Below each fiber stack lies a guard region of JSPI_FIBER_STACK_GUARD bytes
// that is ours, so an overflow of up to that size corrupts nothing else, and
// is caught when the fiber leaves its stack at a suspension or exit: the
// stack pointer must still be inside the stack and the cookies at its low end
// intact. STACK_OVERFLOW_CHECK=2 catches the overflowing store itself.
#define COOKIE0 0x4a535049
#define COOKIE1 0x46494252

static void write_cookies(uintptr_t low) {
  ((uint32_t*)low)[0] = COOKIE0;
  ((uint32_t*)low)[1] = COOKIE1;
}

static void check_overflow(uintptr_t low, uintptr_t sp) {
  if (sp < low || ((uint32_t*)low)[0] != COOKIE0 ||
      ((uint32_t*)low)[1] != COOKIE1) {
    emscripten_err("REENTRANT_JSPI: fiber stack overflow; increase "
                   "-sJSPI_FIBER_STACK_SIZE");
    abort();
  }
}

// The stack region within a fiber's allocation.
static uintptr_t stack_low(jspi_fiber* f) {
  return (uintptr_t)f->stack + guard_size;
}

#ifdef __EMSCRIPTEN_PTHREADS__
#include <pthread.h>

static pthread_key_t cleanup_key;
static pthread_once_t cleanup_once = PTHREAD_ONCE_INIT;

static void thread_cleanup(void* arg) {
  while (stack_pool_count) {
    emscripten_builtin_free(stack_pool[--stack_pool_count]);
  }
}

static void create_cleanup_key(void) {
  pthread_key_create(&cleanup_key, thread_cleanup);
}
#endif

static void init_thread(void) {
  stack_size = (__jspi_fiber_stack_size() + 15) & ~(size_t)15;
  guard_size = (__jspi_fiber_stack_guard() + 15) & ~(size_t)15;
  __asm__ volatile("local.get %0\n"
                   "global.set __jspi_stack_check"
                   :
                   : "r"(__jspi_stack_checked()));
#ifdef __EMSCRIPTEN_PTHREADS__
  pthread_once(&cleanup_once, create_cleanup_key);
  pthread_setspecific(cleanup_key, (void*)1);
#endif
}

static void alloc_stack(jspi_fiber* f) {
  if (!stack_size) {
    init_thread();
  }
  if (stack_pool_count) {
    f->stack = stack_pool[--stack_pool_count];
  } else {
    f->stack = emscripten_builtin_memalign(16, guard_size + stack_size);
    if (!f->stack) {
      emscripten_err("REENTRANT_JSPI: out of memory allocating a fiber stack");
      abort();
    }
    write_cookies(stack_low(f));
  }
}

static void release_stack(jspi_fiber* f) {
  if (stack_pool_count < STACK_POOL) {
    stack_pool[stack_pool_count++] = f->stack;
  } else {
    emscripten_builtin_free(f->stack);
  }
}

// Switching to the fiber's stack: remember the host's limits (the host may
// itself be a fiber) and prepare the fiber's.
static void switch_to_fiber(jspi_fiber* f, uintptr_t host_sp) {
  f->host_sp = host_sp;
  f->host_base = emscripten_stack_get_base();
  f->host_end = emscripten_stack_get_end();
  set_pending_limits(stack_low(f) + stack_size, stack_low(f));
}

static uintptr_t switch_to_host(jspi_fiber* f, uintptr_t sp) {
  check_overflow(stack_low(f), sp);
  set_pending_limits(f->host_base, f->host_end);
  return f->host_sp;
}
#endif

// The implementation behind the hook exports (shims in jspi_ops.S):
// delivers the event (0 ENTER, 1 EXIT, 2 SUSPEND, 3 RESUME), returns the stack
// pointer to install and leaves the token to return (the fiber) in the
// __jspi_token global.
__asm__(".globaltype __jspi_token, i64\n");

static void set_token(jspi_fiber* f) {
  uint64_t token = (uintptr_t)f;
  __asm__ volatile("local.get %0\n"
                   "global.set __jspi_token"
                   :
                   : "r"(token));
}

uintptr_t
__jspi_hook_impl(uintptr_t sp, uint32_t event, uint64_t token, int error) {
  jspi_fiber* f = (jspi_fiber*)(uintptr_t)token;
  set_token(NULL);
  switch ((jspi_event)(1u << event)) {
    case JSPI_ENTER:
      f = alloc_fiber();
      f->host = get_cur_fiber();
      set_cur_fiber(f);
#if REENTRANT_JSPI
      alloc_stack(f);
      switch_to_fiber(f, sp);
      sp = stack_low(f) + stack_size;
#endif
      dispatch(JSPI_ENTER, f, 0);
      set_token(f);
      break;
    case JSPI_EXIT:
      dispatch(JSPI_EXIT, f, error);
      set_cur_fiber(f->host);
#if REENTRANT_JSPI
      sp = switch_to_host(f, sp);
      release_stack(f);
#endif
      free_fiber(f);
      break;
    case JSPI_SUSPEND:
      f = get_cur_fiber();
      // Without a fiber the import is about to fail with a SuspendError.
      if (f) {
        dispatch(JSPI_SUSPEND, f, 0);
        set_cur_fiber(f->host);
#if REENTRANT_JSPI
        f->sp = sp;
        sp = switch_to_host(f, sp);
#endif
      }
      set_token(f);
      break;
    case JSPI_RESUME:
      if (f) {
        f->host = get_cur_fiber();
        set_cur_fiber(f);
#if REENTRANT_JSPI
        switch_to_fiber(f, sp);
        sp = f->sp;
#endif
        dispatch(JSPI_RESUME, f, error);
      }
      break;
  }
  return sp;
}

int jspi_register(jspi_hook fn, uint32_t mask) {
  if (hook_count == JSPI_MAX_HOOKS) {
    return -2;
  }
  hooks[hook_count++] = (jspi_registration){fn, mask};
  return 0;
}
