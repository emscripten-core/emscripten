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

    function getFile(file) {
      if (wasmFS$JSMemoryFiles[file]) {
        // The data is already here, so nothing to do before we continue on to
        // the actual read below.
        return Promise.resolve();
      }

      // This is the first time we want the file's data.
      // TODO: URL!
      var url = 'data.dat';
      var promise;
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
        console.log('node1');
        promise = new Promise((resolve, reject) => {
          console.log('node2');
          fs.readFile(url, "binary", (err, data) => {
            console.log('node3', err, data);
            if (err) reject();
            resolve(data);
          });
        });
      }
      return promise.then((buffer) => {
        wasmFS$JSMemoryFiles[file] = new Uint8Array(buffer);
      });
    }

    // Add the async operations on top.
    var jsFileOps = wasmFS$backends[backend];
    wasmFS$backends[backend] = {
      allocFile: function(file) {
        jsFileOps.allocFile(file);
        return Promise.resolve();
      },
      freeFile: function(file) {
        jsFileOps.freeFile(file);
        return Promise.resolve();
      },
      write: function(file, buffer, length, offset) {
        abort();
      },
      read: function(file, buffer, length, offset) {
        return getFile(file).then(() => {
          return jsFileOps.read(file, buffer, length, offset);
        });
      },
      getSize: function(file) {
        return getFile(file).then(() => {
          var ret = jsFileOps.getSize(file);
          console.log('sizeey', ret);
          return ret;
        });
      },
    };
  },
});
