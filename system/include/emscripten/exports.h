/*
 * Copyright 2012 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#pragma once

// API that gives access to the function exports of a Wasm module. Build with
// -lexports.js to use this API.

#ifdef __cplusplus
extern "C" {
#endif

// Returns a function pointer to the given exported function by name. Cast the returned pointer
// to its proper signature before calling the function.
void *emscripten_get_exported_function(const char *fname);

#ifdef __cplusplus
}
#endif
