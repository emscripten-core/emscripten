/*
 * Copyright 2020 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>

volatile int* addr = (int*)0xfffffff;

EMSCRIPTEN_KEEPALIVE void crash() {
  printf("crashing\n");
  *addr = 1337;
}

EM_JS(void, unhandled_exception_wrapper, (void), {
  try {
	// Crash the program
	_crash();
  }
  catch(e) {
	// Catch the abort
	out(true);
  }
  
  out("again");
  
  try {
    // Try executing some function again
	_crash();
  }
  catch(e) {
	// Make sure it failed with the expected exception
	out(e === "program has already aborted!" || e === "DEAD");
  }
});

int main() {
  unhandled_exception_wrapper();
  return 0;
} 

