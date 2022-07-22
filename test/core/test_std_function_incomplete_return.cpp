#include <iostream>
#include <functional>
struct foo;
using fnc = std::function<foo(int)>;
struct bar {
    bar() {
        std::cout << "construct me" << std::endl;
    }
    fnc _f{nullptr};
};
int main(int argc, const char * argv[]) {
    std::cout << "Hello, World!\n";
    bar b{};
    return 0;
}
