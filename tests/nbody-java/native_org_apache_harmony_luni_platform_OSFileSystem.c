
#include "xmlvm.h"
#include "org_apache_harmony_luni_platform_OSFileSystem.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION

#include <unistd.h>

#include "xmlvm-util.h"
#include "xmlvm-hy.h"
#include "hycomp.h"
#include "xmlvm-file.h"
#include "xmlvm-sock.h"

#include "java_io_IOException.h"



//XMLVM_END_NATIVE_IMPLEMENTATION

void org_apache_harmony_luni_platform_OSFileSystem_oneTimeInitializationImpl__()
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_oneTimeInitializationImpl__]
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSFileSystem_lockImpl___long_long_long_int_boolean(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2, JAVA_LONG n3, JAVA_INT n4, JAVA_BOOLEAN n5)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_lockImpl___long_long_long_int_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSFileSystem_getAllocGranularity__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_getAllocGranularity__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSFileSystem_unlockImpl___long_long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2, JAVA_LONG n3)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_unlockImpl___long_long_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSFileSystem_fflushImpl___long_boolean(JAVA_OBJECT me, JAVA_LONG n1, JAVA_BOOLEAN n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_fflushImpl___long_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_seekImpl___long_long_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_seekImpl___long_long_int]
    I_32 hywhence = 0;            /* The HY PPL equivalent of our whence arg.*/
    JAVA_LONG* fd = n1;
    JAVA_LONG offset = n2;
    JAVA_INT whence = n3;
    
    /* Convert whence argument */
    switch (whence)
    {
        case org_apache_harmony_luni_platform_IFileSystem_SEEK_SET:
            hywhence = HySeekSet;
            break;
        case org_apache_harmony_luni_platform_IFileSystem_SEEK_CUR:
            hywhence = HySeekCur;
            break;
        case org_apache_harmony_luni_platform_IFileSystem_SEEK_END:
            hywhence = HySeekEnd;
            break;
        default:
            return -1;
    }
    
    return (JAVA_LONG) hyfile_seek (fd, (I_64) offset, hywhence);
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_readDirectImpl___long_long_int_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_readDirectImpl___long_long_int_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_writeDirectImpl___long_long_int_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_writeDirectImpl___long_long_int_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_readImpl___long_byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_readImpl___long_byte_1ARRAY_int_int]
    JAVA_LONG fd                            = n1;
    org_xmlvm_runtime_XMLVMArray* byteArray = n2;
    JAVA_INT offset                         = n3;
    JAVA_INT nbytes                         = n4;
    
    JAVA_ARRAY_BYTE *bytes = byteArray->fields.org_xmlvm_runtime_XMLVMArray.array_;
    JAVA_LONG result;
    
    result = (JAVA_LONG) hyfile_read ((IDATA) fd, (void*) (bytes + offset), (IDATA) nbytes);
    if(result == -1 && hyerror_last_error_number() == HYPORT_ERROR_FILE_LOCKED){
        XMLVM_THROW_WITH_CSTRING(java_io_IOException, netLookupErrorString(HYPORT_ERROR_FILE_LOCKED))
    }
    
    return result;
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_writeImpl___long_byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2, JAVA_INT n3, JAVA_INT n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_writeImpl___long_byte_1ARRAY_int_int]
    JAVA_LONG fd                            = n1;
    org_xmlvm_runtime_XMLVMArray* byteArray = n2;
    JAVA_INT offset                         = n3;
    JAVA_INT nbytes                         = n4;

    JAVA_ARRAY_BYTE* bytes = byteArray->fields.org_xmlvm_runtime_XMLVMArray.array_;
    JAVA_LONG result;
    
    result = (JAVA_LONG) hyfile_write ((IDATA) fd, (void *) (bytes + offset), (IDATA) nbytes);
    if(result == -1 && hyerror_last_error_number() == HYPORT_ERROR_FILE_LOCKED){
        XMLVM_THROW_WITH_CSTRING(java_io_IOException, netLookupErrorString(HYPORT_ERROR_FILE_LOCKED))
    }
    
    return result;
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_readvImpl___long_long_1ARRAY_int_1ARRAY_int_1ARRAY_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_OBJECT n4, JAVA_INT n5)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_readvImpl___long_long_1ARRAY_int_1ARRAY_int_1ARRAY_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_writev___long_java_lang_Object_1ARRAY_int_1ARRAY_int_1ARRAY_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2, JAVA_OBJECT n3, JAVA_OBJECT n4, JAVA_INT n5)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_writev___long_java_lang_Object_1ARRAY_int_1ARRAY_int_1ARRAY_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSFileSystem_closeImpl___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_closeImpl___long]
    JAVA_LONG fd = n1;
    return (JAVA_INT) hyfile_close ((IDATA) fd);
    //XMLVM_END_NATIVE
}

JAVA_INT org_apache_harmony_luni_platform_OSFileSystem_truncateImpl___long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_truncateImpl___long_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_openImpl___byte_1ARRAY_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_openImpl___byte_1ARRAY_int]
    org_xmlvm_runtime_XMLVMArray* path = n1;
    JAVA_INT jflags = n2;
    
    I_32 flags = 0;
    I_32 mode = 0; 
    IDATA portFD;
    jsize length;
    char pathCopy[HyMaxPath];
    
    switch(jflags){
        case org_apache_harmony_luni_platform_IFileSystem_O_RDONLY:
            flags = HyOpenRead;
            mode = 0;
            break;
        case org_apache_harmony_luni_platform_IFileSystem_O_WRONLY:
            flags = HyOpenCreate | HyOpenWrite | HyOpenTruncate;
            mode = 0666;
            break;
        case org_apache_harmony_luni_platform_IFileSystem_O_RDWR:
            flags = HyOpenRead | HyOpenWrite | HyOpenCreate;
            mode = 0666;
            break;
        case org_apache_harmony_luni_platform_IFileSystem_O_APPEND:
            flags = HyOpenWrite | HyOpenCreate | HyOpenAppend; 
            mode = 0666;
            break;
        case org_apache_harmony_luni_platform_IFileSystem_O_RDWRSYNC:
            flags = HyOpenRead | HyOpenWrite | HyOpenCreate | HyOpenSync;
            mode = 0666;
            break;
    }
    
    length = XMLVMArray_count(path);
    length = length < HyMaxPath - 1 ? length : HyMaxPath - 1;
    XMLVM_MEMCPY(pathCopy, path->fields.org_xmlvm_runtime_XMLVMArray.array_, length);
    pathCopy[length] = '\0';
    ioh_convertToPlatform (pathCopy);
    
    portFD = hyfile_open (pathCopy, flags, mode);
    return (JAVA_LONG) portFD;
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_transferImpl___long_java_io_FileDescriptor_long_long(JAVA_OBJECT me, JAVA_LONG n1, JAVA_OBJECT n2, JAVA_LONG n3, JAVA_LONG n4)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_transferImpl___long_java_io_FileDescriptor_long_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_ttyAvailableImpl__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_ttyAvailableImpl__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_availableImpl___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_availableImpl___long]
    JAVA_OBJECT thiz = me;
    JAVA_LONG fd = n1;
    
    JAVA_LONG currentPosition = 
        org_apache_harmony_luni_platform_OSFileSystem_seekImpl___long_long_int(thiz, fd, 0, org_apache_harmony_luni_platform_IFileSystem_SEEK_CUR);
    
    JAVA_LONG endPosition =
        org_apache_harmony_luni_platform_OSFileSystem_seekImpl___long_long_int(thiz, fd, 0, org_apache_harmony_luni_platform_IFileSystem_SEEK_END);
    
    org_apache_harmony_luni_platform_OSFileSystem_seekImpl___long_long_int(thiz, fd, currentPosition, org_apache_harmony_luni_platform_IFileSystem_SEEK_SET);
    
    return (JAVA_LONG) (endPosition - currentPosition);
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_sizeImpl___long(JAVA_OBJECT me, JAVA_LONG n1)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_sizeImpl___long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG org_apache_harmony_luni_platform_OSFileSystem_ttyReadImpl___byte_1ARRAY_int_int(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_INT n3)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_platform_OSFileSystem_ttyReadImpl___byte_1ARRAY_int_int]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}


void xmlvm_init_native_org_apache_harmony_luni_platform_OSFileSystem()
{
    //XMLVM_BEGIN_NATIVE_IMPLEMENTATION_INIT
    //XMLVM_END_NATIVE_IMPLEMENTATION_INIT
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSFileSystem_getAllocGranularity__
    __TIB_org_apache_harmony_luni_platform_OSFileSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSFileSystem_getAllocGranularity__] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSFileSystem_getAllocGranularity__;
#endif
#ifdef XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSFileSystem_writev___long_java_lang_Object_1ARRAY_int_1ARRAY_int_1ARRAY_int
    __TIB_org_apache_harmony_luni_platform_OSFileSystem.vtable[XMLVM_VTABLE_IDX_org_apache_harmony_luni_platform_OSFileSystem_writev___long_java_lang_Object_1ARRAY_int_1ARRAY_int_1ARRAY_int] = 
        (VTABLE_PTR) org_apache_harmony_luni_platform_OSFileSystem_writev___long_java_lang_Object_1ARRAY_int_1ARRAY_int_1ARRAY_int;
#endif
}
