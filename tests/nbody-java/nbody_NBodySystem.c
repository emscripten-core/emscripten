#include "xmlvm.h"
#include "java_lang_Math.h"
#include "nbody_Body.h"

#include "nbody_NBodySystem.h"

#define XMLVM_CURRENT_CLASS_NAME NBodySystem
#define XMLVM_CURRENT_PKG_CLASS_NAME nbody_NBodySystem

__TIB_DEFINITION_nbody_NBodySystem __TIB_nbody_NBodySystem = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_nbody_NBodySystem, // classInitializer
    "nbody.NBodySystem", // className
    "nbody", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(nbody_NBodySystem), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_nbody_NBodySystem;
JAVA_OBJECT __CLASS_nbody_NBodySystem_1ARRAY;
JAVA_OBJECT __CLASS_nbody_NBodySystem_2ARRAY;
JAVA_OBJECT __CLASS_nbody_NBodySystem_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION


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

void __INIT_nbody_NBodySystem()
{
    staticInitializerLock(&__TIB_nbody_NBodySystem);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_nbody_NBodySystem.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_nbody_NBodySystem.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_nbody_NBodySystem);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_nbody_NBodySystem.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_nbody_NBodySystem.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_nbody_NBodySystem.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("nbody.NBodySystem")
        __INIT_IMPL_nbody_NBodySystem();
    }
}

void __INIT_IMPL_nbody_NBodySystem()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_nbody_NBodySystem.newInstanceFunc = __NEW_INSTANCE_nbody_NBodySystem;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_nbody_NBodySystem.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_nbody_NBodySystem.numImplementedInterfaces = 0;
    __TIB_nbody_NBodySystem.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_nbody_NBodySystem.declaredFields = &__field_reflection_data[0];
    __TIB_nbody_NBodySystem.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_nbody_NBodySystem.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_nbody_NBodySystem.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_nbody_NBodySystem.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_nbody_NBodySystem.methodDispatcherFunc = method_dispatcher;
    __TIB_nbody_NBodySystem.declaredMethods = &__method_reflection_data[0];
    __TIB_nbody_NBodySystem.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_nbody_NBodySystem = XMLVM_CREATE_CLASS_OBJECT(&__TIB_nbody_NBodySystem);
    __TIB_nbody_NBodySystem.clazz = __CLASS_nbody_NBodySystem;
    __TIB_nbody_NBodySystem.baseType = JAVA_NULL;
    __CLASS_nbody_NBodySystem_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_nbody_NBodySystem);
    __CLASS_nbody_NBodySystem_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_nbody_NBodySystem_1ARRAY);
    __CLASS_nbody_NBodySystem_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_nbody_NBodySystem_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_nbody_NBodySystem]
    //XMLVM_END_WRAPPER

    __TIB_nbody_NBodySystem.classInitialized = 1;
}

void __DELETE_nbody_NBodySystem(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_nbody_NBodySystem]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_nbody_NBodySystem(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((nbody_NBodySystem*) me)->fields.nbody_NBodySystem.bodies_ = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_nbody_NBodySystem]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_nbody_NBodySystem()
{    XMLVM_CLASS_INIT(nbody_NBodySystem)
nbody_NBodySystem* me = (nbody_NBodySystem*) XMLVM_MALLOC(sizeof(nbody_NBodySystem));
    me->tib = &__TIB_nbody_NBodySystem;
    __INIT_INSTANCE_MEMBERS_nbody_NBodySystem(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_nbody_NBodySystem]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_nbody_NBodySystem()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_nbody_NBodySystem();
    nbody_NBodySystem___INIT___(me);
    return me;
}

void nbody_NBodySystem___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[nbody_NBodySystem___INIT___]
    XMLVM_ENTER_METHOD("nbody.NBodySystem", "<init>", "?")
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
    XMLVMElem _r11;
    XMLVMElem _r12;
    _r12.o = me;
    _r11.i = 0;
    _r3.d = 0.0;
    XMLVM_SOURCE_POSITION("nbody.java", 39)
    XMLVM_CHECK_NPE(12)
    java_lang_Object___INIT___(_r12.o);
    XMLVM_SOURCE_POSITION("nbody.java", 40)
    _r0.i = 5;
    XMLVM_CLASS_INIT(nbody_Body)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_nbody_Body, _r0.i);
    _r1.o = nbody_Body_sun__();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r11.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r11.i] = _r1.o;
    _r1.i = 1;
    _r2.o = nbody_Body_jupiter__();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 2;
    _r2.o = nbody_Body_saturn__();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 3;
    _r2.o = nbody_Body_uranus__();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    _r1.i = 4;
    _r2.o = nbody_Body_neptune__();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.o;
    XMLVM_CHECK_NPE(12)
    ((nbody_NBodySystem*) _r12.o)->fields.nbody_NBodySystem.bodies_ = _r0.o;
    _r0 = _r11;
    _r5 = _r3;
    _r1 = _r3;
    label48:;
    XMLVM_SOURCE_POSITION("nbody.java", 51)
    XMLVM_CHECK_NPE(12)
    _r7.o = ((nbody_NBodySystem*) _r12.o)->fields.nbody_NBodySystem.bodies_;
    _r7.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r7.o));
    if (_r0.i >= _r7.i) goto label98;
    XMLVM_SOURCE_POSITION("nbody.java", 52)
    XMLVM_CHECK_NPE(12)
    _r7.o = ((nbody_NBodySystem*) _r12.o)->fields.nbody_NBodySystem.bodies_;
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r0.i);
    _r7.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(7)
    _r7.d = ((nbody_Body*) _r7.o)->fields.nbody_Body.vx_;
    XMLVM_CHECK_NPE(12)
    _r9.o = ((nbody_NBodySystem*) _r12.o)->fields.nbody_NBodySystem.bodies_;
    XMLVM_CHECK_NPE(9)
    XMLVM_CHECK_ARRAY_BOUNDS(_r9.o, _r0.i);
    _r9.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r9.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(9)
    _r9.d = ((nbody_Body*) _r9.o)->fields.nbody_Body.mass_;
    _r7.d = _r7.d * _r9.d;
    _r1.d = _r1.d + _r7.d;
    XMLVM_SOURCE_POSITION("nbody.java", 53)
    XMLVM_CHECK_NPE(12)
    _r7.o = ((nbody_NBodySystem*) _r12.o)->fields.nbody_NBodySystem.bodies_;
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r0.i);
    _r7.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(7)
    _r7.d = ((nbody_Body*) _r7.o)->fields.nbody_Body.vy_;
    XMLVM_CHECK_NPE(12)
    _r9.o = ((nbody_NBodySystem*) _r12.o)->fields.nbody_NBodySystem.bodies_;
    XMLVM_CHECK_NPE(9)
    XMLVM_CHECK_ARRAY_BOUNDS(_r9.o, _r0.i);
    _r9.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r9.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(9)
    _r9.d = ((nbody_Body*) _r9.o)->fields.nbody_Body.mass_;
    _r7.d = _r7.d * _r9.d;
    _r3.d = _r3.d + _r7.d;
    XMLVM_SOURCE_POSITION("nbody.java", 54)
    XMLVM_CHECK_NPE(12)
    _r7.o = ((nbody_NBodySystem*) _r12.o)->fields.nbody_NBodySystem.bodies_;
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r0.i);
    _r7.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(7)
    _r7.d = ((nbody_Body*) _r7.o)->fields.nbody_Body.vz_;
    XMLVM_CHECK_NPE(12)
    _r9.o = ((nbody_NBodySystem*) _r12.o)->fields.nbody_NBodySystem.bodies_;
    XMLVM_CHECK_NPE(9)
    XMLVM_CHECK_ARRAY_BOUNDS(_r9.o, _r0.i);
    _r9.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r9.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_CHECK_NPE(9)
    _r9.d = ((nbody_Body*) _r9.o)->fields.nbody_Body.mass_;
    _r7.d = _r7.d * _r9.d;
    _r5.d = _r5.d + _r7.d;
    _r0.i = _r0.i + 1;
    goto label48;
    label98:;
    XMLVM_SOURCE_POSITION("nbody.java", 56)
    XMLVM_CHECK_NPE(12)
    _r0.o = ((nbody_NBodySystem*) _r12.o)->fields.nbody_NBodySystem.bodies_;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r11.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r11.i];
    XMLVM_CHECK_NPE(0)
    nbody_Body_offsetMomentum___double_double_double(_r0.o, _r1.d, _r3.d, _r5.d);
    XMLVM_SOURCE_POSITION("nbody.java", 57)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void nbody_NBodySystem_advance___double(JAVA_OBJECT me, JAVA_DOUBLE n1)
{
    //XMLVM_BEGIN_WRAPPER[nbody_NBodySystem_advance___double]
    XMLVM_ENTER_METHOD("nbody.NBodySystem", "advance", "?")
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
    XMLVMElem _r11;
    XMLVMElem _r12;
    XMLVMElem _r13;
    XMLVMElem _r14;
    XMLVMElem _r15;
    XMLVMElem _r16;
    XMLVMElem _r17;
    XMLVMElem _r18;
    XMLVMElem _r19;
    _r17.o = me;
    _r18.d = n1;
    XMLVM_SOURCE_POSITION("nbody.java", 61)
    _r1.i = 0;
    label1:;
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r2 = _r0;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r2.o));
    if (_r1.i >= _r2.i) goto label183;
    XMLVM_SOURCE_POSITION("nbody.java", 62)
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r2 = _r0;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r1.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("nbody.java", 63)
    _r3.i = _r1.i + 1;
    label18:;
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r4 = _r0;
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    if (_r3.i >= _r4.i) goto label179;
    XMLVM_SOURCE_POSITION("nbody.java", 64)
    XMLVM_CHECK_NPE(2)
    _r4.d = ((nbody_Body*) _r2.o)->fields.nbody_Body.x_;
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r6 = _r0;
    XMLVM_CHECK_NPE(6)
    XMLVM_CHECK_ARRAY_BOUNDS(_r6.o, _r3.i);
    _r6.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r6.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(6)
    _r6.d = ((nbody_Body*) _r6.o)->fields.nbody_Body.x_;
    _r4.d = _r4.d - _r6.d;
    XMLVM_SOURCE_POSITION("nbody.java", 65)
    XMLVM_CHECK_NPE(2)
    _r6.d = ((nbody_Body*) _r2.o)->fields.nbody_Body.y_;
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r8 = _r0;
    XMLVM_CHECK_NPE(8)
    XMLVM_CHECK_ARRAY_BOUNDS(_r8.o, _r3.i);
    _r8.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r8.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(8)
    _r8.d = ((nbody_Body*) _r8.o)->fields.nbody_Body.y_;
    _r6.d = _r6.d - _r8.d;
    XMLVM_SOURCE_POSITION("nbody.java", 66)
    XMLVM_CHECK_NPE(2)
    _r8.d = ((nbody_Body*) _r2.o)->fields.nbody_Body.z_;
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r10 = _r0;
    XMLVM_CHECK_NPE(10)
    XMLVM_CHECK_ARRAY_BOUNDS(_r10.o, _r3.i);
    _r10.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r10.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(10)
    _r10.d = ((nbody_Body*) _r10.o)->fields.nbody_Body.z_;
    _r8.d = _r8.d - _r10.d;
    _r10.d = _r4.d * _r4.d;
    _r12.d = _r6.d * _r6.d;
    _r10.d = _r10.d + _r12.d;
    _r12.d = _r8.d * _r8.d;
    _r10.d = _r10.d + _r12.d;
    XMLVM_SOURCE_POSITION("nbody.java", 69)
    _r12.d = java_lang_Math_sqrt___double(_r10.d);
    XMLVM_SOURCE_POSITION("nbody.java", 70)
    _r10.d = _r10.d * _r12.d;
    _r10.d = _r18.d / _r10.d;
    XMLVM_SOURCE_POSITION("nbody.java", 72)
    XMLVM_CHECK_NPE(2)
    _r12.d = ((nbody_Body*) _r2.o)->fields.nbody_Body.vx_;
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r14 = _r0;
    XMLVM_CHECK_NPE(14)
    XMLVM_CHECK_ARRAY_BOUNDS(_r14.o, _r3.i);
    _r14.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r14.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(14)
    _r14.d = ((nbody_Body*) _r14.o)->fields.nbody_Body.mass_;
    _r14.d = _r14.d * _r4.d;
    _r14.d = _r14.d * _r10.d;
    _r12.d = _r12.d - _r14.d;
    XMLVM_CHECK_NPE(2)
    ((nbody_Body*) _r2.o)->fields.nbody_Body.vx_ = _r12.d;
    XMLVM_SOURCE_POSITION("nbody.java", 73)
    XMLVM_CHECK_NPE(2)
    _r12.d = ((nbody_Body*) _r2.o)->fields.nbody_Body.vy_;
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r14 = _r0;
    XMLVM_CHECK_NPE(14)
    XMLVM_CHECK_ARRAY_BOUNDS(_r14.o, _r3.i);
    _r14.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r14.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(14)
    _r14.d = ((nbody_Body*) _r14.o)->fields.nbody_Body.mass_;
    _r14.d = _r14.d * _r6.d;
    _r14.d = _r14.d * _r10.d;
    _r12.d = _r12.d - _r14.d;
    XMLVM_CHECK_NPE(2)
    ((nbody_Body*) _r2.o)->fields.nbody_Body.vy_ = _r12.d;
    XMLVM_SOURCE_POSITION("nbody.java", 74)
    XMLVM_CHECK_NPE(2)
    _r12.d = ((nbody_Body*) _r2.o)->fields.nbody_Body.vz_;
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r14 = _r0;
    XMLVM_CHECK_NPE(14)
    XMLVM_CHECK_ARRAY_BOUNDS(_r14.o, _r3.i);
    _r14.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r14.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(14)
    _r14.d = ((nbody_Body*) _r14.o)->fields.nbody_Body.mass_;
    _r14.d = _r14.d * _r8.d;
    _r14.d = _r14.d * _r10.d;
    _r12.d = _r12.d - _r14.d;
    XMLVM_CHECK_NPE(2)
    ((nbody_Body*) _r2.o)->fields.nbody_Body.vz_ = _r12.d;
    XMLVM_SOURCE_POSITION("nbody.java", 76)
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r12 = _r0;
    XMLVM_CHECK_NPE(12)
    XMLVM_CHECK_ARRAY_BOUNDS(_r12.o, _r3.i);
    _r12.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r12.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(12)
    _r13.d = ((nbody_Body*) _r12.o)->fields.nbody_Body.vx_;
    XMLVM_CHECK_NPE(2)
    _r15.d = ((nbody_Body*) _r2.o)->fields.nbody_Body.mass_;
    _r4.d = _r4.d * _r15.d;
    _r4.d = _r4.d * _r10.d;
    _r4.d = _r4.d + _r13.d;
    XMLVM_CHECK_NPE(12)
    ((nbody_Body*) _r12.o)->fields.nbody_Body.vx_ = _r4.d;
    XMLVM_SOURCE_POSITION("nbody.java", 77)
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r4 = _r0;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r3.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(4)
    _r12.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.vy_;
    XMLVM_CHECK_NPE(2)
    _r14.d = ((nbody_Body*) _r2.o)->fields.nbody_Body.mass_;
    _r5.d = _r6.d * _r14.d;
    _r5.d = _r5.d * _r10.d;
    _r5.d = _r5.d + _r12.d;
    XMLVM_CHECK_NPE(4)
    ((nbody_Body*) _r4.o)->fields.nbody_Body.vy_ = _r5.d;
    XMLVM_SOURCE_POSITION("nbody.java", 78)
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r4 = _r0;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r3.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_CHECK_NPE(4)
    _r5.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.vz_;
    XMLVM_CHECK_NPE(2)
    _r12.d = ((nbody_Body*) _r2.o)->fields.nbody_Body.mass_;
    _r7.d = _r8.d * _r12.d;
    _r7.d = _r7.d * _r10.d;
    _r5.d = _r5.d + _r7.d;
    XMLVM_CHECK_NPE(4)
    ((nbody_Body*) _r4.o)->fields.nbody_Body.vz_ = _r5.d;
    _r3.i = _r3.i + 1;
    goto label18;
    label179:;
    _r1.i = _r1.i + 1;
    goto label1;
    label183:;
    XMLVM_SOURCE_POSITION("nbody.java", 82)
    _r0 = _r17;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r1 = _r0;
    _r2.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    _r3.i = 0;
    label190:;
    if (_r3.i >= _r2.i) goto label224;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r3.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    XMLVM_SOURCE_POSITION("nbody.java", 83)
    XMLVM_CHECK_NPE(4)
    _r5.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.x_;
    XMLVM_CHECK_NPE(4)
    _r7.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.vx_;
    _r7.d = _r7.d * _r18.d;
    _r5.d = _r5.d + _r7.d;
    XMLVM_CHECK_NPE(4)
    ((nbody_Body*) _r4.o)->fields.nbody_Body.x_ = _r5.d;
    XMLVM_SOURCE_POSITION("nbody.java", 84)
    XMLVM_CHECK_NPE(4)
    _r5.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.y_;
    XMLVM_CHECK_NPE(4)
    _r7.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.vy_;
    _r7.d = _r7.d * _r18.d;
    _r5.d = _r5.d + _r7.d;
    XMLVM_CHECK_NPE(4)
    ((nbody_Body*) _r4.o)->fields.nbody_Body.y_ = _r5.d;
    XMLVM_SOURCE_POSITION("nbody.java", 85)
    XMLVM_CHECK_NPE(4)
    _r5.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.z_;
    XMLVM_CHECK_NPE(4)
    _r7.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.vz_;
    _r7.d = _r7.d * _r18.d;
    _r5.d = _r5.d + _r7.d;
    XMLVM_CHECK_NPE(4)
    ((nbody_Body*) _r4.o)->fields.nbody_Body.z_ = _r5.d;
    _r3.i = _r3.i + 1;
    goto label190;
    label224:;
    XMLVM_SOURCE_POSITION("nbody.java", 87)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_DOUBLE nbody_NBodySystem_energy__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[nbody_NBodySystem_energy__]
    XMLVM_ENTER_METHOD("nbody.NBodySystem", "energy", "?")
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
    XMLVMElem _r11;
    XMLVMElem _r12;
    XMLVMElem _r13;
    XMLVMElem _r14;
    XMLVMElem _r15;
    XMLVMElem _r16;
    XMLVMElem _r17;
    XMLVMElem _r18;
    _r18.o = me;
    XMLVM_SOURCE_POSITION("nbody.java", 91)
    _r1.d = 0.0;
    _r3.i = 0;
    _r15 = _r3;
    _r16 = _r1;
    _r2 = _r16;
    _r1 = _r15;
    label9:;
    XMLVM_SOURCE_POSITION("nbody.java", 93)
    _r0 = _r18;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r4 = _r0;
    _r4.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    if (_r1.i >= _r4.i) goto label108;
    XMLVM_SOURCE_POSITION("nbody.java", 94)
    _r0 = _r18;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r4 = _r0;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r1.i);
    _r4.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("nbody.java", 95)
    _r5.d = 0.5;
    XMLVM_CHECK_NPE(4)
    _r7.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.mass_;
    _r5.d = _r5.d * _r7.d;
    XMLVM_CHECK_NPE(4)
    _r7.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.vx_;
    XMLVM_CHECK_NPE(4)
    _r9.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.vx_;
    _r7.d = _r7.d * _r9.d;
    XMLVM_CHECK_NPE(4)
    _r9.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.vy_;
    XMLVM_CHECK_NPE(4)
    _r11.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.vy_;
    _r9.d = _r9.d * _r11.d;
    _r7.d = _r7.d + _r9.d;
    XMLVM_CHECK_NPE(4)
    _r9.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.vz_;
    XMLVM_CHECK_NPE(4)
    _r11.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.vz_;
    _r9.d = _r9.d * _r11.d;
    _r7.d = _r7.d + _r9.d;
    _r5.d = _r5.d * _r7.d;
    _r2.d = _r2.d + _r5.d;
    _r5.i = _r1.i + 1;
    _r15 = _r5;
    _r5 = _r2;
    _r2 = _r15;
    label53:;
    XMLVM_SOURCE_POSITION("nbody.java", 100)
    _r0 = _r18;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r3 = _r0;
    _r3.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    if (_r2.i >= _r3.i) goto label104;
    XMLVM_SOURCE_POSITION("nbody.java", 101)
    _r0 = _r18;
    XMLVM_CHECK_NPE(0)
    _r0.o = ((nbody_NBodySystem*) _r0.o)->fields.nbody_NBodySystem.bodies_;
    _r3 = _r0;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r2.i);
    _r3.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("nbody.java", 102)
    XMLVM_CHECK_NPE(4)
    _r7.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.x_;
    XMLVM_CHECK_NPE(3)
    _r9.d = ((nbody_Body*) _r3.o)->fields.nbody_Body.x_;
    _r7.d = _r7.d - _r9.d;
    XMLVM_SOURCE_POSITION("nbody.java", 103)
    XMLVM_CHECK_NPE(4)
    _r9.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.y_;
    XMLVM_CHECK_NPE(3)
    _r11.d = ((nbody_Body*) _r3.o)->fields.nbody_Body.y_;
    _r9.d = _r9.d - _r11.d;
    XMLVM_SOURCE_POSITION("nbody.java", 104)
    XMLVM_CHECK_NPE(4)
    _r11.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.z_;
    XMLVM_CHECK_NPE(3)
    _r13.d = ((nbody_Body*) _r3.o)->fields.nbody_Body.z_;
    _r11.d = _r11.d - _r13.d;
    _r7.d = _r7.d * _r7.d;
    _r9.d = _r9.d * _r9.d;
    _r7.d = _r7.d + _r9.d;
    _r9.d = _r11.d * _r11.d;
    _r7.d = _r7.d + _r9.d;
    XMLVM_SOURCE_POSITION("nbody.java", 106)
    _r7.d = java_lang_Math_sqrt___double(_r7.d);
    XMLVM_SOURCE_POSITION("nbody.java", 107)
    XMLVM_CHECK_NPE(4)
    _r9.d = ((nbody_Body*) _r4.o)->fields.nbody_Body.mass_;
    XMLVM_CHECK_NPE(3)
    _r11.d = ((nbody_Body*) _r3.o)->fields.nbody_Body.mass_;
    _r9.d = _r9.d * _r11.d;
    _r7.d = _r9.d / _r7.d;
    _r5.d = _r5.d - _r7.d;
    _r2.i = _r2.i + 1;
    goto label53;
    label104:;
    _r1.i = _r1.i + 1;
    _r2 = _r5;
    goto label9;
    label108:;
    XMLVM_SOURCE_POSITION("nbody.java", 110)
    XMLVM_EXIT_METHOD()
    return _r2.d;
    //XMLVM_END_WRAPPER
}

