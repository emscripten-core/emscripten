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

#if RELOCATABLE
{{{ makeModuleReceiveWithVar('dynamicLibraries', undefined, '[]', true) }}}
#endif

{{{ makeModuleReceiveWithVar('wasmBinary') }}}

#if WASM != 2 && MAYBE_WASM2JS
#if !WASM2JS
if (Module['doWasm2JS']) {
#endif
#include "wasm2js.js"
#if !WASM2JS
}
#endif
#endif

#if MAYBE_WASM2JS
if (WebAssembly.isWasm2js) {
  // We don't need to actually download a wasm binary, mark it as present but
  // empty.
  wasmBinary = [];
}
#endif

#if ASSERTIONS && WASM == 1
if (typeof WebAssembly != 'object') {
  err('no native wasm support detected');
}
#endif

#if SAFE_HEAP
#include "runtime_safe_heap.js"
#endif

#if USE_ASAN
#include "runtime_asan.js"
#endif

#if SUPPORT_BASE64_EMBEDDING || FORCE_FILESYSTEM
#include "base64Utils.js"
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

#if ASSERTIONS || !STRICT
// In STRICT mode, we only define assert() when ASSERTIONS is set.  i.e. we
// don't define it at all in release modes.  This matches the behaviour of
// MINIMAL_RUNTIME.
// TODO(sbc): Make this the default even without STRICT enabled.
/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) {
#if ASSERTIONS
    abort('Assertion failed' + (text ? ': ' + text : ''));
#else
    // This build was created without ASSERTIONS defined.  `assert()` should not
    // ever be called in this configuration but in case there are callers in
    // the wild leave this simple abort() implementation here for now.
    abort(text);
#endif
  }
}
#endif

#if ASSERTIONS
// We used to include malloc/free by default in the past. Show a helpful error in
// builds with assertions.
#if !hasExportedSymbol('malloc')
function _malloc() {
  abort('malloc() called but not included in the build - add `_malloc` to EXPORTED_FUNCTIONS');
}
#endif // malloc
#if !hasExportedSymbol('free')
function _free() {
  // Show a helpful error since we used to include free by default in the past.
  abort('free() called but not included in the build - add `_free` to EXPORTED_FUNCTIONS');
}
#endif // free
#endif // ASSERTIONS

// Memory management

var HEAP,
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
/* BigUint64Array type is not correctly defined in closure
/** not-t@type {!BigUint64Array} */
  HEAPU64,
#endif
/** @type {!Float64Array} */
  HEAPF64;

#if SUPPORT_BIG_ENDIAN
var HEAP_DATA_VIEW;
#endif

#include "runtime_shared.js"

#if PTHREADS
#include "runtime_pthread.js"
#endif

#if ASSERTIONS
assert(!Module['STACK_SIZE'], 'STACK_SIZE can no longer be set at runtime.  Use -sSTACK_SIZE at link time')
#endif

#if ASSERTIONS
assert(typeof Int32Array != 'undefined' && typeof Float64Array !== 'undefined' && Int32Array.prototype.subarray != undefined && Int32Array.prototype.set != undefined,
       'JS engine does not provide full typed array support');
#endif

#if IMPORTED_MEMORY
// In non-standalone/normal mode, we create the memory here.
#include "runtime_init_memory.js"
#elif ASSERTIONS
// If memory is defined in wasm, the user can't provide it, or set INITIAL_MEMORY
assert(!Module['wasmMemory'], 'Use of `wasmMemory` detected.  Use -sIMPORTED_MEMORY to define wasmMemory externally');
assert(!Module['INITIAL_MEMORY'], 'Detected runtime INITIAL_MEMORY setting.  Use -sIMPORTED_MEMORY to define wasmMemory dynamically');
#endif // !IMPORTED_MEMORY && ASSERTIONS

#include "runtime_stack_check.js"

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
#endif

function preRun() {
#if ASSERTIONS && PTHREADS
  assert(!ENVIRONMENT_IS_PTHREAD); // PThreads reuse the runtime from the main thread.
#endif
#if expectToReceiveOnModule('preRun')
  var preRuns = Module['preRun'];
  if (preRuns) {
    if (typeof preRuns == 'function') preRuns = [preRuns];
    preRuns.forEach(addOnPreRun);
  }
#endif
  callRuntimeCallbacks(__ATPRERUN__);
}

function initRuntime() {
#if RUNTIME_DEBUG
  dbg('initRuntime');
#endif
#if ASSERTIONS
  assert(!runtimeInitialized);
#endif
  runtimeInitialized = true;

#if WASM_WORKERS
  if (ENVIRONMENT_IS_WASM_WORKER) return _wasmWorkerInitializeRuntime();
#endif

#if PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return;
#endif

#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif

#if STACK_OVERFLOW_CHECK >= 2
  setStackLimits();
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
#if PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  <<< ATMAINS >>>
  callRuntimeCallbacks(__ATMAIN__);
}
#endif

#if EXIT_RUNTIME
function exitRuntime() {
#if RUNTIME_DEBUG
  dbg('exitRuntime');
#endif
#if ASSERTIONS
  assert(!runtimeExited);
#endif
#if ASYNCIFY == 1 && ASSERTIONS
  // ASYNCIFY cannot be used once the runtime starts shutting down.
  Asyncify.state = Asyncify.State.Disabled;
#endif
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
#if !STANDALONE_WASM
  ___funcs_on_exit(); // Native atexit() functions
#endif
  callRuntimeCallbacks(__ATEXIT__);
  <<< ATEXITS >>>
#if PTHREADS
  PThread.terminateAllThreads();
#endif
  runtimeExited = true;
}
#endif

function postRun() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif

#if expectToReceiveOnModule('postRun')
  var postRuns = Module['postRun'];
  if (postRuns) {
    if (typeof postRuns == 'function') postRuns = [postRuns];
    postRuns.forEach(addOnPostRun);
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
  Module['monitorRunDependencies']?.(runDependencies);
#endif

#if ASSERTIONS
  if (id) {
    assert(!runDependencyTracking[id]);
    runDependencyTracking[id] = 1;
    if (runDependencyWatcher === null && typeof setInterval != 'undefined') {
      // Check for missing dependencies every few seconds
      runDependencyWatcher = setInterval(() => {
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
          err(`dependency: ${dep}`);
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
  Module['monitorRunDependencies']?.(runDependencies);
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
  Module['onAbort']?.(what);
#endif

  what = 'Aborted(' + what + ')';
  // TODO(sbc): Should we remove printing and leave it up to whoever
  // catches the exception?
  err(what);

  ABORT = true;

#if ASSERTIONS == 0
  what += '. Build with -sASSERTIONS for more info.';
#elif ASYNCIFY == 1
  if (what.indexOf('RuntimeError: unreachable') >= 0) {
    what += '. "unreachable" may be due to ASYNCIFY_STACK_SIZE not being large enough (try increasing it)';
  }
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
  // definition for WebAssembly.RuntimeError claims it takes no arguments even
  // though it can.
  // TODO(https://github.com/google/closure-compiler/pull/3913): Remove if/when upstream closure gets fixed.
#if WASM_EXCEPTIONS == 1
  // See above, in the meantime, we resort to wasm code for trapping.
  //
  // In case abort() is called before the module is initialized, wasmExports
  // and its exported '__trap' function is not available, in which case we throw
  // a RuntimeError.
  //
  // We trap instead of throwing RuntimeError to prevent infinite-looping in
  // Wasm EH code (because RuntimeError is considered as a foreign exception and
  // caught by 'catch_all'), but in case throwing RuntimeError is fine because
  // the module has not even been instantiated, even less running.
  if (runtimeInitialized) {
    ___trap();
  }
#endif
  /** @suppress {checkTypes} */
  var e = new WebAssembly.RuntimeError(what);

#if MODULARIZE
  readyPromiseReject(e);
#endif
  // Throw the error whether or not MODULARIZE is set because abort is used
  // in code paths apart from instantiation where an exception is expected
  // to be thrown when abort is called.
  throw e;
}

#include "memoryprofiler.js"

#if ASSERTIONS && !('$FS' in addedLibraryItems)
// show errors on likely calls to FS when it was not included
var FS = {
  error() {
    abort('Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with -sFORCE_FILESYSTEM');
  },
  init() { FS.error() },
  createDataFile() { FS.error() },
  createPreloadedFile() { FS.error() },
  createLazyFile() { FS.error() },
  open() { FS.error() },
  mkdev() { FS.error() },
  registerDevice() { FS.error() },
  analyzePath() { FS.error() },

  ErrnoError() { FS.error() },
};
Module['FS_createDataFile'] = FS.createDataFile;
Module['FS_createPreloadedFile'] = FS.createPreloadedFile;
#endif

#include "URIUtils.js"

#if ASSERTIONS
function createExportWrapper(name, nargs) {
  return (...args) => {
    assert(runtimeInitialized, `native function \`${name}\` called before runtime initialization`);
#if EXIT_RUNTIME
    assert(!runtimeExited, `native function \`${name}\` called after runtime exit (use NO_EXIT_RUNTIME to keep it alive after main() exits)`);
#endif
    var f = wasmExports[name];
    assert(f, `exported native function \`${name}\` not found`);
    // Only assert for too many arguments. Too few can be valid since the missing arguments will be zero filled.
    assert(args.length <= nargs, `native function \`${name}\` called with ${args.length} args but expects ${nargs}`);
    return f(...args);
  };
}
#endif

#include "runtime_exceptions.js"

#if ABORT_ON_WASM_EXCEPTIONS
// `abortWrapperDepth` counts the recursion level of the wrapper function so
// that we only handle exceptions at the top level letting the exception
// mechanics work uninterrupted at the inner level.  Additionally,
// `abortWrapperDepth` is also manually incremented in callMain so that we know
// to ignore exceptions from there since they're handled by callMain directly.
var abortWrapperDepth = 0;

function makeAbortWrapper(original) {
  return (...args) => {
    // Don't allow this function to be called if we're aborted!
    if (ABORT) {
      throw 'program has already aborted!';
    }

    abortWrapperDepth++;
    try {
      return original(...args);
    } catch (e) {
      if (
        ABORT // rethrow exception if abort() was called in the original function call above
        || abortWrapperDepth > 1 // rethrow exceptions not caught at the top level if exception catching is enabled; rethrow from exceptions from within callMain
#if SUPPORT_LONGJMP == 'emscripten' // Rethrow longjmp if enabled
#if EXCEPTION_STACK_TRACES
        || e instanceof EmscriptenSjLj // EXCEPTION_STACK_TRACES=1 will throw an instance of EmscriptenSjLj
#else
        || e === Infinity // EXCEPTION_STACK_TRACES=0 will throw Infinity
#endif // EXCEPTION_STACK_TRACES
#endif
        || e === 'unwind'
      ) {
        throw e;
      }

      abort('unhandled exception: ' + [e, e.stack]);
    }
    finally {
      abortWrapperDepth--;
    }
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
        func,
        wrapper: makeAbortWrapper(func)
      }
    }
    return cached.wrapper;
  };
}
#endif

function findWasmBinary() {
#if EXPORT_ES6 && USE_ES6_IMPORT_META && !SINGLE_FILE && !AUDIO_WORKLET
  if (Module['locateFile']) {
#endif
    var f = '{{{ WASM_BINARY_FILE }}}';
#if !SINGLE_FILE
    if (!isDataURI(f)) {
      return locateFile(f);
    }
#endif
    return f;
#if EXPORT_ES6 && USE_ES6_IMPORT_META && !SINGLE_FILE && !AUDIO_WORKLET // In single-file mode, repeating WASM_BINARY_FILE would emit the contents again. For an Audio Worklet, we cannot use `new URL()`.
  }
#if ENVIRONMENT_MAY_BE_SHELL
  if (ENVIRONMENT_IS_SHELL)
    return '{{{ WASM_BINARY_FILE }}}';
#endif
  // Use bundler-friendly `new URL(..., import.meta.url)` pattern; works in browsers too.
  return new URL('{{{ WASM_BINARY_FILE }}}', import.meta.url).href;
#endif
}

var wasmBinaryFile;

function getBinarySync(file) {
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
  }
#if WASM_ASYNC_COMPILATION
  throw 'both async and sync fetching of the wasm failed';
#else
  throw 'sync fetching of the wasm failed: you can preload it to Module["wasmBinary"] manually, or emcc.py will do that for you when generating HTML (but not JS)';
#endif
}

function getBinaryPromise(binaryFile) {
#if !SINGLE_FILE
  // If we don't have the binary yet, load it asynchronously using readAsync.
  if (!wasmBinary
#if SUPPORT_BASE64_EMBEDDING
      || isDataURI(binaryFile)
#endif
      ) {
    // Fetch the binary using readAsync
    return readAsync(binaryFile).then(
      (response) => new Uint8Array(/** @type{!ArrayBuffer} */(response)),
      // Fall back to getBinarySync if readAsync fails
      () => getBinarySync(binaryFile)
    );
  }
#endif

  // Otherwise, getBinarySync should be able to get it synchronously
  return Promise.resolve().then(() => getBinarySync(binaryFile));
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
  get(target, prop, receiver) {
    return (...args) => {
#if ASYNCIFY == 2
      throw new Error('Placeholder function "' + prop + '" should not be called when using JSPI.');
#else
      err(`placeholder function called: ${prop}`);
      var imports = {'primary': wasmExports};
      // Replace '.wasm' suffix with '.deferred.wasm'.
      var deferred = wasmBinaryFile.slice(0, -5) + '.deferred.wasm'
      loadSplitModule(deferred, imports, prop);
      err('instantiated deferred module, continuing');
#if RELOCATABLE
      // When the table is dynamically laid out, the placeholder functions names
      // are offsets from the table base. In the main module, the table base is
      // always 1.
      return wasmTable.get(1 + parseInt(prop))(...args);
#else
      return wasmTable.get(prop)(...args);
#endif
#endif
    }
  }
};
#endif

#if LOAD_SOURCE_MAP
function receiveSourceMapJSON(sourceMap) {
  wasmSourceMap = new WasmSourceMap(sourceMap);
  {{{ runIfMainThread("removeRunDependency('source-map');") }}}
}
#endif

#if SPLIT_MODULE || !WASM_ASYNC_COMPILATION
function instantiateSync(file, info) {
  var module;
  var binary = getBinarySync(file);
#if NODE_CODE_CACHING
  if (ENVIRONMENT_IS_NODE) {
    var v8 = require('v8');
    // Include the V8 version in the cache name, so that we don't try to
    // load cached code from another version, which fails silently (it seems
    // to load ok, but we do actually recompile the binary every time).
    var cachedCodeFile = '{{{ WASM_BINARY_FILE }}}.' + v8.cachedDataVersionTag() + '.cached';
    cachedCodeFile = locateFile(cachedCodeFile);
    var hasCached = fs.existsSync(cachedCodeFile);
    if (hasCached) {
#if RUNTIME_DEBUG
      dbg('NODE_CODE_CACHING: loading module');
#endif
      try {
        module = v8.deserialize(fs.readFileSync(cachedCodeFile));
      } catch (e) {
        err(`NODE_CODE_CACHING: failed to deserialize, bad cache file? (${cachedCodeFile})`);
        // Save the new compiled code when we have it.
        hasCached = false;
      }
    }
  }
  module ||= new WebAssembly.Module(binary);
  if (ENVIRONMENT_IS_NODE && !hasCached) {
#if RUNTIME_DEBUG
    dbg('NODE_CODE_CACHING: saving module');
#endif
    fs.writeFileSync(cachedCodeFile, v8.serialize(module));
  }
#else // NODE_CODE_CACHING
  module = new WebAssembly.Module(binary);
#endif // NODE_CODE_CACHING
  var instance = new WebAssembly.Instance(module, info);
#if USE_OFFSET_CONVERTER
  wasmOffsetConverter = new WasmOffsetConverter(binary, module);
#endif
#if LOAD_SOURCE_MAP
  receiveSourceMapJSON(getSourceMap());
#endif
  return [instance, module];
}
#endif

#if (PTHREADS || WASM_WORKERS) && (LOAD_SOURCE_MAP || USE_OFFSET_CONVERTER)
// When using postMessage to send an object, it is processed by the structured
// clone algorithm.  The prototype, and hence methods, on that object is then
// lost. This function adds back the lost prototype.  This does not work with
// nested objects that has prototypes, but it suffices for WasmSourceMap and
// WasmOffsetConverter.
function resetPrototype(constructor, attrs) {
  var object = Object.create(constructor.prototype);
  return Object.assign(object, attrs);
}
#endif

#if WASM_ASYNC_COMPILATION
function instantiateArrayBuffer(binaryFile, imports, receiver) {
#if USE_OFFSET_CONVERTER
  var savedBinary;
#endif
  return getBinaryPromise(binaryFile).then((binary) => {
#if USE_OFFSET_CONVERTER
    savedBinary = binary;
#endif
    return WebAssembly.instantiate(binary, imports);
#if USE_OFFSET_CONVERTER
  }).then((instance) => {
    // wasmOffsetConverter needs to be assigned before calling the receiver
    // (receiveInstantiationResult).  See comments below in instantiateAsync.
    wasmOffsetConverter = new WasmOffsetConverter(savedBinary, instance.module);
    return instance;
#endif
  }).then(receiver, (reason) => {
    err(`failed to asynchronously prepare wasm: ${reason}`);

#if WASM == 2
#if ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
    if (typeof location != 'undefined') {
#endif
      // WebAssembly compilation failed, try running the JS fallback instead.
      var search = location.search;
      if (search.indexOf('_rwasm=0') < 0) {
        location.href += (search ? search + '&' : '?') + '_rwasm=0';
        // Return here to avoid calling abort() below.  The application
        // still has a chance to start successfully do we don't want to
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
      err(`warning: Loading from a file URI (${wasmBinaryFile}) is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing`);
    }
#endif
    abort(reason);
  });
}

function instantiateAsync(binary, binaryFile, imports, callback) {
#if !SINGLE_FILE
  if (!binary &&
      typeof WebAssembly.instantiateStreaming == 'function' &&
      !isDataURI(binaryFile) &&
#if ENVIRONMENT_MAY_BE_WEBVIEW
      // Don't use streaming for file:// delivered objects in a webview, fetch them synchronously.
      !isFileURI(binaryFile) &&
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
    return fetch(binaryFile, {{{ makeModuleReceiveExpr('fetchSettings', "{ credentials: 'same-origin' }") }}}).then((response) => {
      // Suppress closure warning here since the upstream definition for
      // instantiateStreaming only allows Promise<Repsponse> rather than
      // an actual Response.
      // TODO(https://github.com/google/closure-compiler/pull/3913): Remove if/when upstream closure is fixed.
      /** @suppress {checkTypes} */
      var result = WebAssembly.instantiateStreaming(response, imports);

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
        callback,
#else
        function(instantiationResult) {
          // When using the offset converter, we must interpose here. First,
          // the instantiation result must arrive (if it fails, the error
          // handling later down will handle it). Once it arrives, we can
          // initialize the offset converter. And only then is it valid to
          // call receiveInstantiationResult, as that function will use the
          // offset converter (in the case of pthreads, it will create the
          // pthreads and send them the offsets along with the wasm instance).

          clonedResponsePromise.then((arrayBufferResult) => {
              wasmOffsetConverter = new WasmOffsetConverter(new Uint8Array(arrayBufferResult), instantiationResult.module);
              callback(instantiationResult);
            },
            (reason) => err(`failed to initialize offset-converter: ${reason}`)
          );
        },
#endif
        function(reason) {
          // We expect the most common failure cause to be a bad MIME type for the binary,
          // in which case falling back to ArrayBuffer instantiation should work.
          err(`wasm streaming compile failed: ${reason}`);
          err('falling back to ArrayBuffer instantiation');
          return instantiateArrayBuffer(binaryFile, imports, callback);
        });
    });
  }
#endif
  return instantiateArrayBuffer(binaryFile, imports, callback);
}
#endif // WASM_ASYNC_COMPILATION

function getWasmImports() {
#if PTHREADS
  assignWasmImports();
#endif
#if ASYNCIFY && (ASSERTIONS || ASYNCIFY == 2)
  // instrumenting imports is used in asyncify in two ways: to add assertions
  // that check for proper import use, and for ASYNCIFY=2 we use them to set up
  // the Promise API on the import side.
#if PTHREADS || ASYNCIFY_LAZY_LOAD_CODE
  // In pthreads builds getWasmImports is called more than once but we only
  // and the instrument the imports once.
  if (!wasmImports.__instrumented) {
    wasmImports.__instrumented = true;
    Asyncify.instrumentWasmImports(wasmImports);
  }
#else
  Asyncify.instrumentWasmImports(wasmImports);
#endif
#endif
  // prepare imports
  return {
#if MINIFY_WASM_IMPORTED_MODULES
    'a': wasmImports,
#else // MINIFY_WASM_IMPORTED_MODULES
    'env': wasmImports,
    '{{{ WASI_MODULE_NAME }}}': wasmImports,
#endif // MINIFY_WASM_IMPORTED_MODULES
#if SPLIT_MODULE
    'placeholder': new Proxy({}, splitModuleProxyHandler),
#endif
#if RELOCATABLE
    'GOT.mem': new Proxy(wasmImports, GOTHandler),
    'GOT.func': new Proxy(wasmImports, GOTHandler),
#endif
  }
}

// Create the wasm instance.
// Receives the wasm imports, returns the exports.
function createWasm() {
  var info = getWasmImports();
  // Load the wasm module and create an instance of using native support in the JS engine.
  // handle a generated wasm instance, receiving its exports and
  // performing other necessary setup
  /** @param {WebAssembly.Module=} module*/
  function receiveInstance(instance, module) {
    wasmExports = instance.exports;

#if RELOCATABLE
    wasmExports = relocateExports(wasmExports, {{{ GLOBAL_BASE }}});
#endif

#if ASYNCIFY
    wasmExports = Asyncify.instrumentWasmExports(wasmExports);
#endif

#if ABORT_ON_WASM_EXCEPTIONS
    wasmExports = instrumentWasmExportsWithAbort(wasmExports);
#endif

#if MAIN_MODULE
    var metadata = getDylinkMetadata(module);
#if AUTOLOAD_DYLIBS
    if (metadata.neededDynlibs) {
      dynamicLibraries = metadata.neededDynlibs.concat(dynamicLibraries);
    }
#endif
    mergeLibSymbols(wasmExports, 'main')
#if '$LDSO' in addedLibraryItems
    LDSO.init();
#endif
    loadDylibs();
#elif RELOCATABLE
    reportUndefinedSymbols();
#endif

#if MEMORY64 || CAN_ADDRESS_2GB
    wasmExports = applySignatureConversions(wasmExports);
#endif

    {{{ receivedSymbol('wasmExports') }}}

#if PTHREADS
#if MAIN_MODULE
    registerTLSInit(wasmExports['_emscripten_tls_init'], instance.exports, metadata);
#else
    registerTLSInit(wasmExports['_emscripten_tls_init']);
#endif
#endif

#if !IMPORTED_MEMORY
    wasmMemory = wasmExports['memory'];
    {{{ receivedSymbol('wasmMemory') }}}
#if ASSERTIONS
    assert(wasmMemory, 'memory not found in wasm exports');
#endif
    updateMemoryViews();
#endif

#if '$wasmTable' in addedLibraryItems && !RELOCATABLE
    wasmTable = wasmExports['__indirect_function_table'];
    {{{ receivedSymbol('wasmTable') }}}
#if ASSERTIONS && !PURE_WASI
    assert(wasmTable, 'table not found in wasm exports');
#endif
#endif

#if hasExportedSymbol('__wasm_call_ctors')
    addOnInit(wasmExports['__wasm_call_ctors']);
#endif

#if hasExportedSymbol('__wasm_apply_data_relocs')
    __RELOC_FUNCS__.push(wasmExports['__wasm_apply_data_relocs']);
#endif

#if ABORT_ON_WASM_EXCEPTIONS
    instrumentWasmTableWithAbort();
#endif

#if !DECLARE_ASM_MODULE_EXPORTS
    // If we didn't declare the asm exports as top level enties this function
    // is in charge of programmatically exporting them on the global object.
    exportWasmSymbols(wasmExports);
#endif

#if PTHREADS || WASM_WORKERS
    // We now have the Wasm module loaded up, keep a reference to the compiled module so we can post it to the workers.
    wasmModule = module;
#endif
    removeRunDependency('wasm-instantiate');
    return wasmExports;
  }
  // wait for the pthread pool (if any)
  addRunDependency('wasm-instantiate');

#if LOAD_SOURCE_MAP
  {{{ runIfMainThread("addRunDependency('source-map');") }}}
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
    // When the regression is fixed, can restore the above PTHREADS-enabled path.
    receiveInstance(result['instance']);
#endif
  }
#endif // WASM_ASYNC_COMPILATION

#if expectToReceiveOnModule('instantiateWasm')
  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to
  // run the instantiation parallel to any other async startup actions they are
  // performing.
  // Also pthreads and wasm workers initialize the wasm instance through this
  // path.
  if (Module['instantiateWasm']) {
    try {
      return Module['instantiateWasm'](info, receiveInstance);
    } catch(e) {
      err(`Module.instantiateWasm callback failed with error: ${e}`);
      #if MODULARIZE
        // If instantiation fails, reject the module ready promise.
        readyPromiseReject(e);
      #else
        return false;
      #endif
    }
  }
#endif

  wasmBinaryFile ??= findWasmBinary();

#if WASM_ASYNC_COMPILATION
#if RUNTIME_DEBUG
  dbg('asynchronously preparing wasm');
#endif
#if MODULARIZE
  // If instantiation fails, reject the module ready promise.
  instantiateAsync(wasmBinary, wasmBinaryFile, info, receiveInstantiationResult).catch(readyPromiseReject);
#else
  instantiateAsync(wasmBinary, wasmBinaryFile, info, receiveInstantiationResult);
#endif
#if LOAD_SOURCE_MAP
  getSourceMapPromise().then(receiveSourceMapJSON);
#endif
  return {}; // no exports yet; we'll fill them in later
#else
  var result = instantiateSync(wasmBinaryFile, info);
#if PTHREADS || MAIN_MODULE
  return receiveInstance(result[0], result[1]);
#else
  // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193,
  // the above line no longer optimizes out down to the following line.
  // When the regression is fixed, we can remove this if/else.
  return receiveInstance(result[0]);
#endif
#endif
}

#if !WASM_BIGINT
// Globals used by JS i64 conversions (see makeSetValue)
var tempDouble;
var tempI64;
#endif

#include "runtime_debug.js"

#if RETAIN_COMPILER_SETTINGS
var compilerSettings = {{{ JSON.stringify(makeRetainedCompilerSettings()) }}} ;

function getCompilerSetting(name) {
  if (!(name in compilerSettings)) return 'invalid compiler setting: ' + name;
  return compilerSettings[name];
}
#endif // RETAIN_COMPILER_SETTINGS

#if MAIN_MODULE && ASYNCIFY
// With MAIN_MODULE + ASYNCIFY the normal method of placing stub functions in
// wasmImports for as-yet-undefined symbols doesn't work since ASYNCIFY then
// wraps these stub functions and we can't then replace them directly.  Instead
// the stub functions call into `asyncifyStubs` which gets populated by the
// dynamic linker as symbols are loaded.
var asyncifyStubs = {};
#endif

// === Body ===
