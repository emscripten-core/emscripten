
#include "xmlvm.h"
#include "org_xmlvm_runtime_XMLVMUtil.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
#include <unistd.h>
#ifdef __OBJC__
#ifndef XMLVM_NEW_IOS_API
#include "org_xmlvm_iphone_NSString.h"
#else
#include "org_xmlvm_ios_NSString.h"
#endif
#endif
//XMLVM_END_NATIVE_IMPLEMENTATION

JAVA_OBJECT org_xmlvm_runtime_XMLVMUtil_getCurrentWorkingDirectory__()
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_XMLVMUtil_getCurrentWorkingDirectory__]
#ifdef __OBJC__
    // Base directory is <App>/Documents/
    // http://developer.apple.com/iphone/library/documentation/iPhone/Conceptual/iPhoneOSProgrammingGuide/FilesandNetworking/FilesandNetworking.html
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* path = [paths objectAtIndex:0];
    JAVA_OBJECT jpath = fromNSString(path);
    return jpath;
#else
    char buf[1024];
    char* err = getcwd(buf, sizeof(buf));
    if (err == NULL) {
        XMLVM_INTERNAL_ERROR();
    }
    return xmlvm_create_java_string(buf);
#endif
    //XMLVM_END_NATIVE
}

