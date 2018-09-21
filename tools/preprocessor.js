// Copyright 2018 The Emscripten Authors.  All rights reserved.
// Emscripten is available under two separate licenses, the MIT license and the
// University of Illinois/NCSA Open Source License.  Both these licenses can be
// found in the LICENSE file.
//
// Preprocessor tool.  This is a wrapper for the 'preprocess' function which
// allows it to be called as a standalone tool.
//
// Parameters:
//    setting file.  Can specify 'settings.js' here, alternatively create a temp
//                   file with modified settings and supply the filename here.
//    shell file     This is the file that will be processed by the preprocessor

// *** Environment setup code ***
var arguments_ = [];
var debug = false;

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
    var prefixes = [process.cwd(), nodePath.join(__dirname, '..', 'src')];
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

if (typeof load === 'undefined' && typeof read != 'undefined') {
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

// These global functions are referenced by parseTools, although only 'assert'
// is used by the preprocess function, so 'set' can be just a stub.
assert = function(condition, text) {
  if (!condition) {
    abort('Assertion failed: ' + text);
  }
}
set = function() {}

var settings_file = arguments_[0];
var shell_file = arguments_[1];

load(settings_file);
load('parseTools.js');

var from_html = read(shell_file);
var to_html = preprocess(from_html);

print(to_html);
