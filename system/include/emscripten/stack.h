/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

// API that gives access to introspecting the Wasm data stack. Build with
// -lstack.js to use this API.

#ifdef __cplusplus
extern "C" {
#endif

// Returns the starting address of the wasm stack. This is the address
// that the stack pointer would point to when no bytes are in use on the stack.
uintptr_t emscripten_stack_get_base(void);

// Returns the end address of the wasm stack. This is the address that the stack
// pointer would point to when the whole stack is in use.  (the address pointed
// to by the end is not part of the stack itself) Note that in fastcomp, the
// stack grows up, whereas in wasm backend, it grows down.  So with wasm
// backend, the address returned by emscripten_stack_get_end() is smaller than
// emscripten_stack_get_base().
uintptr_t emscripten_stack_get_end(void);

// Returns the current stack pointer.
uintptr_t emscripten_stack_get_current(void);

// Returns the number of free bytes left on the stack.
size_t emscripten_stack_get_free(void);

#ifdef __cplusplus
}
#endif
