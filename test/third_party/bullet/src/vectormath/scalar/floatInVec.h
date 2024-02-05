/*
   Copyright (C) 2009 Sony Computer Entertainment Inc.
   All rights reserved.

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

*/
#ifndef _FLOATINVEC_H
#define _FLOATINVEC_H

#include <math.h>
namespace Vectormath {

class boolInVec;

//--------------------------------------------------------------------------------------------------
// floatInVec class
//

// A class representing a scalar float value contained in a vector register
// This class does not support fastmath
class floatInVec
{
private:
    float mData;

public:
    // Default constructor; does no initialization
    //
    inline floatInVec( ) { };

    // Construct from a value converted from bool
    //
    inline floatInVec(boolInVec vec);

    // Explicit cast from float
    //
    explicit inline floatInVec(float scalar);

    // Explicit cast to float
    //
    inline float getAsFloat() const;

#ifndef _VECTORMATH_NO_SCALAR_CAST
    // Implicit cast to float
    //
    inline operator float() const;
#endif

    // Post increment (add 1.0f)
    //
    inline const floatInVec operator ++ (int);

    // Post decrement (subtract 1.0f)
    //
    inline const floatInVec operator -- (int);

    // Pre increment (add 1.0f)
    //
    inline floatInVec& operator ++ ();

    // Pre decrement (subtract 1.0f)
    //
    inline floatInVec& operator -- ();

    // Negation operator
    //
    inline const floatInVec operator - () const;

    // Assignment operator
    //
    inline floatInVec& operator = (floatInVec vec);

    // Multiplication assignment operator
    //
    inline floatInVec& operator *= (floatInVec vec);

    // Division assignment operator
    //
    inline floatInVec& operator /= (floatInVec vec);

    // Addition assignment operator
    //
    inline floatInVec& operator += (floatInVec vec);

    // Subtraction assignment operator
    //
    inline floatInVec& operator -= (floatInVec vec);

};

// Multiplication operator
//
inline const floatInVec operator * (floatInVec vec0, floatInVec vec1);

// Division operator
//
inline const floatInVec operator / (floatInVec vec0, floatInVec vec1);

// Addition operator
//
inline const floatInVec operator + (floatInVec vec0, floatInVec vec1);

// Subtraction operator
//
inline const floatInVec operator - (floatInVec vec0, floatInVec vec1);

// Less than operator
//
inline const boolInVec operator < (floatInVec vec0, floatInVec vec1);

// Less than or equal operator
//
inline const boolInVec operator <= (floatInVec vec0, floatInVec vec1);

// Greater than operator
//
inline const boolInVec operator > (floatInVec vec0, floatInVec vec1);

// Greater than or equal operator
//
inline const boolInVec operator >= (floatInVec vec0, floatInVec vec1);

// Equal operator
//
inline const boolInVec operator == (floatInVec vec0, floatInVec vec1);

// Not equal operator
//
inline const boolInVec operator != (floatInVec vec0, floatInVec vec1);

// Conditionally select between two values
//
inline const floatInVec select(floatInVec vec0, floatInVec vec1, boolInVec select_vec1);


} // namespace Vectormath


//--------------------------------------------------------------------------------------------------
// floatInVec implementation
//

#include "boolInVec.h"

namespace Vectormath {

inline
floatInVec::floatInVec(boolInVec vec)
{
    mData = float(vec.getAsBool());
}

inline
floatInVec::floatInVec(float scalar)
{
    mData = scalar;
}

inline
float
floatInVec::getAsFloat() const
{
    return mData;
}

#ifndef _VECTORMATH_NO_SCALAR_CAST
inline
floatInVec::operator float() const
{
    return getAsFloat();
}
#endif

inline
const floatInVec
floatInVec::operator ++ (int)
{
    float olddata = mData;
    operator ++();
    return floatInVec(olddata);
}

inline
const floatInVec
floatInVec::operator -- (int)
{
    float olddata = mData;
    operator --();
    return floatInVec(olddata);
}

inline
floatInVec&
floatInVec::operator ++ ()
{
    *this += floatInVec(1.0f);
    return *this;
}

inline
floatInVec&
floatInVec::operator -- ()
{
    *this -= floatInVec(1.0f);
    return *this;
}

inline
const floatInVec
floatInVec::operator - () const
{
    return floatInVec(-mData);
}

inline
floatInVec&
floatInVec::operator = (floatInVec vec)
{
    mData = vec.mData;
    return *this;
}

inline
floatInVec&
floatInVec::operator *= (floatInVec vec)
{
    *this = *this * vec;
    return *this;
}

inline
floatInVec&
floatInVec::operator /= (floatInVec vec)
{
    *this = *this / vec;
    return *this;
}

inline
floatInVec&
floatInVec::operator += (floatInVec vec)
{
    *this = *this + vec;
    return *this;
}

inline
floatInVec&
floatInVec::operator -= (floatInVec vec)
{
    *this = *this - vec;
    return *this;
}

inline
const floatInVec
operator * (floatInVec vec0, floatInVec vec1)
{
    return floatInVec(vec0.getAsFloat() * vec1.getAsFloat());
}

inline
const floatInVec
operator / (floatInVec num, floatInVec den)
{
    return floatInVec(num.getAsFloat() / den.getAsFloat());
}

inline
const floatInVec
operator + (floatInVec vec0, floatInVec vec1)
{
    return floatInVec(vec0.getAsFloat() + vec1.getAsFloat());
}

inline
const floatInVec
operator - (floatInVec vec0, floatInVec vec1)
{
    return floatInVec(vec0.getAsFloat() - vec1.getAsFloat());
}

inline
const boolInVec
operator < (floatInVec vec0, floatInVec vec1)
{
    return boolInVec(vec0.getAsFloat() < vec1.getAsFloat());
}

inline
const boolInVec
operator <= (floatInVec vec0, floatInVec vec1)
{
    return !(vec0 > vec1);
}

inline
const boolInVec
operator > (floatInVec vec0, floatInVec vec1)
{
    return boolInVec(vec0.getAsFloat() > vec1.getAsFloat());
}

inline
const boolInVec
operator >= (floatInVec vec0, floatInVec vec1)
{
    return !(vec0 < vec1);
}

inline
const boolInVec
operator == (floatInVec vec0, floatInVec vec1)
{
    return boolInVec(vec0.getAsFloat() == vec1.getAsFloat());
}

inline
const boolInVec
operator != (floatInVec vec0, floatInVec vec1)
{
    return !(vec0 == vec1);
}

inline
const floatInVec
select(floatInVec vec0, floatInVec vec1, boolInVec select_vec1)
{
    return (select_vec1.getAsBool() == 0) ? vec0 : vec1;
}

} // namespace Vectormath

#endif // floatInVec_h
