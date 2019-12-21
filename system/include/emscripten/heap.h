#pragma once

#define ASMJS_PAGE_SIZE 16777216
#define WASM_PAGE_SIZE 65536

#ifdef __wasm__
#define EMSCRIPTEN_PAGE_SIZE WASM_PAGE_SIZE
#else
#define EMSCRIPTEN_PAGE_SIZE ASMJS_PAGE_SIZE
#endif

#ifdef __cplusplus
extern "C" {
#endif

intptr_t *emscripten_get_sbrk_ptr(void);
int emscripten_resize_heap(size_t requested_size);
size_t emscripten_get_heap_size(void);

#ifdef __cplusplus
}
#endif
