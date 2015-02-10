mergeInto(LibraryManager.library, {
  emscripten_trace_record_allocation: function (address, size) {
    memProf.mallocProxy(address, size);
  },
  emscripten_trace_record_free: function (address) {
    memProf.freeProxy(address);
  },
});

