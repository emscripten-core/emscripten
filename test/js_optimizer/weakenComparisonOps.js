if (typeof f === 'function') {
  f();
}

if (typeof f !== 'undefined') {
  f();
}

if ('string' === typeof s) {
  foo(s);
}

if ('object' !== typeof o) {
  bar(o);
}

if (typeof f === typeof g) {
  baz();
}

var a = typeof x.y === 'number' ? 1 : 2;

// Numeric operations with known number primitives
if ((x & 10) === 10) {
  foo();
}

if ((x | 0) === 0) {
  foo();
}

if ((x & 1) !== 0) {
  foo();
}

if ((x >>> 0) === 0) {
  foo();
}

if (x % 4 === 0) {
  foo();
}

if (+x === 0) {
  foo();
}

if (-1 === -1) {
  foo();
}

if (-1n === -1n) {
  foo();
}

// Boolean operations
if (!a === true) {
  foo();
}

if (!a === !b) {
  foo();
}

if ((a < b) === (c < d)) {
  foo();
}

// Non-weakened comparisons: types are not known to be identical primitives
if (f === 'function') {
  f();
}

if (typeof f === x) {
  f();
}

if (x === null) {
  f();
}

if (x !== null) {
  f();
}

if (x === undefined) {
  f();
}

if (x !== undefined) {
  f();
}

if (x === 0) {
  f();
}

if (x === true) {
  f();
}

if ((a + b) === 0) {
  f();
}

if (a !== b) {
  foo();
}

if ((x - y) === 0) {
  foo();
}

if (~x !== -1) {
  foo();
}

if (-1n === -1) {
  f();
}

if ((-0n) === 0) {
  f();
}

if ((~0n) === -1) {
  f();
}

if ((-x) === 0) {
  f();
}
