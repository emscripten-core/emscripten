var memoryProfiler = {
  emscripten_memprof_sbrk_grow: (old_brk, new_brk) => {
#if MEMORYPROFILER
    emscriptenMemoryProfiler.onSbrkGrow(old_brk, new_brk);
#endif
  },
};

addToLibrary(memoryProfiler);
