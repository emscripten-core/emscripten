function foo(a, b = undefined, c = false, d = undefined) {
  return a + b + c + d;
}

var bar = (a, b = undefined, c = 123) => a + b + c;

var single = (a = undefined) => a;

var obj = {
  method(a, b = undefined, c = undefined) {
    return a + b + c;
  },
};

function destruct({a = undefined, b = 1} = undefined, [c = undefined, d = 2] = undefined) {
  return a + b + c + d;
}

var x = undefined;
x = undefined;
