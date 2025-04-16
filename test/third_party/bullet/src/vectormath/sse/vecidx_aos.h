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

#ifndef _VECTORMATH_VECIDX_AOS_H
#define _VECTORMATH_VECIDX_AOS_H


#include "floatInVec.h"

namespace Vectormath {
namespace Aos {

//-----------------------------------------------------------------------------
// VecIdx 
// Used in setting elements of Vector3, Vector4, Point3, or Quat with the 
// subscripting operator.
//

VM_ATTRIBUTE_ALIGNED_CLASS16 (class) VecIdx
{
private:
   __m128 &ref;
   int i;
public:
    inline VecIdx( __m128& vec, int idx ): ref(vec) { i = idx; }

    // implicitly casts to float unless _VECTORMATH_NO_SCALAR_CAST defined
    // in which case, implicitly casts to floatInVec, and one must call
    // getAsFloat to convert to float.
    //
#ifdef _VECTORMATH_NO_SCALAR_CAST
    inline operator floatInVec() const;
    inline float getAsFloat() const;
#else
    inline operator float() const;
#endif

    inline float operator =( float scalar );
    inline floatInVec operator =( const floatInVec &scalar );
    inline floatInVec operator =( const VecIdx& scalar );
    inline floatInVec operator *=( float scalar );
    inline floatInVec operator *=( const floatInVec &scalar );
    inline floatInVec operator /=( float scalar );
    inline floatInVec operator /=( const floatInVec &scalar );
    inline floatInVec operator +=( float scalar );
    inline floatInVec operator +=( const floatInVec &scalar );
    inline floatInVec operator -=( float scalar );
    inline floatInVec operator -=( const floatInVec &scalar );
};

} // namespace Aos
} // namespace Vectormath

#endif
