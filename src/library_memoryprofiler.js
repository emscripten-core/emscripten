var memoryProfiler = {
  emscripten_memprof_sbrk_grow: function(old_brk, new_brk) {
#if MEMORYPROFILER
    emscriptenMemoryProfiler.onSbrkGrow(old_brk, new_brk);
#endif
  },
};

mergeInto(LibraryManager.library, memoryProfiler);
