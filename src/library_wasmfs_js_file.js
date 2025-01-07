addToLibrary({
  // JSFile backend: Store a file's data in JS. We map File objects in C++ to
  // entries here that contain typed arrays.
  $wasmFS$JSMemoryFiles: {},

  _wasmfs_create_js_file_backend_js__deps: [
    '$wasmFS$backends',
    '$wasmFS$JSMemoryFiles',
  ],
  _wasmfs_create_js_file_backend_js: (backend) => {
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
          return length;
        } catch (err) {
          return -{{{ cDefs.EIO }}};
        }
      },
      read: (file, buffer, length, offset) => {
        var fileData = wasmFS$JSMemoryFiles[file];
        // We can't read past the end of the file's data.
        var dataAfterOffset = Math.max(0, fileData.length - offset);
        // We only read as much as we were asked.
        length = Math.min(length, dataAfterOffset);
        HEAPU8.set(fileData.subarray(offset, offset + length), buffer);
        return length;
      },
      getSize: (file) => wasmFS$JSMemoryFiles[file]?.length || 0,
      setSize: (file, size) => {
        // Allocate a new array of the proper size, and copy as much data as
        // possible.
        var old = wasmFS$JSMemoryFiles[file];
        var new_ = wasmFS$JSMemoryFiles[file] = new Uint8Array(size);
        if (old) {
          new_.set(old.subarray(0, size));
        }
        return 0;
      }
    };
  },
});
