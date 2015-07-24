// This file implements a browser utility function to asychronously fetch,
// decode and compile a given WebAssembly module. The caller passes in the url
// of the .wasm file and the returned promise resolves to a compiled (but
// unlinked) module.

var loadWebAssembly = (function() {
  // *** Environment setup code ***
  var ENVIRONMENT_IS_WEB = typeof window === 'object';
  var ENVIRONMENT_IS_WORKER = typeof importScripts === 'function';
  var ENVIRONMENT_IS_NODE = typeof process === 'object' && typeof require === 'function' && !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_WORKER;
  var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

  var Module = {};
  function assert(x, y) {
    if (!x) throw 'assert failed: ' + y + ' at ' + new Error().stack;
  }

  if (ENVIRONMENT_IS_NODE) {
    Module['print'] = function print(x) {
      process['stdout'].write(x + '\n');
    };
    var nodeFS = require('fs');
    var nodePath = require('path');
    Module['read'] = function read(filename, binary) {
      filename = nodePath['normalize'](filename);
      var ret = nodeFS['readFileSync'](filename);
      // The path is absolute if the normalized version is the same as the resolved.
      if (!ret && filename != nodePath['resolve'](filename)) {
        filename = path.join(__dirname, '..', 'src', filename);
        ret = nodeFS['readFileSync'](filename);
      }
      if (ret && !binary) ret = ret.toString();
      return ret;
    };
    Module['readBinary'] = function readBinary(filename) {
      var ret = Module['read'](filename, true);
      if (!ret.buffer) {
        ret = new Uint8Array(ret);
      }
      assert(ret.buffer);
      return ret;
    };
    Module['load'] = function load(f) {
      globalEval(read(f));
    };
  } else if (ENVIRONMENT_IS_SHELL) {
    if (typeof read != 'undefined') {
      Module['read'] = read;
    } else {
      Module['read'] = function read() { throw 'no read() available (jsc?)' };
    }
    Module['readBinary'] = function readBinary(f) {
      if (typeof readbuffer === 'function') {
        return new Uint8Array(readbuffer(f));
      }
      var data = read(f, 'binary');
      assert(typeof data === 'object');
      return data;
    };
  } else if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
    Module['read'] = function read(url) {
      var xhr = new XMLHttpRequest();
      xhr.open('GET', url, false);
      xhr.send(null);
      return xhr.responseText;
    };
    if (typeof console !== 'undefined') {
      if (!Module['print']) Module['print'] = function print(x) {
        console.log(x);
      };
    }
    if (ENVIRONMENT_IS_WORKER) {
      Module['load'] = importScripts;
    }
    if (ENVIRONMENT_IS_WEB) {
      Module['loadScriptFromBlob'] = function(blob) {
        var url = URL.createObjectURL(blob);
        function after() {
          URL.revokeObjectURL(url);
        }
        var script = document.createElement('script');
        script.onload = script.onerror = after;
        script.src = url;
        document.body.appendChild(script);
      };
    } else {
      Module['loadScriptFromBlob'] = function(blob) {
        var url = URL.createObjectURL(blob);
        importScripts(url);
        URL.revokeObjectURL(url);
      };
    }
  } else {
    // Unreachable because SHELL is dependant on the others
    throw 'Unknown runtime environment. Where are we?';
  }

  function globalEval(x) {
    eval.call(null, x);
  }
  if (!Module['load'] && Module['read']) {
    Module['load'] = function load(f) {
      globalEval(Module['read'](f));
    };
  }
  if (!Module['print']) {
    Module['print'] = function(){};
  }
  if (!Module['loadScriptFromBlob']) {
    Module['loadScriptFromBlob'] = function(blob) {
      assert(blob.args.length === 1);
      var array = blob.args[0];
      var str = [];
      for (var i = 0; i < array.length; i++) {
        var chr = array[i];
        if (chr === 0) break;
        if (chr > 0xFF) {
          chr &= 0xFF;
        }
        str.push(String.fromCharCode(chr));
      }
      globalEval(str.join(''));
    };
  }
  if (typeof Worker === 'undefined') {
    Worker = function(url) {
      var that = this;
      var onmessage = eval('(function(postMessage) { var onmessage, Module; ' + Module['read'](url) + '; return onmessage })')(function(data) {
        that.onmessage({ data: data });
      });
      this.postMessage = function(data) {
        onmessage({ data: data });
      };
    };
  }
  if (typeof console === 'undefined') {
    console = {
      log: function(x) { Module['print'](x) },
      error: function(x) { Module['printErr'](x) }
    };
  }
  if (typeof Blob === 'undefined') {
    Blob = function(args) {
      this.args = args;
    };
  }
  // *** Environment setup code ***

  // main

  var globalNameCounter = 0;
  var jobs = {};

  reportWebAssembly = function(callbackName, asm) { // global scope, so the new script tags can find it
    var job = jobs[callbackName];
    if (!job) throw 'bad job';
    delete jobs[callbackName];
    if (job.resolve) {
      job.resolve(asm);
    } else {
      // synchronous execution, then() has not been called yet. queue us for later
      job.then = function(resolve) {
        resolve(asm);
      };
    }
  };

  var workerURL = 'load-wasm-worker.js';
  var worker = new Worker(workerURL);
  worker.onmessage = function(e) {
    if (typeof e.data !== 'object') {
      throw "load-wasm-worker.js failed with: " + e.data;
      return;
    }
    var callbackName = e.data.callbackName;
    Module['loadScriptFromBlob'](e.data.data);
  };

  return function(packedURL) {
    var job = {
      callbackName: 'onFinishLoadWebAssembly_' + globalNameCounter++,
      then: function(resolve) {
        this.resolve = resolve;
      }
    };
    globalEval(job.callbackName + ' = function(asm) { reportWebAssembly("' + job.callbackName + '", asm) }');
    jobs[job.callbackName] = job;
    worker.postMessage({ url: packedURL, callbackName: job.callbackName });
    return job;
  }
})();

