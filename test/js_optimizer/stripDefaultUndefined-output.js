function foo(a, b, c = false, d) {
  return a + b + c + d;
}

var bar = (a, b, c = 123) => a + b + c;

var single = a => a;

var obj = {
  method(a, b, c) {
    return a + b + c;
  }
};

function destruct({a, b = 1}, [c, d = 2]) {
  return a + b + c + d;
}

var x = undefined;

x = undefined;
