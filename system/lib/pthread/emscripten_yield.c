#include <stdbool.h>
#include <sched.h>
#include <threads.h>

#include <emscripten/threading.h>

#include "syscall.h"
#include "threading_internal.h"

static _Atomic bool thread_crashed = false;

void _emscripten_thread_crashed() {
  thread_crashed = true;
}

static void dummy(double now)
{
}

weak_alias(dummy, _emscripten_check_timers);

void _emscripten_yield(double now) {
  int is_runtime_thread = emscripten_is_main_runtime_thread();

  // When a secondary thread crashes, we need to be able to interrupt the main
  // thread even if it's in a blocking/looping on a mutex.  We want to avoid
  // using the normal proxying mechanism to send this message since it can
  // allocate (or otherwise itself crash) so use a low level atomic primitive
  // for this signal.
  if (is_runtime_thread) {
    if (thread_crashed) {
      // Return the event loop so we can handle the message from the crashed
      // thread.
      emscripten_unwind_to_js_event_loop();
    }

    // This is no-op in programs that don't include use of itimer/alarm.
    _emscripten_check_timers(now);

    // Assist other threads by executing proxied operations that are effectively
    // singlethreaded.
    emscripten_main_thread_process_queued_calls();
  }
#ifdef __PIC__
  else {
    _emscripten_process_dlopen_queue();
  }
#endif
}
