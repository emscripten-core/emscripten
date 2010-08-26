
// === Auto-generated postamble setup entry stuff ===

function run(args) {
  var argc = args.length+1;
  var argv = [Pointer_make(intArrayFromString("/bin/this.program")) ];
  for (var i = 0; i < argc-1; i = i + 1) {
    argv.push(Pointer_make(intArrayFromString(args[i])));
  }
  argv = Pointer_make(argv);

  __globalConstructor__();

  _main(argc, argv);
}

try {
  run(arguments);
} catch (e) {
  print("Fatal exception: " + e.stack);
  throw e;
}

