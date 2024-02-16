/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#include <emscripten/emscripten.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct asyncify_data_s {
  void *stack_ptr;     /** Current position in the Asyncify stack (*not* the C stack) */
  void *stack_limit;   /** Where the Asyncify stack ends. */
  int rewind_id;       /** Interned ID of the rewind entry point; opaque to application. */
} asyncify_data_t;

typedef struct emscripten_fiber_s {
  void *stack_base;             /** Where the C stack starts (NOTE: grows down). */
  void *stack_limit;            /** Where the C stack ends. */
  void *stack_ptr;              /** Current position in the C stack. */
  em_arg_callback_func entry;   /** Function to call when resuming this context. If NULL, asyncify_data is used to rewind the call stack. */
  void *user_data;              /** Opaque pointer, passed as-is to the entry function. */
  asyncify_data_t asyncify_data;
} emscripten_fiber_t;

void emscripten_fiber_init(
  emscripten_fiber_t *fiber __attribute__((nonnull)),
  em_arg_callback_func entry_func,
  void *entry_func_arg,
  void *c_stack __attribute__((nonnull)),
  size_t c_stack_size,
  void *asyncify_stack __attribute__((nonnull)),
  size_t asyncify_stack_size
);

void emscripten_fiber_init_from_current_context(
  emscripten_fiber_t *fiber __attribute__((nonnull)),
  void *asyncify_stack __attribute__((nonnull)),
  size_t asyncify_stack_size
);

void emscripten_fiber_swap(
  emscripten_fiber_t *old_fiber __attribute__((nonnull)),
  emscripten_fiber_t *new_fiber __attribute__((nonnull))
);

#ifdef __cplusplus
}
#endif
