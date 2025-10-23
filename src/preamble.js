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
{{{ makeModuleReceiveWithVar('dynamicLibraries', undefined, '[]') }}}
#endif

{{{ makeModuleReceiveWithVar('wasmBinary') }}}

#if WASM2JS
#if WASM != 2
// WASM == 2 includes wasm2js.js separately.
#include "wasm2js.js"
#endif

if (WebAssembly.isWasm2js) {
  // We don't need to actually download a wasm binary, mark it as present but
  // empty.
  wasmBinary = [];
}
#endif

#if ASSERTIONS && WASM == 1
if (!globalThis.WebAssembly) {
  err('no native wasm support detected');
}
#endif

// Wasm globals

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

/**
 * Indicates whether filename is delivered via file protocol (as opposed to http/https)
 * @noinline
 */
var isFileURI = (filename) => filename.startsWith('file://');

#include "runtime_common.js"

#if ASSERTIONS
assert(globalThis.Int32Array && globalThis.Float64Array && Int32Array.prototype.subarray && Int32Array.prototype.set,
       'JS engine does not provide full typed array support');
#endif

#if RELOCATABLE
var __RELOC_FUNCS__ = [];
#endif

function preRun() {
#if ASSERTIONS && PTHREADS
  assert(!ENVIRONMENT_IS_PTHREAD); // PThreads reuse the runtime from the main thread.
#endif
#if expectToReceiveOnModule('preRun')
  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    while (Module['preRun'].length) {
      addOnPreRun(Module['preRun'].shift());
    }
  }
#if ASSERTIONS
  consumedModuleProp('preRun');
#endif
#endif
  <<< ATPRERUNS >>>
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
  if (ENVIRONMENT_IS_PTHREAD) return startWorker();
#endif

#if STACK_OVERFLOW_CHECK >= 2
  setStackLimits();
#endif

#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif

#if RELOCATABLE
  callRuntimeCallbacks(__RELOC_FUNCS__);
#endif

  <<< ATINITS >>>

#if hasExportedSymbol('__wasm_call_ctors')
#if WASM_ESM_INTEGRATION
  ___wasm_call_ctors();
#else
  wasmExports['__wasm_call_ctors']();
#endif
#endif

  <<< ATPOSTCTORS >>>
}

#if HAS_MAIN
function preMain() {
#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
  <<< ATMAINS >>>
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
  {{{ runIfWorkerThread('return;') }}} // PThreads reuse the runtime from the main thread.
#if !STANDALONE_WASM
  ___funcs_on_exit(); // Native atexit() functions
#endif
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
  {{{ runIfWorkerThread('return;') }}} // PThreads reuse the runtime from the main thread.

#if expectToReceiveOnModule('postRun')
  if (Module['postRun']) {
    if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
    while (Module['postRun'].length) {
      addOnPostRun(Module['postRun'].shift());
    }
  }
#if ASSERTIONS
  consumedModuleProp('postRun');
#endif
#endif

  <<< ATPOSTRUNS >>>
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
  readyPromiseReject?.(e);
#endif
  // Throw the error whether or not MODULARIZE is set because abort is used
  // in code paths apart from instantiation where an exception is expected
  // to be thrown when abort is called.
  throw e;
}

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
{{{
addAtModule(`
Module['FS_createDataFile'] = FS.createDataFile;
Module['FS_createPreloadedFile'] = FS.createPreloadedFile;
`);
}}}
#endif

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
      throw new Error('program has already aborted!');
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
    var func = realGet.call(wasmTable, {{{ toIndexType('i') }}});
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

#if !SOURCE_PHASE_IMPORTS && !WASM_ESM_INTEGRATION
var wasmBinaryFile;

#if WASM2JS && WASM != 2

// When building with wasm2js these 3 functions all no-ops.
function findWasmBinary(file) {}
function getBinarySync(file) {}
function getWasmBinary(file) {}

#else

function findWasmBinary() {
#if SINGLE_FILE && SINGLE_FILE_BINARY_ENCODE && !WASM2JS
  return binaryDecode("<<< WASM_BINARY_DATA >>>");
#elif SINGLE_FILE
  return base64Decode('<<< WASM_BINARY_DATA >>>');
#elif AUDIO_WORKLET || !EXPORT_ES6 // For an Audio Worklet, we cannot use `new URL()`.
  return locateFile('{{{ WASM_BINARY_FILE }}}');
#else

#if ENVIRONMENT_MAY_BE_SHELL
  if (ENVIRONMENT_IS_SHELL) {
    return '{{{ WASM_BINARY_FILE }}}';
  }
#endif

  if (Module['locateFile']) {
    return locateFile('{{{ WASM_BINARY_FILE }}}');
  }

  // Use bundler-friendly `new URL(..., import.meta.url)` pattern; works in browsers too.
  return new URL('{{{ WASM_BINARY_FILE }}}', import.meta.url).href;

#endif
}

function getBinarySync(file) {
#if SINGLE_FILE && SINGLE_FILE_BINARY_ENCODE
  return file;
#else
#if SINGLE_FILE
  if (ArrayBuffer.isView(file)) {
    return file;
  }
#endif
#if expectToReceiveOnModule('wasmBinary') || WASM2JS
  if (file == wasmBinaryFile && wasmBinary) {
    return new Uint8Array(wasmBinary);
  }
#endif
  if (readBinary) {
    return readBinary(file);
  }
  // Throwing a plain string here, even though it not normally adviables since
  // this gets turning into an `abort` in instantiateArrayBuffer.
#if WASM_ASYNC_COMPILATION
  throw 'both async and sync fetching of the wasm failed';
#else
  throw 'sync fetching of the wasm failed: you can preload it to Module["wasmBinary"] manually, or emcc.py will do that for you when generating HTML (but not JS)';
#endif
#endif
}

async function getWasmBinary(binaryFile) {
#if !SINGLE_FILE
  // If we don't have the binary yet, load it asynchronously using readAsync.
  if (!wasmBinary) {
    // Fetch the binary using readAsync
    try {
      var response = await readAsync(binaryFile);
      return new Uint8Array(response);
    } catch {
      // Fall back to getBinarySync below;
    }
  }
#endif

  // Otherwise, getBinarySync should be able to get it synchronously
  return getBinarySync(binaryFile);
}
#endif

#if SPLIT_MODULE
{{{ makeModuleReceiveWithVar('loadSplitModule', undefined, 'instantiateSync') }}}
var splitModuleProxyHandler = {
  get(target, moduleName, receiver) {
    if (moduleName.startsWith('placeholder')) {
      let secondaryFile;
      if (moduleName == 'placeholder') { // old format
        secondaryFile = wasmBinaryFile.slice(0, -5) + '.deferred.wasm';
      } else { // new format
        let moduleID = moduleName.split('.')[1];
        secondaryFile = wasmBinaryFile.slice(0, -5) + '.' + moduleID + '.wasm';
      }
      return new Proxy({}, {
        get(target, base, receiver) {
          return (...args) => {
#if ASYNCIFY == 2
            throw new Error('Placeholder function "' + base + '" should not be called when using JSPI.');
#else
#if RUNTIME_DEBUG
            dbg(`placeholder function called: ${base}`);
#endif
            var imports = {'primary': wasmExports};
            // Replace '.wasm' suffix with '.deferred.wasm'.
            loadSplitModule(secondaryFile, imports, base);
#if RUNTIME_DEBUG
            dbg('instantiated deferred module, continuing');
#endif
#if RELOCATABLE
            // When the table is dynamically laid out, the placeholder functions names
            // are offsets from the table base. In the main module, the table base is
            // always 1.
            base = 1 + parseInt(base);
#endif
            return wasmTable.get({{{ toIndexType('base') }}})(...args);
#endif
          }
        }
      });
    }
    return target[moduleName];
  }
};
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
  return [instance, module];
}
#endif

#if WASM_ASYNC_COMPILATION
async function instantiateArrayBuffer(binaryFile, imports) {
  try {
    var binary = await getWasmBinary(binaryFile);
    var instance = await WebAssembly.instantiate(binary, imports);
    return instance;
  } catch (reason) {
    err(`failed to asynchronously prepare wasm: ${reason}`);
#if WASM == 2
#if ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
    if (globalThis.location) {
#endif
      // WebAssembly compilation failed, try running the JS fallback instead.
      var search = location.search;
      if (search.indexOf('_rwasm=0') < 0) {
        // Reload the page with the `_rwasm=0` argument
        location.href += (search ? search + '&' : '?') + '_rwasm=0';
        // Return a promise that never resolves.  We don't want to
        // call abort below, or return an error to our caller.
        return new Promise(() => {});
      }
#if ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL
    }
#endif
#endif // WASM == 2

#if ASSERTIONS
    // Warn on some common problems.
    if (isFileURI(binaryFile)) {
      err(`warning: Loading from a file URI (${binaryFile}) is not supported in most browsers. See https://emscripten.org/docs/getting_started/FAQ.html#how-do-i-run-a-local-webserver-for-testing-why-does-my-program-stall-in-downloading-or-preparing`);
    }
#endif
    abort(reason);
  }
}

async function instantiateAsync(binary, binaryFile, imports) {
#if !SINGLE_FILE
  if (!binary
#if MIN_SAFARI_VERSION < 150000
      // See: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WebAssembly/instantiateStreaming
      && WebAssembly.instantiateStreaming
#endif
#if ENVIRONMENT_MAY_BE_WEBVIEW
      // Don't use streaming for file:// delivered objects in a webview, fetch them synchronously.
      && !isFileURI(binaryFile)
#endif
#if ENVIRONMENT_MAY_BE_NODE
      // Avoid instantiateStreaming() on Node.js environment for now, as while
      // Node.js v18.1.0 implements it, it does not have a full fetch()
      // implementation yet.
      //
      // Reference:
      //   https://github.com/emscripten-core/emscripten/pull/16917
      && !ENVIRONMENT_IS_NODE
#endif
#if ENVIRONMENT_MAY_BE_SHELL
      // Shell environments don't have fetch.
      && !ENVIRONMENT_IS_SHELL
#endif
     ) {
    try {
      var response = fetch(binaryFile, {{{ makeModuleReceiveExpr('fetchSettings', "{ credentials: 'same-origin' }") }}});
      var instantiationResult = await WebAssembly.instantiateStreaming(response, imports);
      return instantiationResult;
    } catch (reason) {
      // We expect the most common failure cause to be a bad MIME type for the binary,
      // in which case falling back to ArrayBuffer instantiation should work.
      err(`wasm streaming compile failed: ${reason}`);
      err('falling back to ArrayBuffer instantiation');
      // fall back of instantiateArrayBuffer below
    };
  }
#endif
  return instantiateArrayBuffer(binaryFile, imports);
}
#endif // WASM_ASYNC_COMPILATION
#endif // SOURCE_PHASE_IMPORTS

#if !WASM_ESM_INTEGRATION
function getWasmImports() {
#if PTHREADS || WASM_WORKERS || (IMPORTED_MEMORY && MODULARIZE == 'instance')
  assignWasmImports();
#endif
#if ASYNCIFY && (ASSERTIONS || ASYNCIFY == 2)
  // instrumenting imports is used in asyncify in two ways: to add assertions
  // that check for proper import use, and for ASYNCIFY=2 we use them to set up
  // the Promise API on the import side.
#if PTHREADS
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
  var imports = {
#if MINIFY_WASM_IMPORTED_MODULES
    'a': wasmImports,
#else // MINIFY_WASM_IMPORTED_MODULES
    'env': wasmImports,
    '{{{ WASI_MODULE_NAME }}}': wasmImports,
#endif // MINIFY_WASM_IMPORTED_MODULES
#if RELOCATABLE
    'GOT.mem': new Proxy(wasmImports, GOTHandler),
    'GOT.func': new Proxy(wasmImports, GOTHandler),
#endif
  };
#if SPLIT_MODULE
  imports = new Proxy(imports, splitModuleProxyHandler);
#endif
  return imports;
}

// Create the wasm instance.
// Receives the wasm imports, returns the exports.
{{{ asyncIf(WASM_ASYNC_COMPILATION) }}}function createWasm() {
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

#if EXPORTED_RUNTIME_METHODS.includes('wasmExports')
    Module['wasmExports'] = wasmExports;
#endif

#if PTHREADS
#if MAIN_MODULE
    registerTLSInit(wasmExports['_emscripten_tls_init'], instance.exports, metadata);
#else
    registerTLSInit(wasmExports['_emscripten_tls_init']);
#endif
#endif

#if hasExportedSymbol('__wasm_apply_data_relocs')
    __RELOC_FUNCS__.push(wasmExports['__wasm_apply_data_relocs']);
#endif

    assignWasmExports(wasmExports);

#if ABORT_ON_WASM_EXCEPTIONS
    instrumentWasmTableWithAbort();
#endif

#if !IMPORTED_MEMORY
    updateMemoryViews();
#endif

#if PTHREADS || WASM_WORKERS
    // We now have the Wasm module loaded up, keep a reference to the compiled module so we can post it to the workers.
    wasmModule = module;
#endif
#if WASM_ASYNC_COMPILATION && !MODULARIZE
    removeRunDependency('wasm-instantiate');
#endif
    return wasmExports;
  }
#if WASM_ASYNC_COMPILATION && !MODULARIZE
  addRunDependency('wasm-instantiate');
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
    return receiveInstance(result['instance'], result['module']);
#else
    // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193, the above line no longer optimizes out down to the following line.
    // When the regression is fixed, can restore the above PTHREADS-enabled path.
    return receiveInstance(result['instance']);
#endif
  }
#endif // WASM_ASYNC_COMPILATION

  var info = getWasmImports();

#if expectToReceiveOnModule('instantiateWasm')
  // User shell pages can write their own Module.instantiateWasm = function(imports, successCallback) callback
  // to manually instantiate the Wasm module themselves. This allows pages to
  // run the instantiation parallel to any other async startup actions they are
  // performing.
  // Also pthreads and wasm workers initialize the wasm instance through this
  // path.
  if (Module['instantiateWasm']) {
    return new Promise((resolve, reject) => {
#if ASSERTIONS
      try {
#endif
        Module['instantiateWasm'](info, (inst, mod) => {
          resolve(receiveInstance(inst, mod));
        });
#if ASSERTIONS
      } catch(e) {
        err(`Module.instantiateWasm callback failed with error: ${e}`);
        reject(e);
      }
#endif
    });
  }
#endif

#if PTHREADS || WASM_WORKERS
  if ({{{ ENVIRONMENT_IS_WORKER_THREAD() }}}) {
    // Instantiate from the module that was recieved via postMessage from
    // the main thread. We can just use sync instantiation in the worker.
#if ASSERTIONS
    assert(wasmModule, "wasmModule should have been received via postMessage");
#endif
    var instance = new WebAssembly.Instance(wasmModule, getWasmImports());
    return receiveInstance(instance, wasmModule);
  }
#endif

#if SOURCE_PHASE_IMPORTS
  var instance = await WebAssembly.instantiate(wasmModule, info);
  var exports = receiveInstantiationResult({instance, 'module':wasmModule});
  return exports;
#else
  wasmBinaryFile ??= findWasmBinary();
#if WASM_ASYNC_COMPILATION
#if RUNTIME_DEBUG
  dbg('asynchronously preparing wasm');
#endif
  var result = await instantiateAsync(wasmBinary, wasmBinaryFile, info);
  var exports = receiveInstantiationResult(result);
  return exports;
#else // WASM_ASYNC_COMPILATION
  var result = instantiateSync(wasmBinaryFile, info);
#if PTHREADS || MAIN_MODULE
  return receiveInstance(result[0], result[1]);
#else
  // TODO: Due to Closure regression https://github.com/google/closure-compiler/issues/3193,
  // the above line no longer optimizes out down to the following line.
  // When the regression is fixed, we can remove this if/else.
  return receiveInstance(result[0]);
#endif
#endif // WASM_ASYNC_COMPILATION
#endif // SOURCE_PHASE_IMPORTS
}
#endif // WASM_ESM_INTEGRATION

#if !WASM_BIGINT
// Globals used by JS i64 conversions (see makeSetValue)
var tempDouble;
var tempI64;
#endif

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
