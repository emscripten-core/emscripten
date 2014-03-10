
#include "xmlvm.h"
#include "org_apache_harmony_luni_util_FloatingPointParser.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
#include <math.h>
//XMLVM_END_NATIVE_IMPLEMENTATION

JAVA_DOUBLE org_apache_harmony_luni_util_FloatingPointParser_parseDblImpl___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_util_FloatingPointParser_parseDblImpl___java_lang_String_int]
    const char* doubleNumStr = xmlvm_java_string_to_const_char(n1);
    JAVA_DOUBLE doubleNum = atof(doubleNumStr);
    if (n2 == 0) {
        return doubleNum;
    }
    return doubleNum * pow(10, n2);
    //XMLVM_END_NATIVE
}

JAVA_FLOAT org_apache_harmony_luni_util_FloatingPointParser_parseFltImpl___java_lang_String_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_util_FloatingPointParser_parseFltImpl___java_lang_String_int]
    const char* floatNumStr = xmlvm_java_string_to_const_char(n1);
    JAVA_FLOAT floatNum = atof(floatNumStr);
    if (n2 == 0) {
        return floatNum;
    }
    return floatNum * pow(10, n2);
    //XMLVM_END_NATIVE
}

