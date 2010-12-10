// General JS utilities

function safeQuote(x) {
  return x.replace(/"/g, '\\"')
          .replace(/'/g, "\\'");
}

function dump(item) {
  var CHUNK = 500;
  function lineify(text) {
    var ret = '';
    while (text.length > 80) {
      ret += '// ' + text.substr(0,80) + '\n';
      text = text.substr(80);
    }
    return ret + '// ' + text;
  }
  try {
    return lineify(JSON.stringify(item));
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
    return lineify(ret.join(', '));
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
    print("Stack: " + new Error().stack);
    throw "Should have been equal: " + a + " : " + b;
  }
}

function assertTrue(a) {
  assertEq(!!a, true);
}
assert = assertTrue;

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

function searchable() {
  if (typeof arguments[0] === 'object') arguments = arguments[0];
  var ret = {};
  for (var i = 0; i < arguments.length; i++) {
    ret[arguments[i]] = 0;
  }
  return ret;
}

function walkJSON(item, func) {
  if (item.length) {
    for (var x = 0; x < item.length; x++) {
      var y = item[x];
      if (y && typeof y === 'object') {
        walkJSON(y, func);
      }
    }
  } else {
    func(item);
    for (x in item) {
      var y = item[x];
      if (y && typeof y === 'object') {
        walkJSON(y, func);
      }
    }
  }
}

function keys(x) {
  var ret = [];
  for (a in x) ret.push(a);
  return ret;
}

function values(x) {
  var ret = [];
  for (a in x) ret.push(x[a]);
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
DPRINT_INDENT = '';
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
  print(text);
}

PROF_ORIGIN = Date.now();
PROF_TIME = PROF_ORIGIN;
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

// Sets

set = searchable; // Create a 'set'
function setSub(x, y) {
  var ret = set(values(x));
  for (yy in y) {
    if (yy in ret) {
      delete ret.yy;
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

function isNumber(x) {
  return x == parseFloat(x);
}

function flatten(x) {
  if (typeof x !== 'object') return x;
  var ret = [];
  for (var i = 0; i < x.length; i++) {
    ret = ret.concat(flatten(x[i]));
  }
  return ret;
}

