/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

#include <inttypes.h>
#include <stddef.h>

// API that gives access to introspecting the Wasm data stack.

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

// Setup internal base/end values based on the initial values that were either
// set at compile time (in static linking) or instantiations time (for dynamic
// linking).
void emscripten_stack_init(void);

// Sets the internal values reported by emscripten_stack_get_base and
// emscripten_stack_get_end.  This should only used by low level libraries
// such as asyncify fibres.
void emscripten_stack_set_limits(void* base, void* end);

// Returns the current stack pointer.
uintptr_t emscripten_stack_get_current(void);

// Returns the number of free bytes left on the stack.  This is required to be
// fast so that it can be called frequently.
size_t emscripten_stack_get_free(void);

#ifdef __cplusplus
}
#endif
