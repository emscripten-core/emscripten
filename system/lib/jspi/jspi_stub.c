/*
 * Copyright 2026 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * Linked in place of jspi.c when JSPI_HOOKS is off: no events are delivered.
 */

#include <emscripten/jspi.h>

int jspi_register(jspi_hook fn, uint32_t mask) { return -1; }
