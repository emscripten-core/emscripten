/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <features.h>

static void dummy(double now) {
}

weak_alias(dummy, _emscripten_check_timers);

void _emscripten_yield(double now) {
  _emscripten_check_timers(now);
}
