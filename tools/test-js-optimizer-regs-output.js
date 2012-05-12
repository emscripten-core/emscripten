function test() {
  var r1 = 0;
  f(r1);
  r1++;
  var r2 = r1 + 2;
  g(r1, r2);
  f(r1);
  var r1 = cheez();
  var r2 = r1 + 2;
  g(r2, r2);
  var r2 = 200;
  var r2 = 203;
  var r2 = 205;
  var r1 = 208;
  c(r2);
  while (f()) {
    var r2 = 5;
    var r1 = 12;
    gg(r2, r1 * 2);
    var r3 = 100;
    gg(r3, 20);
  }
  var r3 = f(), r1 = 100, r1 = 1e3, r1 = 1e5;
  f(r3());
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["test"]
