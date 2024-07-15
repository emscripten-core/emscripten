#include <assert.h>

#include "emscripten_internal.h"

typedef struct set_cavas_size_t {
  const char* target;
  int width;
  int height;
} set_cavas_size_t;

static void do_set_size(void* arg) {
  set_cavas_size_t* args = (set_cavas_size_t*)arg;
  _emscripten_set_offscreencanvas_size(args->target, args->width, args->height);
  free(arg);
}

void _emscripten_set_offscreencanvas_size_on_thread(pthread_t t,
                                                    const char* target,
                                                    int width,
                                                    int height) {
  set_cavas_size_t* arg = malloc(sizeof(set_cavas_size_t));
  arg->target = target;
  arg->width = width;
  arg->height = height;

  em_proxying_queue* q = emscripten_proxy_get_system_queue();

  // Note: If we are also a pthread, the call below could theoretically be
  // done synchronously. However if the target pthread is waiting for a
  // mutex from us, then these two threads will deadlock. At the moment,
  // we'd like to consider that this kind of deadlock would be an Emscripten
  // runtime bug, although if emscripten_set_canvas_element_size() was
  // documented to require running an event in the queue of thread that owns
  // the OffscreenCanvas, then that might be ok.  (safer this way however)
  if (!emscripten_proxy_async(q, t, do_set_size, arg)) {
    assert(false && "emscripten_proxy_async failed");
  }
}
