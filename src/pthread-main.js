// Pthread Web Worker startup routine:
// This is the entry point file that is loaded first by each Web Worker
// that executes pthreads on the Emscripten application.

// Thread-local:
var threadInfoStruct = 0; // Info area for this thread in Emscripten HEAP (shared). If zero, this worker is not currently hosting an executing pthread.
var selfThreadId = 0; // The ID of this thread. 0 if not hosting a pthread.
var parentThreadId = 0; // The ID of the parent pthread that launched this thread.
var tempDoublePtr = 0; // A temporary memory area for global float and double marshalling operations.

// Thread-local: Each thread has its own allocated stack space.
var STACK_BASE = 0;
var STACKTOP = 0;
var STACK_MAX = 0;

// These are system-wide memory area parameters that are set at main runtime startup in main thread, and stay constant throughout the application.
var buffer; // All pthreads share the same Emscripten HEAP as SharedArrayBuffer with the main execution thread.
var DYNAMICTOP_PTR = 0;
var TOTAL_MEMORY = 0;
var STATICTOP = 0;
var staticSealed = true; // When threads are being initialized, the static memory area has been already sealed a long time ago.
var DYNAMIC_BASE = 0;

var ENVIRONMENT_IS_PTHREAD = true;

// performance.now() is specced to return a wallclock time in msecs since that Web Worker/main thread launched. However for pthreads this can cause
// subtle problems in emscripten_get_now() as this essentially would measure time from pthread_create(), meaning that the clocks between each threads
// would be wildly out of sync. Therefore sync all pthreads to the clock on the main browser thread, so that different threads see a somewhat
// coherent clock across each of them (+/- 0.1msecs in testing)
var __performance_now_clock_drift = 0;

// Cannot use console.log or console.error in a web worker, since that would risk a browser deadlock! https://bugzilla.mozilla.org/show_bug.cgi?id=1049091
// Therefore implement custom logging facility for threads running in a worker, which queue the messages to main thread to print.
var Module = {};

// When error objects propagate from Web Worker to main thread, they lose helpful call stack and thread ID information, so print out errors early here,
// before that happens.
this.addEventListener('error', function(e) {
  if (e.message.indexOf('SimulateInfiniteLoop') != -1) return e.preventDefault();

  var errorSource = ' in ' + e.filename + ':' + e.lineno + ':' + e.colno;
  console.error('Pthread ' + selfThreadId + ' uncaught exception' + (e.filename || e.lineno || e.colno ? errorSource : '') + ': ' + e.message + '. Error object:');
  console.error(e.error);
});

function threadPrint() {
  var text = Array.prototype.slice.call(arguments).join(' ');
  console.log(text);
}
function threadPrintErr() {
  var text = Array.prototype.slice.call(arguments).join(' ');
  console.error(text);
  console.error(new Error().stack);
}
function threadAlert() {
  var text = Array.prototype.slice.call(arguments).join(' ');
  postMessage({cmd: 'alert', text: text, threadId: selfThreadId});
}
out = threadPrint;
err = threadPrintErr;
this.alert = threadAlert;

// #if WASM
Module['instantiateWasm'] = function(info, receiveInstance) {
  // Instantiate from the module posted from the main thread.
  // We can just use sync instantiation in the worker.
  instance = new WebAssembly.Instance(Module['wasmModule'], info);
  // We don't need the module anymore; new threads will be spawned from the main thread.
  delete Module['wasmModule'];
  receiveInstance(instance);
  return instance.exports;
}
//#endif

this.onmessage = function(e) {
  try {
    if (e.data.cmd === 'load') { // Preload command that is called once per worker to parse and load the Emscripten code.
      // Initialize the thread-local field(s):
      tempDoublePtr = e.data.tempDoublePtr;

      // Initialize the global "process"-wide fields:
      Module['TOTAL_MEMORY'] = TOTAL_MEMORY = e.data.TOTAL_MEMORY;
      STATICTOP = e.data.STATICTOP;
      DYNAMIC_BASE = e.data.DYNAMIC_BASE;
      DYNAMICTOP_PTR = e.data.DYNAMICTOP_PTR;


//#if WASM
      if (e.data.wasmModule) {
        // Module and memory were sent from main thread
        Module['wasmModule'] = e.data.wasmModule;
        Module['wasmMemory'] = e.data.wasmMemory;
        buffer = Module['wasmMemory'].buffer;
      } else {
//#else
        buffer = e.data.buffer;
      }
//#endif

      PthreadWorkerInit = e.data.PthreadWorkerInit;
      if (typeof e.data.urlOrBlob === 'string') {
        importScripts(e.data.urlOrBlob);
      } else {
        var objectUrl = URL.createObjectURL(e.data.urlOrBlob);
        importScripts(objectUrl);
        URL.revokeObjectURL(objectUrl);
      }
//#if !ASMFS
      if (typeof FS !== 'undefined' && typeof FS.createStandardStreams === 'function') FS.createStandardStreams();
//#endif
      postMessage({ cmd: 'loaded' });
    } else if (e.data.cmd === 'objectTransfer') {
      PThread.receiveObjectTransfer(e.data);
    } else if (e.data.cmd === 'run') { // This worker was idle, and now should start executing its pthread entry point.
      __performance_now_clock_drift = performance.now() - e.data.time; // Sync up to the clock of the main thread.
      threadInfoStruct = e.data.threadInfoStruct;
      __register_pthread_ptr(threadInfoStruct, /*isMainBrowserThread=*/0, /*isMainRuntimeThread=*/0); // Pass the thread address inside the asm.js scope to store it for fast access that avoids the need for a FFI out.
      assert(threadInfoStruct);
      selfThreadId = e.data.selfThreadId;
      parentThreadId = e.data.parentThreadId;
      assert(selfThreadId);
      assert(parentThreadId);
      // TODO: Emscripten runtime has these variables twice(!), once outside the asm.js module, and a second time inside the asm.js module.
      //       Review why that is? Can those get out of sync?
      STACK_BASE = STACKTOP = e.data.stackBase;
      STACK_MAX = STACK_BASE + e.data.stackSize;
      assert(STACK_BASE != 0);
      assert(STACK_MAX > STACK_BASE);
      Module['establishStackSpace'](e.data.stackBase, e.data.stackBase + e.data.stackSize);
      var result = 0;
//#if STACK_OVERFLOW_CHECK
      if (typeof writeStackCookie === 'function') writeStackCookie();
//#endif

      PThread.receiveObjectTransfer(e.data);
      PThread.setThreadStatus(_pthread_self(), 1/*EM_THREAD_STATUS_RUNNING*/);

      try {
        // pthread entry points are always of signature 'void *ThreadMain(void *arg)'
        // Native codebases sometimes spawn threads with other thread entry point signatures,
        // such as void ThreadMain(void *arg), void *ThreadMain(), or void ThreadMain().
        // That is not acceptable per C/C++ specification, but x86 compiler ABI extensions
        // enable that to work. If you find the following line to crash, either change the signature
        // to "proper" void *ThreadMain(void *arg) form, or try linking with the Emscripten linker
        // flag -s EMULATE_FUNCTION_POINTER_CASTS=1 to add in emulation for this x86 ABI extension.
        result = Module['dynCall_ii'](e.data.start_routine, e.data.arg);

//#if STACK_OVERFLOW_CHECK
        if (typeof checkStackCookie === 'function') checkStackCookie();
//#endif

      } catch(e) {
        if (e === 'Canceled!') {
          PThread.threadCancel();
          return;
        } else if (e === 'SimulateInfiniteLoop') {
          return;
        } else {
          Atomics.store(HEAPU32, (threadInfoStruct + 4 /*{{{ C_STRUCTS.pthread.threadExitCode }}}*/ ) >> 2, (e instanceof ExitStatus) ? e.status : -2 /*A custom entry specific to Emscripten denoting that the thread crashed.*/);
          Atomics.store(HEAPU32, (threadInfoStruct + 0 /*{{{ C_STRUCTS.pthread.threadStatus }}}*/ ) >> 2, 1); // Mark the thread as no longer running.
          _emscripten_futex_wake(threadInfoStruct + 0 /*{{{ C_STRUCTS.pthread.threadStatus }}}*/, 0x7FFFFFFF/*INT_MAX*/); // Wake all threads waiting on this thread to finish.
          if (!(e instanceof ExitStatus)) throw e;
        }
      }
      // The thread might have finished without calling pthread_exit(). If so, then perform the exit operation ourselves.
      // (This is a no-op if explicit pthread_exit() had been called prior.)
      PThread.threadExit(result);
    } else if (e.data.cmd === 'cancel') { // Main thread is asking for a pthread_cancel() on this thread.
      if (threadInfoStruct && PThread.thisThreadCancelState == 0/*PTHREAD_CANCEL_ENABLE*/) {
        PThread.threadCancel();
      }
    } else if (e.data.target === 'setimmediate') {
      // no-op
    } else if (e.data.cmd === 'processThreadQueue') {
      if (threadInfoStruct) { // If this thread is actually running?
        _emscripten_current_thread_process_queued_calls();
      }
    } else {
      err('pthread-main.js received unknown command ' + e.data.cmd);
      console.error(e.data);
    }
  } catch(e) {
    console.error('pthread-main.js onmessage() captured an uncaught exception: ' + e);
    console.error(e.stack);
    throw e;
  }
}
