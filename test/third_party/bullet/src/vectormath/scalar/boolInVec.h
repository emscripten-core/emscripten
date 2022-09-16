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
    unsigned int mData;

public:
    // Default constructor; does no initialization
    //
    inline boolInVec( ) { };

    // Construct from a value converted from float
    //
    inline boolInVec(floatInVec vec);

    // Explicit cast from bool
    //
    explicit inline boolInVec(bool scalar);

    // Explicit cast to bool
    //
    inline bool getAsBool() const;

#ifndef _VECTORMATH_NO_SCALAR_CAST
    // Implicit cast to bool
    //
    inline operator bool() const;
#endif

    // Boolean negation operator
    //
    inline const boolInVec operator ! () const;

    // Assignment operator
    //
    inline boolInVec& operator = (boolInVec vec);

    // Boolean and assignment operator
    //
    inline boolInVec& operator &= (boolInVec vec);

    // Boolean exclusive or assignment operator
    //
    inline boolInVec& operator ^= (boolInVec vec);

    // Boolean or assignment operator
    //
    inline boolInVec& operator |= (boolInVec vec);

};

// Equal operator
//
inline const boolInVec operator == (boolInVec vec0, boolInVec vec1);

// Not equal operator
//
inline const boolInVec operator != (boolInVec vec0, boolInVec vec1);

// And operator
//
inline const boolInVec operator & (boolInVec vec0, boolInVec vec1);

// Exclusive or operator
//
inline const boolInVec operator ^ (boolInVec vec0, boolInVec vec1);

// Or operator
//
inline const boolInVec operator | (boolInVec vec0, boolInVec vec1);

// Conditionally select between two values
//
inline const boolInVec select(boolInVec vec0, boolInVec vec1, boolInVec select_vec1);


} // namespace Vectormath


//--------------------------------------------------------------------------------------------------
// boolInVec implementation
//

#include "floatInVec.h"

namespace Vectormath {

inline
boolInVec::boolInVec(floatInVec vec)
{
    *this = (vec != floatInVec(0.0f));
}

inline
boolInVec::boolInVec(bool scalar)
{
    mData = -(int)scalar;
}

inline
bool
boolInVec::getAsBool() const
{
    return (mData > 0);
}

#ifndef _VECTORMATH_NO_SCALAR_CAST
inline
boolInVec::operator bool() const
{
    return getAsBool();
}
#endif

inline
const boolInVec
boolInVec::operator ! () const
{
    return boolInVec(!mData);
}

inline
boolInVec&
boolInVec::operator = (boolInVec vec)
{
    mData = vec.mData;
    return *this;
}

inline
boolInVec&
boolInVec::operator &= (boolInVec vec)
{
    *this = *this & vec;
    return *this;
}

inline
boolInVec&
boolInVec::operator ^= (boolInVec vec)
{
    *this = *this ^ vec;
    return *this;
}

inline
boolInVec&
boolInVec::operator |= (boolInVec vec)
{
    *this = *this | vec;
    return *this;
}

inline
const boolInVec
operator == (boolInVec vec0, boolInVec vec1)
{
    return boolInVec(vec0.getAsBool() == vec1.getAsBool());
}

inline
const boolInVec
operator != (boolInVec vec0, boolInVec vec1)
{
    return !(vec0 == vec1);
}

inline
const boolInVec
operator & (boolInVec vec0, boolInVec vec1)
{
    return boolInVec(vec0.getAsBool() & vec1.getAsBool());
}

inline
const boolInVec
operator | (boolInVec vec0, boolInVec vec1)
{
    return boolInVec(vec0.getAsBool() | vec1.getAsBool());
}

inline
const boolInVec
operator ^ (boolInVec vec0, boolInVec vec1)
{
    return boolInVec(vec0.getAsBool() ^ vec1.getAsBool());
}

inline
const boolInVec
select(boolInVec vec0, boolInVec vec1, boolInVec select_vec1)
{
    return (select_vec1.getAsBool() == 0) ? vec0 : vec1;
}

} // namespace Vectormath

#endif // boolInVec_h
