function main(x) {
 constructor(); // special word in JS, must not be confused by it
}
function constructor() {
}
function foo() {
  var bar = 10;
  return bar;
}
function bar(foo, a, b) {
  var main, c, d, e;
  main = foo;
  c = a;
  d = b;
  e = foo;
  return foo + a + b + main + c + d + e;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS
// EXTRA_INFO: { "globals": { "main": "a", "constructor": "b", "foo": "c", "bar": "d" } }
