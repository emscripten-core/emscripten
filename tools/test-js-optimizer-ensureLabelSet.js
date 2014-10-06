function noNeed() {
 var x = 0;
 f(x);
}
function need() {
 var x = 0, label = 0;
 f(x);
 if (y) {
  label = 0;
 } else {
  label = 1;
 }
 g(label);
}
function noNeed2() {
 var x = 0, label = 0;
 f(x);
 label = 0;
 if (y) {
  label = 1;
 }
 g(label);
}
function need2() {
 var x = 0, label = 0;
 f(x);
 waka: if (y) {
  label = 0;
 } else {
  label = 1;
 }
 g(label);
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["noNeed", "need", "noNeed2", "need2"]
