var foo = 1;
var baz = {};

// Verify that shorthand object literals are preserved.
var bar = {
  foo,
  baz,
};

// Objects that could use shorthand are also converted where possible.
var bar2 = {
  foo: foo,
  "baz": baz,
  bar: foo,
};
