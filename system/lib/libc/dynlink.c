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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dynlink.h>

//#define DYLINK_DEBUG

extern void* _dlopen_js(struct dso* handle, int mode);
extern void* _dlsym_js(struct dso* handle, const char* symbol);
extern void _emscripten_dlopen_js(struct dso* handle, int flags,
                                  em_arg_callback_func onsuccess,
                                  em_arg_callback_func onerror);

static struct dso *head, *tail;
static pthread_rwlock_t lock;

void __dl_vseterr(const char*, va_list);

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
  strcpy(p->name, name);

  return p;
}

static void dlopen_js_onsuccess(void* handle) {
  struct dso* p = (struct dso*)handle;
#ifdef DYLINK_DEBUG
  printf("dlopen_js_onsuccess: dso=%p\n", p);
#endif
  load_library_done(p);
  pthread_rwlock_unlock(&lock);
  p->onsuccess(p->user_data, p);
}

static void dlopen_js_onerror(void* handle) {
  struct dso* p = (struct dso*)handle;
#ifdef DYLINK_DEBUG
  printf("dlopen_js_onsuccess: dso=%p\n", p);
#endif
  pthread_rwlock_unlock(&lock);
  p->onerror(p->user_data);
  free(p);
}

static void init_dso_list() {
  // Initialize the dso list.  This happens on first run.
  pthread_rwlock_wrlock(&lock);
  if (!head) {
    // Flags are not important since the main module is already loaded.
    int flags = RTLD_NOW|RTLD_GLOBAL;
    struct dso* p = load_library_start("__main__", flags);
    assert(p);
    void* success = _dlopen_js(p, flags);
    assert(success);
    load_library_done(p);
    assert(head);
  }
  pthread_rwlock_unlock(&lock);
}

void* dlopen(const char* file, int flags) {
  if (!head) {
    init_dso_list();
  }
  if (!file) {
    return head;
  }
#ifdef DYLINK_DEBUG
  printf("dlopen: %s [%d]\n", file, flags);
#endif

  struct dso* p;
  int cs;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
  pthread_rwlock_wrlock(&lock);

  /* Search for the name to see if it's already loaded */
  for (p = head; p; p = p->next) {
    if (!strcmp(p->name, file)) {
#ifdef DYLINK_DEBUG
      printf("dlopen: already opened: %p\n", p);
#endif
      goto end;
    }
  }

  p = load_library_start(file, flags);
  if (!p) {
    goto end;
  }
  void* success = _dlopen_js(p, flags);
  if (!success) {
#ifdef DYLINK_DEBUG
    printf("dlopen_js: failed\n", p);
#endif
    free(p);
    p = NULL;
    goto end;
  }
#ifdef DYLINK_DEBUG
  printf("dlopen_js: success: %p\n", p);
#endif
  load_library_done(p);
end:
  pthread_rwlock_unlock(&lock);
  pthread_setcancelstate(cs, 0);
  return p;
}

void emscripten_dlopen(const char* filename, int flags, void* user_data,
                       em_dlopen_callback onsuccess, em_arg_callback_func onerror) {
  if (!head) {
    init_dso_list();
  }
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
  printf("calling emscripten_dlopen_js %p\n", p);
#endif
  // Unlock happens in dlopen_js_onsuccess/dlopen_js_onerror
  _emscripten_dlopen_js(p, flags, dlopen_js_onsuccess, dlopen_js_onerror);
}

void* __dlsym(void* restrict p, const char* restrict s, void* restrict ra) {
#ifdef DYLINK_DEBUG
  printf("__dlsym dso:%p sym:%s\n", p, s);
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
