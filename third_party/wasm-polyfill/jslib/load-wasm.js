// This file implements a browser utility function to asychronously fetch,
// decode and compile a given WebAssembly module. The caller passes in the url
// of the .wasm file and the returned promise resolves to a compiled (but
// unlinked) module.

var loadWebAssembly = (function() {
  var globalNameCounter = 0;
  return function(packedURL, workerURL) {
    workerURL = workerURL || 'load-wasm-worker.js';

    return new Promise(function(resolve, reject) {
      var callbackName = "";
      do {
        callbackName = "onFinishLoadWebAssembly_" + globalNameCounter++;
      } while (Object.hasOwnProperty(window, callbackName));
      window[callbackName] = function(asmModule) {
        delete window[callbackName];
        resolve(asmModule);
      }

      var worker = new Worker(workerURL);
      worker.postMessage({url:packedURL, callbackName:callbackName});
      worker.onmessage = function (e) {
        if (!(e.data instanceof Blob)) {
          reject("load-wasm-worker.js failed with: " + e.data);
          return;
        }

        var script = document.createElement('script');
        var url = URL.createObjectURL(e.data);
        script.onload = script.onerror = function() { URL.revokeObjectURL(url) }
        script.src = url;
        document.body.appendChild(script);
      }
    });
  }
})();
