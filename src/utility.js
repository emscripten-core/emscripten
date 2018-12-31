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

var _PROF_ORIGIN = Date.now();
var _PROF_TIME = _PROF_ORIGIN;
function PROF(pass) {
  if (!pass) {
    dprint("Profiling: " + ((Date.now() - _PROF_TIME)/1000) + ' seconds, total: ' + ((Date.now() - _PROF_ORIGIN)/1000));
  }
  PROF_TIME = Date.now();
}

// Usage: arrayOfArrays.reduce(concatenator, []);
function concatenator(x, y) {
  return x.concat(y);
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
  for (var yy in y) {
    if (yy in ret) {
      delete ret[yy];
    }
  }
  return ret;
}

// Intersection of 2 sets. Faster if |xx| << |yy|
function setIntersect(x, y) {
  var ret = {};
  for (var xx in x) {
    if (xx in y) {
      ret[xx] = 0;
    }
  }
  return ret;
}

function setUnion(x, y) {
  var ret = set(keys(x));
  for (var yy in y) {
    ret[yy] = 0;
  }
  return ret;
}

function setSize(x) {
  var ret = 0;
  for (var xx in x) ret++;
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

function sortedJsonCompare(x, y) {
  if (x === null || typeof x !== 'object') return x === y;
  for (var i in x) {
    if (!sortedJsonCompare(x[i], y[i])) return false;
  }
  for (var i in y) {
    if (!sortedJsonCompare(x[i], y[i])) return false;
  }
  return true;
}

function escapeJSONKey(x) {
  if (/^[\d\w_]+$/.exec(x) || x[0] === '"' || x[0] === "'") return x;
  assert(x.indexOf("'") < 0, 'cannot have internal single quotes in keys: ' + x);
  return "'" + x + "'";
}

function stringifyWithFunctions(obj) {
  if (typeof obj === 'function') return obj.toString();
  if (obj === null || typeof obj !== 'object') return JSON.stringify(obj);
  if (isArray(obj)) {
    return '[' + obj.map(stringifyWithFunctions).join(',') + ']';
  } else {
    return '{' + keys(obj).map(function(key) { return escapeJSONKey(key) + ':' + stringifyWithFunctions(obj[key]) }).join(',') + '}';
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
};

