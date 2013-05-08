
#include "xmlvm.h"
#include "org_xmlvm_runtime_XMLVMArray.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION

#include "java_lang_Class.h"

typedef struct {
    short sizeOfBaseType;
    short useAtomicMalloc;
} TYPE_INFO;


org_xmlvm_runtime_XMLVMArray* __NEW_XMLVMArray(JAVA_OBJECT type, JAVA_INT length, JAVA_OBJECT data)
{
    org_xmlvm_runtime_XMLVMArray* array = __NEW_org_xmlvm_runtime_XMLVMArray();
    org_xmlvm_runtime_XMLVMArray___INIT____java_lang_Class_int_java_lang_Object(array, type, length, data);
    return array;
}

/**
 * This function determines two pieces of information of a type: the size of the type in byte
 * and whether or not an atomic malloc() should be used. For arrays of primitive type it is possible
 * to use an atomic malloc() offered by the GC. In this case the GC knows that the memory region cannot
 * contain any pointers and can therefore skip scanning the memory region.
 */
TYPE_INFO XMLVMArray_getTypeInfo(JAVA_OBJECT type)
{
    TYPE_INFO info;
    info.useAtomicMalloc = 1;
    
    if (type == __CLASS_byte || type == __CLASS_boolean) {
        info.sizeOfBaseType = sizeof(JAVA_ARRAY_BYTE);
    } else if (type == __CLASS_char || type == __CLASS_short) {
        info.sizeOfBaseType = sizeof(JAVA_ARRAY_SHORT);
    } else if (type == __CLASS_int) {
        info.sizeOfBaseType = sizeof(JAVA_ARRAY_INT);
    } else if (type == __CLASS_float) {
        info.sizeOfBaseType = sizeof(JAVA_ARRAY_FLOAT);
    } else if (type == __CLASS_double) {
        info.sizeOfBaseType = sizeof(JAVA_ARRAY_DOUBLE);
    } else if (type == __CLASS_long) {
        info.sizeOfBaseType = sizeof(JAVA_ARRAY_LONG);
    } else {
        info.sizeOfBaseType = sizeof(void*);
        // For arrays of object references we need to use the regular malloc() as the
        // GC needs to scan this memory region.
        info.useAtomicMalloc = 0;
    }
    
    return info;
}

static org_xmlvm_runtime_XMLVMArray* XMLVMArray_createMultiDimensionsWithCount(JAVA_OBJECT type, JAVA_ARRAY_INT* dim, int count)
{
    JAVA_ARRAY_INT dimensions = *dim;
    org_xmlvm_runtime_XMLVMArray* slice;
    int i;
    dim++;
    count--;
    if (count == 0) {
        return XMLVMArray_createSingleDimension(type, dimensions);
    }
    slice = XMLVMArray_createSingleDimension(type, dimensions);
    JAVA_ARRAY_OBJECT* sliceData = (JAVA_ARRAY_OBJECT*) slice->fields.org_xmlvm_runtime_XMLVMArray.array_;
    // Get base type
    java_lang_Class* typeClass = (java_lang_Class*) type;
    __TIB_DEFINITION_TEMPLATE* typeTIB = (__TIB_DEFINITION_TEMPLATE*) typeClass->fields.java_lang_Class.tib_;
    JAVA_OBJECT baseType = typeTIB->baseType;
    for (i = 0; i < dimensions; i++) {
        //TODO should 'type' be baseType of this type?
        sliceData[i] = XMLVMArray_createMultiDimensionsWithCount(baseType, dim, count);
    }
    return slice;
}

JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_getClass__(JAVA_OBJECT me)
{
    org_xmlvm_runtime_XMLVMArray* thiz = me;
    return thiz->fields.org_xmlvm_runtime_XMLVMArray.type_;
}

//XMLVM_END_NATIVE_IMPLEMENTATION

void org_xmlvm_runtime_XMLVMArray_initNativeLayer__()
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_XMLVMArray_initNativeLayer__]
    __TIB_org_xmlvm_runtime_XMLVMArray.vtable[XMLVM_VTABLE_IDX_java_lang_Object_getClass__] = 
        (VTABLE_PTR) org_xmlvm_runtime_XMLVMArray_getClass__;
    //XMLVM_END_NATIVE
}

JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_createSingleDimension___java_lang_Class_int(JAVA_OBJECT n1, JAVA_INT n2)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_XMLVMArray_createSingleDimension___java_lang_Class_int]
    JAVA_OBJECT type = n1;
    JAVA_INT size = n2;
    TYPE_INFO info = XMLVMArray_getTypeInfo(type);
    int mem_size = info.sizeOfBaseType * size;
    JAVA_OBJECT data = info.useAtomicMalloc ? XMLVM_ATOMIC_MALLOC(mem_size) : XMLVM_MALLOC(mem_size);
    XMLVM_BZERO(data, mem_size);
    return org_xmlvm_runtime_XMLVMArray_createSingleDimensionWithData___java_lang_Class_int_java_lang_Object(type, size, data);
    //XMLVM_END_NATIVE
}

JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_createSingleDimensionWithData___java_lang_Class_int_java_lang_Object(JAVA_OBJECT n1, JAVA_INT n2, JAVA_OBJECT n3)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_XMLVMArray_createSingleDimensionWithData___java_lang_Class_int_java_lang_Object]
    org_xmlvm_runtime_XMLVMArray* array = __NEW_org_xmlvm_runtime_XMLVMArray();
    java_lang_Class* baseType = n1;
    __TIB_DEFINITION_TEMPLATE* baseTIB = baseType->fields.java_lang_Class.tib_;
    JAVA_OBJECT arrayType = baseTIB->arrayType;
    if (arrayType == JAVA_NULL) {
        XMLVM_INTERNAL_ERROR();
    }
    org_xmlvm_runtime_XMLVMArray___INIT____java_lang_Class_int_java_lang_Object(array, arrayType, n2, n3);
    return array;
    //XMLVM_END_NATIVE
}

JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_createMultiDimensions___java_lang_Class_org_xmlvm_runtime_XMLVMArray(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_XMLVMArray_createMultiDimensions___java_lang_Class_org_xmlvm_runtime_XMLVMArray]
    org_xmlvm_runtime_XMLVMArray* dimArray = (org_xmlvm_runtime_XMLVMArray*) n2;
    if (dimArray->fields.org_xmlvm_runtime_XMLVMArray.type_ != __CLASS_int_1ARRAY) {
        XMLVM_INTERNAL_ERROR();
    }
    int count = dimArray->fields.org_xmlvm_runtime_XMLVMArray.length_;
    JAVA_ARRAY_INT* data = (JAVA_ARRAY_INT*) dimArray->fields.org_xmlvm_runtime_XMLVMArray.array_;
    int i;
    java_lang_Class* arrayType = n1;
    for (i = 0; i < count - 1; i++) {
        __TIB_DEFINITION_TEMPLATE* arrayTIB = arrayType->fields.java_lang_Class.tib_;
        arrayType = arrayTIB->arrayType;
        if (arrayType == JAVA_NULL) {
            XMLVM_INTERNAL_ERROR();
        }
    }
    return XMLVMArray_createMultiDimensionsWithCount(arrayType, data, count);
    //XMLVM_END_NATIVE
}

void org_xmlvm_runtime_XMLVMArray_fillArray___org_xmlvm_runtime_XMLVMArray_java_lang_Object(JAVA_OBJECT n1, JAVA_OBJECT n2)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_XMLVMArray_fillArray___org_xmlvm_runtime_XMLVMArray_java_lang_Object]
    org_xmlvm_runtime_XMLVMArray* array = (org_xmlvm_runtime_XMLVMArray*) n1;
    java_lang_Class* arrayType = array->fields.org_xmlvm_runtime_XMLVMArray.type_;
    __TIB_DEFINITION_TEMPLATE* arrayTIB = arrayType->fields.java_lang_Class.tib_;
    TYPE_INFO info = XMLVMArray_getTypeInfo(arrayTIB->baseType);
    int n = info.sizeOfBaseType * array->fields.org_xmlvm_runtime_XMLVMArray.length_;
    XMLVM_MEMCPY(array->fields.org_xmlvm_runtime_XMLVMArray.array_, n2, n);
    //XMLVM_END_NATIVE
}

JAVA_OBJECT org_xmlvm_runtime_XMLVMArray_clone__(JAVA_OBJECT me)
{
    //XMLVM_BEGIN_NATIVE[org_xmlvm_runtime_XMLVMArray_clone__]
    org_xmlvm_runtime_XMLVMArray* thiz = (org_xmlvm_runtime_XMLVMArray*) me;
    JAVA_OBJECT type = thiz->fields.org_xmlvm_runtime_XMLVMArray.type_;
    JAVA_INT length = thiz->fields.org_xmlvm_runtime_XMLVMArray.length_;
    JAVA_OBJECT data = thiz->fields.org_xmlvm_runtime_XMLVMArray.array_;
    TYPE_INFO info = XMLVMArray_getTypeInfo(type);
    int sizeOfArrayInBytes = info.sizeOfBaseType * length;
    JAVA_OBJECT copyData = info.useAtomicMalloc ? XMLVM_ATOMIC_MALLOC(sizeOfArrayInBytes) : XMLVM_MALLOC(sizeOfArrayInBytes);
    XMLVM_MEMCPY(copyData, data, sizeOfArrayInBytes);
    return XMLVMArray_createSingleDimensionWithData(type, length, copyData);
    //XMLVM_END_NATIVE
}


void xmlvm_init_native_org_xmlvm_runtime_XMLVMArray()
{
    //XMLVM_BEGIN_NATIVE_IMPLEMENTATION_INIT
    //XMLVM_END_NATIVE_IMPLEMENTATION_INIT
#ifdef XMLVM_VTABLE_IDX_org_xmlvm_runtime_XMLVMArray_clone__
    __TIB_org_xmlvm_runtime_XMLVMArray.vtable[XMLVM_VTABLE_IDX_org_xmlvm_runtime_XMLVMArray_clone__] = 
        (VTABLE_PTR) org_xmlvm_runtime_XMLVMArray_clone__;
#endif
}
