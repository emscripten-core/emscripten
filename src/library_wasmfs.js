var WasmfsLibrary = {
  $wasmfsBuffers: [null, [], []],
  emscripten_wasmfs_printbuffer__deps: ['$wasmfsBuffers'],
  emscripten_wasmfs_printbuffer: function(stream, ptr, len) {
    for (var j = 0; j < len; j++) {
      if (HEAPU8[ptr+j] === 0 || HEAPU8[ptr+j] === 10) {
        (stream === 1 ? out : err)(UTF8ArrayToString(wasmfsBuffers[stream], 0));
        wasmfsBuffers[stream].length = 0;
      } else {
        wasmfsBuffers[stream].push(HEAPU8[ptr+j]);
      }
    }
    
  }
}

mergeInto(LibraryManager.library, WasmfsLibrary);
