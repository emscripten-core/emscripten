#include <string>
#include <malloc.h>
#include <emscripten/bind.h>

using namespace emscripten;

val emval_test_mallinfo() {
    const auto& i = mallinfo();
    val rv(val::object());
    rv.set("arena", val(i.arena));
    rv.set("ordblks", val(i.ordblks));
    rv.set("smblks", val(i.smblks));
    rv.set("hblks", val(i.hblks));
    rv.set("usmblks", val(i.usmblks));
    rv.set("fsmblks", val(i.fsmblks));
    rv.set("uordblks", val(i.uordblks));
    rv.set("fordblks", val(i.fordblks));
    rv.set("keepcost", val(i.keepcost));
    return rv;
}

val emval_test_new_integer() {
    return val(15);
}

val emval_test_new_string() {
    return val("Hello everyone");
}

val emval_test_new_object() {
    val rv(val::object());
    rv.set("foo", val("bar"));
    rv.set("baz", val(1));
    return rv;
}

unsigned emval_test_passthrough_unsigned(unsigned v) {
    return v;
}

val emval_test_passthrough(val v) {
    return v;
}

void emval_test_return_void() {
}

bool emval_test_not(bool b) {
    return !b;
}

unsigned emval_test_as_unsigned(val v) {
    return v.as<unsigned>();
}

unsigned emval_test_get_length(val v) {
    return v.get("length").as<unsigned>();
}

double emval_test_add(char c, signed char sc, unsigned char uc, signed short ss, unsigned short us, signed int si, unsigned int ui, signed long sl, unsigned long ul, float f, double d) {
    return c + sc + uc + ss + us + si + ui + sl + ul + f + d;
}

unsigned emval_test_sum(val v) {
    unsigned length = v.get("length").as<unsigned>();
    double rv = 0;
    for (unsigned i = 0; i < length; ++i) {
        rv += v.get(i).as<double>();
    }
    return rv;
}

std::string emval_test_take_and_return_const_char_star(const char* str) {
    return str;
}

std::string emval_test_take_and_return_std_string(std::string str) {
    return str;
}

std::string emval_test_take_and_return_std_string_const_ref(const std::string& str) {
    return str;
}

class ValHolder {
public:
    ValHolder(val v)
        : v(v)
    {}

    val getVal() const {
        return v;
    }

    void setVal(val v) {
        this->v = v;
    }

    int returnIntPlusFive( int x ) {
        return x + 5;
    }

    static int some_class_method(int i) {
        return i;
    }

private:
    val v;
};

ValHolder emval_test_return_ValHolder() {
    return val::object();
}

void emval_test_set_ValHolder_to_empty_object(ValHolder& vh) {
    vh.setVal(val::object());
}

class StringHolder {
public:
    StringHolder(const std::string& s)
        : str(s)
    {}

    void set(const std::string& s) {
        str = s;
    }
    std::string get() const {
        return str;
    }

private:
    std::string str;
};

struct TupleVector {
    float x, y, z;
};

float readTupleVectorZ(const TupleVector& v) {
    return v.z;
}

void writeTupleVectorZ(TupleVector& v, float z) {
    v.z = z;
}

struct TupleVectorTuple {
    TupleVector v;
};

TupleVector emval_test_return_TupleVector() {
    TupleVector cv;
    cv.x = 1;
    cv.y = 2;
    cv.z = 3;
    return cv;
}

TupleVector emval_test_take_and_return_TupleVector(TupleVector v) {
    return v;
}

TupleVectorTuple emval_test_return_TupleVectorTuple() {
    TupleVectorTuple cvt;
    cvt.v = emval_test_return_TupleVector();
    return cvt;
}

struct StructVector {
    float x, y, z;
};

StructVector emval_test_return_StructVector() {
    StructVector v;
    v.x = 1;
    v.y = 2;
    v.z = 3;
    return v;
}

StructVector emval_test_take_and_return_StructVector(StructVector v) {
    return v;
}

struct CustomStruct {
    CustomStruct()
        : field(10)
    {}
    int field;
};

struct TupleInStruct {
    TupleVector field;
};

TupleInStruct emval_test_take_and_return_TupleInStruct(TupleInStruct cs) {
    return cs;
}

enum Enum { ONE, TWO };

Enum emval_test_take_and_return_Enum(Enum e) {
    return e;
}

enum class EnumClass { ONE, TWO };

EnumClass emval_test_take_and_return_EnumClass(EnumClass e) {
    return e;
}

class Interface {
public:
    virtual int method() = 0;
    virtual TupleInStruct method2(const TupleInStruct& arg1, float arg2) = 0;
    virtual void method3() = 0;
};

int emval_test_call_method(Interface& i) {
    return i.method();
}

TupleInStruct emval_test_call_method2(Interface& i, const TupleInStruct& arg1, float arg2) {
    return i.method2(arg1, arg2);
}

void emval_test_call_method3(Interface& i) {
    i.method3();
}

void emval_test_call_function(val v, int i, float f, TupleVector tv, StructVector sv) {
    v(i, f, tv, sv);
}

void optional_test_copy() {
    using emscripten::internal::optional;

    optional<int> foo = 22;
    optional<int> bar(foo);

    return bool(bar);
}

void optional_test_copy2() {
    using emscripten::internal::optional;

    optional<int> foo;
    optional<int> bar(foo);

    return bool(bar);
}

EMSCRIPTEN_BINDINGS(([]() {
    function("mallinfo", &emval_test_mallinfo);

    function("emval_test_new_integer", &emval_test_new_integer);
    function("emval_test_new_string", &emval_test_new_string);
    function("emval_test_new_object", &emval_test_new_object);
    function("emval_test_passthrough_unsigned", &emval_test_passthrough_unsigned);
    function("emval_test_passthrough", &emval_test_passthrough);
    function("emval_test_return_void", &emval_test_return_void);
    function("emval_test_not", &emval_test_not);

    function("emval_test_as_unsigned", &emval_test_as_unsigned);
    function("emval_test_get_length", &emval_test_get_length);
    function("emval_test_add", &emval_test_add);
    function("emval_test_sum", &emval_test_sum);

    //function("emval_test_take_and_return_const_char_star", &emval_test_take_and_return_const_char_star);
    function("emval_test_take_and_return_std_string", &emval_test_take_and_return_std_string);
    function("emval_test_take_and_return_std_string_const_ref", &emval_test_take_and_return_std_string_const_ref);

    //function("emval_test_take_and_return_CustomStruct", &emval_test_take_and_return_CustomStruct);

    value_tuple<TupleVector>("TupleVector")
        .element(&TupleVector::x)
        .element(&TupleVector::y)
        //.element(&TupleVector::z)
        .element(&readTupleVectorZ, &writeTupleVectorZ)
        ;

    function("emval_test_return_TupleVector", &emval_test_return_TupleVector);
    function("emval_test_take_and_return_TupleVector", &emval_test_take_and_return_TupleVector);

    value_tuple<TupleVectorTuple>("TupleVectorTuple")
        .element(&TupleVectorTuple::v)
        ;

    function("emval_test_return_TupleVectorTuple", &emval_test_return_TupleVectorTuple);

    value_struct<StructVector>("StructVector")
        .field("x", &StructVector::x)
        .field("y", &StructVector::y)
        .field("z", &StructVector::z)
        ;

    function("emval_test_return_StructVector", &emval_test_return_StructVector);
    function("emval_test_take_and_return_StructVector", &emval_test_take_and_return_StructVector);

    value_struct<TupleInStruct>("TupleInStruct")
        .field("field", &TupleInStruct::field)
        ;

    function("emval_test_take_and_return_TupleInStruct", &emval_test_take_and_return_TupleInStruct);

    class_<ValHolder>("ValHolder")
        .constructor<val>()
        .method("getVal", &ValHolder::getVal)
        .method("setVal", &ValHolder::setVal)
        .method("returnIntPlusFive", &ValHolder::returnIntPlusFive)
        .classmethod("some_class_method", &ValHolder::some_class_method)
        ;
    function("emval_test_return_ValHolder", &emval_test_return_ValHolder);
    function("emval_test_set_ValHolder_to_empty_object", &emval_test_set_ValHolder_to_empty_object);

    class_<StringHolder>("StringHolder")
        .constructor<std::string>()
        .method("set", &StringHolder::set)
        .method("get", &StringHolder::get)
        ;

    class_<CustomStruct>("CustomStruct")
        .constructor<>()
        .field("field", &CustomStruct::field)
        ;

    enum_<Enum>("Enum")
        .value("ONE", ONE)
        .value("TWO", TWO)
        ;
    function("emval_test_take_and_return_Enum", &emval_test_take_and_return_Enum);

    enum_<EnumClass>("EnumClass")
        .value("ONE", EnumClass::ONE)
        .value("TWO", EnumClass::TWO)
        ;
    function("emval_test_take_and_return_EnumClass", &emval_test_take_and_return_EnumClass);

    class InterfaceWrapper : public wrapper<Interface> {
        int method() {
            return call<int>("method");
        }
        TupleInStruct method2(const TupleInStruct& arg1, float arg2) {
            return call<TupleInStruct>("method2", arg1, arg2);
        }
        void method3() {
            return call<void>("method3");
        }
    };
    interface<InterfaceWrapper>("Interface")
        ;
    function("emval_test_call_method", &emval_test_call_method);
    function("emval_test_call_method2", &emval_test_call_method2);
    function("emval_test_call_method3", &emval_test_call_method3);

    function("emval_test_call_function", &emval_test_call_function);

    function('optional_test_copy', &optional_test_copy);
    function('optional_test_copy2', &optional_test_copy2);
}));
