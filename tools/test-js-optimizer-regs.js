function test() {
  var i = 0;
  f(i);
  i++;
  var j = i + 2;
  g(i, j);
  f(i);
  var i2 = cheez();
  var j2 = i2 + 2;
  g(j2, j2);
  var k1 = 200;
  var k2 = 203;
  var k3 = 205;
  var k4 = 208;
  c(k3);
  while (f()) {
    var apple = 5;
    var orange = 12;
    gg(apple, orange*2);
    var tangerine = 100;
    gg(tangerine, 20);
  }
  var ck = f(), ck2 = 100, ck3 = 1000, ck4 = 100000;
  f(ck());
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["test"]
