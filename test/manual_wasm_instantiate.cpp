// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <emscripten/emscripten.h>

int main()
{
	printf("OK\n");
#ifdef REPORT_RESULT
	int result = EM_ASM_INT({return Module.testWasmInstantiationSucceeded;});
	REPORT_RESULT(result);
#endif
}
