// Pthread Web Worker startup routine:
// This is the entry point file that is loaded first by each Web Worker
// that executes pthreads on the Emscripten application.

// Thread-local, communicated via globals:
var threadInfoStruct = 0; // Info area for this thread in Emscripten HEAP (shared). If zero, this worker is not currently hosting an executing pthread.
var selfThreadId = 0; // The ID of this thread. 0 if not hosting a pthread.
var parentThreadId = 0; // The ID of the parent pthread that launched this thread.

// Send the pthreads mode and other params in through Module object settings
var Module = {
  ENVIRONMENT: 'PTHREAD'
};

// Cannot use console.log or console.error in a web worker, since that would risk a browser deadlock! https://bugzilla.mozilla.org/show_bug.cgi?id=1049091
// Therefore implement custom logging facility for threads running in a worker, which queue the messages to main thread to print.
function threadPrint() {
  var text = Array.prototype.slice.call(arguments).join(' ');
  console.log(text);
}
function threadPrintErr() {
  var text = Array.prototype.slice.call(arguments).join(' ');
  console.error(text);
}
function threadAlert() {
  var text = Array.prototype.slice.call(arguments).join(' ');
  postMessage({cmd: 'alert', text: text, threadId: selfThreadId});
}
Module['print'] = threadPrint;
Module['printErr'] = threadPrintErr;
this.alert = threadAlert;

// If modularized, we can't reuse the module's assert() function.
function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed: ' + text);
  }
}

this.onmessage = function(e) {
  if (e.data.cmd === 'load') { // Preload command that is called once per worker to parse and load the Emscripten code.
    // Initialize the thread-local field(s):
    Module['tempDoublePtr'] = e.data.tempDoublePtr;

    // Initialize the global "process"-wide fields:
    Module['buffer'] = e.data.buffer;
    Module['TOTAL_MEMORY'] = e.data.TOTAL_MEMORY;
    Module['STATICTOP'] = e.data.STATICTOP;
    Module['DYNAMIC_BASE'] = e.data.DYNAMIC_BASE;
    Module['DYNAMICTOP_PTR'] = e.data.DYNAMICTOP_PTR;

    Module['pthreadWorkerInit'] = e.data.PthreadWorkerInit;
    importScripts(e.data.url);
    if (e.data.modularize) {
      // Feed input options into the modularized constructor...
      // 'this' is the Worker, which is also global scope.
      Module = new this[e.data.moduleExportName](Module);
    }
    if (typeof FS !== 'undefined') FS.createStandardStreams();
    postMessage({ cmd: 'loaded' });
  } else if (e.data.cmd === 'objectTransfer') {
    Module.PThread.receiveObjectTransfer(e.data);
  } else if (e.data.cmd === 'run') { // This worker was idle, and now should start executing its pthread entry point.
    threadInfoStruct = e.data.threadInfoStruct;
    Module.PThread.registerPthreadPtr(threadInfoStruct, /*isMainBrowserThread=*/0, /*isMainRuntimeThread=*/0); // Pass the thread address inside the asm.js scope to store it for fast access that avoids the need for a FFI out.
    assert(threadInfoStruct);
    selfThreadId = e.data.selfThreadId;
    parentThreadId = e.data.parentThreadId;
    assert(selfThreadId);
    assert(parentThreadId);
    Module.PThread.setStackSpace(e.data.stackBase, e.data.stackBase + e.data.stackSize);
    var result = 0;

    Module.PThread.receiveObjectTransfer(e.data);

    Module.PThread.setThreadStatus(threadInfoStruct, 1/*EM_THREAD_STATUS_RUNNING*/);

    try {
      Module.PThread.runThreadFunc(e.data.start_routine, e.data.arg);
    } catch(e) {
      if (e === 'Canceled!') {
        Module.PThread.threadCancel();
        return;
      } else {
        Atomics.store(Module.HEAPU32, (threadInfoStruct + 4 /*{{{ C_STRUCTS.pthread.threadExitCode }}}*/ ) >> 2, -2 /*A custom entry specific to Emscripten denoting that the thread crashed.*/);
        Atomics.store(Module.HEAPU32, (threadInfoStruct + 0 /*{{{ C_STRUCTS.pthread.threadStatus }}}*/ ) >> 2, 1); // Mark the thread as no longer running.
        Module.PThread.wakeAllThreads();
        throw e;
      }
    }
    // The thread might have finished without calling pthread_exit(). If so, then perform the exit operation ourselves.
    // (This is a no-op if explicit pthread_exit() had been called prior.)
    if (!Module['noExitRuntime']) Module.PThread.threadExit(result);
    else console.log('pthread noExitRuntime: not quitting.');
  } else if (e.data.cmd === 'cancel') { // Main thread is asking for a pthread_cancel() on this thread.
    if (threadInfoStruct && Module.PThread.thisThreadCancelState == 0/*PTHREAD_CANCEL_ENABLE*/) {
      Module.PThread.threadCancel();
    }
  } else {
    Module['printErr']('pthread-main.js received unknown command ' + e.data.cmd);
  }
}
