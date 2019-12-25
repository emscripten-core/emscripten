/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdint.h>

#include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EM_FIBER_ASYNCIFY_STACK_SIZE 4096

typedef struct asyncify_fiber_s {
  void *stack_base;
  void *stack_limit;
  void *stack_ptr;
  em_arg_callback_func entry;
  void *user_data;
} asyncify_fiber_t;

typedef struct asyncify_data_s {
  void *stack_ptr;
  void *stack_limit;
  int rewind_id;
} asyncify_data_t;

typedef struct emscripten_fiber_s {
  asyncify_fiber_t asyncify_fiber;
  asyncify_data_t asyncify_data;
  char asyncify_stack[EM_FIBER_ASYNCIFY_STACK_SIZE];
} emscripten_fiber_t;

void emscripten_fiber_init(emscripten_fiber_t *fiber, size_t fiber_struct_size, em_arg_callback_func entry_func, void *entry_func_arg, void *stack, size_t stack_size);

void emscripten_fiber_init_from_current_context(emscripten_fiber_t *fiber, size_t fiber_struct_size);

void emscripten_fiber_swap(emscripten_fiber_t *old_fiber, emscripten_fiber_t *new_fiber);

#ifdef __cplusplus
}
#endif
