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

void emscripten_unwind_to_js_event_loop(void) __attribute__((__noreturn__));

int emscripten_set_timeout(void (*cb)(void *user_data), double msecs, void *user_data);
void emscripten_clear_timeout(int id);
void emscripten_set_timeout_loop(EM_BOOL (*cb)(double time, void *user_data), double interval_ms, void *user_data);

int emscripten_set_immediate(void (*cb)(void *user_data), void *user_data);
void emscripten_clear_immediate(int id);
void emscripten_set_immediate_loop(EM_BOOL (*cb)(void *user_data), void *user_data);

int emscripten_set_interval(void (*cb)(void *user_data), double interval_ms, void *user_data);
void emscripten_clear_interval(int id);

void emscripten_runtime_keepalive_push();
void emscripten_runtime_keepalive_pop();
EM_BOOL emscripten_runtime_keepalive_check();

int emscripten_promise_create(void (*start_async)(void* user_data, int promise_id), void* user_data);
void emscripten_promise_resolve(int promise_id, void* value);
void emscripten_promise_reject(int promise_id);

#ifdef __cplusplus
}
#endif
