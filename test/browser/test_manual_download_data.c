// Copyright 2017 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <assert.h>

int main()
{
	FILE *handle = fopen("file.txt", "r");
	char str[128] = {};
	fread(str, 1, sizeof(str), handle);
	printf("str: %s\n", str);
	int success = (strcmp(str, "Hello!") == 0);
	assert(success);
	printf("OK\n");
	return 0;
}
