
// === Auto-generated postamble setup entry stuff ===

function run(args) {
  __initializeRuntime__();

  var globalFuncs = [];

{{GLOBAL_VARS}}

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

  __globalConstructor__();

  if (Module['_main']) {
    _main(argc, argv);
    __shutdownRuntime__();
  }
}

run(args);

