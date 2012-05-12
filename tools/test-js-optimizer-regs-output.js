function test() {
  var r1, r2, r3;
  r1 = 0;
  f(r1);
  r1++;
  r2 = r1 + 2;
  g(r1, r2);
  f(r1);
  r1 = cheez();
  r2 = r1 + 2;
  g(r2, r2);
  r2 = 200;
  r2 = 203;
  r2 = 205;
  r1 = 208;
  c(r2);
  while (f()) {
    r2 = 5;
    r1 = 12;
    gg(r2, r1 * 2);
    r3 = 100;
    gg(r3, 20);
  }
  r3 = f(), r1 = 100, r1 = 1e3, r1 = 1e5;
  f(r3());
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["test"]
