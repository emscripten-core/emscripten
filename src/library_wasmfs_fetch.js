mergeInto(LibraryManager.library, {
  // Fetch backend: On first read, does a fetch() to get the data. It then
  // behaves like JSFile

  _wasmfs_create_fetch_backend_js__deps: [
    '$wasmFS$backends',
    '$wasmFS$JSMemoryFiles',
    '_wasmfs_create_js_file_backend_js',
  ],
  _wasmfs_create_fetch_backend_js: function(backend) {
console.log('maek fetch', backend);
    // Start with the normal JSFile operations.
    __wasmfs_create_js_file_backend_js(backend);

    // Add the async operations on top.
    var jsFileOps = wasmFS$backends[backend];
    wasmFS$backends[backend] = {
      allocFile: function(file) {
        jsFileOps.allocFile(file);
        return new Promise.resolve();
      },
      freeFile: function(file) {
        jsFileOps.freeFile(file);
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
          var url = 'data.dat';
          if (typeof fetch === 'function') {
            // On the web use fetch() normally.
            promise = fetch().then((response) => {
              if (!response['ok']) {
                throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
              }
              return response['arrayBuffer']();
            });
          } else {
            // Until Node.js gets fetch support, use an async read.
            promise = fs.readFile(url, "binary");
          }
          promise.then((buffer) => {
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
