#include <emscripten/bind.h>

using namespace emscripten;

struct Base {
    virtual ~Base() {}
};

struct Sub : Base {
    Sub(Base* b) {}
};

struct SubWrapper : wrapper<Sub> {
    EMSCRIPTEN_WRAPPER(SubWrapper);
};

EMSCRIPTEN_BINDINGS(test) {
    class_<Base>("Base")
        .constructor<>()
        ;

    class_<Sub>("Sub")
        .allow_subclass<SubWrapper>("SubWrapper", constructor<Base*>(), allow_raw_pointer<arg<0>>())
        ;
}

int main() {
    return 0;
}
