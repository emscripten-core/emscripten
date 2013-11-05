
#include "xmlvm.h"
#include "java_lang_Double.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
//XMLVM_END_NATIVE_IMPLEMENTATION

JAVA_LONG java_lang_Double_doubleToLongBits___double(JAVA_DOUBLE n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Double_doubleToLongBits___double]
    union {
        JAVA_DOUBLE d;
        JAVA_LONG   l;
    } u;
    
    u.d = n1;
    return u.l;
    //XMLVM_END_NATIVE
}

JAVA_LONG java_lang_Double_doubleToRawLongBits___double(JAVA_DOUBLE n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Double_doubleToRawLongBits___double]
    union {
        JAVA_DOUBLE d;
        JAVA_LONG   l;
    } u;
    
    u.d = n1;
    return u.l;
    //XMLVM_END_NATIVE
}

JAVA_DOUBLE java_lang_Double_longBitsToDouble___long(JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_Double_longBitsToDouble___long]
    union {
        JAVA_DOUBLE d;
        JAVA_LONG   l;
    } u;
    
    u.l = n1;
    return u.d;
    //XMLVM_END_NATIVE
}

