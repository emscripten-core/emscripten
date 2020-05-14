mergeInto(LibraryManager.library, {
	emscripten_stack_get_base: function() {
		return STACK_BASE;
	},
	emscripten_stack_get_end: function() {
		return STACK_MAX;
	},

#if !WASM_BACKEND
	// In wasm backend, these functions are implemented
	// locally in wasm module code as stackSave()/
	// stackGetFree() functions.
	emscripten_stack_get_current__asm: true,
	emscripten_stack_get_current__sig: 'i',
	emscripten_stack_get_current: function() {
		return STACKTOP|0;
	},
	emscripten_stack_get_free__asm: true,
	emscripten_stack_get_free__sig: 'i',
	emscripten_stack_get_free: function() {
		return (STACK_MAX|0) - (STACKTOP|0);
	}
#endif
});
