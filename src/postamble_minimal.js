
// === Auto-generated postamble setup entry stuff ===

{{{ exportRuntime() }}}

function run() {
#if MEMORYPROFILER
  emscriptenMemoryProfiler.onPreloadComplete();
#endif

#if PROXY_TO_PTHREAD
    // User requested the PROXY_TO_PTHREAD option, so call a stub main which pthread_create()s a new thread
    // that will call the user's real main() for the application.
    var ret = _proxy_main();
#else
    var ret = _main();
#endif

#if STACK_OVERFLOW_CHECK
  checkStackCookie();
#endif
}

function initRuntime(asm) {
#if ASSERTIONS
  runtimeInitialized = true;
#endif

#if USE_PTHREADS
  if (ENVIRONMENT_IS_PTHREAD) return;
  // Pass the thread address inside the asm.js scope to store it for fast access that avoids the need for a FFI out.
  __register_pthread_ptr(PThread.mainThreadBlock, /*isMainBrowserThread=*/!ENVIRONMENT_IS_WORKER, /*isMainRuntimeThread=*/1);
  _emscripten_register_main_browser_thread_id(PThread.mainThreadBlock);
#endif

#if STACK_OVERFLOW_CHECK
  writeStackCookie();
#endif

  /*** RUN_GLOBAL_INITIALIZERS(); ***/

  {{{ getQuoted('ATINITS') }}}
}

#if WASM

// Initialize wasm (asynchronous)

var imports = {
  'env': asmLibraryArg
  // TODO: Fix size bloat coming from WASI properly. The -s FILESYSTEM=1 check is too weak to properly DCE WASI linkage away.
  // (Emscripten now unconditionally uses WASI for stdio, perhaps replace that with web-friendly stdio)
  , '{{{ WASI_MODULE_NAME }}}': asmLibraryArg
#if WASM_BACKEND == 0
  , 'global': {
    'NaN': NaN,
    'Infinity': Infinity
  },
  'global.Math': Math,
  'asm2wasm': {
    'f64-rem': function(x, y) { return x % y; },
    'debugger': function() {
#if ASSERTIONS // Disable debugger; statement from being present in release builds to avoid Firefox deoptimizations, see https://bugzilla.mozilla.org/show_bug.cgi?id=1538375
      debugger;
#endif
    }
  }
#endif
};

#if DECLARE_ASM_MODULE_EXPORTS
/*** ASM_MODULE_EXPORTS_DECLARES ***/
#endif

#if MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION
// https://caniuse.com/#feat=wasm and https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WebAssembly/instantiateStreaming
// Firefox 52 added Wasm support, but only Firefox 58 added instantiateStreaming.
// Chrome 57 added Wasm support, but only Chrome 61 added instantiateStreaming.
// Node.js and Safari do not support instantiateStreaming.
#if MIN_FIREFOX_VERSION < 58 || MIN_CHROME_VERSION < 61 || ENVIRONMENT_MAY_BE_NODE || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
#if ASSERTIONS
// Module['wasm'] should contain a typed array of the Wasm object data, or a precompiled WebAssembly Module.
if (!WebAssembly.instantiateStreaming && !Module['wasm']) throw 'Must load WebAssembly Module in to variable Module.wasm before adding compiled output .js script to the DOM';
#endif
(WebAssembly.instantiateStreaming
  ? WebAssembly.instantiateStreaming(fetch('{{{ TARGET_BASENAME }}}.wasm'), imports)
  : WebAssembly.instantiate(Module['wasm'], imports)).then(function(output) {
#else
WebAssembly.instantiateStreaming(fetch('{{{ TARGET_BASENAME }}}.wasm'), imports).then(function(output) {
#endif

#else // Non-streaming instantiation
#if ASSERTIONS
// Module['wasm'] should contain a typed array of the Wasm object data, or a precompiled WebAssembly Module.
if (!Module['wasm']) throw 'Must load WebAssembly Module in to variable Module.wasm before adding compiled output .js script to the DOM';
#endif
WebAssembly.instantiate(Module['wasm'], imports).then(function(output) {
#endif

// WebAssembly instantiation API gotcha: if Module['wasm'] above was a typed array, then the
// output object will have an output.instance and output.module objects. But if Module['wasm']
// is an already compiled WebAssembly module, then output is the WebAssembly instance itself.
// Depending on the build mode, Module['wasm'] can mean a different thing.
#if MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION || MINIMAL_RUNTIME_STREAMING_WASM_INSTANTIATION
// https://caniuse.com/#feat=wasm and https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/WebAssembly/instantiateStreaming
// Firefox 52 added Wasm support, but only Firefox 58 added compileStreaming & instantiateStreaming.
// Chrome 57 added Wasm support, but only Chrome 61 added compileStreaming & instantiateStreaming.
// Node.js and Safari do not support compileStreaming or instantiateStreaming.
#if MIN_FIREFOX_VERSION < 58 || MIN_CHROME_VERSION < 61 || ENVIRONMENT_MAY_BE_NODE || MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED
  var asm = output.instance ? output.instance.exports : output.exports;
#else
  var asm = output.exports;
#endif
#else
  var asm = output.instance.exports;
#endif

#if DECLARE_ASM_MODULE_EXPORTS == 0

#if WASM_BACKEND
  // XXX Hack: some function names need to be mangled when exporting them from wasm module, others do not. 
  // https://github.com/emscripten-core/emscripten/issues/10054
  // Keep in sync with emscripten.py function treat_as_user_function(name).
  function asmjs_mangle(x) {
    var unmangledSymbols = {{{ buildStringArray(WASM_FUNCTIONS_THAT_ARE_NOT_NAME_MANGLED) }}};
    return x.indexOf('dynCall_') == 0 || unmangledSymbols.indexOf(x) != -1 ? x : '_' + x;
  }

#if ENVIRONMENT_MAY_BE_NODE
  for(var i in asm) (typeof process !== "undefined" ? global : this)[asmjs_mangle(i)] = asm[i];
#else
  for(var i in asm) this[asmjs_mangle(i)] = asm[i];
#endif

#else

#if ENVIRONMENT_MAY_BE_NODE
  for(var i in asm) (typeof process !== "undefined" ? global : this)[i] = asm[i];
#else
  for(var i in asm) this[i] = asm[i];
#endif

#endif

#else
  /*** ASM_MODULE_EXPORTS ***/
#endif

    initRuntime(asm);
    ready();
})
#if ASSERTIONS
.catch(function(error) {
  console.error(error);
})
#endif
;

#else

// Initialize asm.js (synchronous)
#if ASSERTIONS
if (!Module['mem']) throw 'Must load memory initializer as an ArrayBuffer in to variable Module.mem before adding compiled output .js script to the DOM';
#endif
initRuntime(asm);
ready();

#endif

{{GLOBAL_VARS}}
