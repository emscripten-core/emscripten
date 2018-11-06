
// all unused
var x;
var y = 1;
var z = fleefl();
var xx, yy = 1, zz = fleefl(); // but zz must remain due to the side effects in the value
var obj = { x: 5 };
var keeperObj = { x: fleefl() };
var array = [1, 2, '3', function() { ignoreInsideIt() }];
var keeperArray = [1, 2, '3', four()];
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
// gl emulation style code
function _glCreateShader() {
 return 1;
}
function emulate() {
  var saved = _glCreateShader;
  _glCreateShader = function _glCreateShader(shaderType) { // the name here is just for show in stack traces!
    return glCreateShader();
  };
}
emulate();
// has only a self-reference
function ___cxa_find_matching_catch() {
 if (!___cxa_find_matching_catch.buffer) ___cxa_find_matching_catch.buffer = {};
}
// use before
___cxa_find_matching_catch_before();
function ___cxa_find_matching_catch_before() {
 if (!___cxa_find_matching_catch_before.buffer) ___cxa_find_matching_catch_before.buffer = {};
}
// use after
function ___cxa_find_matching_catch_after() {
 if (!___cxa_find_matching_catch_after.buffer) ___cxa_find_matching_catch_after.buffer = {};
}
___cxa_find_matching_catch_after();
// dot stuff
var dotMath = Math.something;
var dotOther = Side.effect;

