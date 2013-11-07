
#include "xmlvm.h"
#include "java_lang_Float.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
#include <math.h>
//XMLVM_END_NATIVE_IMPLEMENTATION

JAVA_INT java_lang_Float_floatToIntBits___float(JAVA_FLOAT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Float_floatToIntBits___float]
    union {
        JAVA_FLOAT f;
        JAVA_INT   i;
    } u;
    
    if (isnan(n1)) {
        return 0x7fc00000;
    } else {
        u.f = n1;
        return u.i;
    }
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_Float_floatToRawIntBits___float(JAVA_FLOAT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Float_floatToRawIntBits___float]
    union {
        JAVA_FLOAT f;
        JAVA_INT   i;
    } u;
    
    u.f = n1;
    return u.i;
    //XMLVM_END_NATIVE
}

JAVA_FLOAT java_lang_Float_intBitsToFloat___int(JAVA_INT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Float_intBitsToFloat___int]
    union {
        JAVA_FLOAT f;
        JAVA_INT   i;
    } u;
    
    u.i = n1;
    return u.f;
    //XMLVM_END_NATIVE
}

