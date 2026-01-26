/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Define the `__c_longjmp` Wasm EH tag which is used to implment setjmp/longjmp
 * in LLVM.
 */

#ifdef __wasm_exception_handling__

#ifdef __wasm64__
#define PTR i64
#else
#define PTR i32
#endif

.globl __c_longjmp
.tagtype __c_longjmp PTR
__c_longjmp:

#endif // !__wasm_exception_handling__
