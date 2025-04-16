// Copyright 2022 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten/fiber.h>
#include <emscripten/stack.h>

void emscripten_fiber_init(
    emscripten_fiber_t *fiber,
    em_arg_callback_func entry_func,
    void *entry_func_arg,
    void *c_stack,
    size_t c_stack_size,
    void *asyncify_stack,
    size_t asyncify_stack_size
) {
    char *c_stack_base = (char*)c_stack + c_stack_size;
    fiber->stack_base = c_stack_base;
    fiber->stack_limit = c_stack;
    fiber->stack_ptr = c_stack_base;
    fiber->entry = entry_func;
    fiber->user_data = entry_func_arg;
    fiber->asyncify_data.stack_ptr = asyncify_stack;
    fiber->asyncify_data.stack_limit = (char*)asyncify_stack + asyncify_stack_size;
}

void emscripten_fiber_init_from_current_context(
    emscripten_fiber_t *fiber,
    void *asyncify_stack,
    size_t asyncify_stack_size
) {
    fiber->stack_base = (void*)emscripten_stack_get_base();
    fiber->stack_limit = (void*)emscripten_stack_get_end();
    fiber->entry = NULL;
    fiber->asyncify_data.stack_ptr = asyncify_stack;
    fiber->asyncify_data.stack_limit = (char*)asyncify_stack + asyncify_stack_size;
}
