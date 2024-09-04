var foo = 1;
var baz = {};

// Verify that shorthand object literals are preserved.
var bar = {
  foo,
  baz,
};

// Objects that don't use shorthand should also be preserved, as-is
var bar2 = {
  foo: foo,
  baz: baz,
};
