/*
 * Copyright 2021 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include "em_types.h"

#ifdef __cplusplus
extern "C" {
#endif

void emscripten_unwind_to_js_event_loop(void) __attribute__((noreturn));

long emscripten_set_timeout(void (*cb)(void *userData), double msecs, void *userData);
void emscripten_clear_timeout(long setTimeoutId);
void emscripten_set_timeout_loop(EM_BOOL (*cb)(double time, void *userData), double intervalMsecs, void *userData);

long emscripten_set_immediate(void (*cb)(void *userData), void *userData);
void emscripten_clear_immediate(long setImmediateId);
void emscripten_set_immediate_loop(EM_BOOL (*cb)(void *userData), void *userData);

long emscripten_set_interval(void (*cb)(void *userData), double intervalMsecs, void *userData);
void emscripten_clear_interval(long setIntervalId);

#ifdef __cplusplus
}
#endif
