#include <stdio.h>
#include <emscripten.h>

#include <stdexcept>
#include <iostream>

using namespace std;

/* random_bool */
#include <stdlib.h>
#include <time.h>

bool get_random_bool()
{
    srand(time(NULL));
    int rand1to10 = rand() % 10 + 1;

    return (rand1to10 % 2 == 0);
}


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
static int incX(int x) { return ++x; }
static int decX(int x) { return --x; }

int test_fn_ptr_calls()
{
    typedef int (*FP)(int);
    FP f = get_random_bool() ? incX : decX;
    int result = f(40);

    return (result<40)?result+1:result-1;
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
    printf("Test fnptr calling: %d\n", test_fn_ptr_calls());
    printf("Test accessing global var: %d == %d\n", test_golbal_vars(), g_globalVar);
    printf("Test js_call: %d\n", js_call(23, 27));
}
