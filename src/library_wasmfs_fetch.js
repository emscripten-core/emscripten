mergeInto(LibraryManager.library, {
  // Fetch backend: On first read, does a fetch() to get the data. It then
  // behaves like JSFile

  _wasmfs_backend_add_fetch__deps: [
    '$wasmFS$backends',
    '$wasmFS$JSMemoryFiles',
    '_wasmfs_backend_add_js_file',
  ],
  _wasmfs_backend_add_fetch: function(backend) {
    // Start with the normal JSFile operations.
    __wasmfs_backend_add_js_file(backend);

    // Add the async operations on top.
    var jsFileOps = wasmFS$backends[backend];
    wasmFS$backends[backend] = {
      alloc_file: function(file) {
        jsFileOps.alloc_file(file);
        return new Promise.resolve();
      },
      free_file: function(file) {
        jsFileOps.free_file(file);
        return new Promise.resolve();
      },
      write: function(file, buffer, length, offset) {
        abort();
      },
      read: function(file, buffer, length, offset) {
        var promise;
        if (!wasmFS$JSMemoryFiles[file]) {
          // This is the first read from this file, fetch it.
          // TODO: URL!
          promise = fetch('data.dat').then((response) =>
            if (!response['ok']) {
              throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
            }
            return response['arrayBuffer']();
          }).then((buffer) => {
            wasmFS$JSMemoryFiles[file] = new Uint8Array(buffer);
          });
        } else {
          // The data is already here, so nothing to do before we continue on to
          // the actual read below.
          promise = Promise.resolve();
        }

        // Do the actual read.
        return promise.then(() => {
          return jsFileOps.read(file, buffer, length, offset);
        });
      },
      getSize: function(file) {
        return new Promise.resolve(jsFileOps.getSize(file));
      },
    };
  },
});
