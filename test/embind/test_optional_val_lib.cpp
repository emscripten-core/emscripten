#include <emscripten/val.h>
#include <optional>
#include <string>

// This file deliberately does NOT include <emscripten/bind.h>

class MyType {
public:
    void RunCallback(emscripten::val callback);
};

void MyType::RunCallback(emscripten::val cb) {
    cb(std::make_optional(std::string{"Hey"}));
}
