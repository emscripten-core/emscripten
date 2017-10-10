mergeInto(LibraryManager.library, {
  $foreignCounter: 0,
  foreignFunctionThatTakesThreeParameters__deps: ['$foreignCounter'],
  foreignFunctionThatTakesThreeParameters: function(a, b, c) {
    foreignCounter += a + b + c;
    return foreignCounter;
  }
});
