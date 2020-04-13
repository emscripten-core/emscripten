// Copyright 2016 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

// Tests that we can use the dlmalloc mallinfo() function to obtain information about malloc()ed blocks and compute how much memory is used/freed.

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <emscripten/emscripten.h>

struct s_mallinfo {
	int arena;    /* non-mmapped space allocated from system */
	int ordblks;  /* number of free chunks */
	int smblks;   /* always 0 */
	int hblks;    /* always 0 */
	int hblkhd;   /* space in mmapped regions */
	int usmblks;  /* maximum total allocated space */
	int fsmblks;  /* always 0 */
	int uordblks; /* total allocated space */
	int fordblks; /* total free space */
	int keepcost; /* releasable (via malloc_trim) space */
};

extern "C" {
	extern s_mallinfo mallinfo();
}

unsigned int getTotalMemory()
{
	return EM_ASM_INT(return HEAP8.length);
}

unsigned int getFreeMemory()
{
	s_mallinfo i = mallinfo();
	unsigned int totalMemory = getTotalMemory();
	unsigned int dynamicTop = EM_ASM_INT(return HEAPU32[DYNAMICTOP_PTR>>2]);
	return totalMemory - dynamicTop + i.fordblks;
}

int main()
{
	printf("Before allocation:\n");
	printf("Total memory: %u bytes\n", getTotalMemory());
	printf("Free memory: %u bytes\n", getFreeMemory());
	printf("Used: %u bytes (%.2f%%)\n", getTotalMemory() - getFreeMemory(), (getTotalMemory() - getFreeMemory()) * 100.0 / getTotalMemory());
	assert(getTotalMemory() == 16777216);
	assert(getFreeMemory() >= 10000000); // 11529552 in test

	void *ptr = malloc(1024*1024);
	printf("\nAfter 1MB allocation:\n");
	printf("Total memory: %u bytes\n", getTotalMemory());
	printf("Free memory: %u bytes\n", getFreeMemory());
	printf("Used: %u bytes (%.2f%%)\n", getTotalMemory() - getFreeMemory(), (getTotalMemory() - getFreeMemory()) * 100.0 / getTotalMemory());
	assert(getTotalMemory() == 16777216);
	assert(getFreeMemory() >= 9000000); // 10480968 in test

	free(ptr);
	printf("\nAfter freeing:\n");
	printf("Total memory: %u bytes\n", getTotalMemory());
	printf("Free memory: %u bytes\n", getFreeMemory());
	printf("Used: %u bytes (%.2f%%)\n", getTotalMemory() - getFreeMemory(), (getTotalMemory() - getFreeMemory()) * 100.0 / getTotalMemory());
	assert(getTotalMemory() == 16777216);
	assert(getFreeMemory() >= 10000000); // 11529552 in test

	printf("OK.\n");
}
