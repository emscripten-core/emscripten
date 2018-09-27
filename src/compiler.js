// Copyright 2010 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.

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

var ENVIRONMENT_IS_WEB = typeof window === 'object';
var ENVIRONMENT_IS_WORKER = typeof importScripts === 'function';
var ENVIRONMENT_IS_NODE = typeof process === 'object' && typeof require === 'function' && !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_WORKER;
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

// Polyfilling

if (!String.prototype.startsWith) {
  String.prototype.startsWith = function(searchString, position) {
    position = position || 0;
    return this.indexOf(searchString, position) === position;
  };
}

// Basic utilities

load('utility.js');

// Load settings, can be overridden by commandline

load('settings.js');

var settings_file = arguments_[0];
additionalLibraries = Array.prototype.slice.call(arguments_, 1);

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


EXPORTED_FUNCTIONS = set(EXPORTED_FUNCTIONS);
EXCEPTION_CATCHING_WHITELIST = set(EXCEPTION_CATCHING_WHITELIST);
IMPLEMENTED_FUNCTIONS = set(IMPLEMENTED_FUNCTIONS);

// TODO: Implement support for proper preprocessing, e.g. "#if A || B" and "#if defined(A) || defined(B)" to
// avoid needing this here.
USES_GL_EMULATION = FULL_ES2 || LEGACY_GL_EMULATION;

DEAD_FUNCTIONS.forEach(function(dead) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push(dead.substr(1));
});
DEAD_FUNCTIONS = numberedSet(DEAD_FUNCTIONS);

RUNTIME_DEBUG = LIBRARY_DEBUG || GL_DEBUG;

// Output some info and warnings based on settings

if (VERBOSE) printErr('VERBOSE is on, this generates a lot of output and can slow down compilation');

if (!BOOTSTRAPPING_STRUCT_INFO && !ONLY_MY_CODE) {
  // Load struct and define information.
  var temp = JSON.parse(read(STRUCT_INFO));
  C_STRUCTS = temp.structs;
  C_DEFINES = temp.defines;
} else {
  C_STRUCTS = {};
  C_DEFINES = {};
}

// Load compiler code

load('modules.js');
load('parseTools.js');
load('jsifier.js');
globalEval(processMacros(preprocess(read('runtime.js'), 'runtime.js')));
Runtime.QUANTUM_SIZE = QUANTUM_SIZE;

// State computations

ENVIRONMENT_MAY_BE_WEB    = !ENVIRONMENT || ENVIRONMENT === 'web';
ENVIRONMENT_MAY_BE_WORKER = !ENVIRONMENT || ENVIRONMENT === 'worker';
ENVIRONMENT_MAY_BE_NODE   = !ENVIRONMENT || ENVIRONMENT === 'node';
ENVIRONMENT_MAY_BE_SHELL  = !ENVIRONMENT || ENVIRONMENT === 'shell';

ENVIRONMENT_MAY_BE_WEB_OR_WORKER = ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER;

if (ENVIRONMENT && !(ENVIRONMENT_MAY_BE_WEB || ENVIRONMENT_MAY_BE_WORKER || ENVIRONMENT_MAY_BE_NODE || ENVIRONMENT_MAY_BE_SHELL)) {
  throw 'Invalid environment specified in "ENVIRONMENT": ' + ENVIRONMENT + '. Should be one of: web, worker, node, shell.';
}

//===============================
// Main
//===============================

B = new Benchmarker();

try {
  var dummyData = {functionStubs: []}
  JSify(dummyData);

  //dumpInterProf();
  //printErr('paths (fast, slow): ' + [fastPaths, slowPaths]);
  B.print('glue');

  if (DEBUG_MEMORY) {
    print('zzz. last gc: ' + gc());
    MemoryDebugger.dump();
    print('zzz. hanging now!');
    while(1){};
  }
} catch(err) {
  if (err.toString().indexOf('Aborting compilation due to previous errors') != -1) {
    // Compiler failed on user error, don't print the stacktrace in this case.
    printErr(err);
  } else {
    // Compiler failed on internal compiler error!
    printErr('Internal compiler error in src/compiler.js!');
    printErr('Please create a bug report at https://github.com/kripken/emscripten/issues/ with a log of the build and the input files used to run. Exception message: "' + err + '" | ' + err.stack);
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

