#include <stdio.h>
#include <emscripten.h>
#include <bind.h>

int counter = 0;

extern "C"
{

int __attribute__((noinline)) get_counter()
{
    return counter;
}

void __attribute__((noinline)) increment_counter()
{
    ++counter;
}

int __attribute__((noinline)) sum_int(int v1, int v2, int v3, int v4, int v5, int v6, int v7, int v8, int v9)
{
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
}

float __attribute__((noinline)) sum_float(float v1, float v2, float v3, float v4, float v5, float v6, float v7, float v8, float v9)
{
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
}

int __attribute__((noinline)) returns_input(int i)
{
    return i;
}

extern void increment_counter_benchmark_js(int N);
extern void returns_input_benchmark_js();
extern void sum_int_benchmark_js();
extern void sum_float_benchmark_js();

extern void increment_counter_benchmark_embind_js(int N);
extern void returns_input_benchmark_embind_js();
extern void sum_int_benchmark_embind_js();
extern void sum_float_benchmark_embind_js();

extern void increment_class_counter_benchmark_embind_js(int N);
}

class Foo
{
public:
    Foo()
    :class_counter(0)
    {
    }

    void __attribute__((noinline)) incr_global_counter()
    {
        ++counter;
    }

    void __attribute__((noinline)) incr_class_counter()
    {
        ++class_counter;
    }

    int class_counter_val() const
    {
        return class_counter;
    }

    int class_counter;
};

EMSCRIPTEN_BINDINGS(benchmark)
{
    using namespace emscripten;
    
    function("get_counter", &get_counter);
    function("increment_counter", &increment_counter);
    function("returns_input", &returns_input);
    function("sum_int", &sum_int);
    function("sum_float", &sum_float);
    
    class_<Foo>("Foo")
        .constructor<>()
        .function("incr_global_counter", &Foo::incr_global_counter)
        .function("incr_class_counter", &Foo::incr_class_counter)
        .function("class_counter_val", &Foo::class_counter_val);
}

void __attribute__((noinline)) emscripten_get_now_benchmark(int N)
{
    volatile float t = emscripten_get_now();
    for(int i = 0; i < N; ++i)
    {
        emscripten_get_now();
        emscripten_get_now();
        emscripten_get_now();
        emscripten_get_now();
        emscripten_get_now();
        emscripten_get_now();
        emscripten_get_now();
        emscripten_get_now();
        emscripten_get_now();
        emscripten_get_now();
    }
    volatile float t2 = emscripten_get_now();
    printf("C++ emscripten_get_now %d iters: %f msecs.\n", N, 1000.f*(t2-t));
}

void __attribute__((noinline)) increment_counter_benchmark(int N)
{
    volatile float t = emscripten_get_now();
    for(int i = 0; i < N; ++i)
    {
        increment_counter();
        increment_counter();
        increment_counter();
        increment_counter();
        increment_counter();
        increment_counter();
        increment_counter();
        increment_counter();
        increment_counter();
        increment_counter();
    }
    volatile float t2 = emscripten_get_now();
    printf("C++ increment_counter %d iters: %f msecs.\n", N, 1000.f*(t2-t));
}

void __attribute__((noinline)) increment_class_counter_benchmark(int N)
{
    Foo foo;
    volatile float t = emscripten_get_now();
    for(int i = 0; i < N; ++i)
    {
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
        foo.incr_class_counter();
    }
    volatile float t2 = emscripten_get_now();
    printf("C++ increment_class_counter %d iters: %f msecs. result: %d\n", N, 1000.f*(t2-t), foo.class_counter);
}

void __attribute__((noinline)) returns_input_benchmark()
{
    volatile int r = 0;
    volatile float t = emscripten_get_now();
    for(int i = 0; i < 100000; ++i)
    {
        r += returns_input(i);
        r += returns_input(i);
        r += returns_input(i);
        r += returns_input(i);
        r += returns_input(i);
        r += returns_input(i);
        r += returns_input(i);
        r += returns_input(i);
        r += returns_input(i);
        r += returns_input(i);
    }
    volatile float t2 = emscripten_get_now();
    printf("C++ returns_input 100000 iters: %f msecs.\n", 1000.f*(t2-t));
}

void __attribute__((noinline)) sum_int_benchmark()
{
    volatile float t = emscripten_get_now();
    volatile int r = 0;
    for(int i = 0; i < 100000; ++i)
    {
        r += sum_int(i,2,3,4,5,6,7,8,9);
        r += sum_int(i,2,3,4,5,6,7,8,9);
        r += sum_int(i,2,3,4,5,6,7,8,9);
        r += sum_int(i,2,3,4,5,6,7,8,9);
        r += sum_int(i,2,3,4,5,6,7,8,9);
        r += sum_int(i,2,3,4,5,6,7,8,9);
        r += sum_int(i,2,3,4,5,6,7,8,9);
        r += sum_int(i,2,3,4,5,6,7,8,9);
        r += sum_int(i,2,3,4,5,6,7,8,9);
        r += sum_int(i,2,3,4,5,6,7,8,9);
    }
    volatile float t2 = emscripten_get_now();
    printf("C++ sum_int 100000 iters: %f msecs.\n", 1000.f*(t2-t));
}

void __attribute__((noinline)) sum_float_benchmark()
{
    volatile float f = 0.f;
    volatile float t = emscripten_get_now();
    for(int i = 0; i < 100000; ++i)
    {
        f += sum_float((float)i,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f);
        f += sum_float((float)i,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f);
        f += sum_float((float)i,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f);
        f += sum_float((float)i,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f);
        f += sum_float((float)i,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f);
        f += sum_float((float)i,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f);
        f += sum_float((float)i,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f);
        f += sum_float((float)i,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f);
        f += sum_float((float)i,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f);
        f += sum_float((float)i,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f);
    }
    volatile float t2 = emscripten_get_now();
    printf("C++ sum_float 100000 iters: %f msecs.\n", 1000.f*(t2-t));
}

int main()
{
    for(int i = 1000; i <= 100000; i *= 10)
        emscripten_get_now_benchmark(i);

    printf("\n");
    for(int i = 1000; i <= 100000; i *= 10)
    {
        increment_counter_benchmark(i);
        increment_counter_benchmark_js(i);
        increment_counter_benchmark_embind_js(i);
        printf("\n");
    }

    for(int i = 1000; i <= 100000; i *= 10)
    {
        increment_class_counter_benchmark(i);
        increment_class_counter_benchmark_embind_js(i);
        printf("\n");
    }

    returns_input_benchmark();
    returns_input_benchmark_js();
    returns_input_benchmark_embind_js();
    printf("\n");
    sum_int_benchmark();
    sum_int_benchmark_js();
    sum_int_benchmark_embind_js();
    printf("\n");
    sum_float_benchmark();
    sum_float_benchmark_js();
    sum_float_benchmark_embind_js();
}
