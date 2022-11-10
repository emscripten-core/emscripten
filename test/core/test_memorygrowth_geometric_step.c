#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emscripten/memory.h>

int main()
{
	size_t memBefore = emscripten_memory_get_size();
	printf("Mem size before allocation: %zu\n", memBefore);
	void *ptr = malloc(60*1024*1024);
	memset(ptr, 0x01, 60*1024*1024); // Avoid -O2 optimizing malloc() away
	printf("Ptr: %d, value: %d, Heap size now: %zu (increase: %zu bytes)\n", (int)!!ptr, ((int*)ptr)[rand()%65536], emscripten_memory_get_size(), emscripten_memory_get_size() - memBefore);
}
