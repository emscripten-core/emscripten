#include "xmlvm.h"

#include "nbody_Body.h"

#define XMLVM_CURRENT_CLASS_NAME Body
#define XMLVM_CURRENT_PKG_CLASS_NAME nbody_Body

__TIB_DEFINITION_nbody_Body __TIB_nbody_Body = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_nbody_Body, // classInitializer
    "nbody.Body", // className
    "nbody", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(nbody_Body), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_nbody_Body;
JAVA_OBJECT __CLASS_nbody_Body_1ARRAY;
JAVA_OBJECT __CLASS_nbody_Body_2ARRAY;
JAVA_OBJECT __CLASS_nbody_Body_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_DOUBLE _STATIC_nbody_Body_PI;
static JAVA_DOUBLE _STATIC_nbody_Body_SOLAR_MASS;
static JAVA_DOUBLE _STATIC_nbody_Body_DAYS_PER_YEAR;

#include "xmlvm-reflection.h"

static XMLVM_FIELD_REFLECTION_DATA __field_reflection_data[] = {
};

static XMLVM_CONSTRUCTOR_REFLECTION_DATA __constructor_reflection_data[] = {
};

static JAVA_OBJECT constructor_dispatcher(JAVA_OBJECT constructor, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

static XMLVM_METHOD_REFLECTION_DATA __method_reflection_data[] = {
};

static JAVA_OBJECT method_dispatcher(JAVA_OBJECT method, JAVA_OBJECT receiver, JAVA_OBJECT arguments)
{
    XMLVM_NOT_IMPLEMENTED();
}

void __INIT_nbody_Body()
{
    staticInitializerLock(&__TIB_nbody_Body);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_nbody_Body.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_nbody_Body.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_nbody_Body);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_nbody_Body.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_nbody_Body.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_nbody_Body.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("nbody.Body")
        __INIT_IMPL_nbody_Body();
    }
}

void __INIT_IMPL_nbody_Body()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_nbody_Body.newInstanceFunc = __NEW_INSTANCE_nbody_Body;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_nbody_Body.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_nbody_Body.numImplementedInterfaces = 0;
    __TIB_nbody_Body.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces
    _STATIC_nbody_Body_PI = 3.141592653589793;
    _STATIC_nbody_Body_SOLAR_MASS = 39.47841760435743;
    _STATIC_nbody_Body_DAYS_PER_YEAR = 365.24;

    __TIB_nbody_Body.declaredFields = &__field_reflection_data[0];
    __TIB_nbody_Body.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_nbody_Body.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_nbody_Body.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_nbody_Body.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_nbody_Body.methodDispatcherFunc = method_dispatcher;
    __TIB_nbody_Body.declaredMethods = &__method_reflection_data[0];
    __TIB_nbody_Body.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_nbody_Body = XMLVM_CREATE_CLASS_OBJECT(&__TIB_nbody_Body);
    __TIB_nbody_Body.clazz = __CLASS_nbody_Body;
    __TIB_nbody_Body.baseType = JAVA_NULL;
    __CLASS_nbody_Body_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_nbody_Body);
    __CLASS_nbody_Body_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_nbody_Body_1ARRAY);
    __CLASS_nbody_Body_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_nbody_Body_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_nbody_Body]
    //XMLVM_END_WRAPPER

    __TIB_nbody_Body.classInitialized = 1;
}

void __DELETE_nbody_Body(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_nbody_Body]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_nbody_Body(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((nbody_Body*) me)->fields.nbody_Body.x_ = 0;
    ((nbody_Body*) me)->fields.nbody_Body.y_ = 0;
    ((nbody_Body*) me)->fields.nbody_Body.z_ = 0;
    ((nbody_Body*) me)->fields.nbody_Body.vx_ = 0;
    ((nbody_Body*) me)->fields.nbody_Body.vy_ = 0;
    ((nbody_Body*) me)->fields.nbody_Body.vz_ = 0;
    ((nbody_Body*) me)->fields.nbody_Body.mass_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_nbody_Body]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_nbody_Body()
{    XMLVM_CLASS_INIT(nbody_Body)
nbody_Body* me = (nbody_Body*) XMLVM_MALLOC(sizeof(nbody_Body));
    me->tib = &__TIB_nbody_Body;
    __INIT_INSTANCE_MEMBERS_nbody_Body(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_nbody_Body]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_nbody_Body()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_nbody_Body();
    nbody_Body___INIT___(me);
    return me;
}

JAVA_DOUBLE nbody_Body_GET_PI()
{
    XMLVM_CLASS_INIT(nbody_Body)
    return _STATIC_nbody_Body_PI;
}

void nbody_Body_PUT_PI(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(nbody_Body)
_STATIC_nbody_Body_PI = v;
}

JAVA_DOUBLE nbody_Body_GET_SOLAR_MASS()
{
    XMLVM_CLASS_INIT(nbody_Body)
    return _STATIC_nbody_Body_SOLAR_MASS;
}

void nbody_Body_PUT_SOLAR_MASS(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(nbody_Body)
_STATIC_nbody_Body_SOLAR_MASS = v;
}

JAVA_DOUBLE nbody_Body_GET_DAYS_PER_YEAR()
{
    XMLVM_CLASS_INIT(nbody_Body)
    return _STATIC_nbody_Body_DAYS_PER_YEAR;
}

void nbody_Body_PUT_DAYS_PER_YEAR(JAVA_DOUBLE v)
{
    XMLVM_CLASS_INIT(nbody_Body)
_STATIC_nbody_Body_DAYS_PER_YEAR = v;
}

void nbody_Body___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[nbody_Body___INIT___]
    XMLVM_ENTER_METHOD("nbody.Body", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("nbody.java", 122)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT nbody_Body_jupiter__()
{
    XMLVM_CLASS_INIT(nbody_Body)
    //XMLVM_BEGIN_WRAPPER[nbody_Body_jupiter__]
    XMLVM_ENTER_METHOD("nbody.Body", "jupiter", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVM_SOURCE_POSITION("nbody.java", 125)
    _r0.o = __NEW_nbody_Body();
    XMLVM_CHECK_NPE(0)
    nbody_Body___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("nbody.java", 126)
    _r1.d = 4.841431442464721;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.x_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 127)
    _r1.d = -1.1603200440274284;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.y_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 128)
    _r1.d = -0.10362204447112311;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.z_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 129)
    _r1.d = 0.606326392995832;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vx_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 130)
    _r1.d = 2.81198684491626;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vy_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 131)
    _r1.d = -0.02521836165988763;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vz_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 132)
    _r1.d = 0.03769367487038949;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.mass_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 133)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT nbody_Body_saturn__()
{
    XMLVM_CLASS_INIT(nbody_Body)
    //XMLVM_BEGIN_WRAPPER[nbody_Body_saturn__]
    XMLVM_ENTER_METHOD("nbody.Body", "saturn", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVM_SOURCE_POSITION("nbody.java", 137)
    _r0.o = __NEW_nbody_Body();
    XMLVM_CHECK_NPE(0)
    nbody_Body___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("nbody.java", 138)
    _r1.d = 8.34336671824458;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.x_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 139)
    _r1.d = 4.124798564124305;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.y_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 140)
    _r1.d = -0.4035234171143214;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.z_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 141)
    _r1.d = -1.0107743461787924;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vx_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 142)
    _r1.d = 1.8256623712304119;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vy_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 143)
    _r1.d = 0.008415761376584154;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vz_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 144)
    _r1.d = 0.011286326131968767;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.mass_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 145)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT nbody_Body_uranus__()
{
    XMLVM_CLASS_INIT(nbody_Body)
    //XMLVM_BEGIN_WRAPPER[nbody_Body_uranus__]
    XMLVM_ENTER_METHOD("nbody.Body", "uranus", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVM_SOURCE_POSITION("nbody.java", 149)
    _r0.o = __NEW_nbody_Body();
    XMLVM_CHECK_NPE(0)
    nbody_Body___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("nbody.java", 150)
    _r1.d = 12.894369562139131;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.x_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 151)
    _r1.d = -15.111151401698631;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.y_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 152)
    _r1.d = -0.22330757889265573;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.z_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 153)
    _r1.d = 1.0827910064415354;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vx_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 154)
    _r1.d = 0.8687130181696082;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vy_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 155)
    _r1.d = -0.010832637401363636;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vz_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 156)
    _r1.d = 0.0017237240570597112;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.mass_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 157)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT nbody_Body_neptune__()
{
    XMLVM_CLASS_INIT(nbody_Body)
    //XMLVM_BEGIN_WRAPPER[nbody_Body_neptune__]
    XMLVM_ENTER_METHOD("nbody.Body", "neptune", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVM_SOURCE_POSITION("nbody.java", 161)
    _r0.o = __NEW_nbody_Body();
    XMLVM_CHECK_NPE(0)
    nbody_Body___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("nbody.java", 162)
    _r1.d = 15.379697114850917;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.x_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 163)
    _r1.d = -25.919314609987964;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.y_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 164)
    _r1.d = 0.17925877295037118;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.z_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 165)
    _r1.d = 0.979090732243898;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vx_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 166)
    _r1.d = 0.5946989986476762;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vy_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 167)
    _r1.d = -0.034755955504078104;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.vz_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 168)
    _r1.d = 0.0020336868699246304;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.mass_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 169)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT nbody_Body_sun__()
{
    XMLVM_CLASS_INIT(nbody_Body)
    //XMLVM_BEGIN_WRAPPER[nbody_Body_sun__]
    XMLVM_ENTER_METHOD("nbody.Body", "sun", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVM_SOURCE_POSITION("nbody.java", 173)
    _r0.o = __NEW_nbody_Body();
    XMLVM_CHECK_NPE(0)
    nbody_Body___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("nbody.java", 174)
    _r1.d = 39.47841760435743;
    XMLVM_CHECK_NPE(0)
    ((nbody_Body*) _r0.o)->fields.nbody_Body.mass_ = _r1.d;
    XMLVM_SOURCE_POSITION("nbody.java", 175)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT nbody_Body_offsetMomentum___double_double_double(JAVA_OBJECT me, JAVA_DOUBLE n1, JAVA_DOUBLE n2, JAVA_DOUBLE n3)
{
    //XMLVM_BEGIN_WRAPPER[nbody_Body_offsetMomentum___double_double_double]
    XMLVM_ENTER_METHOD("nbody.Body", "offsetMomentum", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    XMLVMElem _r9;
    XMLVMElem _r10;
    _r4.o = me;
    _r5.d = n1;
    _r7.d = n2;
    _r9.d = n3;
    _r2.d = 39.47841760435743;
    XMLVM_SOURCE_POSITION("nbody.java", 179)
    _r0.d = -_r5.d;
    _r0.d = _r0.d / _r2.d;
    XMLVM_CHECK_NPE(4)
    ((nbody_Body*) _r4.o)->fields.nbody_Body.vx_ = _r0.d;
    XMLVM_SOURCE_POSITION("nbody.java", 180)
    _r0.d = -_r7.d;
    _r0.d = _r0.d / _r2.d;
    XMLVM_CHECK_NPE(4)
    ((nbody_Body*) _r4.o)->fields.nbody_Body.vy_ = _r0.d;
    XMLVM_SOURCE_POSITION("nbody.java", 181)
    _r0.d = -_r9.d;
    _r0.d = _r0.d / _r2.d;
    XMLVM_CHECK_NPE(4)
    ((nbody_Body*) _r4.o)->fields.nbody_Body.vz_ = _r0.d;
    XMLVM_SOURCE_POSITION("nbody.java", 182)
    XMLVM_EXIT_METHOD()
    return _r4.o;
    //XMLVM_END_WRAPPER
}

