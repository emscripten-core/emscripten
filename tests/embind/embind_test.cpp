#include <string>
#include <malloc.h>
#include <functional>
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

std::string emval_test_get_string_from_val(val v) {
    return v["key"].as<std::string>();
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

bool emval_test_is_true(val v) {
    return v.isTrue();
}

bool emval_test_is_false(val v) {
    return v.isFalse();
}

bool emval_test_is_null(val v) {
    return v.isNull();
}

bool emval_test_is_undefined(val v) {
    return v.isUndefined();
}

bool emval_test_equals(val v1,val v2) {
    return v1.equals(v2);
}

bool emval_test_strictly_equals(val v1, val v2) {
    return v1.strictlyEquals(v2);
}

unsigned emval_test_as_unsigned(val v) {
    return v.as<unsigned>();
}

unsigned emval_test_get_length(val v) {
    return v["length"].as<unsigned>();
}

double emval_test_add(char c, signed char sc, unsigned char uc, signed short ss, unsigned short us, signed int si, unsigned int ui, signed long sl, unsigned long ul, float f, double d) {
    return c + sc + uc + ss + us + si + ui + sl + ul + f + d;
}

float const_ref_adder(const int& i, const float& f) {
    return i + f;
}

unsigned emval_test_sum(val v) {
    unsigned length = v["length"].as<unsigned>();
    double rv = 0;
    for (unsigned i = 0; i < length; ++i) {
        rv += v[i].as<double>();
    }
    return rv;
}

std::string get_non_ascii_string() {
    char c[128 + 1];
    c[128] = 0;
    for (int i = 0; i < 128; ++i) {
        c[i] = 128 + i;
    }
    return c;
}

std::wstring get_non_ascii_wstring() {
    std::wstring ws(4, 0);
    ws[0] = 10;
    ws[1] = 1234;
    ws[2] = 2345;
    ws[3] = 65535;
    return ws;
}

std::wstring get_literal_wstring() {
    return L"get_literal_wstring";
}

void force_memory_growth() {
    auto heapu8 = val::global("Module")["HEAPU8"];
    delete [] new char[heapu8["byteLength"].as<size_t>() + 1];
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

std::basic_string<unsigned char> emval_test_take_and_return_std_basic_string_unsigned_char(std::basic_string<unsigned char> str) {
    return str;
}

std::wstring take_and_return_std_wstring(std::wstring str) {
    return str;
}

std::function<std::string (std::string)> emval_test_get_function_ptr() {
    return emval_test_take_and_return_std_string;
}

std::string emval_test_take_and_call_functor(std::function<std::string(std::string)> func) {
    return func("asdf");
}

class ValHolder {
public:
    ValHolder(val v)
        : v_(v)
    {}

    val getVal() const {
        return v_;
    }

    val getValNonConst() {
        return v_;
    }

    const val getConstVal() const {
        return v_;
    }

    const val& getValConstRef() const {
        return v_;
    }

    void setVal(val v) {
        this->v_ = v;
    }

    static int some_class_method(int i) {
        return i;
    }

    static const ValHolder* makeConst(val v) {
        return new ValHolder(v);
    }

    static ValHolder makeValHolder(val v) {
        return ValHolder(v);
    }

    static void set_via_raw_pointer(ValHolder* vh, val v) {
        vh->setVal(v);
    }

    static val get_via_raw_pointer(const ValHolder* vh) {
        return vh->getVal();
    }

    static void transfer_via_raw_pointer(ValHolder* target, const ValHolder* source) {
        target->setVal(source->getVal());
    }

    static val getValNonMember(const ValHolder& target) {
        return target.getVal();
    }

private:
    val v_;
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
        : str_(s)
    {}

    void set(const std::string& s) {
        str_ = s;
    }

    std::string get() const {
        return str_;
    }

    std::string& get_ref() {
        return str_;
    }

    const std::string& get_const_ref() const {
        return str_;
    }

private:
    std::string str_;
};

class SharedPtrHolder {
public:
    SharedPtrHolder()
        : ptr_(new StringHolder("a string"))
    {}

    std::shared_ptr<StringHolder> get() const {
        return ptr_;
    }
    
    void set(std::shared_ptr<StringHolder> p) {
        ptr_ = p;
    }
private:
    std::shared_ptr<StringHolder> ptr_;
};

class VectorHolder {
public:
    VectorHolder() {
        v_.push_back(StringHolder("string #1"));
        v_.push_back(StringHolder("string #2"));
    }

    std::vector<StringHolder> get() const {
        return v_;
    }

    void set(std::vector<StringHolder> vec) {
        v_ = vec;
    }

private:
    std::vector<StringHolder> v_;
};

class SmallClass {
public:
    SmallClass(): member(7) {};
    int member;
};

class BigClass {
public:
    BigClass(): member(11) {};
    int member;
    int otherMember;
    int yetAnotherMember;

    int getMember() {
        return member;
    }
};

class ParentClass {
public:
    ParentClass(): bigClass() {};

    BigClass bigClass;

    const BigClass& getBigClass() {
        return bigClass;
    };
};

template<typename T>
class TemplateClass {
public:
    TemplateClass(T a, T b, T c) {
        members[0] = a;
        members[1] = b;
        members[2] = c;
    };

    const T getMember(int n) {
        return members[n];
    }

protected:
    T   members[3];
};

class ContainsTemplatedMemberClass {
public:
    ContainsTemplatedMemberClass(): testTemplate(86, 87, 88) {};

    TemplateClass<int> testTemplate;

    const TemplateClass<int>& getTestTemplate() {
        return testTemplate;
    };
};

// Begin Inheritance Hierarchy Class Definitions

class Base {
public:
    Base(): name("Base"),
            member(0),
            baseMember(0)
    {}

    std::string getClassName() const {
        return name;
    }
    std::string getClassNameFromBase() const {
        return name;
    }
    std::string getClassNameNotAvailableInDerivedClasses() {
        // but wait -- if you act now we will throw in a SECOND base class method ABSOLUTELY FREE!!
        return name;
    }
    void setMember(int value) {
        member = value;
    }
    int getMember() {
        return member;
    }
    void setBaseMember(int value) {
        baseMember = value;
    }
    int getBaseMember() {
        return baseMember;
    }
    std::string name;
    int member;
    int baseMember;
};

class SecondBase {
public:
    SecondBase()
        : name("SecondBase"),
          member(0),
          secondBaseMember(0)
    {}

    std::string getClassName() const {
        return name;
    }
    std::string getClassNameNotAvailableInDerivedClasses() {
        return name;
    }
    std::string getClassNameFromSecondBase() const {
        return name;
    }
    void setMember(int value) {
        member = value;
    }
    int getMember() {
        return member;
    }
    void setSecondBaseMember(int value) {
        secondBaseMember = value;
    }
    int getSecondBaseMember() {
        return secondBaseMember;
    }
    std::string name;
    int member;
    int secondBaseMember;
};

class Derived : public Base{
public:
    Derived()
        : Base()
        , member(0)
        , name_("Derived")
    {}

    std::string getClassName() const {
        return name_;
    }
    void setMember(int value) {
        member = value;
    }
    int getMember() {
        return member;
    }
    int member;
private:
    std::string name_;
};

class DerivedHolder {
public:
    DerivedHolder() {
        derived_.reset();
    }
    void newDerived() {
        deleteDerived();
        derived_ = std::shared_ptr<Derived>(new Derived());
    }
    void deleteDerived() {
        derived_.reset();
    }
    std::shared_ptr<Derived> getDerived() {
        return derived_;
    }
    std::string getDerivedClassName() {
        return derived_->getClassName();
    }
private:
    std::shared_ptr<Derived> derived_;
};

class SiblingDerived : public Base {
public:
    SiblingDerived()
        : Base(),
        name_("SiblingDerived")
    {}

    std::string getClassName() const {
        return name_;
    }

private:
    std::string name_;
};

class MultiplyDerived : public Base, public SecondBase {
public:
    MultiplyDerived()
        : Base(), SecondBase(),
        name_("MultiplyDerived")
    { instanceCount_ ++; }

    ~MultiplyDerived()
    { instanceCount_ --; }

    std::string getClassName() const {
        return name_;
    }

    static int getInstanceCount() {
        return instanceCount_;
    }
private:
    std::string name_;
    static int instanceCount_;
};
int MultiplyDerived::instanceCount_ = 0;

class DerivedTwice : public Derived {
public:
    DerivedTwice()
        : Derived(),
        name_("DerivedTwice")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class DerivedTwiceNotBound : public Derived {
public:
    DerivedTwiceNotBound()
        : Derived(),
        name_("DerivedTwiceNotBound")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class DerivedThrice: public DerivedTwiceNotBound {
public:
    DerivedThrice()
        : DerivedTwiceNotBound(),
        name_("DerivedThrice")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class DerivedFourTimesNotBound: public DerivedThrice {
public:
    DerivedFourTimesNotBound()
        : DerivedThrice(),
        name_("DerivedFourTimesNotBound")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class PolyBase {
public:
    PolyBase(const std::string& s)
        : str_(s),
          name_("PolyBase")
    {}

    PolyBase(): name_("PolyBase") {}

    virtual ~PolyBase() {}

    virtual std::string virtualGetClassName() const {
        return name_;
    }

    std::string getClassName() const {
        return name_;
    }

private:
    std::string str_;
    std::string name_;
};

class PolySecondBase {
public:
    PolySecondBase(): name_("PolySecondBase")
    {}

    virtual ~PolySecondBase() {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class PolyDerived : public PolyBase{
public:
    PolyDerived()
        : PolyBase("PolyDerived"),
          name_("PolyDerived")
    {}

    std::string virtualGetClassName() const {
        return name_;
    }

    std::string getClassName() const {
        return name_;
    }

    static void setPtrDerived() {
        ptr_ = std::shared_ptr<PolyDerived>(new PolyDerived());
    }

    static void releasePtr() {
        ptr_.reset();
    }

    static std::string getPtrClassName() {
        return ptr_->getClassName();
    }

    static std::shared_ptr<PolyBase> getPtr() {
        return ptr_;
    }

private:
    std::string name_;
    static std::shared_ptr<PolyBase> ptr_;
};
std::shared_ptr<PolyBase> PolyDerived::ptr_;

class PolySiblingDerived : public PolyBase {
public:
    PolySiblingDerived()
        : PolyBase(),
        name_("PolySiblingDerived")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class PolyMultiplyDerived : public PolyBase, public PolySecondBase {
public:
    PolyMultiplyDerived()
        : PolyBase(), PolySecondBase(),
        name_("PolyMultiplyDerived")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class PolyDerivedTwiceWithoutSmartPointer: public PolyDerived {
public:
    PolyDerivedTwiceWithoutSmartPointer()
        : PolyDerived(),
        name_("PolyDerivedTwiceWithoutSmartPointer")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class PolyDerivedTwiceNotBound : public PolyDerived {
public:
    PolyDerivedTwiceNotBound()
        : PolyDerived(),
        name_("PolyDerivedTwiceNotBound")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class PolyDerivedThrice: public PolyDerivedTwiceNotBound {
public:
    PolyDerivedThrice()
        : PolyDerivedTwiceNotBound(),
        name_("PolyDerivedThrice")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class PolyDerivedFourTimesNotBound: public PolyDerivedThrice {
public:
    PolyDerivedFourTimesNotBound()
        : PolyDerivedThrice(),
        name_("PolyDerivedFourTimesNotBound")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class PolyDiamondBase {
public:
    PolyDiamondBase():
          name_("PolyBase")
    {}
    ~PolyDiamondBase() {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class PolyDiamondDerived: public PolyDiamondBase {
public:
    PolyDiamondDerived()
        : PolyDiamondBase(),
        name_("PolyDiamondDerived")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class PolyDiamondSiblingDerived: public PolyDiamondBase {
public:
    PolyDiamondSiblingDerived()
        : PolyDiamondBase(),
        name_("PolyDiamondSiblingDerived")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

class PolyDiamondMultiplyDerived: public PolyDiamondDerived, public PolyDiamondSiblingDerived {
public:
    PolyDiamondMultiplyDerived()
        : PolyDiamondDerived(), PolyDiamondSiblingDerived(),
        name_("PolyDiamondMultiplyDerived")
    {}

    std::string getClassName() const {
        return name_;
    }
private:
    std::string name_;
};

// End Inheritance Hierarchy Class Definitions

std::map<std::string, int> embind_test_get_string_int_map() {
    std::map<std::string, int> m;

    m["one"] = 1;
    m["two"] = 2;

    return m;
};

struct Vector {
    Vector() = delete;

    Vector(float x_, float y_, float z_, float w_)
        : x(x_)
        , y(y_)
        , z(z_)
        , w(w_)
    {}

    float x, y, z, w;

    float& operator[](int i) {
        return (&x)[i];
    }

    const float& operator[](int i) const {
        return (&x)[i];
    }

    float getY() const {
        return y;
    }
    void setY(float _y) {
        y = _y;
    }
};

struct DummyDataToTestPointerAdjustment {
    std::string dummy;
};

struct TupleVector : DummyDataToTestPointerAdjustment, Vector {
    TupleVector(): Vector(0, 0, 0, 0) {}
    TupleVector(float x, float y, float z, float w): Vector(x, y, z, w) {}
};

struct StructVector : DummyDataToTestPointerAdjustment, Vector {
    StructVector(): Vector(0, 0, 0, 0) {}
    StructVector(float x, float y, float z, float w): Vector(x, y, z, w) {}
};

float readVectorZ(const Vector& v) {
    return v.z;
}

void writeVectorZ(Vector& v, float z) {
    v.z = z;
}

struct TupleVectorTuple {
    TupleVector v = TupleVector(0, 0, 0, 0);
};

TupleVector emval_test_return_TupleVector() {
    return TupleVector(1, 2, 3, 4);
}

TupleVector emval_test_take_and_return_TupleVector(TupleVector v) {
    return v;
}

TupleVectorTuple emval_test_return_TupleVectorTuple() {
    TupleVectorTuple cvt;
    cvt.v = emval_test_return_TupleVector();
    return cvt;
}

StructVector emval_test_return_StructVector() {
    return StructVector(1, 2, 3, 4);
}

StructVector emval_test_take_and_return_StructVector(StructVector v) {
    return v;
}

struct CustomStruct {
    CustomStruct()
        : field(10)
    {}

    const int& getField() const {
        return field;
    }

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

enum class EnumClass : char { ONE, TWO };

EnumClass emval_test_take_and_return_EnumClass(EnumClass e) {
    return e;
}

void emval_test_call_function(val v, int i, float f, TupleVector tv, StructVector sv) {
    v(i, f, tv, sv);
}

class UniquePtrToConstructor {
public:
    UniquePtrToConstructor(std::unique_ptr<int> p)
        : value(*p)
    {}
    
    int getValue() const {
        return value;
    }
    
private:
    int value;
};

std::unique_ptr<int> embind_test_return_unique_ptr(int v) {
    return std::unique_ptr<int>(new int(v));
}

UniquePtrToConstructor* embind_test_construct_class_with_unique_ptr(int v) {
    return new UniquePtrToConstructor(embind_test_return_unique_ptr(v));
}

int embind_test_accept_unique_ptr(std::unique_ptr<int> p) {
    return *p.get();
}

std::unique_ptr<ValHolder> emval_test_return_unique_ptr() {
    return std::unique_ptr<ValHolder>(new ValHolder(val::object()));
}

class UniquePtrLifetimeMock {
public:
    UniquePtrLifetimeMock(val l) : logger(l) {
        logger(std::string("(constructor)"));
    }
    ~UniquePtrLifetimeMock() {
        logger(std::string("(destructor)"));
    }

private:
    val logger;
};

std::unique_ptr<UniquePtrLifetimeMock> emval_test_return_unique_ptr_lifetime(val logger) {
    return std::unique_ptr<UniquePtrLifetimeMock>(new UniquePtrLifetimeMock(logger));
}

std::shared_ptr<ValHolder> emval_test_return_shared_ptr() {
    return std::shared_ptr<ValHolder>(new ValHolder(val::object()));
}

std::shared_ptr<ValHolder> emval_test_return_empty_shared_ptr() {
    return std::shared_ptr<ValHolder>();
}

bool emval_test_is_shared_ptr_null(std::shared_ptr<ValHolder> p) {
    return !p;
}

static SmallClass smallClass;
static BigClass bigClass;

SmallClass embind_test_return_small_class_instance() {
    return smallClass;
}

BigClass embind_test_return_big_class_instance() {
    return bigClass;
}

int embind_test_accept_small_class_instance(SmallClass c) {
    return c.member;
}

int embind_test_accept_big_class_instance(BigClass c) {
    return c.member;
}

// Begin Inheritance Hierarchy Test Wrappers

Base* embind_test_return_raw_base_ptr() {
    return new Base();
}

Base* embind_test_return_raw_derived_ptr_as_base() {
    return new Derived();
}

Base* embind_test_return_raw_sibling_derived_ptr_as_base() {
    return new SiblingDerived();
}

PolyBase* embind_test_return_raw_polymorphic_derived_ptr_as_base() {
    return new PolyDerived();
}

PolyBase* embind_test_return_raw_polymorphic_sibling_derived_ptr_as_base() {
    return new PolySiblingDerived();
}

PolyBase* embind_test_return_raw_polymorphic_multiply_derived_ptr_as_base() {
    return new PolyMultiplyDerived();
}

PolySecondBase* embind_test_return_raw_polymorphic_multiply_derived_ptr_as_second_base() {
    return new PolyMultiplyDerived();
}

PolyBase* embind_test_return_raw_polymorphic_derived_four_times_not_bound_as_base() {
    return new PolyDerivedFourTimesNotBound();
}

std::shared_ptr<Base> embind_test_return_smart_base_ptr() {
    return std::shared_ptr<Base>(new Base());
}

std::shared_ptr<PolyBase> embind_test_return_smart_polymorphic_base_ptr() {
    return std::shared_ptr<PolyBase>(new PolyBase("PolyBase"));
}

std::shared_ptr<Derived> embind_test_return_smart_derived_ptr() {
    return std::shared_ptr<Derived>(new Derived());
}

std::shared_ptr<SiblingDerived> embind_test_return_smart_sibling_derived_ptr() {
    return std::shared_ptr<SiblingDerived>(new SiblingDerived());
}

std::shared_ptr<MultiplyDerived> embind_test_return_smart_multiply_derived_ptr() {
    return std::shared_ptr<MultiplyDerived>(new MultiplyDerived());
}

std::shared_ptr<DerivedThrice> embind_test_return_smart_derived_thrice_ptr() {
    return std::shared_ptr<DerivedThrice>(new DerivedThrice());
}

std::shared_ptr<PolyDerived> embind_test_return_smart_polymorphic_derived_ptr() {
    return std::shared_ptr<PolyDerived>(new PolyDerived());
}

std::shared_ptr<PolySiblingDerived> embind_test_return_smart_polymorphic_sibling_derived_ptr() {
    return std::shared_ptr<PolySiblingDerived>(new PolySiblingDerived());
}

std::shared_ptr<PolyMultiplyDerived> embind_test_return_smart_polymorphic_multiply_derived_ptr() {
    return std::shared_ptr<PolyMultiplyDerived>(new PolyMultiplyDerived());
}

std::shared_ptr<PolyBase> embind_test_return_poly_derived_twice_without_smart_pointer_as_poly_base() {
    return std::shared_ptr<PolyBase>(new PolyDerivedTwiceWithoutSmartPointer());
}

std::shared_ptr<PolyDerivedThrice> embind_test_return_smart_poly_derived_thrice_ptr() {
    return std::shared_ptr<PolyDerivedThrice>(new PolyDerivedThrice());
}

std::shared_ptr<PolyBase> embind_test_return_smart_derived_ptr_as_base() {
    return std::shared_ptr<PolyBase>(new PolyDerived());
}

val embind_test_return_smart_derived_ptr_as_val() {
    return val(std::shared_ptr<PolyBase>(new PolyDerived()));
}

std::shared_ptr<PolyBase> embind_test_return_smart_polymorphic_derived_ptr_as_base() {
    return std::shared_ptr<PolyBase>(new PolyDerived());
}

std::shared_ptr<PolyBase> embind_test_return_smart_polymorphic_sibling_derived_ptr_as_base() {
    return std::shared_ptr<PolyBase>(new PolySiblingDerived());
}

std::string embind_test_get_class_name_via_base_ptr(Base *p) {
    return p->getClassName();
}

std::string embind_test_get_class_name_via_second_base_ptr(SecondBase *p) {
    return p->getClassName();
}

std::string embind_test_get_class_name_via_polymorphic_base_ptr(PolyBase *p) {
    return p->getClassName();
}

std::string embind_test_get_class_name_via_polymorphic_second_base_ptr(PolySecondBase *p) {
    return p->getClassName();
}

std::string embind_test_get_class_name_via_smart_base_ptr(std::shared_ptr<Base> p) {
    return p->getClassName();
}

std::string embind_test_get_class_name_via_reference_to_smart_base_ptr(std::shared_ptr<Base>& p) {
    return p->getClassName();
}

std::string embind_test_get_class_name_via_smart_second_base_ptr(std::shared_ptr<SecondBase> p) {
    return p->getClassName();
}

std::string embind_test_get_class_name_via_smart_polymorphic_base_ptr(std::shared_ptr<PolyBase> p) {
    return p->getClassName();
}

std::string embind_test_get_virtual_class_name_via_smart_polymorphic_base_ptr(std::shared_ptr<PolyBase> p) {
    return p->virtualGetClassName();
}

std::string embind_test_get_class_name_via_smart_polymorphic_second_base_ptr(std::shared_ptr<PolySecondBase> p) {
    return p->getClassName();
}

void embind_modify_smart_pointer_passed_by_reference(std::shared_ptr<Base>& p) {
    p = std::shared_ptr<Base>(new Base());
    p->name = "Changed";
}

void embind_attempt_to_modify_smart_pointer_when_passed_by_value(std::shared_ptr<Base> p) {
    p = std::shared_ptr<Base>(new Base());
    p->name = "Changed";
}

static std::shared_ptr<Base> savedBasePointer;

void embind_save_smart_base_pointer(std::shared_ptr<Base> p) {
    savedBasePointer = p;
}

// End Inheritance Hierarchy Test Wrappers

std::vector<int> emval_test_return_vector() {
    int myints[] = { 10, 20, 30 };
    return std::vector<int>(myints, myints + sizeof(myints) / sizeof(int));
}

std::vector<std::vector<int> > emval_test_return_vector_of_vectors() {
    int myints1[] = { 10, 20, 30 };
    int myints2[] = { 40, 50, 60 };
    std::vector<int> vec1(myints1, myints1 + sizeof(myints1) / sizeof(int));
    std::vector<int> vec2(myints2, myints2 + sizeof(myints2) / sizeof(int));
    std::vector<std::vector<int>>vec3;
    vec3.emplace_back(vec1);
    vec3.emplace_back(vec2);
    return vec3;
}

std::vector<std::shared_ptr<StringHolder>> emval_test_return_shared_ptr_vector() {
    std::vector<std::shared_ptr<StringHolder>> sharedStrVector;
    sharedStrVector.push_back(std::shared_ptr<StringHolder>(new StringHolder("string #1")));
    sharedStrVector.push_back(std::shared_ptr<StringHolder>(new StringHolder("string #2")));

    return sharedStrVector;
}

void test_string_with_vec(const std::string& p1, std::vector<std::string>& v1) {
    // THIS DOES NOT WORK -- need to get as val and then call vecFromJSArray
    printf("%s\n", p1.c_str());
}

val embind_test_getglobal() {
    return val::global();
}

val embind_test_new_Object() {
    return val::global("Object").new_();
}

val embind_test_new_factory(val factory, val argument) {
    return factory.new_(10, std::string("hello"), argument);
}

class AbstractClass {
public:
    virtual ~AbstractClass() {}
    virtual std::string abstractMethod() const = 0;
    virtual std::string optionalMethod(std::string s) const {
        return "optional" + s;
    }

    virtual std::shared_ptr<Derived> returnsSharedPtr() = 0;
    virtual void differentArguments(int i, double d, unsigned char f, double q, std::string) = 0;

    std::string concreteMethod() const {
        return "concrete";
    }

    virtual void passShared(const std::shared_ptr<Derived>&) {
    }

    virtual void passVal(const val& v) {
    }
};

EMSCRIPTEN_SYMBOL(optionalMethod);

class AbstractClassWrapper : public wrapper<AbstractClass> {
public:
    EMSCRIPTEN_WRAPPER(AbstractClassWrapper);

    std::string abstractMethod() const override {
        return call<std::string>("abstractMethod");
    }

    std::string optionalMethod(std::string s) const override {
        return call<std::string>("optionalMethod", s);
    }

    std::shared_ptr<Derived> returnsSharedPtr() override {
        return call<std::shared_ptr<Derived> >("returnsSharedPtr");
    }

    void differentArguments(int i, double d, unsigned char f, double q, std::string s) override {
        return call<void>("differentArguments", i, d, f, q, s);
    }

    virtual void passShared(const std::shared_ptr<Derived>& p) override {
        return call<void>("passShared", p);
    }

    virtual void passVal(const val& v) override {
        return call<void>("passVal", v);
    }
};

class ConcreteClass : public AbstractClass {
    std::string abstractMethod() const {
        return "from concrete";
    }

    void differentArguments(int i, double d, unsigned char f, double q, std::string s) {
    }

    std::shared_ptr<Derived> returnsSharedPtr() {
        return std::shared_ptr<Derived>();
    }
};

std::shared_ptr<AbstractClass> getAbstractClass() {
    return std::make_shared<ConcreteClass>();
}

std::string callAbstractMethod(AbstractClass& ac) {
    return ac.abstractMethod();
}

std::string callOptionalMethod(AbstractClass& ac, std::string s) {
    return ac.optionalMethod(s);
}

void callReturnsSharedPtrMethod(AbstractClass& ac) {
    std::shared_ptr<Derived> sp = ac.returnsSharedPtr();
    // unused: sp
}

void callDifferentArguments(AbstractClass& ac, int i, double d, unsigned char f, double q, std::string s) {
    return ac.differentArguments(i, d, f, q, s);
}

struct AbstractClassWithConstructor {
    explicit AbstractClassWithConstructor(std::string s)
        : s(s)
    {}

    virtual ~AbstractClassWithConstructor() {};

    virtual std::string abstractMethod() = 0;
    std::string concreteMethod() {
        return s;
    }

    std::string s;
};

struct AbstractClassWithConstructorWrapper : public wrapper<AbstractClassWithConstructor> {
    EMSCRIPTEN_WRAPPER(AbstractClassWithConstructorWrapper);

    virtual std::string abstractMethod() override {
        return call<std::string>("abstractMethod");
    }
};

std::string callAbstractMethod2(AbstractClassWithConstructor& ac) {
    return ac.abstractMethod();
}

struct HeldAbstractClass : public PolyBase, public PolySecondBase {
    virtual void method() = 0;
};
struct HeldAbstractClassWrapper : wrapper<HeldAbstractClass> {
    EMSCRIPTEN_WRAPPER(HeldAbstractClassWrapper);

    virtual void method() override {
        return call<void>("method");
    }
};

std::shared_ptr<PolySecondBase> passHeldAbstractClass(std::shared_ptr<HeldAbstractClass> p) {
    return p;
}

void passShared(AbstractClass& ac) {
    auto p = std::make_shared<Derived>();
    ac.passShared(p);
}

void passVal(AbstractClass& ac, val v) {
    return ac.passVal(v);
}

EMSCRIPTEN_BINDINGS(interface_tests) {
    class_<AbstractClass>("AbstractClass")
        .smart_ptr<std::shared_ptr<AbstractClass>>("shared_ptr<AbstractClass>")
        .allow_subclass<AbstractClassWrapper>("AbstractClassWrapper")
        .function("abstractMethod", &AbstractClass::abstractMethod, pure_virtual())
        // The optional_override is necessary because, otherwise, the C++ compiler
        // cannot deduce the signature of the lambda function.
        .function("optionalMethod", optional_override(
            [](AbstractClass& this_, const std::string& s) {
                return this_.AbstractClass::optionalMethod(s);
            }
        ))
        .function("concreteMethod", &AbstractClass::concreteMethod)
        .function("passShared", select_overload<void(AbstractClass&, const std::shared_ptr<Derived>&)>([](AbstractClass& self, const std::shared_ptr<Derived>& derived) {
            self.AbstractClass::passShared(derived);
        }))
        .function("passVal", select_overload<void(AbstractClass&, const val&)>([](AbstractClass& self, const val& v) {
            self.AbstractClass::passVal(v);
        }))
        ;
    
    function("getAbstractClass", &getAbstractClass);
    function("callAbstractMethod", &callAbstractMethod);
    function("callOptionalMethod", &callOptionalMethod);
    function("callReturnsSharedPtrMethod", &callReturnsSharedPtrMethod);
    function("callDifferentArguments", &callDifferentArguments);
    function("passShared", &passShared);
    function("passVal", &passVal);

    class_<AbstractClassWithConstructor>("AbstractClassWithConstructor")
        .allow_subclass<AbstractClassWithConstructorWrapper>("AbstractClassWithConstructorWrapper", constructor<std::string>())
        .function("abstractMethod", &AbstractClassWithConstructor::abstractMethod, pure_virtual())
        .function("concreteMethod", &AbstractClassWithConstructor::concreteMethod)
        ;
    function("callAbstractMethod2", &callAbstractMethod2);

    class_<HeldAbstractClass, base<PolySecondBase>>("HeldAbstractClass")
        .smart_ptr<std::shared_ptr<HeldAbstractClass>>("shared_ptr<HeldAbstractClass>")
        .allow_subclass<HeldAbstractClassWrapper, std::shared_ptr<HeldAbstractClassWrapper>>("HeldAbstractClassWrapper", "HeldAbstractClassWrapperPtr")
        .function("method", &HeldAbstractClass::method, pure_virtual())
        ;
    function("passHeldAbstractClass", &passHeldAbstractClass);
}

template<typename T, size_t sizeOfArray>
constexpr size_t getElementCount(T (&)[sizeOfArray]) {
    return sizeOfArray;
}

static void callWithMemoryView(val v) {
    // static so the JS test can read the memory after callTakeMemoryView runs
    static unsigned char data[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    v(typed_memory_view(getElementCount(data), data));
    static float f[] = { 1.5f, 2.5f, 3.5f, 4.5f };
    v(typed_memory_view(getElementCount(f), f));
    static short s[] = { 1000, 100, 10, 1 };
    v(typed_memory_view(getElementCount(s), s));
}

EMSCRIPTEN_BINDINGS(memory_view_tests) {
    function("callWithMemoryView", &callWithMemoryView);
}

class HasExternalConstructor {
public:
    HasExternalConstructor(const std::string& str)
        : m(str)
    {}

    std::string getString() const {
        return m;
    }

    std::string m;
};

HasExternalConstructor* createHasExternalConstructor(const std::string& str) {
    return new HasExternalConstructor(str);
}

template<typename T>
class CustomSmartPtr {
public:
    CustomSmartPtr()
        : CustomSmartPtr(nullptr)
    {
        std::fill(d_, d_ + N_, Valid);
    }

    explicit CustomSmartPtr(T* t)
        : ptr_(t)
    {
        std::fill(d_, d_ + N_, Valid);
    }

    CustomSmartPtr(const CustomSmartPtr& other)
        : ptr_(other.ptr_)
    {
        other.verify();
        std::fill(d_, d_ + N_, Valid);
        if (ptr_) {
            ++(ptr_->refcount);
        }
    }

    ~CustomSmartPtr() {
        verify();
        std::fill(d_, d_ + N_, Deleted);
        
        if (ptr_ && --ptr_->refcount == 0) {
            delete ptr_;
        }
    }

    T* get_raw() const {
        return ptr_;
    }

private:
    void verify() const {
        for (size_t i = 0; i < N_; ++i) {
            if (d_[i] != Valid) {
                abort();
            }
        }
    }

    enum {
        Valid = 255,
        Deleted = 127,
    };
    static constexpr size_t N_ = 1000000;
    unsigned char d_[N_];
    T* ptr_;

    CustomSmartPtr& operator=(const CustomSmartPtr&) = delete;
};

class HeldBySmartPtr {
public:
    HeldBySmartPtr(int i, const std::string& s)
        : i(i)
        , s(s)
    {}

    static CustomSmartPtr<HeldBySmartPtr> newCustomPtr(int i, const std::string& s) {
        return CustomSmartPtr<HeldBySmartPtr>(new HeldBySmartPtr(i, s));
    }

    int refcount = 1;
    int i;
    std::string s;
};

HeldBySmartPtr takesHeldBySmartPtr(HeldBySmartPtr p) {
    return p;
}
std::shared_ptr<HeldBySmartPtr> takesHeldBySmartPtrSharedPtr(std::shared_ptr<HeldBySmartPtr> p) {
    return p;
}

namespace emscripten {
    template<typename T>
    struct smart_ptr_trait<CustomSmartPtr<T>> {
        typedef CustomSmartPtr<T> pointer_type;
        typedef T element_type;

        static sharing_policy get_sharing_policy() {
            return sharing_policy::NONE;
        }

        static T* get(const CustomSmartPtr<T>& p) {
            return p.get_raw();
        }

        static CustomSmartPtr<T> share(const CustomSmartPtr<T>& r, T* ptr) {
            ++ptr->refcount; // implement an adopt API?
            return CustomSmartPtr<T>(ptr);
        }

        static pointer_type* construct_null() {
            return new pointer_type;
        }
    };
}

typedef CustomSmartPtr<class HeldByCustomSmartPtr> HeldByCustomSmartPtrPtr;

class HeldByCustomSmartPtr {
public:
    HeldByCustomSmartPtr(int i, const std::string& s)
        : i(i)
        , s(s)
    {}

    static HeldByCustomSmartPtrPtr create(int i, const std::string& s) {
        return HeldByCustomSmartPtrPtr(new HeldByCustomSmartPtr(i, s));
    }

    static std::shared_ptr<HeldByCustomSmartPtr> createSharedPtr(int i, const std::string& s) {
        return std::make_shared<HeldByCustomSmartPtr>(i, s);
    };

    int refcount = 1;
    int i;
    std::string s;
};

HeldByCustomSmartPtr* passThroughRawPtr(HeldByCustomSmartPtr* p) {
    return p;
}
HeldByCustomSmartPtrPtr passThroughCustomSmartPtr(HeldByCustomSmartPtrPtr p) {
    return p;
}

struct Base1 {
public:
    Base1(): field1("Base1") {}
    std::string field1;

    std::string getField() const {
        return field1;
    }
};

struct Base2 {
public:
    Base2(): field2("Base2") {}
    std::string field2;

    std::string getField() const {
        return field2;
    }
};

struct HasTwoBases : public Base1, public Base2 {
};

val get_module_property(const std::string& s) {
    return val::module_property(s.c_str());
}

std::string char_to_string(char ch) {
    char str[256];
    sprintf(str, "%d", (int)ch);
    return str;
}

std::string signed_char_to_string(signed char ch) {
    char str[256];
    sprintf(str, "%hhd", ch);
    return str;
}

std::string unsigned_char_to_string(unsigned char ch) {
    char str[256];
    sprintf(str, "%hhu", ch);
    return str;
}

std::string short_to_string(short val) {
    char str[256];
    sprintf(str, "%hd", val);
    return str;
}

std::string unsigned_short_to_string(unsigned short val) {
    char str[256];
    sprintf(str, "%hu", val);
    return str;
}

std::string int_to_string(int val) {
    char str[256];
    sprintf(str, "%d", val);
    return str;
}

std::string unsigned_int_to_string(unsigned int val) {
    char str[256];
    sprintf(str, "%u", val);
    return str;
}

std::string long_to_string(long val) {
    char str[256];
    sprintf(str, "%ld", val);
    return str;
}

std::string unsigned_long_to_string(unsigned long val) {
    char str[256];
    sprintf(str, "%lu", val);
    return str;
}

//test loading unsigned value from memory
static unsigned char uchar;
void store_unsigned_char(unsigned char arg) {
	uchar = arg;
}

unsigned char load_unsigned_char() {
	return uchar;
}

static unsigned short ushort;
void store_unsigned_short(unsigned short arg) {
	ushort = arg;
}

unsigned short load_unsigned_short() {
	return ushort;
}

static unsigned int uint;
void store_unsigned_int(unsigned int arg) {
	uint = arg;
}

unsigned int load_unsigned_int() {
	return uint;
}

static unsigned long ulong;
void store_unsigned_long(unsigned long arg) {
	ulong = arg;
}

unsigned long load_unsigned_long() {
	return ulong;
}

EMSCRIPTEN_BINDINGS(tests) {
    register_vector<int>("IntegerVector");
    register_vector<char>("CharVector");
    register_vector<unsigned>("VectorUnsigned");
    register_vector<unsigned char>("VectorUnsignedChar");
    register_vector<std::string>("StringVector");
    register_vector<emscripten::val>("EmValVector");
    register_vector<float>("FloatVector");
    register_vector<std::vector<int>>("IntegerVectorVector");

    function("mallinfo", &emval_test_mallinfo);
    function("emval_test_new_integer", &emval_test_new_integer);
    function("emval_test_new_string", &emval_test_new_string);
    function("emval_test_get_string_from_val", &emval_test_get_string_from_val);
    function("emval_test_new_object", &emval_test_new_object);
    function("emval_test_passthrough_unsigned", &emval_test_passthrough_unsigned);
    function("emval_test_passthrough", &emval_test_passthrough);
    function("emval_test_return_void", &emval_test_return_void);
    function("emval_test_not", &emval_test_not);

    function("emval_test_is_true", &emval_test_is_true);
    function("emval_test_is_false", &emval_test_is_false);
    function("emval_test_is_null", &emval_test_is_null);
    function("emval_test_is_undefined", &emval_test_is_undefined);
    function("emval_test_equals", &emval_test_equals);
    function("emval_test_strictly_equals", &emval_test_strictly_equals);

    function("emval_test_as_unsigned", &emval_test_as_unsigned);
    function("emval_test_get_length", &emval_test_get_length);
    function("emval_test_add", &emval_test_add);
    function("const_ref_adder", &const_ref_adder);
    function("emval_test_sum", &emval_test_sum);

    function("get_non_ascii_string", &get_non_ascii_string);
    function("get_non_ascii_wstring", &get_non_ascii_wstring);
    function("get_literal_wstring", &get_literal_wstring);
    function("force_memory_growth", &force_memory_growth);

    //function("emval_test_take_and_return_const_char_star", &emval_test_take_and_return_const_char_star);
    function("emval_test_take_and_return_std_string", &emval_test_take_and_return_std_string);
    function("emval_test_take_and_return_std_string_const_ref", &emval_test_take_and_return_std_string_const_ref);
    function("emval_test_take_and_return_std_basic_string_unsigned_char", &emval_test_take_and_return_std_basic_string_unsigned_char);
    function("take_and_return_std_wstring", &take_and_return_std_wstring);

    //function("emval_test_take_and_return_CustomStruct", &emval_test_take_and_return_CustomStruct);

    value_array<TupleVector>("TupleVector")
        .element(&TupleVector::x)
        .element(&Vector::getY, &Vector::setY)
        .element(&readVectorZ, &writeVectorZ)
        .element(index<3>())
        ;

    function("emval_test_return_TupleVector", &emval_test_return_TupleVector);
    function("emval_test_take_and_return_TupleVector", &emval_test_take_and_return_TupleVector);

    value_array<TupleVectorTuple>("TupleVectorTuple")
        .element(&TupleVectorTuple::v)
        ;

    function("emval_test_return_TupleVectorTuple", &emval_test_return_TupleVectorTuple);

    value_object<StructVector>("StructVector")
        .field("x", &StructVector::x)
        .field("y", &Vector::getY, &Vector::setY)
        .field("z", &readVectorZ, &writeVectorZ)
        .field("w", index<3>())
        ;

    function("emval_test_return_StructVector", &emval_test_return_StructVector);
    function("emval_test_take_and_return_StructVector", &emval_test_take_and_return_StructVector);

    value_object<TupleInStruct>("TupleInStruct")
        .field("field", &TupleInStruct::field)
        ;

    function("emval_test_take_and_return_TupleInStruct", &emval_test_take_and_return_TupleInStruct);

    class_<ValHolder>("ValHolder")
        .smart_ptr<std::shared_ptr<ValHolder>>("std::shared_ptr<ValHolder>")
        .constructor<val>()
        .function("getVal", &ValHolder::getVal)
        .function("getValNonConst", &ValHolder::getValNonConst)
        .function("getConstVal", &ValHolder::getConstVal)
        .function("getValConstRef", &ValHolder::getValConstRef)
        .function("setVal", &ValHolder::setVal)
        .property("val", &ValHolder::getVal, &ValHolder::setVal)
        .property("val_readonly", &ValHolder::getVal)
        .class_function("makeConst", &ValHolder::makeConst, allow_raw_pointer<ret_val>())
        .class_function("makeValHolder", &ValHolder::makeValHolder)
        .class_function("some_class_method", &ValHolder::some_class_method)
        .class_function("set_via_raw_pointer",
                        &ValHolder::set_via_raw_pointer,
                        allow_raw_pointer<arg<0>>())
        .class_function("get_via_raw_pointer",
                        &ValHolder::get_via_raw_pointer,
                        allow_raw_pointer<arg<0>>())
        .class_function("transfer_via_raw_pointer",
                        &ValHolder::transfer_via_raw_pointer,
                        allow_raw_pointers())

        // non-member method
        .function("setEmpty", &emval_test_set_ValHolder_to_empty_object)
        .function("getValNonMember", &ValHolder::getValNonMember)
        ;

    function("emval_test_return_ValHolder", &emval_test_return_ValHolder);
    function("emval_test_set_ValHolder_to_empty_object", &emval_test_set_ValHolder_to_empty_object);

    class_<std::function<std::string(std::string)>>("StringFunctorString")
        .constructor<>()
        .function("opcall", &std::function<std::string(std::string)>::operator())
        ;

    function("emval_test_get_function_ptr", &emval_test_get_function_ptr);
    function("emval_test_take_and_call_functor", &emval_test_take_and_call_functor);

    class_<StringHolder>("StringHolder")
        .smart_ptr<std::shared_ptr<StringHolder>>("shared_ptr<StringHolder>")
        .constructor<std::string>()
        .function("set", &StringHolder::set)
        .function("get", &StringHolder::get)
        .function("get_const_ref", &StringHolder::get_const_ref)
        ;
    
    class_<SharedPtrHolder>("SharedPtrHolder")
        .constructor<>()
        .function("get", &SharedPtrHolder::get)
        .function("set", &SharedPtrHolder::set)
        ;

    class_<SmallClass>("SmallClass")
        .constructor<>()
        .property("member", &SmallClass::member)
        ;

    class_<BigClass>("BigClass")
        .constructor<>()
        .property("member", &BigClass::member)
        .property("otherMember", &BigClass::otherMember)
        .property("yetAnotherMember", &BigClass::yetAnotherMember)
        .function("getMember", &BigClass::getMember)
        ;

    class_<ParentClass>("ParentClass")
        .constructor<>()
        .function("getBigClass", &ParentClass::getBigClass)
        ;

    class_<TemplateClass<int>>("IntTemplateClass")
        .constructor<int, int, int>()
        .function("getMember", &TemplateClass<int>::getMember)
        ;

    class_<ContainsTemplatedMemberClass>("ContainsTemplatedMemberClass")
        .constructor<>()
        .function("getTestTemplate", &ContainsTemplatedMemberClass::getTestTemplate)
        ;

    // register Derived before Base as a test that it's possible to
    // register base classes afterwards
    class_<Derived, base<Base>>("Derived")
        .smart_ptr<std::shared_ptr<Derived>>("shared_ptr<Derived>")
        .constructor<>()
        .function("getClassName", &Derived::getClassName)
        .function("getMember", &Derived::getMember)
        .function("setMember", &Derived::setMember)
        .property("member", &Derived::member)
        ;

    class_<Base>("Base")
        .smart_ptr<std::shared_ptr<Base>>("shared_ptr<Base")
        .constructor<>()
        .function("getClassName", &Base::getClassName)
        .function("getClassNameFromBase", &Base::getClassNameFromBase)
        .function("getClassNameNotAvailableInDerivedClasses", &Base::getClassNameNotAvailableInDerivedClasses)
        .function("getMember", &Base::getMember)
        .function("setMember", &Base::setMember)
        .function("getBaseMember", &Base::getBaseMember)
        .function("setBaseMember", &Base::setBaseMember)
        .property("member", &Base::member)
        .property("baseMember", &Base::baseMember)
        ;

    class_<SecondBase>("SecondBase")
        .smart_ptr<std::shared_ptr<SecondBase>>("shared_ptr<SecondBase>")
        .constructor<>()
        .function("getClassName", &SecondBase::getClassName)
        .function("getClassNameFromSecondBase", &SecondBase::getClassNameFromSecondBase)
        .function("getClassNameNotAvailableInDerivedClasses", &SecondBase::getClassNameNotAvailableInDerivedClasses)
        .function("getMember", &SecondBase::getMember)
        .function("setMember", &SecondBase::setMember)
        .function("getSecondBaseMember", &SecondBase::getSecondBaseMember)
        .function("setSecondBaseMember", &SecondBase::setSecondBaseMember)
        .property("member", &SecondBase::member)
        .property("secondBaseMember", &SecondBase::secondBaseMember)
        ;
    

    class_<DerivedHolder>("DerivedHolder")
        .constructor<>()
        .function("newDerived", &DerivedHolder::newDerived)
        .function("deleteDerived", &DerivedHolder::deleteDerived)
        .function("getDerived", &DerivedHolder::getDerived)
        .function("getDerivedClassName", &DerivedHolder::getDerivedClassName)
        ;

    class_<SiblingDerived>("SiblingDerived")
        .smart_ptr<std::shared_ptr<SiblingDerived>>("shared_ptr<SiblingDerived>")
        .constructor<>()
        .function("getClassName", &SiblingDerived::getClassName)
        ;
    
    class_<MultiplyDerived, base<Base>>("MultiplyDerived")
        .smart_ptr<std::shared_ptr<MultiplyDerived>>("shared_ptr<MultiplyDerived>")
        .constructor<>()
        .function("getClassName", &MultiplyDerived::getClassName)
        .class_function("getInstanceCount", &MultiplyDerived::getInstanceCount)
        ;

    class_<DerivedTwice, base<Derived> >("DerivedTwice")
        .constructor<>()
        .function("getClassName", &DerivedTwice::getClassName)
        ;

    class_<DerivedThrice, base<Derived> >("DerivedThrice")
        .smart_ptr<std::shared_ptr<DerivedThrice>>("shared_ptr<DerivedThrice>")
        .constructor<>()
        .function("getClassName", &DerivedThrice::getClassName)
        ;

    class_<PolyBase>("PolyBase")
        .smart_ptr<std::shared_ptr<PolyBase>>("shared_ptr<PolyBase>")
        .constructor<>()
        .function("virtualGetClassName", &PolyBase::virtualGetClassName)
        .function("getClassName", &PolyBase::getClassName)
        ;

    class_<PolySecondBase>("PolySecondBase")
        .smart_ptr<std::shared_ptr<PolySecondBase>>("shared_ptr<PolySecondBase>")
        .constructor<>()
        .function("getClassName", &PolySecondBase::getClassName)
        ;

    class_<PolyDerived, base<PolyBase>>("PolyDerived")
        .smart_ptr<std::shared_ptr<PolyDerived>>("shared_ptr<PolyDerived>")
        .constructor<>()
        .function("virtualGetClassName", &PolyDerived::virtualGetClassName)
        .function("getClassName", &PolyDerived::getClassName)
        .class_function("setPtrDerived", &PolyDerived::setPtrDerived)
        .class_function("releasePtr", &PolyDerived::releasePtr)
        .class_function("getPtrClassName", &PolyDerived::getPtrClassName)
        .class_function("getPtr", &PolyDerived::getPtr)
        ;
//    static void setPtrDerived() {
//        ptr = std::shared_ptr<PolyDerived>(new PolyDerived());
//    }
//
//    static std::string getPtrClassName() {
//        return ptr->getClassName();
//    }
//
//    static std::shared_ptr<PolyBase> getPtr() {
//        return ptr;
//    }

    class_<PolySiblingDerived, base<PolyBase>>("PolySiblingDerived")
        .smart_ptr<std::shared_ptr<PolySiblingDerived>>("shared_ptr<PolySiblingDerived>")
        .constructor<>()
        .function("getClassName", &PolySiblingDerived::getClassName)
        ;

    class_<PolyMultiplyDerived, base<PolyBase>>("PolyMultiplyDerived")
        .smart_ptr<std::shared_ptr<PolyMultiplyDerived>>("shared_ptr<PolyMultiplyDerived>")
        .constructor<>()
        .function("getClassName", &PolyMultiplyDerived::getClassName)
        ;

    class_<PolyDerivedThrice, base<PolyDerived>>("PolyDerivedThrice")
        .smart_ptr<std::shared_ptr<PolyDerivedThrice>>("shared_ptr<PolyDerivedThrice>")
        .constructor<>()
        .function("getClassName", &PolyDerivedThrice::getClassName)
        ;

    class_<PolyDiamondBase>("PolyDiamondBase")
        .smart_ptr<std::shared_ptr<PolyDiamondBase>>("shared_ptr<PolyDiamondBase>")
        .constructor<>()
        .function("getClassName", &PolyDiamondBase::getClassName)
        ;
    
    class_<PolyDiamondDerived>("PolyDiamondDerived")
        .smart_ptr<std::shared_ptr<PolyDiamondDerived>>("shared_ptr<PolyDiamondDerived>")
        .constructor<>()
        .function("getClassName", &PolyDiamondDerived::getClassName)
        ;

    class_<PolyDiamondSiblingDerived>("PolyDiamondSiblingDerived")
        .smart_ptr<std::shared_ptr<PolyDiamondSiblingDerived>>("shared_ptr<PolyDiamondSiblingDerived>")
        .constructor<>()
        .function("getClassName", &PolyDiamondSiblingDerived::getClassName)
        ;

    class_<PolyDiamondMultiplyDerived>("PolyDiamondMultiplyDerived")
        .smart_ptr<std::shared_ptr<PolyDiamondMultiplyDerived>>("shared_ptr<PolyDiamondMultiplyDerived>")
        .constructor<>()
        .function("getClassName", &PolyDiamondMultiplyDerived::getClassName)
        ;

    function("embind_test_return_small_class_instance", &embind_test_return_small_class_instance);
    function("embind_test_return_big_class_instance", &embind_test_return_big_class_instance);
    function("embind_test_accept_small_class_instance", &embind_test_accept_small_class_instance);
    function("embind_test_accept_big_class_instance", &embind_test_accept_big_class_instance);

    class_<UniquePtrToConstructor>("UniquePtrToConstructor")
        .constructor<std::unique_ptr<int>>()
        .function("getValue", &UniquePtrToConstructor::getValue)
        ;

    function("embind_test_construct_class_with_unique_ptr", embind_test_construct_class_with_unique_ptr, allow_raw_pointer<ret_val>());
    function("embind_test_return_unique_ptr", embind_test_return_unique_ptr);
    function("embind_test_accept_unique_ptr", embind_test_accept_unique_ptr);

    function("embind_test_return_raw_base_ptr", embind_test_return_raw_base_ptr, allow_raw_pointer<ret_val>());
    function("embind_test_return_raw_derived_ptr_as_base", embind_test_return_raw_derived_ptr_as_base, allow_raw_pointer<ret_val>());
    function("embind_test_return_raw_sibling_derived_ptr_as_base", embind_test_return_raw_sibling_derived_ptr_as_base, allow_raw_pointer<ret_val>());
    function("embind_test_return_raw_polymorphic_derived_ptr_as_base", embind_test_return_raw_polymorphic_derived_ptr_as_base, allow_raw_pointer<ret_val>());
    function("embind_test_return_raw_polymorphic_sibling_derived_ptr_as_base", embind_test_return_raw_polymorphic_sibling_derived_ptr_as_base, allow_raw_pointer<ret_val>());
    function("embind_test_return_raw_polymorphic_multiply_derived_ptr_as_base", embind_test_return_raw_polymorphic_multiply_derived_ptr_as_base, allow_raw_pointer<ret_val>());
    function("embind_test_return_raw_polymorphic_multiply_derived_ptr_as_second_base", embind_test_return_raw_polymorphic_multiply_derived_ptr_as_second_base, allow_raw_pointer<ret_val>());
    function("embind_test_return_raw_polymorphic_derived_four_times_not_bound_as_base", embind_test_return_raw_polymorphic_derived_four_times_not_bound_as_base, allow_raw_pointer<ret_val>());
    function("embind_test_return_smart_derived_ptr", embind_test_return_smart_derived_ptr);
    function("embind_test_return_smart_sibling_derived_ptr", embind_test_return_smart_sibling_derived_ptr);
    function("embind_test_return_smart_multiply_derived_ptr", embind_test_return_smart_multiply_derived_ptr);
    function("embind_test_return_smart_derived_thrice_ptr", embind_test_return_smart_derived_thrice_ptr);
    function("embind_test_return_smart_base_ptr", embind_test_return_smart_base_ptr);
    function("embind_test_return_smart_polymorphic_base_ptr", embind_test_return_smart_polymorphic_base_ptr);
    function("embind_test_return_smart_polymorphic_derived_ptr", embind_test_return_smart_polymorphic_derived_ptr);
    function("embind_test_return_smart_polymorphic_sibling_derived_ptr", embind_test_return_smart_polymorphic_sibling_derived_ptr);
    function("embind_test_return_smart_polymorphic_multiply_derived_ptr", embind_test_return_smart_polymorphic_multiply_derived_ptr);
    function("embind_test_return_poly_derived_twice_without_smart_pointer_as_poly_base", embind_test_return_poly_derived_twice_without_smart_pointer_as_poly_base);
    function("embind_test_return_smart_poly_derived_thrice_ptr", embind_test_return_smart_poly_derived_thrice_ptr);
    function("embind_test_return_smart_derived_ptr_as_base", embind_test_return_smart_derived_ptr_as_base);
    function("embind_test_return_smart_derived_ptr_as_val", embind_test_return_smart_derived_ptr_as_val);
    function("embind_test_return_smart_polymorphic_derived_ptr_as_base", embind_test_return_smart_polymorphic_derived_ptr_as_base);
    function("embind_test_return_smart_polymorphic_sibling_derived_ptr_as_base", embind_test_return_smart_polymorphic_sibling_derived_ptr_as_base);
    function("embind_test_get_class_name_via_base_ptr", embind_test_get_class_name_via_base_ptr, allow_raw_pointer<arg<0>>());
    function("embind_test_get_class_name_via_second_base_ptr", embind_test_get_class_name_via_second_base_ptr, allow_raw_pointer<arg<0>>());
    function("embind_test_get_class_name_via_polymorphic_base_ptr", embind_test_get_class_name_via_polymorphic_base_ptr, allow_raw_pointer<arg<0>>());
    function("embind_test_get_class_name_via_polymorphic_second_base_ptr", embind_test_get_class_name_via_polymorphic_second_base_ptr, allow_raw_pointer<arg<0>>());
    // todo: allow_raw_pointer should fail earlier if argument is not a pointer
    function("embind_test_get_class_name_via_smart_base_ptr", embind_test_get_class_name_via_smart_base_ptr);
    function("embind_test_get_class_name_via_reference_to_smart_base_ptr", embind_test_get_class_name_via_reference_to_smart_base_ptr);
    function("embind_test_get_class_name_via_smart_second_base_ptr", embind_test_get_class_name_via_smart_second_base_ptr);
    function("embind_test_get_class_name_via_smart_polymorphic_base_ptr", embind_test_get_class_name_via_smart_polymorphic_base_ptr);
    function("embind_test_get_virtual_class_name_via_smart_polymorphic_base_ptr", embind_test_get_virtual_class_name_via_smart_polymorphic_base_ptr);
    function("embind_test_get_class_name_via_smart_polymorphic_second_base_ptr", embind_test_get_class_name_via_smart_polymorphic_second_base_ptr);
    function("embind_modify_smart_pointer_passed_by_reference", embind_modify_smart_pointer_passed_by_reference);
    function("embind_attempt_to_modify_smart_pointer_when_passed_by_value", embind_attempt_to_modify_smart_pointer_when_passed_by_value);
    function("embind_save_smart_base_pointer", embind_save_smart_base_pointer);

    class_<Base1>("Base1")
        .constructor()
        .function("getField", &Base1::getField)
        ;

    class_<Base2>("Base2")
        .function("getField", &Base2::getField)
        .property("field", &Base2::field2)
        ;

    class_<HasTwoBases, base<Base2>>("HasTwoBases")
        .constructor()
        ;

    class_<CustomStruct>("CustomStruct")
        .constructor<>()
        .property("field", &CustomStruct::field)
        .function("getField", &CustomStruct::getField)
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

    function("emval_test_call_function", &emval_test_call_function);

    function("emval_test_return_unique_ptr", &emval_test_return_unique_ptr);

    class_<UniquePtrLifetimeMock>("UniquePtrLifetimeMock");
    function("emval_test_return_unique_ptr_lifetime", &emval_test_return_unique_ptr_lifetime);

    function("emval_test_return_shared_ptr", &emval_test_return_shared_ptr);
    function("emval_test_return_empty_shared_ptr", &emval_test_return_empty_shared_ptr);
    function("emval_test_is_shared_ptr_null", &emval_test_is_shared_ptr_null);

    function("emval_test_return_vector", &emval_test_return_vector);
    function("emval_test_return_vector_of_vectors", &emval_test_return_vector_of_vectors);

    register_vector<std::shared_ptr<StringHolder>>("SharedPtrVector");
    function("emval_test_return_shared_ptr_vector", &emval_test_return_shared_ptr_vector);

    function("get_module_property", &get_module_property);

    register_vector<StringHolder>("StringHolderVector");
    class_<VectorHolder>("VectorHolder")
        .constructor<>()
        .function("get", &VectorHolder::get)
        .function("set", &VectorHolder::set)
        ;

    function("test_string_with_vec", &test_string_with_vec);

    register_map<std::string, int>("StringIntMap");
    function("embind_test_get_string_int_map", embind_test_get_string_int_map);

    function("embind_test_getglobal", &embind_test_getglobal);

    function("embind_test_new_Object", &embind_test_new_Object);
    function("embind_test_new_factory", &embind_test_new_factory);

    class_<HasExternalConstructor>("HasExternalConstructor")
        .constructor(&createHasExternalConstructor)
        .function("getString", &HasExternalConstructor::getString)
        ;

    auto HeldBySmartPtr_class = class_<HeldBySmartPtr>("HeldBySmartPtr");
    HeldBySmartPtr_class
        .smart_ptr<CustomSmartPtr<HeldBySmartPtr>>("CustomSmartPtr<HeldBySmartPtr>")
        .smart_ptr_constructor("shared_ptr<HeldbySmartPtr>", &std::make_shared<HeldBySmartPtr, int, std::string>)
        .class_function("newCustomPtr", HeldBySmartPtr::newCustomPtr)
        .function("returnThis", &takesHeldBySmartPtrSharedPtr)
        .property("i", &HeldBySmartPtr::i)
        .property("s", &HeldBySmartPtr::s)
        ;
    function("takesHeldBySmartPtr", &takesHeldBySmartPtr);
    function("takesHeldBySmartPtrSharedPtr", &takesHeldBySmartPtrSharedPtr);

    class_<HeldByCustomSmartPtr>("HeldByCustomSmartPtr")
        .smart_ptr<std::shared_ptr<HeldByCustomSmartPtr>>("shared_ptr<HeldByCustomSmartPtr>")
        .smart_ptr_constructor("CustomSmartPtr<HeldByCustomSmartPtr>", &HeldByCustomSmartPtr::create)
        .class_function("createSharedPtr", &HeldByCustomSmartPtr::createSharedPtr)
        .property("i", &HeldByCustomSmartPtr::i)
        .property("s", &HeldByCustomSmartPtr::s)
        ;

    function("passThroughRawPtr", &passThroughRawPtr, allow_raw_pointers());
    function("passThroughCustomSmartPtr", &passThroughCustomSmartPtr);

    function("char_to_string", &char_to_string);
    function("signed_char_to_string", &signed_char_to_string);
    function("unsigned_char_to_string", &unsigned_char_to_string);
    function("short_to_string", &short_to_string);
    function("unsigned_short_to_string", &unsigned_short_to_string);
    function("int_to_string", &int_to_string);
    function("unsigned_int_to_string", &unsigned_int_to_string);
    function("long_to_string", &long_to_string);
    function("unsigned_long_to_string", &unsigned_long_to_string);

    function("store_unsigned_char", &store_unsigned_char);
    function("load_unsigned_char", &load_unsigned_char);
    function("store_unsigned_short", &store_unsigned_short);
    function("load_unsigned_short", &load_unsigned_short);
    function("store_unsigned_int", &store_unsigned_int);
    function("load_unsigned_int", &load_unsigned_int);
    function("store_unsigned_long", &store_unsigned_long);
    function("load_unsigned_long", &load_unsigned_long);
}

int overloaded_function(int i) {
    assert(i == 10);
    return 1;
}

int overloaded_function(int i, int j) {
    assert(i == 20);
    assert(j == 20);
    return 2;
}

class MultipleCtors {
public:
    int value = 0;

    MultipleCtors(int i) {
        value = 1;
        assert(i == 10);
    }
    MultipleCtors(int i, int j) {
        value = 2;
        assert(i == 20);
        assert(j == 20);
    }
    MultipleCtors(int i, int j, int k) {
        value = 3;
        assert(i == 30);
        assert(j == 30);
        assert(k == 30);
    }

    int WhichCtorCalled() const {
        return value;
    }
};

class MultipleSmartCtors {
public:
    int value = 0;

    MultipleSmartCtors(int i) {
        value = 1;
        assert(i == 10);
    }
    MultipleSmartCtors(int i, int j) {
        value = 2;
        assert(i == 20);
        assert(j == 20);
    }

    int WhichCtorCalled() const {
        return value;
    }
};

class MultipleOverloads {
public:
    MultipleOverloads() {}
    
    int value;
    static int staticValue;
    
    int Func(int i) {
        assert(i == 10);
        value = 1;
        return 1;
    }
    int Func(int i, int j) {
        assert(i == 20);
        assert(j == 20);
        value = 2;
        return 2;
    }

    int WhichFuncCalled() const {
        return value;
    }
    
    static int StaticFunc(int i) {
        assert(i == 10);
        staticValue = 1;
        return 1;
    }
    static int StaticFunc(int i, int j) {
        assert(i == 20);
        assert(j == 20);
        staticValue = 2;
        return 2;
    }

    static int WhichStaticFuncCalled() {
        return staticValue;
    }
};

int MultipleOverloads::staticValue = 0;

class MultipleOverloadsDerived : public MultipleOverloads {
public:
    MultipleOverloadsDerived() {}
        
    int Func(int i, int j, int k) {
        assert(i == 30);
        assert(j == 30);
        assert(k == 30);
        value = 3;
        return 3;
    }
    int Func(int i, int j, int k, int l) {
        assert(i == 40);
        assert(j == 40);
        assert(k == 40);
        assert(l == 40);
        value = 4;
        return 4;
    }
    
    static int StaticFunc(int i, int j, int k) {
        assert(i == 30);
        assert(j == 30);
        assert(k == 30);
        staticValue = 3;
        return 3;
    }
    static int StaticFunc(int i, int j, int k, int l) {
        assert(i == 40);
        assert(j == 40);
        assert(k == 40);
        assert(l == 40);
        staticValue = 4;
        return 4;
    }
};

struct MultipleAccessors {
    int getConst() {
        return 1;
    }
    int getConst() const {
        return 2;
    }
    int getConst(int i) const {
        return i;
    }
};

struct ConstAndNonConst {
    void method(int) {
    }

    int method() const {
        return 10;
    }
};

EMSCRIPTEN_BINDINGS(overloads) {
    function("overloaded_function", select_overload<int(int)>(&overloaded_function));
    function("overloaded_function", select_overload<int(int, int)>(&overloaded_function));

    class_<MultipleCtors>("MultipleCtors")
        .constructor<int>()
        .constructor<int, int>()
        .constructor<int, int, int>()
        .function("WhichCtorCalled", &MultipleCtors::WhichCtorCalled)
        ;
        
    class_<MultipleSmartCtors>("MultipleSmartCtors")
        .smart_ptr<std::shared_ptr<MultipleSmartCtors>>("shared_ptr<MultipleSmartCtors>")
        .constructor(&std::make_shared<MultipleSmartCtors, int>)
        .constructor(&std::make_shared<MultipleSmartCtors, int, int>)
        .function("WhichCtorCalled", &MultipleSmartCtors::WhichCtorCalled)
        ;
        
    class_<MultipleOverloads>("MultipleOverloads")
        .constructor<>()
        .function("Func", select_overload<int(int)>(&MultipleOverloads::Func))
        .function("Func", select_overload<int(int, int)>(&MultipleOverloads::Func))
        .function("WhichFuncCalled", &MultipleOverloads::WhichFuncCalled)
        .class_function("StaticFunc", select_overload<int(int)>(&MultipleOverloads::StaticFunc))
        .class_function("StaticFunc", select_overload<int(int,int)>(&MultipleOverloads::StaticFunc))
        .class_function("WhichStaticFuncCalled", &MultipleOverloads::WhichStaticFuncCalled)
        ;

    class_<MultipleOverloadsDerived, base<MultipleOverloads> >("MultipleOverloadsDerived")
        .constructor<>()
        .function("Func", select_overload<int(int,int,int)>(&MultipleOverloadsDerived::Func))
        .function("Func", select_overload<int(int,int,int,int)>(&MultipleOverloadsDerived::Func))
        .class_function("StaticFunc", select_overload<int(int,int,int)>(&MultipleOverloadsDerived::StaticFunc))
        .class_function("StaticFunc", select_overload<int(int,int,int,int)>(&MultipleOverloadsDerived::StaticFunc))
        ;

    class_<MultipleAccessors>("MultipleAccessors")
        .function("getConst", select_overload<int(int)const>(&MultipleAccessors::getConst))
        ;

    class_<ConstAndNonConst>("ConstAndNonConst")
        .function("method", select_const(&ConstAndNonConst::method))
        ;
}

// tests for out-of-order registration

class SecondElement {
};

class FirstElement {
};

struct OrderedTuple {
    FirstElement first;
    SecondElement second;
};

struct OrderedStruct {
    FirstElement first;
    SecondElement second;
};

OrderedTuple getOrderedTuple() {
    return OrderedTuple();
}

OrderedStruct getOrderedStruct() {
    return OrderedStruct();
}

EMSCRIPTEN_BINDINGS(order) {
    value_array<OrderedTuple>("OrderedTuple")
        .element(&OrderedTuple::first)
        .element(&OrderedTuple::second)
        ;

    value_object<OrderedStruct>("OrderedStruct")
        .field("first", &OrderedStruct::first)
        .field("second", &OrderedStruct::second)
        ;
    
    class_<SecondElement>("SecondElement")
        ;

    class_<FirstElement>("FirstElement")
        ;

    function("getOrderedTuple", &getOrderedTuple);
    function("getOrderedStruct", &getOrderedStruct);
}

// tests for unbound types

template<typename T>
T passThrough(T t) {
    return t;
}

struct UnboundClass {
};

struct HasUnboundBase : public UnboundClass {
    static void noop() {
    }
};

HasUnboundBase getHasUnboundBase(HasUnboundBase f) {
    return f;
}

struct HasConstructorUsingUnboundArgument {
    HasConstructorUsingUnboundArgument(UnboundClass) {
    }
};

struct SecondUnboundClass {
};

struct HasConstructorUsingUnboundArgumentAndUnboundBase : public SecondUnboundClass {
    HasConstructorUsingUnboundArgumentAndUnboundBase(UnboundClass) {
    }
};

struct BoundClass {
    UnboundClass method(UnboundClass t) {
        return t;
    }

    static UnboundClass classfunction(UnboundClass t) {
        return t;
    }

    UnboundClass property;
};

EMSCRIPTEN_BINDINGS(incomplete) {
    constant("hasUnboundTypeNames", emscripten::has_unbound_type_names);

    function("getUnboundClass", &passThrough<UnboundClass>);

    class_<HasUnboundBase, base<UnboundClass>>("HasUnboundBase")
        .class_function("noop", &HasUnboundBase::noop)
        ;
    function("getHasUnboundBase", &passThrough<HasUnboundBase>);

    class_<HasConstructorUsingUnboundArgument>("HasConstructorUsingUnboundArgument")
        .constructor<UnboundClass>()
        ;

    class_<HasConstructorUsingUnboundArgumentAndUnboundBase, base<SecondUnboundClass>>("HasConstructorUsingUnboundArgumentAndUnboundBase")
        .constructor<UnboundClass>()
        ;

    class_<BoundClass>("BoundClass")
        .constructor<>()
        .function("method", &BoundClass::method)
        .class_function("classfunction", &BoundClass::classfunction)
        .property("property", &BoundClass::property)
        ;
}

class Noncopyable {
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
    
public:
    Noncopyable() {}
    Noncopyable(Noncopyable&& other) {
        other.valid = false;
    }

    std::string method() const {
        return "foo";
    }

    bool valid = true;
};

Noncopyable getNoncopyable() {
    return Noncopyable();
}

EMSCRIPTEN_BINDINGS(noncopyable) {
    class_<Noncopyable>("Noncopyable")
        .constructor<>()
        .function("method", &Noncopyable::method)
        ;

    function("getNoncopyable", &getNoncopyable);
}

struct HasReadOnlyProperty {
    HasReadOnlyProperty(int i)
        : i(i)
    {}

    const int i;
};

EMSCRIPTEN_BINDINGS(read_only_properties) {
    class_<HasReadOnlyProperty>("HasReadOnlyProperty")
        .constructor<int>()
        .property("i", &HasReadOnlyProperty::i)
        ;
}

struct StaticConstIntStruct {
    static const int STATIC_CONST_INTEGER_VALUE_1;
    static const int STATIC_CONST_INTEGER_VALUE_1000;
};

const int StaticConstIntStruct::STATIC_CONST_INTEGER_VALUE_1 = 1;
const int StaticConstIntStruct::STATIC_CONST_INTEGER_VALUE_1000 = 1000;

EMSCRIPTEN_BINDINGS(constants) {
    constant("INT_CONSTANT", 10);

    constant("STATIC_CONST_INTEGER_VALUE_1", StaticConstIntStruct::STATIC_CONST_INTEGER_VALUE_1);
    constant("STATIC_CONST_INTEGER_VALUE_1000", StaticConstIntStruct::STATIC_CONST_INTEGER_VALUE_1000);

    constant("STRING_CONSTANT", std::string("some string"));

    TupleVector tv(1, 2, 3, 4);
    constant("VALUE_ARRAY_CONSTANT", tv);

    StructVector sv(1, 2, 3, 4);
    constant("VALUE_OBJECT_CONSTANT", sv);
}

class DerivedWithOffset : public DummyDataToTestPointerAdjustment, public Base {    
};

std::shared_ptr<Base> return_Base_from_DerivedWithOffset(std::shared_ptr<DerivedWithOffset> ptr) {
    return ptr;
}

EMSCRIPTEN_BINDINGS(with_adjustment) {
    class_<DerivedWithOffset, base<Base>>("DerivedWithOffset")
        .smart_ptr_constructor("shared_ptr<DerivedWithOffset>", &std::make_shared<DerivedWithOffset>)
        ;

    function("return_Base_from_DerivedWithOffset", &return_Base_from_DerivedWithOffset);
}

void clear_StringHolder(StringHolder& sh) {
    sh.set("");
}

EMSCRIPTEN_BINDINGS(references) {
    function("clear_StringHolder", &clear_StringHolder);
}

StringHolder return_StringHolder_copy(val func) {
    return func.as<StringHolder>();
}

StringHolder call_StringHolder_func(val func) {
    return func().as<StringHolder>();
}

EMSCRIPTEN_BINDINGS(return_values) {
    function("return_StringHolder_copy", &return_StringHolder_copy);
    function("call_StringHolder_func", &call_StringHolder_func);
}


struct Mixin {
    int get10() const {
        return 10;
    }
};

template<typename ClassBinding>
const ClassBinding& registerMixin(const ClassBinding& binding) {
    // need a wrapper for implicit conversion from DerivedWithMixin to Mixin
    struct Local {
        static int get10(const typename ClassBinding::class_type& self) {
            return self.get10();
        }
    };

    return binding
        .function("get10", &Local::get10)
        ;
}

class DerivedWithMixin : public Base, public Mixin {
};

EMSCRIPTEN_BINDINGS(mixins) {
    registerMixin(
        class_<DerivedWithMixin, base<Base>>("DerivedWithMixin")
            .constructor<>()
    );
}

template<typename T>
T val_as(const val& v) {
    return v.as<T>();
}

EMSCRIPTEN_BINDINGS(val_as) {
    function("val_as_bool",   &val_as<bool>);
    function("val_as_char",   &val_as<char>);
    function("val_as_short",  &val_as<short>);
    function("val_as_int",    &val_as<int>);
    function("val_as_long",   &val_as<long>);

    function("val_as_float",  &val_as<float>);
    function("val_as_double", &val_as<double>);

    function("val_as_string", &val_as<std::string>);
    function("val_as_wstring", &val_as<std::wstring>);
    function("val_as_val", &val_as<val>);

    function("val_as_value_object", &val_as<StructVector>);
    function("val_as_value_array", &val_as<TupleVector>);

    function("val_as_enum", &val_as<Enum>);

    // memory_view is always JS -> C++
    //function("val_as_memory_view", &val_as<memory_view>);
}

val construct_with_6(val factory) {
    unsigned char a1 = 6;
    double a2 = -12.5;
    std::string a3("a3");
    StructVector a4(1, 2, 3, 4);
    EnumClass a5 = EnumClass::TWO;
    TupleVector a6(-1, -2, -3, -4);
    return factory.new_(a1, a2, a3, a4, a5, a6);
}

val construct_with_memory_view(val factory) {
    static const char data[11] = "0123456789";
    return factory.new_(
        std::string("before"),
        typed_memory_view(10, data),
        std::string("after"));
}

val construct_with_ints_and_float(val factory) {
    return factory.new_(65537, 4.0f, 65538);
}

EMSCRIPTEN_BINDINGS(val_new_) {
    function("construct_with_6_arguments", &construct_with_6);
    function("construct_with_memory_view", &construct_with_memory_view);
    function("construct_with_ints_and_float", &construct_with_ints_and_float);
}

template <typename T>
class intrusive_ptr {
public:
    typedef T element_type;

    intrusive_ptr(std::nullptr_t = nullptr)
        : px(nullptr)
    {}

    template <typename U>
    explicit intrusive_ptr(U* px)
        : px(px)
    {
        addRef(px);
    }

    intrusive_ptr(const intrusive_ptr& that)
        : px(that.px)
    {
        addRef(px);
    }

    template<typename U>
    intrusive_ptr(const intrusive_ptr<U>& that)
        : px(that.get())
    {
        addRef(px);
    }

    intrusive_ptr& operator=(const intrusive_ptr& that) {
        reset(that.get());
        return *this;
    }

    intrusive_ptr& operator=(intrusive_ptr&& that) {
        release(px);
        px = that.px;
        that.px = 0;
        return *this;
    }

    template<typename U>
    intrusive_ptr& operator=(const intrusive_ptr<U>& that) {
        reset(that.get());
        return *this;
    }

    template<typename U>
    intrusive_ptr& operator=(intrusive_ptr<U>&& that) {
        release(px);
        px = that.px;
        that.px = 0;
        return *this;
    }

    ~intrusive_ptr() {
        release(px);
    }

    void reset(T* nx = nullptr) {
        addRef(nx);
        release(px);
        px = nx;
    }

    T* get() const {
        return px;
    }

    T& operator*() const {
        return *px;
    }

    T* operator->() const {
        return px;
    }

    explicit operator bool() const {
        return px != nullptr;
    }

    void swap(intrusive_ptr& rhs) {
        std::swap(px, rhs.px);
    }

private:
    void addRef(T* px) {
        if (px) {
            ++px->referenceCount;
        }
    }

    void release(T* px) {
        if (--px->referenceCount == 0) {
            delete px;
        }
    }

    T* px;

    template<typename U>
    friend class intrusive_ptr;
};

namespace emscripten {
    template<typename T>
    struct smart_ptr_trait<intrusive_ptr<T>> {
        typedef intrusive_ptr<T> pointer_type;
        typedef T element_type;

        static sharing_policy get_sharing_policy() {
            return sharing_policy::INTRUSIVE;
        }

        static T* get(const intrusive_ptr<T>& p) {
            return p.get();
        }

        static intrusive_ptr<T> share(const intrusive_ptr<T>& r, T* ptr) {
            return intrusive_ptr<T>(ptr);
        }

        static pointer_type* construct_null() {
            return new pointer_type;
        }
    };
}

template<typename T>
intrusive_ptr<T> make_intrusive_ptr() {
    return intrusive_ptr<T>(new T);
}

struct IntrusiveClass {
    virtual ~IntrusiveClass() {}
    long referenceCount = 0;
};

struct IntrusiveClassWrapper : public wrapper<IntrusiveClass> {
    EMSCRIPTEN_WRAPPER(IntrusiveClassWrapper);
};

template<typename T>
struct Holder {
    void set(const T& v) {
        value = v;
    }
    const T& get() const {
        return value;
    }
    T value;
};

EMSCRIPTEN_BINDINGS(intrusive_pointers) {
    class_<IntrusiveClass>("IntrusiveClass")
        .smart_ptr_constructor("intrusive_ptr<IntrusiveClass>", &make_intrusive_ptr<IntrusiveClass>)
        .allow_subclass<IntrusiveClassWrapper, intrusive_ptr<IntrusiveClassWrapper>>("IntrusiveClassWrapper", "IntrusiveClassWrapperPtr")
        ;

    typedef Holder<intrusive_ptr<IntrusiveClass>> IntrusiveClassHolder;
    class_<IntrusiveClassHolder>("IntrusiveClassHolder")
        .constructor<>()
        .function("set", &IntrusiveClassHolder::set)
        .function("get", &IntrusiveClassHolder::get)
        ;

    function("passThroughIntrusiveClass", &passThrough<intrusive_ptr<IntrusiveClass>>);
}

std::string getTypeOfVal(const val& v) {
    return v.typeof().as<std::string>();
}

EMSCRIPTEN_BINDINGS(typeof) {
    function("getTypeOfVal", &getTypeOfVal);
}

struct HasStaticMember {
    static const int c;
    static int v;
};

const int HasStaticMember::c = 10;
int HasStaticMember::v = 20;

EMSCRIPTEN_BINDINGS(static_member) {
    class_<HasStaticMember>("HasStaticMember")
        .class_property("c", &HasStaticMember::c)
        .class_property("v", &HasStaticMember::v)
        ;
}
