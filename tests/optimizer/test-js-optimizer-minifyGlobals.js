// Ignore the first (used internally).
var first;
// A first definition.
for (var i = 25; i >= 0; --i) {
  foo(i);
}
// Another definition, which we need to minify to the same thing.
for (var i = 25; i >= 0; --i) {
  foo(i);
}
// EXTRA_INFO: { "globals": [] }

