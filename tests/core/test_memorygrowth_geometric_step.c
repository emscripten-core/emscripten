#include <stdio.h>
#include <stdlib.h>

#include <emscripten/heap.h>

int main()
{
	malloc(60*1024*1024);
	printf("Heap size: %zu\n", emscripten_get_heap_size());
}
