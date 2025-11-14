/*
 * Copyright 2023 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */
#include <assert.h>
#include <string.h>
#include <emscripten/html5.h>

#include "emscripten_internal.h"

typedef bool (*event_callback)(int event_type, void *event_data __attribute__((nonnull)), void *user_data);

typedef struct callback_args_t {
  event_callback callback;
  int event_type;
  void *user_data;
  uint8_t event_data[];
} callback_args_t;

static void do_callback(void* arg) {
  callback_args_t* args = (callback_args_t*)arg;
  args->callback(args->event_type, args->event_data, args->user_data);
  free(arg);
}

void _emscripten_run_callback_on_thread(pthread_t t,
                                        event_callback f,
                                        int event_type,
                                        void* event_data,
                                        size_t event_data_size,
                                        void* user_data) {
  em_proxying_queue* q = emscripten_proxy_get_system_queue();
  callback_args_t* arg = malloc(sizeof(callback_args_t) + event_data_size);
  arg->callback = f;
  arg->event_type = event_type;
  arg->user_data = user_data;
  memcpy(arg->event_data, event_data, event_data_size);

  if (!emscripten_proxy_async(q, t, do_callback, arg)) {
    assert(false && "emscripten_proxy_async failed");
  }
}

static EMSCRIPTEN_RESULT _get_last_event(void* out, void *latest, size_t len) {
  if (!latest) {
    return EMSCRIPTEN_RESULT_NO_DATA;
  }
  memcpy(out, latest, len);
  return EMSCRIPTEN_RESULT_SUCCESS;
}

EMSCRIPTEN_RESULT emscripten_get_deviceorientation_status(EmscriptenDeviceOrientationEvent *out) {
  return _get_last_event(out, _emscripten_get_last_deviceorientation_event(), sizeof(*out));
}

EMSCRIPTEN_RESULT emscripten_get_devicemotion_status(EmscriptenDeviceMotionEvent *out) {
  return _get_last_event(out, _emscripten_get_last_devicemotion_event(), sizeof(*out));
}

EMSCRIPTEN_RESULT emscripten_get_mouse_status(EmscriptenMouseEvent *out) {
  return _get_last_event(out, _emscripten_get_last_mouse_event(), sizeof(*out));
}
