#include <stdio.h>
#include <emscripten.h>
#include <bind.h>
#include <memory>

int counter = 0;

extern "C" {

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
extern void move_gameobjects_benchmark_embind_js();

extern void pass_gameobject_ptr_benchmark();
extern void pass_gameobject_ptr_benchmark_embind_js();

extern void call_through_interface0();
extern void call_through_interface1();
extern void call_through_interface2();

extern void returns_val_benchmark();
}

emscripten::val returns_val(emscripten::val value)
{
    return emscripten::val(value.as<unsigned>() + 1);
}

class Vec3
{
public:
    Vec3():x(0),y(0),z(0) {}
    Vec3(float x_, float y_, float z_):x(x_),y(y_),z(z_) {}
    float x,y,z;
};

Vec3 add(const Vec3 &lhs, const Vec3 &rhs) { return Vec3(lhs.x+rhs.x, lhs.y+rhs.y, lhs.z+rhs.z); }

class Transform
{
public:
    Transform():scale(1) {}

    Vec3 pos;
    Vec3 rot;
    float scale;

    Vec3 __attribute__((noinline)) GetPosition() const { return pos; }
    Vec3 __attribute__((noinline)) GetRotation() const { return rot; }
    float __attribute__((noinline)) GetScale() const { return scale; }
    
    void __attribute__((noinline)) SetPosition(const Vec3 &pos_) { pos = pos_; }
    void __attribute__((noinline)) SetRotation(const Vec3 &rot_) { rot = rot_; }
    void __attribute__((noinline)) SetScale(float scale_) { scale = scale_; }
};
typedef std::shared_ptr<Transform> TransformPtr;

class GameObject
{
public:
    GameObject()
    {
        transform = std::make_shared<Transform>();
    }
    std::shared_ptr<Transform> transform;
    
    TransformPtr __attribute__((noinline)) GetTransform() const { return transform; }
};
typedef std::shared_ptr<GameObject> GameObjectPtr;

GameObjectPtr create_game_object()
{
    return std::make_shared<GameObject>();
}

GameObjectPtr __attribute__((noinline)) pass_gameobject_ptr(GameObjectPtr p)
{
    return p;
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

class Interface
{
public:
    virtual void call0() = 0;
    virtual std::wstring call1(const std::wstring& str1, const std::wstring& str2) = 0;
    virtual void call_with_typed_array(size_t size, const void*) = 0;
    virtual void call_with_memory_view(size_t size, const void*) = 0;
};

EMSCRIPTEN_SYMBOL(HEAP8);
EMSCRIPTEN_SYMBOL(buffer);

EMSCRIPTEN_SYMBOL(call0);
EMSCRIPTEN_SYMBOL(call1);
EMSCRIPTEN_SYMBOL(call_with_typed_array);
EMSCRIPTEN_SYMBOL(call_with_memory_view);
EMSCRIPTEN_SYMBOL(Uint8Array);

class InterfaceWrapper : public emscripten::wrapper<Interface>
{
public:
    EMSCRIPTEN_WRAPPER(InterfaceWrapper);

    void call0() override {
        return call<void>(call0_symbol);
    }

    std::wstring call1(const std::wstring& str1, const std::wstring& str2) {
        return call<std::wstring>(call1_symbol, str1, str2);
    }

    void call_with_typed_array(size_t size, const void* data) {
        return call<void>(
            call_with_typed_array_symbol,
            emscripten::val::global(Uint8Array_symbol).new_(
                emscripten::val::module_property(HEAP8_symbol)[buffer_symbol],
                reinterpret_cast<uintptr_t>(data),
                size));
    }

    void call_with_memory_view(size_t size, const void* data) {
        return call<void>(
            call_with_memory_view_symbol,
            emscripten::typed_memory_view(size, data));
    }
};

void callInterface0(unsigned N, Interface& o) {
    for (unsigned i = 0; i < N; i += 8) {
        o.call0();
        o.call0();
        o.call0();
        o.call0();
        o.call0();
        o.call0();
        o.call0();
        o.call0();
    }
}

void callInterface1(unsigned N, Interface& o) {
    static std::wstring foo(L"foo");
    static std::wstring bar(L"bar");
    static std::wstring baz(L"baz");
    static std::wstring qux(L"qux");
    for (unsigned i = 0; i < N; i += 7) {
        o.call1(
            o.call1(
                o.call1(foo, bar),
                o.call1(baz, qux)),
            o.call1(
                o.call1(qux, foo),
                o.call1(bar, baz)));
    }
}

void callInterface2(unsigned N, Interface& o) {
    int i = 0;
    for (unsigned i = 0; i < N; i += 8) {
        o.call_with_typed_array(sizeof(int), &i);
        o.call_with_typed_array(sizeof(int), &i);
        o.call_with_typed_array(sizeof(int), &i);
        o.call_with_typed_array(sizeof(int), &i);
        o.call_with_typed_array(sizeof(int), &i);
        o.call_with_typed_array(sizeof(int), &i);
        o.call_with_typed_array(sizeof(int), &i);
        o.call_with_typed_array(sizeof(int), &i);
    }
}

void callInterface3(unsigned N, Interface& o) {
    for (unsigned i = 0; i < N; i += 8) {
        o.call_with_memory_view(sizeof(int), &i);
        o.call_with_memory_view(sizeof(int), &i);
        o.call_with_memory_view(sizeof(int), &i);
        o.call_with_memory_view(sizeof(int), &i);
        o.call_with_memory_view(sizeof(int), &i);
        o.call_with_memory_view(sizeof(int), &i);
        o.call_with_memory_view(sizeof(int), &i);
        o.call_with_memory_view(sizeof(int), &i);
    }
}

EMSCRIPTEN_BINDINGS(benchmark)
{
    using namespace emscripten;
    
    class_<GameObject>("GameObject")
        .smart_ptr<GameObjectPtr>("GameObjectPtr")
        .function("GetTransform", &GameObject::GetTransform);
        
    class_<Transform>("Transform")
        .smart_ptr<TransformPtr>("TransformPtr")
        .function("GetPosition", &Transform::GetPosition)
        .function("GetRotation", &Transform::GetRotation)
        .function("GetScale", &Transform::GetScale)
        .function("SetPosition", &Transform::SetPosition)
        .function("SetRotation", &Transform::SetRotation)
        .function("SetScale", &Transform::SetScale);
        
    value_array<Vec3>("Vec3")
        .element(&Vec3::x)
        .element(&Vec3::y)
        .element(&Vec3::z);
        
    function("create_game_object", &create_game_object);
    function("pass_gameobject_ptr", &pass_gameobject_ptr);
    function("add", &add);
    
    function("get_counter", &get_counter);
    function("increment_counter", &increment_counter);
    function("returns_input", &returns_input);
    function("sum_int", &sum_int);
    function("sum_float", &sum_float);
    
    class_<Foo>("Foo")
        .constructor<>()
        .function("incr_global_counter", &Foo::incr_global_counter)
        .function("incr_class_counter", &Foo::incr_class_counter)
        .function("class_counter_val", &Foo::class_counter_val)
        ;

    class_<Interface>("Interface")
        .allow_subclass<InterfaceWrapper>("InterfaceWrapper")
        ;

    function("callInterface0", &callInterface0);
    function("callInterface1", &callInterface1);
    function("callInterface2", &callInterface2);
    function("callInterface3", &callInterface3);

    function("returns_val", &returns_val);
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
    printf("C++ emscripten_get_now %d iters: %f msecs.\n", N, (t2-t));
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
    printf("C++ increment_counter %d iters: %f msecs.\n", N, (t2-t));
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
    printf("C++ increment_class_counter %d iters: %f msecs. result: %d\n", N, (t2-t), foo.class_counter);
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
    printf("C++ returns_input 100000 iters: %f msecs.\n", (t2-t));
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
    printf("C++ sum_int 100000 iters: %f msecs.\n", (t2-t));
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
    printf("C++ sum_float 100000 iters: %f msecs.\n", (t2-t));
}

void __attribute__((noinline)) move_gameobjects_benchmark()
{
    const int N = 10000;
    GameObjectPtr objects[N];
    for(int i = 0; i < N; ++i)
        objects[i] = create_game_object();
    
    volatile float t = emscripten_get_now();
    for(int i = 0; i < N; ++i)
    {
        TransformPtr t = objects[i]->GetTransform();
        Vec3 pos = add(t->GetPosition(), Vec3(2.f, 0.f, 1.f));
        Vec3 rot = add(t->GetRotation(), Vec3(0.1f, 0.2f, 0.3f));
        t->SetPosition(pos);
        t->SetRotation(rot);
    }
    volatile float t2 = emscripten_get_now();

    Vec3 accum;
    for(int i = 0; i < N; ++i)
        accum = add(add(accum, objects[i]->GetTransform()->GetPosition()), objects[i]->GetTransform()->GetRotation());
    printf("C++ move_gameobjects %d iters: %f msecs. Result: %f\n", N, (t2-t), accum.x+accum.y+accum.z);
}

void __attribute__((noinline)) pass_gameobject_ptr_benchmark()
{
    const int N = 100000;
    GameObjectPtr objects[N];
    for(int i = 0; i < N; ++i)
        objects[i] = create_game_object();
    
    volatile float t = emscripten_get_now();
    for(int i = 0; i < N; ++i)
    {
        objects[i] = pass_gameobject_ptr(objects[i]);
    }
    volatile float t2 = emscripten_get_now();

    printf("C++ pass_gameobject_ptr %d iters: %f msecs.\n", N, (t2-t));
}

int main()
{
    /*
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
    printf("\n");
    move_gameobjects_benchmark();
    move_gameobjects_benchmark_embind_js();
    printf("\n");
    pass_gameobject_ptr_benchmark();
    pass_gameobject_ptr_benchmark_embind_js();
    */
    emscripten_get_now();
    call_through_interface0();
    call_through_interface1();
    call_through_interface2();
    returns_val_benchmark();
}
