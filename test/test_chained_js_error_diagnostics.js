addToLibrary({
  bar__deps: ['nonexistent_function'],
  bar: function() {},

  foo__deps: ['bar'],
  foo: function() {},
});
