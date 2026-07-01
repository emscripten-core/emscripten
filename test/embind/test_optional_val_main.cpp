#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <string>
#include <iostream>

using namespace emscripten;

class MyType {
public:
    void RunCallback(emscripten::val callback);
};

int main() {
    EM_ASM(
        let value = new Module.MyType();
        value.RunCallback((e) => {
            console.log("Received: " + e);
            if (e !== "Hey") throw "Expected 'Hey', got " + e;
        });
    );
    std::cout << "done" << std::endl;
}

EMSCRIPTEN_BINDINGS(my_module) {
    register_optional<std::string>();

    class_<MyType>("MyType")
        .constructor<>()
        .function("RunCallback", &MyType::RunCallback);
}
