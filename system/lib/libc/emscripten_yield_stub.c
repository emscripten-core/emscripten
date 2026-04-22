/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <features.h>
#include <emscripten/threading.h>

#include "threading_internal.h"

static bool dummy(double now) {
  return false;
}

weak_alias(dummy, _emscripten_check_timers);

bool _emscripten_yield(double now) {
  if (emscripten_is_main_runtime_thread()) {
    return _emscripten_check_timers(now);
  }
  return false;
}
