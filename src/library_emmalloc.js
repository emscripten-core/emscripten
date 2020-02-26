mergeInto(LibraryManager.library, {
  emmalloc_unclaimed_heap_memory__deps: ['emscripten_get_sbrk_ptr'],
  emmalloc_unclaimed_heap_memory: function() {
  	var dynamicTop = HEAPU32[_emscripten_get_sbrk_ptr()>>2];
#if ALLOW_MEMORY_GROWTH
#if WASM
#if MAXIMUM_MEMORY != -1
	// Using MAXIMUM_MEMORY to constrain max heap size.
	return {{{ MAXIMUM_MEMORY }}} - dynamicTop;
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
