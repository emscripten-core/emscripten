#include <emscripten/emmalloc.h>
#include <stdio.h>

int main()
{
	void *ptr = malloc(1);
	printf("%zu\n", malloc_usable_size(ptr));
}
