/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>

extern void getDomElementParentInnerHTML(const char *domElement, char *dst, int size);
extern int isThisInWorker(void);
extern int isThisInWorkerOnMainThread(void);
extern int receivesAndReturnsAnInteger(int i);

// Define this if compiling via -s PROXY_TO_PTHREAD=1
// #define PROXY_TO_PTHREAD 1

int main()
{
	char dst[256];
	char name[7] = "resize";
	getDomElementParentInnerHTML(name, dst, sizeof(dst));
	memset(name, 0, sizeof(name)); // Try to uncover if there might be a race condition and above line was not synchronously processed, and we could take name string away.
	int inWorker1 = isThisInWorker(); // Build this application with -s USE_PTHREADS=1 -s PROXY_TO_PTHREAD=1 for this to return 1, otherwise returns 0.
	int inWorker2 = isThisInWorkerOnMainThread(); // This should always return 0
	int returnedInt = receivesAndReturnsAnInteger(4);
	printf("text: \"%s\". inWorker1: %d, inWorker2: %d, returnedInt: %d\n", dst, inWorker1, inWorker2, returnedInt);
	assert(strstr(dst, "Resize canvas"));
	assert(inWorker1 == PROXY_TO_PTHREAD);
	assert(inWorker2 == 0);
	assert(returnedInt == 42 + 4);
#ifdef REPORT_RESULT
	REPORT_RESULT(1);
#endif
}
