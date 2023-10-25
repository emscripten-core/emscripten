var usedAsDefaultArg = 42;
var notUsed = 43;

// exported
function g(a, b = usedAsDefaultArg) {
  return a+b+1;
}

Module['g'] = g;
