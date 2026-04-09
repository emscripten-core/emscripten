#include <stdbool.h>
#include <sched.h>
#include <threads.h>

#include <emscripten/threading.h>

#include "syscall.h"
#include "threading_internal.h"

static _Atomic pthread_t crashed_thread_id = NULL;

void _emscripten_thread_crashed() {
  // Notify the main thread that the calling thread has crashed. The will bring
  // down the whole program next time the main thread calls `_emscripten_yield`.
  crashed_thread_id = pthread_self();
  // Force the main runtime thread to wake up in case it is waiting in
  // `_emscripten_thread_notify`.
  _emscripten_thread_notify(emscripten_main_runtime_thread_id());
}

static void dummy(double now)
{
}

weak_alias(dummy, _emscripten_check_timers);

void _emscripten_yield(double now) {
  // When a secondary thread crashes, we need to be able to interrupt the main
  // thread even if it's in a blocking/looping on a mutex.  We want to avoid
  // using the normal proxying mechanism to send this message since it can
  // allocate (or otherwise itself crash) so use a low level atomic primitive
  // for this signal.
  if (emscripten_is_main_runtime_thread()) {
    if (crashed_thread_id) {
      // Mark the crashed thread as strongly referenced so that Node.js doesn't
      // exit while the pthread is propagating the uncaught exception back to
      // the main thread.
      _emscripten_thread_set_strongref(crashed_thread_id);
      // Return the event loop so we can handle the message from the crashed
      // thread.
      emscripten_exit_with_live_runtime();
    }

    // This is no-op in programs that don't include use of itimer/alarm.
    _emscripten_check_timers(now);

    // Assist other threads by executing proxied operations that are effectively
    // singlethreaded.
    emscripten_main_thread_process_queued_calls();
  }
#ifdef EMSCRIPTEN_DYNAMIC_LINKING
  else {
    _emscripten_process_dlopen_queue();
  }
#endif
}
