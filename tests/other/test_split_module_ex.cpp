#include <stdio.h>
#include <emscripten.h>

#include <stdexcept>
#include <limits>
#include <iostream>

using namespace std;

void fn_throws_exception()
{
    throw invalid_argument("fn_throws_exception argument too large.");
}

int test_exception_handling()
{
    try
    {
        fn_throws_exception();
    }
    catch (invalid_argument& e)
    {
        cerr << e.what() << endl;
        return -1;
    }
    return 0;
}

EMSCRIPTEN_KEEPALIVE void cpp_main() {
    printf("Test exception handling: %d\n", test_exception_handling());
}
