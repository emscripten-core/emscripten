// TODO: " u s e   s t r i c t ";

try {
  this['Module'] = Module;
} catch(e) {
  this['Module'] = Module = {};
}

// The environment setup code below is customized to use Module.
// *** Environment setup code ***
var ENVIRONMENT_IS_NODE = typeof process === 'object';
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

  Module['read'] = function(filename) {
    filename = nodePath['normalize'](filename);
    var ret = nodeFS['readFileSync'](filename).toString();
    // The path is absolute if the normalized version is the same as the resolved.
    if (!ret && filename != nodePath['resolve'](filename)) {
      filename = path.join(__dirname, '..', 'src', filename);
      ret = nodeFS['readFileSync'](filename).toString();
    }
    return ret;
  };

  Module['load'] = function(f) {
    globalEval(read(f));
  };

  Module['arguments'] = process['argv'].slice(2);

} else if (ENVIRONMENT_IS_SHELL) {
  Module['print'] = print;
  Module['printErr'] = printErr;

  // Polyfill over SpiderMonkey/V8 differences
  if (typeof read != 'undefined') {
    Module['read'] = read;
  } else {
    Module['read'] = function(f) { snarf(f) };
  }

  if (typeof scriptArgs != 'undefined') {
    Module['arguments'] = scriptArgs;
  } else if (typeof arguments != 'undefined') {
    Module['arguments'] = arguments;
  }

} else if (ENVIRONMENT_IS_WEB) {
  if (!Module['print']) {
    Module['print'] = function(x) {
      console.log(x);
    };
  }

  if (!Module['printErr']) {
    Module['printErr'] = function(x) {
      console.log(x);
    };
  }

  Module['read'] = function(url) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, false);
    xhr.send(null);
    return xhr.responseText;
  };

  if (typeof arguments != 'undefined') {
    Module['arguments'] = arguments;
  }
} else if (ENVIRONMENT_IS_WORKER) {
  // We can do very little here...

  Module['load'] = importScripts;

} else {
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
if (!Module['printErr']) {
  Module['printErr'] = function(){};
}
if (!Module['print']) {
  Module['print'] = Module['printErr'];
}
if (!Module['arguments']) {
  Module['arguments'] = [];
}
// *** Environment setup code ***

  {{BODY}}

  // {{MODULE_ADDITIONS}}

