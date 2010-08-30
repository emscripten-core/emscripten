
function dump(item) {
  try {
    return JSON.stringify(item).substr(0,200);
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
    return ret.join(', ').substr(0,200);
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

function searchable() {
  var ret = {};
  for (var i = 0; i < arguments.length; i++) {
    ret[arguments[i]] = 0;
  }
  return ret;
}

function walkJSON(item, func) {
  func(item);
  for (x in item) {
    if (typeof item[x] === 'object') {
      walkJSON(item[x], func);
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

