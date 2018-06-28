// int main(int argc, char* argv[]) { return (fmod(argc, 2) == 1); }
function _main(i1, i2) {
 i1 = i1 | 0;
 i2 = i2 | 0;
 return +(i1 | 0) % +2 == +1 | 0;
}
// EMSCRIPTEN_GENERATED_FUNCTIONS
// EXTRA_INFO:{"emterpretedFuncs": ["_main"]}
