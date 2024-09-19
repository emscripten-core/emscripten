/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// Pthread Web Worker handling code.
// This code runs only on pthread web workers and handles pthread setup
// and communication with the main thread via postMessage.

#if ASSERTIONS
// Unique ID of the current pthread worker (zero on non-pthread-workers
// including the main thread).
var workerID = 0;
#endif

if (ENVIRONMENT_IS_PTHREAD) {
#if !MINIMAL_RUNTIME
  var wasmPromiseResolve;
  var wasmPromiseReject;
#endif
  var receivedWasmModule;

#if ENVIRONMENT_MAY_BE_NODE
  // Node.js support
  if (ENVIRONMENT_IS_NODE) {
    // Create as web-worker-like an environment as we can.

    var parentPort = worker_threads['parentPort'];
    parentPort.on('message', (data) => onmessage({ data: data }));

    Object.assign(globalThis, {
      self: global,
      // Dummy importScripts.  The presence of this global is used
      // to detect that we are running on a Worker.
      // TODO(sbc): Find another way?
      importScripts: () => {
#if ASSERTIONS
        assert(false, 'dummy importScripts called');
#endif
      },
      postMessage: (msg) => parentPort.postMessage(msg),
    });
  }
#endif // ENVIRONMENT_MAY_BE_NODE

  // Thread-local guard variable for one-time init of the JS state
  var initializedJS = false;

  function threadPrintErr(...args) {
    var text = args.join(' ');
#if ENVIRONMENT_MAY_BE_NODE
    // See https://github.com/emscripten-core/emscripten/issues/14804
    if (ENVIRONMENT_IS_NODE) {
      fs.writeSync(2, text + '\n');
      return;
    }
#endif
    console.error(text);
  }

#if expectToReceiveOnModule('printErr')
  if (!Module['printErr'])
#endif
    err = threadPrintErr;
#if ASSERTIONS || RUNTIME_DEBUG
  dbg = threadPrintErr;
#endif
  function threadAlert(...args) {
    var text = args.join(' ');
    postMessage({cmd: 'alert', text, threadId: _pthread_self()});
  }
  self.alert = threadAlert;

#if !MINIMAL_RUNTIME
  Module['instantiateWasm'] = (info, receiveInstance) => {
    return new Promise((resolve, reject) => {
      wasmPromiseResolve = (module) => {
        // Instantiate from the module posted from the main thread.
        // We can just use sync instantiation in the worker.
        var instance = new WebAssembly.Instance(module, getWasmImports());
#if RELOCATABLE || MAIN_MODULE
        receiveInstance(instance, module);
#else
        // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193,
        // the above line no longer optimizes out down to the following line.
        // When the regression is fixed, we can remove this if/else.
        receiveInstance(instance);
#endif
        resolve();
      };
      wasmPromiseReject = reject;
    });
  }
#endif

  // Turn unhandled rejected promises into errors so that the main thread will be
  // notified about them.
  self.onunhandledrejection = (e) => { throw e.reason || e; };

  function handleMessage(e) {
    try {
      var msgData = e['data'];
      //dbg('msgData: ' + Object.keys(msgData));
      var cmd = msgData.cmd;
      if (cmd === 'load') { // Preload command that is called once per worker to parse and load the Emscripten code.
#if ASSERTIONS
        workerID = msgData.workerID;
#endif
#if PTHREADS_DEBUG
        dbg('worker: loading module')
#endif

        // Until we initialize the runtime, queue up any further incoming messages.
        let messageQueue = [];
        self.onmessage = (e) => messageQueue.push(e);

        // And add a callback for when the runtime is initialized.
        self.startWorker = (instance) => {
          // Notify the main thread that this thread has loaded.
          postMessage({ cmd: 'loaded' });
          // Process any messages that were queued before the thread was ready.
          for (let msg of messageQueue) {
            handleMessage(msg);
          }
          // Restore the real message handler.
          self.onmessage = handleMessage;
        };

#if MAIN_MODULE
        dynamicLibraries = msgData.dynamicLibraries;
        sharedModules = msgData.sharedModules;
#if RUNTIME_DEBUG
        dbg(`worker: received ${Object.keys(msgData.sharedModules).length} shared modules: ${Object.keys(msgData.sharedModules)}`);
#endif
#endif

        // Use `const` here to ensure that the variable is scoped only to
        // that iteration, allowing safe reference from a closure.
        for (const handler of msgData.handlers) {
          // The the main module has a handler for a certain even, but no
          // handler exists on the pthread worker, then proxy that handler
          // back to the main thread.
          if (!Module[handler] || Module[handler].proxy) {
#if RUNTIME_DEBUG
            dbg(`worker: installer proxying handler: ${handler}`);
#endif
            Module[handler] = (...args) => {
#if RUNTIME_DEBUG
              dbg(`worker: calling handler on main thread: ${handler}`);
#endif
              postMessage({ cmd: 'callHandler', handler, args: args });
            }
            // Rebind the out / err handlers if needed
            if (handler == 'print') out = Module[handler];
            if (handler == 'printErr') err = Module[handler];
          }
#if RUNTIME_DEBUG
          else dbg(`worker: using thread-local handler: ${handler}`);
#endif
        }

        wasmMemory = msgData.wasmMemory;
        updateMemoryViews();

#if LOAD_SOURCE_MAP
        wasmSourceMap = resetPrototype(WasmSourceMap, msgData.wasmSourceMap);
#endif
#if USE_OFFSET_CONVERTER
        wasmOffsetConverter = resetPrototype(WasmOffsetConverter, msgData.wasmOffsetConverter);
#endif

#if MINIMAL_RUNTIME
        // Pass the shared Wasm module in the Module object for MINIMAL_RUNTIME.
        Module['wasm'] = msgData.wasmModule;
        loadModule();
#else
        wasmPromiseResolve(msgData.wasmModule);
#endif // MINIMAL_RUNTIME
      } else if (cmd === 'run') {
#if ASSERTIONS
        assert(msgData.pthread_ptr);
#endif
        // Call inside JS module to set up the stack frame for this pthread in JS module scope.
        // This needs to be the first thing that we do, as we cannot call to any C/C++ functions
        // until the thread stack is initialized.
        establishStackSpace(msgData.pthread_ptr);

        // Pass the thread address to wasm to store it for fast access.
        __emscripten_thread_init(msgData.pthread_ptr, /*is_main=*/0, /*is_runtime=*/0, /*can_block=*/1, 0, 0);

        PThread.receiveObjectTransfer(msgData);
        PThread.threadInitTLS();

        // Await mailbox notifications with `Atomics.waitAsync` so we can start
        // using the fast `Atomics.notify` notification path.
        __emscripten_thread_mailbox_await(msgData.pthread_ptr);

        if (!initializedJS) {
#if EMBIND
#if PTHREADS_DEBUG
          dbg(`worker: Pthread 0x${_pthread_self().toString(16)} initializing embind.`);
#endif
          // Embind must initialize itself on all threads, as it generates support JS.
          // We only do this once per worker since they get reused
          __embind_initialize_bindings();
#endif // EMBIND
          initializedJS = true;
        }

        try {
          invokeEntryPoint(msgData.start_routine, msgData.arg);
        } catch(ex) {
          if (ex != 'unwind') {
            // The pthread "crashed".  Do not call `_emscripten_thread_exit` (which
            // would make this thread joinable).  Instead, re-throw the exception
            // and let the top level handler propagate it back to the main thread.
            throw ex;
          }
#if RUNTIME_DEBUG
          dbg(`worker: Pthread 0x${_pthread_self().toString(16)} completed its main entry point with an 'unwind', keeping the worker alive for asynchronous operation.`);
#endif
        }
      } else if (msgData.target === 'setimmediate') {
        // no-op
      } else if (cmd === 'checkMailbox') {
        if (initializedJS) {
          checkMailbox();
        }
      } else if (cmd) {
        // The received message looks like something that should be handled by this message
        // handler, (since there is a cmd field present), but is not one of the
        // recognized commands:
        err(`worker: received unknown command ${cmd}`);
        err(msgData);
      }
    } catch(ex) {
#if ASSERTIONS
      err(`worker: onmessage() captured an uncaught exception: ${ex}`);
      if (ex?.stack) err(ex.stack);
#endif
      __emscripten_thread_crashed();
      throw ex;
    }
  };

  self.onmessage = handleMessage;

} // ENVIRONMENT_IS_PTHREAD
