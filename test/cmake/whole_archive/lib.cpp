#include <iostream>

static void init(void) __attribute__((constructor));
static void init(void) {
    std::cout << "init" << std::endl;
}
