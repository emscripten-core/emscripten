/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>

/* We had a regression where the stack position was not taking into account
 * the data and bss.  This test runs with 1024 byte stack which given that bug
 * would put the end of the bss after the entire stack region */
#define BSS_BYTES (10 * 1024)

int static_data = 1;
int static_bss[BSS_BYTES];

int main(int argc, char* argv[]) {
  int stack_var = 1;
  int* data_address = &static_data;
  int* bss_address = &static_bss[BSS_BYTES-1];
  int* stack_address = &stack_var;
  printf("data: %p bss: %p stack: %p\n", data_address, bss_address, stack_address);
  assert(stack_address > bss_address);
  assert(bss_address > data_address);
  printf("success.\n");
  return 0;
}
