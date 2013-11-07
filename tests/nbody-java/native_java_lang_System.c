
#include "xmlvm.h"
#include "java_lang_System.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
#include <locale.h>
#include <sys/time.h>
#include "java_lang_String.h"

//From HARMONY/classlib/modules/luni/src/main/native/luni/shared/luniglob.c
#define CHARSETBUFF 64

//From HARMONY/classlib/modules/luni/src/main/native/luni/unix/helpers.c
void getOSCharset(char *locale, const size_t size) {
    char * codec = NULL;
    size_t cur = 0;
    short flag = 0;
    setlocale(LC_CTYPE, "");
    codec = setlocale(LC_CTYPE, NULL);
    // get codeset from language[_territory][.codeset][@modifier]
    while (*codec) {
        if (!flag) {
            if (*codec != '.') {
                codec++;
                continue;
            } else {
                flag = 1;
                codec++;
            }
        } else {
            if (*codec == '@') {
                break;
            } else {
                locale[cur++] = (*codec);
                codec++;
                if (cur >= size) {
                    // Not enough size
                    cur = 0;
                    break;
                }
            }
        }
    }
    locale[cur] = '\0';
    if (!strlen(locale)) {
        strcpy(locale, "8859_1");
    }
    return;
}

//XMLVM_END_NATIVE_IMPLEMENTATION

void java_lang_System_initNativeLayer__()
{
    //XMLVM_BEGIN_NATIVE[java_lang_System_initNativeLayer__]
    // Nothing to be done
    //XMLVM_END_NATIVE
}

JAVA_LONG java_lang_System_currentTimeMillis__()
{
    //XMLVM_BEGIN_NATIVE[java_lang_System_currentTimeMillis__]
    struct timeval now;
    gettimeofday(&now, NULL);

    JAVA_LONG msec = ((JAVA_LONG) now.tv_sec) * 1000;
    msec += now.tv_usec / 1000;

    return msec;
    //XMLVM_END_NATIVE
}

JAVA_LONG java_lang_System_nanoTime__()
{
    //XMLVM_BEGIN_NATIVE[java_lang_System_nanoTime__]
    //TODO implement
    return java_lang_System_nanoTime__() * 1000000L;
    //XMLVM_END_NATIVE
}

void java_lang_System_nativeExit___int(JAVA_INT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_System_nativeExit___int]
    exit(n1);
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_System_getPropertyList__()
{
    //XMLVM_BEGIN_NATIVE[java_lang_System_getPropertyList__]
    return XMLVMArray_createSingleDimension(__CLASS_java_lang_String, 0);
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_lang_System_getEncoding___int(JAVA_INT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_System_getEncoding___int]
    //Get charset from the OS
#ifdef EMSCRIPTEN
	return xmlvm_create_java_string("UTF-8");
#else
    char charset[CHARSETBUFF];
    getOSCharset(charset, CHARSETBUFF);
    return xmlvm_create_java_string(charset);
#endif
    //XMLVM_END_NATIVE
}

JAVA_INT java_lang_System_identityHashCode___java_lang_Object(JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_lang_System_identityHashCode___java_lang_Object]
    return (JAVA_INT) n1;
    //XMLVM_END_NATIVE
}

