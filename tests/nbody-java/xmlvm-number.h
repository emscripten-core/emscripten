/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  cbigint.c has been adapted for xmlvm
 */
#ifndef __XMLVM_NUMBER_H__
#define __XMLVM_NUMBER_H__

#include <string.h>
#include <math.h>
#include "xmlvm-hy.h"
#include "hycomp.h"

#define LONG_LO_OFFSET			0
#define LONG_HI_OFFSET			1

#define LOW_U32_FROM_LONG64_PTR(long64ptr) (((U64U32DBL *)(long64ptr))->u32val[LONG_LO_OFFSET])
#define HIGH_U32_FROM_LONG64_PTR(long64ptr) (((U64U32DBL *)(long64ptr))->u32val[LONG_HI_OFFSET])
#define LOW_U32_FROM_VAR(u64)     LOW_U32_FROM_LONG64_PTR(&(u64))
#define LOW_U32_FROM_PTR(u64ptr)  LOW_U32_FROM_LONG64_PTR(u64ptr)
#define HIGH_U32_FROM_VAR(u64)    HIGH_U32_FROM_LONG64_PTR(&(u64))
#define HIGH_U32_FROM_PTR(u64ptr) HIGH_U32_FROM_LONG64_PTR(u64ptr)


#define HIGH_IN_U64(u64) ((u64) >> 32)
#if defined(USE_LL)
#define LOW_IN_U64(u64) ((u64) & 0x00000000FFFFFFFFLL)
#else
#if defined(USE_L)
#define LOW_IN_U64(u64) ((u64) & 0x00000000FFFFFFFFL)
#else
#define LOW_IN_U64(u64) ((u64) & 0x00000000FFFFFFFF)
#endif /* USE_L */
#endif /* USE_LL */

typedef union u64u32dbl_tag {
	U_64    u64val;
	U_32    u32val[2];
    I_32    i32val[2];
	double  dval;
} U64U32DBL;

#if defined(USE_LL)
#define TEN_E1 (0xALL)
#define TEN_E2 (0x64LL)
#define TEN_E3 (0x3E8LL)
#define TEN_E4 (0x2710LL)
#define TEN_E5 (0x186A0LL)
#define TEN_E6 (0xF4240LL)
#define TEN_E7 (0x989680LL)
#define TEN_E8 (0x5F5E100LL)
#define TEN_E9 (0x3B9ACA00LL)
#define TEN_E19 (0x8AC7230489E80000LL)
#else
#if defined(USE_L)
#define TEN_E1 (0xAL)
#define TEN_E2 (0x64L)
#define TEN_E3 (0x3E8L)
#define TEN_E4 (0x2710L)
#define TEN_E5 (0x186A0L)
#define TEN_E6 (0xF4240L)
#define TEN_E7 (0x989680L)
#define TEN_E8 (0x5F5E100L)
#define TEN_E9 (0x3B9ACA00L)
#define TEN_E19 (0x8AC7230489E80000L)
#else
#define TEN_E1 (0xA)
#define TEN_E2 (0x64)
#define TEN_E3 (0x3E8)
#define TEN_E4 (0x2710)
#define TEN_E5 (0x186A0)
#define TEN_E6 (0xF4240)
#define TEN_E7 (0x989680)
#define TEN_E8 (0x5F5E100)
#define TEN_E9 (0x3B9ACA00)
#define TEN_E19 (0x8AC7230489E80000)
#endif /* USE_L */
#endif /* USE_LL */

#define TIMES_TEN(x) (((x) << 3) + ((x) << 1))

U_32 simpleMultiplyHighPrecision (U_64 * arg1, IDATA length, U_64 arg2);
void simpleShiftLeftHighPrecision (U_64 * arg1, IDATA length, IDATA arg2);
U_64 simpleMultiplyHighPrecision64 (U_64 * arg1, IDATA length, U_64 arg2);
U_32 simpleAppendDecimalDigitHighPrecision (U_64 * arg1, IDATA length, U_64 digit);
IDATA timesTenToTheEHighPrecision (U_64 * result, IDATA length, JAVA_INT e);
IDATA addHighPrecision (U_64 * arg1, IDATA length1, U_64 * arg2, IDATA length2);
IDATA compareHighPrecision (U_64 * arg1, IDATA length1, U_64 * arg2, IDATA length2);
IDATA simpleAddHighPrecision (U_64 * arg1, IDATA length, U_64 arg2);
void subtractHighPrecision (U_64 * arg1, IDATA length1, U_64 * arg2, IDATA length2);

#endif
