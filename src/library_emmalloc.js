mergeInto(LibraryManager.library, {
  emmalloc_unclaimed_heap_memory__deps: ['emscripten_get_sbrk_ptr'],
  emmalloc_unclaimed_heap_memory: function() {
  	var dynamicTop = HEAPU32[_emscripten_get_sbrk_ptr()>>2];
#if ALLOW_MEMORY_GROWTH
#if WASM
#if WASM_MEM_MAX != -1
	// Using WASM_MEM_MAX to constrain max heap size.
	return {{{ WASM_MEM_MAX }}} - dynamicTop;
#else
	// Not using a Wasm memory bound.
	return 2*1024*1024*1024 - 65536 - dynamicTop;
#endif
#else
	// asm.js:
	return 2*1024*1024*1024 - 16777216 - dynamicTop;
#endif
#else
	// ALLOW_MEMORY_GROWTH is disabled, the current heap size
	// is all we got.
	return HEAPU8.length - dynamicTop;
#endif
  }
});
