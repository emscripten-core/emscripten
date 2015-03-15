//"use strict";

// LLVM => JavaScript compiler, main entry point

try {
  // On SpiderMonkey, prepare a large amount of GC space
  gcparam('maxBytes', 1024*1024*1024);
} catch(e) {}


// The environment setup code appears here, in js_optimizer.js and in tests/hello_world.js because it can't be shared. Keep them in sync!
// It also appears, in modified form, in shell.js.
// *** Environment setup code ***
var arguments_ = [];

var ENVIRONMENT_IS_NODE = typeof process === 'object' && typeof require === 'function';
var ENVIRONMENT_IS_WEB = typeof window === 'object';
var ENVIRONMENT_IS_WORKER = typeof importScripts === 'function';
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

if (ENVIRONMENT_IS_NODE) {
  // Expose functionality in the same simple way that the shells work
  // Note that we pollute the global namespace here, otherwise we break in node
  print = function(x) {
    process['stdout'].write(x + '\n');
  };
  printErr = function(x) {
    process['stderr'].write(x + '\n');
  };

  var nodeFS = require('fs');
  var nodePath = require('path');

  if (!nodeFS.existsSync) {
    nodeFS.existsSync = function(path) {
      try {
        return !!nodeFS.readFileSync(path);
      } catch(e) {
        return false;
      }
    }
  }

  function find(filename) {
    var prefixes = [__dirname, process.cwd()];
    for (var i = 0; i < prefixes.length; ++i) {
      var combined = nodePath.join(prefixes[i], filename);
      if (nodeFS.existsSync(combined)) {
        return combined;
      }
    }
    return filename;
  }

  read = function(filename) {
    var absolute = find(filename);
    return nodeFS['readFileSync'](absolute).toString();
  };

  load = function(f) {
    globalEval(read(f));
  };

  arguments_ = process['argv'].slice(2);

} else if (ENVIRONMENT_IS_SHELL) {
  // Polyfill over SpiderMonkey/V8 differences
  if (!this['read']) {
    this['read'] = function(f) { snarf(f) };
  }

  if (typeof scriptArgs != 'undefined') {
    arguments_ = scriptArgs;
  } else if (typeof arguments != 'undefined') {
    arguments_ = arguments;
  }

} else if (ENVIRONMENT_IS_WEB) {
  printErr = function(x) {
    console.log(x);
  };

  if (!this['print']) this['print'] = printErr;

  this['read'] = function(url) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, false);
    xhr.send(null);
    return xhr.responseText;
  };

  if (this['arguments']) {
    arguments_ = arguments;
  }
} else if (ENVIRONMENT_IS_WORKER) {
  // We can do very little here...

  this['load'] = importScripts;

} else {
  throw 'Unknown runtime environment. Where are we?';
}

function globalEval(x) {
  eval.call(null, x);
}

if (typeof load == 'undefined' && typeof read != 'undefined') {
  this['load'] = function(f) {
    globalEval(read(f));
  };
}

if (typeof printErr === 'undefined') {
  this['printErr'] = function(){};
}

if (typeof print === 'undefined') {
  this['print'] = printErr;
}
// *** Environment setup code ***


DEBUG_MEMORY = false;

// Basic utilities

load('utility.js');

// Load settings, can be overridden by commandline

load('settings.js');

var settings_file = arguments_[0];
var ll_file = arguments_[1];
phase = arguments_[2];
if (phase == 'pre' || phase == 'glue') {
  additionalLibraries = Array.prototype.slice.call(arguments_, 3);
} else {
  var forwardedDataFile = arguments_[3];
  additionalLibraries = Array.prototype.slice.call(arguments_, 4);
}

if (settings_file) {
  var settings = JSON.parse(read(settings_file));
  for (key in settings) {
    var value = settings[key];
    if (value[0] == '@') {
      // response file type thing, workaround for large inputs: value is @path-to-file
      try {
        value = JSON.parse(read(value.substr(1)));
      } catch(e) {
        // continue normally; assume it is not a response file
      }
    }
    eval(key + ' = ' + JSON.stringify(value));
  }
}


if (CORRECT_SIGNS >= 2) {
  CORRECT_SIGNS_LINES = set(CORRECT_SIGNS_LINES); // for fast checking
}
if (CORRECT_OVERFLOWS >= 2) {
  CORRECT_OVERFLOWS_LINES = set(CORRECT_OVERFLOWS_LINES); // for fast checking
}
if (CORRECT_ROUNDINGS >= 2) {
  CORRECT_ROUNDINGS_LINES = set(CORRECT_ROUNDINGS_LINES); // for fast checking
}
if (SAFE_HEAP >= 2) {
  SAFE_HEAP_LINES = set(SAFE_HEAP_LINES); // for fast checking
}

EXPORTED_FUNCTIONS = set(EXPORTED_FUNCTIONS);
EXPORTED_GLOBALS = set(EXPORTED_GLOBALS);
EXCEPTION_CATCHING_WHITELIST = set(EXCEPTION_CATCHING_WHITELIST);

// TODO: Implement support for proper preprocessing, e.g. "#if A || B" and "#if defined(A) || defined(B)" to
// avoid needing this here.
USES_GL_EMULATION = FULL_ES2 || LEGACY_GL_EMULATION;

DEAD_FUNCTIONS.forEach(function(dead) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push(dead.substr(1));
});
DEAD_FUNCTIONS = numberedSet(DEAD_FUNCTIONS);

RUNTIME_DEBUG = LIBRARY_DEBUG || GL_DEBUG;

if (SAFE_HEAP) USE_BSS = 0; // must initialize heap for safe heap

if (NO_BROWSER) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE = DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.filter(function(func) { return func !== '$Browser' });
}

// Settings sanity checks

assert(!(USE_TYPED_ARRAYS === 2 && QUANTUM_SIZE !== 4), 'For USE_TYPED_ARRAYS == 2, must have normal QUANTUM_SIZE of 4');
assert(!(!NAMED_GLOBALS && BUILD_AS_SHARED_LIB), 'shared libraries must have named globals');

// Output some info and warnings based on settings

if (phase == 'pre') {
  if (!MICRO_OPTS || !RELOOP || ASSERTIONS || CHECK_SIGNS || CHECK_OVERFLOWS || INIT_HEAP ||
      !SKIP_STACK_IN_SMALL || SAFE_HEAP || !DISABLE_EXCEPTION_CATCHING) {
    print('// Note: Some Emscripten settings will significantly limit the speed of the generated code.');
  } else {
    print('// Note: For maximum-speed code, see "Optimizing Code" on the Emscripten wiki, https://github.com/kripken/emscripten/wiki/Optimizing-Code');
  }

  if (DOUBLE_MODE || CORRECT_SIGNS || CORRECT_OVERFLOWS || CORRECT_ROUNDINGS || CHECK_HEAP_ALIGN) {
    print('// Note: Some Emscripten settings may limit the speed of the generated code.');
  }
}

if (VERBOSE) printErr('VERBOSE is on, this generates a lot of output and can slow down compilation');

if (!BOOTSTRAPPING_STRUCT_INFO) {
  // Load struct and define information.
  //try {
    var temp = JSON.parse(read(STRUCT_INFO));
  //} catch(e) {
  //  printErr('cannot load struct info at ' + STRUCT_INFO + ' : ' + e + ', trying in current dir');
  //  temp = JSON.parse(read('struct_info.compiled.json'));
  //}
  C_STRUCTS = temp.structs;
  C_DEFINES = temp.defines;
} else {
  C_STRUCTS = {};
  C_DEFINES = {};
}

// Load compiler code

load('modules.js');
load('parseTools.js');
load('intertyper.js');
load('analyzer.js');
load('jsifier.js');
if (phase == 'funcs' && RELOOP) { // XXX handle !singlePhase
  RelooperModule = { TOTAL_MEMORY: ceilPowerOfTwo(2*RELOOPER_BUFFER_SIZE) };
  //try {
    load(RELOOPER);
  //} catch(e) {
  //  printErr('cannot load relooper at ' + RELOOPER + ' : ' + e + ', trying in current dir');
  //  load('relooper.js');
  //}
  assert(typeof Relooper != 'undefined');
}
globalEval(processMacros(preprocess(read('runtime.js'))));
Runtime.QUANTUM_SIZE = QUANTUM_SIZE;

var temp = {};
for (var i = 0; i < NECESSARY_BLOCKADDRS.length; i++) {
  var func = toNiceIdent(NECESSARY_BLOCKADDRS[i][0]);
  var label = toNiceIdent(NECESSARY_BLOCKADDRS[i][1]);
  if (!temp[func]) temp[func] = {};
  temp[func][label] = 1;
}
NECESSARY_BLOCKADDRS = temp;

//===============================
// Main
//===============================

// Read llvm

function compile(raw) {
  if (FAKE_X86_FP80) {
    raw = raw.replace(/x86_fp80/g, 'double');
  }
  if (raw.search('\r\n') >= 0) {
    raw = raw.replace(/\r\n/g, '\n'); // fix windows line endings
  }
  var lines = raw.split('\n');
  raw = null;

  // Pre-process the LLVM assembly

  Debugging.handleMetadata(lines);

  function runPhase(currPhase) {
    //printErr('// JS compiler in action, phase ' + currPhase + typeof lines + (lines === null));
    phase = currPhase;
    if (phase != 'pre' && phase != 'glue') {
      if (singlePhase) PassManager.load(read(forwardedDataFile));

      if (phase == 'funcs') {
        PreProcessor.eliminateUnneededIntrinsics(lines);
      }
    }

    // Do it

    var intertyped = intertyper(lines);
    if (singlePhase) lines = null;
    var analyzed = analyzer(intertyped);
    intertyped = null;
    JSify(analyzed);

    //dumpInterProf();
    //printErr(phase + ' paths (fast, slow): ' + [fastPaths, slowPaths]);
    B.print(phase);

    phase = null;

    if (DEBUG_MEMORY) {
      print('zzz. last gc: ' + gc());
      MemoryDebugger.dump();
      print('zzz. hanging now!');
      while(1){};
    }
  }

  // Normal operation is for each execution of compiler.js to run a single phase. The calling script sends us exactly the information we need, and it is easy to parallelize operation that way. However, it is also possible to run in an unoptimal multiphase mode, where a single invocation goes from ll to js directly. This is not recommended and will likely do a lot of duplicate processing.
  singlePhase = !!phase;

  if (singlePhase) {
    runPhase(phase);
  } else {
    runPhase('pre');
    runPhase('funcs');
    runPhase('post');
  }
}

B = new Benchmarker();

try {
  if (ll_file) {
    if (phase === 'glue') {
      compile(';');
    } else if (ll_file.indexOf(String.fromCharCode(10)) == -1) {
      compile(read(ll_file));
    } else {
      compile(ll_file); // we are given raw .ll
    }
  }
} catch(err) {
  if (err.toString().indexOf('Aborting compilation due to previous errors') != -1) {
    // Compiler failed on user error, print out the error message.
    printErr(err + ' | ' + err.stack);
  } else {
    // Compiler failed on internal compiler error!
    printErr('Internal compiler error in src/compiler.js! Please raise a bug report at https://github.com/kripken/emscripten/issues/ with a log of the build and the input files used to run. Exception message: "' + err + '" | ' + err.stack);
  }

  if (ENVIRONMENT_IS_NODE) {
    // Work around a node.js bug where stdout buffer is not flushed at process exit:
    // Instead of process.exit() directly, wait for stdout flush event.
    // See https://github.com/joyent/node/issues/1669 and https://github.com/kripken/emscripten/issues/2582
    // Workaround is based on https://github.com/RReverser/acorn/commit/50ab143cecc9ed71a2d66f78b4aec3bb2e9844f6
    process['stdout']['once']('drain', function () {
      process['exit'](1);
    });
    console.log(' '); // Make sure to print something to force the drain event to occur, in case the stdout buffer was empty.
    // Work around another node bug where sometimes 'drain' is never fired - make another effort
    // to emit the exit status, after a significant delay (if node hasn't fired drain by then, give up)
    setTimeout(function() {
      process['exit'](1);
    }, 500);
  } else throw err;
}

//var M = keys(tokenCacheMisses).map(function(m) { return [m, misses[m]] }).sort(function(a, b) { return a[1] - b[1] });
//printErr(dump(M.slice(M.length-10)));
//printErr('hits: ' + hits);

