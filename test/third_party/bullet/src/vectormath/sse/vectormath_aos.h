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


#ifndef _VECTORMATH_AOS_CPP_SSE_H
#define _VECTORMATH_AOS_CPP_SSE_H

#include <math.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <assert.h>

#define Vector3Ref Vector3&
#define QuatRef	Quat&
#define Matrix3Ref Matrix3&

#if (defined (_WIN32) && (_MSC_VER) && _MSC_VER >= 1400)
	#define USE_SSE3_LDDQU

	#define VM_ATTRIBUTE_ALIGNED_CLASS16(a) __declspec(align(16)) a
	#define VM_ATTRIBUTE_ALIGN16 __declspec(align(16))
	#define VECTORMATH_FORCE_INLINE __forceinline 
#else
	#define VM_ATTRIBUTE_ALIGNED_CLASS16(a) a __attribute__ ((aligned (16)))	
	#define VM_ATTRIBUTE_ALIGN16 __attribute__ ((aligned (16)))	
	#define VECTORMATH_FORCE_INLINE inline 
	#ifdef __SSE3__
		#define USE_SSE3_LDDQU
	#endif //__SSE3__
#endif//_WIN32


#ifdef USE_SSE3_LDDQU
#include <pmmintrin.h>//_mm_lddqu_si128
#endif //USE_SSE3_LDDQU


// TODO: Tidy
typedef __m128 vec_float4;
typedef __m128 vec_uint4;
typedef __m128 vec_int4;
typedef __m128i vec_uchar16;
typedef __m128i vec_ushort8;

#define vec_splat(x, e) _mm_shuffle_ps(x, x, _MM_SHUFFLE(e,e,e,e))

#define _mm_ror_ps(vec,i)	\
	(((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((unsigned char)(i+3)%4,(unsigned char)(i+2)%4,(unsigned char)(i+1)%4,(unsigned char)(i+0)%4))) : (vec))
#define _mm_rol_ps(vec,i)	\
	(((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((unsigned char)(7-i)%4,(unsigned char)(6-i)%4,(unsigned char)(5-i)%4,(unsigned char)(4-i)%4))) : (vec))

#define vec_sld(vec,vec2,x) _mm_ror_ps(vec, ((x)/4))

#define _mm_abs_ps(vec)		_mm_andnot_ps(_MASKSIGN_,vec)
#define _mm_neg_ps(vec)		_mm_xor_ps(_MASKSIGN_,vec)

#define vec_madd(a, b, c) _mm_add_ps(c, _mm_mul_ps(a, b) )

union SSEFloat
{
	__m128i vi;
	__m128 m128;
	__m128 vf;
	unsigned int	ui[4];
	unsigned short s[8];
	float f[4];
	SSEFloat(__m128 v) : m128(v) {}
    SSEFloat(__m128i v) : vi(v) {}
	SSEFloat() {}//uninitialized
};

static VECTORMATH_FORCE_INLINE __m128 vec_sel(__m128 a, __m128 b, __m128 mask)
{
	return _mm_or_ps(_mm_and_ps(mask, b), _mm_andnot_ps(mask, a));
}
static VECTORMATH_FORCE_INLINE __m128 vec_sel(__m128 a, __m128 b, const unsigned int *_mask)
{
	return vec_sel(a, b, _mm_load_ps((float *)_mask));
}
static VECTORMATH_FORCE_INLINE __m128 vec_sel(__m128 a, __m128 b, unsigned int _mask)
{
	return vec_sel(a, b, _mm_set1_ps(*(float *)&_mask));
}

static VECTORMATH_FORCE_INLINE __m128 toM128(unsigned int x)
{
    return _mm_set1_ps( *(float *)&x );
}

static VECTORMATH_FORCE_INLINE __m128 fabsf4(__m128 x)
{
    return _mm_and_ps( x, toM128( 0x7fffffff ) );
}
/*
union SSE64
{
	__m128 m128;
	struct
	{
		__m64 m01;
		__m64 m23;
	} m64;
};

static VECTORMATH_FORCE_INLINE __m128 vec_cts(__m128 x, int a)
{
	assert(a == 0); // Only 2^0 supported
	(void)a;
	SSE64 sse64;
	sse64.m64.m01 = _mm_cvttps_pi32(x);
	sse64.m64.m23 = _mm_cvttps_pi32(_mm_ror_ps(x,2));
	_mm_empty();
    return sse64.m128;
}

static VECTORMATH_FORCE_INLINE __m128 vec_ctf(__m128 x, int a)
{
	assert(a == 0); // Only 2^0 supported
	(void)a;
	SSE64 sse64;
	sse64.m128 = x;
	__m128 result =_mm_movelh_ps(
		_mm_cvt_pi2ps(_mm_setzero_ps(), sse64.m64.m01),
		_mm_cvt_pi2ps(_mm_setzero_ps(), sse64.m64.m23));
	_mm_empty();
	return result;
}
*/
static VECTORMATH_FORCE_INLINE __m128 vec_cts(__m128 x, int a)
{
	assert(a == 0); // Only 2^0 supported
	(void)a;
	__m128i result = _mm_cvtps_epi32(x);
    return (__m128 &)result;
}

static VECTORMATH_FORCE_INLINE __m128 vec_ctf(__m128 x, int a)
{
	assert(a == 0); // Only 2^0 supported
	(void)a;
	return _mm_cvtepi32_ps((__m128i &)x);
}

#define vec_nmsub(a,b,c) _mm_sub_ps( c, _mm_mul_ps( a, b ) )
#define vec_sub(a,b) _mm_sub_ps( a, b )
#define vec_add(a,b) _mm_add_ps( a, b )
#define vec_mul(a,b) _mm_mul_ps( a, b )
#define vec_xor(a,b) _mm_xor_ps( a, b )
#define vec_and(a,b) _mm_and_ps( a, b )
#define vec_cmpeq(a,b) _mm_cmpeq_ps( a, b )
#define vec_cmpgt(a,b) _mm_cmpgt_ps( a, b )

#define vec_mergeh(a,b) _mm_unpacklo_ps( a, b )
#define vec_mergel(a,b) _mm_unpackhi_ps( a, b )

#define vec_andc(a,b) _mm_andnot_ps( b, a )

#define sqrtf4(x) _mm_sqrt_ps( x )
#define rsqrtf4(x) _mm_rsqrt_ps( x )
#define recipf4(x) _mm_rcp_ps( x )
#define negatef4(x) _mm_sub_ps( _mm_setzero_ps(), x )

static VECTORMATH_FORCE_INLINE __m128 newtonrapson_rsqrt4( const __m128 v )
{   
#define _half4 _mm_setr_ps(.5f,.5f,.5f,.5f) 
#define _three _mm_setr_ps(3.f,3.f,3.f,3.f)
const __m128 approx = _mm_rsqrt_ps( v );   
const __m128 muls = _mm_mul_ps(_mm_mul_ps(v, approx), approx);   
return _mm_mul_ps(_mm_mul_ps(_half4, approx), _mm_sub_ps(_three, muls) );
}

static VECTORMATH_FORCE_INLINE __m128 acosf4(__m128 x)
{
    __m128 xabs = fabsf4(x);
	__m128 select = _mm_cmplt_ps( x, _mm_setzero_ps() );
    __m128 t1 = sqrtf4(vec_sub(_mm_set1_ps(1.0f), xabs));
    
    /* Instruction counts can be reduced if the polynomial was
     * computed entirely from nested (dependent) fma's. However, 
     * to reduce the number of pipeline stalls, the polygon is evaluated 
     * in two halves (hi amd lo). 
     */
    __m128 xabs2 = _mm_mul_ps(xabs,  xabs);
    __m128 xabs4 = _mm_mul_ps(xabs2, xabs2);
    __m128 hi = vec_madd(vec_madd(vec_madd(_mm_set1_ps(-0.0012624911f),
		xabs, _mm_set1_ps(0.0066700901f)),
			xabs, _mm_set1_ps(-0.0170881256f)),
				xabs, _mm_set1_ps( 0.0308918810f));
    __m128 lo = vec_madd(vec_madd(vec_madd(_mm_set1_ps(-0.0501743046f),
		xabs, _mm_set1_ps(0.0889789874f)),
			xabs, _mm_set1_ps(-0.2145988016f)),
				xabs, _mm_set1_ps( 1.5707963050f));
    
    __m128 result = vec_madd(hi, xabs4, lo);
    
    // Adjust the result if x is negactive.
    return vec_sel(
		vec_mul(t1, result),									// Positive
		vec_nmsub(t1, result, _mm_set1_ps(3.1415926535898f)),	// Negative
		select);
}

static VECTORMATH_FORCE_INLINE __m128 sinf4(vec_float4 x)
{

//
// Common constants used to evaluate sinf4/cosf4/tanf4
//
#define _SINCOS_CC0  -0.0013602249f
#define _SINCOS_CC1   0.0416566950f
#define _SINCOS_CC2  -0.4999990225f
#define _SINCOS_SC0  -0.0001950727f
#define _SINCOS_SC1   0.0083320758f
#define _SINCOS_SC2  -0.1666665247f

#define _SINCOS_KC1  1.57079625129f
#define _SINCOS_KC2  7.54978995489e-8f

    vec_float4 xl,xl2,xl3,res;

    // Range reduction using : xl = angle * TwoOverPi;
    //  
    xl = vec_mul(x, _mm_set1_ps(0.63661977236f));

    // Find the quadrant the angle falls in
    // using:  q = (int) (ceil(abs(xl))*sign(xl))
    //
    vec_int4 q = vec_cts(xl,0);

    // Compute an offset based on the quadrant that the angle falls in
    // 
    vec_int4 offset = _mm_and_ps(q,toM128(0x3));

    // Remainder in range [-pi/4..pi/4]
    //
    vec_float4 qf = vec_ctf(q,0);
    xl  = vec_nmsub(qf,_mm_set1_ps(_SINCOS_KC2),vec_nmsub(qf,_mm_set1_ps(_SINCOS_KC1),x));
    
    // Compute x^2 and x^3
    //
    xl2 = vec_mul(xl,xl);
    xl3 = vec_mul(xl2,xl);
    
    // Compute both the sin and cos of the angles
    // using a polynomial expression:
    //   cx = 1.0f + xl2 * ((C0 * xl2 + C1) * xl2 + C2), and
    //   sx = xl + xl3 * ((S0 * xl2 + S1) * xl2 + S2)
    //
    
    vec_float4 cx =
		vec_madd(
			vec_madd(
				vec_madd(_mm_set1_ps(_SINCOS_CC0),xl2,_mm_set1_ps(_SINCOS_CC1)),xl2,_mm_set1_ps(_SINCOS_CC2)),xl2,_mm_set1_ps(1.0f));
    vec_float4 sx =
		vec_madd(
			vec_madd(
				vec_madd(_mm_set1_ps(_SINCOS_SC0),xl2,_mm_set1_ps(_SINCOS_SC1)),xl2,_mm_set1_ps(_SINCOS_SC2)),xl3,xl);

    // Use the cosine when the offset is odd and the sin
    // when the offset is even
    //
    res = vec_sel(cx,sx,vec_cmpeq(vec_and(offset,
                                          toM128(0x1)),
										  _mm_setzero_ps()));

    // Flip the sign of the result when (offset mod 4) = 1 or 2
    //
    return vec_sel(
		vec_xor(toM128(0x80000000U), res),	// Negative
		res,								// Positive
		vec_cmpeq(vec_and(offset,toM128(0x2)),_mm_setzero_ps()));
}

static VECTORMATH_FORCE_INLINE void sincosf4(vec_float4 x, vec_float4* s, vec_float4* c)
{
    vec_float4 xl,xl2,xl3;
    vec_int4   offsetSin, offsetCos;

    // Range reduction using : xl = angle * TwoOverPi;
    //  
    xl = vec_mul(x, _mm_set1_ps(0.63661977236f));

    // Find the quadrant the angle falls in
    // using:  q = (int) (ceil(abs(xl))*sign(xl))
    //
    //vec_int4 q = vec_cts(vec_add(xl,vec_sel(_mm_set1_ps(0.5f),xl,(0x80000000))),0);
    vec_int4 q = vec_cts(xl,0);
     
    // Compute the offset based on the quadrant that the angle falls in.
    // Add 1 to the offset for the cosine. 
    //
    offsetSin = vec_and(q,toM128((int)0x3));
	__m128i temp = _mm_add_epi32(_mm_set1_epi32(1),(__m128i &)offsetSin);
	offsetCos = (__m128 &)temp;

    // Remainder in range [-pi/4..pi/4]
    //
    vec_float4 qf = vec_ctf(q,0);
    xl  = vec_nmsub(qf,_mm_set1_ps(_SINCOS_KC2),vec_nmsub(qf,_mm_set1_ps(_SINCOS_KC1),x));
    
    // Compute x^2 and x^3
    //
    xl2 = vec_mul(xl,xl);
    xl3 = vec_mul(xl2,xl);
    
    // Compute both the sin and cos of the angles
    // using a polynomial expression:
    //   cx = 1.0f + xl2 * ((C0 * xl2 + C1) * xl2 + C2), and
    //   sx = xl + xl3 * ((S0 * xl2 + S1) * xl2 + S2)
    //
    vec_float4 cx =
		vec_madd(
			vec_madd(
				vec_madd(_mm_set1_ps(_SINCOS_CC0),xl2,_mm_set1_ps(_SINCOS_CC1)),xl2,_mm_set1_ps(_SINCOS_CC2)),xl2,_mm_set1_ps(1.0f));
    vec_float4 sx =
		vec_madd(
			vec_madd(
				vec_madd(_mm_set1_ps(_SINCOS_SC0),xl2,_mm_set1_ps(_SINCOS_SC1)),xl2,_mm_set1_ps(_SINCOS_SC2)),xl3,xl);

    // Use the cosine when the offset is odd and the sin
    // when the offset is even
    //
    vec_uint4 sinMask = (vec_uint4)vec_cmpeq(vec_and(offsetSin,toM128(0x1)),_mm_setzero_ps());
    vec_uint4 cosMask = (vec_uint4)vec_cmpeq(vec_and(offsetCos,toM128(0x1)),_mm_setzero_ps());    
    *s = vec_sel(cx,sx,sinMask);
    *c = vec_sel(cx,sx,cosMask);

    // Flip the sign of the result when (offset mod 4) = 1 or 2
    //
    sinMask = vec_cmpeq(vec_and(offsetSin,toM128(0x2)),_mm_setzero_ps());
    cosMask = vec_cmpeq(vec_and(offsetCos,toM128(0x2)),_mm_setzero_ps());
    
    *s = vec_sel((vec_float4)vec_xor(toM128(0x80000000),(vec_uint4)*s),*s,sinMask);
    *c = vec_sel((vec_float4)vec_xor(toM128(0x80000000),(vec_uint4)*c),*c,cosMask);    
}

#include "vecidx_aos.h"
#include "floatInVec.h"
#include "boolInVec.h"

#ifdef _VECTORMATH_DEBUG
#include <stdio.h>
#endif
namespace Vectormath {

namespace Aos {

//-----------------------------------------------------------------------------
// Forward Declarations
//

class Vector3;
class Vector4;
class Point3;
class Quat;
class Matrix3;
class Matrix4;
class Transform3;

// A 3-D vector in array-of-structures format
//
class Vector3
{
    __m128 mVec128;

	VECTORMATH_FORCE_INLINE void set128(vec_float4 vec);
	 
	 VECTORMATH_FORCE_INLINE  vec_float4& get128Ref();

public:
    // Default constructor; does no initialization
    // 
    VECTORMATH_FORCE_INLINE Vector3( ) { };

	// Default copy constructor
    // 
	VECTORMATH_FORCE_INLINE Vector3(const Vector3& vec);

    // Construct a 3-D vector from x, y, and z elements
    // 
    VECTORMATH_FORCE_INLINE Vector3( float x, float y, float z );

    // Construct a 3-D vector from x, y, and z elements (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector3( const floatInVec &x, const floatInVec &y, const floatInVec &z );

    // Copy elements from a 3-D point into a 3-D vector
    // 
    explicit VECTORMATH_FORCE_INLINE Vector3( const Point3 &pnt );

    // Set all elements of a 3-D vector to the same scalar value
    // 
    explicit VECTORMATH_FORCE_INLINE Vector3( float scalar );

    // Set all elements of a 3-D vector to the same scalar value (scalar data contained in vector data type)
    // 
    explicit VECTORMATH_FORCE_INLINE Vector3( const floatInVec &scalar );

    // Set vector float data in a 3-D vector
    // 
    explicit VECTORMATH_FORCE_INLINE Vector3( __m128 vf4 );

    // Get vector float data from a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE __m128 get128( ) const;

    // Assign one 3-D vector to another
    // 
    VECTORMATH_FORCE_INLINE Vector3 & operator =( const Vector3 &vec );

    // Set the x element of a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE Vector3 & setX( float x );

    // Set the y element of a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE Vector3 & setY( float y );

    // Set the z element of a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE Vector3 & setZ( float z );

    // Set the x element of a 3-D vector (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector3 & setX( const floatInVec &x );

    // Set the y element of a 3-D vector (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector3 & setY( const floatInVec &y );

    // Set the z element of a 3-D vector (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector3 & setZ( const floatInVec &z );

    // Get the x element of a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getX( ) const;

    // Get the y element of a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getY( ) const;

    // Get the z element of a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getZ( ) const;

    // Set an x, y, or z element of a 3-D vector by index
    // 
    VECTORMATH_FORCE_INLINE Vector3 & setElem( int idx, float value );

    // Set an x, y, or z element of a 3-D vector by index (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector3 & setElem( int idx, const floatInVec &value );

    // Get an x, y, or z element of a 3-D vector by index
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getElem( int idx ) const;

    // Subscripting operator to set or get an element
    // 
    VECTORMATH_FORCE_INLINE VecIdx operator []( int idx );

    // Subscripting operator to get an element
    // 
    VECTORMATH_FORCE_INLINE const floatInVec operator []( int idx ) const;

    // Add two 3-D vectors
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator +( const Vector3 &vec ) const;

    // Subtract a 3-D vector from another 3-D vector
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator -( const Vector3 &vec ) const;

    // Add a 3-D vector to a 3-D point
    // 
    VECTORMATH_FORCE_INLINE const Point3 operator +( const Point3 &pnt ) const;

    // Multiply a 3-D vector by a scalar
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator *( float scalar ) const;

    // Divide a 3-D vector by a scalar
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator /( float scalar ) const;

    // Multiply a 3-D vector by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator *( const floatInVec &scalar ) const;

    // Divide a 3-D vector by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator /( const floatInVec &scalar ) const;

    // Perform compound assignment and addition with a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE Vector3 & operator +=( const Vector3 &vec );

    // Perform compound assignment and subtraction by a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE Vector3 & operator -=( const Vector3 &vec );

    // Perform compound assignment and multiplication by a scalar
    // 
    VECTORMATH_FORCE_INLINE Vector3 & operator *=( float scalar );

    // Perform compound assignment and division by a scalar
    // 
    VECTORMATH_FORCE_INLINE Vector3 & operator /=( float scalar );

    // Perform compound assignment and multiplication by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector3 & operator *=( const floatInVec &scalar );

    // Perform compound assignment and division by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector3 & operator /=( const floatInVec &scalar );

    // Negate all elements of a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator -( ) const;

    // Construct x axis
    // 
    static VECTORMATH_FORCE_INLINE const Vector3 xAxis( );

    // Construct y axis
    // 
    static VECTORMATH_FORCE_INLINE const Vector3 yAxis( );

    // Construct z axis
    // 
    static VECTORMATH_FORCE_INLINE const Vector3 zAxis( );

};

// Multiply a 3-D vector by a scalar
// 
VECTORMATH_FORCE_INLINE const Vector3 operator *( float scalar, const Vector3 &vec );

// Multiply a 3-D vector by a scalar (scalar data contained in vector data type)
// 
VECTORMATH_FORCE_INLINE const Vector3 operator *( const floatInVec &scalar, const Vector3 &vec );

// Multiply two 3-D vectors per element
// 
VECTORMATH_FORCE_INLINE const Vector3 mulPerElem( const Vector3 &vec0, const Vector3 &vec1 );

// Divide two 3-D vectors per element
// NOTE: 
// Floating-point behavior matches standard library function divf4.
// 
VECTORMATH_FORCE_INLINE const Vector3 divPerElem( const Vector3 &vec0, const Vector3 &vec1 );

// Compute the reciprocal of a 3-D vector per element
// NOTE: 
// Floating-point behavior matches standard library function recipf4.
// 
VECTORMATH_FORCE_INLINE const Vector3 recipPerElem( const Vector3 &vec );

// Compute the absolute value of a 3-D vector per element
// 
VECTORMATH_FORCE_INLINE const Vector3 absPerElem( const Vector3 &vec );

// Copy sign from one 3-D vector to another, per element
// 
VECTORMATH_FORCE_INLINE const Vector3 copySignPerElem( const Vector3 &vec0, const Vector3 &vec1 );

// Maximum of two 3-D vectors per element
// 
VECTORMATH_FORCE_INLINE const Vector3 maxPerElem( const Vector3 &vec0, const Vector3 &vec1 );

// Minimum of two 3-D vectors per element
// 
VECTORMATH_FORCE_INLINE const Vector3 minPerElem( const Vector3 &vec0, const Vector3 &vec1 );

// Maximum element of a 3-D vector
// 
VECTORMATH_FORCE_INLINE const floatInVec maxElem( const Vector3 &vec );

// Minimum element of a 3-D vector
// 
VECTORMATH_FORCE_INLINE const floatInVec minElem( const Vector3 &vec );

// Compute the sum of all elements of a 3-D vector
// 
VECTORMATH_FORCE_INLINE const floatInVec sum( const Vector3 &vec );

// Compute the dot product of two 3-D vectors
// 
VECTORMATH_FORCE_INLINE const floatInVec dot( const Vector3 &vec0, const Vector3 &vec1 );

// Compute the square of the length of a 3-D vector
// 
VECTORMATH_FORCE_INLINE const floatInVec lengthSqr( const Vector3 &vec );

// Compute the length of a 3-D vector
// 
VECTORMATH_FORCE_INLINE const floatInVec length( const Vector3 &vec );

// Normalize a 3-D vector
// NOTE: 
// The result is unpredictable when all elements of vec are at or near zero.
// 
VECTORMATH_FORCE_INLINE const Vector3 normalize( const Vector3 &vec );

// Compute cross product of two 3-D vectors
// 
VECTORMATH_FORCE_INLINE const Vector3 cross( const Vector3 &vec0, const Vector3 &vec1 );

// Outer product of two 3-D vectors
// 
VECTORMATH_FORCE_INLINE const Matrix3 outer( const Vector3 &vec0, const Vector3 &vec1 );

// Pre-multiply a row vector by a 3x3 matrix
// NOTE: 
// Slower than column post-multiply.
// 
VECTORMATH_FORCE_INLINE const Vector3 rowMul( const Vector3 &vec, const Matrix3 & mat );

// Cross-product matrix of a 3-D vector
// 
VECTORMATH_FORCE_INLINE const Matrix3 crossMatrix( const Vector3 &vec );

// Create cross-product matrix and multiply
// NOTE: 
// Faster than separately creating a cross-product matrix and multiplying.
// 
VECTORMATH_FORCE_INLINE const Matrix3 crossMatrixMul( const Vector3 &vec, const Matrix3 & mat );

// Linear interpolation between two 3-D vectors
// NOTE: 
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Vector3 lerp( float t, const Vector3 &vec0, const Vector3 &vec1 );

// Linear interpolation between two 3-D vectors (scalar data contained in vector data type)
// NOTE: 
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Vector3 lerp( const floatInVec &t, const Vector3 &vec0, const Vector3 &vec1 );

// Spherical linear interpolation between two 3-D vectors
// NOTE: 
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Vector3 slerp( float t, const Vector3 &unitVec0, const Vector3 &unitVec1 );

// Spherical linear interpolation between two 3-D vectors (scalar data contained in vector data type)
// NOTE: 
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Vector3 slerp( const floatInVec &t, const Vector3 &unitVec0, const Vector3 &unitVec1 );

// Conditionally select between two 3-D vectors
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the boolInVec version for better performance.
// 
VECTORMATH_FORCE_INLINE const Vector3 select( const Vector3 &vec0, const Vector3 &vec1, bool select1 );

// Conditionally select between two 3-D vectors (scalar data contained in vector data type)
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// 
VECTORMATH_FORCE_INLINE const Vector3 select( const Vector3 &vec0, const Vector3 &vec1, const boolInVec &select1 );

// Store x, y, and z elements of 3-D vector in first three words of a quadword, preserving fourth word
// 
VECTORMATH_FORCE_INLINE void storeXYZ( const Vector3 &vec, __m128 * quad );

// Load four three-float 3-D vectors, stored in three quadwords
// 
VECTORMATH_FORCE_INLINE void loadXYZArray( Vector3 & vec0, Vector3 & vec1, Vector3 & vec2, Vector3 & vec3, const __m128 * threeQuads );

// Store four 3-D vectors in three quadwords
// 
VECTORMATH_FORCE_INLINE void storeXYZArray( const Vector3 &vec0, const Vector3 &vec1, const Vector3 &vec2, const Vector3 &vec3, __m128 * threeQuads );

// Store eight 3-D vectors as half-floats
// 
VECTORMATH_FORCE_INLINE void storeHalfFloats( const Vector3 &vec0, const Vector3 &vec1, const Vector3 &vec2, const Vector3 &vec3, const Vector3 &vec4, const Vector3 &vec5, const Vector3 &vec6, const Vector3 &vec7, vec_ushort8 * threeQuads );

#ifdef _VECTORMATH_DEBUG

// Print a 3-D vector
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Vector3 &vec );

// Print a 3-D vector and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Vector3 &vec, const char * name );

#endif

// A 4-D vector in array-of-structures format
//
class Vector4
{
    __m128 mVec128;

public:
    // Default constructor; does no initialization
    // 
    VECTORMATH_FORCE_INLINE Vector4( ) { };

    // Construct a 4-D vector from x, y, z, and w elements
    // 
    VECTORMATH_FORCE_INLINE Vector4( float x, float y, float z, float w );

    // Construct a 4-D vector from x, y, z, and w elements (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector4( const floatInVec &x, const floatInVec &y, const floatInVec &z, const floatInVec &w );

    // Construct a 4-D vector from a 3-D vector and a scalar
    // 
    VECTORMATH_FORCE_INLINE Vector4( const Vector3 &xyz, float w );

    // Construct a 4-D vector from a 3-D vector and a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector4( const Vector3 &xyz, const floatInVec &w );

    // Copy x, y, and z from a 3-D vector into a 4-D vector, and set w to 0
    // 
    explicit VECTORMATH_FORCE_INLINE Vector4( const Vector3 &vec );

    // Copy x, y, and z from a 3-D point into a 4-D vector, and set w to 1
    // 
    explicit VECTORMATH_FORCE_INLINE Vector4( const Point3 &pnt );

    // Copy elements from a quaternion into a 4-D vector
    // 
    explicit VECTORMATH_FORCE_INLINE Vector4( const Quat &quat );

    // Set all elements of a 4-D vector to the same scalar value
    // 
    explicit VECTORMATH_FORCE_INLINE Vector4( float scalar );

    // Set all elements of a 4-D vector to the same scalar value (scalar data contained in vector data type)
    // 
    explicit VECTORMATH_FORCE_INLINE Vector4( const floatInVec &scalar );

    // Set vector float data in a 4-D vector
    // 
    explicit VECTORMATH_FORCE_INLINE Vector4( __m128 vf4 );

    // Get vector float data from a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE __m128 get128( ) const;

    // Assign one 4-D vector to another
    // 
    VECTORMATH_FORCE_INLINE Vector4 & operator =( const Vector4 &vec );

    // Set the x, y, and z elements of a 4-D vector
    // NOTE: 
    // This function does not change the w element.
    // 
    VECTORMATH_FORCE_INLINE Vector4 & setXYZ( const Vector3 &vec );

    // Get the x, y, and z elements of a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getXYZ( ) const;

    // Set the x element of a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE Vector4 & setX( float x );

    // Set the y element of a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE Vector4 & setY( float y );

    // Set the z element of a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE Vector4 & setZ( float z );

    // Set the w element of a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE Vector4 & setW( float w );

    // Set the x element of a 4-D vector (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector4 & setX( const floatInVec &x );

    // Set the y element of a 4-D vector (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector4 & setY( const floatInVec &y );

    // Set the z element of a 4-D vector (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector4 & setZ( const floatInVec &z );

    // Set the w element of a 4-D vector (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector4 & setW( const floatInVec &w );

    // Get the x element of a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getX( ) const;

    // Get the y element of a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getY( ) const;

    // Get the z element of a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getZ( ) const;

    // Get the w element of a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getW( ) const;

    // Set an x, y, z, or w element of a 4-D vector by index
    // 
    VECTORMATH_FORCE_INLINE Vector4 & setElem( int idx, float value );

    // Set an x, y, z, or w element of a 4-D vector by index (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector4 & setElem( int idx, const floatInVec &value );

    // Get an x, y, z, or w element of a 4-D vector by index
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getElem( int idx ) const;

    // Subscripting operator to set or get an element
    // 
    VECTORMATH_FORCE_INLINE VecIdx operator []( int idx );

    // Subscripting operator to get an element
    // 
    VECTORMATH_FORCE_INLINE const floatInVec operator []( int idx ) const;

    // Add two 4-D vectors
    // 
    VECTORMATH_FORCE_INLINE const Vector4 operator +( const Vector4 &vec ) const;

    // Subtract a 4-D vector from another 4-D vector
    // 
    VECTORMATH_FORCE_INLINE const Vector4 operator -( const Vector4 &vec ) const;

    // Multiply a 4-D vector by a scalar
    // 
    VECTORMATH_FORCE_INLINE const Vector4 operator *( float scalar ) const;

    // Divide a 4-D vector by a scalar
    // 
    VECTORMATH_FORCE_INLINE const Vector4 operator /( float scalar ) const;

    // Multiply a 4-D vector by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE const Vector4 operator *( const floatInVec &scalar ) const;

    // Divide a 4-D vector by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE const Vector4 operator /( const floatInVec &scalar ) const;

    // Perform compound assignment and addition with a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE Vector4 & operator +=( const Vector4 &vec );

    // Perform compound assignment and subtraction by a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE Vector4 & operator -=( const Vector4 &vec );

    // Perform compound assignment and multiplication by a scalar
    // 
    VECTORMATH_FORCE_INLINE Vector4 & operator *=( float scalar );

    // Perform compound assignment and division by a scalar
    // 
    VECTORMATH_FORCE_INLINE Vector4 & operator /=( float scalar );

    // Perform compound assignment and multiplication by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector4 & operator *=( const floatInVec &scalar );

    // Perform compound assignment and division by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Vector4 & operator /=( const floatInVec &scalar );

    // Negate all elements of a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE const Vector4 operator -( ) const;

    // Construct x axis
    // 
    static VECTORMATH_FORCE_INLINE const Vector4 xAxis( );

    // Construct y axis
    // 
    static VECTORMATH_FORCE_INLINE const Vector4 yAxis( );

    // Construct z axis
    // 
    static VECTORMATH_FORCE_INLINE const Vector4 zAxis( );

    // Construct w axis
    // 
    static VECTORMATH_FORCE_INLINE const Vector4 wAxis( );

};

// Multiply a 4-D vector by a scalar
// 
VECTORMATH_FORCE_INLINE const Vector4 operator *( float scalar, const Vector4 &vec );

// Multiply a 4-D vector by a scalar (scalar data contained in vector data type)
// 
VECTORMATH_FORCE_INLINE const Vector4 operator *( const floatInVec &scalar, const Vector4 &vec );

// Multiply two 4-D vectors per element
// 
VECTORMATH_FORCE_INLINE const Vector4 mulPerElem( const Vector4 &vec0, const Vector4 &vec1 );

// Divide two 4-D vectors per element
// NOTE: 
// Floating-point behavior matches standard library function divf4.
// 
VECTORMATH_FORCE_INLINE const Vector4 divPerElem( const Vector4 &vec0, const Vector4 &vec1 );

// Compute the reciprocal of a 4-D vector per element
// NOTE: 
// Floating-point behavior matches standard library function recipf4.
// 
VECTORMATH_FORCE_INLINE const Vector4 recipPerElem( const Vector4 &vec );

// Compute the absolute value of a 4-D vector per element
// 
VECTORMATH_FORCE_INLINE const Vector4 absPerElem( const Vector4 &vec );

// Copy sign from one 4-D vector to another, per element
// 
VECTORMATH_FORCE_INLINE const Vector4 copySignPerElem( const Vector4 &vec0, const Vector4 &vec1 );

// Maximum of two 4-D vectors per element
// 
VECTORMATH_FORCE_INLINE const Vector4 maxPerElem( const Vector4 &vec0, const Vector4 &vec1 );

// Minimum of two 4-D vectors per element
// 
VECTORMATH_FORCE_INLINE const Vector4 minPerElem( const Vector4 &vec0, const Vector4 &vec1 );

// Maximum element of a 4-D vector
// 
VECTORMATH_FORCE_INLINE const floatInVec maxElem( const Vector4 &vec );

// Minimum element of a 4-D vector
// 
VECTORMATH_FORCE_INLINE const floatInVec minElem( const Vector4 &vec );

// Compute the sum of all elements of a 4-D vector
// 
VECTORMATH_FORCE_INLINE const floatInVec sum( const Vector4 &vec );

// Compute the dot product of two 4-D vectors
// 
VECTORMATH_FORCE_INLINE const floatInVec dot( const Vector4 &vec0, const Vector4 &vec1 );

// Compute the square of the length of a 4-D vector
// 
VECTORMATH_FORCE_INLINE const floatInVec lengthSqr( const Vector4 &vec );

// Compute the length of a 4-D vector
// 
VECTORMATH_FORCE_INLINE const floatInVec length( const Vector4 &vec );

// Normalize a 4-D vector
// NOTE: 
// The result is unpredictable when all elements of vec are at or near zero.
// 
VECTORMATH_FORCE_INLINE const Vector4 normalize( const Vector4 &vec );

// Outer product of two 4-D vectors
// 
VECTORMATH_FORCE_INLINE const Matrix4 outer( const Vector4 &vec0, const Vector4 &vec1 );

// Linear interpolation between two 4-D vectors
// NOTE: 
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Vector4 lerp( float t, const Vector4 &vec0, const Vector4 &vec1 );

// Linear interpolation between two 4-D vectors (scalar data contained in vector data type)
// NOTE: 
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Vector4 lerp( const floatInVec &t, const Vector4 &vec0, const Vector4 &vec1 );

// Spherical linear interpolation between two 4-D vectors
// NOTE: 
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Vector4 slerp( float t, const Vector4 &unitVec0, const Vector4 &unitVec1 );

// Spherical linear interpolation between two 4-D vectors (scalar data contained in vector data type)
// NOTE: 
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Vector4 slerp( const floatInVec &t, const Vector4 &unitVec0, const Vector4 &unitVec1 );

// Conditionally select between two 4-D vectors
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the boolInVec version for better performance.
// 
VECTORMATH_FORCE_INLINE const Vector4 select( const Vector4 &vec0, const Vector4 &vec1, bool select1 );

// Conditionally select between two 4-D vectors (scalar data contained in vector data type)
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// 
VECTORMATH_FORCE_INLINE const Vector4 select( const Vector4 &vec0, const Vector4 &vec1, const boolInVec &select1 );

// Store four 4-D vectors as half-floats
// 
VECTORMATH_FORCE_INLINE void storeHalfFloats( const Vector4 &vec0, const Vector4 &vec1, const Vector4 &vec2, const Vector4 &vec3, vec_ushort8 * twoQuads );

#ifdef _VECTORMATH_DEBUG

// Print a 4-D vector
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Vector4 &vec );

// Print a 4-D vector and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Vector4 &vec, const char * name );

#endif

// A 3-D point in array-of-structures format
//
class Point3
{
    __m128 mVec128;

public:
    // Default constructor; does no initialization
    // 
    VECTORMATH_FORCE_INLINE Point3( ) { };

    // Construct a 3-D point from x, y, and z elements
    // 
    VECTORMATH_FORCE_INLINE Point3( float x, float y, float z );

    // Construct a 3-D point from x, y, and z elements (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Point3( const floatInVec &x, const floatInVec &y, const floatInVec &z );

    // Copy elements from a 3-D vector into a 3-D point
    // 
    explicit VECTORMATH_FORCE_INLINE Point3( const Vector3 &vec );

    // Set all elements of a 3-D point to the same scalar value
    // 
    explicit VECTORMATH_FORCE_INLINE Point3( float scalar );

    // Set all elements of a 3-D point to the same scalar value (scalar data contained in vector data type)
    // 
    explicit VECTORMATH_FORCE_INLINE Point3( const floatInVec &scalar );

    // Set vector float data in a 3-D point
    // 
    explicit VECTORMATH_FORCE_INLINE Point3( __m128 vf4 );

    // Get vector float data from a 3-D point
    // 
    VECTORMATH_FORCE_INLINE __m128 get128( ) const;

    // Assign one 3-D point to another
    // 
    VECTORMATH_FORCE_INLINE Point3 & operator =( const Point3 &pnt );

    // Set the x element of a 3-D point
    // 
    VECTORMATH_FORCE_INLINE Point3 & setX( float x );

    // Set the y element of a 3-D point
    // 
    VECTORMATH_FORCE_INLINE Point3 & setY( float y );

    // Set the z element of a 3-D point
    // 
    VECTORMATH_FORCE_INLINE Point3 & setZ( float z );

    // Set the x element of a 3-D point (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Point3 & setX( const floatInVec &x );

    // Set the y element of a 3-D point (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Point3 & setY( const floatInVec &y );

    // Set the z element of a 3-D point (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Point3 & setZ( const floatInVec &z );

    // Get the x element of a 3-D point
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getX( ) const;

    // Get the y element of a 3-D point
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getY( ) const;

    // Get the z element of a 3-D point
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getZ( ) const;

    // Set an x, y, or z element of a 3-D point by index
    // 
    VECTORMATH_FORCE_INLINE Point3 & setElem( int idx, float value );

    // Set an x, y, or z element of a 3-D point by index (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Point3 & setElem( int idx, const floatInVec &value );

    // Get an x, y, or z element of a 3-D point by index
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getElem( int idx ) const;

    // Subscripting operator to set or get an element
    // 
    VECTORMATH_FORCE_INLINE VecIdx operator []( int idx );

    // Subscripting operator to get an element
    // 
    VECTORMATH_FORCE_INLINE const floatInVec operator []( int idx ) const;

    // Subtract a 3-D point from another 3-D point
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator -( const Point3 &pnt ) const;

    // Add a 3-D point to a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE const Point3 operator +( const Vector3 &vec ) const;

    // Subtract a 3-D vector from a 3-D point
    // 
    VECTORMATH_FORCE_INLINE const Point3 operator -( const Vector3 &vec ) const;

    // Perform compound assignment and addition with a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE Point3 & operator +=( const Vector3 &vec );

    // Perform compound assignment and subtraction by a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE Point3 & operator -=( const Vector3 &vec );

};

// Multiply two 3-D points per element
// 
VECTORMATH_FORCE_INLINE const Point3 mulPerElem( const Point3 &pnt0, const Point3 &pnt1 );

// Divide two 3-D points per element
// NOTE: 
// Floating-point behavior matches standard library function divf4.
// 
VECTORMATH_FORCE_INLINE const Point3 divPerElem( const Point3 &pnt0, const Point3 &pnt1 );

// Compute the reciprocal of a 3-D point per element
// NOTE: 
// Floating-point behavior matches standard library function recipf4.
// 
VECTORMATH_FORCE_INLINE const Point3 recipPerElem( const Point3 &pnt );

// Compute the absolute value of a 3-D point per element
// 
VECTORMATH_FORCE_INLINE const Point3 absPerElem( const Point3 &pnt );

// Copy sign from one 3-D point to another, per element
// 
VECTORMATH_FORCE_INLINE const Point3 copySignPerElem( const Point3 &pnt0, const Point3 &pnt1 );

// Maximum of two 3-D points per element
// 
VECTORMATH_FORCE_INLINE const Point3 maxPerElem( const Point3 &pnt0, const Point3 &pnt1 );

// Minimum of two 3-D points per element
// 
VECTORMATH_FORCE_INLINE const Point3 minPerElem( const Point3 &pnt0, const Point3 &pnt1 );

// Maximum element of a 3-D point
// 
VECTORMATH_FORCE_INLINE const floatInVec maxElem( const Point3 &pnt );

// Minimum element of a 3-D point
// 
VECTORMATH_FORCE_INLINE const floatInVec minElem( const Point3 &pnt );

// Compute the sum of all elements of a 3-D point
// 
VECTORMATH_FORCE_INLINE const floatInVec sum( const Point3 &pnt );

// Apply uniform scale to a 3-D point
// 
VECTORMATH_FORCE_INLINE const Point3 scale( const Point3 &pnt, float scaleVal );

// Apply uniform scale to a 3-D point (scalar data contained in vector data type)
// 
VECTORMATH_FORCE_INLINE const Point3 scale( const Point3 &pnt, const floatInVec &scaleVal );

// Apply non-uniform scale to a 3-D point
// 
VECTORMATH_FORCE_INLINE const Point3 scale( const Point3 &pnt, const Vector3 &scaleVec );

// Scalar projection of a 3-D point on a unit-length 3-D vector
// 
VECTORMATH_FORCE_INLINE const floatInVec projection( const Point3 &pnt, const Vector3 &unitVec );

// Compute the square of the distance of a 3-D point from the coordinate-system origin
// 
VECTORMATH_FORCE_INLINE const floatInVec distSqrFromOrigin( const Point3 &pnt );

// Compute the distance of a 3-D point from the coordinate-system origin
// 
VECTORMATH_FORCE_INLINE const floatInVec distFromOrigin( const Point3 &pnt );

// Compute the square of the distance between two 3-D points
// 
VECTORMATH_FORCE_INLINE const floatInVec distSqr( const Point3 &pnt0, const Point3 &pnt1 );

// Compute the distance between two 3-D points
// 
VECTORMATH_FORCE_INLINE const floatInVec dist( const Point3 &pnt0, const Point3 &pnt1 );

// Linear interpolation between two 3-D points
// NOTE: 
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Point3 lerp( float t, const Point3 &pnt0, const Point3 &pnt1 );

// Linear interpolation between two 3-D points (scalar data contained in vector data type)
// NOTE: 
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Point3 lerp( const floatInVec &t, const Point3 &pnt0, const Point3 &pnt1 );

// Conditionally select between two 3-D points
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the boolInVec version for better performance.
// 
VECTORMATH_FORCE_INLINE const Point3 select( const Point3 &pnt0, const Point3 &pnt1, bool select1 );

// Conditionally select between two 3-D points (scalar data contained in vector data type)
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// 
VECTORMATH_FORCE_INLINE const Point3 select( const Point3 &pnt0, const Point3 &pnt1, const boolInVec &select1 );

// Store x, y, and z elements of 3-D point in first three words of a quadword, preserving fourth word
// 
VECTORMATH_FORCE_INLINE void storeXYZ( const Point3 &pnt, __m128 * quad );

// Load four three-float 3-D points, stored in three quadwords
// 
VECTORMATH_FORCE_INLINE void loadXYZArray( Point3 & pnt0, Point3 & pnt1, Point3 & pnt2, Point3 & pnt3, const __m128 * threeQuads );

// Store four 3-D points in three quadwords
// 
VECTORMATH_FORCE_INLINE void storeXYZArray( const Point3 &pnt0, const Point3 &pnt1, const Point3 &pnt2, const Point3 &pnt3, __m128 * threeQuads );

// Store eight 3-D points as half-floats
// 
VECTORMATH_FORCE_INLINE void storeHalfFloats( const Point3 &pnt0, const Point3 &pnt1, const Point3 &pnt2, const Point3 &pnt3, const Point3 &pnt4, const Point3 &pnt5, const Point3 &pnt6, const Point3 &pnt7, vec_ushort8 * threeQuads );

#ifdef _VECTORMATH_DEBUG

// Print a 3-D point
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Point3 &pnt );

// Print a 3-D point and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Point3 &pnt, const char * name );

#endif

// A quaternion in array-of-structures format
//
class Quat
{
    __m128 mVec128;

public:
    // Default constructor; does no initialization
    // 
    VECTORMATH_FORCE_INLINE Quat( ) { };

	VECTORMATH_FORCE_INLINE  Quat(const Quat& quat);

    // Construct a quaternion from x, y, z, and w elements
    // 
    VECTORMATH_FORCE_INLINE Quat( float x, float y, float z, float w );

    // Construct a quaternion from x, y, z, and w elements (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Quat( const floatInVec &x, const floatInVec &y, const floatInVec &z, const floatInVec &w );

    // Construct a quaternion from a 3-D vector and a scalar
    // 
    VECTORMATH_FORCE_INLINE Quat( const Vector3 &xyz, float w );

    // Construct a quaternion from a 3-D vector and a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Quat( const Vector3 &xyz, const floatInVec &w );

    // Copy elements from a 4-D vector into a quaternion
    // 
    explicit VECTORMATH_FORCE_INLINE Quat( const Vector4 &vec );

    // Convert a rotation matrix to a unit-length quaternion
    // 
    explicit VECTORMATH_FORCE_INLINE Quat( const Matrix3 & rotMat );

    // Set all elements of a quaternion to the same scalar value
    // 
    explicit VECTORMATH_FORCE_INLINE Quat( float scalar );

    // Set all elements of a quaternion to the same scalar value (scalar data contained in vector data type)
    // 
    explicit VECTORMATH_FORCE_INLINE Quat( const floatInVec &scalar );

    // Set vector float data in a quaternion
    // 
    explicit VECTORMATH_FORCE_INLINE Quat( __m128 vf4 );

    // Get vector float data from a quaternion
    // 
    VECTORMATH_FORCE_INLINE __m128 get128( ) const;

	// Set a quaterion from vector float data
    //
	VECTORMATH_FORCE_INLINE void set128(vec_float4 vec);

    // Assign one quaternion to another
    // 
    VECTORMATH_FORCE_INLINE Quat & operator =( const Quat &quat );

    // Set the x, y, and z elements of a quaternion
    // NOTE: 
    // This function does not change the w element.
    // 
    VECTORMATH_FORCE_INLINE Quat & setXYZ( const Vector3 &vec );

    // Get the x, y, and z elements of a quaternion
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getXYZ( ) const;

    // Set the x element of a quaternion
    // 
    VECTORMATH_FORCE_INLINE Quat & setX( float x );

    // Set the y element of a quaternion
    // 
    VECTORMATH_FORCE_INLINE Quat & setY( float y );

    // Set the z element of a quaternion
    // 
    VECTORMATH_FORCE_INLINE Quat & setZ( float z );

    // Set the w element of a quaternion
    // 
    VECTORMATH_FORCE_INLINE Quat & setW( float w );

    // Set the x element of a quaternion (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Quat & setX( const floatInVec &x );

    // Set the y element of a quaternion (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Quat & setY( const floatInVec &y );

    // Set the z element of a quaternion (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Quat & setZ( const floatInVec &z );

    // Set the w element of a quaternion (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Quat & setW( const floatInVec &w );

    // Get the x element of a quaternion
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getX( ) const;

    // Get the y element of a quaternion
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getY( ) const;

    // Get the z element of a quaternion
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getZ( ) const;

    // Get the w element of a quaternion
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getW( ) const;

    // Set an x, y, z, or w element of a quaternion by index
    // 
    VECTORMATH_FORCE_INLINE Quat & setElem( int idx, float value );

    // Set an x, y, z, or w element of a quaternion by index (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Quat & setElem( int idx, const floatInVec &value );

    // Get an x, y, z, or w element of a quaternion by index
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getElem( int idx ) const;

    // Subscripting operator to set or get an element
    // 
    VECTORMATH_FORCE_INLINE VecIdx operator []( int idx );

    // Subscripting operator to get an element
    // 
    VECTORMATH_FORCE_INLINE const floatInVec operator []( int idx ) const;

    // Add two quaternions
    // 
    VECTORMATH_FORCE_INLINE const Quat operator +( const Quat &quat ) const;

    // Subtract a quaternion from another quaternion
    // 
    VECTORMATH_FORCE_INLINE const Quat operator -( const Quat &quat ) const;

    // Multiply two quaternions
    // 
    VECTORMATH_FORCE_INLINE const Quat operator *( const Quat &quat ) const;

    // Multiply a quaternion by a scalar
    // 
    VECTORMATH_FORCE_INLINE const Quat operator *( float scalar ) const;

    // Divide a quaternion by a scalar
    // 
    VECTORMATH_FORCE_INLINE const Quat operator /( float scalar ) const;

    // Multiply a quaternion by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE const Quat operator *( const floatInVec &scalar ) const;

    // Divide a quaternion by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE const Quat operator /( const floatInVec &scalar ) const;

    // Perform compound assignment and addition with a quaternion
    // 
    VECTORMATH_FORCE_INLINE Quat & operator +=( const Quat &quat );

    // Perform compound assignment and subtraction by a quaternion
    // 
    VECTORMATH_FORCE_INLINE Quat & operator -=( const Quat &quat );

    // Perform compound assignment and multiplication by a quaternion
    // 
    VECTORMATH_FORCE_INLINE Quat & operator *=( const Quat &quat );

    // Perform compound assignment and multiplication by a scalar
    // 
    VECTORMATH_FORCE_INLINE Quat & operator *=( float scalar );

    // Perform compound assignment and division by a scalar
    // 
    VECTORMATH_FORCE_INLINE Quat & operator /=( float scalar );

    // Perform compound assignment and multiplication by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Quat & operator *=( const floatInVec &scalar );

    // Perform compound assignment and division by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Quat & operator /=( const floatInVec &scalar );

    // Negate all elements of a quaternion
    // 
    VECTORMATH_FORCE_INLINE const Quat operator -( ) const;

    // Construct an identity quaternion
    // 
    static VECTORMATH_FORCE_INLINE const Quat identity( );

    // Construct a quaternion to rotate between two unit-length 3-D vectors
    // NOTE: 
    // The result is unpredictable if unitVec0 and unitVec1 point in opposite directions.
    // 
    static VECTORMATH_FORCE_INLINE const Quat rotation( const Vector3 &unitVec0, const Vector3 &unitVec1 );

    // Construct a quaternion to rotate around a unit-length 3-D vector
    // 
    static VECTORMATH_FORCE_INLINE const Quat rotation( float radians, const Vector3 &unitVec );

    // Construct a quaternion to rotate around a unit-length 3-D vector (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Quat rotation( const floatInVec &radians, const Vector3 &unitVec );

    // Construct a quaternion to rotate around the x axis
    // 
    static VECTORMATH_FORCE_INLINE const Quat rotationX( float radians );

    // Construct a quaternion to rotate around the y axis
    // 
    static VECTORMATH_FORCE_INLINE const Quat rotationY( float radians );

    // Construct a quaternion to rotate around the z axis
    // 
    static VECTORMATH_FORCE_INLINE const Quat rotationZ( float radians );

    // Construct a quaternion to rotate around the x axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Quat rotationX( const floatInVec &radians );

    // Construct a quaternion to rotate around the y axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Quat rotationY( const floatInVec &radians );

    // Construct a quaternion to rotate around the z axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Quat rotationZ( const floatInVec &radians );

};

// Multiply a quaternion by a scalar
// 
VECTORMATH_FORCE_INLINE const Quat operator *( float scalar, const Quat &quat );

// Multiply a quaternion by a scalar (scalar data contained in vector data type)
// 
VECTORMATH_FORCE_INLINE const Quat operator *( const floatInVec &scalar, const Quat &quat );

// Compute the conjugate of a quaternion
// 
VECTORMATH_FORCE_INLINE const Quat conj( const Quat &quat );

// Use a unit-length quaternion to rotate a 3-D vector
// 
VECTORMATH_FORCE_INLINE const Vector3 rotate( const Quat &unitQuat, const Vector3 &vec );

// Compute the dot product of two quaternions
// 
VECTORMATH_FORCE_INLINE const floatInVec dot( const Quat &quat0, const Quat &quat1 );

// Compute the norm of a quaternion
// 
VECTORMATH_FORCE_INLINE const floatInVec norm( const Quat &quat );

// Compute the length of a quaternion
// 
VECTORMATH_FORCE_INLINE const floatInVec length( const Quat &quat );

// Normalize a quaternion
// NOTE: 
// The result is unpredictable when all elements of quat are at or near zero.
// 
VECTORMATH_FORCE_INLINE const Quat normalize( const Quat &quat );

// Linear interpolation between two quaternions
// NOTE: 
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Quat lerp( float t, const Quat &quat0, const Quat &quat1 );

// Linear interpolation between two quaternions (scalar data contained in vector data type)
// NOTE: 
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Quat lerp( const floatInVec &t, const Quat &quat0, const Quat &quat1 );

// Spherical linear interpolation between two quaternions
// NOTE: 
// Interpolates along the shortest path between orientations.
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Quat slerp( float t, const Quat &unitQuat0, const Quat &unitQuat1 );

// Spherical linear interpolation between two quaternions (scalar data contained in vector data type)
// NOTE: 
// Interpolates along the shortest path between orientations.
// Does not clamp t between 0 and 1.
// 
VECTORMATH_FORCE_INLINE const Quat slerp( const floatInVec &t, const Quat &unitQuat0, const Quat &unitQuat1 );

// Spherical quadrangle interpolation
// 
VECTORMATH_FORCE_INLINE const Quat squad( float t, const Quat &unitQuat0, const Quat &unitQuat1, const Quat &unitQuat2, const Quat &unitQuat3 );

// Spherical quadrangle interpolation (scalar data contained in vector data type)
// 
VECTORMATH_FORCE_INLINE const Quat squad( const floatInVec &t, const Quat &unitQuat0, const Quat &unitQuat1, const Quat &unitQuat2, const Quat &unitQuat3 );

// Conditionally select between two quaternions
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the boolInVec version for better performance.
// 
VECTORMATH_FORCE_INLINE const Quat select( const Quat &quat0, const Quat &quat1, bool select1 );

// Conditionally select between two quaternions (scalar data contained in vector data type)
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// 
VECTORMATH_FORCE_INLINE const Quat select( const Quat &quat0, const Quat &quat1, const boolInVec &select1 );

#ifdef _VECTORMATH_DEBUG

// Print a quaternion
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Quat &quat );

// Print a quaternion and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Quat &quat, const char * name );

#endif

// A 3x3 matrix in array-of-structures format
//
class Matrix3
{
    Vector3 mCol0;
    Vector3 mCol1;
    Vector3 mCol2;

public:
    // Default constructor; does no initialization
    // 
    VECTORMATH_FORCE_INLINE Matrix3( ) { };

    // Copy a 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix3( const Matrix3 & mat );

    // Construct a 3x3 matrix containing the specified columns
    // 
    VECTORMATH_FORCE_INLINE Matrix3( const Vector3 &col0, const Vector3 &col1, const Vector3 &col2 );

    // Construct a 3x3 rotation matrix from a unit-length quaternion
    // 
    explicit VECTORMATH_FORCE_INLINE Matrix3( const Quat &unitQuat );

    // Set all elements of a 3x3 matrix to the same scalar value
    // 
    explicit VECTORMATH_FORCE_INLINE Matrix3( float scalar );

    // Set all elements of a 3x3 matrix to the same scalar value (scalar data contained in vector data type)
    // 
    explicit VECTORMATH_FORCE_INLINE Matrix3( const floatInVec &scalar );

    // Assign one 3x3 matrix to another
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & operator =( const Matrix3 & mat );

    // Set column 0 of a 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & setCol0( const Vector3 &col0 );

    // Set column 1 of a 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & setCol1( const Vector3 &col1 );

    // Set column 2 of a 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & setCol2( const Vector3 &col2 );

    // Get column 0 of a 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getCol0( ) const;

    // Get column 1 of a 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getCol1( ) const;

    // Get column 2 of a 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getCol2( ) const;

    // Set the column of a 3x3 matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & setCol( int col, const Vector3 &vec );

    // Set the row of a 3x3 matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & setRow( int row, const Vector3 &vec );

    // Get the column of a 3x3 matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getCol( int col ) const;

    // Get the row of a 3x3 matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getRow( int row ) const;

    // Subscripting operator to set or get a column
    // 
    VECTORMATH_FORCE_INLINE Vector3 & operator []( int col );

    // Subscripting operator to get a column
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator []( int col ) const;

    // Set the element of a 3x3 matrix referred to by column and row indices
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & setElem( int col, int row, float val );

    // Set the element of a 3x3 matrix referred to by column and row indices (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & setElem( int col, int row, const floatInVec &val );

    // Get the element of a 3x3 matrix referred to by column and row indices
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getElem( int col, int row ) const;

    // Add two 3x3 matrices
    // 
    VECTORMATH_FORCE_INLINE const Matrix3 operator +( const Matrix3 & mat ) const;

    // Subtract a 3x3 matrix from another 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE const Matrix3 operator -( const Matrix3 & mat ) const;

    // Negate all elements of a 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE const Matrix3 operator -( ) const;

    // Multiply a 3x3 matrix by a scalar
    // 
    VECTORMATH_FORCE_INLINE const Matrix3 operator *( float scalar ) const;

    // Multiply a 3x3 matrix by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE const Matrix3 operator *( const floatInVec &scalar ) const;

    // Multiply a 3x3 matrix by a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator *( const Vector3 &vec ) const;

    // Multiply two 3x3 matrices
    // 
    VECTORMATH_FORCE_INLINE const Matrix3 operator *( const Matrix3 & mat ) const;

    // Perform compound assignment and addition with a 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & operator +=( const Matrix3 & mat );

    // Perform compound assignment and subtraction by a 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & operator -=( const Matrix3 & mat );

    // Perform compound assignment and multiplication by a scalar
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & operator *=( float scalar );

    // Perform compound assignment and multiplication by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & operator *=( const floatInVec &scalar );

    // Perform compound assignment and multiplication by a 3x3 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix3 & operator *=( const Matrix3 & mat );

    // Construct an identity 3x3 matrix
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 identity( );

    // Construct a 3x3 matrix to rotate around the x axis
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 rotationX( float radians );

    // Construct a 3x3 matrix to rotate around the y axis
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 rotationY( float radians );

    // Construct a 3x3 matrix to rotate around the z axis
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 rotationZ( float radians );

    // Construct a 3x3 matrix to rotate around the x axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 rotationX( const floatInVec &radians );

    // Construct a 3x3 matrix to rotate around the y axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 rotationY( const floatInVec &radians );

    // Construct a 3x3 matrix to rotate around the z axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 rotationZ( const floatInVec &radians );

    // Construct a 3x3 matrix to rotate around the x, y, and z axes
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 rotationZYX( const Vector3 &radiansXYZ );

    // Construct a 3x3 matrix to rotate around a unit-length 3-D vector
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 rotation( float radians, const Vector3 &unitVec );

    // Construct a 3x3 matrix to rotate around a unit-length 3-D vector (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 rotation( const floatInVec &radians, const Vector3 &unitVec );

    // Construct a rotation matrix from a unit-length quaternion
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 rotation( const Quat &unitQuat );

    // Construct a 3x3 matrix to perform scaling
    // 
    static VECTORMATH_FORCE_INLINE const Matrix3 scale( const Vector3 &scaleVec );

};
// Multiply a 3x3 matrix by a scalar
// 
VECTORMATH_FORCE_INLINE const Matrix3 operator *( float scalar, const Matrix3 & mat );

// Multiply a 3x3 matrix by a scalar (scalar data contained in vector data type)
// 
VECTORMATH_FORCE_INLINE const Matrix3 operator *( const floatInVec &scalar, const Matrix3 & mat );

// Append (post-multiply) a scale transformation to a 3x3 matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
VECTORMATH_FORCE_INLINE const Matrix3 appendScale( const Matrix3 & mat, const Vector3 &scaleVec );

// Prepend (pre-multiply) a scale transformation to a 3x3 matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
VECTORMATH_FORCE_INLINE const Matrix3 prependScale( const Vector3 &scaleVec, const Matrix3 & mat );

// Multiply two 3x3 matrices per element
// 
VECTORMATH_FORCE_INLINE const Matrix3 mulPerElem( const Matrix3 & mat0, const Matrix3 & mat1 );

// Compute the absolute value of a 3x3 matrix per element
// 
VECTORMATH_FORCE_INLINE const Matrix3 absPerElem( const Matrix3 & mat );

// Transpose of a 3x3 matrix
// 
VECTORMATH_FORCE_INLINE const Matrix3 transpose( const Matrix3 & mat );

// Compute the inverse of a 3x3 matrix
// NOTE: 
// Result is unpredictable when the determinant of mat is equal to or near 0.
// 
VECTORMATH_FORCE_INLINE const Matrix3 inverse( const Matrix3 & mat );

// Determinant of a 3x3 matrix
// 
VECTORMATH_FORCE_INLINE const floatInVec determinant( const Matrix3 & mat );

// Conditionally select between two 3x3 matrices
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the boolInVec version for better performance.
// 
VECTORMATH_FORCE_INLINE const Matrix3 select( const Matrix3 & mat0, const Matrix3 & mat1, bool select1 );

// Conditionally select between two 3x3 matrices (scalar data contained in vector data type)
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// 
VECTORMATH_FORCE_INLINE const Matrix3 select( const Matrix3 & mat0, const Matrix3 & mat1, const boolInVec &select1 );

#ifdef _VECTORMATH_DEBUG

// Print a 3x3 matrix
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Matrix3 & mat );

// Print a 3x3 matrix and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Matrix3 & mat, const char * name );

#endif

// A 4x4 matrix in array-of-structures format
//
class Matrix4
{
    Vector4 mCol0;
    Vector4 mCol1;
    Vector4 mCol2;
    Vector4 mCol3;

public:
    // Default constructor; does no initialization
    // 
    VECTORMATH_FORCE_INLINE Matrix4( ) { };

    // Copy a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix4( const Matrix4 & mat );

    // Construct a 4x4 matrix containing the specified columns
    // 
    VECTORMATH_FORCE_INLINE Matrix4( const Vector4 &col0, const Vector4 &col1, const Vector4 &col2, const Vector4 &col3 );

    // Construct a 4x4 matrix from a 3x4 transformation matrix
    // 
    explicit VECTORMATH_FORCE_INLINE Matrix4( const Transform3 & mat );

    // Construct a 4x4 matrix from a 3x3 matrix and a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE Matrix4( const Matrix3 & mat, const Vector3 &translateVec );

    // Construct a 4x4 matrix from a unit-length quaternion and a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE Matrix4( const Quat &unitQuat, const Vector3 &translateVec );

    // Set all elements of a 4x4 matrix to the same scalar value
    // 
    explicit VECTORMATH_FORCE_INLINE Matrix4( float scalar );

    // Set all elements of a 4x4 matrix to the same scalar value (scalar data contained in vector data type)
    // 
    explicit VECTORMATH_FORCE_INLINE Matrix4( const floatInVec &scalar );

    // Assign one 4x4 matrix to another
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & operator =( const Matrix4 & mat );

    // Set the upper-left 3x3 submatrix
    // NOTE: 
    // This function does not change the bottom row elements.
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & setUpper3x3( const Matrix3 & mat3 );

    // Get the upper-left 3x3 submatrix of a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE const Matrix3 getUpper3x3( ) const;

    // Set translation component
    // NOTE: 
    // This function does not change the bottom row elements.
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & setTranslation( const Vector3 &translateVec );

    // Get the translation component of a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getTranslation( ) const;

    // Set column 0 of a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & setCol0( const Vector4 &col0 );

    // Set column 1 of a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & setCol1( const Vector4 &col1 );

    // Set column 2 of a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & setCol2( const Vector4 &col2 );

    // Set column 3 of a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & setCol3( const Vector4 &col3 );

    // Get column 0 of a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector4 getCol0( ) const;

    // Get column 1 of a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector4 getCol1( ) const;

    // Get column 2 of a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector4 getCol2( ) const;

    // Get column 3 of a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector4 getCol3( ) const;

    // Set the column of a 4x4 matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & setCol( int col, const Vector4 &vec );

    // Set the row of a 4x4 matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & setRow( int row, const Vector4 &vec );

    // Get the column of a 4x4 matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE const Vector4 getCol( int col ) const;

    // Get the row of a 4x4 matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE const Vector4 getRow( int row ) const;

    // Subscripting operator to set or get a column
    // 
    VECTORMATH_FORCE_INLINE Vector4 & operator []( int col );

    // Subscripting operator to get a column
    // 
    VECTORMATH_FORCE_INLINE const Vector4 operator []( int col ) const;

    // Set the element of a 4x4 matrix referred to by column and row indices
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & setElem( int col, int row, float val );

    // Set the element of a 4x4 matrix referred to by column and row indices (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & setElem( int col, int row, const floatInVec &val );

    // Get the element of a 4x4 matrix referred to by column and row indices
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getElem( int col, int row ) const;

    // Add two 4x4 matrices
    // 
    VECTORMATH_FORCE_INLINE const Matrix4 operator +( const Matrix4 & mat ) const;

    // Subtract a 4x4 matrix from another 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE const Matrix4 operator -( const Matrix4 & mat ) const;

    // Negate all elements of a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE const Matrix4 operator -( ) const;

    // Multiply a 4x4 matrix by a scalar
    // 
    VECTORMATH_FORCE_INLINE const Matrix4 operator *( float scalar ) const;

    // Multiply a 4x4 matrix by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE const Matrix4 operator *( const floatInVec &scalar ) const;

    // Multiply a 4x4 matrix by a 4-D vector
    // 
    VECTORMATH_FORCE_INLINE const Vector4 operator *( const Vector4 &vec ) const;

    // Multiply a 4x4 matrix by a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE const Vector4 operator *( const Vector3 &vec ) const;

    // Multiply a 4x4 matrix by a 3-D point
    // 
    VECTORMATH_FORCE_INLINE const Vector4 operator *( const Point3 &pnt ) const;

    // Multiply two 4x4 matrices
    // 
    VECTORMATH_FORCE_INLINE const Matrix4 operator *( const Matrix4 & mat ) const;

    // Multiply a 4x4 matrix by a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE const Matrix4 operator *( const Transform3 & tfrm ) const;

    // Perform compound assignment and addition with a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & operator +=( const Matrix4 & mat );

    // Perform compound assignment and subtraction by a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & operator -=( const Matrix4 & mat );

    // Perform compound assignment and multiplication by a scalar
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & operator *=( float scalar );

    // Perform compound assignment and multiplication by a scalar (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & operator *=( const floatInVec &scalar );

    // Perform compound assignment and multiplication by a 4x4 matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & operator *=( const Matrix4 & mat );

    // Perform compound assignment and multiplication by a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE Matrix4 & operator *=( const Transform3 & tfrm );

    // Construct an identity 4x4 matrix
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 identity( );

    // Construct a 4x4 matrix to rotate around the x axis
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 rotationX( float radians );

    // Construct a 4x4 matrix to rotate around the y axis
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 rotationY( float radians );

    // Construct a 4x4 matrix to rotate around the z axis
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 rotationZ( float radians );

    // Construct a 4x4 matrix to rotate around the x axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 rotationX( const floatInVec &radians );

    // Construct a 4x4 matrix to rotate around the y axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 rotationY( const floatInVec &radians );

    // Construct a 4x4 matrix to rotate around the z axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 rotationZ( const floatInVec &radians );

    // Construct a 4x4 matrix to rotate around the x, y, and z axes
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 rotationZYX( const Vector3 &radiansXYZ );

    // Construct a 4x4 matrix to rotate around a unit-length 3-D vector
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 rotation( float radians, const Vector3 &unitVec );

    // Construct a 4x4 matrix to rotate around a unit-length 3-D vector (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 rotation( const floatInVec &radians, const Vector3 &unitVec );

    // Construct a rotation matrix from a unit-length quaternion
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 rotation( const Quat &unitQuat );

    // Construct a 4x4 matrix to perform scaling
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 scale( const Vector3 &scaleVec );

    // Construct a 4x4 matrix to perform translation
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 translation( const Vector3 &translateVec );

    // Construct viewing matrix based on eye, position looked at, and up direction
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 lookAt( const Point3 &eyePos, const Point3 &lookAtPos, const Vector3 &upVec );

    // Construct a perspective projection matrix
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 perspective( float fovyRadians, float aspect, float zNear, float zFar );

    // Construct a perspective projection matrix based on frustum
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 frustum( float left, float right, float bottom, float top, float zNear, float zFar );

    // Construct an orthographic projection matrix
    // 
    static VECTORMATH_FORCE_INLINE const Matrix4 orthographic( float left, float right, float bottom, float top, float zNear, float zFar );

};
// Multiply a 4x4 matrix by a scalar
// 
VECTORMATH_FORCE_INLINE const Matrix4 operator *( float scalar, const Matrix4 & mat );

// Multiply a 4x4 matrix by a scalar (scalar data contained in vector data type)
// 
VECTORMATH_FORCE_INLINE const Matrix4 operator *( const floatInVec &scalar, const Matrix4 & mat );

// Append (post-multiply) a scale transformation to a 4x4 matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
VECTORMATH_FORCE_INLINE const Matrix4 appendScale( const Matrix4 & mat, const Vector3 &scaleVec );

// Prepend (pre-multiply) a scale transformation to a 4x4 matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
VECTORMATH_FORCE_INLINE const Matrix4 prependScale( const Vector3 &scaleVec, const Matrix4 & mat );

// Multiply two 4x4 matrices per element
// 
VECTORMATH_FORCE_INLINE const Matrix4 mulPerElem( const Matrix4 & mat0, const Matrix4 & mat1 );

// Compute the absolute value of a 4x4 matrix per element
// 
VECTORMATH_FORCE_INLINE const Matrix4 absPerElem( const Matrix4 & mat );

// Transpose of a 4x4 matrix
// 
VECTORMATH_FORCE_INLINE const Matrix4 transpose( const Matrix4 & mat );

// Compute the inverse of a 4x4 matrix
// NOTE: 
// Result is unpredictable when the determinant of mat is equal to or near 0.
// 
VECTORMATH_FORCE_INLINE const Matrix4 inverse( const Matrix4 & mat );

// Compute the inverse of a 4x4 matrix, which is expected to be an affine matrix
// NOTE: 
// This can be used to achieve better performance than a general inverse when the specified 4x4 matrix meets the given restrictions.  The result is unpredictable when the determinant of mat is equal to or near 0.
// 
VECTORMATH_FORCE_INLINE const Matrix4 affineInverse( const Matrix4 & mat );

// Compute the inverse of a 4x4 matrix, which is expected to be an affine matrix with an orthogonal upper-left 3x3 submatrix
// NOTE: 
// This can be used to achieve better performance than a general inverse when the specified 4x4 matrix meets the given restrictions.
// 
VECTORMATH_FORCE_INLINE const Matrix4 orthoInverse( const Matrix4 & mat );

// Determinant of a 4x4 matrix
// 
VECTORMATH_FORCE_INLINE const floatInVec determinant( const Matrix4 & mat );

// Conditionally select between two 4x4 matrices
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the boolInVec version for better performance.
// 
VECTORMATH_FORCE_INLINE const Matrix4 select( const Matrix4 & mat0, const Matrix4 & mat1, bool select1 );

// Conditionally select between two 4x4 matrices (scalar data contained in vector data type)
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// 
VECTORMATH_FORCE_INLINE const Matrix4 select( const Matrix4 & mat0, const Matrix4 & mat1, const boolInVec &select1 );

#ifdef _VECTORMATH_DEBUG

// Print a 4x4 matrix
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Matrix4 & mat );

// Print a 4x4 matrix and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Matrix4 & mat, const char * name );

#endif

// A 3x4 transformation matrix in array-of-structures format
//
class Transform3
{
    Vector3 mCol0;
    Vector3 mCol1;
    Vector3 mCol2;
    Vector3 mCol3;

public:
    // Default constructor; does no initialization
    // 
    VECTORMATH_FORCE_INLINE Transform3( ) { };

    // Copy a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE Transform3( const Transform3 & tfrm );

    // Construct a 3x4 transformation matrix containing the specified columns
    // 
    VECTORMATH_FORCE_INLINE Transform3( const Vector3 &col0, const Vector3 &col1, const Vector3 &col2, const Vector3 &col3 );

    // Construct a 3x4 transformation matrix from a 3x3 matrix and a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE Transform3( const Matrix3 & tfrm, const Vector3 &translateVec );

    // Construct a 3x4 transformation matrix from a unit-length quaternion and a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE Transform3( const Quat &unitQuat, const Vector3 &translateVec );

    // Set all elements of a 3x4 transformation matrix to the same scalar value
    // 
    explicit VECTORMATH_FORCE_INLINE Transform3( float scalar );

    // Set all elements of a 3x4 transformation matrix to the same scalar value (scalar data contained in vector data type)
    // 
    explicit VECTORMATH_FORCE_INLINE Transform3( const floatInVec &scalar );

    // Assign one 3x4 transformation matrix to another
    // 
    VECTORMATH_FORCE_INLINE Transform3 & operator =( const Transform3 & tfrm );

    // Set the upper-left 3x3 submatrix
    // 
    VECTORMATH_FORCE_INLINE Transform3 & setUpper3x3( const Matrix3 & mat3 );

    // Get the upper-left 3x3 submatrix of a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE const Matrix3 getUpper3x3( ) const;

    // Set translation component
    // 
    VECTORMATH_FORCE_INLINE Transform3 & setTranslation( const Vector3 &translateVec );

    // Get the translation component of a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getTranslation( ) const;

    // Set column 0 of a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE Transform3 & setCol0( const Vector3 &col0 );

    // Set column 1 of a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE Transform3 & setCol1( const Vector3 &col1 );

    // Set column 2 of a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE Transform3 & setCol2( const Vector3 &col2 );

    // Set column 3 of a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE Transform3 & setCol3( const Vector3 &col3 );

    // Get column 0 of a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getCol0( ) const;

    // Get column 1 of a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getCol1( ) const;

    // Get column 2 of a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getCol2( ) const;

    // Get column 3 of a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getCol3( ) const;

    // Set the column of a 3x4 transformation matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE Transform3 & setCol( int col, const Vector3 &vec );

    // Set the row of a 3x4 transformation matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE Transform3 & setRow( int row, const Vector4 &vec );

    // Get the column of a 3x4 transformation matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE const Vector3 getCol( int col ) const;

    // Get the row of a 3x4 transformation matrix referred to by the specified index
    // 
    VECTORMATH_FORCE_INLINE const Vector4 getRow( int row ) const;

    // Subscripting operator to set or get a column
    // 
    VECTORMATH_FORCE_INLINE Vector3 & operator []( int col );

    // Subscripting operator to get a column
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator []( int col ) const;

    // Set the element of a 3x4 transformation matrix referred to by column and row indices
    // 
    VECTORMATH_FORCE_INLINE Transform3 & setElem( int col, int row, float val );

    // Set the element of a 3x4 transformation matrix referred to by column and row indices (scalar data contained in vector data type)
    // 
    VECTORMATH_FORCE_INLINE Transform3 & setElem( int col, int row, const floatInVec &val );

    // Get the element of a 3x4 transformation matrix referred to by column and row indices
    // 
    VECTORMATH_FORCE_INLINE const floatInVec getElem( int col, int row ) const;

    // Multiply a 3x4 transformation matrix by a 3-D vector
    // 
    VECTORMATH_FORCE_INLINE const Vector3 operator *( const Vector3 &vec ) const;

    // Multiply a 3x4 transformation matrix by a 3-D point
    // 
    VECTORMATH_FORCE_INLINE const Point3 operator *( const Point3 &pnt ) const;

    // Multiply two 3x4 transformation matrices
    // 
    VECTORMATH_FORCE_INLINE const Transform3 operator *( const Transform3 & tfrm ) const;

    // Perform compound assignment and multiplication by a 3x4 transformation matrix
    // 
    VECTORMATH_FORCE_INLINE Transform3 & operator *=( const Transform3 & tfrm );

    // Construct an identity 3x4 transformation matrix
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 identity( );

    // Construct a 3x4 transformation matrix to rotate around the x axis
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 rotationX( float radians );

    // Construct a 3x4 transformation matrix to rotate around the y axis
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 rotationY( float radians );

    // Construct a 3x4 transformation matrix to rotate around the z axis
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 rotationZ( float radians );

    // Construct a 3x4 transformation matrix to rotate around the x axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 rotationX( const floatInVec &radians );

    // Construct a 3x4 transformation matrix to rotate around the y axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 rotationY( const floatInVec &radians );

    // Construct a 3x4 transformation matrix to rotate around the z axis (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 rotationZ( const floatInVec &radians );

    // Construct a 3x4 transformation matrix to rotate around the x, y, and z axes
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 rotationZYX( const Vector3 &radiansXYZ );

    // Construct a 3x4 transformation matrix to rotate around a unit-length 3-D vector
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 rotation( float radians, const Vector3 &unitVec );

    // Construct a 3x4 transformation matrix to rotate around a unit-length 3-D vector (scalar data contained in vector data type)
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 rotation( const floatInVec &radians, const Vector3 &unitVec );

    // Construct a rotation matrix from a unit-length quaternion
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 rotation( const Quat &unitQuat );

    // Construct a 3x4 transformation matrix to perform scaling
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 scale( const Vector3 &scaleVec );

    // Construct a 3x4 transformation matrix to perform translation
    // 
    static VECTORMATH_FORCE_INLINE const Transform3 translation( const Vector3 &translateVec );

};
// Append (post-multiply) a scale transformation to a 3x4 transformation matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
VECTORMATH_FORCE_INLINE const Transform3 appendScale( const Transform3 & tfrm, const Vector3 &scaleVec );

// Prepend (pre-multiply) a scale transformation to a 3x4 transformation matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
VECTORMATH_FORCE_INLINE const Transform3 prependScale( const Vector3 &scaleVec, const Transform3 & tfrm );

// Multiply two 3x4 transformation matrices per element
// 
VECTORMATH_FORCE_INLINE const Transform3 mulPerElem( const Transform3 & tfrm0, const Transform3 & tfrm1 );

// Compute the absolute value of a 3x4 transformation matrix per element
// 
VECTORMATH_FORCE_INLINE const Transform3 absPerElem( const Transform3 & tfrm );

// Inverse of a 3x4 transformation matrix
// NOTE: 
// Result is unpredictable when the determinant of the left 3x3 submatrix is equal to or near 0.
// 
VECTORMATH_FORCE_INLINE const Transform3 inverse( const Transform3 & tfrm );

// Compute the inverse of a 3x4 transformation matrix, expected to have an orthogonal upper-left 3x3 submatrix
// NOTE: 
// This can be used to achieve better performance than a general inverse when the specified 3x4 transformation matrix meets the given restrictions.
// 
VECTORMATH_FORCE_INLINE const Transform3 orthoInverse( const Transform3 & tfrm );

// Conditionally select between two 3x4 transformation matrices
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// However, the transfer of select1 to a VMX register may use more processing time than a branch.
// Use the boolInVec version for better performance.
// 
VECTORMATH_FORCE_INLINE const Transform3 select( const Transform3 & tfrm0, const Transform3 & tfrm1, bool select1 );

// Conditionally select between two 3x4 transformation matrices (scalar data contained in vector data type)
// NOTE: 
// This function uses a conditional select instruction to avoid a branch.
// 
VECTORMATH_FORCE_INLINE const Transform3 select( const Transform3 & tfrm0, const Transform3 & tfrm1, const boolInVec &select1 );

#ifdef _VECTORMATH_DEBUG

// Print a 3x4 transformation matrix
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Transform3 & tfrm );

// Print a 3x4 transformation matrix and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
VECTORMATH_FORCE_INLINE void print( const Transform3 & tfrm, const char * name );

#endif

} // namespace Aos
} // namespace Vectormath

#include "vec_aos.h"
#include "quat_aos.h"
#include "mat_aos.h"

#endif
