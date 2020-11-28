/**
 * @license
 * Copyright 2010 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

//"use strict";

// General JS utilities - things that might be useful in any JS project.
// Nothing specific to Emscripten appears here.

function safeQuote(x) {
  return x.replace(/"/g, '\\"')
          .replace(/'/g, "\\'");
}

function dump(item) {
  try {
    if (typeof item == 'object' && item !== null && item.funcData) {
      var funcData = item.funcData;
      item.funcData = null;
    }
    return '// ' + JSON.stringify(item, null, '  ').replace(/\n/g, '\n// ');
  } catch(e) {
    var ret = [];
    for (var i in item) {
      var j = item[i];
      if (typeof j === 'string' || typeof j === 'number') {
        ret.push(i + ': ' + j);
      } else {
        ret.push(i + ': [?]');
      }
    }
    return ret.join(',\n');
  } finally {
    if (funcData) item.funcData = funcData;
  }
}

function assert(a, msg) {
  if (!a) {
    msg = 'Assertion failed: ' + msg;
    print(msg);
    printErr('Stack: ' + new Error().stack);
    throw msg;
  }
}

function warn(a, msg) {
  if (!msg) {
    msg = a;
    a = false;
  }
  if (!a) {
    printErr('warning: ' + msg);
  }
}

function warnOnce(a, msg) {
  if (!msg) {
    msg = a;
    a = false;
  }
  if (!a) {
    if (!warnOnce.msgs) warnOnce.msgs = {};
    if (msg in warnOnce.msgs) return;
    warnOnce.msgs[msg] = true;
    printErr('warning: ' + msg);
  }
}

var abortExecution = false;

function error(msg) {
  abortExecution = true;
  printErr('error: ' + msg);
}

function range(size) {
  var ret = [];
  for (var i = 0; i < size; i++) ret.push(i);
  return ret;
}

function keys(x) {
  var ret = [];
  for (var a in x) ret.push(a);
  return ret;
}

function bind(self, func) {
  return function() {
    func.apply(self, arguments);
  };
}

function sum(x) {
  return x.reduce(function(a,b) { return a+b }, 0);
}

function mergeInto(obj, other) {
  for (var i in other) {
    obj[i] = other[i];
  }
  return obj;
}

function isNumber(x) {
  // XXX this does not handle 0xabc123 etc. We should likely also do x == parseInt(x) (which handles that), and remove hack |// handle 0x... as well|
  return x == parseFloat(x) || (typeof x == 'string' && x.match(/^-?\d+$/)) || x === 'NaN';
}

function isJsLibraryConfigIdentifier(ident) {
  return ident.endsWith('__sig') || ident.endsWith('__proxy') || ident.endsWith('__asm') || ident.endsWith('__inline')
   || ident.endsWith('__deps') || ident.endsWith('__postset') || ident.endsWith('__docs') || ident.endsWith('__import')
   || ident.endsWith('__nothrow');
}

// Sets

function set() {
  var args = typeof arguments[0] === 'object' ? arguments[0] : arguments;
  var ret = {};
  for (var i = 0; i < args.length; i++) {
    ret[args[i]] = 0;
  }
  return ret;
}
var unset = keys;

function isPowerOfTwo(x) {
  return x > 0 && ((x & (x-1)) == 0);
}

function Benchmarker() {
  var totals = {};
  var ids = [], lastTime = 0;
  this.start = function(id) {
    var now = Date.now();
    if (ids.length > 0) {
      totals[ids[ids.length-1]] += now - lastTime;
    }
    lastTime = now;
    ids.push(id);
    totals[id] = totals[id] || 0;
  };
  this.stop = function(id) {
    var now = Date.now();
    assert(id === ids[ids.length-1]);
    totals[id] += now - lastTime;
    lastTime = now;
    ids.pop();
  };
  this.print = function(text) {
    var ids = keys(totals);
    if (ids.length > 0) {
      ids.sort(function(a, b) { return totals[b] - totals[a] });
      printErr(text + ' times: \n' + ids.map(function(id) { return id + ' : ' + totals[id] + ' ms' }).join('\n'));
    }
  };
}
