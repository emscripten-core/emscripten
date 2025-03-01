#include "assert.h"
#include "stdio.h"

int __attribute__((import_module("env"), import_name("js_fun"))) js_func(int x, int y);

int main() {
    int x = 7;
    int y = 9;
    printf("Calling js_func(%d, %d);\n", x, y);
    int res = js_func(x, y);
    assert(res == 16);
    return 0;
}
