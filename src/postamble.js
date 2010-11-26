
// === Auto-generated postamble setup entry stuff ===

function run(args) {
  __initializeRuntime__();

  var globalFuncs = [];

{{GLOBAL_VARS}}

  var failures = 0;
  while (globalFuncs.length > 0) {
    var func = globalFuncs.pop();
    try {
      var x = func();
      if (x == undefined) throw 'undefined';
      failures = 0;
    } catch (e) {
      failures++;
      if (failures > 2*globalFuncs.length) {
        throw 'Failed to generate global values';
      }
      globalFuncs.unshift(func);
      // We will try again later. The global vars we depend on should be resolved by then
    }
  }
  assert(globalFuncs.length === 0);

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

