/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <assert.h>
#include <emscripten/html5.h>

#include "emscripten_internal.h"

typedef bool (*event_callback)(int event_type, void *event_data __attribute__((nonnull)), void *user_data);

typedef struct callback_args_t {
  event_callback callback;
  int event_type;
  void *event_data;
  void *user_data;
} callback_args_t;

static void do_callback(void* arg) {
  callback_args_t* args = (callback_args_t*)arg;
  args->callback(args->event_type, args->event_data, args->user_data);
}

void _emscripten_run_callback_on_thread(pthread_t t,
                                        event_callback f,
                                        int event_type,
                                        void* event_data,
                                        void* user_data) {
  em_proxying_queue* q = emscripten_proxy_get_system_queue();
  callback_args_t arg = {
    .callback = f,
    .event_type = event_type,
    .event_data = event_data,
    .user_data = user_data,
  };

  if (!emscripten_proxy_sync(q, t, do_callback, &arg)) {
    assert(false && "emscripten_proxy_sync failed");
  }
}
