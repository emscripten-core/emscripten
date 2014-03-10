
#include "xmlvm.h"
#include "org_apache_harmony_luni_util_NumberConverter.h"


//XMLVM_BEGIN_NATIVE_IMPLEMENTATION
#include "xmlvm-hy.h"
#include "hycomp.h"

#define RM_SIZE 21
#define STemp_SIZE 22
#define INV_LOG_OF_TEN_BASE_2 (0.30102999566398114)

//XMLVM_END_NATIVE_IMPLEMENTATION

void org_apache_harmony_luni_util_NumberConverter_bigIntDigitGeneratorInstImpl___long_int_boolean_boolean_int(JAVA_OBJECT me, JAVA_LONG n1, JAVA_INT n2, JAVA_BOOLEAN n3, JAVA_BOOLEAN n4, JAVA_INT n5)
{
    //XMLVM_BEGIN_NATIVE[org_apache_harmony_luni_util_NumberConverter_bigIntDigitGeneratorInstImpl___long_int_boolean_boolean_int]
		JAVA_LONG f = n1;
		JAVA_INT e = n2;
		JAVA_BOOLEAN isDenormalized = n3;
		JAVA_BOOLEAN mantissaIsZero = n4;
		JAVA_INT p = n5;

		JAVA_INT RLength, SLength, TempLength, mplus_Length, mminus_Length;
		JAVA_INT high, low, i;
		JAVA_LONG k, firstK, U;
		JAVA_LONG getCount, setCount;
		JAVA_ARRAY_INT* uArray;

		U_64 R[RM_SIZE], S[STemp_SIZE], mplus[RM_SIZE], mminus[RM_SIZE],Temp[STemp_SIZE];

		memset (R, 0, RM_SIZE * sizeof (U_64));
		memset (S, 0, STemp_SIZE * sizeof (U_64));
		memset (mplus, 0, RM_SIZE * sizeof (U_64));
		memset (mminus, 0, RM_SIZE * sizeof (U_64));
		memset (Temp, 0, STemp_SIZE * sizeof (U_64));

		if (e >= 0)
	    {
			*R = f;
			*mplus = *mminus = 1;
			simpleShiftLeftHighPrecision (mminus, RM_SIZE, e);
			if (f != (2 << (p - 1)))
	        {
				simpleShiftLeftHighPrecision (R, RM_SIZE, e + 1);
				*S = 2;
				simpleShiftLeftHighPrecision (mplus, RM_SIZE, e);
			}
			else {
				simpleShiftLeftHighPrecision (R, RM_SIZE, e+2);
				*S = 4;
				simpleShiftLeftHighPrecision (mplus, RM_SIZE, e + 1);

			}
		}
		else {
			if (isDenormalized || (f != (2 << (p - 1))))
	        {
				*R = f << 1;
				*S = 1;
				simpleShiftLeftHighPrecision (S, STemp_SIZE, 1 - e);
				*mplus = *mminus = 1;
	        }
			else
	        {
				*R = f << 2;
				*S = 1;
				simpleShiftLeftHighPrecision (S, STemp_SIZE, 2 - e);
				*mplus = 2;
				*mminus = 1;
	        }
		}
		k = (int) ceil ((e + p - 1) * INV_LOG_OF_TEN_BASE_2 - 1e-10);

		if (k > 0)
	    {
			timesTenToTheEHighPrecision (S, STemp_SIZE, k);
	    }
		else
	    {
			timesTenToTheEHighPrecision (R, RM_SIZE, -k);
			timesTenToTheEHighPrecision (mplus, RM_SIZE, -k);
			timesTenToTheEHighPrecision (mminus, RM_SIZE, -k);
	    }

		RLength = mplus_Length = mminus_Length = RM_SIZE;
		SLength = TempLength = STemp_SIZE;

		memset (Temp + RM_SIZE, 0, (STemp_SIZE - RM_SIZE) * sizeof (U_64));
		memcpy (Temp, R, RM_SIZE * sizeof (U_64));

		while (RLength > 1 && R[RLength - 1] == 0)
			--RLength;
		while (mplus_Length > 1 && mplus[mplus_Length - 1] == 0)
			--mplus_Length;
		while (mminus_Length > 1 && mminus[mminus_Length - 1] == 0)
			--mminus_Length;
		while (SLength > 1 && S[SLength - 1] == 0)
			--SLength;
		TempLength = (RLength > mplus_Length ? RLength : mplus_Length) + 1;
		addHighPrecision (Temp, TempLength, mplus, mplus_Length);

		if (compareHighPrecision (Temp, TempLength, S, SLength) >= 0)
	    {
			firstK = k;
	    }
		else
	    {
			firstK = k - 1;
			simpleAppendDecimalDigitHighPrecision (R, ++RLength, 0);
			simpleAppendDecimalDigitHighPrecision (mplus, ++mplus_Length, 0);
			simpleAppendDecimalDigitHighPrecision (mminus, ++mminus_Length, 0);
			while (RLength > 1 && R[RLength - 1] == 0)
				--RLength;
			while (mplus_Length > 1 && mplus[mplus_Length - 1] == 0)
				--mplus_Length;
			while (mminus_Length > 1 && mminus[mminus_Length - 1] == 0)
				--mminus_Length;
	    }

		org_apache_harmony_luni_util_NumberConverter* inst = (org_apache_harmony_luni_util_NumberConverter*)me;
		org_xmlvm_runtime_XMLVMArray* uArrayObject = inst->fields.org_apache_harmony_luni_util_NumberConverter.uArray_;
		uArray = uArrayObject->fields.org_xmlvm_runtime_XMLVMArray.array_;

		getCount = setCount = 0;
		do
	    {
			U = 0;
			for (i = 3; i >= 0; --i)
	        {
				TempLength = SLength + 1;
				Temp[SLength] = 0;
				memcpy (Temp, S, SLength * sizeof (U_64));
				simpleShiftLeftHighPrecision (Temp, TempLength, i);
				if (compareHighPrecision (R, RLength, Temp, TempLength) >= 0)
	            {
					subtractHighPrecision (R, RLength, Temp, TempLength);
					U += 1 << i;
	            }
	        }

			low = compareHighPrecision (R, RLength, mminus, mminus_Length) <= 0;

			memset (Temp + RLength, 0, (STemp_SIZE - RLength) * sizeof (U_64));
			memcpy (Temp, R, RLength * sizeof (U_64));
			TempLength = (RLength > mplus_Length ? RLength : mplus_Length) + 1;
			addHighPrecision (Temp, TempLength, mplus, mplus_Length);

			high = compareHighPrecision (Temp, TempLength, S, SLength) >= 0;

			if (low || high)
				break;

			simpleAppendDecimalDigitHighPrecision (R, ++RLength, 0);
			simpleAppendDecimalDigitHighPrecision (mplus, ++mplus_Length, 0);
			simpleAppendDecimalDigitHighPrecision (mminus, ++mminus_Length, 0);
			while (RLength > 1 && R[RLength - 1] == 0)
				--RLength;
			while (mplus_Length > 1 && mplus[mplus_Length - 1] == 0)
				--mplus_Length;
			while (mminus_Length > 1 && mminus[mminus_Length - 1] == 0)
				--mminus_Length;
			uArray[setCount++] = U;

		}
		while (1);

		simpleShiftLeftHighPrecision (R, ++RLength, 1);
		if (low && !high)
			uArray[setCount++] = U;
		else if (high && !low)
			uArray[setCount++] = U + 1;
		else if (compareHighPrecision (R, RLength, S, SLength) < 0)
			uArray[setCount++] = U;
		else
			uArray[setCount++] = U + 1;

		inst->fields.org_apache_harmony_luni_util_NumberConverter.setCount_ = setCount;
		inst->fields.org_apache_harmony_luni_util_NumberConverter.getCount_ = getCount;
		inst->fields.org_apache_harmony_luni_util_NumberConverter.firstK_ = firstK;


    //XMLVM_END_NATIVE
}

