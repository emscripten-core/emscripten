/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// === Preamble library stuff ===

// Documentation for the public APIs defined in this file must be updated in:
//    site/source/docs/api_reference/preamble.js.rst
// A prebuilt local version of the documentation is available at:
//    site/build/text/docs/api_reference/preamble.js.txt
// You can also build docs locally as HTML or other formats in site/
// An online HTML version (which may be of a different version of Emscripten)
//    is up at http://kripken.github.io/emscripten-site/docs/api_reference/preamble.js.html

#if BENCHMARK
Module.realPrint = out;
out = err = () => {};
#endif

#if RELOCATABLE
{{{ makeModuleReceiveWithVar('dynamicLibraries', undefined, '[]', true) }}}
#endif

{{{ makeModuleReceiveWithVar('wasmBinary') }}}
{{{ makeModuleReceiveWithVar('noExitRuntime', undefined, EXIT_RUNTIME ? 'false' : 'true') }}}

#if WASM != 2 && MAYBE_WASM2JS
#if !WASM2JS
if (Module['doWasm2JS']) {
#endif
#include "wasm2js.js"
#if !WASM2JS
}
#endif
#endif

#if WASM == 1
if (typeof WebAssembly != 'object') {
  abort('no native wasm support detected');
}
#endif

#if SAFE_HEAP
#include "runtime_safe_heap.js"
#endif

#if USE_ASAN
#include "runtime_asan.js"
#endif

// Wasm globals

var wasmMemory;

#if SHARED_MEMORY
// For sending to workers.
var wasmModule;
#endif // SHARED_MEMORY

//========================================
// Runtime essentials
//========================================

// whether we are quitting the application. no code should run after this.
// set in exit() and abort()
var ABORT = false;

// set by exit() and abort().  Passed to 'onExit' handler.
// NOTE: This is also used as the process return code code in shell environments
// but only when noExitRuntime is false.
var EXITSTATUS;

/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) {
#if ASSERTIONS
    abort('Assertion failed' + (text ? ': ' + text : ''));
#else
    // This build was created without ASSERTIONS defined.  `assert()` should not
    // ever be called in this configuration but in case there are callers in
    // the wild leave this simple abort() implemenation here for now.
    abort(text);
#endif
  }
}

// Returns the C function with a specified identifier (for C++, you need to do manual name mangling)
function getCFunc(ident) {
  var func = Module['_' + ident]; // closure exported function
#if ASSERTIONS
  assert(func, 'Cannot call unknown function ' + ident + ', make sure it is exported');
#endif
  return func;
}

// C calling interface.
/** @param {string|null=} returnType
    @param {Array=} argTypes
    @param {Arguments|Array=} args
    @param {Object=} opts */
function ccall(ident, returnType, argTypes, args, opts) {
  // For fast lookup of conversion functions
  var toC = {
#if MEMORY64
    'pointer': (p) => {{{ to64('p') }}},
#endif
    'string': function(str) {
      var ret = 0;
      if (str !== null && str !== undefined && str !== 0) { // null string
        // at most 4 bytes per UTF-8 code point, +1 for the trailing '\0'
        var len = (str.length << 2) + 1;
        ret = stackAlloc(len);
        stringToUTF8(str, ret, len);
      }
      return {{{ to64('ret') }}};
    },
    'array': function(arr) {
      var ret = stackAlloc(arr.length);
      writeArrayToMemory(arr, ret);
      return {{{ to64('ret') }}};
    }
  };

  function convertReturnValue(ret) {
    if (returnType === 'string') {
      {{{ from64('ret') }}}
      return UTF8ToString(ret);
    }
#if MEMORY64
    if (returnType === 'pointer') return Number(ret);
#endif
    if (returnType === 'boolean') return Boolean(ret);
    return ret;
  }

  var func = getCFunc(ident);
  var cArgs = [];
  var stack = 0;
#if ASSERTIONS
  assert(returnType !== 'array', 'Return type should not be "array".');
#endif
  if (args) {
    for (var i = 0; i < args.length; i++) {
      var converter = toC[argTypes[i]];
      if (converter) {
        if (stack === 0) stack = stackSave();
        cArgs[i] = converter(args[i]);
      } else {
        cArgs[i] = args[i];
      }
    }
  }
#if ASYNCIFY
  // Data for a previous async operation that was in flight before us.
  var previousAsync = Asyncify.currData;
#endif
  var ret = func.apply(null, cArgs);
  function onDone(ret) {
#if ASYNCIFY
    runtimeKeepalivePop();
#endif
    if (stack !== 0) stackRestore(stack);
    return convertReturnValue(ret);
  }
#if ASYNCIFY
  // Keep the runtime alive through all calls. Note that this call might not be
  // async, but for simplicity we push and pop in all calls.
  runtimeKeepalivePush();
  var asyncMode = opts && opts.async;
  if (Asyncify.currData != previousAsync) {
#if ASSERTIONS
    // A change in async operation happened. If there was already an async
    // operation in flight before us, that is an error: we should not start
    // another async operation while one is active, and we should not stop one
    // either. The only valid combination is to have no change in the async
    // data (so we either had one in flight and left it alone, or we didn't have
    // one), or to have nothing in flight and to start one.
    assert(!(previousAsync && Asyncify.currData), 'We cannot start an async operation when one is already flight');
    assert(!(previousAsync && !Asyncify.currData), 'We cannot stop an async operation in flight');
#endif
    // This is a new async operation. The wasm is paused and has unwound its stack.
    // We need to return a Promise that resolves the return value
    // once the stack is rewound and execution finishes.
#if ASSERTIONS
    assert(asyncMode, 'The call to ' + ident + ' is running asynchronously. If this was intended, add the async option to the ccall/cwrap call.');
#endif
    return Asyncify.whenDone().then(onDone);
  }
#endif

  ret = onDone(ret);
#if ASYNCIFY
  // If this is an async ccall, ensure we return a promise
  if (asyncMode) return Promise.resolve(ret);
#endif
  return ret;
}

/** @param {string=} returnType
    @param {Array=} argTypes
    @param {Object=} opts */
function cwrap(ident, returnType, argTypes, opts) {
#if !ASSERTIONS
  argTypes = argTypes || [];
  // When the function takes numbers and returns a number, we can just return
  // the original function
  var numericArgs = argTypes.every(function(type){ return type === 'number'});
  var numericRet = returnType !== 'string';
  if (numericRet && numericArgs && !opts) {
    return getCFunc(ident);
  }
#endif
  return function() {
    return ccall(ident, returnType, argTypes, arguments, opts);
  }
}

#if ASSERTIONS
// We used to include malloc/free by default in the past. Show a helpful error in
// builds with assertions.
#if !hasExportedSymbol('malloc')
function _malloc() {
  abort("malloc() called but not included in the build - add '_malloc' to EXPORTED_FUNCTIONS");
}
#endif // malloc
#if !hasExportedSymbol('free')
function _free() {
  // Show a helpful error since we used to include free by default in the past.
  abort("free() called but not included in the build - add '_free' to EXPORTED_FUNCTIONS");
}
#endif // free
#endif // ASSERTIONS

#include "runtime_strings.js"
#include "runtime_strings_extra.js"

// Memory management

var HEAP,
/** @type {!ArrayBuffer} */
  buffer,
/** @type {!Int8Array} */
  HEAP8,
/** @type {!Uint8Array} */
  HEAPU8,
/** @type {!Int16Array} */
  HEAP16,
/** @type {!Uint16Array} */
  HEAPU16,
/** @type {!Int32Array} */
  HEAP32,
/** @type {!Uint32Array} */
  HEAPU32,
/** @type {!Float32Array} */
  HEAPF32,
#if WASM_BIGINT
/* BigInt64Array type is not correctly defined in closure
/** not-@type {!BigInt64Array} */
  HEAP64,
/* BigUInt64Array type is not correctly defined in closure
/** not-t@type {!BigUint64Array} */
  HEAPU64,
#endif
/** @type {!Float64Array} */
  HEAPF64;

#if SUPPORT_BIG_ENDIAN
var HEAP_DATA_VIEW;
#endif

#if USE_PTHREADS
if (ENVIRONMENT_IS_PTHREAD) {
  // Grab imports from the pthread to local scope.
  buffer = Module['buffer'];
  // Note that not all runtime fields are imported above
}
#endif

function updateGlobalBufferAndViews(buf) {
  buffer = buf;
#if SUPPORT_BIG_ENDIAN
  Module['HEAP_DATA_VIEW'] = HEAP_DATA_VIEW = new DataView(buf);
#endif
  Module['HEAP8'] = HEAP8 = new Int8Array(buf);
  Module['HEAP16'] = HEAP16 = new Int16Array(buf);
  Module['HEAP32'] = HEAP32 = new Int32Array(buf);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(buf);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(buf);
  Module['HEAPU32'] = HEAPU32 = new Uint32Array(buf);
  Module['HEAPF32'] = HEAPF32 = new Float32Array(buf);
  Module['HEAPF64'] = HEAPF64 = new Float64Array(buf);
#if WASM_BIGINT
  Module['HEAP64'] = HEAP64 = new BigInt64Array(buf);
  Module['HEAPU64'] = HEAPU64 = new BigUint64Array(buf);
#endif
}

var TOTAL_STACK = {{{ TOTAL_STACK }}};
#if ASSERTIONS
if (Module['TOTAL_STACK']) assert(TOTAL_STACK === Module['TOTAL_STACK'], 'the stack size can no longer be determined at runtime')
#endif

{{{ makeModuleReceiveWithVar('INITIAL_MEMORY', undefined, INITIAL_MEMORY) }}}

#if ASSERTIONS
assert(INITIAL_MEMORY >= TOTAL_STACK, 'INITIAL_MEMORY should be larger than TOTAL_STACK, was ' + INITIAL_MEMORY + '! (TOTAL_STACK=' + TOTAL_STACK + ')');

// check for full engine support (use string 'subarray' to avoid closure compiler confusion)
assert(typeof Int32Array != 'undefined' && typeof Float64Array !== 'undefined' && Int32Array.prototype.subarray != undefined && Int32Array.prototype.set != undefined,
       'JS engine does not provide full typed array support');
#endif

#if IMPORTED_MEMORY
// In non-standalone/normal mode, we create the memory here.
#include "runtime_init_memory.js"
#else // IMPORTED_MEMORY
#if ASSERTIONS
// If memory is defined in wasm, the user can't provide it.
assert(!Module['wasmMemory'], 'Use of `wasmMemory` detected.  Use -sIMPORTED_MEMORY to define wasmMemory externally');
assert(INITIAL_MEMORY == {{{INITIAL_MEMORY}}}, 'Detected runtime INITIAL_MEMORY setting.  Use -sIMPORTED_MEMORY to define wasmMemory dynamically');
#endif // ASSERTIONS
#endif // IMPORTED_MEMORY

#include "runtime_init_table.js"
#include "runtime_stack_check.js"
#include "runtime_assertions.js"

var __ATPRERUN__  = []; // functions called before the runtime is initialized
var __ATINIT__    = []; // functions called during startup
#if HAS_MAIN
var __ATMAIN__    = []; // functions called when main() is to be run
#endif
var __ATEXIT__    = []; // functions called during shutdown
var __ATPOSTRUN__ = []; // functions called after the main() is called

#if RELOCATABLE
var __RELOC_FUNCS__ = [];
#endif

var runtimeInitialized = false;

#if EXIT_RUNTIME
var runtimeExited = false;
var runtimeKeepaliveCounter = 0;

function keepRuntimeAlive() {
  return noExitRuntime || runtimeKeepaliveCounter > 0;
}
#else
function keepRuntimeAlive() {
  return noExitRuntime;
}
#endif

function preRun() {
#if ASSERTIONS && USE_PTHREADS
  assert(!ENVIRONMENT_IS_PTHREAD); // PThreads reuse the runtime from the main thread.
#endif

#if expectToReceiveOnModule('preRun')
  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    while (Module['preRun'].length) {
      addOnPreRun(Module['preRun'].shift());
    }
  }
#endif

  callRuntimeCallbacks(__ATPRERUN__);
}

function initRuntime() {
#if ASSERTIONS
  assert(!runtimeInitialized);
#endif
  runtimeInitialized = true;

#if WASM_WORKERS
  if (ENVIRONMENT_IS_WASM_WORKER) return __wasm_worker_initializeRuntime();
#endif

#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return;
#endif

#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif

#if STACK_OVERFLOW_CHECK >= 2
#if RUNTIME_LOGGING
  err('__set_stack_limits: ' + _emscripten_stack_get_base() + ', ' + _emscripten_stack_get_end());
#endif
  ___set_stack_limits(_emscripten_stack_get_base(), _emscripten_stack_get_end());
#endif
#if RELOCATABLE
  callRuntimeCallbacks(__RELOC_FUNCS__);
#endif
  <<< ATINITS >>>
  callRuntimeCallbacks(__ATINIT__);
}

#if HAS_MAIN
function preMain() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  <<< ATMAINS >>>
  callRuntimeCallbacks(__ATMAIN__);
}
#endif

#if EXIT_RUNTIME
function exitRuntime() {
#if RUNTIME_DEBUG
  err('exitRuntime');
#endif
#if ASYNCIFY && ASSERTIONS
  // ASYNCIFY cannot be used once the runtime starts shutting down.
  Asyncify.state = Asyncify.State.Disabled;
#endif
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
#if !STANDALONE_WASM
  ___funcs_on_exit(); // Native atexit() functions
#endif
  callRuntimeCallbacks(__ATEXIT__);
  <<< ATEXITS >>>
#if USE_PTHREADS
  PThread.terminateAllThreads();
#endif
  runtimeExited = true;
}
#endif

function postRun() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif

#if expectToReceiveOnModule('postRun')
  if (Module['postRun']) {
    if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
    while (Module['postRun'].length) {
      addOnPostRun(Module['postRun'].shift());
    }
  }
#endif

  callRuntimeCallbacks(__ATPOSTRUN__);
}

function addOnPreRun(cb) {
  __ATPRERUN__.unshift(cb);
}

function addOnInit(cb) {
  __ATINIT__.unshift(cb);
}

#if HAS_MAIN
function addOnPreMain(cb) {
  __ATMAIN__.unshift(cb);
}
#endif

function addOnExit(cb) {
#if EXIT_RUNTIME
  __ATEXIT__.unshift(cb);
#endif
}

function addOnPostRun(cb) {
  __ATPOSTRUN__.unshift(cb);
}

#include "runtime_math.js"

// A counter of dependencies for calling run(). If we need to
// do asynchronous work before running, increment this and
// decrement it. Incrementing must happen in a place like
// Module.preRun (used by emcc to add file preloading).
// Note that you can add dependencies in preRun, even though
// it happens right before run - run will be postponed until
// the dependencies are met.
var runDependencies = 0;
var runDependencyWatcher = null;
var dependenciesFulfilled = null; // overridden to take different actions when all run dependencies are fulfilled
#if ASSERTIONS
var runDependencyTracking = {};
#endif

function getUniqueRunDependency(id) {
#if ASSERTIONS
  var orig = id;
  while (1) {
    if (!runDependencyTracking[id]) return id;
    id = orig + Math.random();
  }
#else
  return id;
#endif
}

function addRunDependency(id) {
  runDependencies++;

#if expectToReceiveOnModule('monitorRunDependencies')
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
#endif

#if ASSERTIONS
  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
    if (runDependencyWatcher === null && typeof setInterval != 'undefined') {
      // Check for missing dependencies every few seconds
      runDependencyWatcher = setInterval(function() {
        if (ABORT) {
          clearInterval(runDependencyWatcher);
          runDependencyWatcher = null;
          return;
        }
        var shown = false;
        for (var dep in runDependencyTracking) {
          if (!shown) {
            shown = true;
            err('still waiting on run dependencies:');
          }
          err('dependency: ' + dep);
        }
        if (shown) {
          err('(end of list)');
        }
      }, 10000);
    }
  } else {
    err('warning: run dependency added without ID');
  }
#endif
}

function removeRunDependency(id) {
  runDependencies--;

#if expectToReceiveOnModule('monitorRunDependencies')
  if (Module['monitorRunDependencies']) {
    Module['monitorRunDependencies'](runDependencies);
  }
#endif

#if ASSERTIONS
  if (id) {
    assert(runDependencyTracking[id]);
    delete runDependencyTracking[id];
  } else {
    err('warning: run dependency removed without ID');
  }
#endif
  if (runDependencies == 0) {
    if (runDependencyWatcher !== null) {
      clearInterval(runDependencyWatcher);
      runDependencyWatcher = null;
    }
    if (dependenciesFulfilled) {
      var callback = dependenciesFulfilled;
      dependenciesFulfilled = null;
      callback(); // can add another dependenciesFulfilled
    }
  }
}

/** @param {string|number=} what */
function abort(what) {
#if expectToReceiveOnModule('onAbort')
#if USE_PTHREADS
  // When running on a pthread, none of the incoming parameters on the module
  // object are present.  The `onAbort` handler only exists on the main thread
  // and so we need to proxy the handling of these back to the main thread.
  // TODO(sbc): Extend this to all such handlers that can be passed into on
  // module creation.
  if (ENVIRONMENT_IS_PTHREAD) {
    postMessage({ 'cmd': 'onAbort', 'arg': what});
  } else
#endif
  {
    if (Module['onAbort']) {
      Module['onAbort'](what);
    }
  }
#endif

  what = 'Aborted(' + what + ')';
  // TODO(sbc): Should we remove printing and leave it up to whoever
  // catches the exception?
  err(what);

  ABORT = true;
  EXITSTATUS = 1;

#if ASSERTIONS == 0
  what += '. Build with -sASSERTIONS for more info.';
#endif // ASSERTIONS

  // Use a wasm runtime error, because a JS error might be seen as a foreign
  // exception, which means we'd run destructors on it. We need the error to
  // simply make the program stop.
  // FIXME This approach does not work in Wasm EH because it currently does not assume
  // all RuntimeErrors are from traps; it decides whether a RuntimeError is from
  // a trap or not based on a hidden field within the object. So at the moment
  // we don't have a way of throwing a wasm trap from JS. TODO Make a JS API that
  // allows this in the wasm spec.

  // Suppress closure compiler warning here. Closure compiler's builtin extern
  // defintion for WebAssembly.RuntimeError claims it takes no arguments even
  // though it can.
  // TODO(https://github.com/google/closure-compiler/pull/3913): Remove if/when upstream closure gets fixed.
#if WASM_EXCEPTIONS == 1
  // See above, in the meantime, we resort to wasm code for trapping.
  ___trap();
#else
  /** @suppress {checkTypes} */
  var e = new WebAssembly.RuntimeError(what);

#if MODULARIZE
  readyPromiseReject(e);
#endif
  // Throw the error whether or not MODULARIZE is set because abort is used
  // in code paths apart from instantiation where an exception is expected
  // to be thrown when abort is called.
  throw e;
#endif
}

// {{MEM_INITIALIZER}}

#include "memoryprofiler.js"

#if ASSERTIONS && !('$FS' in addedLibraryItems) && !WASMFS
// show errors on likely calls to FS when it was not included
var FS = {
  error: function() {
    abort('Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with -sFORCE_FILESYSTEM');
  },
  init: function() { FS.error() },
  createDataFile: function() { FS.error() },
  createPreloadedFile: function() { FS.error() },
  createLazyFile: function() { FS.error() },
  open: function() { FS.error() },
  mkdev: function() { FS.error() },
  registerDevice: function() { FS.error() },
  analyzePath: function() { FS.error() },
  loadFilesFromDB: function() { FS.error() },

  ErrnoError: function ErrnoError() { FS.error() },
};
Module['FS_createDataFile'] = FS.createDataFile;
Module['FS_createPreloadedFile'] = FS.createPreloadedFile;
#endif

#include "URIUtils.js"

#if ASSERTIONS
/** @param {boolean=} fixedasm */
function createExportWrapper(name, fixedasm) {
  return function() {
    var displayName = name;
    var asm = fixedasm;
    if (!fixedasm) {
      asm = Module['asm'];
    }
    assert(runtimeInitialized, 'native function `' + displayName + '` called before runtime initialization');
#if EXIT_RUNTIME
    assert(!runtimeExited, 'native function `' + displayName + '` called after runtime exit (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
#endif
    if (!asm[name]) {
      assert(asm[name], 'exported native function `' + displayName + '` not found');
    }
    return asm[name].apply(null, arguments);
  };
}
#endif

#if ABORT_ON_WASM_EXCEPTIONS
// When exception catching is enabled (!DISABLE_EXCEPTION_CATCHING)
// `abortWrapperDepth` counts the recursion level of the wrapper function so
// that we only handle exceptions at the top level letting the exception
// mechanics work uninterrupted at the inner level.  Additionally,
// `abortWrapperDepth` is also manually incremented in callMain so that we know
// to ignore exceptions from there since they're handled by callMain directly.
var abortWrapperDepth = 0;

// Creates a wrapper in a closure so that each wrapper gets it's own copy of 'original'
function makeAbortWrapper(original) {
  return function() {
    // Don't allow this function to be called if we're aborted!
    if (ABORT) {
      throw "program has already aborted!";
    }

#if !DISABLE_EXCEPTION_CATCHING
    abortWrapperDepth += 1;
#endif
    try {
      return original.apply(null, arguments);
    }
    catch (e) {
      if (
        ABORT // rethrow exception if abort() was called in the original function call above
        || abortWrapperDepth > 1 // rethrow exceptions not caught at the top level if exception catching is enabled; rethrow from exceptions from within callMain
#if SUPPORT_LONGJMP == 'emscripten'
        || e === Infinity // rethrow longjmp if enabled (In Emscripten EH format longjmp will throw Infinity)
#endif
      ) {
        throw e;
      }

      abort("unhandled exception: " + [e, e.stack]);
    }
#if !DISABLE_EXCEPTION_CATCHING
    finally {
      abortWrapperDepth -= 1;
    }
#endif
    }
}

// Instrument all the exported functions to:
// - abort if an unhandled exception occurs
// - throw an exception if someone tries to call them after the program has aborted
// See settings.ABORT_ON_WASM_EXCEPTIONS for more info.
function instrumentWasmExportsWithAbort(exports) {
  // Override the exported functions with the wrappers and copy over any other symbols
  var instExports = {};
  for (var name in exports) {
      var original = exports[name];
      if (typeof original == 'function') {
        instExports[name] = makeAbortWrapper(original);
      } else {
        instExports[name] = original;
      }
  }

  return instExports;
}

function instrumentWasmTableWithAbort() {
  // Override the wasmTable get function to return the wrappers
  var realGet = wasmTable.get;
  var wrapperCache = {};
  wasmTable.get = (i) => {
    var func = realGet.call(wasmTable, i);
    var cached = wrapperCache[i];
    if (!cached || cached.func !== func) {
      cached = wrapperCache[i] = {
        func: func,
        wrapper: makeAbortWrapper(func)
      }
    }
    return cached.wrapper;
  };
}
#endif

var wasmBinaryFile;
#if EXPORT_ES6 && USE_ES6_IMPORT_META && !SINGLE_FILE
if (Module['locateFile']) {
#endif
  wasmBinaryFile = '{{{ WASM_BINARY_FILE }}}';
  if (!isDataURI(wasmBinaryFile)) {
    wasmBinaryFile = locateFile(wasmBinaryFile);
  }
#if EXPORT_ES6 && USE_ES6_IMPORT_META && !SINGLE_FILE // in single-file mode, repeating WASM_BINARY_FILE would emit the contents again
} else {
  // Use bundler-friendly `new URL(..., import.meta.url)` pattern; works in browsers too.
  wasmBinaryFile = new URL('{{{ WASM_BINARY_FILE }}}', import.meta.url).toString();
}
#endif

function getBinary(file) {
  try {
    if (file == wasmBinaryFile && wasmBinary) {
      return new Uint8Array(wasmBinary);
    }
#if SUPPORT_BASE64_EMBEDDING
    var binary = tryParseAsDataURI(file);
    if (binary) {
      return binary;
    }
#endif
    if (readBinary) {
      return readBinary(file);
    } else {
#if WASM_ASYNC_COMPILATION
      throw "both async and sync fetching of the wasm failed";
#else
      throw "sync fetching of the wasm failed: you can preload it to Module['wasmBinary'] manually, or emcc.py will do that for you when generating HTML (but not JS)";
#endif
    }
  }
  catch (err) {
    abort(err);
  }
}

function getBinaryPromise() {
  // If we don't have the binary yet, try to to load it asynchronously.
  // Fetch has some additional restrictions over XHR, like it can't be used on a file:// url.
  // See https://github.com/github/fetch/pull/92#issuecomment-140665932
  // Cordova or Electron apps are typically loaded from a file:// url.
  // So use fetch if it is available and the url is not a file, otherwise fall back to XHR.
  if (!wasmBinary && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER)) {
    if (typeof fetch == 'function'
#if ENVIRONMENT_MAY_BE_WEBVIEW
      && !isFileURI(wasmBinaryFile)
#endif
    ) {
      return fetch(wasmBinaryFile, {{{ makeModuleReceiveExpr('fetchSettings', "{ credentials: 'same-origin' }") }}}).then(function(response) {
        if (!response['ok']) {
          throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
        }
        return response['arrayBuffer']();
      }).catch(function () {
          return getBinary(wasmBinaryFile);
      });
    }
#if ENVIRONMENT_MAY_BE_WEBVIEW
    else {
      if (readAsync) {
        // fetch is not available or url is file => try XHR (readAsync uses XHR internally)
        return new Promise(function(resolve, reject) {
          readAsync(wasmBinaryFile, function(response) { resolve(new Uint8Array(/** @type{!ArrayBuffer} */(response))) }, reject)
        });
      }
    }
#endif
  }

  // Otherwise, getBinary should be able to get it synchronously
  return Promise.resolve().then(function() { return getBinary(wasmBinaryFile); });
}

#if LOAD_SOURCE_MAP
var wasmSourceMap;
#include "source_map_support.js"
#endif

#if USE_OFFSET_CONVERTER
var wasmOffsetConverter;
#include "wasm_offset_converter.js"
#endif

#if SPLIT_MODULE
{{{ makeModuleReceiveWithVar('loadSplitModule', undefined, 'instantiateSync',  true) }}}
var splitModuleProxyHandler = {
  'get': function(target, prop, receiver) {
    return function() {
      err('placeholder function called: ' + prop);
      var imports = {'primary': Module['asm']};
      // Replace '.wasm' suffix with '.deferred.wasm'.
      var deferred = wasmBinaryFile.slice(0, -5) + '.deferred.wasm'
      loadSplitModule(deferred, imports, prop);
      err('instantiated deferred module, continuing');
#if RELOCATABLE
      // When the table is dynamically laid out, the placeholder functions names
      // are offsets from the table base. In the main module, the table base is
      // always 1.
      return wasmTable.get(1 + parseInt(prop)).apply(null, arguments);
#else
      return wasmTable.get(prop).apply(null, arguments);
#endif
    }
  }
};
#endif

#if LOAD_SOURCE_MAP
function receiveSourceMapJSON(sourceMap) {
  wasmSourceMap = new WasmSourceMap(sourceMap);
  {{{ runOnMainThread("removeRunDependency('source-map');") }}}
}
#endif

#if SPLIT_MODULE || !WASM_ASYNC_COMPILATION
function instantiateSync(file, info) {
  var instance;
  var module;
  var binary;
  try {
    binary = getBinary(file);
#if NODE_CODE_CACHING
    if (ENVIRONMENT_IS_NODE) {
      var v8 = require('v8');
      // Include the V8 version in the cache name, so that we don't try to
      // load cached code from another version, which fails silently (it seems
      // to load ok, but we do actually recompile the binary every time).
      var cachedCodeFile = '{{{ WASM_BINARY_FILE }}}.' + v8.cachedDataVersionTag() + '.cached';
      cachedCodeFile = locateFile(cachedCodeFile);
      requireNodeFS();
      var hasCached = fs.existsSync(cachedCodeFile);
      if (hasCached) {
#if RUNTIME_LOGGING
        err('NODE_CODE_CACHING: loading module');
#endif
        try {
          module = v8.deserialize(fs.readFileSync(cachedCodeFile));
        } catch (e) {
          err('NODE_CODE_CACHING: failed to deserialize, bad cache file? (' + cachedCodeFile + ')');
          // Save the new compiled code when we have it.
          hasCached = false;
        }
      }
    }
    if (!module) {
      module = new WebAssembly.Module(binary);
    }
    if (ENVIRONMENT_IS_NODE && !hasCached) {
#if RUNTIME_LOGGING
      err('NODE_CODE_CACHING: saving module');
#endif
      fs.writeFileSync(cachedCodeFile, v8.serialize(module));
    }
#else // NODE_CODE_CACHING
    module = new WebAssembly.Module(binary);
#endif // NODE_CODE_CACHING
    instance = new WebAssembly.Instance(module, info);
#if USE_OFFSET_CONVERTER
    wasmOffsetConverter = new WasmOffsetConverter(binary, module);
#endif
  } catch (e) {
    var str = e.toString();
    err('failed to compile wasm module: ' + str);
    if (str.includes('imported Memory') ||
        str.includes('memory import')) {
      err('Memory size incompatibility issues may be due to changing INITIAL_MEMORY at runtime to something too large. Use ALLOW_MEMORY_GROWTH to allow any size memory (and also make sure not to set INITIAL_MEMORY at runtime to something smaller than it was at compile time).');
    }
    throw e;
  }
#if LOAD_SOURCE_MAP
  receiveSourceMapJSON(getSourceMap());
#endif
  return [instance, module];
}
#endif

#if LOAD_SOURCE_MAP || USE_OFFSET_CONVERTER
// When using postMessage to send an object, it is processed by the structured clone algorithm.
// The prototype, and hence methods, on that object is then lost. This function adds back the lost prototype.
// This does not work with nested objects that has prototypes, but it suffices for WasmSourceMap and WasmOffsetConverter.
function resetPrototype(constructor, attrs) {
  var object = Object.create(constructor.prototype);
  return Object.assign(object, attrs);
}
#endif

// Create the wasm instance.
// Receives the wasm imports, returns the exports.
function createWasm() {
  // prepare imports
  var info = {
#if MINIFY_WASM_IMPORTED_MODULES
    'a': asmLibraryArg,
#else // MINIFY_WASM_IMPORTED_MODULES
    'env': asmLibraryArg,
    '{{{ WASI_MODULE_NAME }}}': asmLibraryArg,
#endif // MINIFY_WASM_IMPORTED_MODULES
#if SPLIT_MODULE
    'placeholder': new Proxy({}, splitModuleProxyHandler),
#endif
#if RELOCATABLE
    'GOT.mem': new Proxy(asmLibraryArg, GOTHandler),
    'GOT.func': new Proxy(asmLibraryArg, GOTHandler),
#endif
  };
  // Load the wasm module and create an instance of using native support in the JS engine.
  // handle a generated wasm instance, receiving its exports and
  // performing other necessary setup
  /** @param {WebAssembly.Module=} module*/
  function receiveInstance(instance, module) {
    var exports = instance.exports;

#if RELOCATABLE
    exports = relocateExports(exports, {{{ GLOBAL_BASE }}});
#endif

#if MEMORY64
    exports = instrumentWasmExportsForMemory64(exports);
#endif

#if ASYNCIFY
    exports = Asyncify.instrumentWasmExports(exports);
#endif

#if ABORT_ON_WASM_EXCEPTIONS
    exports = instrumentWasmExportsWithAbort(exports);
#endif

    Module['asm'] = exports;

#if MAIN_MODULE
    var metadata = getDylinkMetadata(module);
#if AUTOLOAD_DYLIBS
    if (metadata.neededDynlibs) {
      dynamicLibraries = metadata.neededDynlibs.concat(dynamicLibraries);
    }
#endif
    mergeLibSymbols(exports, 'main')
#endif

#if USE_PTHREADS
#if MAIN_MODULE
    registerTLSInit(Module['asm']['_emscripten_tls_init'], instance.exports, metadata);
#else
    registerTLSInit(Module['asm']['_emscripten_tls_init']);
#endif
#endif

#if !IMPORTED_MEMORY
    wasmMemory = Module['asm']['memory'];
#if ASSERTIONS
    assert(wasmMemory, "memory not found in wasm exports");
    // This assertion doesn't hold when emscripten is run in --post-link
    // mode.
    // TODO(sbc): Read INITIAL_MEMORY out of the wasm file in post-link mode.
    //assert(wasmMemory.buffer.byteLength === {{{ INITIAL_MEMORY }}});
#endif
    updateGlobalBufferAndViews(wasmMemory.buffer);
#endif
#if !MEM_INIT_IN_WASM
    runMemoryInitializer();
#endif

#if !RELOCATABLE
    wasmTable = Module['asm']['__indirect_function_table'];
#if ASSERTIONS && !PURE_WASI
    assert(wasmTable, "table not found in wasm exports");
#endif
#endif

#if hasExportedSymbol('__wasm_call_ctors')
    addOnInit(Module['asm']['__wasm_call_ctors']);
#endif

#if hasExportedSymbol('__wasm_apply_data_relocs')
    __RELOC_FUNCS__.push(Module['asm']['__wasm_apply_data_relocs']);
#endif

#if ABORT_ON_WASM_EXCEPTIONS
    instrumentWasmTableWithAbort();
#endif

#if !DECLARE_ASM_MODULE_EXPORTS
    // If we didn't declare the asm exports as top level enties this function
    // is in charge of programatically exporting them on the global object.
    exportAsmFunctions(exports);
#endif

#if USE_PTHREADS || WASM_WORKERS
    // We now have the Wasm module loaded up, keep a reference to the compiled module so we can post it to the workers.
    wasmModule = module;
#endif

#if WASM_WORKERS
    if (!ENVIRONMENT_IS_WASM_WORKER) {
#endif
#if USE_PTHREADS
    // Instantiation is synchronous in pthreads and we assert on run dependencies.
    if (!ENVIRONMENT_IS_PTHREAD) {
#if PTHREAD_POOL_SIZE
      var numWorkersToLoad = PThread.unusedWorkers.length;
      PThread.unusedWorkers.forEach(function(w) { PThread.loadWasmModuleToWorker(w, function() {
#if !PTHREAD_POOL_DELAY_LOAD
        // PTHREAD_POOL_DELAY_LOAD==0: we wanted to synchronously wait until the Worker pool
        // has loaded up. If all Workers have finished loading up the Wasm Module, proceed with main()
        if (!--numWorkersToLoad) removeRunDependency('wasm-instantiate');
#endif
      })});
#endif
#if PTHREAD_POOL_DELAY_LOAD || !PTHREAD_POOL_SIZE
      // PTHREAD_POOL_DELAY_LOAD==1 (or no preloaded pool in use): do not wait up for the Workers to
      // instantiate the Wasm module, but proceed with main() immediately.
      removeRunDependency('wasm-instantiate');
#endif
    }
#else // singlethreaded build:
    removeRunDependency('wasm-instantiate');
#endif // ~USE_PTHREADS
#if WASM_WORKERS
    }
#endif

  }
  // we can't run yet (except in a pthread, where we have a custom sync instantiator)
  {{{ runOnMainThread("addRunDependency('wasm-instantiate');") }}}

#if LOAD_SOURCE_MAP
  {{{ runOnMainThread("addRunDependency('source-map');") }}}
#endif

  // Prefer streaming instantiation if available.
#if WASM_ASYNC_COMPILATION
#if ASSERTIONS
  // Async compilation can be confusing when an error on the page overwrites Module
  // (for example, if the order of elements is wrong, and the one defining Module is
  // later), so we save Module and check it later.
  var trueModule = Module;
#endif
  function receiveInstantiationResult(result) {
    // 'result' is a ResultObject object which has both the module and instance.
    // receiveInstance() will swap in the exports (to Module.asm) so they can be called
#if ASSERTIONS
    assert(Module === trueModule, 'the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?');
    trueModule = null;
#endif
#if SHARED_MEMORY || RELOCATABLE
    receiveInstance(result['instance'], result['module']);
#else
    // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193, the above line no longer optimizes out down to the following line.
    // When the regression is fixed, can restore the above USE_PTHREADS-enabled path.
    receiveInstance(result['instance']);
#endif
  }

  function instantiateArrayBuffer(receiver) {
#if USE_OFFSET_CONVERTER
    var savedBinary;
#endif
    return getBinaryPromise().then(function(binary) {
#if USE_OFFSET_CONVERTER
      savedBinary = binary;
#endif
      return WebAssembly.instantiate(binary, info);
    }).then(function (instance) {
#if USE_OFFSET_CONVERTER
      // wasmOffsetConverter needs to be assigned before calling the receiver
      // (receiveInstantiationResult).  See comments below in instantiateAsync.
      wasmOffsetConverter = new WasmOffsetConverter(savedBinary, instance.module);
#endif
      return instance;
    }).then(receiver, function(reason) {
      err('failed to asynchronously prepare wasm: ' + reason);

#if WASM == 2
#if ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
      if (typeof location != 'undefined') {
#endif
        // WebAssembly compilation failed, try running the JS fallback instead.
        var search = location.search;
        if (search.indexOf('_rwasm=0') < 0) {
          location.href += (search ? search + '&' : '?') + '_rwasm=0';
          // Return here to avoid calling abort() below.  The application
          // still has a chance to start sucessfully do we don't want to
          // trigger onAbort or onExit handlers.
          return;
        }
#if ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
      }
#endif
#endif // WASM == 2

#if ASSERTIONS
      // Warn on some common problems.
      if (isFileURI(wasmBinaryFile)) {
        err('warning: Loading from a file URI (' + wasmBinaryFile + ') is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing');
      }
#endif
      abort(reason);
    });
  }

  function instantiateAsync() {
    if (!wasmBinary &&
        typeof WebAssembly.instantiateStreaming == 'function' &&
        !isDataURI(wasmBinaryFile) &&
#if ENVIRONMENT_MAY_BE_WEBVIEW
        // Don't use streaming for file:// delivered objects in a webview, fetch them synchronously.
        !isFileURI(wasmBinaryFile) &&
#endif
#if ENVIRONMENT_MAY_BE_NODE
        // Avoid instantiateStreaming() on Node.js environment for now, as while
        // Node.js v18.1.0 implements it, it does not have a full fetch()
        // implementation yet.
        //
        // Reference:
        //   https://github.com/emscripten-core/emscripten/pull/16917
        !ENVIRONMENT_IS_NODE &&
#endif
        typeof fetch == 'function') {
      return fetch(wasmBinaryFile, {{{ makeModuleReceiveExpr('fetchSettings', "{ credentials: 'same-origin' }") }}}).then(function(response) {
        // Suppress closure warning here since the upstream definition for
        // instantiateStreaming only allows Promise<Repsponse> rather than
        // an actual Response.
        // TODO(https://github.com/google/closure-compiler/pull/3913): Remove if/when upstream closure is fixed.
        /** @suppress {checkTypes} */
        var result = WebAssembly.instantiateStreaming(response, info);

#if USE_OFFSET_CONVERTER
        // We need the wasm binary for the offset converter. Clone the response
        // in order to get its arrayBuffer (cloning should be more efficient
        // than doing another entire request).
        // (We must clone the response now in order to use it later, as if we
        // try to clone it asynchronously lower down then we will get a
        // "response was already consumed" error.)
        var clonedResponsePromise = response.clone().arrayBuffer();
#endif

        return result.then(
#if !USE_OFFSET_CONVERTER
          receiveInstantiationResult,
#else
          function(instantiationResult) {
            // When using the offset converter, we must interpose here. First,
            // the instantiation result must arrive (if it fails, the error
            // handling later down will handle it). Once it arrives, we can
            // initialize the offset converter. And only then is it valid to
            // call receiveInstantiationResult, as that function will use the
            // offset converter (in the case of pthreads, it will create the
            // pthreads and send them the offsets along with the wasm instance).

            clonedResponsePromise.then(function(arrayBufferResult) {
              wasmOffsetConverter = new WasmOffsetConverter(new Uint8Array(arrayBufferResult), instantiationResult.module);
              receiveInstantiationResult(instantiationResult);
            }, function(reason) {
              err('failed to initialize offset-converter: ' + reason);
            });
          },
#endif
          function(reason) {
            // We expect the most common failure cause to be a bad MIME type for the binary,
            // in which case falling back to ArrayBuffer instantiation should work.
            err('wasm streaming compile failed: ' + reason);
            err('falling back to ArrayBuffer instantiation');
            return instantiateArrayBuffer(receiveInstantiationResult);
          });
      });
    } else {
      return instantiateArrayBuffer(receiveInstantiationResult);
    }
  }
#endif

  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to run the instantiation parallel
  // to any other async startup actions they are performing.
  // Also pthreads and wasm workers initialize the wasm instance through this path.
  if (Module['instantiateWasm']) {
#if USE_OFFSET_CONVERTER
#if ASSERTIONS && USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) {
      assert(Module['wasmOffsetData'], 'wasmOffsetData not found on Module object');
    }
#endif
    wasmOffsetConverter = resetPrototype(WasmOffsetConverter, Module['wasmOffsetData']);
#endif
#if LOAD_SOURCE_MAP
#if ASSERTIONS && USE_PTHREADS
    if (ENVIRONMENT_IS_PTHREAD) {
      assert(Module['wasmSourceMapData'], 'wasmSourceMapData not found on Module object');
    }
#endif
    wasmSourceMap = resetPrototype(WasmSourceMap, Module['wasmSourceMapData']);
#endif
    try {
      var exports = Module['instantiateWasm'](info, receiveInstance);
#if ASYNCIFY
      exports = Asyncify.instrumentWasmExports(exports);
#endif
      return exports;
    } catch(e) {
      err('Module.instantiateWasm callback failed with error: ' + e);
      return false;
    }
  }

#if WASM_ASYNC_COMPILATION
#if RUNTIME_LOGGING
  err('asynchronously preparing wasm');
#endif
#if MODULARIZE
  // If instantiation fails, reject the module ready promise.
  instantiateAsync().catch(readyPromiseReject);
#else
  instantiateAsync();
#endif
#if LOAD_SOURCE_MAP
  getSourceMapPromise().then(receiveSourceMapJSON);
#endif
  return {}; // no exports yet; we'll fill them in later
#else
  var result = instantiateSync(wasmBinaryFile, info);
#if USE_PTHREADS || MAIN_MODULE
  receiveInstance(result[0], result[1]);
#else
  // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193,
  // the above line no longer optimizes out down to the following line.
  // When the regression is fixed, we can remove this if/else.
  receiveInstance(result[0]);
#endif
  return Module['asm']; // exports were assigned here
#endif
}

// Globals used by JS i64 conversions (see makeSetValue)
var tempDouble;
var tempI64;

// === Body ===
