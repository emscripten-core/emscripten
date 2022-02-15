mergeInto(LibraryManager.library, {
  // JSFile backend: Store a file's data in JS. We map File objects in C++ to
  // entries here that contain typed arrays.
  $wasmFS$JSMemoryFiles: {},

  _wasmfs_create_js_file_backend_js__deps: [
    '$wasmFS$backends',
    '$wasmFS$JSMemoryFiles',
  ],
  _wasmfs_create_js_file_backend_js: function(backend) {
    wasmFS$backends[backend] = {
      allocFile: (file) => {
        // Do nothing: we allocate the typed array lazily, see write()
      },
      freeFile: (file) => {
        // Release the memory, as it now has no references to it any more.
        wasmFS$JSMemoryFiles[file] = undefined;
      },
      write: (file, buffer, length, offset) => {
        try {
          if (!wasmFS$JSMemoryFiles[file]) {
            // Initialize typed array on first write operation.
            wasmFS$JSMemoryFiles[file] = new Uint8Array(offset + length);
          }

          if (offset + length > wasmFS$JSMemoryFiles[file].length) {
            // Resize the typed array if the length of the write buffer exceeds its capacity.
            var oldContents = wasmFS$JSMemoryFiles[file];
            var newContents = new Uint8Array(offset + length);
            newContents.set(oldContents);
            wasmFS$JSMemoryFiles[file] = newContents;
          }

          wasmFS$JSMemoryFiles[file].set(HEAPU8.subarray(buffer, buffer + length), offset);
          return 0;
        } catch (err) {
          return {{{ cDefine('EIO') }}};
        }
      },
      read: (file, buffer, length, offset) => {
        try {
          HEAPU8.set(wasmFS$JSMemoryFiles[file].subarray(offset, offset + length), buffer);
          return 0;
        } catch (err) {
          return {{{ cDefine('EIO') }}};
        }
      },
      getSize: (file) => {
        return wasmFS$JSMemoryFiles[file] ? wasmFS$JSMemoryFiles[file].length : 0;
      },
    };
  },
});
