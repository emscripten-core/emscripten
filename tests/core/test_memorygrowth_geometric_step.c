#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emscripten/heap.h>

int main()
{
	void *ptr = malloc(60*1024*1024);
	memset(ptr, 0x01, 60*1024*1024); // Avoid -O2 optimizing malloc() away
	printf("Ptr: %d, value: %d, Heap size: %zu\n", (int)!!ptr, ((int*)ptr)[rand()%65536], emscripten_get_heap_size());
}
