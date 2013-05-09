#include "xmlvm.h"
#include "java_io_PrintStream.h"
#include "java_lang_Integer.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_System.h"
#include "nbody_NBodySystem.h"

#include "nbody_nbody.h"

#define XMLVM_CURRENT_CLASS_NAME nbody
#define XMLVM_CURRENT_PKG_CLASS_NAME nbody_nbody

__TIB_DEFINITION_nbody_nbody __TIB_nbody_nbody = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_nbody_nbody, // classInitializer
    "nbody.nbody", // className
    "nbody", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(nbody_nbody), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_nbody_nbody;
JAVA_OBJECT __CLASS_nbody_nbody_1ARRAY;
JAVA_OBJECT __CLASS_nbody_nbody_2ARRAY;
JAVA_OBJECT __CLASS_nbody_nbody_3ARRAY;
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

void __INIT_nbody_nbody()
{
    staticInitializerLock(&__TIB_nbody_nbody);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_nbody_nbody.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_nbody_nbody.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_nbody_nbody);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_nbody_nbody.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_nbody_nbody.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_nbody_nbody.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("nbody.nbody")
        __INIT_IMPL_nbody_nbody();
    }
}

void __INIT_IMPL_nbody_nbody()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_nbody_nbody.newInstanceFunc = __NEW_INSTANCE_nbody_nbody;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_nbody_nbody.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    // Initialize interface information
    __TIB_nbody_nbody.numImplementedInterfaces = 0;
    __TIB_nbody_nbody.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 0);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    __TIB_nbody_nbody.declaredFields = &__field_reflection_data[0];
    __TIB_nbody_nbody.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_nbody_nbody.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_nbody_nbody.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_nbody_nbody.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_nbody_nbody.methodDispatcherFunc = method_dispatcher;
    __TIB_nbody_nbody.declaredMethods = &__method_reflection_data[0];
    __TIB_nbody_nbody.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_nbody_nbody = XMLVM_CREATE_CLASS_OBJECT(&__TIB_nbody_nbody);
    __TIB_nbody_nbody.clazz = __CLASS_nbody_nbody;
    __TIB_nbody_nbody.baseType = JAVA_NULL;
    __CLASS_nbody_nbody_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_nbody_nbody);
    __CLASS_nbody_nbody_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_nbody_nbody_1ARRAY);
    __CLASS_nbody_nbody_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_nbody_nbody_2ARRAY);
    //XMLVM_BEGIN_WRAPPER[__INIT_nbody_nbody]
    //XMLVM_END_WRAPPER

    __TIB_nbody_nbody.classInitialized = 1;
}

void __DELETE_nbody_nbody(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_nbody_nbody]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_nbody_nbody(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_nbody_nbody]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_nbody_nbody()
{    XMLVM_CLASS_INIT(nbody_nbody)
nbody_nbody* me = (nbody_nbody*) XMLVM_MALLOC(sizeof(nbody_nbody));
    me->tib = &__TIB_nbody_nbody;
    __INIT_INSTANCE_MEMBERS_nbody_nbody(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_nbody_nbody]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_nbody_nbody()
{
    JAVA_OBJECT me = JAVA_NULL;
    me = __NEW_nbody_nbody();
    nbody_nbody___INIT___(me);
    return me;
}

void nbody_nbody___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[nbody_nbody___INIT___]
    XMLVM_ENTER_METHOD("nbody.nbody", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("nbody.java", 15)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void nbody_nbody_main___java_lang_String_1ARRAY(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(nbody_nbody)
    //XMLVM_BEGIN_WRAPPER[nbody_nbody_main___java_lang_String_1ARRAY]
    XMLVM_ENTER_METHOD("nbody.nbody", "main", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r7.o = n1;
    _r4.i = 0;
    XMLVM_SOURCE_POSITION("nbody.java", 17)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r7.o));
    if (_r0.i <= 0) goto label32;
    XMLVM_CHECK_NPE(7)
    XMLVM_CHECK_ARRAY_BOUNDS(_r7.o, _r4.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r7.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    _r0.i = java_lang_Integer_parseInt___java_lang_String(_r0.o);
    label10:;
    XMLVM_SOURCE_POSITION("nbody.java", 19)
    _r1.l = java_lang_System_currentTimeMillis__();
    XMLVM_SOURCE_POSITION("nbody.java", 21)
    _r3.o = __NEW_nbody_NBodySystem();
    XMLVM_CHECK_NPE(3)
    nbody_NBodySystem___INIT___(_r3.o);
    label19:;
    XMLVM_SOURCE_POSITION("nbody.java", 23)
    if (_r4.i >= _r0.i) goto label36;
    XMLVM_SOURCE_POSITION("nbody.java", 24)
    _r5.d = 0.01;
    XMLVM_CHECK_NPE(3)
    nbody_NBodySystem_advance___double(_r3.o, _r5.d);
    _r4.i = _r4.i + 1;
    goto label19;
    label32:;
    _r0.i = 5000000;
    goto label10;
    label36:;
    XMLVM_SOURCE_POSITION("nbody.java", 27)
    _r4.l = java_lang_System_currentTimeMillis__();
    XMLVM_SOURCE_POSITION("nbody.java", 29)
    _r0.l = _r4.l - _r1.l;
    _r0.d = (JAVA_DOUBLE) _r0.l;
    _r4.d = 0.001;
    _r0.d = _r0.d * _r4.d;
    XMLVM_SOURCE_POSITION("nbody.java", 31)
    _r2.o = java_lang_System_GET_out();
    XMLVM_CHECK_NPE(3)
    _r3.d = nbody_NBodySystem_energy__(_r3.o);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_println___double(_r2.o, _r3.d);
    XMLVM_SOURCE_POSITION("nbody.java", 32)
    _r2.o = java_lang_System_GET_out();
    _r3.o = __NEW_java_lang_StringBuilder();
    XMLVM_CHECK_NPE(3)
    java_lang_StringBuilder___INIT___(_r3.o);
    // "Time(s): "
    _r4.o = xmlvm_create_java_string_from_pool(813);
    XMLVM_CHECK_NPE(3)
    _r3.o = java_lang_StringBuilder_append___java_lang_String(_r3.o, _r4.o);
    XMLVM_CHECK_NPE(3)
    _r0.o = java_lang_StringBuilder_append___double(_r3.o, _r0.d);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_CHECK_NPE(2)
    java_io_PrintStream_println___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("nbody.java", 33)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}



//#if 0

#include <pthread.h> // for pthread_exit so the main thread doesn't terminate early

int main(int argc, char* argv[])
{
    // translate our normalized argument (0-0, 1-0.1secs, 2-0.5secs, 3-1sec, 4-5secs, 5-10secs) to nbody
    int arg = argc > 1 ? argv[1][0] - '0' : 3;
    char buffer[100];
    argv[1] = buffer;
    int n;
    switch(arg) {
      case 0: return 0; break;
      case 1: n = 600000; break;
      case 2: n = 3600000; break;
      case 3: n = 6550000; break;
      case 4: n = 30000000; break;
      case 5: n = 60000000; break;
      default: printf("error: %d\\n", arg); return -1;
    }
    snprintf(buffer, 50, "%d", n);

    xmlvm_init();

    // Initialize the main thread before entering XMLVM_SETJMP
    java_lang_Thread* mainThread = java_lang_Thread_currentThread__();
    if (XMLVM_SETJMP(xmlvm_exception_env_main_thread)) {
        // Technically, XMLVM_UNWIND_EXCEPTION() should be called, but
        // exceptions will not be used anymore and XMLVM_ENTER_METHOD() wasn't
        // called (excessive), so a compilation error would occur

        xmlvm_unhandled_exception();
    } else {
        // Convert command-line args to String[]. First C-arg is omitted.
        JAVA_OBJECT args = xmlvm_create_java_string_array(argc-1, argv+1);
        nbody_nbody_main___java_lang_String_1ARRAY(args);
    }

    xmlvm_destroy(mainThread);

    return 0;
}
//#endif

