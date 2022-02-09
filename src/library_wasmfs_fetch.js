mergeInto(LibraryManager.library, {
  // Fetch backend: On first access of the file (either a read or a getSize), it
  // will fetch() the data from the network asynchronously. Otherwise, after
  // that fetch it behaves just like JSFile (and it reuses the code from there).

  _wasmfs_create_fetch_backend_js__deps: [
    '$wasmFS$backends',
    '$wasmFS$JSMemoryFiles',
    '_wasmfs_create_js_file_backend_js',
  ],
  _wasmfs_create_fetch_backend_js: function(backend) {
    // Get a promise that fetches the data and stores it in JS memory (if it has
    // not already been fetched).
    function getFile(file) {
      if (wasmFS$JSMemoryFiles[file]) {
        // The data is already here, so nothing to do before we continue on to
        // the actual read below.
        return Promise.resolve();
      }

      // This is the first time we want the file's data.
      // TODO: real URL!
      var url = 'data.dat';
      return fetch(url).then((response) => {
        if (!response['ok']) {
          throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
        }
        return response['arrayBuffer']();
      }).then((buffer) => {
        wasmFS$JSMemoryFiles[file] = new Uint8Array(buffer);
      });
    }

    // Start with the normal JSFile operations. This sets
    //   wasmFS$backends[backend]
    // which we will then augment.
    __wasmfs_create_js_file_backend_js(backend);

    // Add the async operations on top.
    var jsFileOps = wasmFS$backends[backend];
    wasmFS$backends[backend] = {
      // alloc/free operations are not actually async. Just forward to the
      // parent class, but we must return a Promise as the caller expects.
      allocFile: function(file) {
        jsFileOps.allocFile(file);
        return Promise.resolve();
      },
      freeFile: function(file) {
        jsFileOps.freeFile(file);
        return Promise.resolve();
      },

      write: function(file, buffer, length, offset) {
        abort("TODO: file writing in fetch backend? read-only for now");
      },

      // read/getSize fetch the data, then forward to the parent class.
      read: function(file, buffer, length, offset) {
        return getFile(file).then(() => {
          return jsFileOps.read(file, buffer, length, offset);
        });
      },
      getSize: function(file) {
        return getFile(file).then(() => {
          return jsFileOps.getSize(file);
        });
      },
    };
  },
});
