#include "xmlvm.h"
#include "java_lang_Double.h"
#include "java_lang_Float.h"
#include "org_apache_harmony_luni_platform_Endianness.h"

#include "org_apache_harmony_luni_platform_OSMemory.h"

#define XMLVM_CURRENT_CLASS_NAME OSMemory
#define XMLVM_CURRENT_PKG_CLASS_NAME org_apache_harmony_luni_platform_OSMemory

__TIB_DEFINITION_org_apache_harmony_luni_platform_OSMemory __TIB_org_apache_harmony_luni_platform_OSMemory = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_org_apache_harmony_luni_platform_OSMemory, // classInitializer
    "org.apache.harmony.luni.platform.OSMemory", // className
    "org.apache.harmony.luni.platform", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    JAVA_NULL, // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(org_apache_harmony_luni_platform_OSMemory), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_OSMemory;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_OSMemory_1ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_OSMemory_2ARRAY;
JAVA_OBJECT __CLASS_org_apache_harmony_luni_platform_OSMemory_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_INT _STATIC_org_apache_harmony_luni_platform_OSMemory_POINTER_SIZE;
static JAVA_OBJECT _STATIC_org_apache_harmony_luni_platform_OSMemory_NATIVE_ORDER;
static JAVA_OBJECT _STATIC_org_apache_harmony_luni_platform_OSMemory_singleton;

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

void __INIT_org_apache_harmony_luni_platform_OSMemory()
{
    staticInitializerLock(&__TIB_org_apache_harmony_luni_platform_OSMemory);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_org_apache_harmony_luni_platform_OSMemory.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_org_apache_harmony_luni_platform_OSMemory.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_org_apache_harmony_luni_platform_OSMemory);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_org_apache_harmony_luni_platform_OSMemory.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_org_apache_harmony_luni_platform_OSMemory.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_org_apache_harmony_luni_platform_OSMemory.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("org.apache.harmony.luni.platform.OSMemory")
        __INIT_IMPL_org_apache_harmony_luni_platform_OSMemory();
    }
}

void __INIT_IMPL_org_apache_harmony_luni_platform_OSMemory()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_org_apache_harmony_luni_platform_OSMemory.newInstanceFunc = __NEW_INSTANCE_org_apache_harmony_luni_platform_OSMemory;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_org_apache_harmony_luni_platform_OSMemory.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[22] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_isLittleEndian__;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[19] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_getPointerSize__;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[25] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[7] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[27] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[26] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[9] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[30] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[10] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[31] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[20] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[21] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_getShort___long_org_apache_harmony_luni_platform_Endianness;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[40] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[41] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_setShort___long_short_org_apache_harmony_luni_platform_Endianness;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[15] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[16] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_getInt___long_org_apache_harmony_luni_platform_Endianness;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[36] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[37] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_setInt___long_int_org_apache_harmony_luni_platform_Endianness;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[17] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[18] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_getLong___long_org_apache_harmony_luni_platform_Endianness;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[38] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[39] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_setLong___long_long_org_apache_harmony_luni_platform_Endianness;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[13] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[14] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_getFloat___long_org_apache_harmony_luni_platform_Endianness;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[34] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[35] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_setFloat___long_float_org_apache_harmony_luni_platform_Endianness;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[11] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[12] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_getDouble___long_org_apache_harmony_luni_platform_Endianness;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[32] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[33] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_setDouble___long_double_org_apache_harmony_luni_platform_Endianness;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[8] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[29] = (VTABLE_PTR) &xmlvm_unimplemented_native_method;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[28] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_mmap___long_long_long_int;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[42] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_unmap___long_long;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[24] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_load___long_long;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[23] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_isLoaded___long_long;
    __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[6] = (VTABLE_PTR) &org_apache_harmony_luni_platform_OSMemory_flush___long_long;
    xmlvm_init_native_org_apache_harmony_luni_platform_OSMemory();
    // Initialize interface information
    __TIB_org_apache_harmony_luni_platform_OSMemory.numImplementedInterfaces = 1;
    __TIB_org_apache_harmony_luni_platform_OSMemory.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_IMemorySystem)

    __TIB_org_apache_harmony_luni_platform_OSMemory.implementedInterfaces[0][0] = &__TIB_org_apache_harmony_luni_platform_IMemorySystem;
    // Initialize itable for this class
    __TIB_org_apache_harmony_luni_platform_OSMemory.itableBegin = &__TIB_org_apache_harmony_luni_platform_OSMemory.itable[0];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_flush___long_long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[6];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_free___long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[7];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getAddress___long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[8];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getByteArray___long_byte_1ARRAY_int_int] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[9];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getByte___long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[10];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getDouble___long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[11];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getDouble___long_org_apache_harmony_luni_platform_Endianness] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[12];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getFloat___long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[13];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getFloat___long_org_apache_harmony_luni_platform_Endianness] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[14];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getInt___long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[15];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getInt___long_org_apache_harmony_luni_platform_Endianness] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[16];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getLong___long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[17];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getLong___long_org_apache_harmony_luni_platform_Endianness] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[18];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getPointerSize__] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[19];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getShort___long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[20];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_getShort___long_org_apache_harmony_luni_platform_Endianness] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[21];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_isLittleEndian__] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[22];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_isLoaded___long_long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[23];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_load___long_long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[24];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_malloc___long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[25];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_memmove___long_long_long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[26];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_memset___long_byte_long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[27];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_mmap___long_long_long_int] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[28];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setAddress___long_long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[29];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setByteArray___long_byte_1ARRAY_int_int] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[30];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setByte___long_byte] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[31];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setDouble___long_double] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[32];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setDouble___long_double_org_apache_harmony_luni_platform_Endianness] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[33];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setFloat___long_float] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[34];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setFloat___long_float_org_apache_harmony_luni_platform_Endianness] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[35];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setInt___long_int] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[36];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setInt___long_int_org_apache_harmony_luni_platform_Endianness] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[37];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setLong___long_long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[38];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setLong___long_long_org_apache_harmony_luni_platform_Endianness] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[39];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setShort___long_short] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[40];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_setShort___long_short_org_apache_harmony_luni_platform_Endianness] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[41];
    __TIB_org_apache_harmony_luni_platform_OSMemory.itable[XMLVM_ITABLE_IDX_org_apache_harmony_luni_platform_IMemorySystem_unmap___long_long] = __TIB_org_apache_harmony_luni_platform_OSMemory.vtable[42];

    _STATIC_org_apache_harmony_luni_platform_OSMemory_POINTER_SIZE = 0;
    _STATIC_org_apache_harmony_luni_platform_OSMemory_NATIVE_ORDER = (org_apache_harmony_luni_platform_Endianness*) JAVA_NULL;
    _STATIC_org_apache_harmony_luni_platform_OSMemory_singleton = (org_apache_harmony_luni_platform_OSMemory*) JAVA_NULL;

    __TIB_org_apache_harmony_luni_platform_OSMemory.declaredFields = &__field_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_OSMemory.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_org_apache_harmony_luni_platform_OSMemory.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_org_apache_harmony_luni_platform_OSMemory.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_OSMemory.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_org_apache_harmony_luni_platform_OSMemory.methodDispatcherFunc = method_dispatcher;
    __TIB_org_apache_harmony_luni_platform_OSMemory.declaredMethods = &__method_reflection_data[0];
    __TIB_org_apache_harmony_luni_platform_OSMemory.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_org_apache_harmony_luni_platform_OSMemory = XMLVM_CREATE_CLASS_OBJECT(&__TIB_org_apache_harmony_luni_platform_OSMemory);
    __TIB_org_apache_harmony_luni_platform_OSMemory.clazz = __CLASS_org_apache_harmony_luni_platform_OSMemory;
    __TIB_org_apache_harmony_luni_platform_OSMemory.baseType = JAVA_NULL;
    __CLASS_org_apache_harmony_luni_platform_OSMemory_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_OSMemory);
    __CLASS_org_apache_harmony_luni_platform_OSMemory_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_OSMemory_1ARRAY);
    __CLASS_org_apache_harmony_luni_platform_OSMemory_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_org_apache_harmony_luni_platform_OSMemory_2ARRAY);
    org_apache_harmony_luni_platform_OSMemory___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_org_apache_harmony_luni_platform_OSMemory]
    //XMLVM_END_WRAPPER

    __TIB_org_apache_harmony_luni_platform_OSMemory.classInitialized = 1;
}

void __DELETE_org_apache_harmony_luni_platform_OSMemory(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_org_apache_harmony_luni_platform_OSMemory]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_platform_OSMemory(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_platform_OSMemory]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_org_apache_harmony_luni_platform_OSMemory()
{    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_OSMemory)
org_apache_harmony_luni_platform_OSMemory* me = (org_apache_harmony_luni_platform_OSMemory*) XMLVM_MALLOC(sizeof(org_apache_harmony_luni_platform_OSMemory));
    me->tib = &__TIB_org_apache_harmony_luni_platform_OSMemory;
    __INIT_INSTANCE_MEMBERS_org_apache_harmony_luni_platform_OSMemory(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_org_apache_harmony_luni_platform_OSMemory]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_org_apache_harmony_luni_platform_OSMemory()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_INT org_apache_harmony_luni_platform_OSMemory_GET_POINTER_SIZE()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_OSMemory)
    return _STATIC_org_apache_harmony_luni_platform_OSMemory_POINTER_SIZE;
}

void org_apache_harmony_luni_platform_OSMemory_PUT_POINTER_SIZE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_OSMemory)
_STATIC_org_apache_harmony_luni_platform_OSMemory_POINTER_SIZE = v;
}

JAVA_OBJECT org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_OSMemory)
    return _STATIC_org_apache_harmony_luni_platform_OSMemory_NATIVE_ORDER;
}

void org_apache_harmony_luni_platform_OSMemory_PUT_NATIVE_ORDER(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_OSMemory)
_STATIC_org_apache_harmony_luni_platform_OSMemory_NATIVE_ORDER = v;
}

JAVA_OBJECT org_apache_harmony_luni_platform_OSMemory_GET_singleton()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_OSMemory)
    return _STATIC_org_apache_harmony_luni_platform_OSMemory_singleton;
}

void org_apache_harmony_luni_platform_OSMemory_PUT_singleton(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_OSMemory)
_STATIC_org_apache_harmony_luni_platform_OSMemory_singleton = v;
}

void org_apache_harmony_luni_platform_OSMemory___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory___CLINIT___]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("OSMemory.java", 56)
    _r0.o = __NEW_org_apache_harmony_luni_platform_OSMemory();
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_platform_OSMemory___INIT___(_r0.o);
    org_apache_harmony_luni_platform_OSMemory_PUT_singleton( _r0.o);
    XMLVM_SOURCE_POSITION("OSMemory.java", 59)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_getPointerSizeImpl__();
    org_apache_harmony_luni_platform_OSMemory_PUT_POINTER_SIZE( _r0.i);
    XMLVM_SOURCE_POSITION("OSMemory.java", 61)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_isLittleEndianImpl__();
    if (_r0.i == 0) goto label24;
    XMLVM_SOURCE_POSITION("OSMemory.java", 62)
    _r0.o = org_apache_harmony_luni_platform_Endianness_GET_LITTLE_ENDIAN();
    org_apache_harmony_luni_platform_OSMemory_PUT_NATIVE_ORDER( _r0.o);
    label23:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 42)
    XMLVM_EXIT_METHOD()
    return;
    label24:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 64)
    _r0.o = org_apache_harmony_luni_platform_Endianness_GET_BIG_ENDIAN();
    org_apache_harmony_luni_platform_OSMemory_PUT_NATIVE_ORDER( _r0.o);
    goto label23;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_luni_platform_OSMemory_getOSMemory__()
{
    XMLVM_CLASS_INIT(org_apache_harmony_luni_platform_OSMemory)
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_getOSMemory__]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "getOSMemory", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("OSMemory.java", 69)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_singleton();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_BOOLEAN org_apache_harmony_luni_platform_OSMemory_isLittleEndianImpl__()]

void org_apache_harmony_luni_platform_OSMemory___INIT___(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory___INIT___]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "<init>", "?")
    XMLVMElem _r0;
    _r0.o = me;
    XMLVM_SOURCE_POSITION("OSMemory.java", 87)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("OSMemory.java", 88)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN org_apache_harmony_luni_platform_OSMemory_isLittleEndian__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_isLittleEndian__]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "isLittleEndian", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("OSMemory.java", 97)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    _r1.o = org_apache_harmony_luni_platform_Endianness_GET_LITTLE_ENDIAN();
    if (_r0.o != _r1.o) goto label8;
    _r0.i = 1;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    _r0.i = 0;
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT org_apache_harmony_luni_platform_OSMemory_getNativeOrder__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_getNativeOrder__]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "getNativeOrder", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("OSMemory.java", 106)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_INT org_apache_harmony_luni_platform_OSMemory_getPointerSizeImpl__()]

JAVA_INT org_apache_harmony_luni_platform_OSMemory_getPointerSize__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_getPointerSize__]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "getPointerSize", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("OSMemory.java", 117)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_GET_POINTER_SIZE();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_LONG org_apache_harmony_luni_platform_OSMemory_malloc___long(JAVA_OBJECT me, JAVA_LONG n1)]

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_free___long(JAVA_OBJECT me, JAVA_LONG n1)]

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_memset___long_byte_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_BYTE n2, JAVA_LONG n3)]

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_memmove___long_long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2, JAVA_LONG n3)]

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_getByteArray___long_byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)]

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_setByteArray___long_byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)]

//XMLVM_NATIVE[JAVA_BYTE org_apache_harmony_luni_platform_OSMemory_getByte___long(JAVA_OBJECT me, JAVA_LONG n1)]

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_setByte___long_byte(JAVA_OBJECT me, JAVA_LONG n1, JAVA_BYTE n2)]

//XMLVM_NATIVE[JAVA_SHORT org_apache_harmony_luni_platform_OSMemory_getShort___long(JAVA_OBJECT me, JAVA_LONG n1)]

JAVA_SHORT org_apache_harmony_luni_platform_OSMemory_getShort___long_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_getShort___long_org_apache_harmony_luni_platform_Endianness]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "getShort", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.l = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("OSMemory.java", 291)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    if (_r4.o != _r0.o) goto label9;
    //org_apache_harmony_luni_platform_OSMemory_getShort___long[20]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_SHORT (*)(JAVA_OBJECT, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[20])(_r1.o, _r2.l);
    label8:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label9:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 292)
    //org_apache_harmony_luni_platform_OSMemory_getShort___long[20]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_SHORT (*)(JAVA_OBJECT, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[20])(_r1.o, _r2.l);
    XMLVM_CHECK_NPE(1)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_swap___short(_r1.o, _r0.i);
    goto label8;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_setShort___long_short(JAVA_OBJECT me, JAVA_LONG n1, JAVA_SHORT n2)]

void org_apache_harmony_luni_platform_OSMemory_setShort___long_short_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, JAVA_LONG n1, JAVA_SHORT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_setShort___long_short_org_apache_harmony_luni_platform_Endianness]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "setShort", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r1.o = me;
    _r2.l = n1;
    _r4.i = n2;
    _r5.o = n3;
    XMLVM_SOURCE_POSITION("OSMemory.java", 312)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    if (_r5.o != _r0.o) goto label8;
    XMLVM_SOURCE_POSITION("OSMemory.java", 313)
    //org_apache_harmony_luni_platform_OSMemory_setShort___long_short[40]
    XMLVM_CHECK_NPE(1)
    (*(void (*)(JAVA_OBJECT, JAVA_LONG, JAVA_SHORT)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[40])(_r1.o, _r2.l, _r4.i);
    label7:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 317)
    XMLVM_EXIT_METHOD()
    return;
    label8:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 315)
    XMLVM_CHECK_NPE(1)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_swap___short(_r1.o, _r4.i);
    //org_apache_harmony_luni_platform_OSMemory_setShort___long_short[40]
    XMLVM_CHECK_NPE(1)
    (*(void (*)(JAVA_OBJECT, JAVA_LONG, JAVA_SHORT)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[40])(_r1.o, _r2.l, _r0.i);
    goto label7;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_INT org_apache_harmony_luni_platform_OSMemory_getInt___long(JAVA_OBJECT me, JAVA_LONG n1)]

JAVA_INT org_apache_harmony_luni_platform_OSMemory_getInt___long_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_getInt___long_org_apache_harmony_luni_platform_Endianness]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "getInt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.l = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("OSMemory.java", 335)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    if (_r4.o != _r0.o) goto label9;
    //org_apache_harmony_luni_platform_OSMemory_getInt___long[15]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[15])(_r1.o, _r2.l);
    label8:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label9:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 336)
    //org_apache_harmony_luni_platform_OSMemory_getInt___long[15]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[15])(_r1.o, _r2.l);
    XMLVM_CHECK_NPE(1)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_swap___int(_r1.o, _r0.i);
    goto label8;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_setInt___long_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2)]

void org_apache_harmony_luni_platform_OSMemory_setInt___long_int_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_setInt___long_int_org_apache_harmony_luni_platform_Endianness]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "setInt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r1.o = me;
    _r2.l = n1;
    _r4.i = n2;
    _r5.o = n3;
    XMLVM_SOURCE_POSITION("OSMemory.java", 356)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    if (_r5.o != _r0.o) goto label8;
    XMLVM_SOURCE_POSITION("OSMemory.java", 357)
    //org_apache_harmony_luni_platform_OSMemory_setInt___long_int[36]
    XMLVM_CHECK_NPE(1)
    (*(void (*)(JAVA_OBJECT, JAVA_LONG, JAVA_INT)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[36])(_r1.o, _r2.l, _r4.i);
    label7:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 361)
    XMLVM_EXIT_METHOD()
    return;
    label8:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 359)
    XMLVM_CHECK_NPE(1)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_swap___int(_r1.o, _r4.i);
    //org_apache_harmony_luni_platform_OSMemory_setInt___long_int[36]
    XMLVM_CHECK_NPE(1)
    (*(void (*)(JAVA_OBJECT, JAVA_LONG, JAVA_INT)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[36])(_r1.o, _r2.l, _r0.i);
    goto label7;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_LONG org_apache_harmony_luni_platform_OSMemory_getLong___long(JAVA_OBJECT me, JAVA_LONG n1)]

JAVA_LONG org_apache_harmony_luni_platform_OSMemory_getLong___long_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_getLong___long_org_apache_harmony_luni_platform_Endianness]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "getLong", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.o = me;
    _r3.l = n1;
    _r5.o = n2;
    XMLVM_SOURCE_POSITION("OSMemory.java", 379)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    if (_r5.o != _r0.o) goto label9;
    //org_apache_harmony_luni_platform_OSMemory_getLong___long[17]
    XMLVM_CHECK_NPE(2)
    _r0.l = (*(JAVA_LONG (*)(JAVA_OBJECT, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r2.o)->tib->vtable[17])(_r2.o, _r3.l);
    label8:;
    XMLVM_EXIT_METHOD()
    return _r0.l;
    label9:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 380)
    //org_apache_harmony_luni_platform_OSMemory_getLong___long[17]
    XMLVM_CHECK_NPE(2)
    _r0.l = (*(JAVA_LONG (*)(JAVA_OBJECT, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r2.o)->tib->vtable[17])(_r2.o, _r3.l);
    XMLVM_CHECK_NPE(2)
    _r0.l = org_apache_harmony_luni_platform_OSMemory_swap___long(_r2.o, _r0.l);
    goto label8;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_setLong___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)]

void org_apache_harmony_luni_platform_OSMemory_setLong___long_long_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_setLong___long_long_org_apache_harmony_luni_platform_Endianness]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "setLong", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r2.o = me;
    _r3.l = n1;
    _r5.l = n2;
    _r7.o = n3;
    XMLVM_SOURCE_POSITION("OSMemory.java", 401)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    if (_r7.o != _r0.o) goto label8;
    XMLVM_SOURCE_POSITION("OSMemory.java", 402)
    //org_apache_harmony_luni_platform_OSMemory_setLong___long_long[38]
    XMLVM_CHECK_NPE(2)
    (*(void (*)(JAVA_OBJECT, JAVA_LONG, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r2.o)->tib->vtable[38])(_r2.o, _r3.l, _r5.l);
    label7:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 406)
    XMLVM_EXIT_METHOD()
    return;
    label8:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 404)
    XMLVM_CHECK_NPE(2)
    _r0.l = org_apache_harmony_luni_platform_OSMemory_swap___long(_r2.o, _r5.l);
    //org_apache_harmony_luni_platform_OSMemory_setLong___long_long[38]
    XMLVM_CHECK_NPE(2)
    (*(void (*)(JAVA_OBJECT, JAVA_LONG, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r2.o)->tib->vtable[38])(_r2.o, _r3.l, _r0.l);
    goto label7;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_FLOAT org_apache_harmony_luni_platform_OSMemory_getFloat___long(JAVA_OBJECT me, JAVA_LONG n1)]

JAVA_FLOAT org_apache_harmony_luni_platform_OSMemory_getFloat___long_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_getFloat___long_org_apache_harmony_luni_platform_Endianness]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "getFloat", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r1.o = me;
    _r2.l = n1;
    _r4.o = n2;
    XMLVM_SOURCE_POSITION("OSMemory.java", 424)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    if (_r4.o != _r0.o) goto label9;
    XMLVM_SOURCE_POSITION("OSMemory.java", 425)
    //org_apache_harmony_luni_platform_OSMemory_getFloat___long[13]
    XMLVM_CHECK_NPE(1)
    _r0.f = (*(JAVA_FLOAT (*)(JAVA_OBJECT, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[13])(_r1.o, _r2.l);
    label8:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 428)
    XMLVM_EXIT_METHOD()
    return _r0.f;
    label9:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 427)
    //org_apache_harmony_luni_platform_OSMemory_getInt___long[15]
    XMLVM_CHECK_NPE(1)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[15])(_r1.o, _r2.l);
    XMLVM_CHECK_NPE(1)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_swap___int(_r1.o, _r0.i);
    _r0.f = java_lang_Float_intBitsToFloat___int(_r0.i);
    goto label8;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_setFloat___long_float(JAVA_OBJECT me, JAVA_LONG n1, JAVA_FLOAT n2)]

void org_apache_harmony_luni_platform_OSMemory_setFloat___long_float_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, JAVA_LONG n1, JAVA_FLOAT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_setFloat___long_float_org_apache_harmony_luni_platform_Endianness]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "setFloat", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r1.o = me;
    _r2.l = n1;
    _r4.f = n2;
    _r5.o = n3;
    XMLVM_SOURCE_POSITION("OSMemory.java", 448)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    if (_r5.o != _r0.o) goto label8;
    XMLVM_SOURCE_POSITION("OSMemory.java", 449)
    //org_apache_harmony_luni_platform_OSMemory_setFloat___long_float[34]
    XMLVM_CHECK_NPE(1)
    (*(void (*)(JAVA_OBJECT, JAVA_LONG, JAVA_FLOAT)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[34])(_r1.o, _r2.l, _r4.f);
    label7:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 454)
    XMLVM_EXIT_METHOD()
    return;
    label8:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 451)
    _r0.i = java_lang_Float_floatToIntBits___float(_r4.f);
    XMLVM_SOURCE_POSITION("OSMemory.java", 452)
    XMLVM_CHECK_NPE(1)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_swap___int(_r1.o, _r0.i);
    //org_apache_harmony_luni_platform_OSMemory_setInt___long_int[36]
    XMLVM_CHECK_NPE(1)
    (*(void (*)(JAVA_OBJECT, JAVA_LONG, JAVA_INT)) ((org_apache_harmony_luni_platform_OSMemory*) _r1.o)->tib->vtable[36])(_r1.o, _r2.l, _r0.i);
    goto label7;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_DOUBLE org_apache_harmony_luni_platform_OSMemory_getDouble___long(JAVA_OBJECT me, JAVA_LONG n1)]

JAVA_DOUBLE org_apache_harmony_luni_platform_OSMemory_getDouble___long_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_getDouble___long_org_apache_harmony_luni_platform_Endianness]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "getDouble", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r2.o = me;
    _r3.l = n1;
    _r5.o = n2;
    XMLVM_SOURCE_POSITION("OSMemory.java", 472)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    if (_r5.o != _r0.o) goto label9;
    XMLVM_SOURCE_POSITION("OSMemory.java", 473)
    //org_apache_harmony_luni_platform_OSMemory_getDouble___long[11]
    XMLVM_CHECK_NPE(2)
    _r0.d = (*(JAVA_DOUBLE (*)(JAVA_OBJECT, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r2.o)->tib->vtable[11])(_r2.o, _r3.l);
    label8:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 476)
    XMLVM_EXIT_METHOD()
    return _r0.d;
    label9:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 475)
    //org_apache_harmony_luni_platform_OSMemory_getLong___long[17]
    XMLVM_CHECK_NPE(2)
    _r0.l = (*(JAVA_LONG (*)(JAVA_OBJECT, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r2.o)->tib->vtable[17])(_r2.o, _r3.l);
    XMLVM_CHECK_NPE(2)
    _r0.l = org_apache_harmony_luni_platform_OSMemory_swap___long(_r2.o, _r0.l);
    _r0.d = java_lang_Double_longBitsToDouble___long(_r0.l);
    goto label8;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_setDouble___long_double(JAVA_OBJECT me, JAVA_LONG n1, JAVA_DOUBLE n2)]

void org_apache_harmony_luni_platform_OSMemory_setDouble___long_double_org_apache_harmony_luni_platform_Endianness(JAVA_OBJECT me, JAVA_LONG n1, JAVA_DOUBLE n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_setDouble___long_double_org_apache_harmony_luni_platform_Endianness]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "setDouble", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r2.o = me;
    _r3.l = n1;
    _r5.d = n2;
    _r7.o = n3;
    XMLVM_SOURCE_POSITION("OSMemory.java", 497)
    _r0.o = org_apache_harmony_luni_platform_OSMemory_GET_NATIVE_ORDER();
    if (_r7.o != _r0.o) goto label8;
    XMLVM_SOURCE_POSITION("OSMemory.java", 498)
    //org_apache_harmony_luni_platform_OSMemory_setDouble___long_double[32]
    XMLVM_CHECK_NPE(2)
    (*(void (*)(JAVA_OBJECT, JAVA_LONG, JAVA_DOUBLE)) ((org_apache_harmony_luni_platform_OSMemory*) _r2.o)->tib->vtable[32])(_r2.o, _r3.l, _r5.d);
    label7:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 503)
    XMLVM_EXIT_METHOD()
    return;
    label8:;
    XMLVM_SOURCE_POSITION("OSMemory.java", 500)
    _r0.l = java_lang_Double_doubleToLongBits___double(_r5.d);
    XMLVM_SOURCE_POSITION("OSMemory.java", 501)
    XMLVM_CHECK_NPE(2)
    _r0.l = org_apache_harmony_luni_platform_OSMemory_swap___long(_r2.o, _r0.l);
    //org_apache_harmony_luni_platform_OSMemory_setLong___long_long[38]
    XMLVM_CHECK_NPE(2)
    (*(void (*)(JAVA_OBJECT, JAVA_LONG, JAVA_LONG)) ((org_apache_harmony_luni_platform_OSMemory*) _r2.o)->tib->vtable[38])(_r2.o, _r3.l, _r0.l);
    goto label7;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_LONG org_apache_harmony_luni_platform_OSMemory_getAddress___long(JAVA_OBJECT me, JAVA_LONG n1)]

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_setAddress___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)]

//XMLVM_NATIVE[JAVA_LONG org_apache_harmony_luni_platform_OSMemory_mmapImpl___long_long_long_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2, JAVA_LONG n3, JAVA_INT n4)]

JAVA_LONG org_apache_harmony_luni_platform_OSMemory_mmap___long_long_long_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2, JAVA_LONG n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_mmap___long_long_long_int]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "mmap", "?")
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
    _r2.o = me;
    _r3.l = n1;
    _r5.l = n2;
    _r7.l = n3;
    _r9.i = n4;
    XMLVM_SOURCE_POSITION("OSMemory.java", 550)
    XMLVM_CHECK_NPE(2)
    _r0.l = org_apache_harmony_luni_platform_OSMemory_mmapImpl___long_long_long_int(_r2.o, _r3.l, _r5.l, _r7.l, _r9.i);
    XMLVM_SOURCE_POSITION("OSMemory.java", 551)
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[void org_apache_harmony_luni_platform_OSMemory_unmapImpl___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)]

void org_apache_harmony_luni_platform_OSMemory_unmap___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_unmap___long_long]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "unmap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r0.o = me;
    _r1.l = n1;
    _r3.l = n2;
    XMLVM_SOURCE_POSITION("OSMemory.java", 557)
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_platform_OSMemory_unmapImpl___long_long(_r0.o, _r1.l, _r3.l);
    XMLVM_SOURCE_POSITION("OSMemory.java", 558)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void org_apache_harmony_luni_platform_OSMemory_load___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_load___long_long]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "load", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r0.o = me;
    _r1.l = n1;
    _r3.l = n2;
    XMLVM_SOURCE_POSITION("OSMemory.java", 561)
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_platform_OSMemory_loadImpl___long_long(_r0.o, _r1.l, _r3.l);
    XMLVM_SOURCE_POSITION("OSMemory.java", 562)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_INT org_apache_harmony_luni_platform_OSMemory_loadImpl___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)]

JAVA_BOOLEAN org_apache_harmony_luni_platform_OSMemory_isLoaded___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_isLoaded___long_long]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "isLoaded", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r2.o = me;
    _r3.l = n1;
    _r5.l = n2;
    XMLVM_SOURCE_POSITION("OSMemory.java", 567)
    _r0.l = 0;
    _r0.i = _r5.l > _r0.l ? 1 : (_r5.l == _r0.l ? 0 : -1);
    if (_r0.i != 0) goto label8;
    _r0.i = 1;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    XMLVM_CHECK_NPE(2)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_isLoadedImpl___long_long(_r2.o, _r3.l, _r5.l);
    goto label7;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_BOOLEAN org_apache_harmony_luni_platform_OSMemory_isLoadedImpl___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)]

void org_apache_harmony_luni_platform_OSMemory_flush___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_flush___long_long]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "flush", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r0.o = me;
    _r1.l = n1;
    _r3.l = n2;
    XMLVM_SOURCE_POSITION("OSMemory.java", 573)
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_luni_platform_OSMemory_flushImpl___long_long(_r0.o, _r1.l, _r3.l);
    XMLVM_SOURCE_POSITION("OSMemory.java", 574)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

//XMLVM_NATIVE[JAVA_INT org_apache_harmony_luni_platform_OSMemory_flushImpl___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)]

JAVA_SHORT org_apache_harmony_luni_platform_OSMemory_swap___short(JAVA_OBJECT me, JAVA_SHORT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_swap___short]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "swap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("OSMemory.java", 582)
    _r0.i = _r3.i << 8;
    _r1.i = _r3.i >> 8;
    _r1.i = _r1.i & 255;
    _r0.i = _r0.i | _r1.i;
    _r0.i = (_r0.i << 16) >> 16;
    XMLVM_SOURCE_POSITION("OSMemory.java", 584)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT org_apache_harmony_luni_platform_OSMemory_swap___int(JAVA_OBJECT me, JAVA_INT n1)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_swap___int]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "swap", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = me;
    _r4.i = n1;
    XMLVM_SOURCE_POSITION("OSMemory.java", 588)
    _r0.i = _r4.i >> 16;
    _r0.i = (_r0.i << 16) >> 16;
    _r1.i = (_r4.i << 16) >> 16;
    XMLVM_SOURCE_POSITION("OSMemory.java", 590)
    XMLVM_CHECK_NPE(3)
    _r1.i = org_apache_harmony_luni_platform_OSMemory_swap___short(_r3.o, _r1.i);
    _r1.i = _r1.i << 16;
    XMLVM_SOURCE_POSITION("OSMemory.java", 591)
    XMLVM_CHECK_NPE(3)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_swap___short(_r3.o, _r0.i);
    _r2.i = 65535;
    _r0.i = _r0.i & _r2.i;
    _r0.i = _r0.i | _r1.i;
    XMLVM_SOURCE_POSITION("OSMemory.java", 592)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_LONG org_apache_harmony_luni_platform_OSMemory_swap___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_WRAPPER[org_apache_harmony_luni_platform_OSMemory_swap___long]
    XMLVM_ENTER_METHOD("org.apache.harmony.luni.platform.OSMemory", "swap", "?")
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
    _r7.o = me;
    _r8.l = n1;
    _r3.i = 32;
    XMLVM_SOURCE_POSITION("OSMemory.java", 596)
    _r0.l = _r8.l >> (0x3f & _r3.l);
    _r0.i = (JAVA_INT) _r0.l;
    _r1.i = (JAVA_INT) _r8.l;
    XMLVM_SOURCE_POSITION("OSMemory.java", 598)
    XMLVM_CHECK_NPE(7)
    _r1.i = org_apache_harmony_luni_platform_OSMemory_swap___int(_r7.o, _r1.i);
    _r1.l = (JAVA_LONG) _r1.i;
    _r1.l = _r1.l << (0x3f & _r3.l);
    XMLVM_SOURCE_POSITION("OSMemory.java", 599)
    XMLVM_CHECK_NPE(7)
    _r0.i = org_apache_harmony_luni_platform_OSMemory_swap___int(_r7.o, _r0.i);
    _r3.l = (JAVA_LONG) _r0.i;
    _r5.l = 4294967295;
    _r3.l = _r3.l & _r5.l;
    _r0.l = _r1.l | _r3.l;
    XMLVM_SOURCE_POSITION("OSMemory.java", 600)
    XMLVM_EXIT_METHOD()
    return _r0.l;
    //XMLVM_END_WRAPPER
}

