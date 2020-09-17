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
out = err = function(){};
#endif

{{{ makeModuleReceiveWithVar('wasmBinary') }}}
{{{ makeModuleReceiveWithVar('noExitRuntime') }}}

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
if (typeof WebAssembly !== 'object') {
  abort('no native wasm support detected');
}
#endif

#include "runtime_safe_heap.js"

// Wasm globals

var wasmMemory;

// In fastcomp asm.js, we don't need a wasm Table at all.
// In the wasm backend, we polyfill the WebAssembly object,
// so this creates a (non-native-wasm) table for us.
#include "runtime_init_table.js"

#if USE_PTHREADS
// For sending to workers.
var wasmModule;
// Only workers actually use these field, but we refer to them from
// library_pthread (which exists on all threads) so this definition is useful
// to avoid accessing the global scope.
var threadInfoStruct = 0;
var selfThreadId = 0;
#endif // USE_PTHREADS

//========================================
// Runtime essentials
//========================================

// whether we are quitting the application. no code should run after this.
// set in exit() and abort()
var ABORT = false;

// set by exit() and abort().  Passed to 'onExit' handler.
// NOTE: This is also used as the process return code code in shell environments
// but only when noExitRuntime is false.
var EXITSTATUS = 0;

/** @type {function(*, string=)} */
function assert(condition, text) {
  if (!condition) {
    abort('Assertion failed: ' + text);
  }
}

// Returns the C function with a specified identifier (for C++, you need to do manual name mangling)
function getCFunc(ident) {
  var func = Module['_' + ident]; // closure exported function
  assert(func, 'Cannot call unknown function ' + ident + ', make sure it is exported');
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
    'string': function(str) {
      var ret = 0;
      if (str !== null && str !== undefined && str !== 0) { // null string
        // at most 4 bytes per UTF-8 code point, +1 for the trailing '\0'
        var len = (str.length << 2) + 1;
        ret = stackAlloc(len);
        stringToUTF8(str, ret, len);
      }
      return ret;
    },
    'array': function(arr) {
      var ret = stackAlloc(arr.length);
      writeArrayToMemory(arr, ret);
      return ret;
    }
  };

  function convertReturnValue(ret) {
    if (returnType === 'string') return UTF8ToString(ret);
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
  var ret = func.apply(null, cArgs);
#if ASYNCIFY
  var asyncMode = opts && opts.async;
  var runningAsync = typeof Asyncify === 'object' && Asyncify.currData;
  var prevRunningAsync = typeof Asyncify === 'object' && Asyncify.asyncFinalizers.length > 0;
#if ASSERTIONS
  assert(!asyncMode || !prevRunningAsync, 'Cannot have multiple async ccalls in flight at once');
#endif
  // Check if we started an async operation just now.
  if (runningAsync && !prevRunningAsync) {
    // If so, the WASM function ran asynchronous and unwound its stack.
    // We need to return a Promise that resolves the return value
    // once the stack is rewound and execution finishes.
#if ASSERTIONS
    assert(asyncMode, 'The call to ' + ident + ' is running asynchronously. If this was intended, add the async option to the ccall/cwrap call.');
#endif
    return new Promise(function(resolve) {
      Asyncify.asyncFinalizers.push(function(ret) {
        if (stack !== 0) stackRestore(stack);
        resolve(convertReturnValue(ret));
      });
    });
  }
#endif

  ret = convertReturnValue(ret);
  if (stack !== 0) stackRestore(stack);
#if ASYNCIFY
  // If this is an async ccall, ensure we return a promise
  if (opts && opts.async) return Promise.resolve(ret);
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
#if !('_malloc' in IMPLEMENTED_FUNCTIONS)
function _malloc() {
  abort("malloc() called but not included in the build - add '_malloc' to EXPORTED_FUNCTIONS");
}
#endif // malloc
#if !('_free' in IMPLEMENTED_FUNCTIONS)
function _free() {
  // Show a helpful error since we used to include free by default in the past.
  abort("free() called but not included in the build - add '_free' to EXPORTED_FUNCTIONS");
}
#endif // free
#endif // ASSERTIONS

var ALLOC_NORMAL = 0; // Tries to use _malloc()
var ALLOC_STACK = 1; // Lives for the duration of the current function call
var ALLOC_NONE = 2; // Do not allocate

// allocate(): This is for internal use. You can use it yourself as well, but the interface
//             is a little tricky (see docs right below). The reason is that it is optimized
//             for multiple syntaxes to save space in generated code. So you should
//             normally not use allocate(), and instead allocate memory using _malloc(),
//             initialize it with setValue(), and so forth.
// @slab: An array of data, or a number. If a number, then the size of the block to allocate,
//        in *bytes* (note that this is sometimes confusing: the next parameter does not
//        affect this!)
// @types: Either an array of types, one for each byte (or 0 if no type at that position),
//         or a single type which is used for the entire block. This only matters if there
//         is initial data - if @slab is a number, then this does not matter at all and is
//         ignored.
// @allocator: How to allocate memory, see ALLOC_*
/** @type {function((TypedArray|Array<number>|number), string, number, number=)} */
function allocate(slab, types, allocator, ptr) {
  var zeroinit, size;
  if (typeof slab === 'number') {
    zeroinit = true;
    size = slab;
  } else {
    zeroinit = false;
    size = slab.length;
  }

  var singleType = typeof types === 'string' ? types : null;

  var ret;
  if (allocator == ALLOC_NONE) {
    ret = ptr;
  } else {
    ret = [{{{ ('_malloc' in IMPLEMENTED_FUNCTIONS) ? '_malloc' : 'null' }}},
#if DECLARE_ASM_MODULE_EXPORTS
    stackAlloc,
#else
    typeof stackAlloc !== 'undefined' ? stackAlloc : null,
#endif
    ][allocator](Math.max(size, singleType ? 1 : types.length));
  }

  if (zeroinit) {
    var stop;
    ptr = ret;
    assert((ret & 3) == 0);
    stop = ret + (size & ~3);
    for (; ptr < stop; ptr += 4) {
      {{{ makeSetValue('ptr', '0', '0', 'i32', null, true) }}};
    }
    stop = ret + size;
    while (ptr < stop) {
      {{{ makeSetValue('ptr++', '0', '0', 'i8', null, true) }}};
    }
    return ret;
  }

  if (singleType === 'i8') {
    if (slab.subarray || slab.slice) {
      HEAPU8.set(/** @type {!Uint8Array} */ (slab), ret);
    } else {
      HEAPU8.set(new Uint8Array(slab), ret);
    }
    return ret;
  }

  var i = 0, type, typeSize, previousType;
  while (i < size) {
    var curr = slab[i];

    type = singleType || types[i];
    if (type === 0) {
      i++;
      continue;
    }
#if ASSERTIONS
    assert(type, 'Must know what type to store in allocate!');
#endif

    if (type == 'i64') type = 'i32'; // special case: we have one i32 here, and one i32 later

    setValue(ret+i, curr, type);

    // no need to look up size unless type changes, so cache it
    if (previousType !== type) {
      typeSize = getNativeTypeSize(type);
      previousType = type;
    }
    i += typeSize;
  }

  return ret;
}

#include "runtime_strings.js"
#include "runtime_strings_extra.js"

// Memory management

var PAGE_SIZE = {{{ POSIX_PAGE_SIZE }}};
var WASM_PAGE_SIZE = {{{ WASM_PAGE_SIZE }}};

function alignUp(x, multiple) {
  if (x % multiple > 0) {
    x += multiple - (x % multiple);
  }
  return x;
}

var HEAP,
/** @type {ArrayBuffer} */
  buffer,
/** @type {Int8Array} */
  HEAP8,
/** @type {Uint8Array} */
  HEAPU8,
/** @type {Int16Array} */
  HEAP16,
/** @type {Uint16Array} */
  HEAPU16,
/** @type {Int32Array} */
  HEAP32,
/** @type {Uint32Array} */
  HEAPU32,
/** @type {Float32Array} */
  HEAPF32,
/** @type {Float64Array} */
  HEAPF64;

function updateGlobalBufferAndViews(buf) {
  buffer = buf;
  Module['HEAP8'] = HEAP8 = new Int8Array(buf);
  Module['HEAP16'] = HEAP16 = new Int16Array(buf);
  Module['HEAP32'] = HEAP32 = new Int32Array(buf);
  Module['HEAPU8'] = HEAPU8 = new Uint8Array(buf);
  Module['HEAPU16'] = HEAPU16 = new Uint16Array(buf);
  Module['HEAPU32'] = HEAPU32 = new Uint32Array(buf);
  Module['HEAPF32'] = HEAPF32 = new Float32Array(buf);
  Module['HEAPF64'] = HEAPF64 = new Float64Array(buf);
}

var STACK_BASE = {{{ getQuoted('STACK_BASE') }}},
    STACKTOP = STACK_BASE,
    STACK_MAX = {{{ getQuoted('STACK_MAX') }}};

#if ASSERTIONS
assert(STACK_BASE % 16 === 0, 'stack must start aligned');
#endif

#if RELOCATABLE
// To support such allocations during startup, track them on __heap_base and
// then when the main module is loaded it reads that value and uses it to
// initialize sbrk (the main module is relocatable itself, and so it does not
// have __heap_base hardcoded into it - it receives it from JS as an extern
// global, basically).
Module['___heap_base'] = {{{ getQuoted('HEAP_BASE') }}};
#endif // RELOCATABLE

#if USE_PTHREADS
if (ENVIRONMENT_IS_PTHREAD) {
  // At the 'load' stage of Worker startup, we are just loading this script
  // but not ready to run yet. At 'run' we receive proper values for the stack
  // etc. and can launch a pthread. Set some fake values there meanwhile to
  // catch bugs, then set the real values in establishStackSpace later.
#if ASSERTIONS || STACK_OVERFLOW_CHECK >= 2
  STACK_MAX = STACKTOP = STACK_MAX = 0x7FFFFFFF;
#endif
}
#endif

var TOTAL_STACK = {{{ TOTAL_STACK }}};
#if ASSERTIONS
if (Module['TOTAL_STACK']) assert(TOTAL_STACK === Module['TOTAL_STACK'], 'the stack size can no longer be determined at runtime')
#endif

{{{ makeModuleReceiveWithVar('INITIAL_INITIAL_MEMORY', 'INITIAL_MEMORY', INITIAL_MEMORY) }}}

#if ASSERTIONS
assert(INITIAL_INITIAL_MEMORY >= TOTAL_STACK, 'INITIAL_MEMORY should be larger than TOTAL_STACK, was ' + INITIAL_INITIAL_MEMORY + '! (TOTAL_STACK=' + TOTAL_STACK + ')');

// check for full engine support (use string 'subarray' to avoid closure compiler confusion)
assert(typeof Int32Array !== 'undefined' && typeof Float64Array !== 'undefined' && Int32Array.prototype.subarray !== undefined && Int32Array.prototype.set !== undefined,
       'JS engine does not provide full typed array support');
#endif

#if IN_TEST_HARNESS

// Test runs in browsers should always be free from uncaught exceptions. If an uncaught exception is thrown, we fail browser test execution in the REPORT_RESULT() macro to output an error value.
if (ENVIRONMENT_IS_WEB) {
  window.addEventListener('error', function(e) {
    if (e.message.indexOf('unwind') != -1) return;
    console.error('Page threw an exception ' + e);
    Module['pageThrewException'] = true;
  });
}

#if USE_PTHREADS
if (typeof SharedArrayBuffer === 'undefined' || typeof Atomics === 'undefined') {
  var xhr = new XMLHttpRequest();
  xhr.open('GET', 'http://localhost:8888/report_result?skipped:%20SharedArrayBuffer%20is%20not%20supported!');
  xhr.send();
  setTimeout(function() { window.close() }, 2000);
}
#endif
#endif

#if STANDALONE_WASM
#if ASSERTIONS
// In standalone mode, the wasm creates the memory, and the user can't provide it.
assert(!Module['wasmMemory']);
#endif // ASSERTIONS
#else // !STANDALONE_WASM
// In non-standalone/normal mode, we create the memory here.
#include "runtime_init_memory.js"
#endif // !STANDALONE_WASM

#include "runtime_stack_check.js"
#include "runtime_assertions.js"

var __ATPRERUN__  = []; // functions called before the runtime is initialized
var __ATINIT__    = []; // functions called during startup
var __ATMAIN__    = []; // functions called when main() is to be run
var __ATEXIT__    = []; // functions called during shutdown
var __ATPOSTRUN__ = []; // functions called after the main() is called

var runtimeInitialized = false;
var runtimeExited = false;

#if USE_PTHREADS
if (ENVIRONMENT_IS_PTHREAD) runtimeInitialized = true; // The runtime is hosted in the main thread, and bits shared to pthreads via SharedArrayBuffer. No need to init again in pthread.
#endif

function preRun() {
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
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
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if ASSERTIONS
  assert(!runtimeInitialized);
#endif
  runtimeInitialized = true;
#if STACK_OVERFLOW_CHECK >= 2
  Module['___set_stack_limits'](STACK_BASE, STACK_MAX);
#endif
  {{{ getQuoted('ATINITS') }}}
  callRuntimeCallbacks(__ATINIT__);
}

function preMain() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
  {{{ getQuoted('ATMAINS') }}}
  callRuntimeCallbacks(__ATMAIN__);
}

function exitRuntime() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return; // PThreads reuse the runtime from the main thread.
#endif
#if EXIT_RUNTIME
  callRuntimeCallbacks(__ATEXIT__);
  {{{ getQuoted('ATEXITS') }}}
#if USE_PTHREADS
  PThread.runExitHandlers();
#endif
#endif
  runtimeExited = true;
}

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

function addOnPreMain(cb) {
  __ATMAIN__.unshift(cb);
}

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
#if USE_PTHREADS
  // We should never get here in pthreads (could no-op this out if called in pthreads, but that might indicate a bug in caller side,
  // so good to be very explicit)
  assert(!ENVIRONMENT_IS_PTHREAD, "addRunDependency cannot be used in a pthread worker");
#endif
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
    if (runDependencyWatcher === null && typeof setInterval !== 'undefined') {
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

Module["preloadedImages"] = {}; // maps url to image data
Module["preloadedAudios"] = {}; // maps url to audio data
#if MAIN_MODULE
Module["preloadedWasm"] = {}; // maps url to wasm instance exports
addOnPreRun(preloadDylibs);
#endif

/** @param {string|number=} what */
function abort(what) {
#if expectToReceiveOnModule('onAbort')
  if (Module['onAbort']) {
    Module['onAbort'](what);
  }
#endif

#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) console.error('Pthread aborting at ' + new Error().stack);
#endif
  what += '';
  err(what);

  ABORT = true;
  EXITSTATUS = 1;

#if ASSERTIONS == 0
  what = 'abort(' + what + '). Build with -s ASSERTIONS=1 for more info.';
#else
  var output = 'abort(' + what + ') at ' + stackTrace();
  what = output;
#endif // ASSERTIONS

  // Use a wasm runtime error, because a JS error might be seen as a foreign
  // exception, which means we'd run destructors on it. We need the error to
  // simply make the program stop.
  var e = new WebAssembly.RuntimeError(what);

#if MODULARIZE
  readyPromiseReject(e);
#endif
  // Throw the error whether or not MODULARIZE is set because abort is used
  // in code paths apart from instantiation where an exception is expected
  // to be thrown when abort is called.
  throw e;
}

var memoryInitializer = null;

#include "memoryprofiler.js"

#if ASSERTIONS && !('$FS' in addedLibraryItems) && !ASMFS
// show errors on likely calls to FS when it was not included
var FS = {
  error: function() {
    abort('Filesystem support (FS) was not included. The problem is that you are using files from JS, but files were not used from C/C++, so filesystem support was not auto-included. You can force-include filesystem support with  -s FORCE_FILESYSTEM=1');
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
function createExportWrapper(name, fixedasm) {
  return function() {
    var displayName = name;
    var asm = fixedasm;
    if (!fixedasm) {
      asm = Module['asm'];
    }
    assert(runtimeInitialized, 'native function `' + displayName + '` called before runtime initialization');
    assert(!runtimeExited, 'native function `' + displayName + '` called after runtime exit (use NO_EXIT_RUNTIME to keep it alive after main() exits)');
    if (!asm[name]) {
      assert(asm[name], 'exported native function `' + displayName + '` not found');
    }
    return asm[name].apply(null, arguments);
  };
}
#endif

#if ABORT_ON_WASM_EXCEPTIONS
// When DISABLE_EXCEPTION_CATCHING != 1 `abortWrapperDepth` counts the recursion 
// level of the wrapper function so that we only handle exceptions at the top level
// letting the exception mechanics work uninterrupted at the inner level.
// Additionally, `abortWrapperDepth` is also manually incremented in callMain so that 
// we know to ignore exceptions from there since they're handled by callMain directly.
var abortWrapperDepth = 0;

// Instrument all the exported functions to:
// - abort if an unhandled exception occurs
// - throw an exception if someone tries to call them after the program has aborted
// See settings.ABORT_ON_WASM_EXCEPTIONS for more info.
function instrumentWasmExportsWithAbort(exports) {
  var instExports = {};
  for (var name in exports) {
    (function(name) {
      var original = exports[name];
      
      // Wrap all functions, copy the other symbols.
      if (typeof original === 'function') {
        instExports[name] = function() {
          // Don't allow this function to be called if we're aborted!
          if (ABORT) { 
            throw "program has already aborted!";
          }
          
#if DISABLE_EXCEPTION_CATCHING != 1
          abortWrapperDepth += 1;
#endif
          try {
            return original.apply(null, arguments);
          }
          catch (e) {
            if (
              ABORT // rethrow exception if abort() was called in the original function call above
              || abortWrapperDepth > 1 // rethrow exceptions not caught at the top level if exception catching is enabled; rethrow from exceptions from within callMain
#if SUPPORT_LONGJMP
              || e === 'longjmp' // rethrow longjmp if enabled
#endif
            ) {
              throw e;
            }
            
            abort("unhandled exception: " + [e, e.stack]);
          }
#if DISABLE_EXCEPTION_CATCHING != 1
          finally {
            abortWrapperDepth -= 1;
          }
#endif
        };
      }
      else {
        instExports[name] = original;
      }
    })(name);
  }
  return instExports;
}
#endif

var wasmBinaryFile = '{{{ WASM_BINARY_FILE }}}';
if (!isDataURI(wasmBinaryFile)) {
  wasmBinaryFile = locateFile(wasmBinaryFile);
}

function getBinary() {
  try {
    if (wasmBinary) {
      return new Uint8Array(wasmBinary);
    }

#if SUPPORT_BASE64_EMBEDDING
    var binary = tryParseAsDataURI(wasmBinaryFile);
    if (binary) {
      return binary;
    }
#endif
    if (readBinary) {
      return readBinary(wasmBinaryFile);
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
  // If we don't have the binary yet, and have the Fetch api, use that;
  // in some environments, like Electron's render process, Fetch api may be present, but have a different context than expected, let's only use it on the Web
  if (!wasmBinary && (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) && typeof fetch === 'function'
#if ENVIRONMENT_MAY_BE_WEBVIEW
      // Let's not use fetch to get objects over file:// as it's most likely Cordova which doesn't support fetch for file://
      && !isFileURI(wasmBinaryFile)
#endif
      ) {
    return fetch(wasmBinaryFile, { credentials: 'same-origin' }).then(function(response) {
      if (!response['ok']) {
        throw "failed to load wasm binary file at '" + wasmBinaryFile + "'";
      }
      return response['arrayBuffer']();
    }).catch(function () {
      return getBinary();
    });
  }
  // Otherwise, getBinary should be able to get it synchronously
  return Promise.resolve().then(getBinary);
}

#if LOAD_SOURCE_MAP
var wasmSourceMap;
#include "source_map_support.js"
#endif

#if USE_OFFSET_CONVERTER
var wasmOffsetConverter;
#include "wasm_offset_converter.js"
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
    '{{{ WASI_MODULE_NAME }}}': asmLibraryArg
#endif // MINIFY_WASM_IMPORTED_MODULES
  };
  // Load the wasm module and create an instance of using native support in the JS engine.
  // handle a generated wasm instance, receiving its exports and
  // performing other necessary setup
  /** @param {WebAssembly.Module=} module*/
  function receiveInstance(instance, module) {
    var exports = instance.exports;
#if RELOCATABLE
    exports = relocateExports(exports, GLOBAL_BASE, 0);
#endif
#if ASYNCIFY
    exports = Asyncify.instrumentWasmExports(exports);
#endif
#if ABORT_ON_WASM_EXCEPTIONS
    exports = instrumentWasmExportsWithAbort(exports);
#endif
    Module['asm'] = exports;
#if !DECLARE_ASM_MODULE_EXPORTS
    // If we didn't declare the asm exports as top level enties this function
    // is in charge of programatically exporting them on the global object.
    exportAsmFunctions(exports);
#endif
#if STANDALONE_WASM
    // In pure wasm mode the memory is created in the wasm (not imported), and
    // then exported.
    // TODO: do not create a Memory earlier in JS
    wasmMemory = exports['memory'];
    wasmTable = exports['__indirect_function_table'];
    updateGlobalBufferAndViews(wasmMemory.buffer);
#if ASSERTIONS
    writeStackCookie();
#endif
#endif
#if USE_PTHREADS
    // We now have the Wasm module loaded up, keep a reference to the compiled module so we can post it to the workers.
    wasmModule = module;
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
#endif
  }
  // we can't run yet (except in a pthread, where we have a custom sync instantiator)
  {{{ runOnMainThread("addRunDependency('wasm-instantiate');") }}}

#if LOAD_SOURCE_MAP
  {{{ runOnMainThread("addRunDependency('source-map');") }}}

  function receiveSourceMapJSON(sourceMap) {
    wasmSourceMap = new WasmSourceMap(sourceMap);
    {{{ runOnMainThread("removeRunDependency('source-map');") }}}
  }
#endif

#if ASSERTIONS
  // Async compilation can be confusing when an error on the page overwrites Module
  // (for example, if the order of elements is wrong, and the one defining Module is
  // later), so we save Module and check it later.
  var trueModule = Module;
#endif
  function receiveInstantiatedSource(output) {
    // 'output' is a WebAssemblyInstantiatedSource object which has both the module and instance.
    // receiveInstance() will swap in the exports (to Module.asm) so they can be called
#if ASSERTIONS
    assert(Module === trueModule, 'the Module object should not be replaced during async compilation - perhaps the order of HTML elements is wrong?');
    trueModule = null;
#endif
#if USE_PTHREADS
    receiveInstance(output['instance'], output['module']);
#else
    // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193, the above line no longer optimizes out down to the following line.
    // When the regression is fixed, can restore the above USE_PTHREADS-enabled path.
    receiveInstance(output['instance']);
#endif
  }

#if USE_OFFSET_CONVERTER
  {{{ runOnMainThread("addRunDependency('offset-converter');") }}}
#endif

  function instantiateArrayBuffer(receiver) {
    return getBinaryPromise().then(function(binary) {
#if USE_OFFSET_CONVERTER
      var result = WebAssembly.instantiate(binary, info);
      result.then(function (instance) {
        wasmOffsetConverter = new WasmOffsetConverter(binary, instance.module);
        {{{ runOnMainThread("removeRunDependency('offset-converter');") }}}
      });
      return result;
#else // USE_OFFSET_CONVERTER
      return WebAssembly.instantiate(binary, info);
#endif // USE_OFFSET_CONVERTER
    }).then(receiver, function(reason) {
      err('failed to asynchronously prepare wasm: ' + reason);

#if WASM == 2
#if ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
      if (typeof location !== 'undefined') {
#endif
        // WebAssembly compilation failed, try running the JS fallback instead.
        var search = location.search;
        if (search.indexOf('_rwasm=0') < 0) {
          location.href += (search ? search + '&' : '?') + '_rwasm=0';
        }
#if ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
      }
#endif
#endif // WASM == 2

      abort(reason);
    });
  }

  // Prefer streaming instantiation if available.
#if WASM_ASYNC_COMPILATION
  function instantiateAsync() {
    if (!wasmBinary &&
        typeof WebAssembly.instantiateStreaming === 'function' &&
        !isDataURI(wasmBinaryFile) &&
#if ENVIRONMENT_MAY_BE_WEBVIEW
        // Don't use streaming for file:// delivered objects in a webview, fetch them synchronously.
        !isFileURI(wasmBinaryFile) &&
#endif
        typeof fetch === 'function') {
      fetch(wasmBinaryFile, { credentials: 'same-origin' }).then(function (response) {
        var result = WebAssembly.instantiateStreaming(response, info);
#if USE_OFFSET_CONVERTER
        // This doesn't actually do another request, it only copies the Response object.
        // Copying lets us consume it independently of WebAssembly.instantiateStreaming.
        Promise.all([response.clone().arrayBuffer(), result]).then(function (results) {
          wasmOffsetConverter = new WasmOffsetConverter(new Uint8Array(results[0]), results[1].module);
          {{{ runOnMainThread("removeRunDependency('offset-converter');") }}}
        });
#endif
        return result.then(receiveInstantiatedSource, function(reason) {
            // We expect the most common failure cause to be a bad MIME type for the binary,
            // in which case falling back to ArrayBuffer instantiation should work.
            err('wasm streaming compile failed: ' + reason);
            err('falling back to ArrayBuffer instantiation');
            return instantiateArrayBuffer(receiveInstantiatedSource);
          });
      });
    } else {
      return instantiateArrayBuffer(receiveInstantiatedSource);
    }
  }
#else
  function instantiateSync() {
    var instance;
    var module;
    var binary;
    try {
      binary = getBinary();
#if NODE_CODE_CACHING
      if (ENVIRONMENT_IS_NODE) {
        var v8 = require('v8');
        // Include the V8 version in the cache name, so that we don't try to
        // load cached code from another version, which fails silently (it seems
        // to load ok, but we do actually recompile the binary every time).
        var cachedCodeFile = '{{{ WASM_BINARY_FILE }}}.' + v8.cachedDataVersionTag() + '.cached';
        cachedCodeFile = locateFile(cachedCodeFile);
        if (!nodeFS) nodeFS = require('fs');
        var hasCached = nodeFS.existsSync(cachedCodeFile);
        if (hasCached) {
#if RUNTIME_LOGGING
          err('NODE_CODE_CACHING: loading module');
#endif
          try {
            module = v8.deserialize(nodeFS.readFileSync(cachedCodeFile));
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
        nodeFS.writeFileSync(cachedCodeFile, v8.serialize(module));
      }
#else // NODE_CODE_CACHING
      module = new WebAssembly.Module(binary);
#endif // NODE_CODE_CACHING
      instance = new WebAssembly.Instance(module, info);
#if USE_OFFSET_CONVERTER
      wasmOffsetConverter = new WasmOffsetConverter(binary, module);
      {{{ runOnMainThread("removeRunDependency('offset-converter');") }}}
#endif
    } catch (e) {
      var str = e.toString();
      err('failed to compile wasm module: ' + str);
      if (str.indexOf('imported Memory') >= 0 ||
          str.indexOf('memory import') >= 0) {
        err('Memory size incompatibility issues may be due to changing INITIAL_MEMORY at runtime to something too large. Use ALLOW_MEMORY_GROWTH to allow any size memory (and also make sure not to set INITIAL_MEMORY at runtime to something smaller than it was at compile time).');
      }
      throw e;
    }
#if LOAD_SOURCE_MAP
    receiveSourceMapJSON(getSourceMap());
#endif
    receiveInstance(instance, module);
  }
#endif
  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to run the instantiation parallel
  // to any other async startup actions they are performing.
  if (Module['instantiateWasm']) {
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
  instantiateAsync();
#if LOAD_SOURCE_MAP
  getSourceMapPromise().then(receiveSourceMapJSON);
#endif
  return {}; // no exports yet; we'll fill them in later
#else
  instantiateSync();
  return Module['asm']; // exports were assigned here
#endif
}

// Globals used by JS i64 conversions
var tempDouble;
var tempI64;

// === Body ===
