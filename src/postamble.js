
// === Auto-generated postamble setup entry stuff ===

Module['callMain'] = function callMain(args) {
  assert(!preRunTasks.length, 'cannot call main when preRun functions remain to be called');

  args = args || Module['arguments'];

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

function preInit(callback) {
  callback = callback || function () {};

  var total = preInitTasks.length;

  var next = function () {
    // nothing left to do
    if (!preInitTasks.length) {
      return callback(null);
    }

    var fn = preInitTasks.shift();
    fn(function (err) {
      if (err) {
        return callback(err);
      }
      next();
    });
  };
  next();
}

function preRun(callback) {
  callback = callback || function () {};

  var progress = Module['monitorRunDependencies'] || function () {};
  var total = preRunTasks.length;

  var next = function () {
    // nothing left to do
    if (!preRunTasks.length) {
      progress(0, total);
      return callback(null);
    }

    var fn = preRunTasks.shift();
    fn(function (err) {
      progress(preRunTasks.length, total);

      if (err) {
        return callback(err);
      }

      next();
    });
  };

  next();

#if ASSERTIONS
  // Check for missing dependencies every few seconds
  if (typeof setTimeout !== 'undefined') {
    var checkDependenciesTimeout;

    var checkDependencies = function () {
      if (!preRunTasks.length) {
        if (checkDependenciesTimeout) {
          clearTimeout(checkDependenciesTimeout);
        }
        return;
      }

      Module.printErr('still waiting on run dependencies:');
      for (var i = 0; i < preRunTasks.length; i++) {
        Module.printErr('dependency: ' + preRunTasks[i].toString());
      }
      Module.printErr('(end of list)');

      checkDependenciesTimeout = setTimeout(checkDependencies, 10000);
    };

    checkDependencies();
  }
#endif
}

function postRun() {
  while (postRunTasks.length) {
    var fn = postRunTasks.shift();
    fn();
  }
}

function _run(args) {
  var ret = 0;

  ensureInitRuntime();

  preMain();

  if (Module['_main']) {
    ret = Module['callMain'](args);
  }

  if (!Module['noExitRuntime']) {
    exitRuntime();
  }

  postRun();

  return ret;
}

var calledRun = false;
function run(args) {
  if (calledRun) {
    Module.printErr('run() has already been called');
    return 0;
  }
  calledRun = true;

  if (Module['setStatus']) {
    Module['setStatus']('Running...');
    setTimeout(function() {
      setTimeout(function() {
        Module['setStatus']('');
      }, 1);
      if (!ABORT) _run(args);
    }, 1);
    return 0;
  } else {
    return _run(args);
  }
}
Module['run'] = Module.run = run;

function preload(onready) {
  // Bootstrap any potential run dependencies.
  preInit(function (err) {
    if (err) throw err;
    preRun(function (err) {
      if (err) throw err;
      onready();
    });
  });
}
Module['preload'] = Module.preload = preload;

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