var memoryProfiler = {
  emscripten_memprof_sbrk_grow: (old_brk, new_brk) => {
#if MEMORYPROFILER
    Module['onSbrkGrow']?.(old_brk, new_brk, new Error().stack.toString());
#endif
  },
};

addToLibrary(memoryProfiler);
