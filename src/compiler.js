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

var ENVIRONMENT_IS_NODE = typeof process === 'object';
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
  this['print'] = printErr = function(x) {
    console.log(x);
  };

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


// Basic utilities

load('utility.js');

// Load settings, can be overridden by commandline

load('settings.js');

var settings_file = arguments_[0];
var ll_file = arguments_[1];
additionalLibraries = Array.prototype.slice.call(arguments_, 2);

if (settings_file) {
  var settings = JSON.parse(read(settings_file));
  for (setting in settings) {
    eval(setting + ' = ' + JSON.stringify(settings[setting]));
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

if (PGO) { // by default, correct everything during PGO
  CORRECT_SIGNS = CORRECT_SIGNS || 1;
  CORRECT_OVERFLOWS = CORRECT_OVERFLOWS || 1;
  CORRECT_ROUNDINGS = CORRECT_ROUNDINGS || 1;
}

EXPORTED_FUNCTIONS = set(EXPORTED_FUNCTIONS);
EXPORTED_GLOBALS = set(EXPORTED_GLOBALS);

RUNTIME_DEBUG = LIBRARY_DEBUG || GL_DEBUG;

// Settings sanity checks

assert(!(USE_TYPED_ARRAYS === 2 && QUANTUM_SIZE !== 4), 'For USE_TYPED_ARRAYS == 2, must have normal QUANTUM_SIZE of 4');

// Output some info and warnings based on settings

if (!MICRO_OPTS || !RELOOP || ASSERTIONS || CHECK_SIGNS || CHECK_OVERFLOWS || INIT_STACK || INIT_HEAP ||
    !SKIP_STACK_IN_SMALL || SAFE_HEAP || PGO || PROFILE || !DISABLE_EXCEPTION_CATCHING) {
  print('// Note: Some Emscripten settings will significantly limit the speed of the generated code.');
} else {
  print('// Note: For maximum-speed code, see "Optimizing Code" on the Emscripten wiki, https://github.com/kripken/emscripten/wiki/Optimizing-Code');
}

if (DOUBLE_MODE || CORRECT_SIGNS || CORRECT_OVERFLOWS || CORRECT_ROUNDINGS) {
  print('// Note: Some Emscripten settings may limit the speed of the generated code.');
}

// Load compiler code

load('framework.js');
load('modules.js');
load('parseTools.js');
load('intertyper.js');
load('analyzer.js');
load('jsifier.js');
globalEval(processMacros(preprocess(read('runtime.js'))));
Runtime.QUANTUM_SIZE = QUANTUM_SIZE;

//===============================
// Main
//===============================

// Read llvm

var raw = read(ll_file);
if (FAKE_X86_FP80) {
  raw = raw.replace(/x86_fp80/g, 'double');
}
var lines = raw.split('\n');
raw = null;

// Pre-process the LLVM assembly

Debugging.handleMetadata(lines);
PreProcessor.eliminateUnneededIntrinsics(lines);

// Do it

var intertyped = intertyper(lines);
lines = null;
var analyzed = analyzer(intertyped);
intertyped = null;
JSify(analyzed);

if (DEBUG_MEMORY) {
  print('zzz. last gc: ' + gc());
  MemoryDebugger.dump();
  print('zzz. hanging now!');
  while(1){};
}

