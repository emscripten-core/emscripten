#ifndef __JAVA_IO_FILE__
#define __JAVA_IO_FILE__

#include "xmlvm.h"

// Preprocessor constants for interfaces:
#define XMLVM_ITABLE_SIZE_java_io_File 8
// Implemented interfaces:
#include "java_io_Serializable.h"
#include "java_lang_Comparable.h"
// Super Class:
#include "java_lang_Object.h"

// Circular references:
#ifndef XMLVM_FORWARD_DECL_java_io_IOException
#define XMLVM_FORWARD_DECL_java_io_IOException
XMLVM_FORWARD_DECL(java_io_IOException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Character
#define XMLVM_FORWARD_DECL_java_lang_Character
XMLVM_FORWARD_DECL(java_lang_Character)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_Class
#define XMLVM_FORWARD_DECL_java_lang_Class
XMLVM_FORWARD_DECL(java_lang_Class)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_NullPointerException
#define XMLVM_FORWARD_DECL_java_lang_NullPointerException
XMLVM_FORWARD_DECL(java_lang_NullPointerException)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_RuntimePermission
#define XMLVM_FORWARD_DECL_java_lang_RuntimePermission
XMLVM_FORWARD_DECL(java_lang_RuntimePermission)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_SecurityManager
#define XMLVM_FORWARD_DECL_java_lang_SecurityManager
XMLVM_FORWARD_DECL(java_lang_SecurityManager)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_String
#define XMLVM_FORWARD_DECL_java_lang_String
XMLVM_FORWARD_DECL(java_lang_String)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_StringBuilder
#define XMLVM_FORWARD_DECL_java_lang_StringBuilder
XMLVM_FORWARD_DECL(java_lang_StringBuilder)
#endif
#ifndef XMLVM_FORWARD_DECL_java_lang_System
#define XMLVM_FORWARD_DECL_java_lang_System
XMLVM_FORWARD_DECL(java_lang_System)
#endif
#ifndef XMLVM_FORWARD_DECL_java_security_AccessController
#define XMLVM_FORWARD_DECL_java_security_AccessController
XMLVM_FORWARD_DECL(java_security_AccessController)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_ArrayList
#define XMLVM_FORWARD_DECL_java_util_ArrayList
XMLVM_FORWARD_DECL(java_util_ArrayList)
#endif
#ifndef XMLVM_FORWARD_DECL_java_util_List
#define XMLVM_FORWARD_DECL_java_util_List
XMLVM_FORWARD_DECL(java_util_List)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_internal_nls_Messages
XMLVM_FORWARD_DECL(org_apache_harmony_luni_internal_nls_Messages)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_PriviAction
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_PriviAction
XMLVM_FORWARD_DECL(org_apache_harmony_luni_util_PriviAction)
#endif
#ifndef XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_Util
#define XMLVM_FORWARD_DECL_org_apache_harmony_luni_util_Util
XMLVM_FORWARD_DECL(org_apache_harmony_luni_util_Util)
#endif
// Class declarations for java.io.File
XMLVM_DEFINE_CLASS(java_io_File, 7, XMLVM_ITABLE_SIZE_java_io_File)

extern JAVA_OBJECT __CLASS_java_io_File;
extern JAVA_OBJECT __CLASS_java_io_File_1ARRAY;
extern JAVA_OBJECT __CLASS_java_io_File_2ARRAY;
extern JAVA_OBJECT __CLASS_java_io_File_3ARRAY;
//XMLVM_BEGIN_DECLARATIONS
#define __ADDITIONAL_INSTANCE_FIELDS_java_io_File
//XMLVM_END_DECLARATIONS

#define __INSTANCE_FIELDS_java_io_File \
    __INSTANCE_FIELDS_java_lang_Object; \
    struct { \
        JAVA_OBJECT path_; \
        JAVA_OBJECT properPath_; \
        __ADDITIONAL_INSTANCE_FIELDS_java_io_File \
    } java_io_File

struct java_io_File {
    __TIB_DEFINITION_java_io_File* tib;
    struct {
        __INSTANCE_FIELDS_java_io_File;
    } fields;
};
#ifndef XMLVM_FORWARD_DECL_java_io_File
#define XMLVM_FORWARD_DECL_java_io_File
typedef struct java_io_File java_io_File;
#endif

#define XMLVM_VTABLE_SIZE_java_io_File 7
#define XMLVM_VTABLE_IDX_java_io_File_equals___java_lang_Object 1
#define XMLVM_VTABLE_IDX_java_io_File_hashCode__ 4
#define XMLVM_VTABLE_IDX_java_io_File_toString__ 5
#define XMLVM_VTABLE_IDX_java_io_File_compareTo___java_lang_Object 6

void __INIT_java_io_File();
void __INIT_IMPL_java_io_File();
void __DELETE_java_io_File(void* me, void* client_data);
void __INIT_INSTANCE_MEMBERS_java_io_File(JAVA_OBJECT me, int derivedClassWillRegisterFinalizer);
JAVA_OBJECT __NEW_java_io_File();
JAVA_OBJECT __NEW_INSTANCE_java_io_File();
void xmlvm_init_native_java_io_File();
JAVA_LONG java_io_File_GET_serialVersionUID();
void java_io_File_PUT_serialVersionUID(JAVA_LONG v);
JAVA_OBJECT java_io_File_GET_EMPTY_STRING();
void java_io_File_PUT_EMPTY_STRING(JAVA_OBJECT v);
JAVA_CHAR java_io_File_GET_separatorChar();
void java_io_File_PUT_separatorChar(JAVA_CHAR v);
JAVA_OBJECT java_io_File_GET_separator();
void java_io_File_PUT_separator(JAVA_OBJECT v);
JAVA_CHAR java_io_File_GET_pathSeparatorChar();
void java_io_File_PUT_pathSeparatorChar(JAVA_CHAR v);
JAVA_OBJECT java_io_File_GET_pathSeparator();
void java_io_File_PUT_pathSeparator(JAVA_OBJECT v);
JAVA_INT java_io_File_GET_counter();
void java_io_File_PUT_counter(JAVA_INT v);
JAVA_INT java_io_File_GET_counterBase();
void java_io_File_PUT_counterBase(JAVA_INT v);
JAVA_OBJECT java_io_File_GET_tempFileLocker();
void java_io_File_PUT_tempFileLocker(JAVA_OBJECT v);
JAVA_BOOLEAN java_io_File_GET_caseSensitive();
void java_io_File_PUT_caseSensitive(JAVA_BOOLEAN v);
void java_io_File_oneTimeInitialization__();
void java_io_File___INIT____java_io_File_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_io_File___INIT____java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_File___INIT____java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_io_File___INIT____java_net_URI(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_io_File_calculatePath___java_lang_String_java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
void java_io_File_checkURI___java_net_URI(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_io_File_rootsImpl__();
JAVA_BOOLEAN java_io_File_isCaseSensitiveImpl__();
JAVA_OBJECT java_io_File_listRoots__();
JAVA_OBJECT java_io_File_fixSlashes___java_lang_String(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_canRead__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_canWrite__(JAVA_OBJECT me);
JAVA_INT java_io_File_compareTo___java_io_File(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_delete__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_deleteDirImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_deleteFileImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_File_deleteOnExit__(JAVA_OBJECT me);
// Vtable index: 1
JAVA_BOOLEAN java_io_File_equals___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_exists__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_existsImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_io_File_getAbsolutePath__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_getAbsoluteFile__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_getCanonicalPath__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_resolve___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_io_File_resolveLink___byte_1ARRAY_int_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_INT n2, JAVA_BOOLEAN n3);
JAVA_OBJECT java_io_File_getCanonicalFile__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_getCanonImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_io_File_getName__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_getParent__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_getParentFile__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_getPath__(JAVA_OBJECT me);
JAVA_LONG java_io_File_getTotalSpace__(JAVA_OBJECT me);
JAVA_LONG java_io_File_getTotalSpaceImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_LONG java_io_File_getUsableSpace__(JAVA_OBJECT me);
JAVA_LONG java_io_File_getUsableSpaceImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_LONG java_io_File_getFreeSpace__(JAVA_OBJECT me);
JAVA_LONG java_io_File_getFreeSpaceImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
// Vtable index: 4
JAVA_INT java_io_File_hashCode__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_isAbsolute__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_isDirectory__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_isDirectoryImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_isFile__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_isFileImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_isHidden__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_isHiddenImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_isReadOnlyImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_isWriteOnlyImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_io_File_getLinkImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_LONG java_io_File_lastModified__(JAVA_OBJECT me);
JAVA_LONG java_io_File_lastModifiedImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_setLastModified___long(JAVA_OBJECT me, JAVA_LONG n1);
JAVA_BOOLEAN java_io_File_setLastModifiedImpl___byte_1ARRAY_long(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_LONG n2);
JAVA_BOOLEAN java_io_File_setReadOnly__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_setReadOnlyImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_setReadable___boolean_boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1, JAVA_BOOLEAN n2);
JAVA_BOOLEAN java_io_File_setReadable___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
JAVA_BOOLEAN java_io_File_setReadableImpl___byte_1ARRAY_boolean_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2, JAVA_BOOLEAN n3);
JAVA_BOOLEAN java_io_File_setWritable___boolean_boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1, JAVA_BOOLEAN n2);
JAVA_BOOLEAN java_io_File_setWritableImpl___byte_1ARRAY_boolean_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2, JAVA_BOOLEAN n3);
JAVA_BOOLEAN java_io_File_setWritable___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
JAVA_LONG java_io_File_length__(JAVA_OBJECT me);
JAVA_LONG java_io_File_lengthImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_io_File_list__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_listFiles__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_listFiles___java_io_FilenameFilter(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_io_File_listFiles___java_io_FileFilter(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_io_File_list___java_io_FilenameFilter(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_io_File_listImpl___byte_1ARRAY(JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_mkdir__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_mkdirImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_mkdirs__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_createNewFile__(JAVA_OBJECT me);
JAVA_INT java_io_File_newFileImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_OBJECT java_io_File_createTempFile___java_lang_String_java_lang_String(JAVA_OBJECT n1, JAVA_OBJECT n2);
JAVA_OBJECT java_io_File_createTempFile___java_lang_String_java_lang_String_java_io_File(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
JAVA_OBJECT java_io_File_genTempFile___java_lang_String_java_lang_String_java_io_File(JAVA_OBJECT n1, JAVA_OBJECT n2, JAVA_OBJECT n3);
JAVA_OBJECT java_io_File_properPath___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
JAVA_BOOLEAN java_io_File_renameTo___java_io_File(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_renameToImpl___byte_1ARRAY_byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_OBJECT n2);
// Vtable index: 5
JAVA_OBJECT java_io_File_toString__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_toURI__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_toURL__(JAVA_OBJECT me);
JAVA_OBJECT java_io_File_getAbsoluteName__(JAVA_OBJECT me);
void java_io_File_writeObject___java_io_ObjectOutputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_File_readObject___java_io_ObjectInputStream(JAVA_OBJECT me, JAVA_OBJECT n1);
JAVA_BOOLEAN java_io_File_setExecutable___boolean_boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1, JAVA_BOOLEAN n2);
JAVA_BOOLEAN java_io_File_setExecutable___boolean(JAVA_OBJECT me, JAVA_BOOLEAN n1);
JAVA_BOOLEAN java_io_File_setExecutableImpl___byte_1ARRAY_boolean_boolean(JAVA_OBJECT me, JAVA_OBJECT n1, JAVA_BOOLEAN n2, JAVA_BOOLEAN n3);
JAVA_BOOLEAN java_io_File_canExecute__(JAVA_OBJECT me);
JAVA_BOOLEAN java_io_File_isExecutableImpl___byte_1ARRAY(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_File_checkExec__(JAVA_OBJECT me);
void java_io_File_checkWrite__(JAVA_OBJECT me);
// Vtable index: 6
JAVA_INT java_io_File_compareTo___java_lang_Object(JAVA_OBJECT me, JAVA_OBJECT n1);
void java_io_File___CLINIT_();

#endif
