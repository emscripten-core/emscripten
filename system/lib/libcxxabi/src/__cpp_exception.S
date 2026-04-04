/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Define the `__cpp_exception` Wasm EH tag which is used to implmenent C++
 * exception handling in LLVM.
 */

#ifdef __wasm_exception_handling__

#ifdef __wasm64__
#define PTR i64
#else
#define PTR i32
#endif

.globl __cpp_exception
.tagtype __cpp_exception PTR
__cpp_exception:

#endif // !__wasm_exception_handling__
