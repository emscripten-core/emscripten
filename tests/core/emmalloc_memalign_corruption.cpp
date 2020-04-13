#include <emscripten/emmalloc.h>
#include <stdio.h>
#include <string.h>

int main()
{
	void *ptr = malloc(20);
	void *ptr2 = malloc(30);
	memset(ptr2, 0xFF, 30);
	free(ptr2);
	printf("%d %d\n", ptr != 0, emmalloc_validate_memory_regions());
	ptr2 = memalign(32, 20);
	printf("%d %d\n", ptr2 != 0, emmalloc_validate_memory_regions());
	free(ptr2);
	printf("%d\n", emmalloc_validate_memory_regions());
	free(ptr);
	printf("%d\n", emmalloc_validate_memory_regions());
}
