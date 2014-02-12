function collideLocal(x) {
 x = x | 0;
 var a = 0;
 a = x*x;
 aGlobal(a); // aGlobal needs to be minified into a, but a is used!
 bGlobal(x);
}
function collideLocal(x) {
 x = x | 0;
 var i1 = 0;
 i1 = x*x;
 aGlobal(i1);
 bGlobal(x); // bGlobal needs to be minified into i1, but i1 is used!
}
function collideLocal(a) {
 a = a | 0;
 var x = 0;
 x = a*a;
 aGlobal(x); // aGlobal needs to be minified into a, but a is used by a param!
 bGlobal(a);
}
function collideLocal(i1) {
 i1 = i1 | 0;
 var x = 0;
 x = i1*i1;
 aGlobal(x);
 bGlobal(i1); // bGlobal needs to be minified into i1, but i1 is used by a param!
}
function collideLocal(i1) {
 i1 = i1 | 0;
 var a = 0;
 a = i1*i1;
 aGlobal(a); // multiple collisions, a and i1
 bGlobal(i1);
}
function collideLocal(i1) {
 i1 = i1 | 0;
 i1 = i1 + 4; // statement is of similar shape to a param coercion
 aGlobal(i1);
}
// EMSCRIPTEN_GENERATED_FUNCTIONS
// EXTRA_INFO: { "names": ["a", "b", "c", "d", "e", "f", "g", "h", "i", "i1", "cl"], "globals": { "aGlobal": "a", "bGlobal": "i1", "collideLocal": "cl" } }
