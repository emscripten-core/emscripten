/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Pthread Web Worker startup routine:
// This is the entry point file that is loaded first by each Web Worker
// that executes pthreads on the Emscripten application.

// Thread-local:
var threadInfoStruct = 0; // Info area for this thread in Emscripten HEAP (shared). If zero, this worker is not currently hosting an executing pthread.
var selfThreadId = 0; // The ID of this thread. 0 if not hosting a pthread.
var parentThreadId = 0; // The ID of the parent pthread that launched this thread.

var Module = {};

#if ASSERTIONS
function assert(condition, text) {
  if (!condition) abort('Assertion failed: ' + text);
}
#endif

function threadPrintErr() {
  var text = Array.prototype.slice.call(arguments).join(' ');
  console.error(text);
}
function threadAlert() {
  var text = Array.prototype.slice.call(arguments).join(' ');
  postMessage({cmd: 'alert', text: text, threadId: selfThreadId});
}
#if ASSERTIONS
// We don't need out() for now, but may need to add it if we want to use it
// here. Or, if this code all moves into the main JS, that problem will go
// away. (For now, adding it here increases code size for no benefit.)
var out = function() {
  throw 'out() is not defined in worker.js.';
}
#endif
var err = threadPrintErr;
this.alert = threadAlert;

#if !MINIMAL_RUNTIME
Module['instantiateWasm'] = function(info, receiveInstance) {
  // Instantiate from the module posted from the main thread.
  // We can just use sync instantiation in the worker.
  var instance = new WebAssembly.Instance(Module['wasmModule'], info);
  // We don't need the module anymore; new threads will be spawned from the main thread.
  Module['wasmModule'] = null;
#if LOAD_SOURCE_MAP
  wasmSourceMap = resetPrototype(WasmSourceMap, wasmSourceMapData);
#endif
#if USE_OFFSET_CONVERTER
  wasmOffsetConverter = resetPrototype(WasmOffsetConverter, wasmOffsetData);
#endif
  receiveInstance(instance); // The second 'module' parameter is intentionally null here, we don't need to keep a ref to the Module object from here.
  return instance.exports;
};
#endif

#if LOAD_SOURCE_MAP
var wasmSourceMapData;
#endif
#if USE_OFFSET_CONVERTER
var wasmOffsetData;
#endif

this.onmessage = function(e) {
  try {
    if (e.data.cmd === 'load') { // Preload command that is called once per worker to parse and load the Emscripten code.
#if MINIMAL_RUNTIME
      var imports = {};
#endif

      // Module and memory were sent from main thread
#if MINIMAL_RUNTIME
#if MODULARIZE
      imports['wasm'] = e.data.wasmModule; // Pass the shared Wasm module in an imports object for the MODULARIZEd build.
#else
      Module['wasm'] = e.data.wasmModule; // Pass the shared Wasm module in the Module object for MINIMAL_RUNTIME.
#endif
#else
      Module['wasmModule'] = e.data.wasmModule;
#endif // MINIMAL_RUNTIME

      {{{ makeAsmImportsAccessInPthread('wasmMemory') }}} = e.data.wasmMemory;

#if LOAD_SOURCE_MAP
      wasmSourceMapData = e.data.wasmSourceMap;
#endif
#if USE_OFFSET_CONVERTER
      wasmOffsetData = e.data.wasmOffsetConverter;
#endif

      {{{ makeAsmImportsAccessInPthread('buffer') }}} = {{{ makeAsmImportsAccessInPthread('wasmMemory') }}}.buffer;

#if !MINIMAL_RUNTIME || MODULARIZE
      {{{ makeAsmImportsAccessInPthread('ENVIRONMENT_IS_PTHREAD') }}} = true;
#endif

#if MODULARIZE && EXPORT_ES6
      import(e.data.urlOrBlob).then(function({{{ EXPORT_NAME }}}) {
        return {{{ EXPORT_NAME }}}.default(Module);
      }).then(function(instance) {
        Module = instance;
        postMessage({ 'cmd': 'loaded' });
      });
#else
      if (typeof e.data.urlOrBlob === 'string') {
        importScripts(e.data.urlOrBlob);
      } else {
        var objectUrl = URL.createObjectURL(e.data.urlOrBlob);
        importScripts(objectUrl);
        URL.revokeObjectURL(objectUrl);
      }
#if MODULARIZE
#if MINIMAL_RUNTIME
      {{{ EXPORT_NAME }}}(imports).then(function (instance) {
        Module = instance;
        postMessage({ 'cmd': 'loaded' });
      });
#else
      {{{ EXPORT_NAME }}}(Module).then(function (instance) {
        Module = instance;
        postMessage({ 'cmd': 'loaded' });
      });
#endif
#endif

#if !MODULARIZE && !MINIMAL_RUNTIME
      // MINIMAL_RUNTIME always compiled Wasm (&Wasm2JS) asynchronously, even in pthreads. But
      // regular runtime and asm.js are loaded synchronously, so in those cases
      // we are now loaded, and can post back to main thread.
      postMessage({ 'cmd': 'loaded' });
#endif

#endif
    } else if (e.data.cmd === 'objectTransfer') {
      Module['PThread'].receiveObjectTransfer(e.data);
    } else if (e.data.cmd === 'run') {
      // This worker was idle, and now should start executing its pthread entry
      // point.
      // performance.now() is specced to return a wallclock time in msecs since
      // that Web Worker/main thread launched. However for pthreads this can
      // cause subtle problems in emscripten_get_now() as this essentially
      // would measure time from pthread_create(), meaning that the clocks
      // between each threads would be wildly out of sync. Therefore sync all
      // pthreads to the clock on the main browser thread, so that different
      // threads see a somewhat coherent clock across each of them
      // (+/- 0.1msecs in testing).
      Module['__performance_now_clock_drift'] = performance.now() - e.data.time;
      threadInfoStruct = e.data.threadInfoStruct;

      // Pass the thread address inside the asm.js scope to store it for fast access that avoids the need for a FFI out.
      Module['registerPthreadPtr'](threadInfoStruct, /*isMainBrowserThread=*/0, /*isMainRuntimeThread=*/0);

      selfThreadId = e.data.selfThreadId;
      parentThreadId = e.data.parentThreadId;
      // Establish the stack frame for this thread in global scope
      // The stack grows downwards
      var max = e.data.stackBase;
      var top = e.data.stackBase + e.data.stackSize;
#if ASSERTIONS
      assert(threadInfoStruct);
      assert(selfThreadId);
      assert(parentThreadId);
      assert(top != 0);
      assert(max != 0);
      assert(top > max);
#endif
      // Also call inside JS module to set up the stack frame for this pthread in JS module scope
      Module['establishStackSpace'](top, max);
      Module['_emscripten_tls_init']();
#if STACK_OVERFLOW_CHECK
      Module['writeStackCookie']();
#endif

      Module['PThread'].receiveObjectTransfer(e.data);
      Module['PThread'].setThreadStatus(Module['_pthread_self'](), 1/*EM_THREAD_STATUS_RUNNING*/);

      try {
        // pthread entry points are always of signature 'void *ThreadMain(void *arg)'
        // Native codebases sometimes spawn threads with other thread entry point signatures,
        // such as void ThreadMain(void *arg), void *ThreadMain(), or void ThreadMain().
        // That is not acceptable per C/C++ specification, but x86 compiler ABI extensions
        // enable that to work. If you find the following line to crash, either change the signature
        // to "proper" void *ThreadMain(void *arg) form, or try linking with the Emscripten linker
        // flag -s EMULATE_FUNCTION_POINTER_CASTS=1 to add in emulation for this x86 ABI extension.
        var result = Module['dynCall']('ii', e.data.start_routine, [e.data.arg]);

#if STACK_OVERFLOW_CHECK
        Module['checkStackCookie']();
#endif
#if !MINIMAL_RUNTIME // In MINIMAL_RUNTIME the noExitRuntime concept does not apply to pthreads. To exit a pthread with live runtime, use the function emscripten_unwind_to_js_event_loop() in the pthread body.
        // The thread might have finished without calling pthread_exit(). If so, then perform the exit operation ourselves.
        // (This is a no-op if explicit pthread_exit() had been called prior.)
        if (!Module['getNoExitRuntime']())
#endif
          Module['PThread'].threadExit(result);
      } catch(ex) {
        if (ex === 'Canceled!') {
          Module['PThread'].threadCancel();
        } else if (ex != 'unwind') {
#if MINIMAL_RUNTIME
          // ExitStatus not present in MINIMAL_RUNTIME
          Atomics.store(Module['HEAPU32'], (threadInfoStruct + 4 /*C_STRUCTS.pthread.threadExitCode*/ ) >> 2, -2 /*A custom entry specific to Emscripten denoting that the thread crashed.*/);
#else
          Atomics.store(Module['HEAPU32'], (threadInfoStruct + 4 /*C_STRUCTS.pthread.threadExitCode*/ ) >> 2, (ex instanceof Module['ExitStatus']) ? ex.status : -2 /*A custom entry specific to Emscripten denoting that the thread crashed.*/);
#endif

          Atomics.store(Module['HEAPU32'], (threadInfoStruct + 0 /*C_STRUCTS.pthread.threadStatus*/ ) >> 2, 1); // Mark the thread as no longer running.
#if ASSERTIONS
          if (typeof(Module['_emscripten_futex_wake']) !== "function") {
            err("Thread Initialisation failed.");
            throw ex;
          }
#endif
          Module['_emscripten_futex_wake'](threadInfoStruct + 0 /*C_STRUCTS.pthread.threadStatus*/, 0x7FFFFFFF/*INT_MAX*/); // Wake all threads waiting on this thread to finish.
#if MINIMAL_RUNTIME
          throw ex; // ExitStatus not present in MINIMAL_RUNTIME
#else
          if (!(ex instanceof Module['ExitStatus'])) throw ex;
#endif
#if ASSERTIONS
        } else {
          // else e == 'unwind', and we should fall through here and keep the pthread alive for asynchronous events.
          err('Pthread 0x' + threadInfoStruct.toString(16) + ' completed its pthread main entry point with an unwind, keeping the pthread worker alive for asynchronous operation.');
#endif
        }
      }
    } else if (e.data.cmd === 'cancel') { // Main thread is asking for a pthread_cancel() on this thread.
      if (threadInfoStruct) {
        Module['PThread'].threadCancel();
      }
    } else if (e.data.target === 'setimmediate') {
      // no-op
    } else if (e.data.cmd === 'processThreadQueue') {
      if (threadInfoStruct) { // If this thread is actually running?
        Module['_emscripten_current_thread_process_queued_calls']();
      }
    } else {
      err('worker.js received unknown command ' + e.data.cmd);
      err(e.data);
    }
  } catch(ex) {
    err('worker.js onmessage() captured an uncaught exception: ' + ex);
    if (ex.stack) err(ex.stack);
    throw ex;
  }
};

#if ENVIRONMENT_MAY_BE_NODE
// Node.js support
if (typeof process === 'object' && typeof process.versions === 'object' && typeof process.versions.node === 'string') {
  // Create as web-worker-like an environment as we can.
  self = {
    location: {
      href: __filename
    }
  };

  var onmessage = this.onmessage;

  var nodeWorkerThreads = require('worker_threads');

  global.Worker = nodeWorkerThreads.Worker;

  var parentPort = nodeWorkerThreads.parentPort;

  parentPort.on('message', function(data) {
    onmessage({ data: data });
  });

  var nodeFS = require('fs');

  var nodeRead = function(filename) {
    return nodeFS.readFileSync(filename, 'utf8');
  };

  function globalEval(x) {
    global.require = require;
    global.Module = Module;
    eval.call(null, x);
  }

  importScripts = function(f) {
    globalEval(nodeRead(f));
  };

  postMessage = function(msg) {
    parentPort.postMessage(msg);
  };

  if (typeof performance === 'undefined') {
    performance = {
      now: function() {
        return Date.now();
      }
    };
  }
}
#endif // ENVIRONMENT_MAY_BE_NODE
