/*
 * Copyright 2018 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

// Extra libc helper functions

char *tzname[2];

void* _get_tzname() {
  return (void*)tzname;
}

int daylight;

int* _get_daylight() {
  return &daylight;
}

long timezone;

long* _get_timezone() {
  return &timezone;
}

// Musl lock internals. As we assume wasi is single-threaded for now, these
// are no-ops.

void __lock(void* ptr) {}
void __unlock(void* ptr) {}

