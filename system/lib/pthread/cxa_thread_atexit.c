/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 * Inspired by libcxxabi/src/cxa_thread_atexit.cpp.
 * The main reasons we don't use that version direclty are:
 * 1. We want to be able to use __cxa_thread_atexit in pure C programs
 *    where libcxxabi is not linked in at all.
 * 2. The libcxxabi relies on TLS variables, which we can't use here.
 *
 * WARNING: We specifically can't use TLS variables or pthread_once in this file
 * because they both end up calling __cxa_thread_atexit.
 *
 * TLS variables: TLS data gets freed using __cxa_thread_atexit.  This means
 * that if we use TLS to store our DtorList it will be free'd while porcessing
 * itself.  See lib/pthread/emscripten_tls_init.c.
 *
 * pthread_once: calls pthread_cleanup_push / pthread_cleanup_pop, which call
 * __cxa_thread_atexit on first use.  See src/thread/pthread_once.c.
 */
#include <assert.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdlib.h>

#include "libc.h"

typedef void (*Dtor)(void*);

typedef struct DtorList {
  Dtor dtor;
  void* obj;
  struct DtorList* next;
} DtorList;

void run_dtors(void* arg) {
  DtorList* dtors = (DtorList*)arg;
  DtorList* head;
  while ((head = dtors)) {
    dtors = head->next;
    head->dtor(head->obj);
    free(head);
  }
}

static pthread_key_t key;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static void ensure_key() {
  // See above for why we can't use pthread_once.
  pthread_mutex_lock(&mutex);
  static atomic_bool key_created = false;
  if (!key_created) {
    pthread_key_create(&key, run_dtors);
    key_created = true;
  }
  pthread_mutex_unlock(&mutex);
}

int __cxa_thread_atexit_impl(Dtor dtor, void* obj, void* dso_symbol) {
  ensure_key();
  DtorList* old_head = pthread_getspecific(key);
  DtorList* head = (DtorList*)(malloc(sizeof(DtorList)));
  assert(head);
  head->dtor = dtor;
  head->obj = obj;
  head->next = old_head;
  pthread_setspecific(key, head);
  return 0;
}

weak_alias(__cxa_thread_atexit_impl, __cxa_thread_atexit);
