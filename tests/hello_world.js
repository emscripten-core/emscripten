// *** Environment setup code ***
var arguments_ = [];

var ENVIRONMENT_IS_NODE = typeof process === 'object';
var ENVIRONMENT_IS_WEB = typeof window === 'object';
var ENVIRONMENT_IS_SHELL = !ENVIRONMENT_IS_WEB && !ENVIRONMENT_IS_NODE;

if (ENVIRONMENT_IS_NODE) {
  // Expose functionality in the same simple way that the shells work
  print = function(x) {
    process['stdout'].write(x + '\n');
  };
  printErr = function(x) {
    process['stderr'].write(x + '\n');
  };

  var nodeFS = require('fs');

  read = function(filename) {
    var ret = nodeFS['readFileSync'](filename).toString();
    if (!ret && filename[0] != '/') {
      filename = __dirname.split('/').slice(0, -1).join('/') + '/src/' + filename;
      ret = nodeFS['readFileSync'](filename).toString();
    }
    return ret;
  };

  arguments_ = process['argv'].slice(2);

} else if (ENVIRONMENT_IS_SHELL) {
  // Polyfill over SpiderMonkey/V8 differences
  if (!this['read']) {
    read = function(f) { snarf(f) };
  }

  if (!this['arguments']) {
    arguments_ = scriptArgs;
  } else {
    arguments_ = arguments;
  }

} else if (ENVIRONMENT_IS_WEB) {
  printErr = function(x) {
    console.log(x);
  };
  if (typeof print === 'undefined') {
    print = printErr;
  }

  read = function(url) {
    var xhr = new XMLHttpRequest();
    xhr.open('GET', url, false);
    xhr.send(null);
    return xhr.responseText;
  };

  if (this['arguments']) {
    arguments_ = arguments;
  }
} else {
  throw 'Unknown runtime environment. Where are we?';
}

function globalEval(x) {
  eval.call(null, x);
}

if (!this['load']) {
  load = function(f) {
    globalEval(read(f));
  };
}
// *** Environment setup code ***

print('hello, world!');

