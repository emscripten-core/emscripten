/* -*- mode: C -*-
 *
 * Copyright (c) 2007-2010 The University of Utah
 * All rights reserved.
 *
 * This file is part of `csmith', a random generator of C programs.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RANDOM_INC_H
#define RANDOM_INC_H


#if defined(STANDALONE)
  #if defined(_MSC_VER)
    #include <limits.h>
    #include "windows/stdint.h"
  #elif defined (IA32_ARCH)
    #include "stdint_ia32.h"
  #elif defined (IA64_ARCH)
    #include "stdint_ia64.h"
  #elif defined (MSP430)
    #include "stdint_msp430.h"
  #elif defined (AVR_ARCH)
    #include "stdint_avr.h"
  #else
    #include "stdint_ia32.h"
  #endif
#else
  #include <limits.h>
  #if defined(_MSC_VER)
    #include "windows/stdint.h"
  #else
    #include <stdint.h>
  #endif
#endif

#include <assert.h>

/*****************************************************************************/

#ifndef DEPUTY
#define COUNT(n)
#define TC
#define SAFE
#endif

/*****************************************************************************/

#ifdef LOG_WRAPPERS
#include "wrapper.h"
char __failed_checks[N_WRAP+1];
char __executed_checks[N_WRAP+1];
#define UNDEFINED(__val) (__failed_checks[index]=1,(__val))
#define LOG_INDEX , int index
#define LOG_EXEC __executed_checks[index]=1;
#else
#define UNDEFINED(__val) (__val)
#define LOG_INDEX
#define LOG_EXEC
#endif

#if defined(AVR_ARCH)
#include "platform_avr.h"
#elif defined (MSP430)
#include "platform_msp430.h"
#else
#include "platform_generic.h"
#endif

#define STATIC static

#if defined (USE_MATH_MACROS_NOTMP)
#include "safe_math_macros_notmp.h"
#elif defined (USE_MATH_MACROS)
#include "safe_math_macros.h"
#else
#define FUNC_NAME(x) (safe_##x)
#include "safe_math.h"
#undef FUNC_NAME
#endif

#define INT_BIT (sizeof(int)*CHAR_BIT)
#define _CSMITH_BITFIELD(x) (((x)>INT_BIT)?((x)%INT_BIT):(x))

#ifdef TCC

void* memcpy(void* dest, const void* src, size_t count) {
  char* dst8 = (char*)dest;
  char* src8 = (char*)src;
  
  while (count--) {
    *dst8++ = *src8++;
  }
  return dest;
}

void *memset(void *s, int c, size_t n)
{
    unsigned char* p=s;
    while(n--)
        *p++ = (unsigned char)c;
    return s;
}

#endif

#endif // RANDOM_INC_H
