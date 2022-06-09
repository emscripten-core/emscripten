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

//#define DYLINK_DEBUG

void _dlinit(struct dso* main_dso_handle);
void* _dlopen_js(struct dso* handle);
void* _dlsym_js(struct dso* handle, const char* symbol);
void _emscripten_dlopen_js(struct dso* handle,
                           em_arg_callback_func onsuccess,
                           em_arg_callback_func onerror);
void __dl_vseterr(const char*, va_list);

static struct dso * _Atomic head, * _Atomic tail;
static thread_local struct dso* thread_local_tail;
static pthread_rwlock_t lock;

static void error(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  __dl_vseterr(fmt, ap);
  va_end(ap);
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
#ifdef DYLINK_DEBUG
  fprintf(stderr, "%p: load_library_done: dso=%p mem_addr=%p mem_size=%d table_addr=%p table_size=%d\n", pthread_self(), p, p->mem_addr, p->mem_size, p->table_addr, p->table_size);
#endif

  // insert into linked list
  p->prev = tail;
  if (tail) {
    tail->next = p;
  }
  tail = p;
  thread_local_tail = p;

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

static void dlopen_js_onsuccess(void* handle) {
  struct dso* p = (struct dso*)handle;
#ifdef DYLINK_DEBUG
  fprintf(stderr, "%p: dlopen_js_onsuccess: dso=%p mem_addr=%p mem_size=%p\n", pthread_self(), p, p->mem_addr, p->mem_size);
#endif
  load_library_done(p);
  pthread_rwlock_unlock(&lock);
  p->onsuccess(p->user_data, p);
}

static void dlopen_js_onerror(void* handle) {
  struct dso* p = (struct dso*)handle;
#ifdef DYLINK_DEBUG
  fprintf(stderr, "%p: dlopen_js_onerror: dso=%p\n", pthread_self(), handle);
#endif
  pthread_rwlock_unlock(&lock);
  p->onerror(p->user_data);
  free(p);
}

// This function is called at the start of all entry points so that the dso
// list gets initialized on first use.
static void ensure_init() {
  if (head) {
    return;
  }
  // Initialize the dso list.  This happens on first run.
  pthread_rwlock_wrlock(&lock);
  if (!head) {
    // Flags are not important since the main module is already loaded.
    struct dso* p = load_library_start("__main__", RTLD_NOW|RTLD_GLOBAL);
    assert(p);
    _dlinit(p);
    load_library_done(p);
    assert(head);
  }
  pthread_rwlock_unlock(&lock);
}

void* dlopen(const char* file, int flags) {
  ensure_init();
  if (!file) {
    return head;
  }
#ifdef DYLINK_DEBUG
  fprintf(stderr, "%p: dlopen: %s [%d]\n", pthread_self(), file, flags);
#endif

  struct dso* p;
  int cs;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
  pthread_rwlock_wrlock(&lock);

  /* Search for the name to see if it's already loaded */
  for (p = head; p; p = p->next) {
    if (!strcmp(p->name, file)) {
#ifdef DYLINK_DEBUG
      fprintf(stderr, "%p: dlopen: already opened: %p\n", pthread_self(), p);
#endif
      goto end;
    }
  }

  p = load_library_start(file, flags);
  if (!p) {
    goto end;
  }
  void* success = _dlopen_js(p);
  if (!success) {
#ifdef DYLINK_DEBUG
    fprintf(stderr, "%p: dlopen_js: failed\n", pthread_self(), p);
#endif
    free(p);
    p = NULL;
    goto end;
  }
#ifdef DYLINK_DEBUG
  fprintf(stderr, "%p: dlopen_js: success: %p\n", pthread_self(), p);
#endif
  load_library_done(p);
end:
  pthread_rwlock_unlock(&lock);
  pthread_setcancelstate(cs, 0);
  return p;
}

void emscripten_dlopen(const char* filename, int flags, void* user_data,
                       em_dlopen_callback onsuccess, em_arg_callback_func onerror) {
  ensure_init();
  if (!filename) {
    onsuccess(user_data, head);
    return;
  }
  pthread_rwlock_wrlock(&lock);
  struct dso* p = load_library_start(filename, flags);
  if (!p) {
    pthread_rwlock_unlock(&lock);
    onerror(user_data);
    return;
  }
  p->user_data = user_data;
  p->onsuccess = onsuccess;
  p->onerror = onerror;
#ifdef DYLINK_DEBUG
  fprintf(stderr, "%p: calling emscripten_dlopen_js %p\n", pthread_self(), p);
#endif
  // Unlock happens in dlopen_js_onsuccess/dlopen_js_onerror
  _emscripten_dlopen_js(p, dlopen_js_onsuccess, dlopen_js_onerror);
}

void* __dlsym(void* restrict p, const char* restrict s, void* restrict ra) {
#ifdef DYLINK_DEBUG
  fprintf(stderr, "%p: __dlsym dso:%p sym:%s\n", pthread_self(), p, s);
#endif
  if (p != RTLD_DEFAULT && p != RTLD_NEXT && __dl_invalid_handle(p)) {
    return 0;
  }
  void* res;
  pthread_rwlock_rdlock(&lock);
  res = _dlsym_js(p, s);
  pthread_rwlock_unlock(&lock);
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

#ifdef _REENTRANT
void _emscripten_thread_sync_code() {
  // This function is called from emscripten_yeild which itself is called
  // whenever we block on a futex.  We need to check to avoid infinite
  // recursion when taking the lock below.
  static thread_local bool syncing = false;
  if (syncing) {
    return;
  }
  syncing = true;
  ensure_init();
  if (thread_local_tail == tail) {
#ifdef DYLINK_DEBUG
    fprintf(stderr, "%p: emscripten_thread_sync_code: already in sync\n", pthread_self());
#endif
    goto done;
  }
  pthread_rwlock_rdlock(&lock);
  if (!thread_local_tail) {
    thread_local_tail = head;
  }
  while (thread_local_tail->next) {
    struct dso* p = thread_local_tail->next;
#ifdef DYLINK_DEBUG
    fprintf(stderr, "%p: emscripten_thread_sync_code: %s mem_addr=%p mem_size=%d table_addr=%p table_size=%d\n", pthread_self(), p->name, p->mem_addr, p->mem_size, p->table_addr, p->table_size);
#endif
    void* success = _dlopen_js(p);
    if (!success) {
      fprintf(stderr, "dlerror: %s\n", dlerror());
    }
    assert(success);
    thread_local_tail = p;
  }
  pthread_rwlock_unlock(&lock);
#ifdef DYLINK_DEBUG
  fprintf(stderr, "%p: emscripten_thread_sync_code done\n", pthread_self());
#endif

done:
  syncing = false;
}
#endif
