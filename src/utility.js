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

function dumpKeys(item) {
  var ret = [];
  for (var i in item) {
    var j = item[i];
    if (typeof j === 'string' || typeof j === 'number') {
      ret.push(i + ': ' + j);
    } else {
      ret.push(i + ': [?]');
    }
  }
  return ret.join(', ');
}

function assertEq(a, b) {
  if (a !== b) {
    printErr('Stack: ' + new Error().stack);
    throw 'Should have been equal: ' + a + ' : ' + b;
  }
  return false;
}

function assertTrue(a, msg) {
  if (!a) {
    msg = 'Assertion failed: ' + msg;
    print(msg);
    printErr('Stack: ' + new Error().stack);
    throw msg;
  }
}
var assert = assertTrue;

function warn(a, msg) {
  if (!msg) {
    msg = a;
    a = false;
  }
  if (!a) {
    printErr('Warning: ' + msg);
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
    printErr('Warning: ' + msg);
  }
}

var abortExecution = false;

function error(msg) {
  abortExecution = true;
  printErr('Error: ' + msg);
}

function dedup(items, ident) {
  var seen = {};
  if (ident) {
    return items.filter(function(item) {
      if (seen[item[ident]]) return false;
      seen[item[ident]] = true;
      return true;
    });
  } else {
    return items.filter(function(item) {
      if (seen[item]) return false;
      seen[item] = true;
      return true;
    });
  }
}

function range(size) {
  var ret = [];
  for (var i = 0; i < size; i++) ret.push(i);
  return ret;
}

function zeros(size) {
  var ret = [];
  for (var i = 0; i < size; i++) ret.push(0);
  return ret;
}

function spaces(size) {
  var ret = '';
  for (var i = 0; i < size; i++) ret += ' ';
  return ret;
}

function keys(x) {
  var ret = [];
  for (var a in x) ret.push(a);
  return ret;
}

function values(x) {
  var ret = [];
  for (var a in x) ret.push(x[a]);
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

function sumTruthy(x) {
  return x.reduce(function(a,b) { return (!!a)+(!!b) }, 0);
}

function sumStringy(x) {
  return x.reduce(function(a,b) { return a+b }, '');
}

function filterTruthy(x) {
  return x.filter(function(y) { return !!y });
}

function loopOn(array, func) {
  for (var i = 0; i < array.length; i++) {
    func(i, array[i]);
  }
}

// Splits out items that pass filter. Returns also the original sans the filtered
function splitter(array, filter) {
  var splitOut = array.filter(filter);
  var leftIn = array.filter(function(x) { return !filter(x) });
  return { leftIn: leftIn, splitOut: splitOut };
}

function dcheck(tag) {
  return DEBUG_TAGS_SHOWING.indexOf(arguments[0]) != -1;
}
var DPRINT_INDENT = '';
function dprint_indent() {
  DPRINT_INDENT += '   ';
}
function dprint_unindent() {
  DPRINT_INDENT = DPRINT_INDENT.substr(3);
}

function dprint() {
  var text;
  if (arguments[1]) {
    if (!dcheck(arguments[0])) return;
    text = arguments[1];
  } else {
    text = arguments[0];
  }
  if (typeof text === 'function') {
    text = text(); // Allows deferred calculation, so dprints don't slow us down when not needed
  }
  text = DPRINT_INDENT + '// ' + text;
  printErr(text);
}

var PROF_ORIGIN = Date.now();
var PROF_TIME = PROF_ORIGIN;
function PROF(pass) {
  if (!pass) {
    dprint("Profiling: " + ((Date.now() - PROF_TIME)/1000) + ' seconds, total: ' + ((Date.now() - PROF_ORIGIN)/1000));
  }
  PROF_TIME = Date.now();
}

// Usage: arrayOfArrays.reduce(concatenator, []);
function concatenator(x, y) {
  return x.concat(y);
}

function mergeInto(obj, other) {
  for (i in other) {
    obj[i] = other[i];
  }
  return obj;
}

function isNumber(x) {
  return x == parseFloat(x) || (typeof x == 'string' && x.match(/^-?\d+$/));
}

function isArray(x) {
  try {
    return typeof x === 'object' && 'length' in x && 'slice' in x;
  } catch(e) {
    return false;
  }
}

// Flattens something like [5, 6, 'hi', [1, 'bye'], 44] into
// [5, 6, 'hi', 1, bye, 44].
function flatten(x) {
  if (typeof x !== 'object') return [x];
  // Avoid multiple concats by finding the size first. This is much faster
  function getSize(y) {
    if (typeof y !== 'object') {
      return 1;
    } else {
      return sum(y.map(getSize));
    }
  }
  var size = getSize(x);
  var ret = new Array(size);
  var index = 0;
  function add(y) {
    for (var i = 0; i < y.length; i++) {
      if (typeof y[i] !== 'object') {
        ret[index++] = y[i];
      } else {
        add(y[i]);
      }
    }
  }
  add(x);
  assert(index == size);
  return ret;
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

function numberedSet() {
  var args = typeof arguments[0] === 'object' ? arguments[0] : arguments;
  var ret = {};
  for (var i = 0; i < args.length; i++) {
    ret[args[i]] = i;
  }
  return ret;
}

function setSub(x, y) {
  var ret = set(keys(x));
  for (yy in y) {
    if (yy in ret) {
      delete ret[yy];
    }
  }
  return ret;
}

// Intersection of 2 sets. Faster if |xx| << |yy|
function setIntersect(x, y) {
  var ret = {};
  for (xx in x) {
    if (xx in y) {
      ret[xx] = true;
    }
  }
  return ret;
}

function invertArray(x) {
  var ret = {};
  for (var i = 0; i < x.length; i++) {
    ret[x[i]] = i;
  }
  return ret;
}

function copy(x) {
  return JSON.parse(JSON.stringify(x));
}

function jsonCompare(x, y) {
  return JSON.stringify(x) == JSON.stringify(y);
}

function stringifyWithFunctions(obj) {
  if (typeof obj === 'function') return obj.toString();
  if (obj === null || typeof obj !== 'object') return JSON.stringify(obj);
  if (isArray(obj)) {
    return '[' + obj.map(stringifyWithFunctions).join(',') + ']';
  } else {
    return '{' + keys(obj).map(function(key) { return key + ':' + stringifyWithFunctions(obj[key]) }).join(',') + '}';
  }
}

function sleep(secs) {
  var start = Date.now();
  while (Date.now() - start < secs*1000) {};
}

function log2(x) {
  return Math.log(x)/Math.LN2;
}

function isPowerOfTwo(x) {
  return x > 0 && ((x & (x-1)) == 0);
}

function ceilPowerOfTwo(x) {
  var ret = 1;
  while (ret < x) ret <<= 1;
  return ret;
}

function Benchmarker() {
  var starts = {}, times = {}, counts = {};
  this.start = function(id) {
    //printErr(['+', id, starts[id]]);
    starts[id] = (starts[id] || []).concat([Date.now()]);
  };
  this.stop = function(id) {
    //printErr(['-', id, starts[id]]);
    assert(starts[id], new Error().stack);
    times[id] = (times[id] || 0) + Date.now() - starts[id].pop();
    counts[id] = (counts[id] || 0) + 1;
    this.print();
  };
  this.print = function() {
    var ids = keys(times);
    ids.sort(function(a, b) { return times[b] - times[a] });
    printErr('times: \n' + ids.map(function(id) { return id + ' : ' + counts[id] + ' times, ' + times[id] + ' ms' }).join('\n'));
  };
};

