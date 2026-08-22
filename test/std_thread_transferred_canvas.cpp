#include <atomic>
#include <thread>
#include <stdio.h>

#include <GLES2/gl2.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/threading.h>

static std::atomic<bool> g_done = false;
static std::atomic<bool> g_ok = false;

static void thread_main() {
  EmscriptenWebGLContextAttributes attr;
  emscripten_webgl_init_context_attributes(&attr);
  attr.explicitSwapControl = true;

  EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context("#canvas", &attr);
  if (ctx > 0 && emscripten_webgl_make_context_current(ctx) == EMSCRIPTEN_RESULT_SUCCESS) {
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    emscripten_webgl_commit_frame();
    emscripten_webgl_make_context_current(0);
    emscripten_webgl_destroy_context(ctx);
    g_ok = true;
  }

  g_done = true;
}

static void poll_done(void*) {
  if (!g_done) {
    emscripten_async_call(poll_done, nullptr, 20);
    return;
  }
  emscripten_force_exit(g_ok ? 0 : 1);
}

int main() {
  if (!emscripten_supports_offscreencanvas()) {
    printf("Current browser does not support OffscreenCanvas. Skipping this test.\n");
    return 0;
  }

  // The new API is intended for std::thread users that cannot pass
  // pthread_attr_t into thread construction.
  emscripten_set_next_thread_transferredcanvases("#canvas");

  std::thread worker(thread_main);
  worker.detach();

  emscripten_async_call(poll_done, nullptr, 20);
  emscripten_exit_with_live_runtime();
  __builtin_trap();
}
