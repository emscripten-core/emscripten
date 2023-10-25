#include "emscripten/em_js.h"


EM_JS(__externref_t, get_ref, (), {
    return {a: 7, b: 9};
});

EM_JS(void, modify_ref, (__externref_t arg), {
    arg.a += 3;
    arg.b -= 3;
});

EM_JS(void, log_ref, (__externref_t arg), {
    console.log(arg);
});

int main() {
    __externref_t a = get_ref();
    log_ref(a);
    modify_ref(a);
    log_ref(a);
}
