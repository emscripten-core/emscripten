// Pthread Web Worker startup routine:
// This is the entry point file that is loaded first by each Web Worker
// that executes pthreads on the Emscripten application.

// All pthreads share the same Emscripten HEAP as SharedArrayBuffer
// with the main execution thread.
var buffer;

var threadBlock = 0; // Info area for this thread in Emscripten HEAP (shared). If zero, this worker is not currently hosting an executing pthread.

var selfThreadId = 0; // The ID of this thread. 0 if not hosting a pthread.

// Each thread has its own allocated stack space.
var STACK_BASE = 0;
var STACKTOP = 0;
var STACK_MAX = 0;

// Cannot use console.log or console.error in a web worker, since that would risk a browser deadlock! https://bugzilla.mozilla.org/show_bug.cgi?id=1049091
// Therefore implement custom logging facility for threads running in a worker, which queue the messages to main thread to print.
var Module = {};
Module['print'] = function() {
  var text = Array.prototype.slice.call(arguments).join(' ');
  postMessage({cmd: 'print', text: text, threadId: selfThreadId});
}
Module['printErr'] = function() {
  var text = Array.prototype.slice.call(arguments).join(' ');
  postMessage({cmd: 'printErr', text: text, threadId: selfThreadId});
}

this.onmessage = function(e) {
  if (e.data.cmd == 'load') { // Preload command that is called once per worker to parse and load the Emscripten code.
    buffer = e.data.buffer;
    importScripts(e.data.url);
    postMessage({ cmd: 'loaded' });
  } else if (e.data.cmd == 'run') { // This worker was idle, and now should start executing its pthread entry point.
    threadBlock = e.data.threadBlock;
    selfThreadId = e.data.selfThreadId;
    Runtime.stackRestore(e.data.stackBase);
    STACK_BASE = STACKTOP = e.data.stackBase;
    STACK_MAX = STACK_BASE + e.data.stackSize;
    var result = 0;
    try {
      result = asm.dynCall_ii(e.data.start_routine, e.data.arg); // pthread entry points are always of signature 'void *ThreadMain(void *arg)'
    } catch(e) {
      if (e === 'Canceled!') {
        Atomics.store(HEAPU32, threadBlock >> 2, 1);
        PThread.runExitHandlers();
        threadBlock = selfThreadId = 0;
        postMessage({ cmd: 'cancel' });
        return;
      } else {
        throw e;
      }
    }
    // Thread finished with exit.
    if (Atomics.load(HEAPU32, threadBlock >> 2) != 1) { // If thread did not use pthread_exit to pass the thread return code, pass it from the return value of the thread main.
      Atomics.store(HEAPU32, threadBlock + 4 >> 2, result); // Exit code.
      Atomics.store(HEAPU32, threadBlock >> 2, 1); // 1 == exited.
    }
  } else if (e.data.cmd == 'cancel') { // Main thread is asking for a pthread_cancel() on this thread.
    if (threadBlock) {
      PThread.runExitHandlers();      
      threadBlock = selfThreadId = 0; // Not hosting a pthread anymore in this worker, reset the info structures to null.
      postMessage({ cmd: 'cancel' });
    }
  } else {
    Module['printErr']('pthread-main.js received unknown command ' + e.data.cmd);
  }
}
