/*
 * Copyright 2015 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

/*
 * Stubs for exceptions code, when exceptions are disabled.
 */

extern "C" {

int __cxa_uncaught_exceptions() {
  return 0;
}

}
