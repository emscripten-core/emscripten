
// === Auto-generated postamble setup entry stuff ===

Module.callMain = function callMain(args) {
  var argc = args.length+1;
  function pad() {
    for (var i = 0; i < QUANTUM_SIZE-1; i++) {
      argv.push(0);
    }
  }
  var argv = [Pointer_make(intArrayFromString("/bin/this.program"), null, ALLOC_STATIC, 'i8') ];
  pad();
  for (var i = 0; i < argc-1; i = i + 1) {
    argv.push(Pointer_make(intArrayFromString(args[i]), null, ALLOC_STATIC, 'i8'));
    pad();
  }
  argv.push(0);
  argv = Pointer_make(argv, null, ALLOC_STATIC, 'i32');

  return _main(argc, argv, 0);
}

function run(args) {
  args = args || Module['arguments'];

  var globalFuncs = [];

{{GLOBAL_VARS}}

  __globalConstructor__();

  var ret = null;
  if (Module['_main']) {
    ret = Module.callMain(args);
    __shutdownRuntime__();
  }
  return ret;
}
Module['run'] = run;

// {{PRE_RUN_ADDITIONS}}

#if INVOKE_RUN
#else
Module['noInitialRun'] = true;
#endif

if (!Module['noInitialRun']) {
  run();
}

// {{POST_RUN_ADDITIONS}}

