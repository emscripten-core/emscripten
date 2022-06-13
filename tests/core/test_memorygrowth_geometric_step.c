#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emscripten/heap.h>

int main()
{
	size_t heapBefore = emscripten_get_heap_size();
	printf("Heap size before allocation: %zu\n", heapBefore);
	void *ptr = malloc(60*1024*1024);
	memset(ptr, 0x01, 60*1024*1024); // Avoid -O2 optimizing malloc() away
	printf("Ptr: %d, value: %d, Heap size now: %zu (increase: %zu bytes)\n", (int)!!ptr, ((int*)ptr)[rand()%65536], emscripten_get_heap_size(), emscripten_get_heap_size() - heapBefore);
}
