
#ifndef _JS_INT_64_H_
#define _JS_INT_64_H_

#include <stdint.h>

typedef uint32_t JSUint64;
//#define JSint64  uint32_t
//#define JSUint64 uint32_t

JSUint64 jsuint64_create();

void jsuint64_plusjs(JSUint64 instance, const JSUint64 value);
void jsuint64_minusjs(JSUint64 instance, const JSUint64 value);
void jsuint64_bitandi(JSUint64 instance, const JSUint64 value);
void jsuint64_bitori(JSUint64 instance, const JSUint64 value);
void jsuint64_muli(JSUint64 instance, const JSUint64 value);
void jsuint64_divi(JSUint64 instance, const JSUint64 value);

void jsuint64_setbit(JSUint64 instance, unsigned short index);
void jsuint64_bitAnd(JSUint64 instance, JSUint64 value2);
void jsuint64_bitOr (JSUint64 instance, JSUint64 value2);
bool jsuint64_isSet (JSUint64 instance);

void jsuint64_destroy(JSUint64 value);

#endif //_JS_INT_64_H_

