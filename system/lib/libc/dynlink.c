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
#include <fcntl.h>
#include <pthread.h>
#include <threads.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <emscripten/console.h>
#include <emscripten/threading.h>
#include <emscripten/promise.h>
#include <emscripten/proxying.h>

#include "dynlink.h"
#include "pthread_impl.h"
#include "emscripten_internal.h"

//#define DYLINK_DEBUG

#ifdef DYLINK_DEBUG
#define dbg(fmt, ...) emscripten_dbgf(fmt, ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif

struct async_data {
  em_dlopen_callback onsuccess;
  em_arg_callback_func onerror;
  void* user_data;
};

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
static thread_local struct dlevent* thread_local_tail = &main_event;
static pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;
static thread_local bool skip_dlsync = false;

static void dlsync();

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
#ifdef _REENTRANT
  // Block until all other threads have loaded this module.
  dlsync();
#endif
  // TODO: figure out some way to tell when its safe to free p->file_data.  Its
  // not safe to do here because some threads could have been alseep then when
  // the "dlsync" occurred and those threads will synchronize when they wake,
  // which could be an arbitrarily long time in the future.
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

  // If the file exists in the filesystem, load it here into linear memory which
  // makes the data available to JS, and to other threads.  This data gets
  // free'd later once all threads have loaded the DSO.
  struct stat statbuf;
  if (stat(name, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
    int fd = open(name, O_RDONLY);
    if (fd >= 0) {
      off_t size = lseek(fd, 0, SEEK_END);
      if (size != (off_t)-1) {
        lseek(fd, 0, SEEK_SET);
        p->file_data = malloc(size);
        if (p->file_data) {
          if (read(fd, p->file_data, size) == size) {
            p->file_data_size = size;
          } else {
            free(p->file_data);
          }
        }
      }
      close(fd);
    }
  }

  return p;
}

#ifdef _REENTRANT
// When we are attempting to synchronize loaded libraries between threads we
// currently abort, rather than rejecting the promises.  We could reject the
// promises, and attempt to return an error from the original dlopen() but we
// would have to also unwind the state on all the threads that were able to load
// the module.
#define ABORT_ON_SYNC_FAILURE 1

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
      emscripten_errf("_dlsym_catchup_js failed: %s", dlerror());
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
        emscripten_errf("_dlsym_catchup_js failed: %s", dlerror());
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
        emscripten_errf("_dlopen_js failed: %s", dlerror());
        return false;
      }
    }
    thread_local_tail = p;
  }
  dbg("_emscripten_dlsync_self: done");
  return true;
}

struct promise_result {
  em_promise_t promise;
  bool result;
};

static void do_thread_sync(void* arg) {
  dbg("do_thread_sync");
  struct promise_result* info = arg;
  info->result = _emscripten_dlsync_self();
}

static void do_thread_sync_out(void* arg) {
  dbg("do_thread_sync_out");
  int* result = (int*)arg;
  *result = _emscripten_dlsync_self();
}

// Called when a thread exists prior to being able to completely sync operation.
// We can just ignore this case and report success.
static void thread_sync_cancelled(void* arg) {
  struct promise_result* info = arg;
  dbg("thread_sync_cancelled: promise=%p result=%i", info->promise, info->result);
  emscripten_promise_resolve(info->promise, EM_PROMISE_FULFILL, NULL);
  emscripten_promise_destroy(info->promise);
  free(info);
}

// Called once do_thread_sync completes
static void thread_sync_done(void* arg) {
  struct promise_result* info = arg;
  em_promise_t promise = info->promise;
  dbg("thread_sync_done: promise=%p result=%i", promise, info->result);
  if (info->result) {
    emscripten_promise_resolve(promise, EM_PROMISE_FULFILL, NULL);
  } else {
#if ABORT_ON_SYNC_FAILURE
    abort();
#else
    emscripten_promise_reject(promise);
#endif
  }
  emscripten_promise_destroy(promise);
  free(info);
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

  struct promise_result* info = malloc(sizeof(struct promise_result));
  if (!info) {
    return false;
  }
  *info = (struct promise_result){
    .promise = promise,
    .result = false,
  };
  int rtn = emscripten_proxy_callback(dlopen_proxying_queue,
                                      target_thread,
                                      do_thread_sync,
                                      thread_sync_done,
                                      thread_sync_cancelled,
                                      info);
  if (!rtn) {
    // If we failed to proxy, then the target thread is no longer alive and no
    // longer needs to be caught up, so we can resolve the promise early.
    emscripten_promise_resolve(promise, EM_PROMISE_FULFILL, NULL);
    emscripten_promise_destroy(promise);
    free(info);
  } else if (target_thread->sleeping) {
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
  dbg("dlsync main=%p", emscripten_main_runtime_thread_id());
  if (emscripten_is_main_runtime_thread()) {
    // dlsync was called on the main thread.  In this case we have no choice by
    // to run the blocking version of emscripten_dlsync_threads.
    _emscripten_dlsync_threads();
  } else {
    // Otherwise we block here while the asynchronous version runs in the main
    // thread.
    em_proxying_queue* q = emscripten_proxy_get_system_queue();
    int success = emscripten_proxy_sync_with_ctx(
      q, emscripten_main_runtime_thread_id(), run_dlsync_async, pthread_self());
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

// Modified version of path_open from musl/ldso/dynlink.c
static int path_find(const char *name, const char *s, char *buf, size_t buf_size) {
  size_t l;
  int fd;
  for (;;) {
    s += strspn(s, ":\n");
    l = strcspn(s, ":\n");
    if (l-1 >= INT_MAX) return -1;
    if (snprintf(buf, buf_size, "%.*s/%s", (int)l, s, name) < buf_size) {
      dbg("dlopen: path_find: %s", buf);
      struct stat statbuf;
      if (stat(buf, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
        return 0;
      }
      switch (errno) {
      case ENOENT:
      case ENOTDIR:
      case EACCES:
      case ENAMETOOLONG:
        break;
      default:
        dbg("dlopen: path_find failed: %s", strerror(errno));
        /* Any negative value but -1 will inhibit
         * futher path search. */
        return -2;
      }
    }
    s += l;
  }
}

// Resolve filename using LD_LIBRARY_PATH
static const char* resolve_path(char* buf, const char* file, size_t buflen) {
  if (!strchr(file, '/')) {
    const char* env_path = getenv("LD_LIBRARY_PATH");
    if (env_path && path_find(file, env_path, buf, buflen) == 0) {
      dbg("dlopen: found in LD_LIBRARY_PATH: %s", buf);
      return buf;
    }
  }
  return file;
}

// Search for library name to see if it's already loaded
static struct dso* find_existing(const char* file) {
  for (struct dlevent* e = head; e; e = e->next) {
    if (e->sym_index == -1 && !strcmp(e->dso->name, file)) {
      dbg("dlopen: already opened: %p", e->dso);
      return e->dso;
    }
  }
  return NULL;
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

  char buf[2*NAME_MAX+2];
  file = resolve_path(buf, file, sizeof buf);

  struct dso* p = find_existing(file);
  if (p) {
    goto end;
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
  dbg("emscripten_dlopen: %s", filename);
  if (!filename) {
    onsuccess(user_data, head->dso);
    return;
  }
  do_write_lock();
  char buf[2*NAME_MAX+2];
  filename = resolve_path(buf, filename, sizeof buf);
  struct dso* p = find_existing(filename);
  if (p) {
    onsuccess(user_data, p);
    return;
  }
  p = load_library_start(filename, flags);
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
