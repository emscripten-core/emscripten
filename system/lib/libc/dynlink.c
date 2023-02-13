/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Emscripten-specific version dlopen and associated functions.  Some code is
 * shared with musl's ldso/dynlink.c.
 */

#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <pthread.h>
#include <threads.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dynlink.h>

#include <emscripten/console.h>
#include <emscripten/threading.h>
#include <emscripten/promise.h>
#include <emscripten/proxying.h>

#include "pthread_impl.h"

//#define DYLINK_DEBUG

#ifdef DYLINK_DEBUG
#define dbg(fmt, ...) _emscripten_dbgf(fmt, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif

struct async_data {
  em_dlopen_callback onsuccess;
  em_arg_callback_func onerror;
  void* user_data;
};
typedef void (*dlopen_callback_func)(struct dso*, void* user_data);

void* _dlopen_js(struct dso* handle);
void* _dlsym_js(struct dso* handle, const char* symbol, int* sym_index);
void _emscripten_dlopen_js(struct dso* handle,
                           dlopen_callback_func onsuccess,
                           dlopen_callback_func onerror,
                           void* user_data);
void __dl_vseterr(const char*, va_list);

// We maintain a list of all dlopen and dlsym events linked list.
// In multi-threaded builds this is used to keep all the threads in sync
// with each other.
// In single-threaded builds its only used to keep track of valid DSO handles.
struct dlevent {
  struct dlevent *next, *prev;
  // Symbol index resulting from dlsym call. -1 means this is a dso event.
  int sym_index;
  // dso handler resulting fomr dleopn call.  Only valid when sym_index is -1.
  struct dso* dso;
#ifdef DYLINK_DEBUG
  int id;
#endif
};

// Handle to "main" dso, needed for dlopen(NULL,..)
static struct dso main_dso = {
  .name = "__main__",
  .flags = 0,
};

static struct dlevent main_event = {
  .prev = NULL,
  .next = NULL,
  .sym_index = -1,
  .dso = &main_dso,
};

static struct dlevent* _Atomic head = &main_event;
static struct dlevent* _Atomic tail = &main_event;

#ifdef _REENTRANT
void* _dlsym_catchup_js(struct dso* handle, int sym_index);

static thread_local struct dlevent* thread_local_tail = &main_event;
static pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;
static thread_local bool skip_dlsync = false;

static void do_write_lock() {
  // Once we have the lock we want to avoid automatic code sync as that would
  // result in a deadlock.
  skip_dlsync = true;
  pthread_mutex_lock(&write_lock);
}

static void do_write_unlock() {
  pthread_mutex_unlock(&write_lock);
  skip_dlsync = false;
}
#else // _REENTRANT
#define do_write_unlock()
#define do_write_lock()
#endif

static void error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  __dl_vseterr(fmt, ap);
  va_end(ap);
#ifdef DYLINK_DEBUG
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
#endif
}

int __dl_invalid_handle(void* h) {
  struct dlevent* p;
  for (p = head; p; p = p->next)
    if (p->sym_index == -1 && p->dso == h)
      return 0;
  dbg("__dl_invalid_handle %p", h);
  error("Invalid library handle %p", (void*)h);
  return 1;
}

void new_dlevent(struct dso* p, int sym_index) {
  struct dlevent* ev = calloc(1, sizeof(struct dlevent));

  ev->dso = p;
  ev->sym_index = sym_index;
  if (p) p->event = ev;

  // insert into linked list
  ev->prev = tail;
  if (tail) {
    tail->next = ev;
#ifdef DYLINK_DEBUG
    ev->id = tail->id + 1;
#endif
  }
  dbg("new_dlevent: ev=%p id=%d %s dso=%p sym_index=%d",
      ev,
      ev->id,
      p ? p->name : "RTLD_DEFAULT",
      p,
      sym_index);
  tail = ev;
#if _REENTRANT
  thread_local_tail = ev;
#endif
}

static void load_library_done(struct dso* p) {
  dbg("load_library_done: dso=%p mem_addr=%p mem_size=%zu "
      "table_addr=%p table_size=%zu",
      p,
      p->mem_addr,
      p->mem_size,
      p->table_addr,
      p->table_size);
  new_dlevent(p, -1);
}

static struct dso* load_library_start(const char* name, int flags) {
  if (!(flags & (RTLD_LAZY | RTLD_NOW))) {
    error("invalid mode for dlopen(): Either RTLD_LAZY or RTLD_NOW is required");
    return NULL;
  }

  struct dso* p;
  size_t alloc_size = sizeof *p + strlen(name) + 1;
  p = calloc(1, alloc_size);
  p->flags = flags;
  strcpy(p->name, name);

  return p;
}

#ifdef _REENTRANT
// When we are attempting to synchronize loaded libraries between threads we
// currently abort, rather than rejecting the promises.  We could reject the
// promises, and attempt to return an error from the original dlopen() but we
// would have to also unwind the state on all the threads that were able to load
// the module.
#define ABORT_ON_SYNC_FAILURE 1

// These functions are defined in JS.

// Synchronous version of "dlsync_threads".  Called only on the main thread.
// Runs _emscripten_dlsync_self on each of the threads that are running at
// the time of the call.
void _emscripten_dlsync_threads();

// Asynchronous version of "dlsync_threads".  Called only on the main thread.
// Runs _emscripten_dlsync_self on each of the threads that are running at
// the time of the call.  Once this is done the callback is called with the
// given em_proxying_ctx.
void _emscripten_dlsync_threads_async(pthread_t calling_thread,
                                      void (*callback)(em_proxying_ctx*),
                                      em_proxying_ctx* ctx);

static void dlsync_next(struct dlevent* dlevent, em_promise_t promise);

static void sync_one_onsuccess(struct dso* dso, void* user_data) {
  em_promise_t promise = (em_promise_t)user_data;
  dbg("sync_one_onsuccess dso=%p event=%p promise=%p", dso, dso->event, promise);
  // Load the next dso in the list
  thread_local_tail = dso->event;
  dlsync_next(thread_local_tail->next, promise);
}

static void sync_one_onerror(struct dso* dso, void* user_data) {
#if ABORT_ON_SYNC_FAILURE
  abort();
#else
  em_promise_t promise = (em_promise_t)user_data;
  emscripten_promise_reject(promise);
#endif
}

// Called on the main thread to asynchronously "catch up" with all the DSOs
// that are currently loaded.
static void dlsync_next(struct dlevent* dlevent, em_promise_t promise) {
  dbg("dlsync_next event=%p promise=%p", dlevent, promise);

  // Process any dlsym events synchronously until we find a dlopen event
  while (dlevent && dlevent->sym_index != -1) {
    dbg("calling _dlsym_catchup_js ....");
    void* success = _dlsym_catchup_js(dlevent->dso, dlevent->sym_index);
    if (!success) {
      _emscripten_errf("_dlsym_catchup_js failed: %s", dlerror());
      sync_one_onerror(dlevent->dso, promise);
      return;
    }
    dlevent = dlevent->next;
  }

  if (!dlevent) {
    // All dso loaded
    emscripten_promise_resolve(promise, EM_PROMISE_FULFILL, NULL);
    return;
  }

  dbg("dlsync_next calling _emscripten_dlopen_js: dso=%p", dlevent->dso);
  _emscripten_dlopen_js(
    dlevent->dso, sync_one_onsuccess, sync_one_onerror, promise);
}

void _emscripten_dlsync_self_async(em_promise_t promise) {
  dbg("_emscripten_dlsync_self_async promise=%p", promise);
  // Unlock happens once all DSO have been loaded, or one of them fails
  // with sync_one_onerror.
  dlsync_next(thread_local_tail->next, promise);
}

// Called on background threads to synchronously "catch up" with all the DSOs
// that are currently loaded.
bool _emscripten_dlsync_self() {
  // Should only ever be called from a background thread.
  assert(!emscripten_is_main_runtime_thread());
  if (thread_local_tail == tail) {
    dbg("_emscripten_dlsync_self: already in sync");
    return true;
  }
  dbg("_emscripten_dlsync_self: catching up %p %p", thread_local_tail, tail);
  while (thread_local_tail->next) {
    struct dlevent* p = thread_local_tail->next;
    if (p->sym_index != -1) {
      dbg("_emscripten_dlsync_self: id=%d %s sym_index=%d",
          p->id,
          p->dso->name,
          p->sym_index);
      void* success = _dlsym_catchup_js(p->dso, p->sym_index);
      if (!success) {
        _emscripten_errf("_dlsym_catchup_js failed: %s", dlerror());
        return false;
      }
    } else {
      dbg("_emscripten_dlsync_self: id=%d %s mem_addr=%p "
          "mem_size=%zu table_addr=%p table_size=%zu",
          p->id,
          p->dso->name,
          p->dso->mem_addr,
          p->dso->mem_size,
          p->dso->table_addr,
          p->dso->table_size);
      void* success = _dlopen_js(p->dso);
      if (!success) {
        // If any on the libraries fails to load here then we give up.
        // TODO(sbc): Ideally this would never happen and we could/should
        // abort, but on the main thread (where we don't have sync xhr) its
        // often not possible to syncronously load side module.
        _emscripten_errf("_dlopen_js failed: %s", dlerror());
        return false;
      }
    }
    thread_local_tail = p;
  }
  dbg("_emscripten_dlsync_self: done");
  return true;
}

static void* do_thread_sync(void* arg) {
  dbg("do_thread_sync");
  return (void*)_emscripten_dlsync_self();
}

static void do_thread_sync_out(void* arg) {
  dbg("do_thread_sync_out");
  int* result = (int*)arg;
  *result = _emscripten_dlsync_self();
}

// Called once _emscripten_proxy_dlsync completes
static void done_thread_sync(void* arg, void* result) {
  em_promise_t promise = (em_promise_t)arg;
  dbg("done_thread_sync: promise=%p result=%p", promise, result);
  if (result) {
    emscripten_promise_resolve(promise, EM_PROMISE_FULFILL, NULL);
  } else {
#if ABORT_ON_SYNC_FAILURE
    abort();
#else
    emscripten_promise_reject(promise);
#endif
  }
  emscripten_promise_destroy(promise);
}

// Proxying queue specically for handling code loading (dlopen) events.
// Initialized by the main thread on the first call to
// `_emscripten_proxy_dlsync` below, and processed by background threads
// that call `_emscripten_process_dlopen_queue` during futex_wait (i.e. whenever
// they block).
static em_proxying_queue * _Atomic dlopen_proxying_queue = NULL;
static thread_local bool processing_queue = false;

void _emscripten_process_dlopen_queue() {
  if (dlopen_proxying_queue && !processing_queue) {
    assert(!emscripten_is_main_runtime_thread());
    processing_queue = true;
    emscripten_proxy_execute_queue(dlopen_proxying_queue);
    processing_queue = false;
  }
}

// Asynchronously runs _emscripten_dlsync_self on the target then and
// resolves (or rejects) the given promise once it is complete.
// This function should only ever be called my the main runtime thread which
// manages the worker pool.
int _emscripten_proxy_dlsync_async(pthread_t target_thread, em_promise_t promise) {
  assert(emscripten_is_main_runtime_thread());
  if (!dlopen_proxying_queue) {
    dlopen_proxying_queue = em_proxying_queue_create();
  }
  int rtn = emscripten_proxy_async_with_callback(dlopen_proxying_queue,
                                                 target_thread,
                                                 do_thread_sync,
                                                 NULL,
                                                 done_thread_sync,
                                                 promise);
  if (target_thread->sleeping) {
    // If the target thread is in the sleeping state (and this check is
    // performed after the enqueuing of the async work) then we know its safe to
    // resolve the promise early, since the thread will process our event as
    // soon as it wakes up.
    emscripten_promise_resolve(promise, EM_PROMISE_FULFILL, NULL);
    return 0;
  }
  return rtn;
}

int _emscripten_proxy_dlsync(pthread_t target_thread) {
  assert(emscripten_is_main_runtime_thread());
  if (!dlopen_proxying_queue) {
    dlopen_proxying_queue = em_proxying_queue_create();
  }
  int result;
  if (!emscripten_proxy_sync(
        dlopen_proxying_queue, target_thread, do_thread_sync_out, &result)) {
    return 0;
  }
  return result;
}

static void done_sync_all(em_proxying_ctx* ctx) {
  dbg("done_sync_all");
  emscripten_proxy_finish(ctx);
}

static void run_dlsync_async(em_proxying_ctx* ctx, void* arg) {
  pthread_t calling_thread = (pthread_t)arg;
  dbg("main_thread_dlsync calling=%p", calling_thread);
  _emscripten_dlsync_threads_async(calling_thread, done_sync_all, ctx);
}

static void dlsync() {
  // Call dlsync process.  This call will block until all threads are in sync.
  // This gets called after a shared library is loaded by a worker.
  pthread_t main_thread = emscripten_main_browser_thread_id();
  dbg("dlsync main=%p", main_thread);
  if (pthread_self() == main_thread) {
    // dlsync was called on the main thread.  In this case we have no choice by
    // to run the blocking version of emscripten_dlsync_threads.
    _emscripten_dlsync_threads();
  } else {
    // Otherwise we block here while the asynchronous version runs in the main
    // thread.
    em_proxying_queue* q = emscripten_proxy_get_system_queue();
    int success = emscripten_proxy_sync_with_ctx(
      q, main_thread, run_dlsync_async, pthread_self());
    assert(success);
  }
}
#endif // _REENTRANT

static void dlopen_onsuccess(struct dso* dso, void* user_data) {
  struct async_data* data = (struct async_data*)user_data;
  dbg("dlopen_js_onsuccess: dso=%p mem_addr=%p mem_size=%zu",
      dso,
      dso->mem_addr,
      dso->mem_size);
  load_library_done(dso);
#ifdef _REENTRANT
  // Block until all other threads have loaded this module.
  dlsync();
#endif
  do_write_unlock();
  data->onsuccess(data->user_data, dso);
  free(data);
}

static void dlopen_onerror(struct dso* dso, void* user_data) {
  struct async_data* data = (struct async_data*)user_data;
  dbg("dlopen_js_onerror: dso=%p", dso);
  do_write_unlock();
  data->onerror(data->user_data);
  free(dso);
  free(data);
}

// Internal version of dlopen with typed return value.
// Without this, the compiler won't tell us if we have the wrong return type.
static struct dso* _dlopen(const char* file, int flags) {
  if (!file) {
    // If a null pointer is passed in path, dlopen() returns a handle equivalent
    // to RTLD_DEFAULT.
    dbg("dlopen: NULL -> %p", head->dso);
    return head->dso;
  }
  dbg("dlopen: %s [%d]", file, flags);

  int cs;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
  do_write_lock();
#ifdef _REENTRANT
  // Make sure we are in sync before performing any write operations.
  dlsync();
#endif

  struct dso* p;

  /* Search for the name to see if it's already loaded */
  for (struct dlevent* e = head; e; e = e->next) {
    if (e->sym_index == -1 && !strcmp(e->dso->name, file)) {
      dbg("dlopen: already opened: %p", e->dso);
      p = e->dso;
      goto end;
    }
  }

  p = load_library_start(file, flags);
  if (!p) {
    goto end;
  }
  void* success = _dlopen_js(p);
  if (!success) {
    dbg("dlopen_js: failed: %p", p);
    free(p);
    p = NULL;
    goto end;
  }
  dbg("dlopen_js: success: %p", p);
  load_library_done(p);
#ifdef _REENTRANT
  // Block until all other threads have loaded this module.
  dlsync();
#endif
end:
  dbg("dlopen(%s): done: %p", file, p);
  do_write_unlock();
  pthread_setcancelstate(cs, 0);
  return p;
}

void* dlopen(const char* file, int flags) {
  return _dlopen(file, flags);
}

void emscripten_dlopen(const char* filename, int flags, void* user_data,
                       em_dlopen_callback onsuccess, em_arg_callback_func onerror) {
  if (!filename) {
    onsuccess(user_data, head);
    return;
  }
  do_write_lock();
#ifdef _REENTRANT
  // Make sure we are in sync before performing any write operations.
  dlsync();
#endif
  struct dso* p = load_library_start(filename, flags);
  if (!p) {
    do_write_unlock();
    onerror(user_data);
    return;
  }

  // For async mode
  struct async_data* d = malloc(sizeof(struct async_data));
  d->user_data = user_data;
  d->onsuccess = onsuccess;
  d->onerror = onerror;

  dbg("calling emscripten_dlopen_js %p", p);
  // Unlock happens in dlopen_onsuccess/dlopen_onerror
  _emscripten_dlopen_js(p, dlopen_onsuccess, dlopen_onerror, d);
}

static void promise_onsuccess(void* user_data, void* handle) {
  em_promise_t p = (em_promise_t)user_data;
  dbg("promise_onsuccess: %p", p);
  emscripten_promise_resolve(p, EM_PROMISE_FULFILL, handle);
  emscripten_promise_destroy(p);
}

static void promise_onerror(void* user_data) {
  em_promise_t p = (em_promise_t)user_data;
  dbg("promise_onerror: %p", p);
  emscripten_promise_resolve(p, EM_PROMISE_REJECT, NULL);
  emscripten_promise_destroy(p);
}

// emscripten_dlopen_promise is currently implemented on top of the callback
// based API (emscripten_dlopen).
// TODO(sbc): Consider inverting this and perhaps deprecating/removing
// the old API.
em_promise_t emscripten_dlopen_promise(const char* filename, int flags) {
  // Create a promise that is resolved (and destroyed) once the operation
  // succeeds.
  em_promise_t p = emscripten_promise_create();
  emscripten_dlopen(filename, flags, p, promise_onsuccess, promise_onerror);

  // Create a second promise bound the first one to return the caller.  It's
  // then up to the caller to destroy this promise.
  em_promise_t ret = emscripten_promise_create();
  emscripten_promise_resolve(ret, EM_PROMISE_MATCH, p);
  return ret;
}

void* __dlsym(void* restrict p, const char* restrict s, void* restrict ra) {
  dbg("__dlsym dso:%p sym:%s", p, s);
  if (p != RTLD_DEFAULT && p != RTLD_NEXT && __dl_invalid_handle(p)) {
    return 0;
  }
  // The first "dso" is always the default one which is equivelent to
  // RTLD_DEFAULT.  This is what is returned from `dlopen(NULL, ...)`.
  if (p == head->dso) {
    p = RTLD_DEFAULT;
  }
  void* res;
  int sym_index = -1;
  do_write_lock();
  res = _dlsym_js(p, s, &sym_index);
  if (sym_index != -1) {
    new_dlevent(p, sym_index);
#ifdef _REENTRANT
    // Block until all other threads have loaded this module.
    dlsync();
#endif
  }
  dbg("__dlsym done dso:%p res:%p", p, res);
  do_write_unlock();
  return res;
}

int dladdr(const void* addr, Dl_info* info) {
  // report all function pointers as coming from this program itself XXX not
  // really correct in any way
  info->dli_fname = "unknown";
  info->dli_fbase = NULL;
  info->dli_sname = NULL;
  info->dli_saddr = NULL;
  return 1;
}
