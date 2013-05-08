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
#include "xmlvm-number.h"


U_32
simpleMultiplyHighPrecision (U_64 * arg1, IDATA length, U_64 arg2)
{
	/* assumes arg2 only holds 32 bits of information */
	U_64 product;
	IDATA index;

	index = 0;
	product = 0;

	do
    {
		product =
        HIGH_IN_U64 (product) + arg2 * LOW_U32_FROM_PTR (arg1 + index);
		LOW_U32_FROM_PTR (arg1 + index) = LOW_U32_FROM_VAR (product);
		product =
        HIGH_IN_U64 (product) + arg2 * HIGH_U32_FROM_PTR (arg1 + index);
		HIGH_U32_FROM_PTR (arg1 + index) = LOW_U32_FROM_VAR (product);
    }
	while (++index < length);

	return HIGH_U32_FROM_VAR (product);
}

void simpleShiftLeftHighPrecision (U_64 * arg1, IDATA length, IDATA arg2)
{
	/* assumes length > 0 */
	IDATA index, offset;
	if (arg2 >= 64)
    {
		offset = arg2 >> 6;
		index = length;

		while (--index - offset >= 0)
			arg1[index] = arg1[index - offset];
		do
        {
			arg1[index] = 0;
        }
		while (--index >= 0);

		arg2 &= 0x3F;
    }

	if (arg2 == 0)
		return;
	while (--length > 0)
    {
		arg1[length] = arg1[length] << arg2 | arg1[length - 1] >> (64 - arg2);
    }
	*arg1 <<= arg2;
}


U_64 simpleMultiplyHighPrecision64 (U_64 * arg1, IDATA length, U_64 arg2)
{
	U_64 intermediate, *pArg1, carry1, carry2, prod1, prod2, sum;
	IDATA index;
	U_32 buf32;

	index = 0;
	intermediate = 0;
	pArg1 = arg1 + index;
	carry1 = carry2 = 0;

	do
    {
		if ((*pArg1 != 0) || (intermediate != 0))
        {
			prod1 =
            (U_64) LOW_U32_FROM_VAR (arg2) * (U_64) LOW_U32_FROM_PTR (pArg1);
			sum = intermediate + prod1;
			if ((sum < prod1) || (sum < intermediate))
            {
				carry1 = 1;
            }
			else
            {
				carry1 = 0;
            }
			prod1 =
            (U_64) LOW_U32_FROM_VAR (arg2) * (U_64) HIGH_U32_FROM_PTR (pArg1);
			prod2 =
            (U_64) HIGH_U32_FROM_VAR (arg2) * (U_64) LOW_U32_FROM_PTR (pArg1);
			intermediate = carry2 + HIGH_IN_U64 (sum) + prod1 + prod2;
			if ((intermediate < prod1) || (intermediate < prod2))
            {
				carry2 = 1;
            }
			else
            {
				carry2 = 0;
            }
			LOW_U32_FROM_PTR (pArg1) = LOW_U32_FROM_VAR (sum);
			buf32 = HIGH_U32_FROM_PTR (pArg1);
			HIGH_U32_FROM_PTR (pArg1) = LOW_U32_FROM_VAR (intermediate);
			intermediate = carry1 + HIGH_IN_U64 (intermediate)
            + (U_64) HIGH_U32_FROM_VAR (arg2) * (U_64) buf32;
        }
		pArg1++;
    }
	while (++index < length);
	return intermediate;
}

U_32 simpleAppendDecimalDigitHighPrecision (U_64 * arg1, IDATA length, U_64 digit)
{
	/* assumes digit is less than 32 bits */
	U_64 arg;
	IDATA index = 0;

	digit <<= 32;
	do
    {
		arg = LOW_IN_U64 (arg1[index]);
		digit = HIGH_IN_U64 (digit) + TIMES_TEN (arg);
		LOW_U32_FROM_PTR (arg1 + index) = LOW_U32_FROM_VAR (digit);

		arg = HIGH_IN_U64 (arg1[index]);
		digit = HIGH_IN_U64 (digit) + TIMES_TEN (arg);
		HIGH_U32_FROM_PTR (arg1 + index) = LOW_U32_FROM_VAR (digit);
    }
	while (++index < length);

	return HIGH_U32_FROM_VAR (digit);
}

IDATA timesTenToTheEHighPrecision (U_64 * result, IDATA length, JAVA_INT e)
{
	/* assumes result can hold value */
	U_64 overflow;
	int exp10 = e;

	if (e == 0)
		return length;

	while (exp10 >= 19)
    {
		overflow = simpleMultiplyHighPrecision64 (result, length, TEN_E19);
		if (overflow)
			result[length++] = overflow;
		exp10 -= 19;
    }
	while (exp10 >= 9)
    {
		overflow = simpleMultiplyHighPrecision (result, length, TEN_E9);
		if (overflow)
			result[length++] = overflow;
		exp10 -= 9;
    }
	if (exp10 == 0)
		return length;
	else if (exp10 == 1)
    {
		overflow = simpleAppendDecimalDigitHighPrecision (result, length, 0);
		if (overflow)
			result[length++] = overflow;
    }
	else if (exp10 == 2)
    {
		overflow = simpleAppendDecimalDigitHighPrecision (result, length, 0);
		if (overflow)
			result[length++] = overflow;
		overflow = simpleAppendDecimalDigitHighPrecision (result, length, 0);
		if (overflow)
			result[length++] = overflow;
    }
	else if (exp10 == 3)
    {
		overflow = simpleMultiplyHighPrecision (result, length, TEN_E3);
		if (overflow)
			result[length++] = overflow;
    }
	else if (exp10 == 4)
    {
		overflow = simpleMultiplyHighPrecision (result, length, TEN_E4);
		if (overflow)
			result[length++] = overflow;
    }
	else if (exp10 == 5)
    {
		overflow = simpleMultiplyHighPrecision (result, length, TEN_E5);
		if (overflow)
			result[length++] = overflow;
    }
	else if (exp10 == 6)
    {
		overflow = simpleMultiplyHighPrecision (result, length, TEN_E6);
		if (overflow)
			result[length++] = overflow;
    }
	else if (exp10 == 7)
    {
		overflow = simpleMultiplyHighPrecision (result, length, TEN_E7);
		if (overflow)
			result[length++] = overflow;
    }
	else if (exp10 == 8)
    {
		overflow = simpleMultiplyHighPrecision (result, length, TEN_E8);
		if (overflow)
			result[length++] = overflow;
    }
	return length;
}

IDATA addHighPrecision (U_64 * arg1, IDATA length1, U_64 * arg2, IDATA length2)
{

	U_64 temp1, temp2, temp3;     /* temporary variables to help the SH-4, and gcc */
	U_64 carry;
	IDATA index;

	if (length1 == 0 || length2 == 0)
    {
		return 0;
    }
	else if (length1 < length2)
    {
		length2 = length1;
    }

	carry = 0;
	index = 0;
	do
    {
		temp1 = arg1[index];
		temp2 = arg2[index];
		temp3 = temp1 + temp2;
		arg1[index] = temp3 + carry;
		if (arg2[index] < arg1[index])
			carry = 0;
		else if (arg2[index] != arg1[index])
			carry = 1;
    }
	while (++index < length2);
	if (!carry)
		return 0;
	else if (index == length1)
		return 1;

	while (++arg1[index] == 0 && ++index < length1);

	return (IDATA) index == length1;
}

IDATA
compareHighPrecision (U_64 * arg1, IDATA length1, U_64 * arg2, IDATA length2)
{
	while (--length1 >= 0 && arg1[length1] == 0);
	while (--length2 >= 0 && arg2[length2] == 0);

	if (length1 > length2)
		return 1;
	else if (length1 < length2)
		return -1;
	else if (length1 > -1)
    {
		do
        {
			if (arg1[length1] > arg2[length1])
				return 1;
			else if (arg1[length1] < arg2[length1])
				return -1;
        }
		while (--length1 >= 0);
    }

	return 0;
}

IDATA
simpleAddHighPrecision (U_64 * arg1, IDATA length, U_64 arg2)
{
	/* assumes length > 0 */
	IDATA index = 1;

	*arg1 += arg2;
	if (arg2 <= *arg1)
		return 0;
	else if (length == 1)
		return 1;

	while (++arg1[index] == 0 && ++index < length);

	return (IDATA) index == length;
}

void
subtractHighPrecision (U_64 * arg1, IDATA length1, U_64 * arg2, IDATA length2)
{
	/* assumes arg1 > arg2 */
	IDATA index;
	for (index = 0; index < length1; ++index)
		arg1[index] = ~arg1[index];
	simpleAddHighPrecision (arg1, length1, 1);

	while (length2 > 0 && arg2[length2 - 1] == 0)
		--length2;

	addHighPrecision (arg1, length1, arg2, length2);

	for (index = 0; index < length1; ++index)
		arg1[index] = ~arg1[index];
	simpleAddHighPrecision (arg1, length1, 1);
}
