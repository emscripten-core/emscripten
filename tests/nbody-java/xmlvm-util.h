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

#ifndef __XMLVM_UTIL_H__
#define __XMLVM_UTIL_H__

#include "xmlvm.h"

JAVA_OBJECT XMLVMUtil_NEW_ArrayList();
JAVA_INT XMLVMUtil_ArrayList_size(JAVA_OBJECT me);
JAVA_BOOLEAN XMLVMUtil_ArrayList_add(JAVA_OBJECT me, JAVA_OBJECT obj);
void XMLVMUtil_ArrayList_addAt(JAVA_OBJECT me, JAVA_INT index, JAVA_OBJECT obj);
JAVA_OBJECT XMLVMUtil_ArrayList_get(JAVA_OBJECT me, JAVA_INT idx);
JAVA_BOOLEAN XMLVMUtil_ArrayList_remove(JAVA_OBJECT me, JAVA_OBJECT obj);
JAVA_INT XMLVMUtil_ArrayList_indexOf(JAVA_OBJECT me, JAVA_OBJECT obj);

JAVA_OBJECT XMLVMUtil_NEW_HashSet();
JAVA_BOOLEAN XMLVMUtil_HashSet_add(JAVA_OBJECT me, JAVA_OBJECT obj);

JAVA_OBJECT XMLVMUtil_NEW_HashMap();
JAVA_OBJECT XMLVMUtil_HashMap_put(JAVA_OBJECT me, JAVA_OBJECT key, JAVA_OBJECT value);
JAVA_OBJECT XMLVMUtil_HashMap_get(JAVA_OBJECT me, JAVA_OBJECT key);

JAVA_OBJECT XMLVMUtil_getFromStringPool(JAVA_OBJECT str);

char* XMLVMUtil_convertFromByteArray(JAVA_OBJECT byteArray);

#endif
