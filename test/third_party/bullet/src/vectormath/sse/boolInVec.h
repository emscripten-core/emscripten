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

#ifndef _BOOLINVEC_H
#define _BOOLINVEC_H

#include <math.h>

namespace Vectormath {

class floatInVec;

//--------------------------------------------------------------------------------------------------
// boolInVec class
//

class boolInVec
{
    private:
        __m128 mData;

        inline boolInVec(__m128 vec);
    public:
        inline boolInVec() {}

        // matches standard type conversions
        //
        inline boolInVec(const floatInVec &vec);

        // explicit cast from bool
        //
        explicit inline boolInVec(bool scalar);

#ifdef _VECTORMATH_NO_SCALAR_CAST
        // explicit cast to bool
        // 
        inline bool getAsBool() const;
#else
        // implicit cast to bool
        // 
        inline operator bool() const;
#endif
        
        // get vector data
        // bool value is splatted across all word slots of vector as 0 (false) or -1 (true)
        //
        inline __m128 get128() const;

        // operators
        //
        inline const boolInVec operator ! () const;
        inline boolInVec& operator = (const boolInVec &vec);
        inline boolInVec& operator &= (const boolInVec &vec);
        inline boolInVec& operator ^= (const boolInVec &vec);
        inline boolInVec& operator |= (const boolInVec &vec);

        // friend functions
        //
        friend inline const boolInVec operator == (const boolInVec &vec0, const boolInVec &vec1);
        friend inline const boolInVec operator != (const boolInVec &vec0, const boolInVec &vec1);
        friend inline const boolInVec operator < (const floatInVec &vec0, const floatInVec &vec1);
        friend inline const boolInVec operator <= (const floatInVec &vec0, const floatInVec &vec1);
        friend inline const boolInVec operator > (const floatInVec &vec0, const floatInVec &vec1);
        friend inline const boolInVec operator >= (const floatInVec &vec0, const floatInVec &vec1);
        friend inline const boolInVec operator == (const floatInVec &vec0, const floatInVec &vec1);
        friend inline const boolInVec operator != (const floatInVec &vec0, const floatInVec &vec1);
        friend inline const boolInVec operator & (const boolInVec &vec0, const boolInVec &vec1);
        friend inline const boolInVec operator ^ (const boolInVec &vec0, const boolInVec &vec1);
        friend inline const boolInVec operator | (const boolInVec &vec0, const boolInVec &vec1);
        friend inline const boolInVec select(const boolInVec &vec0, const boolInVec &vec1, const boolInVec &select_vec1);
};

//--------------------------------------------------------------------------------------------------
// boolInVec functions
//

// operators
//
inline const boolInVec operator == (const boolInVec &vec0, const boolInVec &vec1);
inline const boolInVec operator != (const boolInVec &vec0, const boolInVec &vec1);
inline const boolInVec operator & (const boolInVec &vec0, const boolInVec &vec1);
inline const boolInVec operator ^ (const boolInVec &vec0, const boolInVec &vec1);
inline const boolInVec operator | (const boolInVec &vec0, const boolInVec &vec1);

// select between vec0 and vec1 using boolInVec.
// false selects vec0, true selects vec1
//
inline const boolInVec select(const boolInVec &vec0, const boolInVec &vec1, const boolInVec &select_vec1);

} // namespace Vectormath

//--------------------------------------------------------------------------------------------------
// boolInVec implementation
//

#include "floatInVec.h"

namespace Vectormath {

inline
boolInVec::boolInVec(__m128 vec)
{
    mData = vec;
}

inline
boolInVec::boolInVec(const floatInVec &vec)
{
    *this = (vec != floatInVec(0.0f));
}

inline
boolInVec::boolInVec(bool scalar)
{
    unsigned int mask = -(int)scalar;
	mData = _mm_set1_ps(*(float *)&mask); // TODO: Union
}

#ifdef _VECTORMATH_NO_SCALAR_CAST
inline
bool
boolInVec::getAsBool() const
#else
inline
boolInVec::operator bool() const
#endif
{
	return *(bool *)&mData;
}

inline
__m128
boolInVec::get128() const
{
    return mData;
}

inline
const boolInVec
boolInVec::operator ! () const
{
    return boolInVec(_mm_andnot_ps(mData, _mm_cmpneq_ps(_mm_setzero_ps(),_mm_setzero_ps())));
}

inline
boolInVec&
boolInVec::operator = (const boolInVec &vec)
{
    mData = vec.mData;
    return *this;
}

inline
boolInVec&
boolInVec::operator &= (const boolInVec &vec)
{
    *this = *this & vec;
    return *this;
}

inline
boolInVec&
boolInVec::operator ^= (const boolInVec &vec)
{
    *this = *this ^ vec;
    return *this;
}

inline
boolInVec&
boolInVec::operator |= (const boolInVec &vec)
{
    *this = *this | vec;
    return *this;
}

inline
const boolInVec
operator == (const boolInVec &vec0, const boolInVec &vec1)
{
	return boolInVec(_mm_cmpeq_ps(vec0.get128(), vec1.get128()));
}

inline
const boolInVec
operator != (const boolInVec &vec0, const boolInVec &vec1)
{
	return boolInVec(_mm_cmpneq_ps(vec0.get128(), vec1.get128()));
}
    
inline
const boolInVec
operator & (const boolInVec &vec0, const boolInVec &vec1)
{
	return boolInVec(_mm_and_ps(vec0.get128(), vec1.get128()));
}

inline
const boolInVec
operator | (const boolInVec &vec0, const boolInVec &vec1)
{
	return boolInVec(_mm_or_ps(vec0.get128(), vec1.get128()));
}

inline
const boolInVec
operator ^ (const boolInVec &vec0, const boolInVec &vec1)
{
	return boolInVec(_mm_xor_ps(vec0.get128(), vec1.get128()));
}

inline
const boolInVec
select(const boolInVec &vec0, const boolInVec &vec1, const boolInVec &select_vec1)
{
	return boolInVec(vec_sel(vec0.get128(), vec1.get128(), select_vec1.get128()));
}
 
} // namespace Vectormath

#endif // boolInVec_h
