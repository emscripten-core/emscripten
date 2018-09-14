/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <emscripten.h>
#include <assert.h>

int main()
{
	int ENVIRONMENT_IS_WORKER = EM_ASM_INT(return ENVIRONMENT_IS_WORKER);
	printf("ENVIRONMENT_IS_WORKER: %d\n", ENVIRONMENT_IS_WORKER);
	assert(ENVIRONMENT_IS_WORKER);
#ifdef REPORT_RESULT
	REPORT_RESULT(1);
#endif
}
