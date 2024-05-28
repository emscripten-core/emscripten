#include <emscripten/bind.h>
#include <string>

void fn1(int x) {}
void fn2(char x) {}
void fn3(std::string x) {}

EMSCRIPTEN_BINDINGS(Test) {
    EMSCRIPTEN_FUNCTION("fn1_1", &fn1, ());
    EMSCRIPTEN_FUNCTION("fn1_2", &fn1, (char x));
    EMSCRIPTEN_FUNCTION("fn1_3", &fn1, (std::string x));

    EMSCRIPTEN_FUNCTION("fn2_1", &fn2, ());
    EMSCRIPTEN_FUNCTION("fn2_2", &fn2, (int x));
    EMSCRIPTEN_FUNCTION("fn2_3", &fn2, (std::string x));

    EMSCRIPTEN_FUNCTION("fn3_1", &fn3, ());
    EMSCRIPTEN_FUNCTION("fn3_2", &fn3, (char x));
    EMSCRIPTEN_FUNCTION("fn3_3", &fn3, (int x));
}
