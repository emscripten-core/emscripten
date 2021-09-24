var WasmfsLibrary = {
$WASMFS: {
  buffers: [null, [], []], // 1 => stdout, 2 => stderr
  printChar: function(stream, curr) {
    if (curr === 0 || curr === 10) {
      (stream === 1 ? out : err)(UTF8ArrayToString(WASMFS.buffers[stream], 0));
      WASMFS.buffers[stream].length = 0;
    } else {
      WASMFS.buffers[stream].push(curr);
    }
    
    return
  },
}
}

mergeInto(LibraryManager.library, WasmfsLibrary);

DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$WASMFS');
