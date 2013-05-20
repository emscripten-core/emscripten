
// === Auto-generated postamble setup entry stuff ===

Module['callMain'] = function callMain(args) {
  assert(runDependencies == 0, 'cannot call main when async dependencies remain! (listen on __ATMAIN__)');
  assert(!Module['preRun'] || Module['preRun'].length == 0, 'cannot call main when preRun functions remain to be called');

  args = args || [];

  ensureInitRuntime();

  var argc = args.length+1;
  function pad() {
    for (var i = 0; i < {{{ QUANTUM_SIZE }}}-1; i++) {
      argv.push(0);
    }
  }
  var argv = [allocate(intArrayFromString("/bin/this.program"), 'i8', ALLOC_NORMAL) ];
  pad();
  for (var i = 0; i < argc-1; i = i + 1) {
    argv.push(allocate(intArrayFromString(args[i]), 'i8', ALLOC_NORMAL));
    pad();
  }
  argv.push(0);
  argv = allocate(argv, 'i32', ALLOC_NORMAL);

#if BENCHMARK
  var start = Date.now();
#endif

  var ret;

  var initialStackTop = STACKTOP;
  try {
    ret = Module['_main'](argc, argv, 0);
  }
  catch(e) {
    if (e.name == 'ExitStatus') {
      return e.status;
    } else if (e == 'SimulateInfiniteLoop') {
      Module['noExitRuntime'] = true;
    } else {
      throw e;
    }
  } finally {
    STACKTOP = initialStackTop;
  }

#if BENCHMARK
  Module.realPrint('main() took ' + (Date.now() - start) + ' milliseconds');
#endif

  return ret;
}

{{GLOBAL_VARS}}

function run(args) {
  args = args || Module['arguments'];

  if (runDependencies > 0) {
    Module.printErr('run() called, but dependencies remain, so not running');
    return 0;
  }

  if (Module['preRun']) {
    if (typeof Module['preRun'] == 'function') Module['preRun'] = [Module['preRun']];
    var toRun = Module['preRun'];
    Module['preRun'] = [];
    for (var i = toRun.length-1; i >= 0; i--) {
      toRun[i]();
    }
    if (runDependencies > 0) {
      // a preRun added a dependency, run will be called later
      return 0;
    }
  }

  function doRun() {
    ensureInitRuntime();

    preMain();

    var ret = 0;
    calledRun = true;
    if (Module['_main'] && shouldRunNow) {
      ret = Module['callMain'](args);
      if (!Module['noExitRuntime']) {
        exitRuntime();
      }
    }
    if (Module['postRun']) {
      if (typeof Module['postRun'] == 'function') Module['postRun'] = [Module['postRun']];
      while (Module['postRun'].length > 0) {
        Module['postRun'].pop()();
      }
    }
    return ret;
  }

  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(function() {
      setTimeout(function() {
        Module['setStatus']('');
      }, 1);
      doRun();
    }, 1);
    return 0;
  } else {
    return doRun();
  }
}
Module['run'] = Module.run = run;

// {{PRE_RUN_ADDITIONS}}

if (Module['preInit']) {
  if (typeof Module['preInit'] == 'function') Module['preInit'] = [Module['preInit']];
  while (Module['preInit'].length > 0) {
    Module['preInit'].pop()();
  }
}

// shouldRunNow refers to calling main(), not run().
#if INVOKE_RUN
var shouldRunNow = true;
#else
var shouldRunNow = false;
#endif
if (Module['noInitialRun']) {
  shouldRunNow = false;
}

run();

// {{POST_RUN_ADDITIONS}}

#if BUILD_AS_WORKER

var buffer = 0, bufferSize = 0;
var inWorkerCall = false, workerResponded = false, workerCallbackId = -1;

onmessage = function(msg) {
  var func = Module['_' + msg.data['funcName']];
  if (!func) throw 'invalid worker function to call: ' + msg.data['funcName'];
  var data = msg.data['data'];
  if (data) {
    if (!data.byteLength) data = new Uint8Array(data);
    if (!buffer || bufferSize < data.length) {
      if (buffer) _free(buffer);
      bufferSize = data.length;
      buffer = _malloc(data.length);
    }
    HEAPU8.set(data, buffer);
  }

  inWorkerCall = true;
  workerResponded = false;
  workerCallbackId = msg.data['callbackId'];
  if (data) {
    func(buffer, data.length);
  } else {
    func(0, 0);
  }
  inWorkerCall = false;
}

#endif

