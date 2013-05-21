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

std::string emval_test_take_and_return_const_char_star(const char* str) {
    return str;
}

std::string emval_test_take_and_return_std_string(std::string str) {
    return str;
}

std::string emval_test_take_and_return_std_string_const_ref(const std::string& str) {
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

enum class EnumClass { ONE, TWO };

EnumClass emval_test_take_and_return_EnumClass(EnumClass e) {
    return e;
}

void emval_test_call_function(val v, int i, float f, TupleVector tv, StructVector sv) {
    v(i, f, tv, sv);
}

std::unique_ptr<ValHolder> emval_test_return_unique_ptr() {
    return std::unique_ptr<ValHolder>(new ValHolder(val::object()));
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
};

EMSCRIPTEN_SYMBOL(optionalMethod);

class AbstractClassWrapper : public wrapper<AbstractClass> {
public:
    EMSCRIPTEN_WRAPPER(AbstractClassWrapper);

    std::string abstractMethod() const {
        return call<std::string>("abstractMethod");
    }

    std::string optionalMethod(std::string s) const {
        return optional_call<std::string>(optionalMethod_symbol, [&] {
            return AbstractClass::optionalMethod(s);
        }, s);
    }

    std::shared_ptr<Derived> returnsSharedPtr() {
        return call<std::shared_ptr<Derived> >("returnsSharedPtr");
    }

    void differentArguments(int i, double d, unsigned char f, double q, std::string s) {
        return call<void>("differentArguments", i, d, f, q, s);
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

EMSCRIPTEN_BINDINGS(interface_tests) {
    class_<AbstractClass>("AbstractClass")
        .smart_ptr<std::shared_ptr<AbstractClass>>()
        .allow_subclass<AbstractClassWrapper>()
        .function("abstractMethod", &AbstractClass::abstractMethod)
        .function("optionalMethod", &AbstractClass::optionalMethod)
        ;
    
    function("getAbstractClass", &getAbstractClass);
    function("callAbstractMethod", &callAbstractMethod);
    function("callOptionalMethod", &callOptionalMethod);
    function("callReturnsSharedPtrMethod", &callReturnsSharedPtrMethod);
    function("callDifferentArguments", &callDifferentArguments);
}

template<typename T, size_t sizeOfArray>
constexpr size_t getElementCount(T (&)[sizeOfArray]) {
    return sizeOfArray;
}

static void callWithMemoryView(val v) {
    // static so the JS test can read the memory after callTakeMemoryView runs
    static unsigned char data[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    v(memory_view(getElementCount(data), data));
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

    function("emval_test_as_unsigned", &emval_test_as_unsigned);
    function("emval_test_get_length", &emval_test_get_length);
    function("emval_test_add", &emval_test_add);
    function("const_ref_adder", &const_ref_adder);
    function("emval_test_sum", &emval_test_sum);

    function("get_non_ascii_string", &get_non_ascii_string);
    function("get_non_ascii_wstring", &get_non_ascii_wstring);
    //function("emval_test_take_and_return_const_char_star", &emval_test_take_and_return_const_char_star);
    function("emval_test_take_and_return_std_string", &emval_test_take_and_return_std_string);
    function("emval_test_take_and_return_std_string_const_ref", &emval_test_take_and_return_std_string_const_ref);
    function("take_and_return_std_wstring", &take_and_return_std_wstring);

    //function("emval_test_take_and_return_CustomStruct", &emval_test_take_and_return_CustomStruct);

    value_tuple<TupleVector>("TupleVector")
        .element(&TupleVector::x)
        .element(&Vector::getY, &Vector::setY)
        .element(&readVectorZ, &writeVectorZ)
        .element(index<3>())
        ;

    function("emval_test_return_TupleVector", &emval_test_return_TupleVector);
    function("emval_test_take_and_return_TupleVector", &emval_test_take_and_return_TupleVector);

    value_tuple<TupleVectorTuple>("TupleVectorTuple")
        .element(&TupleVectorTuple::v)
        ;

    function("emval_test_return_TupleVectorTuple", &emval_test_return_TupleVectorTuple);

    value_struct<StructVector>("StructVector")
        .field("x", &StructVector::x)
        .field("y", &Vector::getY, &Vector::setY)
        .field("z", &readVectorZ, &writeVectorZ)
        .field("w", index<3>())
        ;

    function("emval_test_return_StructVector", &emval_test_return_StructVector);
    function("emval_test_take_and_return_StructVector", &emval_test_take_and_return_StructVector);

    value_struct<TupleInStruct>("TupleInStruct")
        .field("field", &TupleInStruct::field)
        ;

    function("emval_test_take_and_return_TupleInStruct", &emval_test_take_and_return_TupleInStruct);

    class_<ValHolder>("ValHolder")
        .smart_ptr<std::shared_ptr<ValHolder>>()
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
        .smart_ptr<std::shared_ptr<StringHolder>>()
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
        .smart_ptr<std::shared_ptr<Derived>>()
        .constructor<>()
        .function("getClassName", &Derived::getClassName)
        .function("getMember", &Derived::getMember)
        .function("setMember", &Derived::setMember)
        .property("member", &Derived::member)
        ;

    class_<Base>("Base")
        .smart_ptr<std::shared_ptr<Base>>()
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
        .smart_ptr<std::shared_ptr<SecondBase>>()
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
        .smart_ptr<std::shared_ptr<SiblingDerived>>()
        .constructor<>()
        .function("getClassName", &SiblingDerived::getClassName)
        ;
    
    class_<MultiplyDerived, base<Base>>("MultiplyDerived")
        .smart_ptr<std::shared_ptr<MultiplyDerived>>()
        .constructor<>()
        .function("getClassName", &MultiplyDerived::getClassName)
        .class_function("getInstanceCount", &MultiplyDerived::getInstanceCount)
        ;

    class_<DerivedTwice, base<Derived> >("DerivedTwice")
        .constructor<>()
        .function("getClassName", &DerivedTwice::getClassName)
        ;

    class_<DerivedThrice, base<Derived> >("DerivedThrice")
        .smart_ptr<std::shared_ptr<DerivedThrice>>()
        .constructor<>()
        .function("getClassName", &DerivedThrice::getClassName)
        ;

    class_<PolyBase>("PolyBase")
        .smart_ptr<std::shared_ptr<PolyBase>>()
        .constructor<>()
        .function("virtualGetClassName", &PolyBase::virtualGetClassName)
        .function("getClassName", &PolyBase::getClassName)
        ;

    class_<PolySecondBase>("PolySecondBase")
        .smart_ptr<std::shared_ptr<PolySecondBase>>()
        .constructor<>()
        .function("getClassName", &PolySecondBase::getClassName)
        ;

    class_<PolyDerived, base<PolyBase>>("PolyDerived")
        .smart_ptr<std::shared_ptr<PolyDerived>>()
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
        .smart_ptr<std::shared_ptr<PolySiblingDerived>>()
        .constructor<>()
        .function("getClassName", &PolySiblingDerived::getClassName)
        ;

    class_<PolyMultiplyDerived, base<PolyBase>>("PolyMultiplyDerived")
        .smart_ptr<std::shared_ptr<PolyMultiplyDerived>>()
        .constructor<>()
        .function("getClassName", &PolyMultiplyDerived::getClassName)
        ;

    class_<PolyDerivedThrice, base<PolyDerived>>("PolyDerivedThrice")
        .smart_ptr<std::shared_ptr<PolyDerivedThrice>>()
        .constructor<>()
        .function("getClassName", &PolyDerivedThrice::getClassName)
        ;

    class_<PolyDiamondBase>("PolyDiamondBase")
        .smart_ptr<std::shared_ptr<PolyDiamondBase>>()
        .constructor<>()
        .function("getClassName", &PolyDiamondBase::getClassName)
        ;
    
    class_<PolyDiamondDerived>("PolyDiamondDerived")
        .smart_ptr<std::shared_ptr<PolyDiamondDerived>>()
        .constructor<>()
        .function("getClassName", &PolyDiamondDerived::getClassName)
        ;

    class_<PolyDiamondSiblingDerived>("PolyDiamondSiblingDerived")
        .smart_ptr<std::shared_ptr<PolyDiamondSiblingDerived>>()
        .constructor<>()
        .function("getClassName", &PolyDiamondSiblingDerived::getClassName)
        ;

    class_<PolyDiamondMultiplyDerived>("PolyDiamondMultiplyDerived")
        .smart_ptr<std::shared_ptr<PolyDiamondMultiplyDerived>>()
        .constructor<>()
        .function("getClassName", &PolyDiamondMultiplyDerived::getClassName)
        ;

    function("embind_test_return_small_class_instance", &embind_test_return_small_class_instance);
    function("embind_test_return_big_class_instance", &embind_test_return_big_class_instance);
    function("embind_test_accept_small_class_instance", &embind_test_accept_small_class_instance);
    function("embind_test_accept_big_class_instance", &embind_test_accept_big_class_instance);

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

    function("embind_test_new_Object", &embind_test_new_Object);
    function("embind_test_new_factory", &embind_test_new_factory);

    class_<HasExternalConstructor>("HasExternalConstructor")
        .constructor(&createHasExternalConstructor)
        .function("getString", &HasExternalConstructor::getString)
        ;

    auto HeldBySmartPtr_class = class_<HeldBySmartPtr>("HeldBySmartPtr");
    HeldBySmartPtr_class
        .smart_ptr<CustomSmartPtr<HeldBySmartPtr>>()
        .smart_ptr_constructor(&std::make_shared<HeldBySmartPtr, int, std::string>)
        .class_function("newCustomPtr", HeldBySmartPtr::newCustomPtr)
        .function("returnThis", &takesHeldBySmartPtrSharedPtr)
        .property("i", &HeldBySmartPtr::i)
        .property("s", &HeldBySmartPtr::s)
        ;
    function("takesHeldBySmartPtr", &takesHeldBySmartPtr);
    function("takesHeldBySmartPtrSharedPtr", &takesHeldBySmartPtrSharedPtr);

    class_<HeldByCustomSmartPtr>("HeldByCustomSmartPtr")
        .smart_ptr<std::shared_ptr<HeldByCustomSmartPtr>>()
        .smart_ptr_constructor(&HeldByCustomSmartPtr::create)
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
        .smart_ptr<std::shared_ptr<MultipleSmartCtors>>()
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
    value_tuple<OrderedTuple>("OrderedTuple")
        .element(&OrderedTuple::first)
        .element(&OrderedTuple::second)
        ;

    value_struct<OrderedStruct>("OrderedStruct")
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

EMSCRIPTEN_BINDINGS(constants) {
    constant("INT_CONSTANT", 10);
    constant("STRING_CONSTANT", std::string("some string"));

    TupleVector tv(1, 2, 3, 4);
    constant("VALUE_TUPLE_CONSTANT", tv);

    StructVector sv(1, 2, 3, 4);
    constant("VALUE_STRUCT_CONSTANT", sv);
}

class DerivedWithOffset : public DummyDataToTestPointerAdjustment, public Base {    
};

std::shared_ptr<Base> return_Base_from_DerivedWithOffset(std::shared_ptr<DerivedWithOffset> ptr) {
    return ptr;
}

EMSCRIPTEN_BINDINGS(with_adjustment) {
    class_<DerivedWithOffset, base<Base>>("DerivedWithOffset")
        .smart_ptr_constructor(&std::make_shared<DerivedWithOffset>)
        ;

    function("return_Base_from_DerivedWithOffset", &return_Base_from_DerivedWithOffset);
}
