/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// === Auto-generated postamble setup entry stuff ===

{{{ exportRuntime() }}}

#if !MEM_INIT_IN_WASM
function runMemoryInitializer() {
#if USE_PTHREADS
  if (!memoryInitializer || ENVIRONMENT_IS_PTHREAD) return;
#else
  if (!memoryInitializer) return
#endif
  if (!isDataURI(memoryInitializer)) {
    memoryInitializer = locateFile(memoryInitializer);
  }
  if (ENVIRONMENT_IS_NODE || ENVIRONMENT_IS_SHELL) {
    var data = readBinary(memoryInitializer);
    HEAPU8.set(data, {{{ GLOBAL_BASE }}});
  } else {
    addRunDependency('memory initializer');
    var applyMemoryInitializer = function(data) {
      if (data.byteLength) data = new Uint8Array(data);
#if ASSERTIONS
      for (var i = 0; i < data.length; i++) {
        assert(HEAPU8[{{{ GLOBAL_BASE }}} + i] === 0, "area for memory initializer should not have been touched before it's loaded");
      }
#endif
      HEAPU8.set(data, {{{ GLOBAL_BASE }}});
      // Delete the typed array that contains the large blob of the memory initializer request response so that
      // we won't keep unnecessary memory lying around. However, keep the XHR object itself alive so that e.g.
      // its .status field can still be accessed later.
      if (Module['memoryInitializerRequest']) delete Module['memoryInitializerRequest'].response;
      removeRunDependency('memory initializer');
    };
    var doBrowserLoad = function() {
      readAsync(memoryInitializer, applyMemoryInitializer, function() {
        var e = new Error('could not load memory initializer ' + memoryInitializer);
#if MODULARIZE
          readyPromiseReject(e);
#else
          throw e;
#endif
      });
    };
#if SUPPORT_BASE64_EMBEDDING
    var memoryInitializerBytes = tryParseAsDataURI(memoryInitializer);
    if (memoryInitializerBytes) {
      applyMemoryInitializer(memoryInitializerBytes.buffer);
    } else
#endif
    if (Module['memoryInitializerRequest']) {
      // a network request has already been created, just use that
      var useRequest = function() {
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
      };
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
#endif // MEM_INIT_IN_WASM == 0

var calledRun;

/**
 * @constructor
 * @this {ExitStatus}
 */
function ExitStatus(status) {
  this.name = "ExitStatus";
  this.message = "Program terminated with exit(" + status + ")";
  this.status = status;
}

var calledMain = false;

#if STANDALONE_WASM && MAIN_READS_PARAMS
var mainArgs = undefined;
#endif

dependenciesFulfilled = function runCaller() {
  // If run has never been called, and we should call run (INVOKE_RUN is true, and Module.noInitialRun is not false)
  if (!calledRun) run();
  if (!calledRun) dependenciesFulfilled = runCaller; // try this again later, after new deps are fulfilled
};

#if HAS_MAIN
function callMain(args) {
#if ASSERTIONS
  assert(runDependencies == 0, 'cannot call main when async dependencies remain! (listen on Module["onRuntimeInitialized"])');
  assert(__ATPRERUN__.length == 0, 'cannot call main when preRun functions remain to be called');
#endif

#if STANDALONE_WASM
#if EXPECT_MAIN
  var entryFunction = Module['__start'];
#else
  var entryFunction = Module['__initialize'];
#endif
#else
#if PROXY_TO_PTHREAD
  // User requested the PROXY_TO_PTHREAD option, so call a stub main which pthread_create()s a new thread
  // that will call the user's real main() for the application.
  var entryFunction = Module['_emscripten_proxy_main'];
#else
  var entryFunction = Module['_main'];
#endif
#endif

#if MAIN_MODULE
  // Main modules can't tell if they have main() at compile time, since it may
  // arrive from a dynamic library.
  if (!entryFunction) return;
#endif

#if MAIN_READS_PARAMS
#if STANDALONE_WASM
  mainArgs = [thisProgram].concat(args)
#else
  args = args || [];

  var argc = args.length+1;
  var argv = stackAlloc((argc + 1) * {{{ Runtime.POINTER_SIZE }}});
  HEAP32[argv >> 2] = allocateUTF8OnStack(thisProgram);
  for (var i = 1; i < argc; i++) {
    HEAP32[(argv >> 2) + i] = allocateUTF8OnStack(args[i - 1]);
  }
  HEAP32[(argv >> 2) + argc] = 0;
#endif // STANDALONE_WASM
#else
  var argc = 0;
  var argv = 0;
#endif // MAIN_READS_PARAMS

  try {
#if BENCHMARK
    var start = Date.now();
#endif

#if ABORT_ON_WASM_EXCEPTIONS
    // See abortWrapperDepth in preamble.js!
    abortWrapperDepth += 2; 
#endif

#if STANDALONE_WASM
    entryFunction();
    // _start (in crt1.c) will call exit() if main return non-zero.  So we know
    // that if we get here main returned zero.
    var ret = 0;
#else
    var ret = entryFunction(argc, argv);
#endif // STANDALONE_WASM

#if BENCHMARK
    Module.realPrint('main() took ' + (Date.now() - start) + ' milliseconds');
#endif

    // In PROXY_TO_PTHREAD builds, we should never exit the runtime below, as
    // execution is asynchronously handed off to a pthread.
#if PROXY_TO_PTHREAD
#if ASSERTIONS
    assert(ret == 0, '_emscripten_proxy_main failed to start proxy thread: ' + ret);
#endif
#else
#if ASYNCIFY
    // if we are saving the stack, then do not call exit, we are not
    // really exiting now, just unwinding the JS stack
    if (!noExitRuntime) {
#endif // ASYNCIFY
      // if we're not running an evented main loop, it's time to exit
      exit(ret, /* implicit = */ true);
#if ASYNCIFY
    }
#endif // ASYNCIFY
  }
  catch(e) {
    if (e instanceof ExitStatus) {
      // exit() throws this once it's done to make sure execution
      // has been stopped completely
      return;
    } else if (e == 'unwind') {
      // running an evented main loop, don't immediately exit
      noExitRuntime = true;
      return;
    } else {
      var toLog = e;
      if (e && typeof e === 'object' && e.stack) {
        toLog = [e, e.stack];
      }
      err('exception thrown: ' + toLog);
      quit_(1, e);
    }
#endif // !PROXY_TO_PTHREAD
  } finally {
    calledMain = true;

#if ABORT_ON_WASM_EXCEPTIONS
    // See abortWrapperDepth in preamble.js!
    abortWrapperDepth -= 2; 
#endif
  }
}
#endif // HAS_MAIN

/** @type {function(Array=)} */
function run(args) {
  args = args || arguments_;

  if (runDependencies > 0) {
#if RUNTIME_LOGGING
    err('run() called, but dependencies remain, so not running');
#endif
    return;
  }

#if STACK_OVERFLOW_CHECK
  // This is normally called automatically during __wasm_call_ctors but need to
  // get these values before even running any of the ctors so we call it redundantly
  // here.
  // TODO(sbc): Move writeStackCookie to native to to avoid this.
#if RELOCATABLE
  _emscripten_stack_set_limits({{{ getQuoted('STACK_BASE') }}}, {{{ getQuoted('STACK_MAX') }}});
#else
  _emscripten_stack_init();
#endif
  writeStackCookie();
#endif

  preRun();

  if (runDependencies > 0) return; // a preRun added a dependency, run will be called later

  function doRun() {
    // run may have just been called through dependencies being fulfilled just in this very frame,
    // or while the async setStatus time below was happening
    if (calledRun) return;
    calledRun = true;
    Module['calledRun'] = true;

    if (ABORT) return;

    initRuntime();

    preMain();

#if MODULARIZE
    readyPromiseResolve(Module);
#endif
#if expectToReceiveOnModule('onRuntimeInitialized')
    if (Module['onRuntimeInitialized']) Module['onRuntimeInitialized']();
#endif

#if HAS_MAIN
    if (shouldRunNow) callMain(args);
#else
#if ASSERTIONS
    assert(!Module['_main'], 'compiled without a main, but one is present. if you added it from JS, use Module["onRuntimeInitialized"]');
#endif // ASSERTIONS
#endif // HAS_MAIN

    postRun();
  }

#if expectToReceiveOnModule('setStatus')
  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(function() {
      setTimeout(function() {
        Module['setStatus']('');
      }, 1);
      doRun();
    }, 1);
  } else
#endif
  {
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
  // How we flush the streams depends on whether we are in SYSCALLS_REQUIRE_FILESYSTEM=0
  // mode (which has its own special function for this; otherwise, all
  // the code is inside libc)
  var oldOut = out;
  var oldErr = err;
  var has = false;
  out = err = function(x) {
    has = true;
  }
  try { // it doesn't matter if it fails
#if SYSCALLS_REQUIRE_FILESYSTEM == 0
    var flush = {{{ '$flush_NO_FILESYSTEM' in addedLibraryItems ? 'flush_NO_FILESYSTEM' : 'null' }}};
    if (flush) flush();
#else
    var flush = Module['_fflush'];
    if (flush) flush(0);
#endif
#if '$FS' in addedLibraryItems && '$TTY' in addedLibraryItems
    // also flush in the JS FS layer
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
#endif
  } catch(e) {}
  out = oldOut;
  err = oldErr;
  if (has) {
    warnOnce('stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the FAQ), or make sure to emit a newline when you printf etc.');
#if FILESYSTEM == 0 || SYSCALLS_REQUIRE_FILESYSTEM == 0
    warnOnce('(this may also be due to not including full filesystem support - try building with -s FORCE_FILESYSTEM=1)');
#endif
  }
}
#endif // EXIT_RUNTIME
#endif // ASSERTIONS

/** @param {boolean|number=} implicit */
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
  if (implicit && noExitRuntime && status === 0) {
    return;
  }

#if USE_PTHREADS
  if (!implicit) {
    if (ENVIRONMENT_IS_PTHREAD) {
#if ASSERTIONS
      err('Pthread 0x' + _pthread_self().toString(16) + ' called exit(), posting exitProcess.');
#endif
      // When running in a pthread we propagate the exit back to the main thread
      // where it can decide if the whole process should be shut down or not.
      // The pthread may have decided not to exit its own runtime, for example
      // because it runs a main loop, but that doesn't affect the main thread.
      postMessage({ 'cmd': 'exitProcess', 'returnCode': status });
      throw new ExitStatus(status);
    } else {
#if ASSERTIONS
      err('main thead called exit: noExitRuntime=' + noExitRuntime);
#endif
    }
  }
#endif

  if (noExitRuntime) {
#if ASSERTIONS
    // if exit() was called, we may warn the user if the runtime isn't actually being shut down
    if (!implicit) {
#if EXIT_RUNTIME == 0
      var msg = 'program exited (with status: ' + status + '), but EXIT_RUNTIME is not set, so halting execution but not exiting the runtime or preventing further async execution (build with EXIT_RUNTIME=1, if you want a true shutdown)';
#else
      var msg = 'program exited (with status: ' + status + '), but noExitRuntime is set due to an async operation, so halting execution but not exiting the runtime or preventing further async execution (you can use emscripten_force_exit, if you want to force a true shutdown)';
#endif // EXIT_RUNTIME
#if MODULARIZE
      readyPromiseReject(msg);
#endif // MODULARIZE
      err(msg);
    }
#endif // ASSERTIONS
  } else {
#if USE_PTHREADS
    PThread.terminateAllThreads();
#endif

    EXITSTATUS = status;

    exitRuntime();

#if expectToReceiveOnModule('onExit')
    if (Module['onExit']) Module['onExit'](status);
#endif

    ABORT = true;
  }

  quit_(status, new ExitStatus(status));
}

#if expectToReceiveOnModule('preInit')
if (Module['preInit']) {
  if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
  while (Module['preInit'].length > 0) {
    Module['preInit'].pop()();
  }
}
#endif

#if HAS_MAIN
// shouldRunNow refers to calling main(), not run().
#if INVOKE_RUN
var shouldRunNow = true;
#else
var shouldRunNow = false;
#endif

#if expectToReceiveOnModule('noInitialRun')
if (Module['noInitialRun']) shouldRunNow = false;
#endif

#endif // HAS_MAIN

#if EXIT_RUNTIME == 0
#if USE_PTHREADS
// EXIT_RUNTIME=0 only applies to the default behavior of the main browser
// thread.
// The default behaviour for pthreads is always to exit once they return
// from their entry point (or call pthread_exit).  If we set noExitRuntime
// to true here on pthreads they would never complete and attempt to
// pthread_join to them would block forever.
// pthreads can still choose to set `noExitRuntime` explicitly, or
// call emscripten_unwind_to_js_event_loop to extend their lifetime beyond
// their main function.  See comment in src/worker.js for more.
noExitRuntime = !ENVIRONMENT_IS_PTHREAD;
#else
noExitRuntime = true;
#endif
#endif

#if USE_PTHREADS
if (!ENVIRONMENT_IS_PTHREAD) {
  run();
} else {
  PThread.initWorker();
}
#else
run();
#endif // USE_PTHREADS

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

#if STANDALONE_WASM && ASSERTIONS && !WASM_BIGINT
err('warning: running JS from STANDALONE_WASM without WASM_BIGINT will fail if a syscall with i64 is used (in standalone mode we cannot legalize syscalls)');
#endif
