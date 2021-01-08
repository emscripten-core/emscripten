/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Pthread Web Worker startup routine:
// This is the entry point file that is loaded first by each Web Worker
// that executes pthreads on the Emscripten application.

// Thread-local:
#if EMBIND
var initializedJS = false; // Guard variable for one-time init of the JS state (currently only embind types registration)
#endif

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
  postMessage({cmd: 'alert', text: text, threadId: Module['_pthread_self']()});
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
  receiveInstance(instance); // The second 'module' parameter is intentionally null here, we don't need to keep a ref to the Module object from here.
  return instance.exports;
};
#endif

#if LOAD_SOURCE_MAP || USE_OFFSET_CONVERTER
// When using postMessage to send an object, it is processed by the structured clone algorithm.
// The prototype, and hence methods, on that object is then lost. This function adds back the lost prototype.
// This does not work with nested objects that has prototypes, but it suffices for WasmSourceMap and WasmOffsetConverter.
function resetPrototype(constructor, attrs) {
  var object = Object.create(constructor.prototype);
  for (var key in attrs) {
    if (attrs.hasOwnProperty(key)) {
      object[key] = attrs[key];
    }
  }
  return object;
}
#endif

#if LOAD_SOURCE_MAP
var wasmSourceMapData;
#endif
#if USE_OFFSET_CONVERTER
var wasmOffsetData;
#endif

function moduleLoaded() {
#if LOAD_SOURCE_MAP
  wasmSourceMap = resetPrototype(WasmSourceMap, wasmSourceMapData);
#endif
#if USE_OFFSET_CONVERTER
  wasmOffsetConverter = resetPrototype(WasmOffsetConverter, wasmOffsetData);
#endif
  postMessage({ 'cmd': 'loaded' });
}

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
        moduleLoaded();
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
        moduleLoaded();
      });
#else
      {{{ EXPORT_NAME }}}(Module).then(function (instance) {
        Module = instance;
        moduleLoaded();
      });
#endif
#endif

#if !MODULARIZE && !MINIMAL_RUNTIME
      // MINIMAL_RUNTIME always compiled Wasm (&Wasm2JS) asynchronously, even in pthreads. But
      // regular runtime and asm.js are loaded synchronously, so in those cases
      // we are now loaded, and can post back to main thread.
      moduleLoaded();
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

      // Pass the thread address inside the asm.js scope to store it for fast access that avoids the need for a FFI out.
      Module['__emscripten_thread_init'](e.data.threadInfoStruct, /*isMainBrowserThread=*/0, /*isMainRuntimeThread=*/0);

      // Establish the stack frame for this thread in global scope
      // The stack grows downwards
      var max = e.data.stackBase;
      var top = e.data.stackBase + e.data.stackSize;
#if ASSERTIONS
      assert(e.data.threadInfoStruct);
      assert(top != 0);
      assert(max != 0);
      assert(top > max);
#endif
      // Also call inside JS module to set up the stack frame for this pthread in JS module scope
      Module['establishStackSpace'](top, max);
      Module['_emscripten_tls_init']();

      Module['PThread'].receiveObjectTransfer(e.data);
      Module['PThread'].setThreadStatus(Module['_pthread_self'](), 1/*EM_THREAD_STATUS_RUNNING*/);

#if EMBIND
      // Embind must initialize itself on all threads, as it generates support JS.
      // We only do this once per worker since they get reused
      if (!initializedJS) {
        Module['___embind_register_native_and_builtin_types']();
        initializedJS = true;
      }
#endif // EMBIND

      try {
        // pthread entry points are always of signature 'void *ThreadMain(void *arg)'
        // Native codebases sometimes spawn threads with other thread entry point signatures,
        // such as void ThreadMain(void *arg), void *ThreadMain(), or void ThreadMain().
        // That is not acceptable per C/C++ specification, but x86 compiler ABI extensions
        // enable that to work. If you find the following line to crash, either change the signature
        // to "proper" void *ThreadMain(void *arg) form, or try linking with the Emscripten linker
        // flag -s EMULATE_FUNCTION_POINTER_CASTS=1 to add in emulation for this x86 ABI extension.
        var result = Module['invokeEntryPoint'](e.data.start_routine, e.data.arg);

#if STACK_OVERFLOW_CHECK
        Module['checkStackCookie']();
#endif
#if !MINIMAL_RUNTIME
        // In MINIMAL_RUNTIME the noExitRuntime concept does not apply to
        // pthreads. To exit a pthread with live runtime, use the function
        // emscripten_unwind_to_js_event_loop() in the pthread body.
        // The thread might have finished without calling pthread_exit(). If so,
        // then perform the exit operation ourselves.
        // (This is a no-op if explicit pthread_exit() had been called prior.)
        if (!Module['getNoExitRuntime']())
#endif
          Module['PThread'].threadExit(result);
      } catch(ex) {
        if (ex === 'Canceled!') {
          Module['PThread'].threadCancel();
        } else if (ex != 'unwind') {
#if ASSERTIONS
          // FIXME(sbc): Figure out if this is still needed or useful.  Its not
          // clear to me how this check could ever fail.  In order to get into
          // this try/catch block at all we have already called bunch of
          // functions on `Module`.. why is this one special?
          if (typeof(Module['_emscripten_futex_wake']) !== "function") {
            err("Thread Initialisation failed.");
            throw ex;
          }
#endif
          // ExitStatus not present in MINIMAL_RUNTIME
#if !MINIMAL_RUNTIME
          if (ex instanceof Module['ExitStatus']) {
            if (Module['getNoExitRuntime']()) {
#if ASSERTIONS
              err('Pthread 0x' + Module['_pthread_self']().toString(16) + ' called exit(), staying alive due to noExitRuntime.');
#endif
            } else {
#if ASSERTIONS
              err('Pthread 0x' + Module['_pthread_self']().toString(16) + ' called exit(), calling threadExit.');
#endif
              Module['PThread'].threadExit(ex.status);
            }
          }
          else
#endif
          {
            Module['PThread'].threadExit(-2);
            throw ex;
          }
#if ASSERTIONS
        } else {
          // else e == 'unwind', and we should fall through here and keep the pthread alive for asynchronous events.
          err('Pthread 0x' + Module['_pthread_self']().toString(16) + ' completed its pthread main entry point with an unwind, keeping the pthread worker alive for asynchronous operation.');
#endif
        }
      }
    } else if (e.data.cmd === 'cancel') { // Main thread is asking for a pthread_cancel() on this thread.
      if (Module['_pthread_self']()) {
        Module['PThread'].threadCancel();
      }
    } else if (e.data.target === 'setimmediate') {
      // no-op
    } else if (e.data.cmd === 'processThreadQueue') {
      if (Module['_pthread_self']()) { // If this thread is actually running?
        Module['_emscripten_current_thread_process_queued_calls']();
      }
    } else {
      err('worker.js received unknown command ' + e.data.cmd);
      err(e.data);
    }
  } catch(ex) {
    err('worker.js onmessage() captured an uncaught exception: ' + ex);
    if (ex && ex.stack) err(ex.stack);
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
