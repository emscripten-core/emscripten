/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <unistd.h>
#include <stdatomic.h>

#include "emscripten_internal.h"

// In case the stub syscall is not linked it
static int dummy_getpid(void) {
  return 42;
}
weak_alias(dummy_getpid, __syscall_getpid);

static _Atomic pid_t next_tid = 0;

pid_t _emscripten_get_next_tid() {
  // Create threads with monotonically increasing TID starting with the main
  // thread which has TID == PID.
  if (next_tid == 0) {
    // Use CAS to initialize next_tid so that one thread will end up
    // initializing it.
    pid_t expected = 0;
    atomic_compare_exchange_strong(&next_tid, &expected, getpid() + 1);
  }
  return next_tid++;
}
