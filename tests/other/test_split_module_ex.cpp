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

/* MAIN */
int main() {
    printf("Test exception handling: %d\n", test_exception_handling());
    printf("Test fnptr calling: %d\n", test_fn_ptr_calls());
    printf("Test accessing global var: %d == %d\n", test_golbal_vars(), g_globalVar);
}
