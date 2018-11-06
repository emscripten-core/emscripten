// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <emscripten.h>
#include <stdio.h>

int main()
{
	// Test that on main browser thread, MAIN_THREAD_ASYNC_EM_ASM() will get
	// synchronously executed.
	printf("Before MAIN_THREAD_ASYNC_EM_ASM\n");
	MAIN_THREAD_ASYNC_EM_ASM(out('Inside MAIN_THREAD_ASYNC_EM_ASM: ' + $0 + ' ' + $1), 42, 3.5);
	printf("After MAIN_THREAD_ASYNC_EM_ASM\n");
}
