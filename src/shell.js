#if USE_MODULE_CLOSURE
(function () {
#endif

var config = typeof Module !== 'undefined' ? Module : null;
var Module = {};

// The environment setup code below is customized to use Module.
// *** Environment setup code ***
var ENVIRONMENT_IS_NODE = typeof process === 'object' && typeof require === 'function';
var ENVIRONMENT_IS_WEB = typeof window === 'object';
var ENVIRONMENT_IS_WORKER = typeof importScripts === 'function';
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_WORKER;

if (ENVIRONMENT_IS_NODE) {
  // Expose functionality in the same simple way that the shells work
  // Note that we pollute the global namespace here, otherwise we break in node
  Module['print'] = function(x) {
    process['stdout'].write(x + '\n');
  };
  Module['printErr'] = function(x) {
    process['stderr'].write(x + '\n');
  };

  var nodeFS = require('fs');
  var nodePath = require('path');

  Module['read'] = function(filename, binary) {
    filename = nodePath['normalize'](filename);
    var ret = nodeFS['readFileSync'](filename);
    // The path is absolute if the normalized version is the same as the resolved.
    if (!ret && filename != nodePath['resolve'](filename)) {
      filename = path.join(__dirname, '..', 'src', filename);
      ret = nodeFS['readFileSync'](filename);
    }
    if (ret && !binary) ret = ret.toString();
    return ret;
  };

  Module['readBinary'] = function(filename) { return Module['read'](filename, true) };

  Module['load'] = function(f) {
    globalEval(read(f));
  };

  Module['arguments'] = process['argv'].slice(2);
}

if (ENVIRONMENT_IS_SHELL) {
  Module['print'] = print;
  if (typeof printErr != 'undefined') Module['printErr'] = printErr; // not present in v8 or older sm

  Module['read'] = read;
  Module['readBinary'] = function(f) {
    return read(f, 'binary');
  };

  if (typeof scriptArgs != 'undefined') {
    Module['arguments'] = scriptArgs;
  } else if (typeof arguments != 'undefined') {
    Module['arguments'] = arguments;
  }
}

if (ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_WORKER) {
  Module['print'] = function(x) {
    console.log(x);
  };

  Module['printErr'] = function(x) {
    console.log(x);
  };
}

if (ENVIRONMENT_IS_WEB || ENVIRONMENT_IS_WORKER) {
  Module['read'] = function(url) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, false);
    xhr.send(null);
    return xhr.responseText;
  };

  if (typeof arguments != 'undefined') {
    Module['arguments'] = arguments;
  }
}

if (ENVIRONMENT_IS_WORKER) {
  // We can do very little here...
  var TRY_USE_DUMP = false;
  Module['print'] = (TRY_USE_DUMP && (typeof(dump) !== "undefined") ? (function(x) {
    dump(x);
  }) : (function(x) {
    // self.postMessage(x); // enable this if you want stdout to be sent as messages
  }));

  Module['load'] = importScripts;
}

if (!ENVIRONMENT_IS_WORKER && !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE && !ENVIRONMENT_IS_SHELL) {
  // Unreachable because SHELL is dependant on the others
  throw 'Unknown runtime environment. Where are we?';
}

function globalEval(x) {
  eval.call(null, x);
}
if (!Module['load'] == 'undefined' && Module['read']) {
  Module['load'] = function(f) {
    globalEval(Module['read'](f));
  };
}
if (!Module['print']) {
  Module['print'] = function(){};
}
if (!Module['printErr']) {
  Module['printErr'] = Module['print'];
}
if (!Module['arguments']) {
  Module['arguments'] = [];
}
// *** Environment setup code ***

// Closure helpers
Module.print = Module['print'];
Module.printErr = Module['printErr'];

  {{BODY}}

  // {{MODULE_ADDITIONS}}

// merge in the configuration global if it exists
if (config) {
  for (var key in config) {
    if (!config.hasOwnProperty(key)) {
      continue;
    }
    var val = config[key];
    // old Module definitions had preInit / preRun / postRun callbacks
    // specified as either a single function or an array of functions.
    // here we're taking those functions and passing them to the 
    // appropriate add<type> function (e.g. addPreRun).
    if (key === 'preInit' || key === 'preRun' || key === 'postRun') {
      key = key.charAt(0).toUpperCase() + key.slice(1);  // capitalize first letter

      var fn = Module['add' + key];
      if (typeof val === 'function') {
        fn(val);
      } else {
        for (var i = 0; i < val.length; i++) {
          fn(val[i]);
        }
      }
    } else {
      Module[key] = config[key];
    }
  }
}

// {{PRE_RUN_ADDITIONS}}

#if INVOKE_RUN
if (!Module['noInitialRun']) {
  Module.preload(Module.run);
}
#endif

// {{POST_RUN_ADDITIONS}}

// require.js
if (typeof define !== 'undefined' && define.amd) {
  define(function () {
    return Module;
  });
}
// Node.js
else if (typeof module !== 'undefined' && module.exports) {
  module.exports = Module;
}
// browser
else {
  this['{{{ MODULE_NAME }}}'] = Module;
}

#if USE_MODULE_CLOSURE
})();
#endif