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
function labels() {
  L1: while (1) {
    L2: while (1) {
      if (foo()) {
        if (bar()) {
          break L1;
        } else {
          continue L2;
        }
      } else {
        if (bar()) {
          break;
        } else {
          continue;
        }
      }
    }
  }
  // Another loop with the same name
  L1: while (1) {
  }
}
// EXTRA_INFO: { "globals": { "main": "a", "constructor": "b", "foo": "c", "bar": "d", "labels": "l" } }
