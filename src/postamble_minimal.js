/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// === Auto-generated postamble setup entry stuff ===
{{{ exportRuntime() }}}

#if hasExportedFunction('_main') // Only if user is exporting a C main(), we will generate a run() function that can be used to launch main.
function run() {
#if MEMORYPROFILER
  emscriptenMemoryProfiler.onPreloadComplete();
#endif

#if STACK_OVERFLOW_CHECK >= 2
  ___set_stack_limits(_emscripten_stack_get_base(), _emscripten_stack_get_end());
#endif

#if PROXY_TO_PTHREAD
  // User requested the PROXY_TO_PTHREAD option, so call a stub main which
  // pthread_create()s a new thread that will call the user's real main() for
  // the application.
  var ret = _emscripten_proxy_main();
#else
  var ret = _main();

#if EXIT_RUNTIME
  callRuntimeCallbacks(__ATEXIT__);
  {{{ getQuoted('ATEXITS') }}}
#if USE_PTHREADS
  PThread.runExitHandlers();
#endif
#endif

#if IN_TEST_HARNESS
  // fflush() filesystem stdio for test harness, since there are existing
  // tests that depend on this behavior.
  // For production use, instead print full lines to avoid this kind of lazy
  // behavior.
  if (typeof _fflush !== 'undefined') _fflush();
#endif

#if ASSERTIONS
  runtimeExited = true;
#endif
#endif

#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
}
#endif

function initRuntime(asm) {
#if ASSERTIONS
  runtimeInitialized = true;
#endif

#if USE_PTHREADS
  // Export needed variables that worker.js needs to Module.
  Module['_emscripten_tls_init'] = _emscripten_tls_init;
  Module['HEAPU32'] = HEAPU32;
  Module['__emscripten_thread_init'] = __emscripten_thread_init;
  Module['_pthread_self'] = _pthread_self;

  if (ENVIRONMENT_IS_PTHREAD) {
    PThread.initWorker();
    return;
  }
#endif

#if STACK_OVERFLOW_CHECK
  _emscripten_stack_init();
  writeStackCookie();
#endif

#if '___wasm_call_ctors' in IMPLEMENTED_FUNCTIONS
  asm['__wasm_call_ctors']();
#endif

  {{{ getQuoted('ATINITS') }}}
}

// Initialize wasm (asynchronous)

var imports = {
#if MINIFY_WASM_IMPORTED_MODULES
  'a': asmLibraryArg,
#else // MINIFY_WASM_IMPORTED_MODULES
  'env': asmLibraryArg
  , '{{{ WASI_MODULE_NAME }}}': asmLibraryArg
#endif // MINIFY_WASM_IMPORTED_MODULES
};

// In non-fastcomp non-asm.js builds, grab wasm exports to outer scope
// for emscripten_get_exported_function() to be able to access them.
#if LibraryManager.has('library_exports.js')
var asm;
#endif

#if USE_PTHREADS
var wasmModule;
#if PTHREAD_POOL_SIZE
function loadWasmModuleToWorkers() {
#if PTHREAD_POOL_DELAY_LOAD
  PThread.unusedWorkers.forEach(PThread.loadWasmModuleToWorker);
#else
  var numWorkersToLoad = PThread.unusedWorkers.length;
  PThread.unusedWorkers.forEach(function(w) { PThread.loadWasmModuleToWorker(w, function() {
    // PTHREAD_POOL_DELAY_LOAD==0: we wanted to synchronously wait until the
    // Worker pool has loaded up. If all Workers have finished loading up the
    // Wasm Module, proceed with main()
    if (!--numWorkersToLoad) ready();
  })});
#endif
}
#endif

#endif

#if DECLARE_ASM_MODULE_EXPORTS
/*** ASM_MODULE_EXPORTS_DECLARES ***/
#endif

#if MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION
// https://caniuse.com/#feat=wasm and https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WebAssembly/instantiateStreaming
// Firefox 52 added Wasm support, but only Firefox 58 added instantiateStreaming.
// Chrome 57 added Wasm support, but only Chrome 61 added instantiateStreaming.
// Node.js and Safari do not support instantiateStreaming.
#if MIN_FIREFOX_VERSION < 58 || MIN_CHROME_VERSION < 61 || ENVIRONMENT_MAY_BE_NODE || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
#if ASSERTIONS && !WASM2JS
// Module['wasm'] should contain a typed array of the Wasm object data, or a
// precompiled WebAssembly Module.
if (!WebAssembly.instantiateStreaming && !Module['wasm']) throw 'Must load WebAssembly Module in to variable Module.wasm before adding compiled output .js script to the DOM';
#endif
(WebAssembly.instantiateStreaming
  ? WebAssembly.instantiateStreaming(fetch('{{{ TARGET_BASENAME }}}.wasm'), imports)
  : WebAssembly.instantiate(Module['wasm'], imports)).then(function(output) {
#else
WebAssembly.instantiateStreaming(fetch('{{{ TARGET_BASENAME }}}.wasm'), imports).then(function(output) {
#endif

#else // Non-streaming instantiation
#if ASSERTIONS && !WASM2JS
// Module['wasm'] should contain a typed array of the Wasm object data, or a
// precompiled WebAssembly Module.
if (!Module['wasm']) throw 'Must load WebAssembly Module in to variable Module.wasm before adding compiled output .js script to the DOM';
#endif

WebAssembly.instantiate(Module['wasm'], imports).then(function(output) {
#endif

#if USE_PTHREADS
  // Export Wasm module for pthread creation to access.
  wasmModule = output.module || Module['wasm'];
#endif
#if !LibraryManager.has('library_exports.js') && !EMBIND
  // If not using the emscripten_get_exported_function() API or embind, keep the
  // 'asm' exports variable in local scope to this instantiate function to save
  // code size.  (otherwise access it without to export it to outer scope)
  var
#endif
  // WebAssembly instantiation API gotcha: if Module['wasm'] above was a typed
  // array, then the output object will have an output.instance and
  // output.module objects. But if Module['wasm'] is an already compiled
  // WebAssembly module, then output is the WebAssembly instance itself.
  // Depending on the build mode, Module['wasm'] can mean a different thing.
#if MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION || MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION || USE_PTHREADS
  // https://caniuse.com/#feat=wasm and https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WebAssembly/instantiateStreaming
  // Firefox 52 added Wasm support, but only Firefox 58 added compileStreaming &
  // instantiateStreaming.
  // Chrome 57 added Wasm support, but only Chrome 61 added compileStreaming &
  // instantiateStreaming.
  // Node.js and Safari do not support compileStreaming or instantiateStreaming.
#if MIN_FIREFOX_VERSION < 58 || MIN_CHROME_VERSION < 61 || ENVIRONMENT_MAY_BE_NODE || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED || USE_PTHREADS
  // In pthreads, Module['wasm'] is an already compiled WebAssembly.Module. In
  // that case, 'output' is a WebAssembly.Instance.
  // In main thread, Module['wasm'] is either a typed array or a fetch stream.
  // In that case, 'output.instance' is the WebAssembly.Instance.
  asm = (output.instance || output).exports;
#else
  asm = output.exports;
#endif
#else
  asm = output.instance.exports;
#endif

#if USE_OFFSET_CONVERTER
#if USE_PTHREADS
  if (!ENVIRONMENT_IS_PTHREAD)
#endif
    wasmOffsetConverter = new WasmOffsetConverter(Module['wasm'], output.module);
#endif

#if !DECLARE_ASM_MODULE_EXPORTS
  exportAsmFunctions(asm);
#else
  /*** ASM_MODULE_EXPORTS ***/
#endif
  wasmTable = asm['__indirect_function_table'];
#if ASSERTIONS
  assert(wasmTable);
#endif

#if !IMPORTED_MEMORY
  wasmMemory = asm['memory'];
#if ASSERTIONS
  assert(wasmMemory);
  assert(wasmMemory.buffer.byteLength === {{{ INITIAL_MEMORY }}});
#endif
  updateGlobalBufferAndViews(wasmMemory.buffer);
#endif

#if MEM_INIT_METHOD == 1 && !MEM_INIT_IN_WASM && !SINGLE_FILE
#if ASSERTIONS
  if (!Module['mem']) throw 'Must load memory initializer as an ArrayBuffer in to variable Module.mem before adding compiled output .js script to the DOM';
#endif
  HEAPU8.set(new Uint8Array(Module['mem']), {{{ GLOBAL_BASE }}});
#endif

  initRuntime(asm);
#if USE_PTHREADS && PTHREAD_POOL_SIZE
  if (!ENVIRONMENT_IS_PTHREAD) loadWasmModuleToWorkers();
#if !PTHREAD_POOL_DELAY_LOAD  
  else
#endif
    ready();
#else
  ready();
#endif

#if USE_PTHREADS
  // This Worker is now ready to host pthreads, tell the main thread we can proceed.
  if (ENVIRONMENT_IS_PTHREAD) {
    moduleLoaded();
  }
#endif

#if ASSERTIONS || WASM == 2
}).catch(function(error) {
#if ASSERTIONS
  console.error(error);
#endif

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
#endif // ASSERTIONS || WASM == 2
});
