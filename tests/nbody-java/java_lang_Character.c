#include "xmlvm.h"
#include "java_lang_CharSequence.h"
#include "java_lang_Class.h"
#include "java_lang_IndexOutOfBoundsException.h"
#include "java_lang_NullPointerException.h"
#include "java_lang_String.h"

#include "java_lang_Character.h"

#define XMLVM_CURRENT_CLASS_NAME Character
#define XMLVM_CURRENT_PKG_CLASS_NAME java_lang_Character

__TIB_DEFINITION_java_lang_Character __TIB_java_lang_Character = {
    0, // classInitializationBegan
    0, // classInitialized
    -1, // initializerThreadId
    __INIT_java_lang_Character, // classInitializer
    "java.lang.Character", // className
    "java.lang", // package
    JAVA_NULL, // enclosingClassName
    JAVA_NULL, // enclosingMethodName
    "Ljava/lang/Object;Ljava/io/Serializable;Ljava/lang/Comparable<Ljava/lang/Character;>;", // signature
    (__TIB_DEFINITION_TEMPLATE*) &__TIB_java_lang_Object, // extends
    sizeof(java_lang_Character), // sizeInstance
    XMLVM_TYPE_CLASS};

JAVA_OBJECT __CLASS_java_lang_Character;
JAVA_OBJECT __CLASS_java_lang_Character_1ARRAY;
JAVA_OBJECT __CLASS_java_lang_Character_2ARRAY;
JAVA_OBJECT __CLASS_java_lang_Character_3ARRAY;
//XMLVM_BEGIN_IMPLEMENTATION
//XMLVM_END_IMPLEMENTATION

static JAVA_LONG _STATIC_java_lang_Character_serialVersionUID;
static JAVA_CHAR _STATIC_java_lang_Character_MIN_VALUE;
static JAVA_CHAR _STATIC_java_lang_Character_MAX_VALUE;
static JAVA_INT _STATIC_java_lang_Character_MIN_RADIX;
static JAVA_INT _STATIC_java_lang_Character_MAX_RADIX;
static JAVA_OBJECT _STATIC_java_lang_Character_TYPE;
static JAVA_BYTE _STATIC_java_lang_Character_UNASSIGNED;
static JAVA_BYTE _STATIC_java_lang_Character_UPPERCASE_LETTER;
static JAVA_BYTE _STATIC_java_lang_Character_LOWERCASE_LETTER;
static JAVA_BYTE _STATIC_java_lang_Character_TITLECASE_LETTER;
static JAVA_BYTE _STATIC_java_lang_Character_MODIFIER_LETTER;
static JAVA_BYTE _STATIC_java_lang_Character_OTHER_LETTER;
static JAVA_BYTE _STATIC_java_lang_Character_NON_SPACING_MARK;
static JAVA_BYTE _STATIC_java_lang_Character_ENCLOSING_MARK;
static JAVA_BYTE _STATIC_java_lang_Character_COMBINING_SPACING_MARK;
static JAVA_BYTE _STATIC_java_lang_Character_DECIMAL_DIGIT_NUMBER;
static JAVA_BYTE _STATIC_java_lang_Character_LETTER_NUMBER;
static JAVA_BYTE _STATIC_java_lang_Character_OTHER_NUMBER;
static JAVA_BYTE _STATIC_java_lang_Character_SPACE_SEPARATOR;
static JAVA_BYTE _STATIC_java_lang_Character_LINE_SEPARATOR;
static JAVA_BYTE _STATIC_java_lang_Character_PARAGRAPH_SEPARATOR;
static JAVA_BYTE _STATIC_java_lang_Character_CONTROL;
static JAVA_BYTE _STATIC_java_lang_Character_FORMAT;
static JAVA_BYTE _STATIC_java_lang_Character_PRIVATE_USE;
static JAVA_BYTE _STATIC_java_lang_Character_SURROGATE;
static JAVA_BYTE _STATIC_java_lang_Character_DASH_PUNCTUATION;
static JAVA_BYTE _STATIC_java_lang_Character_START_PUNCTUATION;
static JAVA_BYTE _STATIC_java_lang_Character_END_PUNCTUATION;
static JAVA_BYTE _STATIC_java_lang_Character_CONNECTOR_PUNCTUATION;
static JAVA_BYTE _STATIC_java_lang_Character_OTHER_PUNCTUATION;
static JAVA_BYTE _STATIC_java_lang_Character_MATH_SYMBOL;
static JAVA_BYTE _STATIC_java_lang_Character_CURRENCY_SYMBOL;
static JAVA_BYTE _STATIC_java_lang_Character_MODIFIER_SYMBOL;
static JAVA_BYTE _STATIC_java_lang_Character_OTHER_SYMBOL;
static JAVA_BYTE _STATIC_java_lang_Character_INITIAL_QUOTE_PUNCTUATION;
static JAVA_BYTE _STATIC_java_lang_Character_FINAL_QUOTE_PUNCTUATION;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_UNDEFINED;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_ARABIC_NUMBER;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_COMMON_NUMBER_SEPARATOR;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_NONSPACING_MARK;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_BOUNDARY_NEUTRAL;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_PARAGRAPH_SEPARATOR;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_SEGMENT_SEPARATOR;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_WHITESPACE;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_OTHER_NEUTRALS;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE;
static JAVA_BYTE _STATIC_java_lang_Character_DIRECTIONALITY_POP_DIRECTIONAL_FORMAT;
static JAVA_CHAR _STATIC_java_lang_Character_MIN_HIGH_SURROGATE;
static JAVA_CHAR _STATIC_java_lang_Character_MAX_HIGH_SURROGATE;
static JAVA_CHAR _STATIC_java_lang_Character_MIN_LOW_SURROGATE;
static JAVA_CHAR _STATIC_java_lang_Character_MAX_LOW_SURROGATE;
static JAVA_CHAR _STATIC_java_lang_Character_MIN_SURROGATE;
static JAVA_CHAR _STATIC_java_lang_Character_MAX_SURROGATE;
static JAVA_INT _STATIC_java_lang_Character_MIN_SUPPLEMENTARY_CODE_POINT;
static JAVA_INT _STATIC_java_lang_Character_MIN_CODE_POINT;
static JAVA_INT _STATIC_java_lang_Character_MAX_CODE_POINT;
static JAVA_INT _STATIC_java_lang_Character_SIZE;
static JAVA_OBJECT _STATIC_java_lang_Character_bidiKeys;
static JAVA_OBJECT _STATIC_java_lang_Character_bidiValues;
static JAVA_OBJECT _STATIC_java_lang_Character_mirrored;
static JAVA_OBJECT _STATIC_java_lang_Character_typeKeys;
static JAVA_OBJECT _STATIC_java_lang_Character_typeValues;
static JAVA_OBJECT _STATIC_java_lang_Character_typeValuesCache;
static JAVA_OBJECT _STATIC_java_lang_Character_uppercaseKeys;
static JAVA_OBJECT _STATIC_java_lang_Character_uppercaseValues;
static JAVA_OBJECT _STATIC_java_lang_Character_uppercaseValuesCache;
static JAVA_OBJECT _STATIC_java_lang_Character_lowercaseKeys;
static JAVA_OBJECT _STATIC_java_lang_Character_lowercaseValues;
static JAVA_OBJECT _STATIC_java_lang_Character_lowercaseValuesCache;
static JAVA_OBJECT _STATIC_java_lang_Character_digitKeys;
static JAVA_OBJECT _STATIC_java_lang_Character_digitValues;
static JAVA_OBJECT _STATIC_java_lang_Character_typeTags;
static JAVA_OBJECT _STATIC_java_lang_Character_DIRECTIONALITY;
static JAVA_INT _STATIC_java_lang_Character_ISJAVASTART;
static JAVA_INT _STATIC_java_lang_Character_ISJAVAPART;
static JAVA_OBJECT _STATIC_java_lang_Character_titlecaseKeys;
static JAVA_OBJECT _STATIC_java_lang_Character_titlecaseValues;
static JAVA_OBJECT _STATIC_java_lang_Character_numericKeys;
static JAVA_OBJECT _STATIC_java_lang_Character_numericValues;
static JAVA_INT _STATIC_java_lang_Character_CACHE_LEN;
static JAVA_BOOLEAN _STATIC_java_lang_Character__assertionsDisabled;

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

void __INIT_java_lang_Character()
{
    staticInitializerLock(&__TIB_java_lang_Character);

    // While the static initializer mutex is locked, locally store the value of
    // whether class initialization began or not
    int initBegan = __TIB_java_lang_Character.classInitializationBegan;

    // Whether or not class initialization had already began, it has begun now
    __TIB_java_lang_Character.classInitializationBegan = 1;

    staticInitializerUnlock(&__TIB_java_lang_Character);

    JAVA_LONG curThreadId = (JAVA_LONG)pthread_self();
    if (initBegan) {
        if (__TIB_java_lang_Character.initializerThreadId != curThreadId) {
            // Busy wait until the other thread finishes initializing this class
            while (!__TIB_java_lang_Character.classInitialized) {
                // do nothing
            }
        }
    } else {
        __TIB_java_lang_Character.initializerThreadId = curThreadId;
        XMLVM_CLASS_USED("java.lang.Character")
        __INIT_IMPL_java_lang_Character();
    }
}

void __INIT_IMPL_java_lang_Character()
{
    // Initialize base class if necessary
    XMLVM_CLASS_INIT(java_lang_Object)
    __TIB_java_lang_Character.newInstanceFunc = __NEW_INSTANCE_java_lang_Character;
    // Copy vtable from base class
    XMLVM_MEMCPY(__TIB_java_lang_Character.vtable, __TIB_java_lang_Object.vtable, sizeof(__TIB_java_lang_Object.vtable));
    // Initialize vtable for this class
    __TIB_java_lang_Character.vtable[1] = (VTABLE_PTR) &java_lang_Character_equals___java_lang_Object;
    __TIB_java_lang_Character.vtable[4] = (VTABLE_PTR) &java_lang_Character_hashCode__;
    __TIB_java_lang_Character.vtable[5] = (VTABLE_PTR) &java_lang_Character_toString__;
    __TIB_java_lang_Character.vtable[6] = (VTABLE_PTR) &java_lang_Character_compareTo___java_lang_Object;
    // Initialize interface information
    __TIB_java_lang_Character.numImplementedInterfaces = 2;
    __TIB_java_lang_Character.implementedInterfaces = (__TIB_DEFINITION_TEMPLATE* (*)[1]) XMLVM_MALLOC(sizeof(__TIB_DEFINITION_TEMPLATE*) * 2);

    // Initialize interfaces if necessary and assign tib to implementedInterfaces

    XMLVM_CLASS_INIT(java_io_Serializable)

    __TIB_java_lang_Character.implementedInterfaces[0][0] = &__TIB_java_io_Serializable;

    XMLVM_CLASS_INIT(java_lang_Comparable)

    __TIB_java_lang_Character.implementedInterfaces[0][1] = &__TIB_java_lang_Comparable;
    // Initialize itable for this class
    __TIB_java_lang_Character.itableBegin = &__TIB_java_lang_Character.itable[0];
    __TIB_java_lang_Character.itable[XMLVM_ITABLE_IDX_java_lang_Comparable_compareTo___java_lang_Object] = __TIB_java_lang_Character.vtable[6];

    _STATIC_java_lang_Character_serialVersionUID = 3786198910865385080;
    _STATIC_java_lang_Character_MIN_VALUE = 0;
    _STATIC_java_lang_Character_MAX_VALUE = 65535;
    _STATIC_java_lang_Character_MIN_RADIX = 2;
    _STATIC_java_lang_Character_MAX_RADIX = 36;
    _STATIC_java_lang_Character_TYPE = (java_lang_Class*) JAVA_NULL;
    _STATIC_java_lang_Character_UNASSIGNED = 0;
    _STATIC_java_lang_Character_UPPERCASE_LETTER = 1;
    _STATIC_java_lang_Character_LOWERCASE_LETTER = 2;
    _STATIC_java_lang_Character_TITLECASE_LETTER = 3;
    _STATIC_java_lang_Character_MODIFIER_LETTER = 4;
    _STATIC_java_lang_Character_OTHER_LETTER = 5;
    _STATIC_java_lang_Character_NON_SPACING_MARK = 6;
    _STATIC_java_lang_Character_ENCLOSING_MARK = 7;
    _STATIC_java_lang_Character_COMBINING_SPACING_MARK = 8;
    _STATIC_java_lang_Character_DECIMAL_DIGIT_NUMBER = 9;
    _STATIC_java_lang_Character_LETTER_NUMBER = 10;
    _STATIC_java_lang_Character_OTHER_NUMBER = 11;
    _STATIC_java_lang_Character_SPACE_SEPARATOR = 12;
    _STATIC_java_lang_Character_LINE_SEPARATOR = 13;
    _STATIC_java_lang_Character_PARAGRAPH_SEPARATOR = 14;
    _STATIC_java_lang_Character_CONTROL = 15;
    _STATIC_java_lang_Character_FORMAT = 16;
    _STATIC_java_lang_Character_PRIVATE_USE = 18;
    _STATIC_java_lang_Character_SURROGATE = 19;
    _STATIC_java_lang_Character_DASH_PUNCTUATION = 20;
    _STATIC_java_lang_Character_START_PUNCTUATION = 21;
    _STATIC_java_lang_Character_END_PUNCTUATION = 22;
    _STATIC_java_lang_Character_CONNECTOR_PUNCTUATION = 23;
    _STATIC_java_lang_Character_OTHER_PUNCTUATION = 24;
    _STATIC_java_lang_Character_MATH_SYMBOL = 25;
    _STATIC_java_lang_Character_CURRENCY_SYMBOL = 26;
    _STATIC_java_lang_Character_MODIFIER_SYMBOL = 27;
    _STATIC_java_lang_Character_OTHER_SYMBOL = 28;
    _STATIC_java_lang_Character_INITIAL_QUOTE_PUNCTUATION = 29;
    _STATIC_java_lang_Character_FINAL_QUOTE_PUNCTUATION = 30;
    _STATIC_java_lang_Character_DIRECTIONALITY_UNDEFINED = -1;
    _STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT = 0;
    _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT = 1;
    _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC = 2;
    _STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER = 3;
    _STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR = 4;
    _STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR = 5;
    _STATIC_java_lang_Character_DIRECTIONALITY_ARABIC_NUMBER = 6;
    _STATIC_java_lang_Character_DIRECTIONALITY_COMMON_NUMBER_SEPARATOR = 7;
    _STATIC_java_lang_Character_DIRECTIONALITY_NONSPACING_MARK = 8;
    _STATIC_java_lang_Character_DIRECTIONALITY_BOUNDARY_NEUTRAL = 9;
    _STATIC_java_lang_Character_DIRECTIONALITY_PARAGRAPH_SEPARATOR = 10;
    _STATIC_java_lang_Character_DIRECTIONALITY_SEGMENT_SEPARATOR = 11;
    _STATIC_java_lang_Character_DIRECTIONALITY_WHITESPACE = 12;
    _STATIC_java_lang_Character_DIRECTIONALITY_OTHER_NEUTRALS = 13;
    _STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING = 14;
    _STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE = 15;
    _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING = 16;
    _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE = 17;
    _STATIC_java_lang_Character_DIRECTIONALITY_POP_DIRECTIONAL_FORMAT = 18;
    _STATIC_java_lang_Character_MIN_HIGH_SURROGATE = 55296;
    _STATIC_java_lang_Character_MAX_HIGH_SURROGATE = 56319;
    _STATIC_java_lang_Character_MIN_LOW_SURROGATE = 56320;
    _STATIC_java_lang_Character_MAX_LOW_SURROGATE = 57343;
    _STATIC_java_lang_Character_MIN_SURROGATE = 55296;
    _STATIC_java_lang_Character_MAX_SURROGATE = 57343;
    _STATIC_java_lang_Character_MIN_SUPPLEMENTARY_CODE_POINT = 65536;
    _STATIC_java_lang_Character_MIN_CODE_POINT = 0;
    _STATIC_java_lang_Character_MAX_CODE_POINT = 1114111;
    _STATIC_java_lang_Character_SIZE = 16;
    _STATIC_java_lang_Character_bidiKeys = (java_lang_String*) xmlvm_create_java_string_from_pool(145);
    _STATIC_java_lang_Character_bidiValues = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_mirrored = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_typeKeys = (java_lang_String*) xmlvm_create_java_string_from_pool(146);
    _STATIC_java_lang_Character_typeValues = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_typeValuesCache = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_uppercaseKeys = (java_lang_String*) xmlvm_create_java_string_from_pool(147);
    _STATIC_java_lang_Character_uppercaseValues = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_uppercaseValuesCache = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_lowercaseKeys = (java_lang_String*) xmlvm_create_java_string_from_pool(148);
    _STATIC_java_lang_Character_lowercaseValues = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_lowercaseValuesCache = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_digitKeys = (java_lang_String*) xmlvm_create_java_string_from_pool(149);
    _STATIC_java_lang_Character_digitValues = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_typeTags = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_DIRECTIONALITY = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_ISJAVASTART = 1;
    _STATIC_java_lang_Character_ISJAVAPART = 2;
    _STATIC_java_lang_Character_titlecaseKeys = (java_lang_String*) xmlvm_create_java_string_from_pool(150);
    _STATIC_java_lang_Character_titlecaseValues = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_numericKeys = (java_lang_String*) xmlvm_create_java_string_from_pool(151);
    _STATIC_java_lang_Character_numericValues = (org_xmlvm_runtime_XMLVMArray*) JAVA_NULL;
    _STATIC_java_lang_Character_CACHE_LEN = 512;
    _STATIC_java_lang_Character__assertionsDisabled = 0;

    __TIB_java_lang_Character.declaredFields = &__field_reflection_data[0];
    __TIB_java_lang_Character.numDeclaredFields = sizeof(__field_reflection_data) / sizeof(XMLVM_FIELD_REFLECTION_DATA);
    __TIB_java_lang_Character.constructorDispatcherFunc = constructor_dispatcher;
    __TIB_java_lang_Character.declaredConstructors = &__constructor_reflection_data[0];
    __TIB_java_lang_Character.numDeclaredConstructors = sizeof(__constructor_reflection_data) / sizeof(XMLVM_CONSTRUCTOR_REFLECTION_DATA);
    __TIB_java_lang_Character.methodDispatcherFunc = method_dispatcher;
    __TIB_java_lang_Character.declaredMethods = &__method_reflection_data[0];
    __TIB_java_lang_Character.numDeclaredMethods = sizeof(__method_reflection_data) / sizeof(XMLVM_METHOD_REFLECTION_DATA);
    __CLASS_java_lang_Character = XMLVM_CREATE_CLASS_OBJECT(&__TIB_java_lang_Character);
    __TIB_java_lang_Character.clazz = __CLASS_java_lang_Character;
    __TIB_java_lang_Character.baseType = JAVA_NULL;
    __CLASS_java_lang_Character_1ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Character);
    __CLASS_java_lang_Character_2ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Character_1ARRAY);
    __CLASS_java_lang_Character_3ARRAY = XMLVM_CREATE_ARRAY_CLASS_OBJECT(__CLASS_java_lang_Character_2ARRAY);
    java_lang_Character___CLINIT_();
    //XMLVM_BEGIN_WRAPPER[__INIT_java_lang_Character]
    //XMLVM_END_WRAPPER

    __TIB_java_lang_Character.classInitialized = 1;
}

void __DELETE_java_lang_Character(void* me, void* client_data)
{
    //XMLVM_BEGIN_WRAPPER[__DELETE_java_lang_Character]
    //XMLVM_END_WRAPPER
}

void __INIT_INSTANCE_MEMBERS_java_lang_Character(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer)
{
    __INIT_INSTANCE_MEMBERS_java_lang_Object(me, 0 || derivedClassWillRegisterFinalizer);
    ((java_lang_Character*) me)->fields.java_lang_Character.value_ = 0;
    //XMLVM_BEGIN_WRAPPER[__INIT_INSTANCE_MEMBERS_java_lang_Character]
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT __NEW_java_lang_Character()
{    XMLVM_CLASS_INIT(java_lang_Character)
java_lang_Character* me = (java_lang_Character*) XMLVM_MALLOC(sizeof(java_lang_Character));
    me->tib = &__TIB_java_lang_Character;
    __INIT_INSTANCE_MEMBERS_java_lang_Character(me, 0);
    //XMLVM_BEGIN_WRAPPER[__NEW_java_lang_Character]
    //XMLVM_END_WRAPPER
    return me;
}

JAVA_OBJECT __NEW_INSTANCE_java_lang_Character()
{
    JAVA_OBJECT me = JAVA_NULL;
    return me;
}

JAVA_LONG java_lang_Character_GET_serialVersionUID()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_serialVersionUID;
}

void java_lang_Character_PUT_serialVersionUID(JAVA_LONG v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_serialVersionUID = v;
}

JAVA_CHAR java_lang_Character_GET_MIN_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MIN_VALUE;
}

void java_lang_Character_PUT_MIN_VALUE(JAVA_CHAR v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MIN_VALUE = v;
}

JAVA_CHAR java_lang_Character_GET_MAX_VALUE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MAX_VALUE;
}

void java_lang_Character_PUT_MAX_VALUE(JAVA_CHAR v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MAX_VALUE = v;
}

JAVA_INT java_lang_Character_GET_MIN_RADIX()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MIN_RADIX;
}

void java_lang_Character_PUT_MIN_RADIX(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MIN_RADIX = v;
}

JAVA_INT java_lang_Character_GET_MAX_RADIX()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MAX_RADIX;
}

void java_lang_Character_PUT_MAX_RADIX(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MAX_RADIX = v;
}

JAVA_OBJECT java_lang_Character_GET_TYPE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_TYPE;
}

void java_lang_Character_PUT_TYPE(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_TYPE = v;
}

JAVA_BYTE java_lang_Character_GET_UNASSIGNED()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_UNASSIGNED;
}

void java_lang_Character_PUT_UNASSIGNED(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_UNASSIGNED = v;
}

JAVA_BYTE java_lang_Character_GET_UPPERCASE_LETTER()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_UPPERCASE_LETTER;
}

void java_lang_Character_PUT_UPPERCASE_LETTER(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_UPPERCASE_LETTER = v;
}

JAVA_BYTE java_lang_Character_GET_LOWERCASE_LETTER()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_LOWERCASE_LETTER;
}

void java_lang_Character_PUT_LOWERCASE_LETTER(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_LOWERCASE_LETTER = v;
}

JAVA_BYTE java_lang_Character_GET_TITLECASE_LETTER()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_TITLECASE_LETTER;
}

void java_lang_Character_PUT_TITLECASE_LETTER(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_TITLECASE_LETTER = v;
}

JAVA_BYTE java_lang_Character_GET_MODIFIER_LETTER()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MODIFIER_LETTER;
}

void java_lang_Character_PUT_MODIFIER_LETTER(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MODIFIER_LETTER = v;
}

JAVA_BYTE java_lang_Character_GET_OTHER_LETTER()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_OTHER_LETTER;
}

void java_lang_Character_PUT_OTHER_LETTER(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_OTHER_LETTER = v;
}

JAVA_BYTE java_lang_Character_GET_NON_SPACING_MARK()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_NON_SPACING_MARK;
}

void java_lang_Character_PUT_NON_SPACING_MARK(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_NON_SPACING_MARK = v;
}

JAVA_BYTE java_lang_Character_GET_ENCLOSING_MARK()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_ENCLOSING_MARK;
}

void java_lang_Character_PUT_ENCLOSING_MARK(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_ENCLOSING_MARK = v;
}

JAVA_BYTE java_lang_Character_GET_COMBINING_SPACING_MARK()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_COMBINING_SPACING_MARK;
}

void java_lang_Character_PUT_COMBINING_SPACING_MARK(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_COMBINING_SPACING_MARK = v;
}

JAVA_BYTE java_lang_Character_GET_DECIMAL_DIGIT_NUMBER()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DECIMAL_DIGIT_NUMBER;
}

void java_lang_Character_PUT_DECIMAL_DIGIT_NUMBER(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DECIMAL_DIGIT_NUMBER = v;
}

JAVA_BYTE java_lang_Character_GET_LETTER_NUMBER()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_LETTER_NUMBER;
}

void java_lang_Character_PUT_LETTER_NUMBER(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_LETTER_NUMBER = v;
}

JAVA_BYTE java_lang_Character_GET_OTHER_NUMBER()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_OTHER_NUMBER;
}

void java_lang_Character_PUT_OTHER_NUMBER(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_OTHER_NUMBER = v;
}

JAVA_BYTE java_lang_Character_GET_SPACE_SEPARATOR()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_SPACE_SEPARATOR;
}

void java_lang_Character_PUT_SPACE_SEPARATOR(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_SPACE_SEPARATOR = v;
}

JAVA_BYTE java_lang_Character_GET_LINE_SEPARATOR()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_LINE_SEPARATOR;
}

void java_lang_Character_PUT_LINE_SEPARATOR(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_LINE_SEPARATOR = v;
}

JAVA_BYTE java_lang_Character_GET_PARAGRAPH_SEPARATOR()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_PARAGRAPH_SEPARATOR;
}

void java_lang_Character_PUT_PARAGRAPH_SEPARATOR(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_PARAGRAPH_SEPARATOR = v;
}

JAVA_BYTE java_lang_Character_GET_CONTROL()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_CONTROL;
}

void java_lang_Character_PUT_CONTROL(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_CONTROL = v;
}

JAVA_BYTE java_lang_Character_GET_FORMAT()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_FORMAT;
}

void java_lang_Character_PUT_FORMAT(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_FORMAT = v;
}

JAVA_BYTE java_lang_Character_GET_PRIVATE_USE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_PRIVATE_USE;
}

void java_lang_Character_PUT_PRIVATE_USE(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_PRIVATE_USE = v;
}

JAVA_BYTE java_lang_Character_GET_SURROGATE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_SURROGATE;
}

void java_lang_Character_PUT_SURROGATE(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_SURROGATE = v;
}

JAVA_BYTE java_lang_Character_GET_DASH_PUNCTUATION()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DASH_PUNCTUATION;
}

void java_lang_Character_PUT_DASH_PUNCTUATION(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DASH_PUNCTUATION = v;
}

JAVA_BYTE java_lang_Character_GET_START_PUNCTUATION()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_START_PUNCTUATION;
}

void java_lang_Character_PUT_START_PUNCTUATION(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_START_PUNCTUATION = v;
}

JAVA_BYTE java_lang_Character_GET_END_PUNCTUATION()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_END_PUNCTUATION;
}

void java_lang_Character_PUT_END_PUNCTUATION(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_END_PUNCTUATION = v;
}

JAVA_BYTE java_lang_Character_GET_CONNECTOR_PUNCTUATION()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_CONNECTOR_PUNCTUATION;
}

void java_lang_Character_PUT_CONNECTOR_PUNCTUATION(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_CONNECTOR_PUNCTUATION = v;
}

JAVA_BYTE java_lang_Character_GET_OTHER_PUNCTUATION()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_OTHER_PUNCTUATION;
}

void java_lang_Character_PUT_OTHER_PUNCTUATION(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_OTHER_PUNCTUATION = v;
}

JAVA_BYTE java_lang_Character_GET_MATH_SYMBOL()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MATH_SYMBOL;
}

void java_lang_Character_PUT_MATH_SYMBOL(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MATH_SYMBOL = v;
}

JAVA_BYTE java_lang_Character_GET_CURRENCY_SYMBOL()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_CURRENCY_SYMBOL;
}

void java_lang_Character_PUT_CURRENCY_SYMBOL(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_CURRENCY_SYMBOL = v;
}

JAVA_BYTE java_lang_Character_GET_MODIFIER_SYMBOL()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MODIFIER_SYMBOL;
}

void java_lang_Character_PUT_MODIFIER_SYMBOL(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MODIFIER_SYMBOL = v;
}

JAVA_BYTE java_lang_Character_GET_OTHER_SYMBOL()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_OTHER_SYMBOL;
}

void java_lang_Character_PUT_OTHER_SYMBOL(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_OTHER_SYMBOL = v;
}

JAVA_BYTE java_lang_Character_GET_INITIAL_QUOTE_PUNCTUATION()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_INITIAL_QUOTE_PUNCTUATION;
}

void java_lang_Character_PUT_INITIAL_QUOTE_PUNCTUATION(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_INITIAL_QUOTE_PUNCTUATION = v;
}

JAVA_BYTE java_lang_Character_GET_FINAL_QUOTE_PUNCTUATION()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_FINAL_QUOTE_PUNCTUATION;
}

void java_lang_Character_PUT_FINAL_QUOTE_PUNCTUATION(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_FINAL_QUOTE_PUNCTUATION = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_UNDEFINED()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_UNDEFINED;
}

void java_lang_Character_PUT_DIRECTIONALITY_UNDEFINED(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_UNDEFINED = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_LEFT_TO_RIGHT()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT;
}

void java_lang_Character_PUT_DIRECTIONALITY_LEFT_TO_RIGHT(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_RIGHT_TO_LEFT()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT;
}

void java_lang_Character_PUT_DIRECTIONALITY_RIGHT_TO_LEFT(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC;
}

void java_lang_Character_PUT_DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_ARABIC = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_EUROPEAN_NUMBER()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER;
}

void java_lang_Character_PUT_DIRECTIONALITY_EUROPEAN_NUMBER(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR;
}

void java_lang_Character_PUT_DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER_SEPARATOR = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR;
}

void java_lang_Character_PUT_DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_EUROPEAN_NUMBER_TERMINATOR = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_ARABIC_NUMBER()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_ARABIC_NUMBER;
}

void java_lang_Character_PUT_DIRECTIONALITY_ARABIC_NUMBER(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_ARABIC_NUMBER = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_COMMON_NUMBER_SEPARATOR()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_COMMON_NUMBER_SEPARATOR;
}

void java_lang_Character_PUT_DIRECTIONALITY_COMMON_NUMBER_SEPARATOR(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_COMMON_NUMBER_SEPARATOR = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_NONSPACING_MARK()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_NONSPACING_MARK;
}

void java_lang_Character_PUT_DIRECTIONALITY_NONSPACING_MARK(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_NONSPACING_MARK = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_BOUNDARY_NEUTRAL()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_BOUNDARY_NEUTRAL;
}

void java_lang_Character_PUT_DIRECTIONALITY_BOUNDARY_NEUTRAL(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_BOUNDARY_NEUTRAL = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_PARAGRAPH_SEPARATOR()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_PARAGRAPH_SEPARATOR;
}

void java_lang_Character_PUT_DIRECTIONALITY_PARAGRAPH_SEPARATOR(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_PARAGRAPH_SEPARATOR = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_SEGMENT_SEPARATOR()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_SEGMENT_SEPARATOR;
}

void java_lang_Character_PUT_DIRECTIONALITY_SEGMENT_SEPARATOR(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_SEGMENT_SEPARATOR = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_WHITESPACE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_WHITESPACE;
}

void java_lang_Character_PUT_DIRECTIONALITY_WHITESPACE(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_WHITESPACE = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_OTHER_NEUTRALS()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_OTHER_NEUTRALS;
}

void java_lang_Character_PUT_DIRECTIONALITY_OTHER_NEUTRALS(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_OTHER_NEUTRALS = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING;
}

void java_lang_Character_PUT_DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT_EMBEDDING = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE;
}

void java_lang_Character_PUT_DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_LEFT_TO_RIGHT_OVERRIDE = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING;
}

void java_lang_Character_PUT_DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_EMBEDDING = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE;
}

void java_lang_Character_PUT_DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_RIGHT_TO_LEFT_OVERRIDE = v;
}

JAVA_BYTE java_lang_Character_GET_DIRECTIONALITY_POP_DIRECTIONAL_FORMAT()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY_POP_DIRECTIONAL_FORMAT;
}

void java_lang_Character_PUT_DIRECTIONALITY_POP_DIRECTIONAL_FORMAT(JAVA_BYTE v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY_POP_DIRECTIONAL_FORMAT = v;
}

JAVA_CHAR java_lang_Character_GET_MIN_HIGH_SURROGATE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MIN_HIGH_SURROGATE;
}

void java_lang_Character_PUT_MIN_HIGH_SURROGATE(JAVA_CHAR v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MIN_HIGH_SURROGATE = v;
}

JAVA_CHAR java_lang_Character_GET_MAX_HIGH_SURROGATE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MAX_HIGH_SURROGATE;
}

void java_lang_Character_PUT_MAX_HIGH_SURROGATE(JAVA_CHAR v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MAX_HIGH_SURROGATE = v;
}

JAVA_CHAR java_lang_Character_GET_MIN_LOW_SURROGATE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MIN_LOW_SURROGATE;
}

void java_lang_Character_PUT_MIN_LOW_SURROGATE(JAVA_CHAR v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MIN_LOW_SURROGATE = v;
}

JAVA_CHAR java_lang_Character_GET_MAX_LOW_SURROGATE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MAX_LOW_SURROGATE;
}

void java_lang_Character_PUT_MAX_LOW_SURROGATE(JAVA_CHAR v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MAX_LOW_SURROGATE = v;
}

JAVA_CHAR java_lang_Character_GET_MIN_SURROGATE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MIN_SURROGATE;
}

void java_lang_Character_PUT_MIN_SURROGATE(JAVA_CHAR v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MIN_SURROGATE = v;
}

JAVA_CHAR java_lang_Character_GET_MAX_SURROGATE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MAX_SURROGATE;
}

void java_lang_Character_PUT_MAX_SURROGATE(JAVA_CHAR v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MAX_SURROGATE = v;
}

JAVA_INT java_lang_Character_GET_MIN_SUPPLEMENTARY_CODE_POINT()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MIN_SUPPLEMENTARY_CODE_POINT;
}

void java_lang_Character_PUT_MIN_SUPPLEMENTARY_CODE_POINT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MIN_SUPPLEMENTARY_CODE_POINT = v;
}

JAVA_INT java_lang_Character_GET_MIN_CODE_POINT()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MIN_CODE_POINT;
}

void java_lang_Character_PUT_MIN_CODE_POINT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MIN_CODE_POINT = v;
}

JAVA_INT java_lang_Character_GET_MAX_CODE_POINT()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_MAX_CODE_POINT;
}

void java_lang_Character_PUT_MAX_CODE_POINT(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_MAX_CODE_POINT = v;
}

JAVA_INT java_lang_Character_GET_SIZE()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_SIZE;
}

void java_lang_Character_PUT_SIZE(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_SIZE = v;
}

JAVA_OBJECT java_lang_Character_GET_bidiKeys()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_bidiKeys;
}

void java_lang_Character_PUT_bidiKeys(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_bidiKeys = v;
}

JAVA_OBJECT java_lang_Character_GET_bidiValues()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_bidiValues;
}

void java_lang_Character_PUT_bidiValues(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_bidiValues = v;
}

JAVA_OBJECT java_lang_Character_GET_mirrored()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_mirrored;
}

void java_lang_Character_PUT_mirrored(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_mirrored = v;
}

JAVA_OBJECT java_lang_Character_GET_typeKeys()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_typeKeys;
}

void java_lang_Character_PUT_typeKeys(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_typeKeys = v;
}

JAVA_OBJECT java_lang_Character_GET_typeValues()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_typeValues;
}

void java_lang_Character_PUT_typeValues(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_typeValues = v;
}

JAVA_OBJECT java_lang_Character_GET_typeValuesCache()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_typeValuesCache;
}

void java_lang_Character_PUT_typeValuesCache(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_typeValuesCache = v;
}

JAVA_OBJECT java_lang_Character_GET_uppercaseKeys()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_uppercaseKeys;
}

void java_lang_Character_PUT_uppercaseKeys(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_uppercaseKeys = v;
}

JAVA_OBJECT java_lang_Character_GET_uppercaseValues()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_uppercaseValues;
}

void java_lang_Character_PUT_uppercaseValues(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_uppercaseValues = v;
}

JAVA_OBJECT java_lang_Character_GET_uppercaseValuesCache()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_uppercaseValuesCache;
}

void java_lang_Character_PUT_uppercaseValuesCache(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_uppercaseValuesCache = v;
}

JAVA_OBJECT java_lang_Character_GET_lowercaseKeys()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_lowercaseKeys;
}

void java_lang_Character_PUT_lowercaseKeys(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_lowercaseKeys = v;
}

JAVA_OBJECT java_lang_Character_GET_lowercaseValues()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_lowercaseValues;
}

void java_lang_Character_PUT_lowercaseValues(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_lowercaseValues = v;
}

JAVA_OBJECT java_lang_Character_GET_lowercaseValuesCache()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_lowercaseValuesCache;
}

void java_lang_Character_PUT_lowercaseValuesCache(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_lowercaseValuesCache = v;
}

JAVA_OBJECT java_lang_Character_GET_digitKeys()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_digitKeys;
}

void java_lang_Character_PUT_digitKeys(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_digitKeys = v;
}

JAVA_OBJECT java_lang_Character_GET_digitValues()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_digitValues;
}

void java_lang_Character_PUT_digitValues(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_digitValues = v;
}

JAVA_OBJECT java_lang_Character_GET_typeTags()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_typeTags;
}

void java_lang_Character_PUT_typeTags(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_typeTags = v;
}

JAVA_OBJECT java_lang_Character_GET_DIRECTIONALITY()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_DIRECTIONALITY;
}

void java_lang_Character_PUT_DIRECTIONALITY(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_DIRECTIONALITY = v;
}

JAVA_INT java_lang_Character_GET_ISJAVASTART()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_ISJAVASTART;
}

void java_lang_Character_PUT_ISJAVASTART(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_ISJAVASTART = v;
}

JAVA_INT java_lang_Character_GET_ISJAVAPART()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_ISJAVAPART;
}

void java_lang_Character_PUT_ISJAVAPART(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_ISJAVAPART = v;
}

JAVA_OBJECT java_lang_Character_GET_titlecaseKeys()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_titlecaseKeys;
}

void java_lang_Character_PUT_titlecaseKeys(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_titlecaseKeys = v;
}

JAVA_OBJECT java_lang_Character_GET_titlecaseValues()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_titlecaseValues;
}

void java_lang_Character_PUT_titlecaseValues(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_titlecaseValues = v;
}

JAVA_OBJECT java_lang_Character_GET_numericKeys()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_numericKeys;
}

void java_lang_Character_PUT_numericKeys(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_numericKeys = v;
}

JAVA_OBJECT java_lang_Character_GET_numericValues()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_numericValues;
}

void java_lang_Character_PUT_numericValues(JAVA_OBJECT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_numericValues = v;
}

JAVA_INT java_lang_Character_GET_CACHE_LEN()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character_CACHE_LEN;
}

void java_lang_Character_PUT_CACHE_LEN(JAVA_INT v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character_CACHE_LEN = v;
}

JAVA_BOOLEAN java_lang_Character_GET__assertionsDisabled()
{
    XMLVM_CLASS_INIT(java_lang_Character)
    return _STATIC_java_lang_Character__assertionsDisabled;
}

void java_lang_Character_PUT__assertionsDisabled(JAVA_BOOLEAN v)
{
    XMLVM_CLASS_INIT(java_lang_Character)
_STATIC_java_lang_Character__assertionsDisabled = v;
}

void java_lang_Character___CLINIT_()
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Character___CLINIT___]
    XMLVM_ENTER_METHOD("java.lang.Character", "<clinit>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    _r7.i = 27;
    _r6.i = 15;
    _r5.i = 6;
    _r4.i = 1;
    _r3.i = 2;
    XMLVM_SOURCE_POSITION("Character.java", 52)
    XMLVM_CLASS_INIT(java_lang_Character)
    _r0.o = __CLASS_java_lang_Character;
    XMLVM_CHECK_NPE(0)
    _r0.i = java_lang_Class_desiredAssertionStatus__(_r0.o);
    if (_r0.i != 0) goto label4189;
    _r0 = _r4;
    label16:;
    java_lang_Character_PUT__assertionsDisabled( _r0.i);
    XMLVM_SOURCE_POSITION("Character.java", 81)
    _r0.i = 0;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r0.i);
    XMLVM_SOURCE_POSITION("Character.java", 82)
    //java_lang_Object_getClass__[3]
    XMLVM_CHECK_NPE(0)
    _r0.o = (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_lang_Object*) _r0.o)->tib->vtable[3])(_r0.o);
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_Class_getComponentType__(_r0.o);
    java_lang_Character_PUT_TYPE( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 459)
    // "\010\012\013\6013\015\5415\033\012\036\013 \6015\042\016%\006*\016.\30100\24049\004;\7010@\016Z\001`\016z\001\176\016\204\012\206\5412\237\012\241\7010\245\006\251\016\253\7001\257\016\261\006\263\004\266\416\270\016\272\2001\277\016\326\001\330\7001\366\001\370\7001\1037\001\1063\001\1255\001\1270\001\1272\016\1301\001\1317\016\1321\001\1337\016\1344\001\1355\016\1356\001\1516\011\1542\011\1565\016\1572\001\1576\016\1605\016\1610\7001\1612\001\1614\001\1641\001\1716\001\1727\001\1763\001\2202\001\2206\011\2211\011\2304\001\2310\001\2314\001\2365\001\2371\001\2526\001\2537\001\2607\001\2612\416\2641\011\2671\011\2675\011\2701\4402\2704\1011\2752\002\2764\002\3014\010\3033\1400\3037\1400\3072\003\3112\003\3125\011\3151\007\3153\3406\3155\1407\3161\1411\3325\003\3344\011\3346\003\3350\011\3352\7011\3355\011\3371\004\3376\003\3415\003\3420\5003\3422\4403\3454\003\3512\011\3645\003\3660\011\4402\011\4403\400\4471\001\4475\411\4500\001\4510\011\4514\001\4515\4400\4521\4401\4524\011\4541\001\4543\011\4560\001\4602\4401\4603\400\4614\001\4620\001\4650\001\4660\001\4662\001\4671\001\4674\011\4700\001\4704\011\4710\001\4714\001\4715\4400\4727\400\4735\001\4741\001\4743\011\4761\001\4763\006\4772\001\5002\011\5012\001\5020\001\5050\001\5060\001\5063\001\5066\001\5071\001\5074\011\5100\001\5102\011\5110\011\5115\011\5134\001\5136\001\5157\001\5161\011\5164\001\5202\011\5203\400\5213\001\5215\400\5221\001\5250\001\5260\001\5263\001\5271\001\5275\411\5300\001\5305\011\5310\011\5311\400\5314\001\5315\4400\5320\001\5340\001\5357\001\5402\4401\5403\400\5414\001\5420\001\5450\001\5460\001\5463\001\5471\001\5475\411\5501\4401\5503\011\5510\001\5514\001\5515\4400\5527\411\5535\001\5541\001\5560\001\5603\411\5612\001\5620\001\5625\001\5632\001\5634\001\5637\001\5644\001\5652\001\5665\001\5671\001\5677\001\5701\411\5702\001\5710\001\5714\001\5715\4400\5727\400\5762\001\6003\001\6014\001\6020\001\6050\001\6063\001\6071\001\6100\011\6104\001\6110\011\6115\011\6126\011\6141\001\6157\001\6203\001\6214\001\6220\001\6250\001\6263\001\6271\001\6300\4401\6304\001\6307\411\6310\001\6313\001\6315\011\6326\001\6336\001\6341\001\6357\001\6403\001\6414\001\6420\001\6450\001\6471\001\6500\001\6503\011\6510\001\6514\001\6515\4400\6527\400\6541\001\6557\001\6603\001\6626\001\6661\001\6673\001\6675\400\6706\001\6712\011\6721\001\6724\011\6726\011\6737\001\6764\001\7060\001\7062\4401\7064\411\7072\011\7100\3001\7106\001\7116\011\7133\001\7202\001\7204\001\7210\001\7212\001\7215\400\7227\001\7237\001\7243\001\7245\400\7247\400\7253\001\7260\001\7262\4401\7264\411\7271\011\7274\011\7275\400\7304\001\7306\001\7315\011\7331\001\7335\001\7427\001\7431\011\7464\001\7471\4401\7475\016\7507\001\7552\001\7576\011\7600\411\7604\011\7606\411\7610\4401\7613\001\7627\011\7674\011\7705\001\7707\411\7714\001\7717\400\10041\001\10047\001\10052\001\10055\4401\10060\011\10062\411\10067\011\10071\4401\10127\001\10131\011\10305\001\10366\001\10373\400\10531\001\10642\001\10771\001\11006\001\11106\001\11110\001\11115\001\11126\001\11130\001\11135\001\11206\001\11210\001\11215\001\11256\001\11260\001\11265\001\11276\001\11300\001\11305\001\11316\001\11326\001\11356\001\11416\001\11420\001\11425\001\11436\001\11506\001\11532\001\11574\001\11764\001\13166\001\13201\415\13232\001\13234\016\13360\001\13666\001\13675\011\13705\001\13707\411\13711\4401\13723\011\13732\001\13734\3001\13751\001\14012\016\14016\012\14031\001\14167\001\14250\001\14251\4400\17233\001\17371\001\17425\001\17435\001\17505\001\17515\001\17527\001\17531\400\17533\400\17535\400\17575\001\17664\001\17674\001\17677\7001\17701\016\17704\001\17714\001\17717\016\17723\001\17733\001\17737\016\17754\001\17757\016\17764\001\17774\001\17776\016\20012\015\20015\012\20017\1001\20047\016\20051\5415\20053\10417\20055\10023\20057\6422\20064\006\20106\016\20115\016\20157\012\20160\004\20171\004\20173\006\20176\016\20200\404\20211\004\20213\006\20216\016\20257\006\20343\011\20401\016\20403\7001\20406\016\20410\416\20412\7001\20423\001\20426\416\20430\016\20435\001\20443\016\20452\7001\20455\001\20457\406\20461\001\20463\416\20471\001\20472\016\20537\016\20603\001\20763\016\21021\016\21023\006\21361\016\21465\016\21572\001\21573\7000\21624\016\21626\416\21632\016\22046\016\22112\016\22233\004\22351\001\22352\004\22625\016\22767\016\23023\016\23161\016\23404\016\23411\016\23447\016\23513\016\23515\7000\23522\016\23526\016\23536\016\23547\016\23624\016\23657\016\23676\016\24377\016\27231\016\27363\016\27725\016\27773\016\30001\7015\30004\016\30007\001\30040\016\30051\001\30057\011\30061\416\30065\001\30067\016\30072\001\30077\016\30224\001\30232\011\30234\016\30236\001\30372\001\30374\7001\30376\001\30454\001\30616\001\30667\001\31034\001\31103\001\31173\001\31260\001\31313\001\31376\001\31566\001\31735\001\31776\001\46665\001\117645\001\122214\001\122241\016\122263\016\122300\016\122304\016\122306\016\153643\001\175055\001\175406\001\175427\001\175437\1011\175450\002\175452\3002\175466\002\175474\002\175476\002\175501\002\175504\002\175517\002\175661\003\176475\003\176477\016\176617\003\176707\003\176773\003\177043\011\177104\016\177117\016\177122\7010\177126\4016\177136\016\177140\3016\177142\7006\177144\3016\177146\016\177151\3016\177153\7006\177162\003\177164\003\177374\003\177377\5000\177402\016\177405\006\177412\016\177416\3010\177420\2404\177431\004\177433\7010\177440\016\177472\001\177500\016\177532\001\177536\016\177545\016\177676\001\177707\001\177717\001\177727\001\177734\001\177741\006\177744\016\177746\006\177756\016\177773\012\177775\016"
    _r0.o = xmlvm_create_java_string_from_pool(152);
    XMLVM_SOURCE_POSITION("Character.java", 460)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_getValue__(_r0.o);
    java_lang_Character_PUT_bidiValues( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 462)
    // "\000\000\1400\50000\000\24000\000\24000\000\000\4000\4000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\3000`\000\000\60000\60000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\37436\136142\174127\175017\17777\100074\147765\177777\117777\407\177714\140777\37000\177703\37777\003\7400\000\3003\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\177400\7763"
    _r0.o = xmlvm_create_java_string_from_pool(153);
    XMLVM_SOURCE_POSITION("Character.java", 463)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_getValue__(_r0.o);
    java_lang_Character_PUT_mirrored( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 468)
    // "\037\017!\14014#\030%\14032'\030)\13025,\14430.\120300\140119\011;\030>\031@\030Z\001\134\12430^\13033`\13433z\002\174\12431\176\13031\237\017\241\14014\245\032\247\034\251\16033\253\16402\255\12031\260\15434\262\14413\264\5433\266\1034\270\14033\272\5402\274\17013\276\013\300\14001\326\001\330\14401\336\001\366\002\370\14402\377\002\467\1001\510\402\570\1001\576\402\600\002\602\001\606\1001\611\402\613\001\615\002\621\001\623\402\626\1001\630\001\633\002\635\001\637\402\646\1001\652\402\656\1001\661\402\663\001\667\402\671\1001\673\2402\675\1001\677\002\703\005\705\1401\707\402\711\1003\713\1401\734\402\757\1001\761\402\763\1003\766\1001\770\001\1037\1001\1063\1001\1255\002\1270\004\1272\033\1301\004\1317\033\1321\004\1337\033\1344\004\1355\033\1356\004\1516\006\1542\006\1565\033\1572\004\1576\030\1605\033\1610\14001\1612\001\1614\001\1617\001\1621\402\1641\001\1653\001\1716\002\1721\002\1724\001\1727\002\1757\1001\1763\002\2057\001\2137\002\2201\1001\2203\3034\2206\006\2211\007\2300\1001\2304\402\2310\402\2314\402\2365\1001\2371\1001\2526\001\2532\2030\2537\030\2607\002\2612\14024\2641\006\2671\006\2675\006\2701\3030\2704\14006\2752\005\2762\005\2764\030\3014\030\3033\14000\3037\14000\3072\005\3101\2404\3112\005\3125\006\3151\011\3155\030\3161\2406\3323\005\3325\2430\3334\006\3336\007\3344\006\3346\004\3350\006\3352\16006\3355\006\3371\011\3374\005\3376\034\3415\030\3420\10005\3422\3005\3454\005\3512\006\3645\005\3660\006\4402\006\4403\4000\4471\005\4475\2406\4500\010\4510\006\4514\010\4515\3000\4521\3005\4524\006\4541\005\4543\006\4545\030\4557\011\4560\030\4602\3010\4603\4000\4614\005\4620\005\4650\005\4660\005\4662\005\4671\005\4674\006\4700\010\4704\006\4710\010\4714\010\4715\3000\4727\4000\4735\005\4741\005\4743\006\4757\011\4761\005\4763\032\4771\013\4772\034\5002\006\5012\005\5020\005\5050\005\5060\005\5063\005\5066\005\5071\005\5074\006\5100\010\5102\006\5110\006\5115\006\5134\005\5136\005\5157\011\5161\006\5164\005\5202\006\5203\4000\5213\005\5215\2400\5221\005\5250\005\5260\005\5263\005\5271\005\5275\2406\5300\010\5305\006\5310\006\5311\4000\5314\010\5315\3000\5320\005\5340\005\5357\011\5402\3010\5403\4000\5414\005\5420\005\5450\005\5460\005\5463\005\5471\005\5475\2406\5501\3010\5503\006\5510\010\5514\010\5515\3000\5527\4006\5535\005\5541\005\5557\011\5560\034\5603\4006\5612\005\5620\005\5625\005\5632\005\5634\005\5637\005\5644\005\5652\005\5665\005\5671\005\5677\010\5701\4006\5702\010\5710\010\5714\010\5715\3000\5727\4000\5757\011\5762\013\6003\010\6014\005\6020\005\6050\005\6063\005\6071\005\6100\006\6104\010\6110\006\6115\006\6126\006\6141\005\6157\011\6203\010\6214\005\6220\005\6250\005\6263\005\6271\005\6300\3010\6304\010\6307\4006\6310\010\6313\010\6315\006\6326\010\6336\005\6341\005\6357\011\6403\010\6414\005\6420\005\6450\005\6471\005\6500\010\6503\006\6510\010\6514\010\6515\3000\6527\4000\6541\005\6557\011\6603\010\6626\005\6661\005\6673\005\6675\2400\6706\005\6712\006\6721\010\6724\006\6726\006\6737\010\6763\010\6764\030\7060\005\7062\3005\7064\2406\7072\006\7100\15005\7105\005\7107\3004\7116\006\7120\14011\7131\011\7133\030\7202\005\7204\005\7210\005\7212\005\7215\2400\7227\005\7237\005\7243\005\7245\2400\7247\2400\7253\005\7260\005\7262\3005\7264\2406\7271\006\7274\006\7275\2400\7304\005\7306\004\7315\006\7331\011\7335\005\7401\16005\7403\034\7422\030\7427\034\7431\006\7437\034\7451\011\7463\013\7471\3034\7475\13025\7477\010\7507\005\7552\005\7576\006\7600\4006\7604\006\7606\14006\7610\3005\7613\005\7627\006\7674\006\7705\034\7707\16006\7714\034\7717\16000\10041\005\10047\005\10052\005\10055\3010\10060\006\10062\4006\10067\006\10071\3010\10111\011\10117\030\10125\005\10127\010\10131\006\10305\001\10366\005\10373\14000\10531\005\10642\005\10771\005\11006\005\11106\005\11110\005\11115\005\11126\005\11130\005\11135\005\11206\005\11210\005\11215\005\11256\005\11260\005\11265\005\11276\005\11300\005\11305\005\11316\005\11326\005\11356\005\11416\005\11420\005\11425\005\11436\005\11506\005\11532\005\11550\030\11561\011\11574\013\11764\005\13154\005\13156\030\13166\005\13201\2414\13232\005\13234\12426\13352\005\13355\030\13360\013\13663\005\13666\010\13675\006\13705\010\13707\4006\13711\3010\13723\006\13732\030\13734\15030\13751\011\14005\030\14007\14024\14012\030\14016\020\14031\011\14102\005\14104\2005\14167\005\14250\005\14251\3000\17225\1001\17233\002\17371\1001\17407\002\17417\001\17425\002\17435\001\17447\002\17457\001\17467\002\17477\001\17505\002\17515\001\17527\002\17531\400\17533\400\17535\400\17540\402\17547\002\17557\001\17575\002\17607\002\17617\003\17627\002\17637\003\17647\002\17657\003\17664\002\17667\002\17673\001\17675\15403\17677\15402\17701\033\17704\002\17707\002\17713\001\17715\15403\17717\033\17723\002\17727\002\17733\001\17737\033\17747\002\17754\001\17757\033\17764\002\17767\002\17773\001\17775\15403\17776\033\20013\014\20017\020\20025\024\20027\030\20031\17035\20033\16425\20035\17035\20037\16425\20047\030\20051\7015\20056\020\20060\6030\20070\030\20072\16436\20076\030\20100\027\20103\030\20105\12431\20106\026\20115\030\20157\020\20160\013\20171\013\20174\031\20176\12426\20200\1013\20211\013\20214\031\20216\12426\20257\032\20334\006\20340\007\20342\3007\20343\3400\20401\034\20403\16001\20406\034\20410\434\20412\16002\20415\001\20417\002\20422\001\20424\1034\20426\434\20430\034\20435\001\20443\034\20452\16001\20455\001\20457\1034\20461\001\20463\434\20465\2402\20470\005\20472\1034\20537\013\20603\012\20624\031\20631\034\20633\031\20637\034\20641\16031\20644\14434\20647\16031\20655\034\20657\16031\20715\034\20717\031\20721\034\20725\16031\20763\034\21361\031\21407\034\21413\031\21437\034\21441\031\21450\034\21452\12426\21573\034\21632\034\22046\034\22112\034\22233\013\22351\034\22352\013\22625\034\22666\034\22670\14434\22700\034\22702\14434\22767\034\23023\034\23156\034\23160\14434\23161\16000\23404\034\23411\034\23447\034\23513\034\23515\16000\23522\034\23526\034\23536\034\23547\034\23623\013\23624\034\23657\034\23676\034\24377\034\27231\034\27363\034\27725\034\27773\034\30001\14014\30003\030\30005\2034\30007\5005\30021\13025\30023\034\30033\13025\30035\12424\30037\026\30041\5034\30051\012\30057\006\30061\2024\30065\004\30067\034\30072\012\30077\034\30224\005\30232\006\30234\033\30236\004\30372\005\30374\13404\30376\004\30454\005\30616\005\30621\034\30625\013\30637\034\30667\005\31034\034\31051\013\31103\034\31173\034\31200\16013\31211\013\31260\034\31313\034\31376\034\31566\034\31735\034\31776\034\46665\005\117645\005\122214\005\122241\034\122263\034\122300\034\122304\034\122306\034\153643\005\157777\023\174377\022\175055\005\175406\002\175427\002\175437\2406\175450\005\175452\14405\175466\005\175474\005\175476\005\175501\005\175504\005\175661\005\176475\005\176477\13025\176617\005\176707\005\176773\005\177043\006\177061\12030\177063\13424\177065\12427\177104\12426\177114\030\177117\027\177122\030\177127\030\177131\12424\177136\12426\177141\030\177144\12031\177146\031\177152\15030\177153\14000\177162\005\177164\005\177374\005\177377\10000\177403\030\177405\14032\177407\030\177411\13025\177414\14430\177416\12030\177420\14011\177431\011\177433\030\177436\031\177440\030\177472\001\177474\12430\177476\13033\177500\13433\177532\002\177534\12431\177536\13031\177542\14025\177544\13030\177546\13405\177557\005\177561\2404\177635\005\177637\004\177676\005\177707\005\177717\005\177727\005\177734\005\177741\032\177743\15431\177745\15034\177746\032\177751\14434\177754\031\177756\034\177773\020\177775\034"
    _r0.o = xmlvm_create_java_string_from_pool(154);
    XMLVM_SOURCE_POSITION("Character.java", 469)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_getValue__(_r0.o);
    java_lang_Character_PUT_typeValues( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 471)
    _r0.i = 1000;
    XMLVM_CLASS_INIT(int)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_int, _r0.i);
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Character.java", 472)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r4.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r6.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r6.i;
    _r1.i = 3;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 5;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r5.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r6.i;
    _r1.i = 7;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 8;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 9;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 10;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 11;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 12;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 13;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 14;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r6.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r6.i;
    _r1.i = 16;
    XMLVM_SOURCE_POSITION("Character.java", 473)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 17;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 18;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 19;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 20;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 21;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 22;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 23;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 25;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 26;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r7.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r7.i] = _r6.i;
    _r1.i = 28;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 29;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 30;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 31;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 32;
    _r2.i = 12;
    XMLVM_SOURCE_POSITION("Character.java", 474)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 33;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 34;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 35;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 36;
    _r2.i = 26;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 37;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 38;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 39;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 40;
    _r2.i = 21;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 41;
    _r2.i = 22;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 42;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 43;
    _r2.i = 25;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 44;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 45;
    _r2.i = 20;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 46;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 47;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 48;
    _r2.i = 9;
    XMLVM_SOURCE_POSITION("Character.java", 475)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 49;
    _r2.i = 9;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 50;
    _r2.i = 9;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 51;
    _r2.i = 9;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 52;
    _r2.i = 9;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 53;
    _r2.i = 9;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 54;
    _r2.i = 9;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 55;
    _r2.i = 9;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 56;
    _r2.i = 9;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 57;
    _r2.i = 9;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 58;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 59;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 60;
    _r2.i = 25;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 61;
    _r2.i = 25;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 62;
    _r2.i = 25;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 63;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 64;
    _r2.i = 24;
    XMLVM_SOURCE_POSITION("Character.java", 476)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 65;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 66;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 67;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 68;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 69;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 70;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 71;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 72;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 73;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 74;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 75;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 76;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 77;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 78;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 79;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 80;
    XMLVM_SOURCE_POSITION("Character.java", 477)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 81;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 82;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 83;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 84;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 85;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 86;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 87;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 88;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 89;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 90;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 91;
    _r2.i = 21;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 92;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 93;
    _r2.i = 22;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 94;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 95;
    _r2.i = 23;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 96;
    XMLVM_SOURCE_POSITION("Character.java", 478)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 97;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 98;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 99;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 100;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 101;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 102;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 103;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 104;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 105;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 106;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 107;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 108;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 109;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 110;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 111;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 112;
    XMLVM_SOURCE_POSITION("Character.java", 479)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 113;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 114;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 115;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 116;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 117;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 118;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 119;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 120;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 121;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 122;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 123;
    _r2.i = 21;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 124;
    _r2.i = 25;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 125;
    _r2.i = 22;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 126;
    _r2.i = 25;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 127;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 128;
    XMLVM_SOURCE_POSITION("Character.java", 480)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 129;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 130;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 131;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 132;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 133;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 134;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 135;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 136;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 137;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 138;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 139;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 140;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 141;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 142;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 143;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 144;
    XMLVM_SOURCE_POSITION("Character.java", 481)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 145;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 146;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 147;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 148;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 149;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 150;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 151;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 152;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 153;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 154;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 155;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 156;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 157;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 158;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 159;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 160;
    _r2.i = 12;
    XMLVM_SOURCE_POSITION("Character.java", 482)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 161;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 162;
    _r2.i = 26;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 163;
    _r2.i = 26;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 164;
    _r2.i = 26;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 165;
    _r2.i = 26;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 166;
    _r2.i = 28;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 167;
    _r2.i = 28;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 168;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 169;
    _r2.i = 28;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 170;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 171;
    _r2.i = 29;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 172;
    _r2.i = 25;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 173;
    _r2.i = 16;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 174;
    _r2.i = 28;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 175;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 176;
    _r2.i = 28;
    XMLVM_SOURCE_POSITION("Character.java", 483)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 177;
    _r2.i = 25;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 178;
    _r2.i = 11;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 179;
    _r2.i = 11;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 180;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 181;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 182;
    _r2.i = 28;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 183;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 184;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 185;
    _r2.i = 11;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 186;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 187;
    _r2.i = 30;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 188;
    _r2.i = 11;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 189;
    _r2.i = 11;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 190;
    _r2.i = 11;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 191;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 192;
    XMLVM_SOURCE_POSITION("Character.java", 484)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 193;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 194;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 195;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 196;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 197;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 198;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 199;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 200;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 201;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 202;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 203;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 204;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 205;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 206;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 207;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 208;
    XMLVM_SOURCE_POSITION("Character.java", 485)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 209;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 210;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 211;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 212;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 213;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 214;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 215;
    _r2.i = 25;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 216;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 217;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 218;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 219;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 220;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 221;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 222;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 223;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 224;
    XMLVM_SOURCE_POSITION("Character.java", 486)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 225;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 226;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 227;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 228;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 229;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 230;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 231;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 232;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 233;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 234;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 235;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 236;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 237;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 238;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 239;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 240;
    XMLVM_SOURCE_POSITION("Character.java", 487)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 241;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 242;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 243;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 244;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 245;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 246;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 247;
    _r2.i = 25;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 248;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 249;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 250;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 251;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 252;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 253;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 254;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 255;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 256;
    XMLVM_SOURCE_POSITION("Character.java", 488)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 257;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 258;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 259;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 260;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 261;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 262;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 263;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 264;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 265;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 266;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 267;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 268;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 269;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 270;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 271;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 272;
    XMLVM_SOURCE_POSITION("Character.java", 489)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 273;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 274;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 275;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 276;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 277;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 278;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 279;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 280;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 281;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 282;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 283;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 284;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 285;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 286;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 287;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 288;
    XMLVM_SOURCE_POSITION("Character.java", 490)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 289;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 290;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 291;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 292;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 293;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 294;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 295;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 296;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 297;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 298;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 299;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 300;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 301;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 302;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 303;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 304;
    XMLVM_SOURCE_POSITION("Character.java", 491)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 305;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 306;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 307;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 308;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 309;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 310;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 311;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 312;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 313;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 314;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 315;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 316;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 317;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 318;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 319;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 320;
    XMLVM_SOURCE_POSITION("Character.java", 492)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 321;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 322;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 323;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 324;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 325;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 326;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 327;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 328;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 329;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 330;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 331;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 332;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 333;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 334;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 335;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 336;
    XMLVM_SOURCE_POSITION("Character.java", 493)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 337;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 338;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 339;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 340;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 341;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 342;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 343;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 344;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 345;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 346;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 347;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 348;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 349;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 350;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 351;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 352;
    XMLVM_SOURCE_POSITION("Character.java", 494)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 353;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 354;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 355;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 356;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 357;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 358;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 359;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 360;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 361;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 362;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 363;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 364;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 365;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 366;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 367;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 368;
    XMLVM_SOURCE_POSITION("Character.java", 495)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 369;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 370;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 371;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 372;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 373;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 374;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 375;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 376;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 377;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 378;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 379;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 380;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 381;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 382;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 383;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 384;
    XMLVM_SOURCE_POSITION("Character.java", 496)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 385;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 386;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 387;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 388;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 389;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 390;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 391;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 392;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 393;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 394;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 395;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 396;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 397;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 398;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 399;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 400;
    XMLVM_SOURCE_POSITION("Character.java", 497)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 401;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 402;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 403;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 404;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 405;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 406;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 407;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 408;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 409;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 410;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 411;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 412;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 413;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 414;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 415;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 416;
    XMLVM_SOURCE_POSITION("Character.java", 498)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 417;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 418;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 419;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 420;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 421;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 422;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 423;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 424;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 425;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 426;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 427;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 428;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 429;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 430;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 431;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 432;
    XMLVM_SOURCE_POSITION("Character.java", 499)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 433;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 434;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 435;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 436;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 437;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 438;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 439;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 440;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 441;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 442;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 443;
    _r2.i = 5;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 444;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 445;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 446;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 447;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 448;
    _r2.i = 5;
    XMLVM_SOURCE_POSITION("Character.java", 500)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 449;
    _r2.i = 5;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 450;
    _r2.i = 5;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 451;
    _r2.i = 5;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 452;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 453;
    _r2.i = 3;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 454;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 455;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 456;
    _r2.i = 3;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 457;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 458;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 459;
    _r2.i = 3;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 460;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 461;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 462;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 463;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 464;
    XMLVM_SOURCE_POSITION("Character.java", 501)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 465;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 466;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 467;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 468;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 469;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 470;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 471;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 472;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 473;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 474;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 475;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 476;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 477;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 478;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 479;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 480;
    XMLVM_SOURCE_POSITION("Character.java", 502)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 481;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 482;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 483;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 484;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 485;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 486;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 487;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 488;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 489;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 490;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 491;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 492;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 493;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 494;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 495;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 496;
    XMLVM_SOURCE_POSITION("Character.java", 503)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 497;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 498;
    _r2.i = 3;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 499;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 500;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 501;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 502;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 503;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 504;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 505;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 506;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 507;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 508;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 509;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 510;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 511;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 512;
    XMLVM_SOURCE_POSITION("Character.java", 504)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 513;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 514;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 515;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 516;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 517;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 518;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 519;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 520;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 521;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 522;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 523;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 524;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 525;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 526;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 527;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 528;
    XMLVM_SOURCE_POSITION("Character.java", 505)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 529;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 530;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 531;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 532;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 533;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 534;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 535;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 536;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 537;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 538;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 539;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 540;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 541;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 542;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 543;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 544;
    XMLVM_SOURCE_POSITION("Character.java", 506)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 545;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 546;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 547;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 548;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 549;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 550;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 551;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 552;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 553;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 554;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 555;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 556;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 557;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 558;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 559;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 560;
    XMLVM_SOURCE_POSITION("Character.java", 507)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 561;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 562;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 563;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 564;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 565;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 566;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 592;
    XMLVM_SOURCE_POSITION("Character.java", 509)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 593;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 594;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 595;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 596;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 597;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 598;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 599;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 600;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 601;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 602;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 603;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 604;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 605;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 606;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 607;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 608;
    XMLVM_SOURCE_POSITION("Character.java", 510)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 609;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 610;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 611;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 612;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 613;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 614;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 615;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 616;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 617;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 618;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 619;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 620;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 621;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 622;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 623;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 624;
    XMLVM_SOURCE_POSITION("Character.java", 511)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 625;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 626;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 627;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 628;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 629;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 630;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 631;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 632;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 633;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 634;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 635;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 636;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 637;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 638;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 639;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 640;
    XMLVM_SOURCE_POSITION("Character.java", 512)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 641;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 642;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 643;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 644;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 645;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 646;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 647;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 648;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 649;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 650;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 651;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 652;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 653;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 654;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 655;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 656;
    XMLVM_SOURCE_POSITION("Character.java", 513)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 657;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 658;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 659;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 660;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 661;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 662;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 663;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 664;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 665;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 666;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 667;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 668;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 669;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 670;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 671;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 672;
    XMLVM_SOURCE_POSITION("Character.java", 514)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 673;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 674;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 675;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 676;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 677;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 678;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 679;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 680;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 681;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 682;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 683;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 684;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 685;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 686;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 687;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 688;
    _r2.i = 4;
    XMLVM_SOURCE_POSITION("Character.java", 515)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 689;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 690;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 691;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 692;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 693;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 694;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 695;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 696;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 697;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 698;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 699;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 700;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 701;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 702;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 703;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 704;
    _r2.i = 4;
    XMLVM_SOURCE_POSITION("Character.java", 516)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 705;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 706;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 707;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 708;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 709;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 710;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 711;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 712;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 713;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 714;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 715;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 716;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 717;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 718;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 719;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 720;
    _r2.i = 4;
    XMLVM_SOURCE_POSITION("Character.java", 517)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 721;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 722;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 723;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 724;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 725;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 726;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 727;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 728;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 729;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 730;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 731;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 732;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 733;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 734;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 735;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 736;
    _r2.i = 4;
    XMLVM_SOURCE_POSITION("Character.java", 518)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 737;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 738;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 739;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 740;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 741;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 742;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 743;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 744;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 745;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 746;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 747;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 748;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 749;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 750;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 751;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 752;
    XMLVM_SOURCE_POSITION("Character.java", 519)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 753;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 754;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 755;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 756;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 757;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 758;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 759;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 760;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 761;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 762;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 763;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 764;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 765;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 766;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 767;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 768;
    XMLVM_SOURCE_POSITION("Character.java", 520)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 769;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 770;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 771;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 772;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 773;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 774;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 775;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 776;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 777;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 778;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 779;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 780;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 781;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 782;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 783;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 784;
    XMLVM_SOURCE_POSITION("Character.java", 521)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 785;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 786;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 787;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 788;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 789;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 790;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 791;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 792;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 793;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 794;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 795;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 796;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 797;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 798;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 799;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 800;
    XMLVM_SOURCE_POSITION("Character.java", 522)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 801;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 802;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 803;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 804;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 805;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 806;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 807;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 808;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 809;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 810;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 811;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 812;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 813;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 814;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 815;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 816;
    XMLVM_SOURCE_POSITION("Character.java", 523)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 817;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 818;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 819;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 820;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 821;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 822;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 823;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 824;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 825;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 826;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 827;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 828;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 829;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 830;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 831;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 832;
    XMLVM_SOURCE_POSITION("Character.java", 524)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 833;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 834;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 835;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 836;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 837;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 838;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 839;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 840;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 841;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 842;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 843;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 844;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 845;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 846;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 847;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 848;
    XMLVM_SOURCE_POSITION("Character.java", 525)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 849;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 850;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 851;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 852;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 853;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 854;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 855;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 861;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 862;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 863;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 864;
    XMLVM_SOURCE_POSITION("Character.java", 526)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 865;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 866;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 867;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 868;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 869;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 870;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 871;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 872;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 873;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 874;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 875;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 876;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 877;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 878;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 879;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    _r1.i = 884;
    XMLVM_SOURCE_POSITION("Character.java", 527)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 885;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 890;
    _r2.i = 4;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 894;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 900;
    XMLVM_SOURCE_POSITION("Character.java", 528)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 901;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r7.i;
    _r1.i = 902;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 903;
    _r2.i = 24;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 904;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 905;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 906;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 908;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 910;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 911;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 912;
    XMLVM_SOURCE_POSITION("Character.java", 529)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 913;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 914;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 915;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 916;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 917;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 918;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 919;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 920;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 921;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 922;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 923;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 924;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 925;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 926;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 927;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 928;
    XMLVM_SOURCE_POSITION("Character.java", 530)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 929;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 931;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 932;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 933;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 934;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 935;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 936;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 937;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 938;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 939;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 940;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 941;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 942;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 943;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 944;
    XMLVM_SOURCE_POSITION("Character.java", 531)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 945;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 946;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 947;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 948;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 949;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 950;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 951;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 952;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 953;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 954;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 955;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 956;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 957;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 958;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 959;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 960;
    XMLVM_SOURCE_POSITION("Character.java", 532)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 961;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 962;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 963;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 964;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 965;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 966;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 967;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 968;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 969;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 970;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 971;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 972;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 973;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 974;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 976;
    XMLVM_SOURCE_POSITION("Character.java", 533)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 977;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 978;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 979;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 980;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 981;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 982;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 983;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 984;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 985;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 986;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 987;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 988;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 989;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 990;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 991;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 992;
    XMLVM_SOURCE_POSITION("Character.java", 534)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 993;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 994;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 995;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 996;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 997;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 998;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r4.i;
    _r1.i = 999;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    java_lang_Character_PUT_typeValuesCache( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 540)
    // "z\177740\265\1347\366\177740\376\177740\377y\100457\177777\461\177430\100467\177777\100510\177777\100567\177777\100576\177777\577\177324\100605\177777\610\177777\614\177777\622\177777\625a\631\177777\100645\177777\650\177777\655\177777\660\177777\100666\177777\671\177777\675\177777\6778\705\177777\706\177776\710\177777\711\177776\713\177777\714\177776\100734\177777\735\177661\100757\177777\762\177777\763\177776\765\177777\101037\177777\101063\177777\1123\177456\1124\177462\1127\177463\1131\177466\1133\177465\1140\177463\1143\177461\1150\177457\1151\177455\1157\177455\1162\177453\1165\177452\1200\177446\1203\177446\1210\177446\1213\177447\1222\177445\1505T\1654\177732\1657\177733\1701\177740\1702\177741\1713\177740\1714\177700\1716\177701\1720\177702\1721\177707\1725\177721\1726\177712\101757\177777\1760\177652\1761\177660\1762\177661\2117\177740\2137\177660\102201\177777\102277\177777\102304\177777\2310\177777\2314\177777\102365\177777\2371\177777\2606\177720\117225\177777\17233\177705\117371\177777\17407\010\17425\010\17447\010\17467\010\17505\010\117527\010\17547\010\17561J\17565V\17567d\17571\200\17573p\17575\176\17607\010\17627\010\17647\010\17661\010\17663\011\17676\161733\17703\011\17721\010\17741\010\17745\007\17763\011\20577\177760\22351\177746\177532\177740"
    _r0.o = xmlvm_create_java_string_from_pool(155);
    XMLVM_SOURCE_POSITION("Character.java", 541)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_getValue__(_r0.o);
    java_lang_Character_PUT_uppercaseValues( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 543)
    _r0.i = 819;
    XMLVM_CLASS_INIT(int)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_int, _r0.i);
    XMLVMArray_fillArray(((org_xmlvm_runtime_XMLVMArray*) _r0.o), (JAVA_ARRAY_INT[]){924, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 247, 216, 217, 218, 219, 220, 221, 222, 376, 256, 256, 258, 258, 260, 260, 262, 262, 264, 264, 266, 266, 268, 268, 270, 270, 272, 272, 274, 274, 276, 276, 278, 278, 280, 280, 282, 282, 284, 284, 286, 286, 288, 288, 290, 290, 292, 292, 294, 294, 296, 296, 298, 298, 300, 300, 302, 302, 304, 73, 306, 306, 308, 308, 310, 310, 312, 313, 313, 315, 315, 317, 317, 319, 319, 321, 321, 323, 323, 325, 325, 327, 327, 329, 330, 330, 332, 332, 334, 334, 336, 336, 338, 338, 340, 340, 342, 342, 344, 344, 346, 346, 348, 348, 350, 350, 352, 352, 354, 354, 356, 356, 358, 358, 360, 360, 362, 362, 364, 364, 366, 366, 368, 368, 370, 370, 372, 372, 374, 374, 376, 377, 377, 379, 379, 381, 381, 83, 384, 385, 386, 386, 388, 388, 390, 391, 391, 393, 394, 395, 395, 397, 398, 399, 400, 401, 401, 403, 404, 502, 406, 407, 408, 408, 410, 411, 412, 413, 544, 415, 416, 416, 418, 418, 420, 420, 422, 423, 423, 425, 426, 427, 428, 428, 430, 431, 431, 433, 434, 435, 435, 437, 437, 439, 440, 440, 442, 443, 444, 444, 446, 503, 448, 449, 450, 451, 452, 452, 452, 455, 455, 455, 458, 458, 458, 461, 461, 463, 463, 465, 465, 467, 467, 469, 469, 471, 471, 473, 473, 475, 475, 398, 478, 478, 480, 480, 482, 482, 484, 484, 486, 486, 488, 488, 490, 490, 492, 492, 494, 494, 496, 497, 497, 497, 500, 500, 502, 503, 504, 504, 506, 506, 508, 508, 510, 510, 512, 512, 514, 514, 516, 516, 518, 518, 520, 520, 522, 522, 524, 524, 526, 526, 528, 528, 530, 530, 532, 532, 534, 534, 536, 536, 538, 538, 540, 540, 542, 542, 544, 545, 546, 546, 548, 548, 550, 550, 552, 552, 554, 554, 556, 556, 558, 558, 560, 560, 562, 562, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 385, 390, 597, 393, 394, 600, 399, 602, 400, 604, 605, 606, 607, 403, 609, 610, 404, 612, 613, 614, 615, 407, 406, 618, 619, 620, 621, 622, 412, 624, 625, 413, 627, 628, 415, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639, 422, 641, 642, 425, 644, 645, 646, 647, 430, 649, 433, 434, 652, 653, 654, 655, 656, 657, 439, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765, 766, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831, 832, 833, 834, 835, 836, 921, 838, 839, 840, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863, 864, 865, 866, 867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 878, 879, 880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 899, 900, 901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 902, 904, 905, 906, 944, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927, 928, 929, 931, 931, 932, 933, 934, 935, 936, 937, 938, 939, 908, 910, 911, 975, 914, 920, 978, 979, 980, 934, 928, 983, 984, 984, 986, 986, 988, 988, 990, 990, 992, 992, 994, 994, 996, 996, 998, 998, });
    java_lang_Character_PUT_uppercaseValuesCache( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 599)
    // "Z \326 \336 \100456\001\460\177471\100466\001\100507\001\100566\001\570\177607\100575\001\601\322\100604\001\606\316\607\001\612\315\613\001\616O\617\312\620\313\621\001\623\315\624\317\626\323\627\321\630\001\634\323\635\325\637\326\100644\001\646\332\647\001\651\332\654\001\656\332\657\001\662\331\100665\001\667\333\670\001\674\001\704\002\705\001\707\002\710\001\712\002\100733\001\100756\001\761\002\100764\001\766\177637\767\177710\101036\001\101062\001\1606&\1612%\1614@\1617?\1641 \1653 \101756\001\2017P\2057 \102200\001\102276\001\102303\001\2307\001\2313\001\102364\001\2370\001\25260\117224\001\117370\001\17417\177770\17435\177770\17457\177770\17477\177770\17515\177770\117537\177770\17557\177770\17617\177770\17637\177770\17657\177770\17671\177770\17673\177666\17674\177767\17713\177652\17714\177767\17731\177770\17733\177634\17751\177770\17753\177620\17754\177771\17771\177600\17773\177602\17774\177767\20446\161243\20452\157501\20453\157672\20557\020\22317\032\177472 "
    _r0.o = xmlvm_create_java_string_from_pool(156);
    XMLVM_SOURCE_POSITION("Character.java", 600)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_getValue__(_r0.o);
    java_lang_Character_PUT_lowercaseValues( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 602)
    _r0.i = 808;
    XMLVM_CLASS_INIT(int)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_int, _r0.i);
    XMLVMArray_fillArray(((org_xmlvm_runtime_XMLVMArray*) _r0.o), (JAVA_ARRAY_INT[]){224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 215, 248, 249, 250, 251, 252, 253, 254, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 257, 257, 259, 259, 261, 261, 263, 263, 265, 265, 267, 267, 269, 269, 271, 271, 273, 273, 275, 275, 277, 277, 279, 279, 281, 281, 283, 283, 285, 285, 287, 287, 289, 289, 291, 291, 293, 293, 295, 295, 297, 297, 299, 299, 301, 301, 303, 303, 105, 305, 307, 307, 309, 309, 311, 311, 312, 314, 314, 316, 316, 318, 318, 320, 320, 322, 322, 324, 324, 326, 326, 328, 328, 329, 331, 331, 333, 333, 335, 335, 337, 337, 339, 339, 341, 341, 343, 343, 345, 345, 347, 347, 349, 349, 351, 351, 353, 353, 355, 355, 357, 357, 359, 359, 361, 361, 363, 363, 365, 365, 367, 367, 369, 369, 371, 371, 373, 373, 375, 375, 255, 378, 378, 380, 380, 382, 382, 383, 384, 595, 387, 387, 389, 389, 596, 392, 392, 598, 599, 396, 396, 397, 477, 601, 603, 402, 402, 608, 611, 405, 617, 616, 409, 409, 410, 411, 623, 626, 414, 629, 417, 417, 419, 419, 421, 421, 640, 424, 424, 643, 426, 427, 429, 429, 648, 432, 432, 650, 651, 436, 436, 438, 438, 658, 441, 441, 442, 443, 445, 445, 446, 447, 448, 449, 450, 451, 454, 454, 454, 457, 457, 457, 460, 460, 460, 462, 462, 464, 464, 466, 466, 468, 468, 470, 470, 472, 472, 474, 474, 476, 476, 477, 479, 479, 481, 481, 483, 483, 485, 485, 487, 487, 489, 489, 491, 491, 493, 493, 495, 495, 496, 499, 499, 499, 501, 501, 405, 447, 505, 505, 507, 507, 509, 509, 511, 511, 513, 513, 515, 515, 517, 517, 519, 519, 521, 521, 523, 523, 525, 525, 527, 527, 529, 529, 531, 531, 533, 533, 535, 535, 537, 537, 539, 539, 541, 541, 543, 543, 414, 545, 547, 547, 549, 549, 551, 551, 553, 553, 555, 555, 557, 557, 559, 559, 561, 561, 563, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611, 612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626, 627, 628, 629, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639, 640, 641, 642, 643, 644, 645, 646, 647, 648, 649, 650, 651, 652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765, 766, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831, 832, 833, 834, 835, 836, 837, 838, 839, 840, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863, 864, 865, 866, 867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 878, 879, 880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 899, 900, 901, 940, 903, 941, 942, 943, 907, 972, 909, 973, 974, 912, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959, 960, 961, 930, 963, 964, 965, 966, 967, 968, 969, 970, 971, 940, 941, 942, 943, 944, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971, 972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 985, 985, 987, 987, 989, 989, 991, 991, 993, 993, 995, 995, 997, 997, 999, 999, });
    java_lang_Character_PUT_lowercaseValuesCache( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 657)
    // "90Z7zW\3151\3140\3371\3360\4557\4546\4757\4746\5157\5146\5357\5346\5557\5546\5757\5746\6157\6146\6357\6346\6557\6546\7131\7120\7331\7320\7451\7440\10111\10100\11561\11550\13751\13740\14031\14020\177431\177420\177472\177427\177532\177467"
    _r0.o = xmlvm_create_java_string_from_pool(157);
    XMLVM_SOURCE_POSITION("Character.java", 658)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_getValue__(_r0.o);
    java_lang_Character_PUT_digitValues( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 660)
    // "\002\002\002\002\002\002\002\002\002\000\000\000\000\000\002\002\002\002\002\002\002\002\002\002\002\002\002\002\000\000\000\000\000\000\000\000\003\000\000\000\000\000\000\000\000\000\000\000\002\002\002\002\002\002\002\002\002\002\000\000\000\000\000\000\000\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\000\000\000\000\003\000\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\003\000\000\000\000\002"
    _r0.o = xmlvm_create_java_string_from_pool(158);
    XMLVM_SOURCE_POSITION("Character.java", 661)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_getValue__(_r0.o);
    java_lang_Character_PUT_typeTags( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 663)
    _r0.i = 19;
    XMLVM_CLASS_INIT(byte)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, _r0.i);
    XMLVM_SOURCE_POSITION("Character.java", 664)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r4.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r4.i;
    XMLVM_SOURCE_POSITION("Character.java", 665)
    _r1.i = 3;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r1.i;
    _r1.i = 3;
    _r2.i = 4;
    XMLVM_SOURCE_POSITION("Character.java", 666)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 4;
    _r2.i = 5;
    XMLVM_SOURCE_POSITION("Character.java", 667)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 5;
    XMLVM_SOURCE_POSITION("Character.java", 668)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r5.i;
    XMLVM_SOURCE_POSITION("Character.java", 669)
    _r1.i = 7;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r5.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r1.i;
    _r1.i = 7;
    _r2.i = 10;
    XMLVM_SOURCE_POSITION("Character.java", 670)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 8;
    _r2.i = 11;
    XMLVM_SOURCE_POSITION("Character.java", 671)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 9;
    _r2.i = 12;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 10;
    _r2.i = 13;
    XMLVM_SOURCE_POSITION("Character.java", 672)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 11;
    _r2.i = 14;
    XMLVM_SOURCE_POSITION("Character.java", 673)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 12;
    XMLVM_SOURCE_POSITION("Character.java", 674)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r6.i;
    _r1.i = 13;
    XMLVM_SOURCE_POSITION("Character.java", 675)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r3.i;
    _r1.i = 14;
    _r2.i = 16;
    XMLVM_SOURCE_POSITION("Character.java", 676)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    XMLVM_SOURCE_POSITION("Character.java", 677)
    _r1.i = 17;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r6.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r6.i] = _r1.i;
    _r1.i = 16;
    _r2.i = 18;
    XMLVM_SOURCE_POSITION("Character.java", 678)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 17;
    _r2.i = 8;
    XMLVM_SOURCE_POSITION("Character.java", 679)
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    _r1.i = 18;
    _r2.i = 9;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r2.i;
    java_lang_Character_PUT_DIRECTIONALITY( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 688)
    // "\705\705\710\710\713\713\762\762"
    _r0.o = xmlvm_create_java_string_from_pool(159);
    XMLVM_SOURCE_POSITION("Character.java", 689)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_getValue__(_r0.o);
    java_lang_Character_PUT_titlecaseValues( _r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 694)
    // "90Z7zW\263\260\271\270\276\000\3151\3140\3371\3360\4557\4546\4757\4746\4767\4763\4771\4751\5157\5146\5357\5346\5557\5546\5760\5746\5761\5615\5762\4012\6157\6146\6357\6346\6557\6546\7131\7120\7331\7320\7451\7440\10111\10100\11562\11550\11563\11537\11564\11526\11565\11515\11566\11504\11567\11473\11570\11462\11571\11451\11572\11440\11573\11427\11574\166154\13360\13335\13751\13740\14031\14020\20160\20160\20171\20160\20211\20200\20536\000\20537\20536\20553\20537\20554\20472\20555\20411\20556\17572\20557\16607\20573\20557\20574\20512\20575\20431\20576\17612\20577\16627\20600\16630\20601\6771\20602\175162\22163\22137\22207\22163\22233\22207\22352\22352\23577\23565\23611\23577\23623\23611\30007\30007\30051\30040\30070\30056\30071\30045\30072\30034\31211\31177\177431\177420\177472\177427\177532\177467"
    _r0.o = xmlvm_create_java_string_from_pool(160);
    XMLVM_SOURCE_POSITION("Character.java", 695)
    XMLVM_CHECK_NPE(0)
    _r0.o = java_lang_String_getValue__(_r0.o);
    java_lang_Character_PUT_numericValues( _r0.o);
    XMLVM_EXIT_METHOD()
    return;
    label4189:;
    _r0.i = 0;
    goto label16;
    label4192:;
    label5834:;
    //XMLVM_END_WRAPPER
}

void java_lang_Character___INIT____char(JAVA_OBJECT me, JAVA_CHAR n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Character___INIT____char]
    XMLVM_ENTER_METHOD("java.lang.Character", "<init>", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r0.o = me;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 1975)
    XMLVM_CHECK_NPE(0)
    java_lang_Object___INIT___(_r0.o);
    XMLVM_SOURCE_POSITION("Character.java", 1976)
    XMLVM_CHECK_NPE(0)
    ((java_lang_Character*) _r0.o)->fields.java_lang_Character.value_ = _r1.i;
    XMLVM_SOURCE_POSITION("Character.java", 1977)
    XMLVM_EXIT_METHOD()
    return;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_lang_Character_charValue__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_charValue__]
    XMLVM_ENTER_METHOD("java.lang.Character", "charValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Character.java", 1985)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Character*) _r1.o)->fields.java_lang_Character.value_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_compareTo___java_lang_Character(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_compareTo___java_lang_Character]
    XMLVM_ENTER_METHOD("java.lang.Character", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Character.java", 2003)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Character*) _r2.o)->fields.java_lang_Character.value_;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_Character*) _r3.o)->fields.java_lang_Character.value_;
    _r0.i = _r0.i - _r1.i;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Character_valueOf___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_valueOf___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "valueOf", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 2018)
    _r0.i = 512;
    if (_r1.i < _r0.i) goto label10;
    XMLVM_SOURCE_POSITION("Character.java", 2019)
    _r0.o = __NEW_java_lang_Character();
    XMLVM_CHECK_NPE(0)
    java_lang_Character___INIT____char(_r0.o, _r1.i);
    label9:;
    XMLVM_SOURCE_POSITION("Character.java", 2021)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label10:;

    
    // Red class access removed: java.lang.Character$valueOfCache::access$0
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.o = ((JAVA_ARRAY_OBJECT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    goto label9;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isValidCodePoint___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isValidCodePoint___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isValidCodePoint", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 2050)
    if (_r1.i < 0) goto label9;
    _r0.i = 1114111;
    if (_r0.i < _r1.i) goto label9;
    _r0.i = 1;
    label8:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label9:;
    _r0.i = 0;
    goto label8;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isSupplementaryCodePoint___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isSupplementaryCodePoint___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isSupplementaryCodePoint", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 2064)
    _r0.i = 65536;
    if (_r0.i > _r1.i) goto label11;
    _r0.i = 1114111;
    if (_r0.i < _r1.i) goto label11;
    _r0.i = 1;
    label10:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label11:;
    _r0.i = 0;
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isHighSurrogate___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isHighSurrogate___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isHighSurrogate", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 2080)
    _r0.i = 55296;
    if (_r0.i > _r1.i) goto label12;
    _r0.i = 56319;
    if (_r0.i < _r1.i) goto label12;
    _r0.i = 1;
    label11:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    _r0.i = 0;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isLowSurrogate___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isLowSurrogate___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isLowSurrogate", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 2096)
    _r0.i = 56320;
    if (_r0.i > _r1.i) goto label12;
    _r0.i = 57343;
    if (_r0.i < _r1.i) goto label12;
    _r0.i = 1;
    label11:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    _r0.i = 0;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isSurrogatePair___char_char(JAVA_CHAR n1, JAVA_CHAR n2)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isSurrogatePair___char_char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isSurrogatePair", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.i = n1;
    _r2.i = n2;
    XMLVM_SOURCE_POSITION("Character.java", 2114)
    _r0.i = java_lang_Character_isHighSurrogate___char(_r1.i);
    if (_r0.i == 0) goto label14;
    _r0.i = java_lang_Character_isLowSurrogate___char(_r2.i);
    if (_r0.i == 0) goto label14;
    _r0.i = 1;
    label13:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label14:;
    _r0.i = 0;
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_charCount___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_charCount___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "charCount", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 2133)
    _r0.i = 65536;
    if (_r1.i < _r0.i) goto label6;
    _r0.i = 2;
    label5:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label6:;
    _r0.i = 1;
    goto label5;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_toCodePoint___char_char(JAVA_CHAR n1, JAVA_CHAR n2)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_toCodePoint___char_char]
    XMLVM_ENTER_METHOD("java.lang.Character", "toCodePoint", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.i = n1;
    _r3.i = n2;
    XMLVM_SOURCE_POSITION("Character.java", 2154)
    _r0.i = _r2.i & 1023;
    _r0.i = _r0.i << 10;
    _r1.i = _r3.i & 1023;
    _r0.i = _r0.i | _r1.i;
    _r1.i = 65536;
    _r0.i = _r0.i + _r1.i;
    XMLVM_SOURCE_POSITION("Character.java", 2156)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_codePointAt___java_lang_CharSequence_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_codePointAt___java_lang_CharSequence_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "codePointAt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("Character.java", 2182)
    if (_r3.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Character.java", 2183)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 2185)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__])(_r3.o);
    XMLVM_SOURCE_POSITION("Character.java", 2186)
    if (_r4.i < 0) goto label16;
    if (_r4.i < _r0.i) goto label22;
    label16:;
    XMLVM_SOURCE_POSITION("Character.java", 2187)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label22:;
    XMLVM_SOURCE_POSITION("Character.java", 2190)
    _r1.i = _r4.i + 1;
    XMLVM_CHECK_NPE(3)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int])(_r3.o, _r4.i);
    XMLVM_SOURCE_POSITION("Character.java", 2191)
    if (_r1.i < _r0.i) goto label32;
    _r0 = _r2;
    label31:;
    XMLVM_SOURCE_POSITION("Character.java", 2192)
    XMLVM_SOURCE_POSITION("Character.java", 2198)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label32:;
    XMLVM_SOURCE_POSITION("Character.java", 2194)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int])(_r3.o, _r1.i);
    XMLVM_SOURCE_POSITION("Character.java", 2195)
    _r1.i = java_lang_Character_isSurrogatePair___char_char(_r2.i, _r0.i);
    if (_r1.i == 0) goto label47;
    XMLVM_SOURCE_POSITION("Character.java", 2196)
    _r0.i = java_lang_Character_toCodePoint___char_char(_r2.i, _r0.i);
    goto label31;
    label47:;
    _r0 = _r2;
    goto label31;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_codePointAt___char_1ARRAY_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_codePointAt___char_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "codePointAt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("Character.java", 2224)
    if (_r3.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Character.java", 2225)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 2227)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    XMLVM_SOURCE_POSITION("Character.java", 2228)
    if (_r4.i < 0) goto label13;
    if (_r4.i < _r0.i) goto label19;
    label13:;
    XMLVM_SOURCE_POSITION("Character.java", 2229)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label19:;
    XMLVM_SOURCE_POSITION("Character.java", 2232)
    _r1.i = _r4.i + 1;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    XMLVM_SOURCE_POSITION("Character.java", 2233)
    if (_r1.i < _r0.i) goto label27;
    _r0 = _r2;
    label26:;
    XMLVM_SOURCE_POSITION("Character.java", 2234)
    XMLVM_SOURCE_POSITION("Character.java", 2240)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label27:;
    XMLVM_SOURCE_POSITION("Character.java", 2236)
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r1.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    XMLVM_SOURCE_POSITION("Character.java", 2237)
    _r1.i = java_lang_Character_isSurrogatePair___char_char(_r2.i, _r0.i);
    if (_r1.i == 0) goto label40;
    XMLVM_SOURCE_POSITION("Character.java", 2238)
    _r0.i = java_lang_Character_toCodePoint___char_char(_r2.i, _r0.i);
    goto label26;
    label40:;
    _r0 = _r2;
    goto label26;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_codePointAt___char_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_codePointAt___char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "codePointAt", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = n1;
    _r4.i = n2;
    _r5.i = n3;
    XMLVM_SOURCE_POSITION("Character.java", 2269)
    if (_r4.i < 0) goto label9;
    if (_r4.i >= _r5.i) goto label9;
    if (_r5.i < 0) goto label9;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    if (_r5.i <= _r0.i) goto label15;
    label9:;
    XMLVM_SOURCE_POSITION("Character.java", 2270)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label15:;
    XMLVM_SOURCE_POSITION("Character.java", 2273)
    _r0.i = _r4.i + 1;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    XMLVM_SOURCE_POSITION("Character.java", 2274)
    if (_r0.i < _r5.i) goto label23;
    _r0 = _r1;
    label22:;
    XMLVM_SOURCE_POSITION("Character.java", 2275)
    XMLVM_SOURCE_POSITION("Character.java", 2281)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label23:;
    XMLVM_SOURCE_POSITION("Character.java", 2277)
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 2278)
    _r2.i = java_lang_Character_isSurrogatePair___char_char(_r1.i, _r0.i);
    if (_r2.i == 0) goto label36;
    XMLVM_SOURCE_POSITION("Character.java", 2279)
    _r0.i = java_lang_Character_toCodePoint___char_char(_r1.i, _r0.i);
    goto label22;
    label36:;
    _r0 = _r1;
    goto label22;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_codePointBefore___java_lang_CharSequence_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_codePointBefore___java_lang_CharSequence_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "codePointBefore", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("Character.java", 2307)
    if (_r3.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Character.java", 2308)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 2310)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__])(_r3.o);
    XMLVM_SOURCE_POSITION("Character.java", 2311)
    _r1.i = 1;
    if (_r4.i < _r1.i) goto label17;
    if (_r4.i <= _r0.i) goto label23;
    label17:;
    XMLVM_SOURCE_POSITION("Character.java", 2312)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label23:;
    XMLVM_SOURCE_POSITION("Character.java", 2315)
    _r0.i = _r4.i + -1;
    XMLVM_CHECK_NPE(3)
    _r1.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int])(_r3.o, _r0.i);
    XMLVM_SOURCE_POSITION("Character.java", 2316)
    _r0.i = _r0.i + -1;
    if (_r0.i >= 0) goto label35;
    _r0 = _r1;
    label34:;
    XMLVM_SOURCE_POSITION("Character.java", 2317)
    XMLVM_SOURCE_POSITION("Character.java", 2323)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label35:;
    XMLVM_SOURCE_POSITION("Character.java", 2319)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int])(_r3.o, _r0.i);
    XMLVM_SOURCE_POSITION("Character.java", 2320)
    _r2.i = java_lang_Character_isSurrogatePair___char_char(_r0.i, _r1.i);
    if (_r2.i == 0) goto label50;
    XMLVM_SOURCE_POSITION("Character.java", 2321)
    _r0.i = java_lang_Character_toCodePoint___char_char(_r0.i, _r1.i);
    goto label34;
    label50:;
    _r0 = _r1;
    goto label34;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_codePointBefore___char_1ARRAY_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_codePointBefore___char_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "codePointBefore", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r3.o = n1;
    _r4.i = n2;
    XMLVM_SOURCE_POSITION("Character.java", 2349)
    if (_r3.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Character.java", 2350)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 2352)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    XMLVM_SOURCE_POSITION("Character.java", 2353)
    _r1.i = 1;
    if (_r4.i < _r1.i) goto label14;
    if (_r4.i <= _r0.i) goto label20;
    label14:;
    XMLVM_SOURCE_POSITION("Character.java", 2354)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label20:;
    XMLVM_SOURCE_POSITION("Character.java", 2357)
    _r0.i = _r4.i + -1;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 2358)
    _r0.i = _r0.i + -1;
    if (_r0.i >= 0) goto label30;
    _r0 = _r1;
    label29:;
    XMLVM_SOURCE_POSITION("Character.java", 2359)
    XMLVM_SOURCE_POSITION("Character.java", 2365)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label30:;
    XMLVM_SOURCE_POSITION("Character.java", 2361)
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 2362)
    _r2.i = java_lang_Character_isSurrogatePair___char_char(_r0.i, _r1.i);
    if (_r2.i == 0) goto label43;
    XMLVM_SOURCE_POSITION("Character.java", 2363)
    _r0.i = java_lang_Character_toCodePoint___char_char(_r0.i, _r1.i);
    goto label29;
    label43:;
    _r0 = _r1;
    goto label29;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_codePointBefore___char_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_codePointBefore___char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "codePointBefore", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = n1;
    _r4.i = n2;
    _r5.i = n3;
    XMLVM_SOURCE_POSITION("Character.java", 2396)
    if (_r3.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Character.java", 2397)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 2399)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r3.o));
    XMLVM_SOURCE_POSITION("Character.java", 2400)
    if (_r4.i <= _r5.i) goto label17;
    if (_r4.i > _r0.i) goto label17;
    if (_r5.i < 0) goto label17;
    if (_r5.i < _r0.i) goto label23;
    label17:;
    XMLVM_SOURCE_POSITION("Character.java", 2401)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label23:;
    XMLVM_SOURCE_POSITION("Character.java", 2404)
    _r0.i = _r4.i + -1;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 2405)
    _r0.i = _r0.i + -1;
    if (_r0.i >= _r5.i) goto label33;
    _r0 = _r1;
    label32:;
    XMLVM_SOURCE_POSITION("Character.java", 2406)
    XMLVM_SOURCE_POSITION("Character.java", 2412)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label33:;
    XMLVM_SOURCE_POSITION("Character.java", 2408)
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 2409)
    _r2.i = java_lang_Character_isSurrogatePair___char_char(_r0.i, _r1.i);
    if (_r2.i == 0) goto label46;
    XMLVM_SOURCE_POSITION("Character.java", 2410)
    _r0.i = java_lang_Character_toCodePoint___char_char(_r0.i, _r1.i);
    goto label32;
    label46:;
    _r0 = _r1;
    goto label32;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_toChars___int_char_1ARRAY_int(JAVA_INT n1, JAVA_OBJECT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_toChars___int_char_1ARRAY_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "toChars", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.i = n1;
    _r4.o = n2;
    _r5.i = n3;
    _r1.i = 1;
    XMLVM_SOURCE_POSITION("Character.java", 2439)
    _r0.i = java_lang_Character_isValidCodePoint___int(_r3.i);
    if (_r0.i != 0) goto label13;
    XMLVM_SOURCE_POSITION("Character.java", 2440)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label13:;
    XMLVM_SOURCE_POSITION("Character.java", 2442)
    if (_r4.o != JAVA_NULL) goto label21;
    XMLVM_SOURCE_POSITION("Character.java", 2443)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label21:;
    XMLVM_SOURCE_POSITION("Character.java", 2445)
    if (_r5.i < 0) goto label26;
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    if (_r5.i < _r0.i) goto label32;
    label26:;
    XMLVM_SOURCE_POSITION("Character.java", 2446)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label32:;
    XMLVM_SOURCE_POSITION("Character.java", 2449)
    _r0.i = java_lang_Character_isSupplementaryCodePoint___int(_r3.i);
    if (_r0.i == 0) goto label76;
    XMLVM_SOURCE_POSITION("Character.java", 2450)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    _r0.i = _r0.i - _r1.i;
    if (_r5.i != _r0.i) goto label48;
    XMLVM_SOURCE_POSITION("Character.java", 2451)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label48:;
    XMLVM_SOURCE_POSITION("Character.java", 2455)
    _r0.i = 65536;
    _r0.i = _r3.i - _r0.i;
    _r1.i = 55296;
    _r2.i = _r0.i >> 10;
    _r2.i = _r2.i & 1023;
    _r1.i = _r1.i | _r2.i;
    _r2.i = 56320;
    _r0.i = _r0.i & 1023;
    _r0.i = _r0.i | _r2.i;
    _r1.i = _r1.i & 0xffff;
    XMLVM_SOURCE_POSITION("Character.java", 2458)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r1.i;
    XMLVM_SOURCE_POSITION("Character.java", 2459)
    _r1.i = _r5.i + 1;
    _r0.i = _r0.i & 0xffff;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r1.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i] = _r0.i;
    XMLVM_SOURCE_POSITION("Character.java", 2460)
    _r0.i = 2;
    label75:;
    XMLVM_SOURCE_POSITION("Character.java", 2464)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label76:;
    XMLVM_SOURCE_POSITION("Character.java", 2463)
    _r0.i = _r3.i & 0xffff;
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r5.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i] = _r0.i;
    _r0 = _r1;
    goto label75;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Character_toChars___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_toChars___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "toChars", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.i = n1;
    _r4.i = 1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("Character.java", 2482)
    _r0.i = java_lang_Character_isValidCodePoint___int(_r5.i);
    if (_r0.i != 0) goto label14;
    XMLVM_SOURCE_POSITION("Character.java", 2483)

    
    // Red class access removed: java.lang.IllegalArgumentException::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.IllegalArgumentException::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label14:;
    XMLVM_SOURCE_POSITION("Character.java", 2486)
    _r0.i = java_lang_Character_isSupplementaryCodePoint___int(_r5.i);
    if (_r0.i == 0) goto label49;
    XMLVM_SOURCE_POSITION("Character.java", 2487)
    _r0.i = 65536;
    _r0.i = _r5.i - _r0.i;
    _r1.i = 55296;
    _r2.i = _r0.i >> 10;
    _r2.i = _r2.i & 1023;
    _r1.i = _r1.i | _r2.i;
    _r2.i = 56320;
    _r0.i = _r0.i & 1023;
    _r0.i = _r0.i | _r2.i;
    _r2.i = 2;
    XMLVM_SOURCE_POSITION("Character.java", 2490)
    XMLVM_CLASS_INIT(char)
    _r2.o = XMLVMArray_createSingleDimension(__CLASS_char, _r2.i);
    _r1.i = _r1.i & 0xffff;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r3.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r1.i;
    _r0.i = _r0.i & 0xffff;
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r4.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i] = _r0.i;
    _r0 = _r2;
    label48:;
    XMLVM_SOURCE_POSITION("Character.java", 2492)
    XMLVM_EXIT_METHOD()
    return _r0.o;
    label49:;
    XMLVM_CLASS_INIT(char)
    _r0.o = XMLVMArray_createSingleDimension(__CLASS_char, _r4.i);
    _r1.i = _r5.i & 0xffff;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i] = _r1.i;
    goto label48;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_codePointCount___java_lang_CharSequence_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_codePointCount___java_lang_CharSequence_int_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "codePointCount", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r3.o = n1;
    _r4.i = n2;
    _r5.i = n3;
    XMLVM_SOURCE_POSITION("Character.java", 2517)
    if (_r3.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Character.java", 2518)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 2520)
    XMLVM_CHECK_NPE(3)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__])(_r3.o);
    XMLVM_SOURCE_POSITION("Character.java", 2521)
    if (_r4.i < 0) goto label18;
    if (_r5.i > _r0.i) goto label18;
    if (_r4.i <= _r5.i) goto label24;
    label18:;
    XMLVM_SOURCE_POSITION("Character.java", 2522)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label24:;
    XMLVM_SOURCE_POSITION("Character.java", 2525)
    _r0.i = 0;
    _r1 = _r0;
    _r0 = _r4;
    label27:;
    XMLVM_SOURCE_POSITION("Character.java", 2526)
    if (_r0.i < _r5.i) goto label30;
    XMLVM_SOURCE_POSITION("Character.java", 2538)
    XMLVM_EXIT_METHOD()
    return _r1.i;
    label30:;
    XMLVM_SOURCE_POSITION("Character.java", 2527)
    XMLVM_CHECK_NPE(3)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int])(_r3.o, _r0.i);
    XMLVM_SOURCE_POSITION("Character.java", 2528)
    _r2.i = java_lang_Character_isHighSurrogate___char(_r2.i);
    if (_r2.i == 0) goto label56;
    XMLVM_SOURCE_POSITION("Character.java", 2529)
    _r0.i = _r0.i + 1;
    if (_r0.i >= _r5.i) goto label56;
    XMLVM_SOURCE_POSITION("Character.java", 2530)
    XMLVM_CHECK_NPE(3)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r3.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int])(_r3.o, _r0.i);
    XMLVM_SOURCE_POSITION("Character.java", 2531)
    _r2.i = java_lang_Character_isLowSurrogate___char(_r2.i);
    if (_r2.i != 0) goto label56;
    XMLVM_SOURCE_POSITION("Character.java", 2532)
    _r1.i = _r1.i + 1;
    label56:;
    XMLVM_SOURCE_POSITION("Character.java", 2536)
    _r1.i = _r1.i + 1;
    _r0.i = _r0.i + 1;
    goto label27;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_codePointCount___char_1ARRAY_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_codePointCount___char_1ARRAY_int_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "codePointCount", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.o = n1;
    _r5.i = n2;
    _r6.i = n3;
    XMLVM_SOURCE_POSITION("Character.java", 2564)
    if (_r4.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Character.java", 2565)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 2567)
    _r0.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    XMLVM_SOURCE_POSITION("Character.java", 2568)
    _r1.i = _r5.i + _r6.i;
    if (_r5.i < 0) goto label17;
    XMLVM_SOURCE_POSITION("Character.java", 2569)
    if (_r6.i < 0) goto label17;
    if (_r1.i <= _r0.i) goto label23;
    label17:;
    XMLVM_SOURCE_POSITION("Character.java", 2570)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label23:;
    XMLVM_SOURCE_POSITION("Character.java", 2573)
    _r0.i = 0;
    _r2 = _r0;
    _r0 = _r5;
    label26:;
    XMLVM_SOURCE_POSITION("Character.java", 2574)
    if (_r0.i < _r1.i) goto label29;
    XMLVM_SOURCE_POSITION("Character.java", 2586)
    XMLVM_EXIT_METHOD()
    return _r2.i;
    label29:;
    XMLVM_SOURCE_POSITION("Character.java", 2575)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r0.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 2576)
    _r3.i = java_lang_Character_isHighSurrogate___char(_r3.i);
    if (_r3.i == 0) goto label51;
    XMLVM_SOURCE_POSITION("Character.java", 2577)
    _r0.i = _r0.i + 1;
    if (_r0.i >= _r1.i) goto label51;
    XMLVM_SOURCE_POSITION("Character.java", 2578)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r0.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 2579)
    _r3.i = java_lang_Character_isLowSurrogate___char(_r3.i);
    if (_r3.i != 0) goto label51;
    XMLVM_SOURCE_POSITION("Character.java", 2580)
    _r2.i = _r2.i + 1;
    label51:;
    XMLVM_SOURCE_POSITION("Character.java", 2584)
    _r2.i = _r2.i + 1;
    _r0.i = _r0.i + 1;
    goto label26;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_offsetByCodePoints___java_lang_CharSequence_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_offsetByCodePoints___java_lang_CharSequence_int_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "offsetByCodePoints", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    _r4.o = n1;
    _r5.i = n2;
    _r6.i = n3;
    XMLVM_SOURCE_POSITION("Character.java", 2614)
    if (_r4.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Character.java", 2615)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 2617)
    XMLVM_CHECK_NPE(4)
    _r0.i = (*(JAVA_INT (*)(JAVA_OBJECT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_length__])(_r4.o);
    XMLVM_SOURCE_POSITION("Character.java", 2618)
    if (_r5.i < 0) goto label16;
    if (_r5.i <= _r0.i) goto label22;
    label16:;
    XMLVM_SOURCE_POSITION("Character.java", 2619)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label22:;
    XMLVM_SOURCE_POSITION("Character.java", 2622)
    if (_r6.i != 0) goto label26;
    _r0 = _r5;
    label25:;
    XMLVM_SOURCE_POSITION("Character.java", 2623)
    XMLVM_SOURCE_POSITION("Character.java", 2661)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label26:;
    XMLVM_SOURCE_POSITION("Character.java", 2626)
    if (_r6.i <= 0) goto label73;
    _r1 = _r5;
    _r2 = _r6;
    label30:;
    XMLVM_SOURCE_POSITION("Character.java", 2629)
    if (_r2.i > 0) goto label34;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Character.java", 2642)
    goto label25;
    label34:;
    XMLVM_SOURCE_POSITION("Character.java", 2630)
    _r2.i = _r2.i + -1;
    if (_r1.i < _r0.i) goto label44;
    XMLVM_SOURCE_POSITION("Character.java", 2631)
    XMLVM_SOURCE_POSITION("Character.java", 2632)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label44:;
    XMLVM_SOURCE_POSITION("Character.java", 2634)
    XMLVM_CHECK_NPE(4)
    _r3.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int])(_r4.o, _r1.i);
    _r3.i = java_lang_Character_isHighSurrogate___char(_r3.i);
    if (_r3.i == 0) goto label70;
    XMLVM_SOURCE_POSITION("Character.java", 2635)
    _r3.i = _r1.i + 1;
    if (_r3.i >= _r0.i) goto label70;
    XMLVM_SOURCE_POSITION("Character.java", 2636)
    XMLVM_CHECK_NPE(4)
    _r3.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int])(_r4.o, _r3.i);
    _r3.i = java_lang_Character_isLowSurrogate___char(_r3.i);
    if (_r3.i == 0) goto label70;
    XMLVM_SOURCE_POSITION("Character.java", 2637)
    _r1.i = _r1.i + 1;
    label70:;
    XMLVM_SOURCE_POSITION("Character.java", 2640)
    _r1.i = _r1.i + 1;
    goto label30;
    label73:;
    XMLVM_SOURCE_POSITION("Character.java", 2645)
    _r0.i = java_lang_Character_GET__assertionsDisabled();
    if (_r0.i != 0) goto label85;
    if (_r6.i < 0) goto label85;

    
    // Red class access removed: java.lang.AssertionError::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.AssertionError::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label85:;
    XMLVM_SOURCE_POSITION("Character.java", 2646)
    _r0.i = -_r6.i;
    _r1 = _r0;
    _r0 = _r5;
    label88:;
    XMLVM_SOURCE_POSITION("Character.java", 2648)
    if (_r1.i <= 0) goto label25;
    XMLVM_SOURCE_POSITION("Character.java", 2649)
    _r1.i = _r1.i + -1;
    _r0.i = _r0.i + -1;
    if (_r0.i >= 0) goto label102;
    XMLVM_SOURCE_POSITION("Character.java", 2651)
    XMLVM_SOURCE_POSITION("Character.java", 2652)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label102:;
    XMLVM_SOURCE_POSITION("Character.java", 2654)
    XMLVM_CHECK_NPE(4)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int])(_r4.o, _r0.i);
    _r2.i = java_lang_Character_isLowSurrogate___char(_r2.i);
    if (_r2.i == 0) goto label88;
    XMLVM_SOURCE_POSITION("Character.java", 2655)
    _r2.i = 1;
    _r2.i = _r0.i - _r2.i;
    if (_r2.i < 0) goto label88;
    XMLVM_SOURCE_POSITION("Character.java", 2656)
    XMLVM_CHECK_NPE(4)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) *(((java_lang_Object*)_r4.o)->tib->itableBegin)[XMLVM_ITABLE_IDX_java_lang_CharSequence_charAt___int])(_r4.o, _r2.i);
    _r2.i = java_lang_Character_isHighSurrogate___char(_r2.i);
    if (_r2.i == 0) goto label88;
    XMLVM_SOURCE_POSITION("Character.java", 2657)
    _r0.i = _r0.i + -1;
    goto label88;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_offsetByCodePoints___char_1ARRAY_int_int_int_int(JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3, JAVA_INT n4, JAVA_INT n5)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_offsetByCodePoints___char_1ARRAY_int_int_int_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "offsetByCodePoints", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r4.o = n1;
    _r5.i = n2;
    _r6.i = n3;
    _r7.i = n4;
    _r8.i = n5;
    XMLVM_SOURCE_POSITION("Character.java", 2698)
    if (_r4.o != JAVA_NULL) goto label8;
    XMLVM_SOURCE_POSITION("Character.java", 2699)
    _r0.o = __NEW_java_lang_NullPointerException();
    XMLVM_CHECK_NPE(0)
    java_lang_NullPointerException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 2701)
    _r0.i = _r5.i + _r6.i;
    if (_r5.i < 0) goto label21;
    XMLVM_SOURCE_POSITION("Character.java", 2702)
    if (_r6.i < 0) goto label21;
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r4.o));
    if (_r0.i > _r1.i) goto label21;
    if (_r7.i < _r5.i) goto label21;
    XMLVM_SOURCE_POSITION("Character.java", 2703)
    if (_r7.i <= _r0.i) goto label27;
    label21:;
    XMLVM_SOURCE_POSITION("Character.java", 2704)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label27:;
    XMLVM_SOURCE_POSITION("Character.java", 2707)
    if (_r8.i != 0) goto label31;
    _r0 = _r7;
    label30:;
    XMLVM_SOURCE_POSITION("Character.java", 2708)
    XMLVM_SOURCE_POSITION("Character.java", 2746)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label31:;
    XMLVM_SOURCE_POSITION("Character.java", 2711)
    if (_r8.i <= 0) goto label74;
    _r1 = _r7;
    _r2 = _r8;
    label35:;
    XMLVM_SOURCE_POSITION("Character.java", 2714)
    if (_r2.i > 0) goto label39;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Character.java", 2727)
    goto label30;
    label39:;
    XMLVM_SOURCE_POSITION("Character.java", 2715)
    _r2.i = _r2.i + -1;
    if (_r1.i < _r0.i) goto label49;
    XMLVM_SOURCE_POSITION("Character.java", 2716)
    XMLVM_SOURCE_POSITION("Character.java", 2717)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label49:;
    XMLVM_SOURCE_POSITION("Character.java", 2719)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r1.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r3.i = java_lang_Character_isHighSurrogate___char(_r3.i);
    if (_r3.i == 0) goto label71;
    XMLVM_SOURCE_POSITION("Character.java", 2720)
    _r3.i = _r1.i + 1;
    if (_r3.i >= _r0.i) goto label71;
    XMLVM_SOURCE_POSITION("Character.java", 2721)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r3.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    _r3.i = java_lang_Character_isLowSurrogate___char(_r3.i);
    if (_r3.i == 0) goto label71;
    XMLVM_SOURCE_POSITION("Character.java", 2722)
    _r1.i = _r1.i + 1;
    label71:;
    XMLVM_SOURCE_POSITION("Character.java", 2725)
    _r1.i = _r1.i + 1;
    goto label35;
    label74:;
    XMLVM_SOURCE_POSITION("Character.java", 2730)
    _r0.i = java_lang_Character_GET__assertionsDisabled();
    if (_r0.i != 0) goto label86;
    if (_r8.i < 0) goto label86;

    
    // Red class access removed: java.lang.AssertionError::new-instance
    XMLVM_RED_CLASS_DEPENDENCY();

    
    // Red class access removed: java.lang.AssertionError::<init>
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_THROW_CUSTOM(_r0.o)
    label86:;
    XMLVM_SOURCE_POSITION("Character.java", 2731)
    _r0.i = -_r8.i;
    _r1 = _r0;
    _r0 = _r7;
    label89:;
    XMLVM_SOURCE_POSITION("Character.java", 2733)
    if (_r1.i <= 0) goto label30;
    XMLVM_SOURCE_POSITION("Character.java", 2734)
    _r1.i = _r1.i + -1;
    _r0.i = _r0.i + -1;
    if (_r0.i >= _r5.i) goto label103;
    XMLVM_SOURCE_POSITION("Character.java", 2736)
    XMLVM_SOURCE_POSITION("Character.java", 2737)
    _r0.o = __NEW_java_lang_IndexOutOfBoundsException();
    XMLVM_CHECK_NPE(0)
    java_lang_IndexOutOfBoundsException___INIT___(_r0.o);
    XMLVM_THROW_CUSTOM(_r0.o)
    label103:;
    XMLVM_SOURCE_POSITION("Character.java", 2739)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r0.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    _r2.i = java_lang_Character_isLowSurrogate___char(_r2.i);
    if (_r2.i == 0) goto label89;
    XMLVM_SOURCE_POSITION("Character.java", 2740)
    _r2.i = 1;
    _r2.i = _r0.i - _r2.i;
    if (_r2.i < _r5.i) goto label89;
    XMLVM_SOURCE_POSITION("Character.java", 2741)
    XMLVM_CHECK_NPE(4)
    XMLVM_CHECK_ARRAY_BOUNDS(_r4.o, _r2.i);
    _r2.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r4.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    _r2.i = java_lang_Character_isHighSurrogate___char(_r2.i);
    if (_r2.i == 0) goto label89;
    XMLVM_SOURCE_POSITION("Character.java", 2742)
    _r0.i = _r0.i + -1;
    goto label89;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_digit___char_int(JAVA_CHAR n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_digit___char_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "digit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r4.i = n1;
    _r5.i = n2;
    _r1.i = 48;
    _r3.i = -1;
    XMLVM_SOURCE_POSITION("Character.java", 2762)
    _r0.i = 2;
    if (_r5.i < _r0.i) goto label84;
    _r0.i = 36;
    if (_r5.i > _r0.i) goto label84;
    XMLVM_SOURCE_POSITION("Character.java", 2763)
    _r0.i = 128;
    if (_r4.i >= _r0.i) goto label53;
    XMLVM_SOURCE_POSITION("Character.java", 2766)
    if (_r1.i > _r4.i) goto label25;
    _r0.i = 57;
    if (_r4.i > _r0.i) goto label25;
    XMLVM_SOURCE_POSITION("Character.java", 2767)
    _r0.i = _r4.i - _r1.i;
    label22:;
    XMLVM_SOURCE_POSITION("Character.java", 2773)
    if (_r0.i >= _r5.i) goto label51;
    label24:;
    XMLVM_SOURCE_POSITION("Character.java", 2784)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label25:;
    XMLVM_SOURCE_POSITION("Character.java", 2768)
    _r0.i = 97;
    if (_r0.i > _r4.i) goto label38;
    _r0.i = 122;
    if (_r4.i > _r0.i) goto label38;
    XMLVM_SOURCE_POSITION("Character.java", 2769)
    _r0.i = 87;
    _r0.i = _r4.i - _r0.i;
    goto label22;
    label38:;
    XMLVM_SOURCE_POSITION("Character.java", 2770)
    _r0.i = 65;
    if (_r0.i > _r4.i) goto label86;
    _r0.i = 90;
    if (_r4.i > _r0.i) goto label86;
    XMLVM_SOURCE_POSITION("Character.java", 2771)
    _r0.i = 55;
    _r0.i = _r4.i - _r0.i;
    goto label22;
    label51:;
    _r0 = _r3;
    goto label24;
    label53:;
    XMLVM_SOURCE_POSITION("Character.java", 2775)
    // "0Aa\3140\3360\4546\4746\5146\5346\5546\5747\6146\6346\6546\7120\7320\7440\10100\11551\13740\14020\177420\177441\177501"
    _r0.o = xmlvm_create_java_string_from_pool(149);

    
    // Red class access removed: org.apache.harmony.luni.util.BinarySearch::binarySearchRange
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Character.java", 2776)
    if (_r0.i < 0) goto label84;
    _r1.o = java_lang_Character_GET_digitValues();
    _r2.i = _r0.i * 2;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    if (_r4.i > _r1.i) goto label84;
    XMLVM_SOURCE_POSITION("Character.java", 2777)
    _r1.o = java_lang_Character_GET_digitValues();
    _r0.i = _r0.i * 2;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    _r0.i = _r4.i - _r0.i;
    _r0.i = _r0.i & 0xffff;
    if (_r0.i < _r5.i) goto label24;
    XMLVM_SOURCE_POSITION("Character.java", 2778)
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Character.java", 2779)
    goto label24;
    label84:;
    _r0 = _r3;
    goto label24;
    label86:;
    _r0 = _r3;
    goto label22;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_digit___int_int(JAVA_INT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_digit___int_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "digit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.i = n1;
    _r2.i = n2;
    XMLVM_SOURCE_POSITION("Character.java", 2801)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::digit
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_equals___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Character", "equals", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.o = me;
    _r3.o = n1;
    XMLVM_SOURCE_POSITION("Character.java", 2816)
    XMLVM_CLASS_INIT(java_lang_Character)
    _r0.i = XMLVM_ISA(_r3.o, __CLASS_java_lang_Character);
    if (_r0.i == 0) goto label14;
    XMLVM_SOURCE_POSITION("Character.java", 2817)
    XMLVM_CHECK_NPE(2)
    _r0.i = ((java_lang_Character*) _r2.o)->fields.java_lang_Character.value_;
    _r3.o = _r3.o;
    XMLVM_CHECK_NPE(3)
    _r1.i = ((java_lang_Character*) _r3.o)->fields.java_lang_Character.value_;
    if (_r0.i != _r1.i) goto label14;
    _r0.i = 1;
    label13:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label14:;
    _r0.i = 0;
    goto label13;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_lang_Character_forDigit___int_int(JAVA_INT n1, JAVA_INT n2)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_forDigit___int_int]
    XMLVM_ENTER_METHOD("java.lang.Character", "forDigit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r2.i = n1;
    _r3.i = n2;
    _r1.i = 10;
    XMLVM_SOURCE_POSITION("Character.java", 2835)
    _r0.i = 2;
    if (_r0.i > _r3.i) goto label23;
    _r0.i = 36;
    if (_r3.i > _r0.i) goto label23;
    XMLVM_SOURCE_POSITION("Character.java", 2836)
    if (_r2.i < 0) goto label23;
    if (_r2.i >= _r3.i) goto label23;
    XMLVM_SOURCE_POSITION("Character.java", 2837)
    if (_r2.i >= _r1.i) goto label19;
    _r0.i = _r2.i + 48;
    label17:;
    _r0.i = _r0.i & 0xffff;
    label18:;
    XMLVM_SOURCE_POSITION("Character.java", 2840)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label19:;
    _r0.i = _r2.i + 97;
    _r0.i = _r0.i - _r1.i;
    goto label17;
    label23:;
    _r0.i = 0;
    goto label18;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_getNumericValue___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_getNumericValue___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "getNumericValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.i = n1;
    _r1.i = 48;
    _r3.i = -1;
    XMLVM_SOURCE_POSITION("Character.java", 2853)
    _r0.i = 128;
    if (_r4.i >= _r0.i) goto label44;
    XMLVM_SOURCE_POSITION("Character.java", 2855)
    if (_r4.i < _r1.i) goto label16;
    _r0.i = 57;
    if (_r4.i > _r0.i) goto label16;
    XMLVM_SOURCE_POSITION("Character.java", 2856)
    _r0.i = _r4.i - _r1.i;
    label15:;
    XMLVM_SOURCE_POSITION("Character.java", 2878)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    XMLVM_SOURCE_POSITION("Character.java", 2858)
    _r0.i = 97;
    if (_r4.i < _r0.i) goto label29;
    _r0.i = 122;
    if (_r4.i > _r0.i) goto label29;
    XMLVM_SOURCE_POSITION("Character.java", 2859)
    _r0.i = 87;
    _r0.i = _r4.i - _r0.i;
    goto label15;
    label29:;
    XMLVM_SOURCE_POSITION("Character.java", 2861)
    _r0.i = 65;
    if (_r4.i < _r0.i) goto label42;
    _r0.i = 90;
    if (_r4.i > _r0.i) goto label42;
    XMLVM_SOURCE_POSITION("Character.java", 2862)
    _r0.i = 55;
    _r0.i = _r4.i - _r0.i;
    goto label15;
    label42:;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Character.java", 2864)
    goto label15;
    label44:;
    XMLVM_SOURCE_POSITION("Character.java", 2866)
    // "0Aa\262\271\274\3140\3360\4546\4746\4764\4771\5146\5346\5546\5747\5761\5762\6146\6346\6546\7120\7320\7440\10100\11551\11563\11564\11565\11566\11567\11570\11571\11572\11573\11574\13356\13740\14020\20160\20164\20200\20523\20537\20540\20554\20555\20556\20557\20560\20574\20575\20576\20577\20600\20601\20602\22140\22164\22210\22352\23566\23600\23612\30007\30041\30070\30071\30072\31200\177420\177441\177501"
    _r0.o = xmlvm_create_java_string_from_pool(151);

    
    // Red class access removed: org.apache.harmony.luni.util.BinarySearch::binarySearchRange
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Character.java", 2867)
    if (_r0.i < 0) goto label81;
    _r1.o = java_lang_Character_GET_numericValues();
    _r2.i = _r0.i * 2;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    if (_r4.i > _r1.i) goto label81;
    XMLVM_SOURCE_POSITION("Character.java", 2868)
    _r1.o = java_lang_Character_GET_numericValues();
    _r0.i = _r0.i * 2;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 2869)
    if (_r0.i != 0) goto label72;
    XMLVM_SOURCE_POSITION("Character.java", 2870)
    _r0.i = -2;
    goto label15;
    label72:;
    XMLVM_SOURCE_POSITION("Character.java", 2873)
    if (_r0.i <= _r4.i) goto label78;
    XMLVM_SOURCE_POSITION("Character.java", 2874)
    _r0.i = (_r0.i << 16) >> 16;
    _r0.i = _r4.i - _r0.i;
    goto label15;
    label78:;
    XMLVM_SOURCE_POSITION("Character.java", 2876)
    _r0.i = _r4.i - _r0.i;
    goto label15;
    label81:;
    _r0 = _r3;
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_getNumericValue___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_getNumericValue___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "getNumericValue", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 2894)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::getNumericValue
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_getType___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_getType___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "getType", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 2905)
    _r0.i = 1000;
    if (_r3.i >= _r0.i) goto label9;
    XMLVM_SOURCE_POSITION("Character.java", 2906)
    _r0.o = java_lang_Character_GET_typeValuesCache();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r3.i);
    _r0.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r3.i];
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 2917)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label9:;
    XMLVM_SOURCE_POSITION("Character.java", 2908)
    // "\000 \042$&(*-/1:<?A[]_a\173\175\177\240\242\246\250\252\254\256\261\263\265\267\271\273\275\277\301\327\331\337\367\371\400\470\511\571\577\601\603\607\612\614\616\622\624\627\631\634\636\640\647\653\657\662\664\670\672\674\676\700\704\706\710\712\714\735\760\762\764\767\771\1042\1120\1260\1271\1273\1302\1320\1322\1340\1345\1356\1400\1540\1564\1572\1576\1604\1606\1611\1614\1616\1620\1622\1643\1654\1720\1722\1725\1732\1760\2000\2060\2140\2202\2204\2210\2214\2301\2307\2313\2320\2370\2461\2531\2533\2541\2611\2621\2643\2673\2676\2702\2720\2760\2763\3014\3033\3037\3041\3100\3102\3113\3140\3152\3160\3162\3324\3326\3335\3337\3345\3347\3351\3353\3360\3372\3375\3400\3417\3421\3423\3460\3600\3646\4401\4403\4405\4474\4476\4501\4511\4515\4520\4522\4530\4542\4544\4546\4560\4601\4603\4605\4617\4623\4652\4662\4666\4674\4676\4701\4707\4713\4715\4727\4734\4737\4742\4746\4760\4762\4764\4772\5002\5005\5017\5023\5052\5062\5065\5070\5074\5076\5101\5107\5113\5131\5136\5146\5160\5162\5201\5203\5205\5215\5217\5223\5252\5262\5265\5274\5276\5301\5307\5311\5313\5315\5320\5340\5346\5401\5403\5405\5417\5423\5452\5462\5466\5474\5476\5502\5507\5513\5515\5526\5534\5537\5546\5560\5602\5605\5616\5622\5631\5634\5636\5643\5650\5656\5667\5676\5700\5702\5706\5712\5715\5727\5747\5760\6001\6005\6016\6022\6052\6065\6076\6101\6106\6112\6125\6140\6146\6202\6205\6216\6222\6252\6265\6276\6301\6306\6310\6312\6314\6325\6336\6340\6346\6402\6405\6416\6422\6452\6476\6501\6506\6512\6515\6527\6540\6546\6602\6605\6632\6663\6675\6700\6712\6717\6722\6726\6730\6762\6764\7001\7061\7063\7065\7077\7101\7106\7110\7117\7121\7132\7201\7204\7207\7212\7215\7224\7231\7241\7245\7247\7252\7255\7261\7263\7265\7273\7275\7300\7306\7310\7320\7334\7400\7402\7404\7423\7430\7432\7440\7452\7464\7472\7476\7500\7511\7561\7577\7601\7605\7607\7611\7620\7631\7676\7706\7710\7717\10000\10043\10051\10054\10056\10061\10066\10070\10100\10112\10120\10126\10130\10240\10320\10373\10400\10537\10650\11000\11010\11110\11112\11120\11130\11132\11140\11210\11212\11220\11260\11262\11270\11300\11302\11310\11320\11330\11360\11420\11422\11430\11440\11510\11541\11551\11562\11640\12001\13155\13157\13200\13202\13233\13240\13353\13356\13600\13664\13667\13676\13706\13710\13712\13724\13733\13740\14000\14006\14010\14013\14020\14040\14103\14105\14200\14251\17000\17226\17240\17400\17410\17420\17430\17440\17450\17460\17470\17500\17510\17520\17531\17533\17535\17537\17541\17550\17560\17600\17610\17620\17630\17640\17650\17660\17666\17670\17674\17676\17700\17702\17706\17710\17714\17716\17720\17726\17730\17735\17740\17750\17755\17762\17766\17770\17774\17776\20000\20014\20020\20026\20030\20032\20034\20036\20040\20050\20052\20057\20061\20071\20073\20077\20101\20104\20106\20110\20152\20160\20164\20172\20175\20177\20201\20212\20215\20240\20320\20335\20341\20343\20400\20402\20404\20407\20411\20413\20416\20420\20423\20425\20427\20431\20436\20444\20453\20456\20460\20462\20464\20466\20471\20523\20540\20620\20625\20632\20634\20640\20642\20645\20650\20656\20660\20716\20720\20722\20726\21000\21400\21410\21414\21440\21442\21451\21453\21575\22000\22100\22140\22234\22352\22400\22640\22667\22671\22701\22703\23000\23031\23157\23161\23401\23406\23414\23451\23515\23517\23526\23530\23541\23566\23624\23630\23661\24000\27200\27233\27400\27760\30000\30002\30004\30006\30010\30022\30024\30034\30036\30040\30042\30052\30060\30062\30066\30070\30076\30101\30231\30233\30235\30241\30373\30375\30405\30461\30620\30622\30626\30640\31000\31040\31052\31140\31177\31201\31212\31300\31320\31400\31573\31740\32000\47000\120000\122220\122244\122265\122302\122306\126000\154000\160000\174400\175400\175423\175435\175440\175451\175453\175470\175476\175500\175503\175506\175723\176476\176520\176622\176760\177040\177060\177062\177064\177066\177111\177115\177120\177124\177130\177132\177137\177142\177145\177150\177153\177160\177164\177166\177377\177401\177404\177406\177410\177412\177415\177417\177421\177432\177434\177437\177441\177473\177475\177477\177501\177533\177535\177541\177543\177545\177547\177560\177562\177636\177640\177702\177712\177722\177732\177740\177742\177744\177746\177750\177752\177755\177771\177774"
    _r0.o = xmlvm_create_java_string_from_pool(146);

    
    // Red class access removed: org.apache.harmony.luni.util.BinarySearch::binarySearchRange
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Character.java", 2909)
    _r1.o = java_lang_Character_GET_typeValues();
    _r2.i = _r0.i * 2;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("Character.java", 2910)
    if (_r3.i > _r1.i) goto label46;
    XMLVM_SOURCE_POSITION("Character.java", 2911)
    _r1.o = java_lang_Character_GET_typeValues();
    _r0.i = _r0.i * 2;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 2912)
    _r1.i = 256;
    if (_r0.i < _r1.i) goto label8;
    XMLVM_SOURCE_POSITION("Character.java", 2915)
    _r1.i = _r3.i & 1;
    _r2.i = 1;
    if (_r1.i != _r2.i) goto label43;
    _r0.i = _r0.i >> 8;
    goto label8;
    label43:;
    _r0.i = _r0.i & 255;
    goto label8;
    label46:;
    _r0.i = 0;
    goto label8;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_getType___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_getType___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "getType", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 2928)
    _r0.i = 1000;
    if (_r2.i >= _r0.i) goto label11;
    if (_r2.i <= 0) goto label11;
    XMLVM_SOURCE_POSITION("Character.java", 2929)
    _r0.o = java_lang_Character_GET_typeValuesCache();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r2.i);
    _r0.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    label10:;
    XMLVM_SOURCE_POSITION("Character.java", 2938)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label11:;
    XMLVM_SOURCE_POSITION("Character.java", 2931)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::getType
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Character.java", 2935)
    _r1.i = 16;
    if (_r0.i <= _r1.i) goto label10;
    _r0.i = _r0.i + 1;
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_lang_Character_getDirectionality___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_getDirectionality___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "getDirectionality", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.i = n1;
    _r3.i = 1;
    XMLVM_SOURCE_POSITION("Character.java", 2949)
    // "\000\011\014\016\034\037!#&+/1:<A[a\173\177\205\207\240\242\246\252\254\260\262\264\267\271\273\300\327\331\367\371\1042\1120\1260\1271\1273\1302\1320\1322\1340\1345\1356\1400\1540\1564\1572\1576\1604\1606\1611\1614\1616\1643\1720\1732\2000\2203\2210\2214\2307\2313\2320\2370\2461\2531\2541\2611\2621\2643\2673\2676\2702\2720\2760\3014\3033\3037\3041\3100\3113\3140\3152\3154\3160\3162\3326\3345\3347\3351\3353\3360\3372\3400\3417\3421\3423\3460\3600\3646\4401\4403\4405\4474\4476\4501\4511\4515\4520\4522\4530\4542\4544\4601\4603\4605\4617\4623\4652\4662\4666\4674\4676\4701\4707\4713\4715\4727\4734\4737\4742\4746\4762\4764\5002\5005\5017\5023\5052\5062\5065\5070\5074\5076\5101\5107\5113\5131\5136\5146\5160\5162\5201\5203\5205\5215\5217\5223\5252\5262\5265\5274\5276\5301\5307\5311\5313\5315\5320\5340\5346\5401\5403\5405\5417\5423\5452\5462\5466\5474\5476\5502\5507\5513\5515\5526\5534\5537\5546\5602\5605\5616\5622\5631\5634\5636\5643\5650\5656\5667\5676\5700\5702\5706\5712\5715\5727\5747\6001\6005\6016\6022\6052\6065\6076\6101\6106\6112\6125\6140\6146\6202\6205\6216\6222\6252\6265\6276\6301\6306\6310\6312\6314\6325\6336\6340\6346\6402\6405\6416\6422\6452\6476\6501\6506\6512\6515\6527\6540\6546\6602\6605\6632\6663\6675\6700\6712\6717\6722\6726\6730\6762\7001\7061\7063\7065\7077\7101\7107\7117\7201\7204\7207\7212\7215\7224\7231\7241\7245\7247\7252\7255\7261\7263\7265\7273\7275\7300\7306\7310\7320\7334\7400\7430\7432\7465\7472\7476\7511\7561\7577\7601\7605\7607\7611\7620\7631\7676\7706\7710\7717\10000\10043\10051\10054\10056\10061\10066\10070\10100\10130\10240\10320\10373\10400\10537\10650\11000\11010\11110\11112\11120\11130\11132\11140\11210\11212\11220\11260\11262\11270\11300\11302\11310\11320\11330\11360\11420\11422\11430\11440\11510\11541\11640\12001\13200\13202\13233\13240\13600\13667\13676\13706\13710\13712\13724\13733\13740\14000\14013\14020\14040\14200\14251\17000\17240\17400\17430\17440\17510\17520\17531\17533\17535\17537\17600\17666\17675\17700\17702\17706\17715\17720\17726\17735\17740\17755\17762\17766\17775\20000\20013\20016\20020\20050\20052\20054\20056\20060\20065\20110\20152\20160\20164\20172\20174\20177\20201\20212\20214\20240\20320\20400\20402\20404\20407\20411\20413\20424\20427\20431\20436\20444\20453\20456\20460\20462\20464\20472\20523\20540\20620\21000\21022\21024\21400\21466\21573\21575\21625\21627\22000\22100\22140\22234\22352\22400\22640\23000\23031\23401\23406\23414\23451\23515\23517\23526\23530\23541\23566\23630\23661\24000\27200\27233\27400\27760\30000\30002\30005\30010\30041\30052\30060\30062\30066\30070\30076\30101\30231\30233\30235\30241\30373\30375\30405\30461\30620\31000\31040\31140\31177\31300\31320\31400\31573\31740\32000\47000\120000\122220\122244\122265\122302\122306\126000\154000\175400\175423\175435\175440\175451\175453\175470\175476\175500\175503\175506\175520\175723\176476\176520\176622\176760\177040\177060\177111\177120\177124\177127\177137\177141\177143\177145\177150\177152\177160\177164\177166\177377\177401\177403\177406\177413\177417\177421\177432\177434\177441\177473\177501\177533\177541\177546\177702\177712\177722\177732\177740\177742\177745\177750\177771\177774"
    _r0.o = xmlvm_create_java_string_from_pool(145);

    
    // Red class access removed: org.apache.harmony.luni.util.BinarySearch::binarySearchRange
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Character.java", 2950)
    _r1.o = java_lang_Character_GET_bidiValues();
    _r2.i = _r0.i * 2;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r2.i);
    _r1.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r2.i];
    XMLVM_SOURCE_POSITION("Character.java", 2951)
    if (_r4.i > _r1.i) goto label42;
    XMLVM_SOURCE_POSITION("Character.java", 2952)
    _r1.o = java_lang_Character_GET_bidiValues();
    _r0.i = _r0.i * 2;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 2953)
    _r1.i = 256;
    if (_r0.i >= _r1.i) goto label30;
    XMLVM_SOURCE_POSITION("Character.java", 2954)
    _r0.i = _r0.i - _r3.i;
    _r0.i = (_r0.i << 24) >> 24;
    label29:;
    XMLVM_SOURCE_POSITION("Character.java", 2958)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label30:;
    XMLVM_SOURCE_POSITION("Character.java", 2956)
    _r1.i = _r4.i & 1;
    if (_r1.i != _r3.i) goto label39;
    _r0.i = _r0.i >> 8;
    label36:;
    _r0.i = _r0.i - _r3.i;
    _r0.i = (_r0.i << 24) >> 24;
    goto label29;
    label39:;
    _r0.i = _r0.i & 255;
    goto label36;
    label42:;
    _r0.i = -1;
    goto label29;
    //XMLVM_END_WRAPPER
}

JAVA_BYTE java_lang_Character_getDirectionality___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_getDirectionality___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "getDirectionality", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.i = n1;
    _r1.i = -1;
    XMLVM_SOURCE_POSITION("Character.java", 2969)
    _r0.i = java_lang_Character_getType___int(_r2.i);
    if (_r0.i != 0) goto label9;
    _r0 = _r1;
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 2970)
    XMLVM_SOURCE_POSITION("Character.java", 2977)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label9:;
    XMLVM_SOURCE_POSITION("Character.java", 2973)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::getDirectionality
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Character.java", 2974)
    if (_r0.i != _r1.i) goto label17;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Character.java", 2975)
    goto label8;
    label17:;
    _r1.o = java_lang_Character_GET_DIRECTIONALITY();
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_BYTE*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    goto label8;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isMirrored___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isMirrored___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isMirrored", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.i = n1;
    _r4.i = 1;
    _r3.i = 0;
    XMLVM_SOURCE_POSITION("Character.java", 2989)
    _r0.i = _r5.i / 16;
    XMLVM_SOURCE_POSITION("Character.java", 2990)
    _r1.o = java_lang_Character_GET_mirrored();
    _r1.i = XMLVMArray_count(((org_xmlvm_runtime_XMLVMArray*) _r1.o));
    if (_r0.i < _r1.i) goto label11;
    _r0 = _r3;
    label10:;
    XMLVM_SOURCE_POSITION("Character.java", 2991)
    XMLVM_SOURCE_POSITION("Character.java", 2994)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label11:;
    XMLVM_SOURCE_POSITION("Character.java", 2993)
    _r1.i = _r5.i % 16;
    _r1.i = _r4.i << _r1.i;
    _r2.o = java_lang_Character_GET_mirrored();
    XMLVM_CHECK_NPE(2)
    XMLVM_CHECK_ARRAY_BOUNDS(_r2.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r2.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    _r0.i = _r0.i & _r1.i;
    if (_r0.i == 0) goto label24;
    _r0 = _r4;
    goto label10;
    label24:;
    _r0 = _r3;
    goto label10;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isMirrored___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isMirrored___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isMirrored", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3006)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isMirrored
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_hashCode__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_hashCode__]
    XMLVM_ENTER_METHOD("java.lang.Character", "hashCode", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Character.java", 3011)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Character*) _r1.o)->fields.java_lang_Character.value_;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isDefined___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isDefined___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isDefined", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3024)
    _r0.i = java_lang_Character_getType___char(_r1.i);
    if (_r0.i == 0) goto label8;
    _r0.i = 1;
    label7:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    _r0.i = 0;
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isDefined___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isDefined___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isDefined", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3037)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isDefined
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isDigit___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isDigit___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isDigit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.i = n1;
    _r3.i = 1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Character.java", 3050)
    _r0.i = 48;
    if (_r0.i > _r4.i) goto label12;
    _r0.i = 57;
    if (_r4.i > _r0.i) goto label12;
    _r0 = _r3;
    label11:;
    XMLVM_SOURCE_POSITION("Character.java", 3051)
    XMLVM_SOURCE_POSITION("Character.java", 3056)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    XMLVM_SOURCE_POSITION("Character.java", 3053)
    _r0.i = 1632;
    if (_r4.i >= _r0.i) goto label18;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Character.java", 3054)
    goto label11;
    label18:;
    _r0.i = java_lang_Character_getType___char(_r4.i);
    _r1.i = 9;
    if (_r0.i != _r1.i) goto label28;
    _r0 = _r3;
    goto label11;
    label28:;
    _r0 = _r2;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isDigit___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isDigit___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isDigit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3068)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isDigit
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isIdentifierIgnorable___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isIdentifierIgnorable___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isIdentifierIgnorable", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3080)
    if (_r2.i < 0) goto label6;
    _r0.i = 8;
    if (_r2.i <= _r0.i) goto label32;
    label6:;
    _r0.i = 14;
    if (_r2.i < _r0.i) goto label14;
    _r0.i = 27;
    if (_r2.i <= _r0.i) goto label32;
    label14:;
    XMLVM_SOURCE_POSITION("Character.java", 3081)
    _r0.i = 127;
    if (_r2.i < _r0.i) goto label22;
    _r0.i = 159;
    if (_r2.i <= _r0.i) goto label32;
    label22:;
    _r0.i = java_lang_Character_getType___char(_r2.i);
    _r1.i = 16;
    if (_r0.i == _r1.i) goto label32;
    _r0.i = 0;
    label31:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label32:;
    _r0.i = 1;
    goto label31;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isIdentifierIgnorable___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isIdentifierIgnorable___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isIdentifierIgnorable", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3094)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isIdentifierIgnorable
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isISOControl___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isISOControl___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isISOControl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3106)
    _r0.i = java_lang_Character_isISOControl___int(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isISOControl___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isISOControl___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isISOControl", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3118)
    if (_r1.i < 0) goto label6;
    _r0.i = 31;
    if (_r1.i <= _r0.i) goto label16;
    label6:;
    _r0.i = 127;
    if (_r1.i < _r0.i) goto label14;
    _r0.i = 159;
    if (_r1.i <= _r0.i) goto label16;
    label14:;
    _r0.i = 0;
    label15:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    _r0.i = 1;
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isJavaIdentifierPart___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isJavaIdentifierPart___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isJavaIdentifierPart", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    _r5.i = n1;
    _r4.i = 128;
    _r3.i = 0;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Character.java", 3132)
    if (_r5.i >= _r4.i) goto label18;
    XMLVM_SOURCE_POSITION("Character.java", 3133)
    _r0.o = java_lang_Character_GET_typeTags();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r5.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r5.i];
    _r0.i = _r0.i & 2;
    if (_r0.i == 0) goto label16;
    _r0 = _r2;
    label15:;
    XMLVM_SOURCE_POSITION("Character.java", 3141)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    _r0 = _r3;
    goto label15;
    label18:;
    XMLVM_SOURCE_POSITION("Character.java", 3136)
    _r0.i = java_lang_Character_getType___char(_r5.i);
    XMLVM_SOURCE_POSITION("Character.java", 3137)
    if (_r0.i < _r2.i) goto label27;
    _r1.i = 5;
    if (_r0.i <= _r1.i) goto label62;
    label27:;
    XMLVM_SOURCE_POSITION("Character.java", 3138)
    _r1.i = 26;
    if (_r0.i == _r1.i) goto label62;
    _r1.i = 23;
    if (_r0.i == _r1.i) goto label62;
    XMLVM_SOURCE_POSITION("Character.java", 3139)
    _r1.i = 9;
    if (_r0.i < _r1.i) goto label43;
    _r1.i = 10;
    if (_r0.i <= _r1.i) goto label62;
    label43:;
    XMLVM_SOURCE_POSITION("Character.java", 3140)
    _r1.i = 6;
    if (_r0.i == _r1.i) goto label62;
    _r1.i = 8;
    if (_r0.i == _r1.i) goto label62;
    if (_r5.i < _r4.i) goto label56;
    _r1.i = 159;
    if (_r5.i <= _r1.i) goto label62;
    label56:;
    _r1.i = 16;
    if (_r0.i == _r1.i) goto label62;
    _r0 = _r3;
    goto label15;
    label62:;
    _r0 = _r2;
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isJavaIdentifierPart___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isJavaIdentifierPart___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isJavaIdentifierPart", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.i = n1;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Character.java", 3154)
    _r0.i = java_lang_Character_getType___int(_r3.i);
    XMLVM_SOURCE_POSITION("Character.java", 3155)
    if (_r0.i < _r2.i) goto label10;
    _r1.i = 5;
    if (_r0.i <= _r1.i) goto label41;
    label10:;
    XMLVM_SOURCE_POSITION("Character.java", 3156)
    _r1.i = 26;
    if (_r0.i == _r1.i) goto label41;
    _r1.i = 23;
    if (_r0.i == _r1.i) goto label41;
    XMLVM_SOURCE_POSITION("Character.java", 3157)
    _r1.i = 9;
    if (_r0.i < _r1.i) goto label26;
    _r1.i = 10;
    if (_r0.i <= _r1.i) goto label41;
    label26:;
    XMLVM_SOURCE_POSITION("Character.java", 3158)
    _r1.i = 8;
    if (_r0.i == _r1.i) goto label41;
    _r1.i = 6;
    if (_r0.i == _r1.i) goto label41;
    XMLVM_SOURCE_POSITION("Character.java", 3159)
    _r0.i = java_lang_Character_isIdentifierIgnorable___int(_r3.i);
    if (_r0.i != 0) goto label41;
    _r0.i = 0;
    label40:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label41:;
    _r0 = _r2;
    goto label40;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isJavaIdentifierStart___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isJavaIdentifierStart___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isJavaIdentifierStart", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.i = n1;
    _r3.i = 0;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Character.java", 3173)
    _r0.i = 128;
    if (_r4.i >= _r0.i) goto label18;
    XMLVM_SOURCE_POSITION("Character.java", 3174)
    _r0.o = java_lang_Character_GET_typeTags();
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r4.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    _r0.i = _r0.i & 1;
    if (_r0.i == 0) goto label16;
    _r0 = _r2;
    label15:;
    XMLVM_SOURCE_POSITION("Character.java", 3180)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    _r0 = _r3;
    goto label15;
    label18:;
    XMLVM_SOURCE_POSITION("Character.java", 3177)
    _r0.i = java_lang_Character_getType___char(_r4.i);
    XMLVM_SOURCE_POSITION("Character.java", 3178)
    if (_r0.i < _r2.i) goto label27;
    _r1.i = 5;
    if (_r0.i <= _r1.i) goto label41;
    label27:;
    XMLVM_SOURCE_POSITION("Character.java", 3179)
    _r1.i = 26;
    if (_r0.i == _r1.i) goto label41;
    _r1.i = 23;
    if (_r0.i == _r1.i) goto label41;
    _r1.i = 10;
    if (_r0.i == _r1.i) goto label41;
    _r0 = _r3;
    goto label15;
    label41:;
    _r0 = _r2;
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isJavaIdentifierStart___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isJavaIdentifierStart___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isJavaIdentifierStart", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3193)
    _r0.i = java_lang_Character_getType___int(_r2.i);
    XMLVM_SOURCE_POSITION("Character.java", 3194)
    _r1.i = java_lang_Character_isLetter___int(_r2.i);
    if (_r1.i != 0) goto label24;
    _r1.i = 26;
    if (_r0.i == _r1.i) goto label24;
    XMLVM_SOURCE_POSITION("Character.java", 3195)
    _r1.i = 23;
    if (_r0.i == _r1.i) goto label24;
    _r1.i = 10;
    if (_r0.i == _r1.i) goto label24;
    _r0.i = 0;
    label23:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label24:;
    _r0.i = 1;
    goto label23;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isJavaLetter___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isJavaLetter___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isJavaLetter", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3209)
    _r0.i = java_lang_Character_isJavaIdentifierStart___char(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isJavaLetterOrDigit___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isJavaLetterOrDigit___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isJavaLetterOrDigit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3224)
    _r0.i = java_lang_Character_isJavaIdentifierPart___char(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isLetter___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isLetter___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isLetter", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.i = n1;
    _r3.i = 0;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Character.java", 3235)
    _r0.i = 65;
    if (_r0.i > _r4.i) goto label10;
    _r0.i = 90;
    if (_r4.i <= _r0.i) goto label18;
    label10:;
    _r0.i = 97;
    if (_r0.i > _r4.i) goto label20;
    _r0.i = 122;
    if (_r4.i > _r0.i) goto label20;
    label18:;
    _r0 = _r2;
    label19:;
    XMLVM_SOURCE_POSITION("Character.java", 3236)
    XMLVM_SOURCE_POSITION("Character.java", 3242)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label20:;
    XMLVM_SOURCE_POSITION("Character.java", 3238)
    _r0.i = 128;
    if (_r4.i >= _r0.i) goto label26;
    _r0 = _r3;
    XMLVM_SOURCE_POSITION("Character.java", 3239)
    goto label19;
    label26:;
    XMLVM_SOURCE_POSITION("Character.java", 3241)
    _r0.i = java_lang_Character_getType___char(_r4.i);
    if (_r0.i < _r2.i) goto label37;
    _r1.i = 5;
    if (_r0.i > _r1.i) goto label37;
    _r0 = _r2;
    goto label19;
    label37:;
    _r0 = _r3;
    goto label19;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isLetter___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isLetter___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isLetter", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3254)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isLetter
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isLetterOrDigit___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isLetterOrDigit___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isLetterOrDigit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.i = n1;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Character.java", 3266)
    _r0.i = java_lang_Character_getType___char(_r3.i);
    XMLVM_SOURCE_POSITION("Character.java", 3267)
    if (_r0.i < _r2.i) goto label10;
    _r1.i = 5;
    if (_r0.i <= _r1.i) goto label16;
    label10:;
    XMLVM_SOURCE_POSITION("Character.java", 3268)
    _r1.i = 9;
    if (_r0.i == _r1.i) goto label16;
    _r0.i = 0;
    label15:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    _r0 = _r2;
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isLetterOrDigit___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isLetterOrDigit___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isLetterOrDigit", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3280)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isLetterOrDigit
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isLowerCase___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isLowerCase___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isLowerCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.i = n1;
    _r3.i = 1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Character.java", 3293)
    _r0.i = 97;
    if (_r0.i > _r4.i) goto label12;
    _r0.i = 122;
    if (_r4.i > _r0.i) goto label12;
    _r0 = _r3;
    label11:;
    XMLVM_SOURCE_POSITION("Character.java", 3294)
    XMLVM_SOURCE_POSITION("Character.java", 3300)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    XMLVM_SOURCE_POSITION("Character.java", 3296)
    _r0.i = 128;
    if (_r4.i >= _r0.i) goto label18;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Character.java", 3297)
    goto label11;
    label18:;
    _r0.i = java_lang_Character_getType___char(_r4.i);
    _r1.i = 2;
    if (_r0.i != _r1.i) goto label27;
    _r0 = _r3;
    goto label11;
    label27:;
    _r0 = _r2;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isLowerCase___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isLowerCase___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isLowerCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3312)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isLowerCase
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isSpace___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isSpace___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isSpace", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3326)
    _r0.i = 10;
    if (_r1.i == _r0.i) goto label22;
    _r0.i = 9;
    if (_r1.i == _r0.i) goto label22;
    _r0.i = 12;
    if (_r1.i == _r0.i) goto label22;
    _r0.i = 13;
    if (_r1.i == _r0.i) goto label22;
    _r0.i = 32;
    if (_r1.i == _r0.i) goto label22;
    _r0.i = 0;
    label21:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label22:;
    _r0.i = 1;
    goto label21;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isSpaceChar___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isSpaceChar___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isSpaceChar", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.i = n1;
    _r2.i = 1;
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Character.java", 3340)
    _r0.i = 32;
    if (_r3.i == _r0.i) goto label14;
    _r0.i = 160;
    if (_r3.i == _r0.i) goto label14;
    _r0.i = 5760;
    if (_r3.i != _r0.i) goto label16;
    label14:;
    _r0 = _r2;
    label15:;
    XMLVM_SOURCE_POSITION("Character.java", 3341)
    XMLVM_SOURCE_POSITION("Character.java", 3347)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    XMLVM_SOURCE_POSITION("Character.java", 3343)
    _r0.i = 8192;
    if (_r3.i >= _r0.i) goto label22;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Character.java", 3344)
    goto label15;
    label22:;
    XMLVM_SOURCE_POSITION("Character.java", 3346)
    _r0.i = 8203;
    if (_r3.i <= _r0.i) goto label44;
    _r0.i = 8232;
    if (_r3.i == _r0.i) goto label44;
    _r0.i = 8233;
    if (_r3.i == _r0.i) goto label44;
    _r0.i = 8239;
    if (_r3.i == _r0.i) goto label44;
    _r0.i = 12288;
    if (_r3.i == _r0.i) goto label44;
    _r0 = _r1;
    goto label15;
    label44:;
    _r0 = _r2;
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isSpaceChar___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isSpaceChar___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isSpaceChar", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3361)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isSpaceChar
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isTitleCase___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isTitleCase___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isTitleCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    _r4.i = n1;
    _r1.i = 8188;
    _r3.i = 1;
    _r2.i = 0;
    XMLVM_SOURCE_POSITION("Character.java", 3373)
    _r0.i = 453;
    if (_r4.i == _r0.i) goto label20;
    _r0.i = 456;
    if (_r4.i == _r0.i) goto label20;
    _r0.i = 459;
    if (_r4.i == _r0.i) goto label20;
    _r0.i = 498;
    if (_r4.i != _r0.i) goto label22;
    label20:;
    _r0 = _r3;
    label21:;
    XMLVM_SOURCE_POSITION("Character.java", 3374)
    XMLVM_SOURCE_POSITION("Character.java", 3384)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label22:;
    XMLVM_SOURCE_POSITION("Character.java", 3376)
    _r0.i = 8072;
    if (_r4.i < _r0.i) goto label60;
    if (_r4.i > _r1.i) goto label60;
    XMLVM_SOURCE_POSITION("Character.java", 3378)
    _r0.i = 8111;
    if (_r4.i <= _r0.i) goto label46;
    XMLVM_SOURCE_POSITION("Character.java", 3379)
    _r0.i = 8124;
    if (_r4.i == _r0.i) goto label44;
    _r0.i = 8140;
    if (_r4.i == _r0.i) goto label44;
    if (_r4.i == _r1.i) goto label44;
    _r0 = _r2;
    goto label21;
    label44:;
    _r0 = _r3;
    goto label21;
    label46:;
    XMLVM_SOURCE_POSITION("Character.java", 3381)
    _r0.i = _r4.i & 15;
    _r1.i = 8;
    if (_r0.i < _r1.i) goto label58;
    XMLVM_SOURCE_POSITION("Character.java", 3382)
    _r1.i = 15;
    if (_r0.i > _r1.i) goto label58;
    _r0 = _r3;
    goto label21;
    label58:;
    _r0 = _r2;
    goto label21;
    label60:;
    _r0 = _r2;
    goto label21;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isTitleCase___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isTitleCase___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isTitleCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3396)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isTitleCase
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isUnicodeIdentifierPart___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isUnicodeIdentifierPart___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isUnicodeIdentifierPart", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.i = n1;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Character.java", 3409)
    _r0.i = java_lang_Character_getType___char(_r3.i);
    XMLVM_SOURCE_POSITION("Character.java", 3410)
    if (_r0.i < _r2.i) goto label10;
    _r1.i = 5;
    if (_r0.i <= _r1.i) goto label37;
    label10:;
    XMLVM_SOURCE_POSITION("Character.java", 3411)
    _r1.i = 23;
    if (_r0.i == _r1.i) goto label37;
    XMLVM_SOURCE_POSITION("Character.java", 3412)
    _r1.i = 9;
    if (_r0.i < _r1.i) goto label22;
    _r1.i = 10;
    if (_r0.i <= _r1.i) goto label37;
    label22:;
    XMLVM_SOURCE_POSITION("Character.java", 3413)
    _r1.i = 6;
    if (_r0.i == _r1.i) goto label37;
    _r1.i = 8;
    if (_r0.i == _r1.i) goto label37;
    XMLVM_SOURCE_POSITION("Character.java", 3414)
    _r0.i = java_lang_Character_isIdentifierIgnorable___char(_r3.i);
    if (_r0.i != 0) goto label37;
    _r0.i = 0;
    label36:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label37:;
    _r0 = _r2;
    goto label36;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isUnicodeIdentifierPart___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isUnicodeIdentifierPart___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isUnicodeIdentifierPart", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3427)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isUnicodeIdentifierPart
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isUnicodeIdentifierStart___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isUnicodeIdentifierStart___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isUnicodeIdentifierStart", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.i = n1;
    _r2.i = 1;
    XMLVM_SOURCE_POSITION("Character.java", 3440)
    _r0.i = java_lang_Character_getType___char(_r3.i);
    XMLVM_SOURCE_POSITION("Character.java", 3441)
    if (_r0.i < _r2.i) goto label10;
    _r1.i = 5;
    if (_r0.i <= _r1.i) goto label16;
    label10:;
    XMLVM_SOURCE_POSITION("Character.java", 3442)
    _r1.i = 10;
    if (_r0.i == _r1.i) goto label16;
    _r0.i = 0;
    label15:;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label16:;
    _r0 = _r2;
    goto label15;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isUnicodeIdentifierStart___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isUnicodeIdentifierStart___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isUnicodeIdentifierStart", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3455)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isUnicodeIdentifierStart
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isUpperCase___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isUpperCase___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isUpperCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.i = n1;
    _r2.i = 0;
    _r1.i = 1;
    XMLVM_SOURCE_POSITION("Character.java", 3468)
    _r0.i = 65;
    if (_r0.i > _r3.i) goto label12;
    _r0.i = 90;
    if (_r3.i > _r0.i) goto label12;
    _r0 = _r1;
    label11:;
    XMLVM_SOURCE_POSITION("Character.java", 3469)
    XMLVM_SOURCE_POSITION("Character.java", 3475)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label12:;
    XMLVM_SOURCE_POSITION("Character.java", 3471)
    _r0.i = 128;
    if (_r3.i >= _r0.i) goto label18;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Character.java", 3472)
    goto label11;
    label18:;
    _r0.i = java_lang_Character_getType___char(_r3.i);
    if (_r0.i != _r1.i) goto label26;
    _r0 = _r1;
    goto label11;
    label26:;
    _r0 = _r2;
    goto label11;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isUpperCase___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isUpperCase___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isUpperCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3487)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isUpperCase
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isWhitespace___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isWhitespace___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "isWhitespace", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    _r3.i = n1;
    _r2.i = 0;
    _r1.i = 1;
    XMLVM_SOURCE_POSITION("Character.java", 3501)
    _r0.i = 28;
    if (_r3.i < _r0.i) goto label10;
    _r0.i = 32;
    if (_r3.i <= _r0.i) goto label18;
    label10:;
    _r0.i = 9;
    if (_r3.i < _r0.i) goto label20;
    _r0.i = 13;
    if (_r3.i > _r0.i) goto label20;
    label18:;
    _r0 = _r1;
    label19:;
    XMLVM_SOURCE_POSITION("Character.java", 3502)
    XMLVM_SOURCE_POSITION("Character.java", 3510)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label20:;
    XMLVM_SOURCE_POSITION("Character.java", 3504)
    _r0.i = 5760;
    if (_r3.i != _r0.i) goto label26;
    _r0 = _r1;
    XMLVM_SOURCE_POSITION("Character.java", 3505)
    goto label19;
    label26:;
    XMLVM_SOURCE_POSITION("Character.java", 3507)
    _r0.i = 8192;
    if (_r3.i < _r0.i) goto label34;
    _r0.i = 8199;
    if (_r3.i != _r0.i) goto label36;
    label34:;
    _r0 = _r2;
    XMLVM_SOURCE_POSITION("Character.java", 3508)
    goto label19;
    label36:;
    _r0.i = 8203;
    if (_r3.i <= _r0.i) goto label54;
    _r0.i = 8232;
    if (_r3.i == _r0.i) goto label54;
    _r0.i = 8233;
    if (_r3.i == _r0.i) goto label54;
    _r0.i = 12288;
    if (_r3.i == _r0.i) goto label54;
    _r0 = _r2;
    goto label19;
    label54:;
    _r0 = _r1;
    goto label19;
    //XMLVM_END_WRAPPER
}

JAVA_BOOLEAN java_lang_Character_isWhitespace___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_isWhitespace___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "isWhitespace", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3524)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::isWhitespace
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_lang_Character_reverseBytes___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_reverseBytes___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "reverseBytes", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3537)
    _r0.i = _r2.i << 8;
    _r1.i = _r2.i >> 8;
    _r0.i = _r0.i | _r1.i;
    _r0.i = _r0.i & 0xffff;
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_lang_Character_toLowerCase___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_toLowerCase___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "toLowerCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r8.i = n1;
    _r1.i = 192;
    _r6.i = 32768;
    // "A\300\330\400\460\462\471\512\570\571\601\602\606\607\611\613\616\617\620\621\623\624\626\627\630\634\635\637\640\646\647\651\654\656\657\661\663\667\670\674\704\705\707\710\712\713\736\761\762\766\767\770\1042\1606\1610\1614\1616\1621\1643\1732\2000\2020\2140\2214\2301\2307\2313\2320\2370\2461\17000\17240\17410\17430\17450\17470\17510\17531\17550\17610\17630\17650\17670\17672\17674\17710\17714\17730\17732\17750\17752\17754\17770\17772\17774\20446\20452\20453\20540\22266\177441"
    _r3.o = xmlvm_create_java_string_from_pool(148);
    XMLVM_SOURCE_POSITION("Character.java", 3552)
    _r0.i = 65;
    if (_r0.i > _r8.i) goto label19;
    _r0.i = 90;
    if (_r8.i > _r0.i) goto label19;
    XMLVM_SOURCE_POSITION("Character.java", 3553)
    _r0.i = _r8.i + 32;
    _r0.i = _r0.i & 0xffff;
    label18:;
    XMLVM_SOURCE_POSITION("Character.java", 3579)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label19:;
    XMLVM_SOURCE_POSITION("Character.java", 3555)
    if (_r8.i >= _r1.i) goto label23;
    _r0 = _r8;
    XMLVM_SOURCE_POSITION("Character.java", 3556)
    goto label18;
    label23:;
    XMLVM_SOURCE_POSITION("Character.java", 3558)
    _r0.i = 1000;
    if (_r8.i >= _r0.i) goto label35;
    XMLVM_SOURCE_POSITION("Character.java", 3559)
    _r0.o = java_lang_Character_GET_lowercaseValuesCache();
    _r1.i = _r8.i - _r1.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r0.i = _r0.i & 0xffff;
    goto label18;
    label35:;
    XMLVM_SOURCE_POSITION("Character.java", 3562)
    // "A\300\330\400\460\462\471\512\570\571\601\602\606\607\611\613\616\617\620\621\623\624\626\627\630\634\635\637\640\646\647\651\654\656\657\661\663\667\670\674\704\705\707\710\712\713\736\761\762\766\767\770\1042\1606\1610\1614\1616\1621\1643\1732\2000\2020\2140\2214\2301\2307\2313\2320\2370\2461\17000\17240\17410\17430\17450\17470\17510\17531\17550\17610\17630\17650\17670\17672\17674\17710\17714\17730\17732\17750\17752\17754\17770\17772\17774\20446\20452\20453\20540\22266\177441"
    _r0.o = xmlvm_create_java_string_from_pool(148);

    
    // Red class access removed: org.apache.harmony.luni.util.BinarySearch::binarySearchRange
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Character.java", 3563)
    if (_r0.i < 0) goto label89;
    XMLVM_SOURCE_POSITION("Character.java", 3564)
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Character.java", 3565)
    // "A\300\330\400\460\462\471\512\570\571\601\602\606\607\611\613\616\617\620\621\623\624\626\627\630\634\635\637\640\646\647\651\654\656\657\661\663\667\670\674\704\705\707\710\712\713\736\761\762\766\767\770\1042\1606\1610\1614\1616\1621\1643\1732\2000\2020\2140\2214\2301\2307\2313\2320\2370\2461\17000\17240\17410\17430\17450\17470\17510\17531\17550\17610\17630\17650\17670\17672\17674\17710\17714\17730\17732\17750\17752\17754\17770\17772\17774\20446\20452\20453\20540\22266\177441"
    _r2.o = xmlvm_create_java_string_from_pool(148);
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(3)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r3.o)->tib->vtable[6])(_r3.o, _r0.i);
    XMLVM_SOURCE_POSITION("Character.java", 3566)
    _r3.o = java_lang_Character_GET_lowercaseValues();
    _r4.i = _r0.i * 2;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    XMLVM_SOURCE_POSITION("Character.java", 3567)
    _r4.i = _r2.i & _r6.i;
    _r5.i = _r3.i & _r6.i;
    if (_r4.i == _r5.i) goto label91;
    XMLVM_SOURCE_POSITION("Character.java", 3568)
    _r1.i = _r3.i ^ _r6.i;
    _r1.i = _r1.i & 0xffff;
    _r3.i = 1;
    label66:;
    XMLVM_SOURCE_POSITION("Character.java", 3569)
    XMLVM_SOURCE_POSITION("Character.java", 3571)
    if (_r8.i > _r1.i) goto label89;
    XMLVM_SOURCE_POSITION("Character.java", 3572)
    if (_r3.i == 0) goto label78;
    _r1.i = _r8.i & 1;
    _r2.i = _r2.i & 1;
    if (_r1.i == _r2.i) goto label78;
    _r0 = _r8;
    XMLVM_SOURCE_POSITION("Character.java", 3573)
    goto label18;
    label78:;
    XMLVM_SOURCE_POSITION("Character.java", 3575)
    _r1.o = java_lang_Character_GET_lowercaseValues();
    _r0.i = _r0.i * 2;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 3576)
    _r0.i = _r0.i + _r8.i;
    _r0.i = _r0.i & 0xffff;
    goto label18;
    label89:;
    _r0 = _r8;
    goto label18;
    label91:;
    _r7 = _r3;
    _r3 = _r1;
    _r1 = _r7;
    goto label66;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_toLowerCase___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_toLowerCase___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "toLowerCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3593)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::toLowerCase
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Character_toString__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_toString__]
    XMLVM_ENTER_METHOD("java.lang.Character", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.o = me;
    XMLVM_SOURCE_POSITION("Character.java", 3598)
    XMLVM_CHECK_NPE(1)
    _r0.i = ((java_lang_Character*) _r1.o)->fields.java_lang_Character.value_;
    _r0.o = java_lang_String_valueOf___char(_r0.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_OBJECT java_lang_Character_toString___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_toString___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "toString", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3609)
    _r0.o = java_lang_String_valueOf___char(_r1.i);
    XMLVM_EXIT_METHOD()
    return _r0.o;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_lang_Character_toTitleCase___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_toTitleCase___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "toTitleCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r2.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3622)
    _r0.i = java_lang_Character_isTitleCase___char(_r2.i);
    if (_r0.i == 0) goto label8;
    _r0 = _r2;
    label7:;
    XMLVM_SOURCE_POSITION("Character.java", 3623)
    XMLVM_SOURCE_POSITION("Character.java", 3629)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label8:;
    XMLVM_SOURCE_POSITION("Character.java", 3625)
    // "\704\706\707\711\712\714\761\763"
    _r0.o = xmlvm_create_java_string_from_pool(150);

    
    // Red class access removed: org.apache.harmony.luni.util.BinarySearch::binarySearch
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Character.java", 3626)
    if (_r0.i < 0) goto label21;
    XMLVM_SOURCE_POSITION("Character.java", 3627)
    _r1.o = java_lang_Character_GET_titlecaseValues();
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    goto label7;
    label21:;
    _r0.i = java_lang_Character_toUpperCase___char(_r2.i);
    goto label7;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_toTitleCase___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_toTitleCase___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "toTitleCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3642)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::toTitleCase
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_CHAR java_lang_Character_toUpperCase___char(JAVA_CHAR n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_toUpperCase___char]
    XMLVM_ENTER_METHOD("java.lang.Character", "toUpperCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    XMLVMElem _r3;
    XMLVMElem _r4;
    XMLVMElem _r5;
    XMLVMElem _r6;
    XMLVMElem _r7;
    XMLVMElem _r8;
    _r8.i = n1;
    _r1.i = 181;
    _r6.i = 32768;
    // "a\265\340\370\377\401\461\463\472\513\572\577\603\610\614\622\625\631\641\650\655\660\664\671\675\677\705\706\710\711\713\714\716\735\737\762\763\765\771\1043\1123\1124\1126\1131\1133\1140\1143\1150\1151\1157\1162\1165\1200\1203\1210\1212\1222\1505\1654\1655\1661\1702\1703\1714\1715\1720\1721\1725\1726\1733\1760\1761\1762\2060\2120\2141\2215\2302\2310\2314\2321\2371\2541\17001\17233\17241\17400\17420\17440\17460\17500\17521\17540\17560\17562\17566\17570\17572\17574\17600\17620\17640\17660\17663\17676\17703\17720\17740\17745\17763\20560\22320\177501"
    _r3.o = xmlvm_create_java_string_from_pool(147);
    XMLVM_SOURCE_POSITION("Character.java", 3657)
    _r0.i = 97;
    if (_r0.i > _r8.i) goto label21;
    _r0.i = 122;
    if (_r8.i > _r0.i) goto label21;
    XMLVM_SOURCE_POSITION("Character.java", 3658)
    _r0.i = 32;
    _r0.i = _r8.i - _r0.i;
    _r0.i = _r0.i & 0xffff;
    label20:;
    XMLVM_SOURCE_POSITION("Character.java", 3683)
    XMLVM_EXIT_METHOD()
    return _r0.i;
    label21:;
    XMLVM_SOURCE_POSITION("Character.java", 3660)
    if (_r8.i >= _r1.i) goto label25;
    _r0 = _r8;
    XMLVM_SOURCE_POSITION("Character.java", 3661)
    goto label20;
    label25:;
    XMLVM_SOURCE_POSITION("Character.java", 3663)
    _r0.i = 1000;
    if (_r8.i >= _r0.i) goto label37;
    XMLVM_SOURCE_POSITION("Character.java", 3664)
    _r0.o = java_lang_Character_GET_uppercaseValuesCache();
    _r1.i = _r8.i - _r1.i;
    XMLVM_CHECK_NPE(0)
    XMLVM_CHECK_ARRAY_BOUNDS(_r0.o, _r1.i);
    _r0.i = ((JAVA_ARRAY_INT*) (((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r1.i];
    _r0.i = _r0.i & 0xffff;
    goto label20;
    label37:;
    XMLVM_SOURCE_POSITION("Character.java", 3666)
    // "a\265\340\370\377\401\461\463\472\513\572\577\603\610\614\622\625\631\641\650\655\660\664\671\675\677\705\706\710\711\713\714\716\735\737\762\763\765\771\1043\1123\1124\1126\1131\1133\1140\1143\1150\1151\1157\1162\1165\1200\1203\1210\1212\1222\1505\1654\1655\1661\1702\1703\1714\1715\1720\1721\1725\1726\1733\1760\1761\1762\2060\2120\2141\2215\2302\2310\2314\2321\2371\2541\17001\17233\17241\17400\17420\17440\17460\17500\17521\17540\17560\17562\17566\17570\17572\17574\17600\17620\17640\17660\17663\17676\17703\17720\17740\17745\17763\20560\22320\177501"
    _r0.o = xmlvm_create_java_string_from_pool(147);

    
    // Red class access removed: org.apache.harmony.luni.util.BinarySearch::binarySearchRange
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_SOURCE_POSITION("Character.java", 3667)
    if (_r0.i < 0) goto label91;
    XMLVM_SOURCE_POSITION("Character.java", 3668)
    _r1.i = 0;
    XMLVM_SOURCE_POSITION("Character.java", 3669)
    // "a\265\340\370\377\401\461\463\472\513\572\577\603\610\614\622\625\631\641\650\655\660\664\671\675\677\705\706\710\711\713\714\716\735\737\762\763\765\771\1043\1123\1124\1126\1131\1133\1140\1143\1150\1151\1157\1162\1165\1200\1203\1210\1212\1222\1505\1654\1655\1661\1702\1703\1714\1715\1720\1721\1725\1726\1733\1760\1761\1762\2060\2120\2141\2215\2302\2310\2314\2321\2371\2541\17001\17233\17241\17400\17420\17440\17460\17500\17521\17540\17560\17562\17566\17570\17572\17574\17600\17620\17640\17660\17663\17676\17703\17720\17740\17745\17763\20560\22320\177501"
    _r2.o = xmlvm_create_java_string_from_pool(147);
    //java_lang_String_charAt___int[6]
    XMLVM_CHECK_NPE(3)
    _r2.i = (*(JAVA_CHAR (*)(JAVA_OBJECT, JAVA_INT)) ((java_lang_String*) _r3.o)->tib->vtable[6])(_r3.o, _r0.i);
    XMLVM_SOURCE_POSITION("Character.java", 3670)
    _r3.o = java_lang_Character_GET_uppercaseValues();
    _r4.i = _r0.i * 2;
    XMLVM_CHECK_NPE(3)
    XMLVM_CHECK_ARRAY_BOUNDS(_r3.o, _r4.i);
    _r3.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r3.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r4.i];
    XMLVM_SOURCE_POSITION("Character.java", 3671)
    _r4.i = _r2.i & _r6.i;
    _r5.i = _r3.i & _r6.i;
    if (_r4.i == _r5.i) goto label93;
    XMLVM_SOURCE_POSITION("Character.java", 3672)
    _r1.i = _r3.i ^ _r6.i;
    _r1.i = _r1.i & 0xffff;
    _r3.i = 1;
    label68:;
    XMLVM_SOURCE_POSITION("Character.java", 3673)
    XMLVM_SOURCE_POSITION("Character.java", 3675)
    if (_r8.i > _r1.i) goto label91;
    XMLVM_SOURCE_POSITION("Character.java", 3676)
    if (_r3.i == 0) goto label80;
    _r1.i = _r8.i & 1;
    _r2.i = _r2.i & 1;
    if (_r1.i == _r2.i) goto label80;
    _r0 = _r8;
    XMLVM_SOURCE_POSITION("Character.java", 3677)
    goto label20;
    label80:;
    XMLVM_SOURCE_POSITION("Character.java", 3679)
    _r1.o = java_lang_Character_GET_uppercaseValues();
    _r0.i = _r0.i * 2;
    _r0.i = _r0.i + 1;
    XMLVM_CHECK_NPE(1)
    XMLVM_CHECK_ARRAY_BOUNDS(_r1.o, _r0.i);
    _r0.i = ((JAVA_ARRAY_CHAR*) (((org_xmlvm_runtime_XMLVMArray*) _r1.o)->fields.org_xmlvm_runtime_XMLVMArray.array_))[_r0.i];
    XMLVM_SOURCE_POSITION("Character.java", 3680)
    _r0.i = _r0.i + _r8.i;
    _r0.i = _r0.i & 0xffff;
    goto label20;
    label91:;
    _r0 = _r8;
    goto label20;
    label93:;
    _r7 = _r3;
    _r3 = _r1;
    _r1 = _r7;
    goto label68;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_toUpperCase___int(JAVA_INT n1)
{
    XMLVM_CLASS_INIT(java_lang_Character)
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_toUpperCase___int]
    XMLVM_ENTER_METHOD("java.lang.Character", "toUpperCase", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    _r1.i = n1;
    XMLVM_SOURCE_POSITION("Character.java", 3697)

    
    // Red class access removed: com.ibm.icu.lang.UCharacter::toUpperCase
    XMLVM_RED_CLASS_DEPENDENCY();
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

JAVA_INT java_lang_Character_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_WRAPPER[java_lang_Character_compareTo___java_lang_Object]
    XMLVM_ENTER_METHOD("java.lang.Character", "compareTo", "?")
    XMLVMElem _r0;
    XMLVMElem _r1;
    XMLVMElem _r2;
    _r1.o = me;
    _r2.o = n1;
    XMLVM_SOURCE_POSITION("Character.java", 1)
    _r2.o = _r2.o;
    XMLVM_CHECK_NPE(1)
    _r0.i = java_lang_Character_compareTo___java_lang_Character(_r1.o, _r2.o);
    XMLVM_EXIT_METHOD()
    return _r0.i;
    //XMLVM_END_WRAPPER
}

