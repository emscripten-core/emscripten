"use strict";

var z = fleefl();

var zz = fleefl();

function g(a) {
  return a + 1;
}

Module["g"] = g;

function h(a) {
  return a + 1;
}

print(h(123));

(function() {
  var z = fleefl();
  var zz = fleefl();
  function g(a) {
    return a + 1;
  }
  Module["g"] = g;
  function hh(a) {
    return a + 1;
  }
  print(hh(123));
})();

function glue() {
}

glue();

new SomethingUnknownWithSideEffects("utf8");

new TextDecoder(Unknown());
