function leaveMeAlone(c) {
}
function replaceMe(a, b) {
}
function a(x, y) {
  replaceMe(H_BASE + 1, F_BASE + 2);
  replaceMe(H_BASE + 1 | 0, F_BASE + 2 | 0);
  leaveMeAlone(F_BASE + x, H_BASE + y);
  leaveMeAlone(F_BASE + x | 0, H_BASE + y | 0);
}
// EMSCRIPTEN_GENERATED_FUNCTIONS
// EXTRA_INFO: { "replacements": { "replaceMe": "fixed" }, "hBase": 33, "fBase": 10 }
