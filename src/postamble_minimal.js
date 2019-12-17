
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

#if ASSERTIONS
// Module['wasm'] should contain a typed array of the Wasm object data, or a precompiled WebAssembly Module.
if (!Module['wasm']) throw 'Must load WebAssembly Module in to variable Module.wasm before adding compiled output .js script to the DOM';
#endif
WebAssembly.instantiate(Module['wasm'], imports).then(function(output) {

  // WebAssembly instantiation API gotcha: if Module['wasm'] above was a typed array, then the
  // output object will have an output.instance and output.module objects. But if Module['wasm']
  // is an already compiled WebAssembly module, then output is the WebAssembly instance itself.
  // Depending on the build mode, Module['wasm'] can mean a different thing.
#if MINIMAL_RUNTIME_STREAMING_WASM_COMPILATION
#if MIN_SAFARI_VERSION != TARGET_NOT_SUPPORTED || ENVIRONMENT_MAY_BE_NODE
  var asm = output.instance ? output.instance.exports : output.exports;
#else
  var asm = output.exports;
#endif
#else
  var asm = output.instance.exports;
#endif

#if DECLARE_ASM_MODULE_EXPORTS == 0

#if ENVIRONMENT_MAY_BE_NODE
  for(var i in asm) (typeof process !== "undefined" ? global : this)[i] = Module[i] = asm[i];
#else
  for(var i in asm) this[i] = Module[i] = asm[i];
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
