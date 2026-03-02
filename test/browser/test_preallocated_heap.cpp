// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main() {
	void *ptr1 = malloc(24*1024*1024);
	printf("ptr1: %p\n", ptr1);
	assert(ptr1 != 0);
	void *ptr2 = malloc(24*1024*1024);
	printf("ptr2: %p\n", ptr2);
	assert(ptr2 == 0);
	return 0;
}
