#include <stdio.h>
#include <emscripten/bind.h>

void print_some_stuff(int a, float b, const std::string& s) {
    printf("print_some_stuff: %d, %f, %s\n", a, b, s.c_str());
}

EMSCRIPTEN_BINDINGS(foo) {
    emscripten::function("print_some_stuff", &print_some_stuff);
}

