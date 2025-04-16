"use strict";

// all unused
var x;
var y = 1;
var z = fleefl();
var xx, yy = 1, zz = fleefl(); // but zz must remain due to the side effects in the value
function f(x, y, z) {
  // shadow the x,y,z
  x = y;
  y = z;
}

// exported
function g(a) {
  return a+1;
}
Module['g'] = g;

// used
function h(a) {
  var t; // unused
  return a+1;
}
print(h(123));

// inner workings
(function() {
  var x;
  var y = 1;
  var z = fleefl();
  var xx, yy = 1, zz = fleefl();
  function f(x, y, z) {
    // shadow the x,y,z
    x = y;
    y = z;
  }

  // exported
  function g(a) {
    return a+1;
  }
  Module['g'] = g;

  // used
  function hh(a) {
    var t; // unused
    return a+1;
  }
  print(hh(123));
})();

function glue() {
  function lookup() { // 2 passes needed for this one
    throw 1;
  }
  function removable() { // first remove this
    lookup();
  }
}
glue();

var buffer = new ArrayBuffer(1024);

// unnecessary leftovers that seem to have side effects
"undefined" !== typeof TextDecoder && new TextDecoder("utf8");
new TextDecoder("utf8");
new Int8Array(buffer);
new Uint8Array(buffer);
new Int16Array(buffer);
new Uint16Array(buffer);
new Int32Array(buffer);
new Uint32Array(buffer);
new Float32Array(buffer);
new Float64Array(buffer);

// for comparison, real side effects
new SomethingUnknownWithSideEffects("utf8");
new TextDecoder(Unknown());

