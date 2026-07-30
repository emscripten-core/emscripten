/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <assert.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef bool (*event_callback)(int event_type, void *event_data, void *user_data);
extern void _emscripten_run_callback_on_thread(pthread_t t,
                                               event_callback f,
                                               int event_type,
                                               void *event_data,
                                               size_t event_data_size,
                                               void *user_data);

static bool on_wheel(int event_type, void *event_data, void *user_data) {
  const EmscriptenWheelEvent *e = (const EmscriptenWheelEvent *)event_data;
  // Test that the event_data contents are properly aligned.  These accesses
  // will fail under SAFE_HEAP if they are not.
  assert(e->deltaX == 1.0);
  assert(e->deltaY == 2.0);
  assert(e->deltaZ == 3.0);
  printf("done\n");
  emscripten_force_exit(0);
  return true;
}

static void *sender(void *arg) {
  pthread_t target = (pthread_t)arg;
  EmscriptenWheelEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.deltaX = 1.0;
  ev.deltaY = 2.0;
  ev.deltaZ = 3.0;
  _emscripten_run_callback_on_thread(target, on_wheel, EMSCRIPTEN_EVENT_WHEEL,
                                     &ev, sizeof(ev), NULL);
  return NULL;
}

static void dummy(void) {}

int main(void) {
  pthread_t t;
  pthread_create(&t, NULL, sender, (void *)pthread_self());
  emscripten_set_main_loop(dummy, 1, 0);
}
