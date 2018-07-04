// int main(int argc, char* argv[]) { return (fmod(argc, 2) == 1); }
function modulus(i1) {
 i1 = i1 | 0;
 return +(i1 | 0) % +2 == +1 | 0;
}
function not(i1) {
 i1 = i1 | 0;
 return (!i1 == 5) | 0;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS
// EXTRA_INFO:{"emterpretedFuncs": ["modulus", "not"]}
