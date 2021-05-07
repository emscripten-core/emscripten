#include <stdio.h>
#include <emscripten.h>

#include <stdexcept>
#include <iostream>

using namespace std;

/* test_exception_handling */
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

/* test_auto_ptr_support */
struct Test_auto_ptr { int x = 1; };

int test_auto_ptr_support()
{
    std::auto_ptr<Test_auto_ptr> pTest_auto_ptr(new Test_auto_ptr);
    int retVal = pTest_auto_ptr.get()->x;
    pTest_auto_ptr.release();
    return retVal;
}

/* test_fn_ptr_calls */
static int incX(int x) { return ++x; } // hot
static int decX(int x) { return --x; } // cold

int test_fn_ptr_calls()
{
    typedef int (*FP)(int);
    FP f[2] = { incX, decX };
    return f[0](40);
}

/* test_golbal_vars */
int g_globalVar = 500;

int __attribute__((noinline)) test_golbal_vars()
{
    g_globalVar+=100;
    return g_globalVar;
}

/* test_js_call */
extern "C" {
    extern int js_call(int x, int y);
}


/* MAIN */
EMSCRIPTEN_KEEPALIVE void cpp_main() {
    printf("Test exception handling: %d\n", test_exception_handling());
    printf("Test c++17 std::auto_ptr support: %d\n", test_auto_ptr_support());
    printf("Test fnptr calling: %d\n", test_fn_ptr_calls());
    printf("Test accessing global var: %d == %d\n", test_golbal_vars(), g_globalVar);
    printf("Test js_call: %d\n", js_call(23, 27));
}
