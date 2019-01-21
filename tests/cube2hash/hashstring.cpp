// Copyright 2012 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

#include "cube2crypto.h"
#include <stdlib.h>
#include <stdio.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

void help()
{
	printf("Usage: hashstring <seed>\n");
}

int main(int argc, char **argv)
{
	if(argc != 2 || !argv[1])
	{
		help();
		return EXIT_FAILURE;
	}

	char *answer = cube2crypto_hashstring(argv[1]);

	printf("hash value: %s\n", answer);

	free(answer);

	return EXIT_SUCCESS;
}
