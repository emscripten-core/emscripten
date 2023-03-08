/*
 * Copyright 2017 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

extern const char *getDomElementContents(const char *domElementSelector);
extern int isThisInWorker(void);
extern int isThisInWorkerOnMainThread(void);
extern int receivesAndReturnsAnInteger(int i);

// Define this if compiling via -sPROXY_TO_PTHREAD
// #define PROXY_TO_PTHREAD 1

int main()
{
	char name[7] = "body";
	const char *dst = getDomElementContents(name);
	memset(name, 0, sizeof(name)); // Try to uncover if there might be a race condition and above line was not synchronously processed, and we could take name string away.
	int inWorker1 = isThisInWorker(); // Build this application with -pthread -sPROXY_TO_PTHREAD for this to return 1, otherwise returns 0.
	int inWorker2 = isThisInWorkerOnMainThread(); // This should always return 0
	int returnedInt = receivesAndReturnsAnInteger(4);
	printf("text: \"%s\". inWorker1: %d, inWorker2: %d, returnedInt: %d\n", dst, inWorker1, inWorker2, returnedInt);
	assert(strstr(dst, "<canvas"));
	free((void*)dst);
	assert(inWorker1 == PROXY_TO_PTHREAD);
	assert(inWorker2 == 0);
	assert(returnedInt == 42 + 4);
	return 0;
}
