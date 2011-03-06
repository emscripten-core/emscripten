
// === Auto-generated postamble setup entry stuff ===

function callMain(args) {
  var argc = args.length+1;
  function pad() {
    for (var i = 0; i < QUANTUM_SIZE-1; i++) {
      argv.push(0);
    }
  }
  var argv = [Pointer_make(intArrayFromString("/bin/this.program"), null) ];
  pad();
  for (var i = 0; i < argc-1; i = i + 1) {
    argv.push(Pointer_make(intArrayFromString(args[i]), null));
    pad();
  }
  argv.push(0);
  argv = Pointer_make(argv, null);

  Module._main(argc, argv, 0);
}
Module['callMain'] = callMain;

function run(args) {
  __initializeRuntime__();

  var globalFuncs = [];

{{GLOBAL_VARS}}

  __globalConstructor__();

  if (Module['_main']) {
    Module.callMain(args);
    __shutdownRuntime__();
  }
}
Module['run'] = run;

// {{PRE_RUN_ADDITIONS}}

#if INVOKE_RUN
run(args);
#endif

// {{POST_RUN_ADDITIONS}}

