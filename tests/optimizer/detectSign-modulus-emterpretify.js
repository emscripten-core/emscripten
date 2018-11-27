// int main(int argc, char* argv[]) { return (fmod(argc, 2) == 1); }
function modulus(i1) {
 i1 = i1 | 0;
 return +(i1 | 0) % +2 == +1 | 0;
}
function not1(i1) {
 i1 = i1 | 0;
 return (!i1 < (i1 | 0)) | 0;
}
function not2(i1) {
 i1 = i1 | 0;
 return (!i1 < (i1 >>> 0)) | 0;
}
function not3(i1) {
 i1 = i1 | 0;
 return ((i1 | 0) < !i1) | 0;
}
function not4(i1) {
 i1 = i1 | 0;
 return ((i1 >>> 0) < !i1) | 0;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS
// EXTRA_INFO:{"emterpretedFuncs": ["modulus", "not1", "not2", "not3", "not4"]}
