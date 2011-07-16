
// === Auto-generated postamble setup entry stuff ===

Module.callMain = function callMain(args) {
  var argc = args.length+1;
  function pad() {
    for (var i = 0; i < QUANTUM_SIZE-1; i++) {
      argv.push(0);
    }
  }
  var argv = [allocate(intArrayFromString("/bin/this.program"), 'i8', ALLOC_STATIC) ];
  pad();
  for (var i = 0; i < argc-1; i = i + 1) {
    argv.push(allocate(intArrayFromString(args[i]), 'i8', ALLOC_STATIC));
    pad();
  }
  argv.push(0);
  argv = allocate(argv, 'i32', ALLOC_STATIC);

  return _main(argc, argv, 0);
}

function run(args) {
  args = args || Module['arguments'];

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

