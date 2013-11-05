
#include "xmlvm.h"
#include "java_io_File.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include "xmlvm-util.h"

char * readlink_malloc (const char *filename)
{
    int size = 100;
    char *buffer = NULL;
          
    while (1)
    {
        buffer = (char *) XMLVM_ATOMIC_MALLOC (size);
        int nchars = readlink (filename, buffer, size);
        if (nchars < 0)
        {
            XMLVM_FREE(buffer);
            return NULL;
        }
        if (nchars < size)
            return buffer;
        XMLVM_FREE(buffer);
        size *= 2;
    }
}
//XMLVM_END_NATIVE_IMPLEMENTATION

void java_io_File_oneTimeInitialization__()
{
    //XMLVM_BEGIN_NATIVE[java_io_File_oneTimeInitialization__]
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_io_File_rootsImpl__()
{
    //XMLVM_BEGIN_NATIVE[java_io_File_rootsImpl__]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_isCaseSensitiveImpl__()
{
    //XMLVM_BEGIN_NATIVE[java_io_File_isCaseSensitiveImpl__]
    return 1;
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_deleteDirImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_deleteDirImpl___byte_1ARRAY]
    char* fileName = XMLVMUtil_convertFromByteArray(n1);
    return rmdir(fileName) == 0;
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_deleteFileImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_deleteFileImpl___byte_1ARRAY]
    char* fileName = XMLVMUtil_convertFromByteArray(n1);
    return remove(fileName) == 0;
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_existsImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_existsImpl___byte_1ARRAY]
    char* fileName = XMLVMUtil_convertFromByteArray(n1);
    struct stat buf;
    int err;
    err = stat(fileName, &buf);
    return err != -1;
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_io_File_getCanonImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_getCanonImpl___byte_1ARRAY]
    //See HARMONY/classlib/modules/luni/src/main/native/luni/shared/file.c
    //They don't seem to do more either on a UNIX file system
    return n1;
    //XMLVM_END_NATIVE
}

JAVA_LONG java_io_File_getTotalSpaceImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_getTotalSpaceImpl___byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG java_io_File_getUsableSpaceImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_getUsableSpaceImpl___byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG java_io_File_getFreeSpaceImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_getFreeSpaceImpl___byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_isDirectoryImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_isDirectoryImpl___byte_1ARRAY]
    char* fileName = XMLVMUtil_convertFromByteArray(n1);
    struct stat buf;
    int err;
    err = stat(fileName, &buf);
    return buf.st_mode; // XXX & S_IFDIR;
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_isFileImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_isFileImpl___byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_isHiddenImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_isHiddenImpl___byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_isReadOnlyImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_isReadOnlyImpl___byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_isWriteOnlyImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_isWriteOnlyImpl___byte_1ARRAY]
    return (JAVA_BOOLEAN) 0; //TODO: Check this natively
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_io_File_getLinkImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_getLinkImpl___byte_1ARRAY]
    const char* fileName = XMLVMUtil_convertFromByteArray(n1);
    char* resolved = readlink_malloc(fileName);
    if(resolved == NULL) {
        return n1;
    } else {
	XMLVMElem _r0;
        int length = strlen(resolved);
        _r0.o = XMLVMArray_createSingleDimension(__CLASS_byte, length);
        char* data = (char*) ((org_xmlvm_runtime_XMLVMArray*) _r0.o)->fields.org_xmlvm_runtime_XMLVMArray.array_;
        XMLVM_MEMCPY(data, resolved, length);
	return _r0.o;
    }
    //XMLVM_END_NATIVE
}

JAVA_LONG java_io_File_lastModifiedImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_lastModifiedImpl___byte_1ARRAY]
    char* fileName = XMLVMUtil_convertFromByteArray(n1);
    struct stat buf;
    int err;
    err = stat(fileName, &buf);
    return buf.st_mtime;
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_setLastModifiedImpl___byte_1ARRAY_long(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_LONG n2)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_setLastModifiedImpl___byte_1ARRAY_long]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_setReadOnlyImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_setReadOnlyImpl___byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_setReadableImpl___byte_1ARRAY_boolean_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2, JAVA_BOOLEAN n3)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_setReadableImpl___byte_1ARRAY_boolean_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_setWritableImpl___byte_1ARRAY_boolean_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2, JAVA_BOOLEAN n3)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_setWritableImpl___byte_1ARRAY_boolean_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_LONG java_io_File_lengthImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_lengthImpl___byte_1ARRAY]
    char* fileName = XMLVMUtil_convertFromByteArray(n1);
    FILE* fp = fopen(fileName, "rb");
    fseek(fp, 0L, SEEK_END);
    JAVA_LONG size = ftell(fp);
    fclose(fp);
    return size;
    //XMLVM_END_NATIVE
}

JAVA_OBJECT java_io_File_listImpl___byte_1ARRAY(JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_listImpl___byte_1ARRAY]
	     // TODO: There may be memory management issues in this implementation
	     org_xmlvm_runtime_XMLVMArray *array =
	         (org_xmlvm_runtime_XMLVMArray *) __NEW_org_xmlvm_runtime_XMLVMArray();
	     array->fields.org_xmlvm_runtime_XMLVMArray.type_   =  __CLASS_org_xmlvm_runtime_XMLVMArray;
	     array->fields.org_xmlvm_runtime_XMLVMArray.length_ =  256;
	     array->fields.org_xmlvm_runtime_XMLVMArray.array_  = XMLVM_MALLOC(sizeof(JAVA_OBJECT) * 256);

	     JAVA_OBJECT *subarray = (JAVA_OBJECT *) array->fields.org_xmlvm_runtime_XMLVMArray.array_;

	     int i = 0;
	     for (i = 0; i < 256; i++) {
	         org_xmlvm_runtime_XMLVMArray *array2 =
	            (org_xmlvm_runtime_XMLVMArray *) __NEW_org_xmlvm_runtime_XMLVMArray();
	         array2->fields.org_xmlvm_runtime_XMLVMArray.type_   = __CLASS_byte;
	         array2->fields.org_xmlvm_runtime_XMLVMArray.length_ = 256;
	         array2->fields.org_xmlvm_runtime_XMLVMArray.array_  = XMLVM_MALLOC(sizeof(char) * 256);
	         subarray[i] = (JAVA_OBJECT) array2;
	     }

	     // Get the
	     char* fileName = XMLVMUtil_convertFromByteArray(n1);

	     DIR *d;
	     struct dirent *e;
	     if (d = opendir(fileName)) {
	         i = 0;
	         int j = 0;
	         while (e = readdir(d)) {
	             if (i < 256) { // TODO: What if there are more than 256 files?
	                 org_xmlvm_runtime_XMLVMArray *array2 = (org_xmlvm_runtime_XMLVMArray *) (subarray[i]);
	                 char *bytearray = array2->fields.org_xmlvm_runtime_XMLVMArray.array_;
	                 for (j = 0; j < 256; j++) {
	                     bytearray[j] = e->d_name[j];
	                     if (e->d_name[j] == 0) {
	                         array2->fields.org_xmlvm_runtime_XMLVMArray.length_ = j;
	                         break;
	                     }
	                 }
	                 i++;

	             }

	             // Add e->d_name to array!
	         }
	         array->fields.org_xmlvm_runtime_XMLVMArray.length_ =  i;
	         closedir(d);
	     } else {
	         // TODO: Throw IOException?
	     }
	     return array;
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_mkdirImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_mkdirImpl___byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_INT java_io_File_newFileImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_newFileImpl___byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_renameToImpl___byte_1ARRAY_byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_renameToImpl___byte_1ARRAY_byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_setExecutableImpl___byte_1ARRAY_boolean_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2, JAVA_BOOLEAN n3)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_setExecutableImpl___byte_1ARRAY_boolean_boolean]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

JAVA_BOOLEAN java_io_File_isExecutableImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1)
{
    //XMLVM_BEGIN_NATIVE[java_io_File_isExecutableImpl___byte_1ARRAY]
    XMLVM_UNIMPLEMENTED_NATIVE_METHOD();
    //XMLVM_END_NATIVE
}

