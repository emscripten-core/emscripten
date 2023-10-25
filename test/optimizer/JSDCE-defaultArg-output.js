var usedAsDefaultArg = 42;

function g(a, b = usedAsDefaultArg) {
 return a + b + 1;
}

Module["g"] = g;
