
#include "xmlvm.h"
#include "org_xmlvm_runtime_XMLVMOutputStream.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
//XMLVM_END_NATIVE_IMPLEMENTATION

void org_xmlvm_runtime_XMLVMOutputStream_nativeWrite___int(JAVA_INT n1)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_XMLVMOutputStream_nativeWrite___int]
    printf("%c", (char) n1);
    //XMLVM_END_NATIVE
}

