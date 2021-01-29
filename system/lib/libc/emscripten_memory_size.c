#include <stddef.h>

size_t emscripten_get_heap_size()
{
	return __builtin_wasm_memory_size(0) << 16;
}