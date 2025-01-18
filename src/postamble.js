/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// === Auto-generated postamble setup entry stuff ===

#if HEADLESS
if (!ENVIRONMENT_IS_WEB) {
#include "headlessCanvas.js"
#include "headless.js"
}
#endif

#if PROXY_TO_WORKER
if (ENVIRONMENT_IS_WORKER) {
#include "webGLWorker.js'
#include "proxyWorker.js"
}
#endif

#if DETERMINISTIC
#include "deterministic.js"
#endif

{{{ exportRuntime() }}}

#if ASSERTIONS
var calledRun;
#endif

#if STANDALONE_WASM && MAIN_READS_PARAMS
var mainArgs = undefined;
#endif

#if HAS_MAIN
#if MAIN_READS_PARAMS
{{{ asyncIf(ASYNCIFY == 2) }}}function callMain(args = []) {
#else
{{{ asyncIf(ASYNCIFY == 2) }}}function callMain() {
#endif
#if ASSERTIONS
  assert(runDependencies == 0, 'cannot call main when async dependencies remain! (listen on Module["onRuntimeInitialized"])');
  assert(__ATPRERUN__.length == 0, 'cannot call main when preRun functions remain to be called');
#endif

  var entryFunction = {{{ getEntryFunction() }}};

#if PROXY_TO_PTHREAD
  // With PROXY_TO_PTHREAD make sure we keep the runtime alive until the
  // proxied main calls exit (see exitOnMainThread() for where Pop is called).
  {{{ runtimeKeepalivePush() }}}
#endif

#if MAIN_MODULE
  // Main modules can't tell if they have main() at compile time, since it may
  // arrive from a dynamic library.
  if (!entryFunction) return;
#endif

#if MAIN_READS_PARAMS && STANDALONE_WASM
  mainArgs = [thisProgram].concat(args)
#elif MAIN_READS_PARAMS
  args.unshift(thisProgram);

  var argc = args.length;
  var argv = stackAlloc((argc + 1) * {{{ POINTER_SIZE }}});
  var argv_ptr = argv;
  args.forEach((arg) => {
    {{{ makeSetValue('argv_ptr', 0, 'stringToUTF8OnStack(arg)', '*') }}};
    argv_ptr += {{{ POINTER_SIZE }}};
  });
  {{{ makeSetValue('argv_ptr', 0, 0, '*') }}};
#else
  var argc = 0;
  var argv = 0;
#endif // MAIN_READS_PARAMS

  try {
#if ABORT_ON_WASM_EXCEPTIONS
    // See abortWrapperDepth in preamble.js!
    abortWrapperDepth++;
#endif

#if STANDALONE_WASM
    entryFunction();
    // _start (in crt1.c) will call exit() if main return non-zero.  So we know
    // that if we get here main returned zero.
    var ret = 0;
#else
    var ret = entryFunction(argc, {{{ to64('argv') }}});
#endif // STANDALONE_WASM

#if ASYNCIFY == 2 && !PROXY_TO_PTHREAD
    // The current spec of JSPI returns a promise only if the function suspends
    // and a plain value otherwise. This will likely change:
    // https://github.com/WebAssembly/js-promise-integration/issues/11
    ret = await ret;
#endif // ASYNCIFY == 2
    // if we're not running an evented main loop, it's time to exit
    exitJS(ret, /* implicit = */ true);
    return ret;
  } catch (e) {
    return handleException(e);
  }
#if ABORT_ON_WASM_EXCEPTIONS
  finally {
    // See abortWrapperDepth in preamble.js!
    abortWrapperDepth--;
  }
#endif
}
#endif // HAS_MAIN

#if STACK_OVERFLOW_CHECK
function stackCheckInit() {
  // This is normally called automatically during __wasm_call_ctors but need to
  // get these values before even running any of the ctors so we call it redundantly
  // here.
#if ASSERTIONS && PTHREADS
  // See $establishStackSpace for the equivalent code that runs on a thread
  assert(!ENVIRONMENT_IS_PTHREAD);
#endif
#if RELOCATABLE
  _emscripten_stack_set_limits({{{ STACK_HIGH }}} , {{{ STACK_LOW }}});
#else
  _emscripten_stack_init();
#endif
  // TODO(sbc): Move writeStackCookie to native to to avoid this.
  writeStackCookie();
}
#endif

#if MAIN_MODULE && PTHREADS
// Map of modules to be shared with new threads.  This gets populated by the
// main thread and shared with all new workers via the initial `load` message.
var sharedModules = {};
#endif

#if MAIN_READS_PARAMS
function run(args = arguments_) {
#else
function run() {
#endif

  if (runDependencies > 0) {
#if RUNTIME_DEBUG
    dbg('run() called, but dependencies remain, so not running');
#endif
    dependenciesFulfilled = run;
    return;
  }

#if PTHREADS || WASM_WORKERS
  if ({{{ ENVIRONMENT_IS_WORKER_THREAD() }}}) {
#if MODULARIZE
    readyPromiseResolve(Module);
#endif
    initRuntime();
    return;
  }
#endif

#if STACK_OVERFLOW_CHECK
  stackCheckInit();
#endif

  preRun();

  // a preRun added a dependency, run will be called later
  if (runDependencies > 0) {
#if RUNTIME_DEBUG
    dbg('run() called, but dependencies remain, so not running');
#endif
    dependenciesFulfilled = run;
    return;
  }

  function doRun() {
    // run may have just been called through dependencies being fulfilled just in this very frame,
    // or while the async setStatus time below was happening
#if ASSERTIONS
    assert(!calledRun);
    calledRun = true;
#endif
    Module['calledRun'] = true;

    if (ABORT) return;

    initRuntime();

#if HAS_MAIN
    preMain();
#endif

#if MODULARIZE
    readyPromiseResolve(Module);
#endif
#if expectToReceiveOnModule('onRuntimeInitialized')
    Module['onRuntimeInitialized']?.();
#endif

#if HAS_MAIN
    {{{ makeModuleReceiveWithVar('noInitialRun', undefined, !INVOKE_RUN) }}}
#if MAIN_READS_PARAMS
    if (!noInitialRun) callMain(args);
#else
    if (!noInitialRun) callMain();
#endif
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
    setTimeout(() => {
      setTimeout(() => Module['setStatus'](''), 1);
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
  out = err = (x) => {
    has = true;
  }
  try { // it doesn't matter if it fails
#if SYSCALLS_REQUIRE_FILESYSTEM == 0 && '$flush_NO_FILESYSTEM' in addedLibraryItems
    flush_NO_FILESYSTEM();
#elif WASMFS && hasExportedSymbol('wasmfs_flush')
    // In WasmFS we must also flush the WasmFS internal buffers, for this check
    // to work.
    _wasmfs_flush();
#elif hasExportedSymbol('fflush')
    _fflush(0);
#endif
#if '$FS' in addedLibraryItems && '$TTY' in addedLibraryItems
    // also flush in the JS FS layer
    ['stdout', 'stderr'].forEach((name) => {
      var info = FS.analyzePath('/dev/' + name);
      if (!info) return;
      var stream = info.object;
      var rdev = stream.rdev;
      var tty = TTY.ttys[rdev];
      if (tty?.output?.length) {
        has = true;
      }
    });
#endif
  } catch(e) {}
  out = oldOut;
  err = oldErr;
  if (has) {
    warnOnce('stdio streams had content in them that was not flushed. you should set EXIT_RUNTIME to 1 (see the Emscripten FAQ), or make sure to emit a newline when you printf etc.');
#if FILESYSTEM == 0 || SYSCALLS_REQUIRE_FILESYSTEM == 0
    warnOnce('(this may also be due to not including full filesystem support - try building with -sFORCE_FILESYSTEM)');
#endif
  }
}
#endif // EXIT_RUNTIME
#endif // ASSERTIONS

#if expectToReceiveOnModule('preInit')
if (Module['preInit']) {
  if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
  while (Module['preInit'].length > 0) {
    Module['preInit'].pop()();
  }
}
#endif

run();

#if BUILD_AS_WORKER

var workerResponded = false, workerCallbackId = -1;

(() => {
  var messageBuffer = null, buffer = 0, bufferSize = 0;

  function flushMessages() {
    if (!messageBuffer) return;
    if (runtimeInitialized) {
      var temp = messageBuffer;
      messageBuffer = null;
      temp.forEach((message) => onmessage(message));
    }
  }

  function messageResender() {
    flushMessages();
    if (messageBuffer) {
      setTimeout(messageResender, 100); // still more to do
    }
  }

  onmessage = (msg) => {
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
