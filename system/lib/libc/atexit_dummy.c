/*
 * Copyright 2022 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Stub implementations of atexit function.  These will be included
// in favor of the regular ones in system/lib/libc/musl/src/exit/atexit.c
// when EXIT_RUNTIME == 0.

#include <stdlib.h>

int atexit(void (*function)(void)) { return 0; }

int __cxa_atexit(void (*func)(void *), void *arg, void *dso) { return 0; }

void __cxa_finalize(void *dso) { }
