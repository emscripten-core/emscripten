
// === Auto-generated postamble setup entry stuff ===

Module['asm'] = asm;

{{{ exportRuntime() }}}

#if MEM_INIT_IN_WASM == 0
#if MEM_INIT_METHOD == 2
#if USE_PTHREADS
if (memoryInitializer && !ENVIRONMENT_IS_PTHREAD) (function(s) {
#else
if (memoryInitializer) (function(s) {
#endif
  var i, n = s.length;
#if ASSERTIONS
  n -= 4;
  var crc, bit, table = new Int32Array(256);
  for (i = 0; i < 256; ++i) {
    for (crc = i, bit = 0; bit < 8; ++bit)
      crc = (crc >>> 1) ^ ((crc & 1) * 0xedb88320);
    table[i] = crc >>> 0;
  }
  crc = -1;
  crc = table[(crc ^ n) & 0xff] ^ (crc >>> 8);
  crc = table[(crc ^ (n >>> 8)) & 0xff] ^ (crc >>> 8);
  for (i = 0; i < s.length; ++i) {
    crc = table[(crc ^ s.charCodeAt(i)) & 0xff] ^ (crc >>> 8);
  }
  assert(crc === 0, "memory initializer checksum");
#endif
  for (i = 0; i < n; ++i) {
    HEAPU8[GLOBAL_BASE + i] = s.charCodeAt(i);
  }
})(memoryInitializer);
#else
#if USE_PTHREADS
if (memoryInitializer && !ENVIRONMENT_IS_PTHREAD) {
#else
if (memoryInitializer) {
#endif
  if (!isDataURI(memoryInitializer)) {
    memoryInitializer = locateFile(memoryInitializer);
  }
  if (ENVIRONMENT_IS_NODE || ENVIRONMENT_IS_SHELL) {
    var data = Module['readBinary'](memoryInitializer);
    HEAPU8.set(data, GLOBAL_BASE);
  } else {
    addRunDependency('memory initializer');
    var applyMemoryInitializer = function(data) {
      if (data.byteLength) data = new Uint8Array(data);
#if ASSERTIONS
      for (var i = 0; i < data.length; i++) {
        assert(HEAPU8[GLOBAL_BASE + i] === 0, "area for memory initializer should not have been touched before it's loaded");
      }
#endif
      HEAPU8.set(data, GLOBAL_BASE);
      // Delete the typed array that contains the large blob of the memory initializer request response so that
      // we won't keep unnecessary memory lying around. However, keep the XHR object itself alive so that e.g.
      // its .status field can still be accessed later.
      if (Module['memoryInitializerRequest']) delete Module['memoryInitializerRequest'].response;
      removeRunDependency('memory initializer');
    }
    function doBrowserLoad() {
      Module['readAsync'](memoryInitializer, applyMemoryInitializer, function() {
        throw 'could not load memory initializer ' + memoryInitializer;
      });
    }
#if SUPPORT_BASE64_EMBEDDING
    var memoryInitializerBytes = tryParseAsDataURI(memoryInitializer);
    if (memoryInitializerBytes) {
      applyMemoryInitializer(memoryInitializerBytes.buffer);
    } else
#endif
    if (Module['memoryInitializerRequest']) {
      // a network request has already been created, just use that
      function useRequest() {
        var request = Module['memoryInitializerRequest'];
        var response = request.response;
        if (request.status !== 200 && request.status !== 0) {
#if SUPPORT_BASE64_EMBEDDING
          var data = tryParseAsDataURI(Module['memoryInitializerRequestURL']);
          if (data) {
            response = data.buffer;
          } else {
#endif
            // If you see this warning, the issue may be that you are using locateFile and defining it in JS. That
            // means that the HTML file doesn't know about it, and when it tries to create the mem init request early, does it to the wrong place.
            // Look in your browser's devtools network console to see what's going on.
            console.warn('a problem seems to have happened with Module.memoryInitializerRequest, status: ' + request.status + ', retrying ' + memoryInitializer);
            doBrowserLoad();
            return;
#if SUPPORT_BASE64_EMBEDDING
          }
#endif
        }
        applyMemoryInitializer(response);
      }
      if (Module['memoryInitializerRequest'].response) {
        setTimeout(useRequest, 0); // it's already here; but, apply it asynchronously
      } else {
        Module['memoryInitializerRequest'].addEventListener('load', useRequest); // wait for it
      }
    } else {
      // fetch it from the network ourselves
      doBrowserLoad();
    }
  }
}
#endif
#endif // MEM_INIT_IN_WASM == 0

#if CYBERDWARF
  Module['cyberdwarf'] = _cyberdwarf_Debugger(cyberDWARFFile);
#endif

#if MODULARIZE
#if MODULARIZE_INSTANCE == 0
// Modularize mode returns a function, which can be called to
// create instances. The instances provide a then() method,
// must like a Promise, that receives a callback. The callback
// is called when the module is ready to run, with the module
// as a parameter. (Like a Promise, it also returns the module
// so you can use the output of .then(..)).
Module['then'] = function(func) {
  // We may already be ready to run code at this time. if
  // so, just queue a call to the callback.
  if (Module['calledRun']) {
    func(Module);
  } else {
    // we are not ready to call then() yet. we must call it
    // at the same time we would call onRuntimeInitialized.
    var old = Module['onRuntimeInitialized'];
    Module['onRuntimeInitialized'] = function() {
      if (old) old();
      func(Module);
    };
  }
  return Module;
};
#endif
#endif

/**
 * @constructor
 * @extends {Error}
 * @this {ExitStatus}
 */
function ExitStatus(status) {
  this.name = "ExitStatus";
  this.message = "Program terminated with exit(" + status + ")";
  this.status = status;
};
ExitStatus.prototype = new Error();
ExitStatus.prototype.constructor = ExitStatus;

var initialStackTop;
var calledMain = false;

dependenciesFulfilled = function runCaller() {
  // If run has never been called, and we should call run (INVOKE_RUN is true, and Module.noInitialRun is not false)
  if (!Module['calledRun']) run();
  if (!Module['calledRun']) dependenciesFulfilled = runCaller; // try this again later, after new deps are fulfilled
}

#if HAS_MAIN
Module['callMain'] = function callMain(args) {
#if ASSERTIONS
  assert(runDependencies == 0, 'cannot call main when async dependencies remain! (listen on __ATMAIN__)');
  assert(__ATPRERUN__.length == 0, 'cannot call main when preRun functions remain to be called');
#endif

  args = args || [];

  ensureInitRuntime();

  var argc = args.length+1;
  var argv = stackAlloc((argc + 1) * {{{ Runtime.POINTER_SIZE }}});
  HEAP32[argv >> 2] = allocateUTF8OnStack(Module['thisProgram']);
  for (var i = 1; i < argc; i++) {
    HEAP32[(argv >> 2) + i] = allocateUTF8OnStack(args[i - 1]);
  }
  HEAP32[(argv >> 2) + argc] = 0;

#if EMTERPRETIFY_ASYNC
  var initialEmtStackTop = Module['emtStackSave']();
#endif

  try {
#if BENCHMARK
    var start = Date.now();
#endif

#if PROXY_TO_PTHREAD
    // User requested the PROXY_TO_PTHREAD option, so call a stub main which pthread_create()s a new thread
    // that will call the user's real main() for the application.
    var ret = Module['_proxy_main'](argc, argv, 0);
#else
    var ret = Module['_main'](argc, argv, 0);
#endif

#if BENCHMARK
    Module.realPrint('main() took ' + (Date.now() - start) + ' milliseconds');
#endif

#if EMTERPRETIFY_ASYNC
    // if we are saving the stack, then do not call exit, we are not
    // really exiting now, just unwinding the JS stack
    if (typeof EmterpreterAsync === 'object' && EmterpreterAsync.state !== 1) {
#endif // EMTERPRETIFY_ASYNC
    // if we're not running an evented main loop, it's time to exit
      exit(ret, /* implicit = */ true);
#if EMTERPRETIFY_ASYNC
    }
#endif // EMTERPRETIFY_ASYNC
  }
  catch(e) {
    if (e instanceof ExitStatus) {
      // exit() throws this once it's done to make sure execution
      // has been stopped completely
      return;
    } else if (e == 'SimulateInfiniteLoop') {
      // running an evented main loop, don't immediately exit
      Module['noExitRuntime'] = true;
#if EMTERPRETIFY_ASYNC
      // an infinite loop keeps the C stack around, but the emterpreter stack must be unwound - we do not want to restore the call stack at infinite loop
      Module['emtStackRestore'](initialEmtStackTop);
#endif
      return;
    } else {
      var toLog = e;
      if (e && typeof e === 'object' && e.stack) {
        toLog = [e, e.stack];
      }
      err('exception thrown: ' + toLog);
      Module['quit'](1, e);
    }
  } finally {
    calledMain = true;
  }
}
#endif // HAS_MAIN

{{GLOBAL_VARS}}

/** @type {function(Array=)} */
function run(args) {
  args = args || Module['arguments'];

  if (runDependencies > 0) {
#if RUNTIME_LOGGING
    err('run() called, but dependencies remain, so not running');
#endif
    return;
  }

#if STACK_OVERFLOW_CHECK
  writeStackCookie();
#endif

  preRun();

  if (runDependencies > 0) return; // a preRun added a dependency, run will be called later
  if (Module['calledRun']) return; // run may have just been called through dependencies being fulfilled just in this very frame

  function doRun() {
    if (Module['calledRun']) return; // run may have just been called while the async setStatus time below was happening
    Module['calledRun'] = true;

    if (ABORT) return;

    ensureInitRuntime();

    preMain();

    if (Module['onRuntimeInitialized']) Module['onRuntimeInitialized']();

#if HAS_MAIN
    if (Module['_main'] && shouldRunNow) Module['callMain'](args);
#else
#if ASSERTIONS
    assert(!Module['_main'], 'compiled without a main, but one is present. if you added it from JS, use Module["onRuntimeInitialized"]');
#endif // ASSERTIONS
#endif // HAS_MAIN

    postRun();
  }

  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(function() {
      setTimeout(function() {
        Module['setStatus']('');
      }, 1);
      doRun();
    }, 1);
  } else {
    doRun();
  }
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
}
Module['run'] = run;

#if ASSERTIONS
#if EXIT_RUNTIME == 0
function checkUnflushedContent() {
  // Compiler settings do not allow exiting the runtime, so flushing
  // the streams is not possible. but in ASSERTIONS mode we check
  // if there was something to flush, and if so tell the user they
  // should request that the runtime be exitable.
  // Normally we would not even include flush() at all, but in ASSERTIONS
  // builds we do so just for this check, and here we see if there is any
  // content to flush, that is, we check if there would have been
  // something a non-ASSERTIONS build would have not seen.
  // How we flush the streams depends on whether we are in FILESYSTEM=0
  // mode (which has its own special function for this; otherwise, all
  // the code is inside libc)
  var print = out;
  var printErr = err;
  var has = false;
  out = err = function(x) {
    has = true;
  }
  try { // it doesn't matter if it fails
#if FILESYSTEM == 0
    var flush = {{{ '$flush_NO_FILESYSTEM' in addedLibraryItems ? 'flush_NO_FILESYSTEM' : 'null' }}};
#else
    var flush = Module['_fflush'];
#endif
    if (flush) flush(0);
#if FILESYSTEM
    // also flush in the JS FS layer
    var hasFS = {{{ '$FS' in addedLibraryItems ? 'true' : 'false' }}};
    if (hasFS) {
      ['stdout', 'stderr'].forEach(function(name) {
        var info = FS.analyzePath('/dev/' + name);
        if (!info) return;
        var stream = info.object;
        var rdev = stream.rdev;
        var tty = TTY.ttys[rdev];
        if (tty && tty.output && tty.output.length) {
          has = true;
        }
      });
    }
#endif
  } catch(e) {}
  out = print;
  err = printErr;
  if (has) {
    warnOnce('stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the FAQ), or make sure to emit a newline when you printf etc.');
#if FILESYSTEM == 0
    warnOnce('(this may also be due to not including full filesystem support - try building with -s FORCE_FILESYSTEM=1)');
#endif
  }
}
#endif // EXIT_RUNTIME
#endif // ASSERTIONS

function exit(status, implicit) {
#if ASSERTIONS
#if EXIT_RUNTIME == 0
  checkUnflushedContent();
#endif // EXIT_RUNTIME
#endif // ASSERTIONS

  // if this is just main exit-ing implicitly, and the status is 0, then we
  // don't need to do anything here and can just leave. if the status is
  // non-zero, though, then we need to report it.
  // (we may have warned about this earlier, if a situation justifies doing so)
  if (implicit && Module['noExitRuntime'] && status === 0) {
    return;
  }

  if (Module['noExitRuntime']) {
#if ASSERTIONS
    // if exit() was called, we may warn the user if the runtime isn't actually being shut down
    if (!implicit) {
#if EXIT_RUNTIME == 0
      err('exit(' + status + ') called, but EXIT_RUNTIME is not set, so halting execution but not exiting the runtime or preventing further async execution (build with EXIT_RUNTIME=1, if you want a true shutdown)');
#else
      err('exit(' + status + ') called, but noExitRuntime is set due to an async operation, so halting execution but not exiting the runtime or preventing further async execution (you can use emscripten_force_exit, if you want to force a true shutdown)');
#endif // EXIT_RUNTIME
    }
#endif // ASSERTIONS
  } else {
#if USE_PTHREADS
    PThread.terminateAllThreads();
#endif

    ABORT = true;
    EXITSTATUS = status;
    STACKTOP = initialStackTop;

    exitRuntime();

    if (Module['onExit']) Module['onExit'](status);
  }

  Module['quit'](status, new ExitStatus(status));
}

var abortDecorators = [];

function abort(what) {
  if (Module['onAbort']) {
    Module['onAbort'](what);
  }

#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) console.error('Pthread aborting at ' + new Error().stack);
#endif
  if (what !== undefined) {
    out(what);
    err(what);
    what = JSON.stringify(what)
  } else {
    what = '';
  }

  ABORT = true;
  EXITSTATUS = 1;

#if ASSERTIONS == 0
  throw 'abort(' + what + '). Build with -s ASSERTIONS=1 for more info.';
#else
  var extra = '';
  var output = 'abort(' + what + ') at ' + stackTrace() + extra;
  if (abortDecorators) {
    abortDecorators.forEach(function(decorator) {
      output = decorator(output, what);
    });
  }
  throw output;
#endif // ASSERTIONS
}
Module['abort'] = abort;

if (Module['preInit']) {
  if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
  while (Module['preInit'].length > 0) {
    Module['preInit'].pop()();
  }
}

#if HAS_MAIN
// shouldRunNow refers to calling main(), not run().
#if INVOKE_RUN
var shouldRunNow = true;
#else
var shouldRunNow = false;
#endif
if (Module['noInitialRun']) {
  shouldRunNow = false;
}
#endif // HAS_MAIN

#if EXIT_RUNTIME == 0
#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) // EXIT_RUNTIME=0 only applies to default behavior of the main browser thread
#endif
  Module["noExitRuntime"] = true;
#endif

#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) run();
#else
run();
#endif

#if BUILD_AS_WORKER

var workerResponded = false, workerCallbackId = -1;

(function() {
  var messageBuffer = null, buffer = 0, bufferSize = 0;

  function flushMessages() {
    if (!messageBuffer) return;
    if (runtimeInitialized) {
      var temp = messageBuffer;
      messageBuffer = null;
      temp.forEach(function(message) {
        onmessage(message);
      });
    }
  }

  function messageResender() {
    flushMessages();
    if (messageBuffer) {
      setTimeout(messageResender, 100); // still more to do
    }
  }

  onmessage = function onmessage(msg) {
    // if main has not yet been called (mem init file, other async things), buffer messages
    if (!runtimeInitialized) {
      if (!messageBuffer) {
        messageBuffer = [];
        setTimeout(messageResender, 100);
      }
      messageBuffer.push(msg);
      return;
    }
    flushMessages();

    var func = Module['_' + msg.data['funcName']];
    if (!func) throw 'invalid worker function to call: ' + msg.data['funcName'];
    var data = msg.data['data'];
    if (data) {
      if (!data.byteLength) data = new Uint8Array(data);
      if (!buffer || bufferSize < data.length) {
        if (buffer) _free(buffer);
        bufferSize = data.length;
        buffer = _malloc(data.length);
      }
      HEAPU8.set(data, buffer);
    }

    workerResponded = false;
    workerCallbackId = msg.data['callbackId'];
    if (data) {
      func(buffer, data.length);
    } else {
      func(0, 0);
    }
  }
})();

#endif
