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

//#define DYLINK_DEBUG

#ifdef DYLINK_DEBUG
#define dbg(fmt, ...) _emscripten_errf("%p: " fmt, pthread_self(), ##__VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif

struct async_data {
  em_dlopen_callback onsuccess;
  em_arg_callback_func onerror;
  void* user_data;
};
typedef void (*dlopen_callback_func)(struct dso*, struct async_data* user_data);

void _dlinit(struct dso* main_dso_handle);
void* _dlopen_js(struct dso* handle);
void* _dlsym_js(struct dso* handle, const char* symbol);
void _emscripten_dlopen_js(struct dso* handle,
                           dlopen_callback_func onsuccess,
                           dlopen_callback_func onerror,
                           struct async_data* user_data);
void __dl_vseterr(const char*, va_list);

static struct dso * _Atomic head, * _Atomic tail;

#ifdef _REENTRANT
static thread_local struct dso* thread_local_tail;
static pthread_mutex_t write_lock = PTHREAD_MUTEX_INITIALIZER;

static void dlsync() {
  if (!thread_local_tail) {
    thread_local_tail = head;
  }
  if (!thread_local_tail->next) {
    return;
  }
  dbg("dlsync: catching up %p %p", thread_local_tail, tail);
  while (thread_local_tail->next) {
    struct dso* p = thread_local_tail->next;
    dbg("dlsync: %s mem_addr=%p "
        "mem_size=%zu table_addr=%p table_size=%zu",
        p->name,
        p->mem_addr,
        p->mem_size,
        p->table_addr,
        p->table_size);
    void* success = _dlopen_js(p);
    if (!success) {
      // If any on the libraries fails to load here then we give up.
      // TODO(sbc): Ideally this would never happen and we could/should
      // abort, but on the main thread (where we don't have sync xhr) its
      // often not possible to syncronously load side module.
      _emscripten_errf("_dlopen_js failed: %s", dlerror());
      break;
    }
    thread_local_tail = p;
  }
  dbg("dlsync: done");
}

// This function is called from emscripten_yield which itself is called whenever
// we block on a futex.  We need to check to avoid infinite recursion when
// taking the lock below.
static thread_local bool skip_dlsync = false;

static void ensure_init();

void _emscripten_thread_sync_code() {
  if (skip_dlsync) {
    return;
  }
  ensure_init();
  if (!thread_local_tail) {
    thread_local_tail = head;
  }
  if (thread_local_tail != tail) {
    skip_dlsync = true;
    dlsync();
    skip_dlsync = false;
  }
}

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
#else
#define do_write_lock()
#define do_write_unlock()
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
  struct dso* p;
  for (p = head; p; p = p->next)
    if (h == p)
      return 0;
  error("Invalid library handle %p", (void*)h);
  return 1;
}

static void load_library_done(struct dso* p) {
  dbg("load_library_done: dso=%p mem_addr=%p mem_size=%zu "
      "table_addr=%p table_size=%zu",
      p,
      p->mem_addr,
      p->mem_size,
      p->table_addr,
      p->table_size);

#ifdef _REENTRANT
  thread_local_tail = p;
#endif

  // insert into linked list
  p->prev = tail;
  if (tail) {
    tail->next = p;
  }
  tail = p;

  if (!head) {
    head = p;
  }
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

static void dlopen_js_onsuccess(struct dso* dso, struct async_data* data) {
  dbg("dlopen_js_onsuccess: dso=%p mem_addr=%p mem_size=%zu",
      dso,
      dso->mem_addr,
      dso->mem_size);
  load_library_done(dso);
  do_write_lock();
  data->onsuccess(data->user_data, dso);
  free(data);
}

static void dlopen_js_onerror(struct dso* dso, struct async_data* data) {
  dbg("dlopen_js_onerror: dso=%p", dso);
  do_write_unlock();
  data->onerror(data->user_data);
  free(dso);
  free(data);
}

// This function is called at the start of all entry points so that the dso
// list gets initialized on first use.
static void ensure_init() {
  if (head) {
    return;
  }
  // Initialize the dso list.  This happens on first run.
  do_write_lock();
  if (!head) {
    // Flags are not important since the main module is already loaded.
    struct dso* p = load_library_start("__main__", RTLD_NOW|RTLD_GLOBAL);
    assert(p);
    _dlinit(p);
    load_library_done(p);
    assert(head);
  }
  do_write_unlock();
}

void* dlopen(const char* file, int flags) {
  ensure_init();
  if (!file) {
    return head;
  }
  dbg("dlopen: %s [%d]", file, flags);

  struct dso* p;
  int cs;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
  do_write_lock();
#ifdef _REENTRANT
  // Make sure we are in sync before loading any new DSOs.
  dlsync();
#endif

  /* Search for the name to see if it's already loaded */
  for (p = head; p; p = p->next) {
    if (!strcmp(p->name, file)) {
      dbg("dlopen: already opened: %p", p);
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
end:
  do_write_unlock();
  pthread_setcancelstate(cs, 0);
  dbg("dlopen: %s done", file);
  return p;
}

void emscripten_dlopen(const char* filename, int flags, void* user_data,
                       em_dlopen_callback onsuccess, em_arg_callback_func onerror) {
  ensure_init();
  if (!filename) {
    onsuccess(user_data, head);
    return;
  }
  do_write_lock();
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
  // Unlock happens in dlopen_js_onsuccess/dlopen_js_onerror
  _emscripten_dlopen_js(p, dlopen_js_onsuccess, dlopen_js_onerror, d);
}

void* __dlsym(void* restrict p, const char* restrict s, void* restrict ra) {
  dbg("__dlsym dso:%p sym:%s", p, s);
  if (p != RTLD_DEFAULT && p != RTLD_NEXT && __dl_invalid_handle(p)) {
    return 0;
  }
  void* res;
  res = _dlsym_js(p, s);
  dbg("__dlsym done dso:%p res:%p", p, res);
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
