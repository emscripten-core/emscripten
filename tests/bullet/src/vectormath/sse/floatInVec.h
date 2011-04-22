/*
   Copyright (C) 2006, 2007 Sony Computer Entertainment Inc.
   All rights reserved.

   Redistribution and use in source and binary forms,
   with or without modification, are permitted provided that the
   following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Sony Computer Entertainment Inc nor the names
      of its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _FLOATINVEC_H
#define _FLOATINVEC_H

#include <math.h>
#include <xmmintrin.h>

namespace Vectormath {

class boolInVec;

//--------------------------------------------------------------------------------------------------
// floatInVec class
//

class floatInVec
{
    private:
        __m128 mData;

    public:
        inline floatInVec(__m128 vec);

        inline floatInVec() {}

        // matches standard type conversions
        //
        inline floatInVec(const boolInVec &vec);

        // construct from a slot of __m128
        //
        inline floatInVec(__m128 vec, int slot);
        
        // explicit cast from float
        //
        explicit inline floatInVec(float scalar);

#ifdef _VECTORMATH_NO_SCALAR_CAST
        // explicit cast to float
        // 
        inline float getAsFloat() const;
#else
        // implicit cast to float
        //
        inline operator float() const;
#endif

        // get vector data
        // float value is splatted across all word slots of vector
        //
        inline __m128 get128() const;

        // operators
        // 
        inline const floatInVec operator ++ (int);
        inline const floatInVec operator -- (int);
        inline floatInVec& operator ++ ();
        inline floatInVec& operator -- ();
        inline const floatInVec operator - () const;
        inline floatInVec& operator = (const floatInVec &vec);
        inline floatInVec& operator *= (const floatInVec &vec);
        inline floatInVec& operator /= (const floatInVec &vec);
        inline floatInVec& operator += (const floatInVec &vec);
        inline floatInVec& operator -= (const floatInVec &vec);

        // friend functions
        //
        friend inline const floatInVec operator * (const floatInVec &vec0, const floatInVec &vec1);
        friend inline const floatInVec operator / (const floatInVec &vec0, const floatInVec &vec1);
        friend inline const floatInVec operator + (const floatInVec &vec0, const floatInVec &vec1);
        friend inline const floatInVec operator - (const floatInVec &vec0, const floatInVec &vec1);
        friend inline const floatInVec select(const floatInVec &vec0, const floatInVec &vec1, boolInVec select_vec1);
};

//--------------------------------------------------------------------------------------------------
// floatInVec functions
//

// operators
// 
inline const floatInVec operator * (const floatInVec &vec0, const floatInVec &vec1);
inline const floatInVec operator / (const floatInVec &vec0, const floatInVec &vec1);
inline const floatInVec operator + (const floatInVec &vec0, const floatInVec &vec1);
inline const floatInVec operator - (const floatInVec &vec0, const floatInVec &vec1);
inline const boolInVec operator < (const floatInVec &vec0, const floatInVec &vec1);
inline const boolInVec operator <= (const floatInVec &vec0, const floatInVec &vec1);
inline const boolInVec operator > (const floatInVec &vec0, const floatInVec &vec1);
inline const boolInVec operator >= (const floatInVec &vec0, const floatInVec &vec1);
inline const boolInVec operator == (const floatInVec &vec0, const floatInVec &vec1);
inline const boolInVec operator != (const floatInVec &vec0, const floatInVec &vec1);

// select between vec0 and vec1 using boolInVec.
// false selects vec0, true selects vec1
//
inline const floatInVec select(const floatInVec &vec0, const floatInVec &vec1, const boolInVec &select_vec1);

} // namespace Vectormath

//--------------------------------------------------------------------------------------------------
// floatInVec implementation
//

#include "boolInVec.h"

namespace Vectormath {

inline
floatInVec::floatInVec(__m128 vec)
{
    mData = vec;
}

inline
floatInVec::floatInVec(const boolInVec &vec)
{
	mData = vec_sel(_mm_setzero_ps(), _mm_set1_ps(1.0f), vec.get128());
}

inline
floatInVec::floatInVec(__m128 vec, int slot)
{
	SSEFloat v;
	v.m128 = vec;
	mData = _mm_set1_ps(v.f[slot]);
}

inline
floatInVec::floatInVec(float scalar)
{
	mData = _mm_set1_ps(scalar);
}

#ifdef _VECTORMATH_NO_SCALAR_CAST
inline
float
floatInVec::getAsFloat() const
#else
inline
floatInVec::operator float() const
#endif
{
    return *((float *)&mData);
}

inline
__m128
floatInVec::get128() const
{
    return mData;
}

inline
const floatInVec
floatInVec::operator ++ (int)
{
    __m128 olddata = mData;
    operator ++();
    return floatInVec(olddata);
}

inline
const floatInVec
floatInVec::operator -- (int)
{
    __m128 olddata = mData;
    operator --();
    return floatInVec(olddata);
}

inline
floatInVec&
floatInVec::operator ++ ()
{
    *this += floatInVec(_mm_set1_ps(1.0f));
    return *this;
}

inline
floatInVec&
floatInVec::operator -- ()
{
    *this -= floatInVec(_mm_set1_ps(1.0f));
    return *this;
}

inline
const floatInVec
floatInVec::operator - () const
{
    return floatInVec(_mm_sub_ps(_mm_setzero_ps(), mData));
}

inline
floatInVec&
floatInVec::operator = (const floatInVec &vec)
{
    mData = vec.mData;
    return *this;
}

inline
floatInVec&
floatInVec::operator *= (const floatInVec &vec)
{
    *this = *this * vec;
    return *this;
}

inline
floatInVec&
floatInVec::operator /= (const floatInVec &vec)
{
    *this = *this / vec;
    return *this;
}

inline
floatInVec&
floatInVec::operator += (const floatInVec &vec)
{
    *this = *this + vec;
    return *this;
}

inline
floatInVec&
floatInVec::operator -= (const floatInVec &vec)
{
    *this = *this - vec;
    return *this;
}

inline
const floatInVec
operator * (const floatInVec &vec0, const floatInVec &vec1)
{
    return floatInVec(_mm_mul_ps(vec0.get128(), vec1.get128()));
}

inline
const floatInVec
operator / (const floatInVec &num, const floatInVec &den)
{
    return floatInVec(_mm_div_ps(num.get128(), den.get128()));
}

inline
const floatInVec
operator + (const floatInVec &vec0, const floatInVec &vec1)
{
    return floatInVec(_mm_add_ps(vec0.get128(), vec1.get128()));
}

inline
const floatInVec
operator - (const floatInVec &vec0, const floatInVec &vec1)
{
    return floatInVec(_mm_sub_ps(vec0.get128(), vec1.get128()));
}

inline
const boolInVec
operator < (const floatInVec &vec0, const floatInVec &vec1)
{
    return boolInVec(_mm_cmpgt_ps(vec1.get128(), vec0.get128()));
}

inline
const boolInVec
operator <= (const floatInVec &vec0, const floatInVec &vec1)
{
    return boolInVec(_mm_cmpge_ps(vec1.get128(), vec0.get128()));
}

inline
const boolInVec
operator > (const floatInVec &vec0, const floatInVec &vec1)
{
    return boolInVec(_mm_cmpgt_ps(vec0.get128(), vec1.get128()));
}

inline
const boolInVec
operator >= (const floatInVec &vec0, const floatInVec &vec1)
{
    return boolInVec(_mm_cmpge_ps(vec0.get128(), vec1.get128()));
}

inline
const boolInVec
operator == (const floatInVec &vec0, const floatInVec &vec1)
{
    return boolInVec(_mm_cmpeq_ps(vec0.get128(), vec1.get128()));
}

inline
const boolInVec
operator != (const floatInVec &vec0, const floatInVec &vec1)
{
    return boolInVec(_mm_cmpneq_ps(vec0.get128(), vec1.get128()));
}
    
inline
const floatInVec
select(const floatInVec &vec0, const floatInVec &vec1, const boolInVec &select_vec1)
{
    return floatInVec(vec_sel(vec0.get128(), vec1.get128(), select_vec1.get128()));
}

} // namespace Vectormath

#endif // floatInVec_h
