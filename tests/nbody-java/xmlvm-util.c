/*
 * Copyright (c) 2002-2011 by XMLVM.org
 *
 * Project Info:  http://www.xmlvm.org
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include "xmlvm-util.h"
#include "java_util_ArrayList.h"
#include "java_util_HashSet.h"
#include "java_util_HashMap.h"
#include "java_util_Map_Entry.h"


/**** ArrayList Utilities ********************************************************************/
JAVA_OBJECT XMLVMUtil_NEW_ArrayList()
{
    JAVA_OBJECT obj = __NEW_java_util_ArrayList();
    java_util_ArrayList___INIT___(obj);
    return obj;
}

JAVA_INT XMLVMUtil_ArrayList_size(JAVA_OBJECT me)
{
#ifdef XMLVM_VTABLE_IDX_java_util_ArrayList_size__
    return (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_ArrayList*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_ArrayList_size__])(me);
#else
    return java_util_ArrayList_size__(me);
#endif
}

JAVA_BOOLEAN XMLVMUtil_ArrayList_add(JAVA_OBJECT me, JAVA_OBJECT obj)
{
#ifdef XMLVM_VTABLE_IDX_java_util_ArrayList_add___java_lang_Object
    return (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_ArrayList_add___java_lang_Object])(me, obj);
#else
    return java_util_ArrayList_add___java_lang_Object(me, obj);
#endif
}

void XMLVMUtil_ArrayList_addAt(JAVA_OBJECT me, JAVA_INT index, JAVA_OBJECT obj)
{
#ifdef XMLVM_VTABLE_IDX_java_util_ArrayList_add___int_java_lang_Object
    (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_INT, JAVA_OBJECT)) ((java_util_ArrayList*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_ArrayList_add___int_java_lang_Object])(me, index, obj);
#else
    java_util_ArrayList_add___int_java_lang_Object(me, index, obj);
#endif
}

JAVA_OBJECT XMLVMUtil_ArrayList_get(JAVA_OBJECT me, JAVA_INT idx)
{
#ifdef XMLVM_VTABLE_IDX_java_util_ArrayList_get___int
    return (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_INT)) ((java_util_ArrayList*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_ArrayList_get___int])(me, idx);
#else
    return java_util_ArrayList_get___int(me, idx);
#endif
}

JAVA_BOOLEAN XMLVMUtil_ArrayList_remove(JAVA_OBJECT me, JAVA_OBJECT obj)
{
#ifdef XMLVM_VTABLE_IDX_java_util_ArrayList_remove___java_lang_Object
    return (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_ArrayList_remove___java_lang_Object])(me, obj);
#else
    return java_util_ArrayList_remove___java_lang_Object(me, obj);
#endif
}

JAVA_INT XMLVMUtil_ArrayList_indexOf(JAVA_OBJECT me, JAVA_OBJECT obj)
{
#ifdef XMLVM_VTABLE_IDX_java_util_ArrayList_indexOf___java_lang_Object
    return (*(JAVA_INT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_ArrayList*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_ArrayList_indexOf___java_lang_Object])(me, obj);
#else
    return java_util_ArrayList_indexOf___java_lang_Object(me, obj);
#endif
}

/**** HashSet Utilities ********************************************************************/
JAVA_OBJECT XMLVMUtil_NEW_HashSet()
{
    JAVA_OBJECT obj = __NEW_java_util_HashSet();
    java_util_HashSet___INIT___(obj);
    return obj;
}

JAVA_BOOLEAN XMLVMUtil_HashSet_add(JAVA_OBJECT me, JAVA_OBJECT obj)
{
#ifdef XMLVM_VTABLE_IDX_java_util_HashSet_add___java_lang_Object
    return (*(JAVA_BOOLEAN (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashSet*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_HashSet_add___java_lang_Object])(me, obj);
#else
    return java_util_HashSet_add___java_lang_Object(me, obj);
#endif
}

JAVA_INT XMLVMUtil_HashSet_iterator(JAVA_OBJECT me)
{
#ifdef XMLVM_VTABLE_IDX_java_util_HashSet_iterator__
    return (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_HashSet*) me)->
            tib->vtable[XMLVM_ITABLE_IDX_java_util_Set_iterator__])(me);
#else
    return java_util_HashSet_iterator__(me);
#endif
}

JAVA_INT XMLVMUtil_Iterator_hasNext(JAVA_OBJECT me)
{
#ifdef XMLVM_VTABLE_IDX_java_util_Iterator_hasNext__
    return (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_Iterator*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_Iterator_hasNext__])(me);
#endif
}

JAVA_INT XMLVMUtil_Iterator_next(JAVA_OBJECT me)
{
#ifdef XMLVM_VTABLE_IDX_java_util_Iterator_next__
    return (*(JAVA_INT (*)(JAVA_OBJECT)) ((java_util_Iterator*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_Iterator_next__])(me);
#endif
}

/**** HashMap Utilities ********************************************************************/
JAVA_OBJECT XMLVMUtil_NEW_HashMap()
{
    JAVA_OBJECT obj = __NEW_java_util_HashMap();
    java_util_HashMap___INIT___(obj);
    return obj;
}

JAVA_OBJECT XMLVMUtil_HashMap_put(JAVA_OBJECT me, JAVA_OBJECT key, JAVA_OBJECT value)
{
#ifdef XMLVM_VTABLE_IDX_java_util_HashMap_put___java_lang_Object_java_lang_Object
    return (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_HashMap_put___java_lang_Object_java_lang_Object])(me, key, value);
#else
    return java_util_HashMap_put___java_lang_Object_java_lang_Object(me, key, value);
#endif
}

JAVA_OBJECT XMLVMUtil_HashMap_get(JAVA_OBJECT me, JAVA_OBJECT key)
{
#ifdef XMLVM_VTABLE_IDX_java_util_HashMap_get___java_lang_Object
    return (*(JAVA_OBJECT (*)(JAVA_OBJECT, JAVA_OBJECT)) ((java_util_HashMap*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_HashMap_get___java_lang_Object])(me, key);
#else
    return java_util_HashMap_get___java_lang_Object(me, key);
#endif
}

JAVA_OBJECT XMLVMUtil_HashMap_entrySet(JAVA_OBJECT me)
{
#ifdef XMLVM_VTABLE_IDX_java_util_HashMap_entrySet__
    return (*(JAVA_OBJECT (*)(JAVA_OBJECT)) ((java_util_HashMap*) me)->
            tib->vtable[XMLVM_VTABLE_IDX_java_util_HashMap_entrySet__])(me);
#else
	return java_util_HashMap_entrySet__(me);
#endif
}

JAVA_OBJECT XMLVMUtil_MapEntry_getKey(JAVA_OBJECT me)
{
    return (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*) me)->
		    tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getKey__])(me);
}

JAVA_OBJECT XMLVMUtil_MapEntry_getValue(JAVA_OBJECT me)
{
    return (*(JAVA_OBJECT (*)(JAVA_OBJECT)) *(((java_lang_Object*) me)->
			tib->itableBegin)[XMLVM_ITABLE_IDX_java_util_Map_Entry_getValue__])(me);
}


/**** ConstantStringPool Utilities ************************************************************/
static JAVA_OBJECT stringPool = JAVA_NULL;

JAVA_OBJECT XMLVMUtil_getFromStringPool(JAVA_OBJECT str)
{
    if (stringPool == JAVA_NULL) {
        stringPool = XMLVMUtil_NEW_ArrayList();
    }
    JAVA_INT i = XMLVMUtil_ArrayList_indexOf(stringPool, str);
    if (i == -1) {
        XMLVMUtil_ArrayList_add(stringPool, str);
        return str;
    }
    return XMLVMUtil_ArrayList_get(stringPool, i);
}


/**** Misc Utilities **********************************************************************/

char* XMLVMUtil_convertFromByteArray(JAVA_OBJECT byteArray) {
    org_xmlvm_runtime_XMLVMArray* a = byteArray;
    char* data = (char*) a->fields.org_xmlvm_runtime_XMLVMArray.array_;
    int length = a->fields.org_xmlvm_runtime_XMLVMArray.length_;
    char* buf = XMLVM_ATOMIC_MALLOC(length + 1);
    XMLVM_MEMCPY(buf, data, length);
    buf[length] = '\0';
    return buf;
}

