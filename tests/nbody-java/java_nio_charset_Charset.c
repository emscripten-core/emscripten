#include "xmlvm.h"
#include "java_io_IOException.h"
#include "java_lang_Class.h"
#include "java_lang_ClassLoader.h"
#include "java_lang_Exception.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_String.h"
#include "java_lang_StringBuilder.h"
#include "java_lang_Thread.h"
#include "java_lang_Throwable.h"
#include "java_nio_ByteBuffer.h"
#include "java_nio_CharBuffer.h"
#include "java_nio_charset_Charset_1.h"
#include "java_nio_charset_Charset_2.h"
#include "java_nio_charset_CharsetDecoder.h"
#include "java_nio_charset_CharsetEncoder.h"
#include "java_nio_charset_CodingErrorAction.h"
#include "java_nio_charset_spi_CharsetProvider.h"
#include "java_security_AccessController.h"
#include "java_util_Collections.h"
#include "java_util_Comparator.h"
#include "java_util_Enumeration.h"
#include "java_util_HashMap.h"
#include "java_util_HashSet.h"
#include "java_util_Iterator.h"
#include "java_util_Locale.h"
#include "java_util_Set.h"
#include "org_apache_harmony_niochar_CharsetProviderImpl.h"

#include "java_nio_charset_Charset.h"

#define XMLVM_CURRENT_CLASS_NAME Charset
#define XMLVM_CURRENT_PKG_CLASS_NAME java_nio_charset_Charset

__TIB_DEFINITION_java_nio_charset_Charset __TIB_java_nio_charset_Charset = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_nio_charset_Charset, // classInitializer
    "java.nio.charset.Charset", // className
    "java.nio.charset", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Object;Ljava/lang/Comparable<Ljava/nio/charset/Charset;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_nio_charset_Charset), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_nio_charset_Charset;
JAVA_OBJECT __CLASS_java_nio_charset_Charset_1ARRAY;
JAVA_OBJECT __CLASS_java_nio_charset_Charset_2ARRAY;
JAVA_OBJECT __CLASS_java_nio_charset_Charset_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_OBJECT _STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_NAME;
static JAVA_OBJECT _STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_ENCODING;
static JAVA_OBJECT _STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_COMMENT;
static JAVA_OBJECT _STATIC_java_nio_charset_Charset_systemClassLoader;
static JAVA_OBJECT _STATIC_java_nio_charset_Charset__builtInProvider;
static JAVA_OBJECT _STATIC_java_nio_charset_Charset__builtInCharsets;
static JAVA_OBJECT _STATIC_java_nio_charset_Charset_cachedCharsetTable;
static JAVA_BOOLEAN _STATIC_java_nio_charset_Charset_inForNameInternal;

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

void __INIT_java_nio_charset_Charset()
{
    staticInitializerLock(&__TIB_java_nio_charset_Charset);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_nio_charset_Charset.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_nio_charset_Charset.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_nio_charset_Charset);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_nio_charset_Charset.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_nio_charset_Charset.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_nio_charset_Charset.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.nio.charset.Charset")
        __INIT_IMPL_java_nio_charset_Charset();
    }
}

void __INIT_IMPL_java_nio_charset_Charset()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_nio_charset_Charset.newInstanceFunc = __NEW_INSTANCE_java_nio_charset_Charset;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_nio_charset_Charset.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_nio_charset_Charset.vtable[1] = (VTABLE_PTR) &java_nio_charset_Charset_equals___java_lang_Object;
    __TIB_java_nio_charset_Charset.vtable[4] = (VTABLE_PTR) &java_nio_charset_Charset_hashCode__;
    __TIB_java_nio_charset_Charset.vtable[5] = (VTABLE_PTR) &java_nio_charset_Charset_toString__;
    __TIB_java_nio_charset_Charset.vtable[6] = (VTABLE_PTR) &java_nio_charset_Charset_compareTo___java_lang_Object;
    // Initialize interface information
    __TIB_java_nio_charset_Charset.numImplementedInterfaces = 1;
    __TIB_java_nio_charset_Charset.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 1);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_nio_charset_Charset.implementedInterfaces[0][0] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_nio_charset_Charset.itableBegin = &__TIB_java_nio_charset_Charset.itable[0];
    __TIB_java_nio_charset_Charset.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_nio_charset_Charset.vtable[6];

    _STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_NAME = (java_lang_String*) xmlvm_create_java_string_from_pool(709);
    _STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_ENCODING = (java_lang_String*) xmlvm_create_java_string_from_pool(100);
    _STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_COMMENT = (java_lang_String*) xmlvm_create_java_string_from_pool(710);
    _STATIC_java_nio_charset_Charset_systemClassLoader = (java_lang_ClassLoader*) JAVA_NULL;
    _STATIC_java_nio_charset_Charset__builtInProvider = (org_apache_harmony_niochar_CharsetProviderImpl*) JAVA_NULL;
    _STATIC_java_nio_charset_Charset__builtInCharsets = (java_lang_Object*) JAVA_NULL;
    _STATIC_java_nio_charset_Charset_cachedCharsetTable = (java_util_HashMap*) JAVA_NULL;
    _STATIC_java_nio_charset_Charset_inForNameInternal = 0;

    __TIB_java_nio_charset_Charset.declaredFields = &__field_reflection_data[0];
    __TIB_java_nio_charset_Charset.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_nio_charset_Charset.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_nio_charset_Charset.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_nio_charset_Charset.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_nio_charset_Charset.methodDispatcherFunc = method_dispatcher;
    __TIB_java_nio_charset_Charset.declaredMethods = &__method_reflection_data[0];
    __TIB_java_nio_charset_Charset.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_nio_charset_Charset = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_nio_charset_Charset);
    __TIB_java_nio_charset_Charset.clazz = __CLASS_java_nio_charset_Charset;
    __TIB_java_nio_charset_Charset.baseType = JAVA_NULL;
    __CLASS_java_nio_charset_Charset_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_Charset);
    __CLASS_java_nio_charset_Charset_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_Charset_1ARRAY);
    __CLASS_java_nio_charset_Charset_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_nio_charset_Charset_2ARRAY);
    java_nio_charset_Charset___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_nio_charset_Charset]
    //XMLVM_END_WRAPPER

    __TIB_java_nio_charset_Charset.classInitialized = 1;
}

void __DELETE_java_nio_charset_Charset(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_nio_charset_Charset]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_nio_charset_Charset(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_nio_charset_Charset*) me)->fields.java_nio_charset_Charset.canonicalName_ = (java_lang_String*) JAVA_NULL;
    ((java_nio_charset_Charset*) me)->fields.java_nio_charset_Charset.aliasesSet_ = (java_util_HashSet*) JAVA_NULL;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_nio_charset_Charset]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_nio_charset_Charset()
{    XMLVM_CLASS_INIT(java_nio_charset_Charset)
java_nio_charset_Charset* me = (java_nio_charset_Charset*) XMLVM_MALLOC(sizeof(java_nio_charset_Charset));
    me->tib = &__TIB_java_nio_charset_Charset;
    __INIT_INSTANCE_MEMBERS_java_nio_charset_Charset(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_nio_charset_Charset]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_nio_charset_Charset()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_OBJECT java_nio_charset_Charset_GET_PROVIDER_CONFIGURATION_FILE_NAME()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    return _STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_NAME;
}

void java_nio_charset_Charset_PUT_PROVIDER_CONFIGURATION_FILE_NAME(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
_STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_NAME = v;
}

JAVA_OBJECT java_nio_charset_Charset_GET_PROVIDER_CONFIGURATION_FILE_ENCODING()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    return _STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_ENCODING;
}

void java_nio_charset_Charset_PUT_PROVIDER_CONFIGURATION_FILE_ENCODING(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
_STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_ENCODING = v;
}

JAVA_OBJECT java_nio_charset_Charset_GET_PROVIDER_CONFIGURATION_FILE_COMMENT()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    return _STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_COMMENT;
}

void java_nio_charset_Charset_PUT_PROVIDER_CONFIGURATION_FILE_COMMENT(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
_STATIC_java_nio_charset_Charset_PROVIDER_CONFIGURATION_FILE_COMMENT = v;
}

JAVA_OBJECT java_nio_charset_Charset_GET_systemClassLoader()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    return _STATIC_java_nio_charset_Charset_systemClassLoader;
}

void java_nio_charset_Charset_PUT_systemClassLoader(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
_STATIC_java_nio_charset_Charset_systemClassLoader = v;
}

JAVA_OBJECT java_nio_charset_Charset_GET__builtInProvider()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    return _STATIC_java_nio_charset_Charset__builtInProvider;
}

void java_nio_charset_Charset_PUT__builtInProvider(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
_STATIC_java_nio_charset_Charset__builtInProvider = v;
}

JAVA_OBJECT java_nio_charset_Charset_GET__builtInCharsets()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    return _STATIC_java_nio_charset_Charset__builtInCharsets;
}

void java_nio_charset_Charset_PUT__builtInCharsets(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
_STATIC_java_nio_charset_Charset__builtInCharsets = v;
}

JAVA_OBJECT java_nio_charset_Charset_GET_cachedCharsetTable()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    return _STATIC_java_nio_charset_Charset_cachedCharsetTable;
}

void java_nio_charset_Charset_PUT_cachedCharsetTable(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
_STATIC_java_nio_charset_Charset_cachedCharsetTable = v;
}

JAVA_BOOLEAN java_nio_charset_Charset_GET_inForNameInternal()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    return _STATIC_java_nio_charset_Charset_inForNameInternal;
}

void java_nio_charset_Charset_PUT_inForNameInternal(JAVA_BOOLEAN v)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
_STATIC_java_nio_charset_Charset_inForNameInternal = v;
}

void java_nio_charset_Charset___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset___CLINIT___]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "<clinit>", "?")
    XMLVMElem _r0;
    _r0.o = JAVA_NULL;
    XMLVM_SOURCE_POSITION("Charset.java", 96)
    java_nio_charset_Charset_PUT__builtInProvider( _r0.o);
    XMLVM_SOURCE_POSITION("Charset.java", 99)
    java_nio_charset_Charset_PUT__builtInCharsets( _r0.o);
    XMLVM_SOURCE_POSITION("Charset.java", 107)
    _r0.o = __NEW_java_util_HashMap();
    XMLVM_CHECK_NPE(0)
    java_util_HashMap___INIT___(_r0.o);
    java_nio_charset_Charset_PUT_cachedCharsetTable( _r0.o);
    XMLVM_SOURCE_POSITION("Charset.java", 109)
    _r0.i = 0;
    java_nio_charset_Charset_PUT_inForNameInternal( _r0.i);
    XMLVM_SOURCE_POSITION("Charset.java", 117)
    _r0.o = __NEW_java_nio_charset_Charset_1();
    XMLVM_CHECK_NPE(0)
    java_nio_charset_Charset_1___INIT___(_r0.o);
    _r0.o = java_security_AccessController_doPrivileged___java_security_PrivilegedAction(_r0.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Charset.java", 116)
    java_nio_charset_Charset_PUT__builtInProvider( _r0.o);
    XMLVM_SOURCE_POSITION("Charset.java", 75)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_Charset___INIT____java_lang_String_java_lang_String_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset___INIT____java_lang_String_java_lang_String_1ARRAY]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = me;
    _r4.o = n1;
    _r5.o = n2;
    XMLVM_SOURCE_POSITION("Charset.java", 137)
    XMLVM_CHECK_NPE(3)
    java_lang_Object___INIT___(_r3.o);
    XMLVM_SOURCE_POSITION("Charset.java", 138)
    if (_r4.o != JAVA_NULL) goto label11;
    XMLVM_SOURCE_POSITION("Charset.java", 139)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label11:;
    XMLVM_SOURCE_POSITION("Charset.java", 142)
    java_nio_charset_Charset_checkCharsetName___java_lang_String(_r4.o);
    XMLVM_SOURCE_POSITION("Charset.java", 143)
    XMLVM_CHECK_NPE(3)
    ((java_nio_charset_Charset*) _r3.o)->fields.java_nio_charset_Charset.canonicalName_ = _r4.o;
    XMLVM_SOURCE_POSITION("Charset.java", 145)
    _r0.o = __NEW_java_util_HashSet();
    XMLVM_CHECK_NPE(0)
    java_util_HashSet___INIT___(_r0.o);
    XMLVM_CHECK_NPE(3)
    ((java_nio_charset_Charset*) _r3.o)->fields.java_nio_charset_Charset.aliasesSet_ = _r0.o;
    XMLVM_SOURCE_POSITION("Charset.java", 146)
    if (_r5.o == JAVA_NULL) goto label29;
    XMLVM_SOURCE_POSITION("Charset.java", 147)
    _r0.i = 0;
    label26:;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r5.o));
    if (_r0.i < _r1.i) goto label30;
    label29:;
    XMLVM_SOURCE_POSITION("Charset.java", 152)
    XMLVM_EXIT_METHOD()
    return;
    label30:;
    XMLVM_SOURCE_POSITION("Charset.java", 148)
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r0.i);
    _r1.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    java_nio_charset_Charset_checkCharsetName___java_lang_String(_r1.o);
    XMLVM_SOURCE_POSITION("Charset.java", 149)
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_nio_charset_Charset*) _r3.o)->fields.java_nio_charset_Charset.aliasesSet_;
    XMLVM_CHECK_NPE(5)
    XMLVM_CHECK_ARRAY_BOUNDS(_r5.o, _r0.i);
    _r2.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r5.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    //java_util_HashSet_add___java_lang_Object[7]
    XMLVM_CHECK_NPE(1)
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashSet*) _r1.o)->tib->vtable[7])(_r1.o, _r2.o);
    _r0.i = _r0.i + 1;
    goto label26;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_Charset_isSpecial___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_isSpecial___char]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "isSpecial", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 159)
    _r0.i = 45;
    if (_r0.i == _r1.i) goto label18;
    _r0.i = 46;
    if (_r0.i == _r1.i) goto label18;
    _r0.i = 58;
    if (_r0.i == _r1.i) goto label18;
    _r0.i = 95;
    if (_r0.i == _r1.i) goto label18;
    _r0.i = 0;
    label17:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label18:;
    _r0.i = 1;
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_Charset_isLetter___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_isLetter___char]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "isLetter", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 167)
    _r0.i = 97;
    if (_r0.i > _r1.i) goto label8;
    _r0.i = 122;
    if (_r1.i <= _r0.i) goto label18;
    label8:;
    _r0.i = 65;
    if (_r0.i > _r1.i) goto label16;
    _r0.i = 90;
    if (_r1.i <= _r0.i) goto label18;
    label16:;
    _r0.i = 0;
    label17:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label18:;
    _r0.i = 1;
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_Charset_isDigit___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_isDigit___char]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "isDigit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 175)
    _r0.i = 48;
    if (_r0.i > _r1.i) goto label10;
    _r0.i = 57;
    if (_r1.i > _r0.i) goto label10;
    _r0.i = 1;
    label9:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label10:;
    _r0.i = 0;
    goto label9;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_Charset_checkCharsetName___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_checkCharsetName___java_lang_String]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "checkCharsetName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.o = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 184)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[8])(_r4.o);
    if (_r0.i != 0) goto label12;
    XMLVM_SOURCE_POSITION("Charset.java", 185)

    
    // Red class access removed: java.nio.charset.IllegalCharsetNameException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.charset.IllegalCharsetNameException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("Charset.java", 194)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r4.o)->tib->vtable[8])(_r4.o);
    XMLVM_SOURCE_POSITION("Charset.java", 195)
    _r1.i = 0;
    label17:;
    if (_r1.i < _r0.i) goto label20;
    XMLVM_SOURCE_POSITION("Charset.java", 201)
    XMLVM_EXIT_METHOD()
    return;
    label20:;
    XMLVM_SOURCE_POSITION("Charset.java", 196)
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(4)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r4.o)->tib->vtable[6])(_r4.o, _r1.i);
    XMLVM_SOURCE_POSITION("Charset.java", 197)
    _r3.i = java_nio_charset_Charset_isLetter___char(_r2.i);
    if (_r3.i != 0) goto label48;
    _r3.i = java_nio_charset_Charset_isDigit___char(_r2.i);
    if (_r3.i != 0) goto label48;
    _r2.i = java_nio_charset_Charset_isSpecial___char(_r2.i);
    if (_r2.i != 0) goto label48;
    XMLVM_SOURCE_POSITION("Charset.java", 198)

    
    // Red class access removed: java.nio.charset.IllegalCharsetNameException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.charset.IllegalCharsetNameException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label48:;
    _r1.i = _r1.i + 1;
    goto label17;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_getContextClassLoader__()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_getContextClassLoader__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "getContextClassLoader", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVM_SOURCE_POSITION("Charset.java", 207)
    _r0.o = java_lang_Thread_currentThread__();
    XMLVM_SOURCE_POSITION("Charset.java", 209)
    _r1.o = __NEW_java_nio_charset_Charset_2();
    XMLVM_CHECK_NPE(1)
    java_nio_charset_Charset_2___INIT____java_lang_Thread(_r1.o, _r0.o);
    _r0.o = java_security_AccessController_doPrivileged___java_security_PrivilegedAction(_r1.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Charset.java", 208)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_Charset_getSystemClassLoader__()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_getSystemClassLoader__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "getSystemClassLoader", "?")
    XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Charset.java", 220)
    _r0.o = java_nio_charset_Charset_GET_systemClassLoader();
    if (_r0.o != JAVA_NULL) goto label17;
    XMLVM_SOURCE_POSITION("Charset.java", 222)

    
    // Red class access removed: java.nio.charset.Charset$3::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.charset.Charset$3::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0.o = java_security_AccessController_doPrivileged___java_security_PrivilegedAction(_r0.o);
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Charset.java", 221)
    java_nio_charset_Charset_PUT_systemClassLoader( _r0.o);
    label17:;
    XMLVM_SOURCE_POSITION("Charset.java", 228)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_Charset_addCharsets___java_nio_charset_spi_CharsetProvider_java_util_TreeMap(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_addCharsets___java_nio_charset_spi_CharsetProvider_java_util_TreeMap]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "addCharsets", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("Charset.java", 235)
    //java_nio_charset_spi_CharsetProvider_charsets__[7]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_nio_charset_spi_CharsetProvider*) _r2.o)->tib->vtable[7])(_r2.o);
    label4:;
    XMLVM_SOURCE_POSITION("Charset.java", 236)
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_hasNext__])(_r0.o);
    if (_r1.i != 0) goto label11;
    XMLVM_SOURCE_POSITION("Charset.java", 243)
    XMLVM_EXIT_METHOD()
    return;
    label11:;
    XMLVM_SOURCE_POSITION("Charset.java", 237)
    XMLVM_CHECK_NPE(0)
    _r2.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r0.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Iterator_next__])(_r0.o);
    _r2.o = _r2.o;
    XMLVM_SOURCE_POSITION("Charset.java", 239)
    XMLVM_CHECK_NPE(2)
    _r1.o = java_nio_charset_Charset_name__(_r2.o);

    
    // Red class access removed: java.util.TreeMap::containsKey
    XMLVM_RED_CLASS_DEPENDENCY();
    if (_r1.i != 0) goto label4;
    XMLVM_SOURCE_POSITION("Charset.java", 240)
    XMLVM_CHECK_NPE(2)
    _r1.o = java_nio_charset_Charset_name__(_r2.o);

    
    // Red class access removed: java.util.TreeMap::put
    XMLVM_RED_CLASS_DEPENDENCY();
    goto label4;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_trimClassName___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_trimClassName___java_lang_String]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "trimClassName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 249)
    XMLVM_SOURCE_POSITION("Charset.java", 250)
    // "#"
    _r0.o = xmlvm_create_java_string_from_pool(710);
    XMLVM_CHECK_NPE(2)
    _r0.i = java_lang_String_indexOf___java_lang_String(_r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Charset.java", 252)
    _r1.i = -1;
    if (_r0.i == _r1.i) goto label19;
    XMLVM_SOURCE_POSITION("Charset.java", 253)
    _r1.i = 0;
    XMLVM_CHECK_NPE(2)
    _r0.o = java_lang_String_substring___int_int(_r2.o, _r1.i, _r0.i);
    label14:;
    XMLVM_SOURCE_POSITION("Charset.java", 255)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_trim__(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label19:;
    _r0 = _r2;
    goto label14;
    //XMLVM_END_WRAPPER
}

void java_nio_charset_Charset_loadConfiguredCharsets___java_net_URL_java_lang_ClassLoader_java_util_TreeMap(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_loadConfiguredCharsets___java_net_URL_java_lang_ClassLoader_java_util_TreeMap]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "loadConfiguredCharsets", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    volatile XMLVMElem _r8;
    _r6.o = n1;
    _r7.o = n2;
    _r8.o = n3;
    XMLVM_SOURCE_POSITION("Charset.java", 264)
    _r0.o = JAVA_NULL;
    XMLVM_TRY_BEGIN(w4567aaac20b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 266)

    
    // Red class access removed: java.net.URL::openStream
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Charset.java", 268)

    
    // Red class access removed: java.io.BufferedReader::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.InputStreamReader::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Charset.java", 269)
    // "UTF-8"
    _r4.o = xmlvm_create_java_string_from_pool(100);

    
    // Red class access removed: java.io.InputStreamReader::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.BufferedReader::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac20b1b6)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1b6,java_io_IOException,114)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1b6,java_lang_Object,97)
    XMLVM_CATCH_END(w4567aaac20b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac20b1b6)
    XMLVM_TRY_BEGIN(w4567aaac20b1b7)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 270)

    
    // Red class access removed: java.io.BufferedReader::readLine
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac20b1b7)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1b7,java_io_IOException,87)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1b7,java_lang_Object,111)
    XMLVM_CATCH_END(w4567aaac20b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac20b1b7)
    label21:;
    XMLVM_SOURCE_POSITION("Charset.java", 271)
    if (_r0.o != JAVA_NULL) goto label29;
    XMLVM_SOURCE_POSITION("Charset.java", 303)
    if (_r2.o == JAVA_NULL) goto label28;
    XMLVM_TRY_BEGIN(w4567aaac20b1c13)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 304)

    
    // Red class access removed: java.io.BufferedReader::close
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac20b1c13)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c13,java_io_IOException,109)
    XMLVM_CATCH_END(w4567aaac20b1c13)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac20b1c13)
    label28:;
    XMLVM_SOURCE_POSITION("Charset.java", 310)
    XMLVM_EXIT_METHOD()
    return;
    label29:;
    XMLVM_TRY_BEGIN(w4567aaac20b1c18)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 272)
    _r0.o = java_nio_charset_Charset_trimClassName___java_lang_String(_r0.o);
    XMLVM_SOURCE_POSITION("Charset.java", 274)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(0)
    _r1.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[8])(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac20b1c18)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c18,java_io_IOException,87)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c18,java_lang_Object,111)
    XMLVM_CATCH_END(w4567aaac20b1c18)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac20b1c18)
    if (_r1.i <= 0) goto label54;
    XMLVM_SOURCE_POSITION("Charset.java", 278)
    _r1.i = 1;
    XMLVM_TRY_BEGIN(w4567aaac20b1c22)
    // Begin try
    _r1.o = java_lang_Class_forName___java_lang_String_boolean_java_lang_ClassLoader(_r0.o, _r1.i, _r7.o);
    XMLVM_SOURCE_POSITION("Charset.java", 280)
    //java_lang_Class_newInstance__[12]
    XMLVM_CHECK_NPE(1)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Class*) _r1.o)->tib->vtable[12])(_r1.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac20b1c22)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c22,java_lang_Exception,59)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c22,java_io_IOException,87)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c22,java_lang_Object,111)
    XMLVM_CATCH_END(w4567aaac20b1c22)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac20b1c22)
    _r6 = _r0;
    label49:;
    XMLVM_TRY_BEGIN(w4567aaac20b1c25)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 294)
    _r6.o = _r6.o;
    java_nio_charset_Charset_addCharsets___java_nio_charset_spi_CharsetProvider_java_util_TreeMap(_r6.o, _r8.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac20b1c25)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c25,java_io_IOException,87)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c25,java_lang_Object,111)
    XMLVM_CATCH_END(w4567aaac20b1c25)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac20b1c25)
    label54:;
    XMLVM_TRY_BEGIN(w4567aaac20b1c27)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 297)

    
    // Red class access removed: java.io.BufferedReader::readLine
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac20b1c27)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c27,java_io_IOException,87)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c27,java_lang_Object,111)
    XMLVM_CATCH_END(w4567aaac20b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac20b1c27)
    goto label21;
    label59:;
    java_lang_Thread* curThread_w4567aaac20b1c30 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w4567aaac20b1c30->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w4567aaac20b1c31)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 285)
    java_nio_charset_Charset_getSystemClassLoader__();
    XMLVM_SOURCE_POSITION("Charset.java", 286)
    _r1.i = 1;
    XMLVM_SOURCE_POSITION("Charset.java", 287)
    _r3.o = java_nio_charset_Charset_GET_systemClassLoader();
    _r0.o = java_lang_Class_forName___java_lang_String_boolean_java_lang_ClassLoader(_r0.o, _r1.i, _r3.o);
    XMLVM_SOURCE_POSITION("Charset.java", 288)
    //java_lang_Class_newInstance__[12]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Class*) _r0.o)->tib->vtable[12])(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac20b1c31)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c31,java_lang_Exception,76)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c31,java_io_IOException,87)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c31,java_lang_Object,111)
    XMLVM_CATCH_END(w4567aaac20b1c31)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac20b1c31)
    _r6 = _r0;
    goto label49;
    label76:;
    XMLVM_SOURCE_POSITION("Charset.java", 289)
    java_lang_Thread* curThread_w4567aaac20b1c36 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac20b1c36->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w4567aaac20b1c37)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 290)

    
    // Red class access removed: java.lang.Error::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    //java_lang_Exception_getMessage__[7]
    XMLVM_CHECK_NPE(0)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Exception*) _r0.o)->tib->vtable[7])(_r0.o);

    
    // Red class access removed: java.lang.Error::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r1.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac20b1c37)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c37,java_io_IOException,87)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c37,java_lang_Object,111)
    XMLVM_CATCH_END(w4567aaac20b1c37)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac20b1c37)
    label87:;
    java_lang_Thread* curThread_w4567aaac20b1c39 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac20b1c39->fields.java_lang_Thread.xmlvmException_;
    _r0 = _r2;
    label89:;
    if (_r0.o == JAVA_NULL) goto label28;
    XMLVM_TRY_BEGIN(w4567aaac20b1c43)
    // Begin try

    
    // Red class access removed: java.io.BufferedReader::close
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac20b1c43)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c43,java_io_IOException,95)
    XMLVM_CATCH_END(w4567aaac20b1c43)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac20b1c43)
    goto label28;
    label95:;
    XMLVM_SOURCE_POSITION("Charset.java", 306)
    java_lang_Thread* curThread_w4567aaac20b1c47 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac20b1c47->fields.java_lang_Thread.xmlvmException_;
    goto label28;
    label97:;
    XMLVM_SOURCE_POSITION("Charset.java", 301)
    java_lang_Thread* curThread_w4567aaac20b1c51 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w4567aaac20b1c51->fields.java_lang_Thread.xmlvmException_;
    _r5 = _r1;
    _r1 = _r0;
    _r0 = _r5;
    label101:;
    if (_r1.o == JAVA_NULL) goto label106;
    XMLVM_TRY_BEGIN(w4567aaac20b1c57)
    // Begin try

    
    // Red class access removed: java.io.BufferedReader::close
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac20b1c57)
        XMLVM_CATCH_SPECIFIC(w4567aaac20b1c57,java_io_IOException,107)
    XMLVM_CATCH_END(w4567aaac20b1c57)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac20b1c57)
    label106:;
    XMLVM_SOURCE_POSITION("Charset.java", 309)
    XMLVM_THROW_CUSTOM(_r0.o)
    label107:;
    java_lang_Thread* curThread_w4567aaac20b1c62 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w4567aaac20b1c62->fields.java_lang_Thread.xmlvmException_;
    goto label106;
    label109:;
    java_lang_Thread* curThread_w4567aaac20b1c65 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac20b1c65->fields.java_lang_Thread.xmlvmException_;
    goto label28;
    label111:;
    java_lang_Thread* curThread_w4567aaac20b1c68 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac20b1c68->fields.java_lang_Thread.xmlvmException_;
    _r1 = _r2;
    goto label101;
    label114:;
    java_lang_Thread* curThread_w4567aaac20b1c72 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w4567aaac20b1c72->fields.java_lang_Thread.xmlvmException_;
    goto label89;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_availableCharsets__()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_availableCharsets__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "availableCharsets", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    // "META-INF/services/java.nio.charset.spi.CharsetProvider"
    _r0.o = xmlvm_create_java_string_from_pool(709);
    XMLVM_SOURCE_POSITION("Charset.java", 325)
    _r0.o = java_nio_charset_Charset_GET__builtInCharsets();
    if (_r0.o != JAVA_NULL) goto label32;
    XMLVM_SOURCE_POSITION("Charset.java", 326)
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    _r0.o = __CLASS_java_nio_charset_Charset;
    java_lang_Object_acquireLockRecursive__(_r0.o);
    XMLVM_TRY_BEGIN(w4567aaac21b1b8)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 327)
    _r1.o = java_nio_charset_Charset_GET__builtInCharsets();
    if (_r1.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w4567aaac21b1b8->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac21b1b8, sizeof(XMLVM_JMP_BUF)); goto label31; };
    XMLVM_SOURCE_POSITION("Charset.java", 328)

    
    // Red class access removed: java.util.TreeMap::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Charset.java", 329)

    
    // Red class access removed: java.nio.charset.Charset$IgnoreCaseComparator::getInstance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.TreeMap::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    java_nio_charset_Charset_PUT__builtInCharsets( _r1.o);
    XMLVM_SOURCE_POSITION("Charset.java", 330)
    _r1.o = java_nio_charset_Charset_GET__builtInProvider();
    _r2.o = java_nio_charset_Charset_GET__builtInCharsets();
    XMLVM_CHECK_NPE(1)
    org_apache_harmony_niochar_CharsetProviderImpl_putCharsets___java_util_Map(_r1.o, _r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac21b1b8)
        XMLVM_CATCH_SPECIFIC(w4567aaac21b1b8,java_lang_Object,64)
    XMLVM_CATCH_END(w4567aaac21b1b8)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac21b1b8)
    label31:;
    XMLVM_TRY_BEGIN(w4567aaac21b1c10)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac21b1c10)
        XMLVM_CATCH_SPECIFIC(w4567aaac21b1c10,java_lang_Object,64)
    XMLVM_CATCH_END(w4567aaac21b1c10)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac21b1c10)
    label32:;
    XMLVM_SOURCE_POSITION("Charset.java", 336)
    _r0.o = java_nio_charset_Charset_GET__builtInCharsets();
    XMLVM_SOURCE_POSITION("Charset.java", 337)

    
    // Red class access removed: java.util.TreeMap::clone
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.TreeMap::check-cast
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Charset.java", 340)
    _r2.o = java_nio_charset_Charset_getContextClassLoader__();
    XMLVM_SOURCE_POSITION("Charset.java", 343)
    if (_r2.o == JAVA_NULL) goto label67;
    XMLVM_TRY_BEGIN(w4567aaac21b1c21)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 345)
    // "META-INF/services/java.nio.charset.spi.CharsetProvider"
    _r1.o = xmlvm_create_java_string_from_pool(709);
    XMLVM_CHECK_NPE(2)
    _r1.o = java_lang_ClassLoader_getResources___java_lang_String(_r2.o, _r1.o);
    _r3 = _r1;
    XMLVM_SOURCE_POSITION("Charset.java", 344)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac21b1c21)
        XMLVM_CATCH_SPECIFIC(w4567aaac21b1c21,java_io_IOException,90)
    XMLVM_CATCH_END(w4567aaac21b1c21)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac21b1c21)
    label53:;
    XMLVM_TRY_BEGIN(w4567aaac21b1c23)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 352)
    XMLVM_CHECK_NPE(3)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_hasMoreElements__])(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac21b1c23)
        XMLVM_CATCH_SPECIFIC(w4567aaac21b1c23,java_io_IOException,90)
    XMLVM_CATCH_END(w4567aaac21b1c23)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac21b1c23)
    if (_r1.i != 0) goto label80;
    label59:;
    XMLVM_SOURCE_POSITION("Charset.java", 359)
    _r0.o = java_util_Collections_unmodifiableSortedMap___java_util_SortedMap(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label64:;
    java_lang_Thread* curThread_w4567aaac21b1c30 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w4567aaac21b1c30->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w4567aaac21b1c31)
    // Begin try
    java_lang_Object_releaseLockRecursive__(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac21b1c31)
        XMLVM_CATCH_SPECIFIC(w4567aaac21b1c31,java_lang_Object,64)
    XMLVM_CATCH_END(w4567aaac21b1c31)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac21b1c31)
    XMLVM_THROW_CUSTOM(_r1.o)
    label67:;
    XMLVM_TRY_BEGIN(w4567aaac21b1c34)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 347)
    java_nio_charset_Charset_getSystemClassLoader__();
    XMLVM_SOURCE_POSITION("Charset.java", 348)
    _r1.o = java_nio_charset_Charset_GET_systemClassLoader();
    XMLVM_SOURCE_POSITION("Charset.java", 349)
    // "META-INF/services/java.nio.charset.spi.CharsetProvider"
    _r3.o = xmlvm_create_java_string_from_pool(709);
    XMLVM_CHECK_NPE(1)
    _r1.o = java_lang_ClassLoader_getResources___java_lang_String(_r1.o, _r3.o);
    _r3 = _r1;
    { XMLVM_MEMCPY(curThread_w4567aaac21b1c34->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac21b1c34, sizeof(XMLVM_JMP_BUF)); goto label53; };
    XMLVM_SOURCE_POSITION("Charset.java", 353)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac21b1c34)
        XMLVM_CATCH_SPECIFIC(w4567aaac21b1c34,java_io_IOException,90)
    XMLVM_CATCH_END(w4567aaac21b1c34)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac21b1c34)
    label80:;
    XMLVM_TRY_BEGIN(w4567aaac21b1c36)
    // Begin try
    XMLVM_CHECK_NPE(3)
    _r1.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_nextElement__])(_r3.o);

    
    // Red class access removed: java.net.URL::check-cast
    XMLVM_RED_CLASS_DEPENDENCY();
    java_nio_charset_Charset_loadConfiguredCharsets___java_net_URL_java_lang_ClassLoader_java_util_TreeMap(_r1.o, _r2.o, _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac21b1c36)
        XMLVM_CATCH_SPECIFIC(w4567aaac21b1c36,java_io_IOException,90)
    XMLVM_CATCH_END(w4567aaac21b1c36)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac21b1c36)
    goto label53;
    label90:;
    java_lang_Thread* curThread_w4567aaac21b1c39 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w4567aaac21b1c39->fields.java_lang_Thread.xmlvmException_;
    goto label59;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_searchConfiguredCharsets___java_lang_String_java_lang_ClassLoader_java_net_URL(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_searchConfiguredCharsets___java_lang_String_java_lang_ClassLoader_java_net_URL]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "searchConfiguredCharsets", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    volatile XMLVMElem _r6;
    volatile XMLVMElem _r7;
    _r5.o = n1;
    _r6.o = n2;
    _r7.o = n3;
    _r4.o = JAVA_NULL;
    XMLVM_TRY_BEGIN(w4567aaac22b1b5)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 369)
    XMLVM_SOURCE_POSITION("Charset.java", 371)

    
    // Red class access removed: java.net.URL::openStream
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Charset.java", 373)

    
    // Red class access removed: java.io.BufferedReader::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.InputStreamReader::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Charset.java", 374)
    // "UTF-8"
    _r3.o = xmlvm_create_java_string_from_pool(100);

    
    // Red class access removed: java.io.InputStreamReader::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.io.BufferedReader::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1b5)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1b5,java_io_IOException,117)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1b5,java_lang_Object,114)
    XMLVM_CATCH_END(w4567aaac22b1b5)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1b5)
    XMLVM_TRY_BEGIN(w4567aaac22b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 375)

    
    // Red class access removed: java.io.BufferedReader::readLine
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1b6)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1b6,java_io_IOException,84)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1b6,java_lang_Object,98)
    XMLVM_CATCH_END(w4567aaac22b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1b6)
    label21:;
    XMLVM_SOURCE_POSITION("Charset.java", 376)
    if (_r0.o != JAVA_NULL) goto label31;
    _r0 = _r1;
    label24:;
    XMLVM_SOURCE_POSITION("Charset.java", 415)
    if (_r0.o == JAVA_NULL) goto label29;
    XMLVM_TRY_BEGIN(w4567aaac22b1c14)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 416)

    
    // Red class access removed: java.io.BufferedReader::close
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1c14)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c14,java_io_IOException,110)
    XMLVM_CATCH_END(w4567aaac22b1c14)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1c14)
    label29:;
    _r0 = _r4;
    label30:;
    XMLVM_SOURCE_POSITION("Charset.java", 409)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label31:;
    XMLVM_TRY_BEGIN(w4567aaac22b1c21)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 377)
    _r0.o = java_nio_charset_Charset_trimClassName___java_lang_String(_r0.o);
    XMLVM_SOURCE_POSITION("Charset.java", 378)
    //java_lang_String_length__[8]
    XMLVM_CHECK_NPE(0)
    _r2.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[8])(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1c21)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c21,java_io_IOException,84)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c21,java_lang_Object,98)
    XMLVM_CATCH_END(w4567aaac22b1c21)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1c21)
    if (_r2.i <= 0) goto label105;
    XMLVM_SOURCE_POSITION("Charset.java", 382)
    _r2.i = 1;
    XMLVM_TRY_BEGIN(w4567aaac22b1c25)
    // Begin try
    _r2.o = java_lang_Class_forName___java_lang_String_boolean_java_lang_ClassLoader(_r0.o, _r2.i, _r6.o);
    XMLVM_SOURCE_POSITION("Charset.java", 384)
    //java_lang_Class_newInstance__[12]
    XMLVM_CHECK_NPE(2)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Class*) _r2.o)->tib->vtable[12])(_r2.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1c25)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c25,java_lang_Exception,66)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c25,java_io_IOException,84)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c25,java_lang_Object,98)
    XMLVM_CATCH_END(w4567aaac22b1c25)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1c25)
    label50:;
    XMLVM_TRY_BEGIN(w4567aaac22b1c27)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 400)
    _r0.o = _r0.o;
    XMLVM_SOURCE_POSITION("Charset.java", 401)
    //java_nio_charset_spi_CharsetProvider_charsetForName___java_lang_String[6]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_nio_charset_spi_CharsetProvider*) _r0.o)->tib->vtable[6])(_r0.o, _r5.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1c27)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c27,java_io_IOException,84)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c27,java_lang_Object,98)
    XMLVM_CATCH_END(w4567aaac22b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1c27)
    XMLVM_SOURCE_POSITION("Charset.java", 402)
    if (_r0.o == JAVA_NULL) goto label105;
    if (_r1.o == JAVA_NULL) goto label30;
    XMLVM_TRY_BEGIN(w4567aaac22b1c31)
    // Begin try

    
    // Red class access removed: java.io.BufferedReader::close
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1c31)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c31,java_io_IOException,64)
    XMLVM_CATCH_END(w4567aaac22b1c31)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1c31)
    goto label30;
    label64:;
    java_lang_Thread* curThread_w4567aaac22b1c34 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w4567aaac22b1c34->fields.java_lang_Thread.xmlvmException_;
    goto label30;
    label66:;
    java_lang_Thread* curThread_w4567aaac22b1c37 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r2.o = curThread_w4567aaac22b1c37->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w4567aaac22b1c38)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 389)
    java_nio_charset_Charset_getSystemClassLoader__();
    XMLVM_SOURCE_POSITION("Charset.java", 390)
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Charset.java", 391)
    _r3.o = java_nio_charset_Charset_GET_systemClassLoader();
    _r0.o = java_lang_Class_forName___java_lang_String_boolean_java_lang_ClassLoader(_r0.o, _r2.i, _r3.o);
    XMLVM_SOURCE_POSITION("Charset.java", 392)
    //java_lang_Class_newInstance__[12]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Class*) _r0.o)->tib->vtable[12])(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1c38)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c38,java_lang_Exception,87)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c38,java_io_IOException,84)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c38,java_lang_Object,98)
    XMLVM_CATCH_END(w4567aaac22b1c38)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1c38)
    goto label50;
    label82:;
    XMLVM_SOURCE_POSITION("Charset.java", 393)
    java_lang_Thread* curThread_w4567aaac22b1c42 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac22b1c42->fields.java_lang_Thread.xmlvmException_;
    XMLVM_TRY_BEGIN(w4567aaac22b1c43)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 394)
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1c43)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c43,java_io_IOException,84)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c43,java_lang_Object,98)
    XMLVM_CATCH_END(w4567aaac22b1c43)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1c43)
    label84:;
    XMLVM_TRY_BEGIN(w4567aaac22b1c45)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 412)
    java_lang_Thread* curThread_w4567aaac22b1c45ab1 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac22b1c45ab1->fields.java_lang_Thread.xmlvmException_;
    _r0 = _r1;
    { XMLVM_MEMCPY(curThread_w4567aaac22b1c45->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac22b1c45, sizeof(XMLVM_JMP_BUF)); goto label24; };
    XMLVM_SOURCE_POSITION("Charset.java", 395)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1c45)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c45,java_io_IOException,84)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c45,java_lang_Object,98)
    XMLVM_CATCH_END(w4567aaac22b1c45)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1c45)
    label87:;
    XMLVM_TRY_BEGIN(w4567aaac22b1c47)
    // Begin try
    java_lang_Thread* curThread_w4567aaac22b1c47aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac22b1c47aa->fields.java_lang_Thread.xmlvmException_;
    XMLVM_SOURCE_POSITION("Charset.java", 396)

    
    // Red class access removed: java.lang.Error::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();
    //java_lang_Exception_getMessage__[7]
    XMLVM_CHECK_NPE(0)
    _r3.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Exception*) _r0.o)->tib->vtable[7])(_r0.o);

    
    // Red class access removed: java.lang.Error::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r2.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1c47)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c47,java_io_IOException,84)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c47,java_lang_Object,98)
    XMLVM_CATCH_END(w4567aaac22b1c47)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1c47)
    label98:;
    XMLVM_SOURCE_POSITION("Charset.java", 413)
    java_lang_Thread* curThread_w4567aaac22b1c50 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac22b1c50->fields.java_lang_Thread.xmlvmException_;
    label99:;
    if (_r1.o == JAVA_NULL) goto label104;
    XMLVM_TRY_BEGIN(w4567aaac22b1c53)
    // Begin try

    
    // Red class access removed: java.io.BufferedReader::close
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1c53)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c53,java_io_IOException,112)
    XMLVM_CATCH_END(w4567aaac22b1c53)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1c53)
    label104:;
    XMLVM_SOURCE_POSITION("Charset.java", 421)
    XMLVM_THROW_CUSTOM(_r0.o)
    label105:;
    XMLVM_TRY_BEGIN(w4567aaac22b1c58)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 407)

    
    // Red class access removed: java.io.BufferedReader::readLine
    XMLVM_RED_CLASS_DEPENDENCY();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac22b1c58)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c58,java_io_IOException,84)
        XMLVM_CATCH_SPECIFIC(w4567aaac22b1c58,java_lang_Object,98)
    XMLVM_CATCH_END(w4567aaac22b1c58)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac22b1c58)
    goto label21;
    label110:;
    java_lang_Thread* curThread_w4567aaac22b1c61 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac22b1c61->fields.java_lang_Thread.xmlvmException_;
    goto label29;
    label112:;
    java_lang_Thread* curThread_w4567aaac22b1c64 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r1.o = curThread_w4567aaac22b1c64->fields.java_lang_Thread.xmlvmException_;
    goto label104;
    label114:;
    java_lang_Thread* curThread_w4567aaac22b1c67 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac22b1c67->fields.java_lang_Thread.xmlvmException_;
    _r1 = _r4;
    goto label99;
    label117:;
    java_lang_Thread* curThread_w4567aaac22b1c71 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac22b1c71->fields.java_lang_Thread.xmlvmException_;
    _r0 = _r4;
    goto label24;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_forNameInternal___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_forNameInternal___java_lang_String]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "forNameInternal", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    volatile XMLVMElem _r5;
    _r5.o = n1;
    // "META-INF/services/java.nio.charset.spi.CharsetProvider"
    _r0.o = xmlvm_create_java_string_from_pool(709);
    XMLVM_SOURCE_POSITION("Charset.java", 431)
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    _r1.o = __CLASS_java_nio_charset_Charset;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_TRY_BEGIN(w4567aaac23b1b6)
    // Begin try
    _r0.o = java_nio_charset_Charset_GET_cachedCharsetTable();
    //java_util_HashMap_get___java_lang_Object[10]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[10])(_r0.o, _r5.o);
    _r0.o = _r0.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1b6)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1b6,java_lang_Object,25)
    XMLVM_CATCH_END(w4567aaac23b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1b6)
    XMLVM_SOURCE_POSITION("Charset.java", 432)
    if (_r0.o == JAVA_NULL) goto label17;
    label15:;
    XMLVM_SOURCE_POSITION("Charset.java", 484)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label17:;
    XMLVM_SOURCE_POSITION("Charset.java", 436)
    if (_r5.o != JAVA_NULL) goto label28;
    XMLVM_TRY_BEGIN(w4567aaac23b1c16)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 437)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c16)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c16,java_lang_Object,25)
    XMLVM_CATCH_END(w4567aaac23b1c16)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c16)
    label25:;
    java_lang_Thread* curThread_w4567aaac23b1c18 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac23b1c18->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label28:;
    XMLVM_TRY_BEGIN(w4567aaac23b1c22)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 439)
    java_nio_charset_Charset_checkCharsetName___java_lang_String(_r5.o);
    XMLVM_SOURCE_POSITION("Charset.java", 441)
    _r0.o = java_nio_charset_Charset_GET__builtInProvider();
    if (_r0.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w4567aaac23b1c22->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac23b1c22, sizeof(XMLVM_JMP_BUF)); goto label42; };
    XMLVM_SOURCE_POSITION("Charset.java", 442)
    _r0.o = __NEW_org_apache_harmony_niochar_CharsetProviderImpl();
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_niochar_CharsetProviderImpl___INIT___(_r0.o);
    java_nio_charset_Charset_PUT__builtInProvider( _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c22)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c22,java_lang_Object,25)
    XMLVM_CATCH_END(w4567aaac23b1c22)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c22)
    label42:;
    XMLVM_TRY_BEGIN(w4567aaac23b1c24)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 444)
    _r0.o = java_nio_charset_Charset_GET__builtInProvider();
    //org_apache_harmony_niochar_CharsetProviderImpl_charsetForName___java_lang_String[6]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((org_apache_harmony_niochar_CharsetProviderImpl*) _r0.o)->tib->vtable[6])(_r0.o, _r5.o);
    XMLVM_SOURCE_POSITION("Charset.java", 445)
    if (_r0.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w4567aaac23b1c24->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac23b1c24, sizeof(XMLVM_JMP_BUF)); goto label54; };
    XMLVM_SOURCE_POSITION("Charset.java", 446)
    java_nio_charset_Charset_cacheCharset___java_nio_charset_Charset_java_lang_String(_r0.o, _r5.o);
    { XMLVM_MEMCPY(curThread_w4567aaac23b1c24->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac23b1c24, sizeof(XMLVM_JMP_BUF)); goto label15; };
    XMLVM_SOURCE_POSITION("Charset.java", 451)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c24)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c24,java_lang_Object,25)
    XMLVM_CATCH_END(w4567aaac23b1c24)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c24)
    label54:;
    XMLVM_TRY_BEGIN(w4567aaac23b1c26)
    // Begin try
    _r2.o = java_nio_charset_Charset_getContextClassLoader__();
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c26)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c26,java_lang_Object,25)
    XMLVM_CATCH_END(w4567aaac23b1c26)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c26)
    XMLVM_SOURCE_POSITION("Charset.java", 454)
    if (_r2.o == JAVA_NULL) goto label78;
    XMLVM_TRY_BEGIN(w4567aaac23b1c29)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 456)
    // "META-INF/services/java.nio.charset.spi.CharsetProvider"
    _r0.o = xmlvm_create_java_string_from_pool(709);
    XMLVM_CHECK_NPE(2)
    _r0.o = java_lang_ClassLoader_getResources___java_lang_String(_r2.o, _r0.o);
    _r3 = _r0;
    XMLVM_SOURCE_POSITION("Charset.java", 455)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c29)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c29,java_io_IOException,131)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c29,java_lang_Object,136)
    XMLVM_CATCH_END(w4567aaac23b1c29)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c29)
    label67:;
    XMLVM_TRY_BEGIN(w4567aaac23b1c31)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 469)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_hasMoreElements__])(_r3.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c31)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c31,java_io_IOException,131)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c31,java_lang_Object,136)
    XMLVM_CATCH_END(w4567aaac23b1c31)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c31)
    if (_r0.i != 0) goto label106;
    XMLVM_SOURCE_POSITION("Charset.java", 482)
    _r0.i = 0;
    XMLVM_TRY_BEGIN(w4567aaac23b1c35)
    // Begin try
    java_nio_charset_Charset_PUT_inForNameInternal( _r0.i);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c35)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c35,java_lang_Object,25)
    XMLVM_CATCH_END(w4567aaac23b1c35)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c35)
    label76:;
    _r0.o = JAVA_NULL;
    goto label15;
    label78:;
    XMLVM_TRY_BEGIN(w4567aaac23b1c40)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 458)
    java_nio_charset_Charset_getSystemClassLoader__();
    XMLVM_SOURCE_POSITION("Charset.java", 459)
    _r0.o = java_nio_charset_Charset_GET_systemClassLoader();
    if (_r0.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w4567aaac23b1c40->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac23b1c40, sizeof(XMLVM_JMP_BUF)); goto label96; };
    XMLVM_SOURCE_POSITION("Charset.java", 461)

    
    // Red class access removed: java.util.Vector::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Vector::<init>
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.util.Vector::elements
    XMLVM_RED_CLASS_DEPENDENCY();
    _r3 = _r0;
    { XMLVM_MEMCPY(curThread_w4567aaac23b1c40->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac23b1c40, sizeof(XMLVM_JMP_BUF)); goto label67; };
    XMLVM_SOURCE_POSITION("Charset.java", 463)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c40)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c40,java_io_IOException,131)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c40,java_lang_Object,136)
    XMLVM_CATCH_END(w4567aaac23b1c40)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c40)
    label96:;
    XMLVM_TRY_BEGIN(w4567aaac23b1c42)
    // Begin try
    _r0.o = java_nio_charset_Charset_GET_systemClassLoader();
    XMLVM_SOURCE_POSITION("Charset.java", 464)
    // "META-INF/services/java.nio.charset.spi.CharsetProvider"
    _r3.o = xmlvm_create_java_string_from_pool(709);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_ClassLoader_getResources___java_lang_String(_r0.o, _r3.o);
    _r3 = _r0;
    { XMLVM_MEMCPY(curThread_w4567aaac23b1c42->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac23b1c42, sizeof(XMLVM_JMP_BUF)); goto label67; };
    XMLVM_SOURCE_POSITION("Charset.java", 470)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c42)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c42,java_io_IOException,131)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c42,java_lang_Object,136)
    XMLVM_CATCH_END(w4567aaac23b1c42)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c42)
    label106:;
    XMLVM_TRY_BEGIN(w4567aaac23b1c44)
    // Begin try
    _r0.i = 1;
    java_nio_charset_Charset_PUT_inForNameInternal( _r0.i);
    XMLVM_SOURCE_POSITION("Charset.java", 472)
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Enumeration_nextElement__])(_r3.o);

    
    // Red class access removed: java.net.URL::check-cast
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Charset.java", 471)
    _r0.o = java_nio_charset_Charset_searchConfiguredCharsets___java_lang_String_java_lang_ClassLoader_java_net_URL(_r5.o, _r2.o, _r0.o);
    XMLVM_SOURCE_POSITION("Charset.java", 473)
    _r4.i = 0;
    java_nio_charset_Charset_PUT_inForNameInternal( _r4.i);
    XMLVM_SOURCE_POSITION("Charset.java", 474)
    if (_r0.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w4567aaac23b1c44->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac23b1c44, sizeof(XMLVM_JMP_BUF)); goto label67; };
    XMLVM_SOURCE_POSITION("Charset.java", 475)
    java_nio_charset_Charset_cacheCharset___java_nio_charset_Charset_java_lang_String(_r0.o, _r5.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c44)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c44,java_io_IOException,131)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c44,java_lang_Object,136)
    XMLVM_CATCH_END(w4567aaac23b1c44)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c44)
    _r2.i = 0;
    XMLVM_TRY_BEGIN(w4567aaac23b1c46)
    // Begin try
    java_nio_charset_Charset_PUT_inForNameInternal( _r2.i);
    { XMLVM_MEMCPY(curThread_w4567aaac23b1c46->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac23b1c46, sizeof(XMLVM_JMP_BUF)); goto label15; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c46)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c46,java_lang_Object,25)
    XMLVM_CATCH_END(w4567aaac23b1c46)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c46)
    label131:;
    XMLVM_TRY_BEGIN(w4567aaac23b1c48)
    // Begin try
    java_lang_Thread* curThread_w4567aaac23b1c48aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac23b1c48aa->fields.java_lang_Thread.xmlvmException_;
    _r0.i = 0;
    java_nio_charset_Charset_PUT_inForNameInternal( _r0.i);
    { XMLVM_MEMCPY(curThread_w4567aaac23b1c48->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac23b1c48, sizeof(XMLVM_JMP_BUF)); goto label76; };
    XMLVM_SOURCE_POSITION("Charset.java", 481)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c48)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c48,java_lang_Object,25)
    XMLVM_CATCH_END(w4567aaac23b1c48)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c48)
    label136:;
    XMLVM_TRY_BEGIN(w4567aaac23b1c50)
    // Begin try
    java_lang_Thread* curThread_w4567aaac23b1c50aa = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac23b1c50aa->fields.java_lang_Thread.xmlvmException_;
    _r2.i = 0;
    java_nio_charset_Charset_PUT_inForNameInternal( _r2.i);
    XMLVM_SOURCE_POSITION("Charset.java", 483)
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac23b1c50)
        XMLVM_CATCH_SPECIFIC(w4567aaac23b1c50,java_lang_Object,25)
    XMLVM_CATCH_END(w4567aaac23b1c50)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac23b1c50)
    //XMLVM_END_WRAPPER
}

void java_nio_charset_Charset_cacheCharset___java_nio_charset_Charset_java_lang_String(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_cacheCharset___java_nio_charset_Charset_java_lang_String]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "cacheCharset", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = n1;
    _r3.o = n2;
    XMLVM_SOURCE_POSITION("Charset.java", 491)
    XMLVM_CHECK_NPE(2)
    _r0.o = java_nio_charset_Charset_name__(_r2.o);
    XMLVM_SOURCE_POSITION("Charset.java", 492)
    _r1.o = java_nio_charset_Charset_GET_cachedCharsetTable();
    //java_util_HashMap_containsKey___java_lang_Object[7]
    XMLVM_CHECK_NPE(1)
    _r1.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) _r1.o)->tib->vtable[7])(_r1.o, _r0.o);
    if (_r1.i != 0) goto label17;
    XMLVM_SOURCE_POSITION("Charset.java", 493)
    _r1.o = java_nio_charset_Charset_GET_cachedCharsetTable();
    //java_util_HashMap_put___java_lang_Object_java_lang_Object[14]
    XMLVM_CHECK_NPE(1)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) _r1.o)->tib->vtable[14])(_r1.o, _r0.o, _r2.o);
    label17:;
    XMLVM_SOURCE_POSITION("Charset.java", 496)
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[1])(_r0.o, _r3.o);
    if (_r0.i != 0) goto label36;
    XMLVM_SOURCE_POSITION("Charset.java", 497)
    _r0.o = java_nio_charset_Charset_GET_cachedCharsetTable();
    //java_util_HashMap_containsKey___java_lang_Object[7]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[7])(_r0.o, _r3.o);
    if (_r0.i != 0) goto label36;
    XMLVM_SOURCE_POSITION("Charset.java", 498)
    _r0.o = java_nio_charset_Charset_GET_cachedCharsetTable();
    //java_util_HashMap_put___java_lang_Object_java_lang_Object[14]
    XMLVM_CHECK_NPE(0)
    (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[14])(_r0.o, _r3.o, _r2.o);
    label36:;
    XMLVM_SOURCE_POSITION("Charset.java", 501)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_forName___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_forName___java_lang_String]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "forName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 515)
    _r0.o = java_nio_charset_Charset_forNameInternal___java_lang_String(_r1.o);
    XMLVM_SOURCE_POSITION("Charset.java", 516)
    if (_r0.o != JAVA_NULL) goto label12;
    XMLVM_SOURCE_POSITION("Charset.java", 517)

    
    // Red class access removed: java.nio.charset.UnsupportedCharsetException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.charset.UnsupportedCharsetException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label12:;
    XMLVM_SOURCE_POSITION("Charset.java", 519)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_Charset_isSupported___java_lang_String(JAVA_OBJECT n1)
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_isSupported___java_lang_String]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "isSupported", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r4.o = n1;
    _r3.i = 0;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Charset.java", 532)
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    _r1.o = __CLASS_java_nio_charset_Charset;
    java_lang_Object_acquireLockRecursive__(_r1.o);
    XMLVM_TRY_BEGIN(w4567aaac26b1b7)
    // Begin try
    _r0.i = java_nio_charset_Charset_GET_inForNameInternal();
    if (_r0.i == 0) { XMLVM_MEMCPY(curThread_w4567aaac26b1b7->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac26b1b7, sizeof(XMLVM_JMP_BUF)); goto label62; };
    XMLVM_SOURCE_POSITION("Charset.java", 533)
    _r0.o = java_nio_charset_Charset_GET_cachedCharsetTable();
    //java_util_HashMap_get___java_lang_Object[10]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) _r0.o)->tib->vtable[10])(_r0.o, _r4.o);
    _r0.o = _r0.o;
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac26b1b7)
        XMLVM_CATCH_SPECIFIC(w4567aaac26b1b7,java_lang_Object,30)
    XMLVM_CATCH_END(w4567aaac26b1b7)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac26b1b7)
    XMLVM_SOURCE_POSITION("Charset.java", 534)
    if (_r0.o == JAVA_NULL) goto label22;
    _r0 = _r2;
    label20:;
    XMLVM_SOURCE_POSITION("Charset.java", 535)
    XMLVM_SOURCE_POSITION("Charset.java", 555)
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label22:;
    XMLVM_SOURCE_POSITION("Charset.java", 538)
    if (_r4.o != JAVA_NULL) goto label33;
    XMLVM_TRY_BEGIN(w4567aaac26b1c19)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 539)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac26b1c19)
        XMLVM_CATCH_SPECIFIC(w4567aaac26b1c19,java_lang_Object,30)
    XMLVM_CATCH_END(w4567aaac26b1c19)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac26b1c19)
    label30:;
    java_lang_Thread* curThread_w4567aaac26b1c21 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac26b1c21->fields.java_lang_Thread.xmlvmException_;
    java_lang_Object_releaseLockRecursive__(_r1.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label33:;
    XMLVM_TRY_BEGIN(w4567aaac26b1c25)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 541)
    java_nio_charset_Charset_checkCharsetName___java_lang_String(_r4.o);
    XMLVM_SOURCE_POSITION("Charset.java", 544)
    _r0.o = java_nio_charset_Charset_GET__builtInProvider();
    if (_r0.o != JAVA_NULL) { XMLVM_MEMCPY(curThread_w4567aaac26b1c25->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac26b1c25, sizeof(XMLVM_JMP_BUF)); goto label47; };
    XMLVM_SOURCE_POSITION("Charset.java", 545)
    _r0.o = __NEW_org_apache_harmony_niochar_CharsetProviderImpl();
    XMLVM_CHECK_NPE(0)
    org_apache_harmony_niochar_CharsetProviderImpl___INIT___(_r0.o);
    java_nio_charset_Charset_PUT__builtInProvider( _r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac26b1c25)
        XMLVM_CATCH_SPECIFIC(w4567aaac26b1c25,java_lang_Object,30)
    XMLVM_CATCH_END(w4567aaac26b1c25)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac26b1c25)
    label47:;
    XMLVM_TRY_BEGIN(w4567aaac26b1c27)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 547)
    _r0.o = java_nio_charset_Charset_GET__builtInProvider();
    //org_apache_harmony_niochar_CharsetProviderImpl_charsetForName___java_lang_String[6]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((org_apache_harmony_niochar_CharsetProviderImpl*) _r0.o)->tib->vtable[6])(_r0.o, _r4.o);
    XMLVM_SOURCE_POSITION("Charset.java", 548)
    if (_r0.o == JAVA_NULL) { XMLVM_MEMCPY(curThread_w4567aaac26b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac26b1c27, sizeof(XMLVM_JMP_BUF)); goto label60; };
    XMLVM_SOURCE_POSITION("Charset.java", 549)
    java_nio_charset_Charset_cacheCharset___java_nio_charset_Charset_java_lang_String(_r0.o, _r4.o);
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Charset.java", 550)
    { XMLVM_MEMCPY(curThread_w4567aaac26b1c27->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac26b1c27, sizeof(XMLVM_JMP_BUF)); goto label20; };
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac26b1c27)
        XMLVM_CATCH_SPECIFIC(w4567aaac26b1c27,java_lang_Object,30)
    XMLVM_CATCH_END(w4567aaac26b1c27)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac26b1c27)
    label60:;
    XMLVM_TRY_BEGIN(w4567aaac26b1c29)
    // Begin try
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Charset.java", 552)
    { XMLVM_MEMCPY(curThread_w4567aaac26b1c29->fields.java_lang_Thread.xmlvmExceptionEnv_, local_env_w4567aaac26b1c29, sizeof(XMLVM_JMP_BUF)); goto label20; };
    XMLVM_SOURCE_POSITION("Charset.java", 554)
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac26b1c29)
        XMLVM_CATCH_SPECIFIC(w4567aaac26b1c29,java_lang_Object,30)
    XMLVM_CATCH_END(w4567aaac26b1c29)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac26b1c29)
    label62:;
    XMLVM_TRY_BEGIN(w4567aaac26b1c31)
    // Begin try
    _r0.o = java_nio_charset_Charset_forNameInternal___java_lang_String(_r4.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac26b1c31)
        XMLVM_CATCH_SPECIFIC(w4567aaac26b1c31,java_lang_Object,30)
    XMLVM_CATCH_END(w4567aaac26b1c31)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac26b1c31)
    if (_r0.o == JAVA_NULL) goto label70;
    _r0 = _r2;
    goto label20;
    label70:;
    _r0 = _r3;
    goto label20;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_name__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_name__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "name", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Charset.java", 591)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_nio_charset_Charset*) _r1.o)->fields.java_nio_charset_Charset.canonicalName_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_aliases__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_aliases__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "aliases", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Charset.java", 600)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_nio_charset_Charset*) _r1.o)->fields.java_nio_charset_Charset.aliasesSet_;
    _r0.o = java_util_Collections_unmodifiableSet___java_util_Set(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_displayName__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_displayName__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "displayName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Charset.java", 612)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_nio_charset_Charset*) _r1.o)->fields.java_nio_charset_Charset.canonicalName_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_displayName___java_util_Locale(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_displayName___java_util_Locale]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "displayName", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 626)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_nio_charset_Charset*) _r1.o)->fields.java_nio_charset_Charset.canonicalName_;
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_Charset_isRegistered__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_isRegistered__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "isRegistered", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Charset.java", 637)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_nio_charset_Charset*) _r2.o)->fields.java_nio_charset_Charset.canonicalName_;
    // "x-"
    _r1.o = xmlvm_create_java_string_from_pool(711);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_startsWith___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label22;
    XMLVM_SOURCE_POSITION("Charset.java", 638)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_nio_charset_Charset*) _r2.o)->fields.java_nio_charset_Charset.canonicalName_;
    // "X-"
    _r1.o = xmlvm_create_java_string_from_pool(712);
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_startsWith___java_lang_String(_r0.o, _r1.o);
    if (_r0.i != 0) goto label22;
    _r0.i = 1;
    label21:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label22:;
    _r0.i = 0;
    goto label21;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_Charset_canEncode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_canEncode__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "canEncode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Charset.java", 647)
    _r0.i = 1;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_encode___java_nio_CharBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_encode___java_nio_CharBuffer]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "encode", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_TRY_BEGIN(w4567aaac36b1b3)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 663)
    //java_nio_charset_Charset_newEncoder__[9]
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_nio_charset_Charset*) _r3.o)->tib->vtable[9])(_r3.o);
    XMLVM_SOURCE_POSITION("Charset.java", 664)
    _r1.o = java_nio_charset_CodingErrorAction_GET_REPLACE();
    XMLVM_CHECK_NPE(0)
    _r0.o = java_nio_charset_CharsetEncoder_onMalformedInput___java_nio_charset_CodingErrorAction(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("Charset.java", 665)
    _r1.o = java_nio_charset_CodingErrorAction_GET_REPLACE();
    XMLVM_CHECK_NPE(0)
    _r0.o = java_nio_charset_CharsetEncoder_onUnmappableCharacter___java_nio_charset_CodingErrorAction(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_nio_charset_CharsetEncoder_encode___java_nio_CharBuffer(_r0.o, _r4.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac36b1b3)
    XMLVM_CATCH_END(w4567aaac36b1b3)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac36b1b3)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label21:;
    XMLVM_SOURCE_POSITION("Charset.java", 668)
    java_lang_Thread* curThread_w4567aaac36b1b7 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac36b1b7->fields.java_lang_Thread.xmlvmException_;
    XMLVM_SOURCE_POSITION("Charset.java", 669)

    
    // Red class access removed: java.lang.Error::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.charset.CharacterCodingException::getMessage
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.Error::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_encode___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_encode___java_lang_String]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "encode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 684)
    _r0.o = java_nio_CharBuffer_wrap___java_lang_CharSequence(_r2.o);
    XMLVM_CHECK_NPE(1)
    _r0.o = java_nio_charset_Charset_encode___java_nio_CharBuffer(_r1.o, _r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_decode___java_nio_ByteBuffer(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_decode___java_nio_ByteBuffer]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "decode", "?")
    volatile XMLVMElem _r0;
    volatile XMLVMElem _r1;
    volatile XMLVMElem _r2;
    volatile XMLVMElem _r3;
    volatile XMLVMElem _r4;
    _r3.o = me;
    _r4.o = n1;
    XMLVM_TRY_BEGIN(w4567aaac38b1b3)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 701)
    //java_nio_charset_Charset_newDecoder__[8]
    XMLVM_CHECK_NPE(3)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_nio_charset_Charset*) _r3.o)->tib->vtable[8])(_r3.o);
    XMLVM_SOURCE_POSITION("Charset.java", 702)
    _r1.o = java_nio_charset_CodingErrorAction_GET_REPLACE();
    XMLVM_CHECK_NPE(0)
    _r0.o = java_nio_charset_CharsetDecoder_onMalformedInput___java_nio_charset_CodingErrorAction(_r0.o, _r1.o);
    XMLVM_SOURCE_POSITION("Charset.java", 703)
    _r1.o = java_nio_charset_CodingErrorAction_GET_REPLACE();
    XMLVM_CHECK_NPE(0)
    _r0.o = java_nio_charset_CharsetDecoder_onUnmappableCharacter___java_nio_charset_CodingErrorAction(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_nio_charset_CharsetDecoder_decode___java_nio_ByteBuffer(_r0.o, _r4.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac38b1b3)
    XMLVM_CATCH_END(w4567aaac38b1b3)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac38b1b3)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label21:;
    XMLVM_SOURCE_POSITION("Charset.java", 706)
    java_lang_Thread* curThread_w4567aaac38b1b7 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac38b1b7->fields.java_lang_Thread.xmlvmException_;
    XMLVM_SOURCE_POSITION("Charset.java", 707)

    
    // Red class access removed: java.lang.Error::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.charset.CharacterCodingException::getMessage
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.Error::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r1.o)
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_charset_Charset_compareTo___java_nio_charset_Charset(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_compareTo___java_nio_charset_Charset]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 727)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_nio_charset_Charset*) _r2.o)->fields.java_nio_charset_Charset.canonicalName_;
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_nio_charset_Charset*) _r3.o)->fields.java_nio_charset_Charset.canonicalName_;
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_String_compareToIgnoreCase___java_lang_String(_r0.o, _r1.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_nio_charset_Charset_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 746)
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    _r0.i = XMLVM_ISA(_r3.o, __CLASS_java_nio_charset_Charset);
    if (_r0.i == 0) goto label15;
    XMLVM_SOURCE_POSITION("Charset.java", 747)
    _r3.o = _r3.o;
    XMLVM_SOURCE_POSITION("Charset.java", 748)
    XMLVM_CHECK_NPE(2)
    _r0.o = ((java_nio_charset_Charset*) _r2.o)->fields.java_nio_charset_Charset.canonicalName_;
    XMLVM_CHECK_NPE(3)
    _r1.o = ((java_nio_charset_Charset*) _r3.o)->fields.java_nio_charset_Charset.canonicalName_;
    //java_lang_String_equals___java_lang_Object[1]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[1])(_r0.o, _r1.o);
    label14:;
    XMLVM_SOURCE_POSITION("Charset.java", 750)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label15:;
    _r0.i = 0;
    goto label14;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_charset_Charset_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_hashCode__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Charset.java", 760)
    XMLVM_CHECK_NPE(1)
    _r0.o = ((java_nio_charset_Charset*) _r1.o)->fields.java_nio_charset_Charset.canonicalName_;
    //java_lang_String_hashCode__[4]
    XMLVM_CHECK_NPE(0)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_lang_String*) _r0.o)->tib->vtable[4])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_toString__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.o = me;
    XMLVM_SOURCE_POSITION("Charset.java", 771)
    _r0.o = __NEW_java_lang_StringBuilder();
    // "Charset["
    _r1.o = xmlvm_create_java_string_from_pool(713);
    XMLVM_CHECK_NPE(0)
    java_lang_StringBuilder___INIT____java_lang_String(_r0.o, _r1.o);
    XMLVM_CHECK_NPE(2)
    _r1.o = ((java_nio_charset_Charset*) _r2.o)->fields.java_nio_charset_Charset.canonicalName_;
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    // "]"
    _r1.o = xmlvm_create_java_string_from_pool(183);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_StringBuilder_append___java_lang_String(_r0.o, _r1.o);
    //java_lang_StringBuilder_toString__[5]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_StringBuilder*) _r0.o)->tib->vtable[5])(_r0.o);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_nio_charset_Charset_defaultCharset__()
{
    XMLVM_CLASS_INIT(java_nio_charset_Charset)
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_defaultCharset__]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "defaultCharset", "?")
    volatile XMLVMElem _r0;
    XMLVM_SOURCE_POSITION("Charset.java", 782)

    
    // Red class access removed: java.nio.charset.Charset$4::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.nio.charset.Charset$4::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    _r0.o = java_security_AccessController_doPrivileged___java_security_PrivilegedAction(_r0.o);
    _r0.o = _r0.o;
    XMLVM_TRY_BEGIN(w4567aaac43b1b6)
    // Begin try
    XMLVM_SOURCE_POSITION("Charset.java", 788)
    _r0.o = java_nio_charset_Charset_forName___java_lang_String(_r0.o);
    // End try
    XMLVM_TRY_END
    XMLVM_CATCH_BEGIN(w4567aaac43b1b6)
    XMLVM_CATCH_END(w4567aaac43b1b6)
    XMLVM_RESTORE_EXCEPTION_ENV(w4567aaac43b1b6)
    label15:;
    XMLVM_SOURCE_POSITION("Charset.java", 792)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label16:;
    XMLVM_SOURCE_POSITION("Charset.java", 790)
    java_lang_Thread* curThread_w4567aaac43b1c12 = (java_lang_Thread*)java_lang_Thread_currentThread__();
    _r0.o = curThread_w4567aaac43b1c12->fields.java_lang_Thread.xmlvmException_;
    // "UTF-8"
    _r0.o = xmlvm_create_java_string_from_pool(100);
    _r0.o = java_nio_charset_Charset_forName___java_lang_String(_r0.o);
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_nio_charset_Charset_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_nio_charset_Charset_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.nio.charset.Charset", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Charset.java", 1)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_nio_charset_Charset_compareTo___java_nio_charset_Charset(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

