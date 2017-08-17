#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

// Without noinline, Clang optimizes this whole application to a while(1) {} infinite loop.
void * __attribute__((noinline)) leak_alloc(uint64_t i)
{
	return malloc(i);
}

int main()
{
	// Keep allocating in ~powers of two until we fail, then print out how much we got.
	uint64_t availableMemory = 0;
	uint64_t i = 0x80000000ULL;
	while(i > 0)
	{
		void *ptr = leak_alloc(i);
		if (ptr) availableMemory += i;
		else i >>= 1;
	}
	printf("Total memory available: %llu\n", availableMemory);
#ifdef REPORT_RESULT
	int result = (availableMemory > 10*1024*1024);
	REPORT_RESULT(result);
#endif
}
