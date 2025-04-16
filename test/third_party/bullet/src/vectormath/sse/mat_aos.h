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


#ifndef _VECTORMATH_MAT_AOS_CPP_H
#define _VECTORMATH_MAT_AOS_CPP_H

namespace Vectormath {
namespace Aos {

//-----------------------------------------------------------------------------
// Constants
// for shuffles, words are labeled [x,y,z,w] [a,b,c,d]

#define _VECTORMATH_PERM_ZBWX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Z, _VECTORMATH_PERM_B, _VECTORMATH_PERM_W, _VECTORMATH_PERM_X })
#define _VECTORMATH_PERM_XCYX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_C, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_X })
#define _VECTORMATH_PERM_XYAB ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_A, _VECTORMATH_PERM_B })
#define _VECTORMATH_PERM_ZWCD ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Z, _VECTORMATH_PERM_W, _VECTORMATH_PERM_C, _VECTORMATH_PERM_D })
#define _VECTORMATH_PERM_XZBX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_Z, _VECTORMATH_PERM_B, _VECTORMATH_PERM_X })     
#define _VECTORMATH_PERM_CXXX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_C, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X })
#define _VECTORMATH_PERM_YAXX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Y, _VECTORMATH_PERM_A, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X })
#define _VECTORMATH_PERM_XAZC ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_A, _VECTORMATH_PERM_Z, _VECTORMATH_PERM_C })
#define _VECTORMATH_PERM_YXWZ ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Y, _VECTORMATH_PERM_X, _VECTORMATH_PERM_W, _VECTORMATH_PERM_Z })
#define _VECTORMATH_PERM_YBWD ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Y, _VECTORMATH_PERM_B, _VECTORMATH_PERM_W, _VECTORMATH_PERM_D })
#define _VECTORMATH_PERM_XYCX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_C, _VECTORMATH_PERM_X })
#define _VECTORMATH_PERM_YCXY ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Y, _VECTORMATH_PERM_C, _VECTORMATH_PERM_X, _VECTORMATH_PERM_Y })
#define _VECTORMATH_PERM_CXYC ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_C, _VECTORMATH_PERM_X, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_C })
#define _VECTORMATH_PERM_ZAYX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Z, _VECTORMATH_PERM_A, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_X })
#define _VECTORMATH_PERM_BZXX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_B, _VECTORMATH_PERM_Z, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X })
#define _VECTORMATH_PERM_XZYA ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_X, _VECTORMATH_PERM_Z, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_A })
#define _VECTORMATH_PERM_ZXXB ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Z, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X, _VECTORMATH_PERM_B })
#define _VECTORMATH_PERM_YXXC ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_Y, _VECTORMATH_PERM_X, _VECTORMATH_PERM_X, _VECTORMATH_PERM_C })
#define _VECTORMATH_PERM_BBYX ((vec_uchar16)(vec_uint4){ _VECTORMATH_PERM_B, _VECTORMATH_PERM_B, _VECTORMATH_PERM_Y, _VECTORMATH_PERM_X })
#define _VECTORMATH_PI_OVER_2 1.570796327f

//-----------------------------------------------------------------------------
// Definitions

VECTORMATH_FORCE_INLINE Matrix3::Matrix3( const Matrix3 & mat )
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
}

VECTORMATH_FORCE_INLINE Matrix3::Matrix3( float scalar )
{
    mCol0 = Vector3( scalar );
    mCol1 = Vector3( scalar );
    mCol2 = Vector3( scalar );
}

VECTORMATH_FORCE_INLINE Matrix3::Matrix3( const floatInVec &scalar )
{
    mCol0 = Vector3( scalar );
    mCol1 = Vector3( scalar );
    mCol2 = Vector3( scalar );
}

VECTORMATH_FORCE_INLINE Matrix3::Matrix3( const Quat &unitQuat )
{
    __m128 xyzw_2, wwww, yzxw, zxyw, yzxw_2, zxyw_2;
    __m128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5;
	VM_ATTRIBUTE_ALIGN16 unsigned int sx[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int sz[4] = {0, 0, 0xffffffff, 0};
	__m128 select_x = _mm_load_ps((float *)sx);
	__m128 select_z = _mm_load_ps((float *)sz);

    xyzw_2 = _mm_add_ps( unitQuat.get128(), unitQuat.get128() );
    wwww = _mm_shuffle_ps( unitQuat.get128(), unitQuat.get128(), _MM_SHUFFLE(3,3,3,3) );
	yzxw = _mm_shuffle_ps( unitQuat.get128(), unitQuat.get128(), _MM_SHUFFLE(3,0,2,1) );
	zxyw = _mm_shuffle_ps( unitQuat.get128(), unitQuat.get128(), _MM_SHUFFLE(3,1,0,2) );
    yzxw_2 = _mm_shuffle_ps( xyzw_2, xyzw_2, _MM_SHUFFLE(3,0,2,1) );
    zxyw_2 = _mm_shuffle_ps( xyzw_2, xyzw_2, _MM_SHUFFLE(3,1,0,2) );

    tmp0 = _mm_mul_ps( yzxw_2, wwww );									// tmp0 = 2yw, 2zw, 2xw, 2w2
	tmp1 = _mm_sub_ps( _mm_set1_ps(1.0f), _mm_mul_ps(yzxw, yzxw_2) );	// tmp1 = 1 - 2y2, 1 - 2z2, 1 - 2x2, 1 - 2w2
    tmp2 = _mm_mul_ps( yzxw, xyzw_2 );									// tmp2 = 2xy, 2yz, 2xz, 2w2
    tmp0 = _mm_add_ps( _mm_mul_ps(zxyw, xyzw_2), tmp0 );				// tmp0 = 2yw + 2zx, 2zw + 2xy, 2xw + 2yz, 2w2 + 2w2
    tmp1 = _mm_sub_ps( tmp1, _mm_mul_ps(zxyw, zxyw_2) );				// tmp1 = 1 - 2y2 - 2z2, 1 - 2z2 - 2x2, 1 - 2x2 - 2y2, 1 - 2w2 - 2w2
    tmp2 = _mm_sub_ps( tmp2, _mm_mul_ps(zxyw_2, wwww) );				// tmp2 = 2xy - 2zw, 2yz - 2xw, 2xz - 2yw, 2w2 -2w2

    tmp3 = vec_sel( tmp0, tmp1, select_x );
    tmp4 = vec_sel( tmp1, tmp2, select_x );
    tmp5 = vec_sel( tmp2, tmp0, select_x );
    mCol0 = Vector3( vec_sel( tmp3, tmp2, select_z ) );
    mCol1 = Vector3( vec_sel( tmp4, tmp0, select_z ) );
    mCol2 = Vector3( vec_sel( tmp5, tmp1, select_z ) );
}

VECTORMATH_FORCE_INLINE Matrix3::Matrix3( const Vector3 &_col0, const Vector3 &_col1, const Vector3 &_col2 )
{
    mCol0 = _col0;
    mCol1 = _col1;
    mCol2 = _col2;
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::setCol0( const Vector3 &_col0 )
{
    mCol0 = _col0;
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::setCol1( const Vector3 &_col1 )
{
    mCol1 = _col1;
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::setCol2( const Vector3 &_col2 )
{
    mCol2 = _col2;
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::setCol( int col, const Vector3 &vec )
{
    *(&mCol0 + col) = vec;
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::setRow( int row, const Vector3 &vec )
{
    mCol0.setElem( row, vec.getElem( 0 ) );
    mCol1.setElem( row, vec.getElem( 1 ) );
    mCol2.setElem( row, vec.getElem( 2 ) );
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::setElem( int col, int row, float val )
{
    (*this)[col].setElem(row, val);
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::setElem( int col, int row, const floatInVec &val )
{
    Vector3 tmpV3_0;
    tmpV3_0 = this->getCol( col );
    tmpV3_0.setElem( row, val );
    this->setCol( col, tmpV3_0 );
    return *this;
}

VECTORMATH_FORCE_INLINE const floatInVec Matrix3::getElem( int col, int row ) const
{
    return this->getCol( col ).getElem( row );
}

VECTORMATH_FORCE_INLINE const Vector3 Matrix3::getCol0( ) const
{
    return mCol0;
}

VECTORMATH_FORCE_INLINE const Vector3 Matrix3::getCol1( ) const
{
    return mCol1;
}

VECTORMATH_FORCE_INLINE const Vector3 Matrix3::getCol2( ) const
{
    return mCol2;
}

VECTORMATH_FORCE_INLINE const Vector3 Matrix3::getCol( int col ) const
{
    return *(&mCol0 + col);
}

VECTORMATH_FORCE_INLINE const Vector3 Matrix3::getRow( int row ) const
{
    return Vector3( mCol0.getElem( row ), mCol1.getElem( row ), mCol2.getElem( row ) );
}

VECTORMATH_FORCE_INLINE Vector3 & Matrix3::operator []( int col )
{
    return *(&mCol0 + col);
}

VECTORMATH_FORCE_INLINE const Vector3 Matrix3::operator []( int col ) const
{
    return *(&mCol0 + col);
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::operator =( const Matrix3 & mat )
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
    return *this;
}

VECTORMATH_FORCE_INLINE const Matrix3 transpose( const Matrix3 & mat )
{
    __m128 tmp0, tmp1, res0, res1, res2;
    tmp0 = vec_mergeh( mat.getCol0().get128(), mat.getCol2().get128() );
    tmp1 = vec_mergel( mat.getCol0().get128(), mat.getCol2().get128() );
    res0 = vec_mergeh( tmp0, mat.getCol1().get128() );
    //res1 = vec_perm( tmp0, mat.getCol1().get128(), _VECTORMATH_PERM_ZBWX );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	res1 = _mm_shuffle_ps( tmp0, tmp0, _MM_SHUFFLE(0,3,2,2));
	res1 = vec_sel(res1, mat.getCol1().get128(), select_y);
    //res2 = vec_perm( tmp1, mat.getCol1().get128(), _VECTORMATH_PERM_XCYX );
	res2 = _mm_shuffle_ps( tmp1, tmp1, _MM_SHUFFLE(0,1,1,0));
	res2 = vec_sel(res2, vec_splat(mat.getCol1().get128(), 2), select_y);
    return Matrix3(
        Vector3( res0 ),
        Vector3( res1 ),
        Vector3( res2 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 inverse( const Matrix3 & mat )
{
    __m128 tmp0, tmp1, tmp2, tmp3, tmp4, dot, invdet, inv0, inv1, inv2;
    tmp2 = _vmathVfCross( mat.getCol0().get128(), mat.getCol1().get128() );
    tmp0 = _vmathVfCross( mat.getCol1().get128(), mat.getCol2().get128() );
    tmp1 = _vmathVfCross( mat.getCol2().get128(), mat.getCol0().get128() );
    dot = _vmathVfDot3( tmp2, mat.getCol2().get128() );
    dot = vec_splat( dot, 0 );
    invdet = recipf4( dot );
    tmp3 = vec_mergeh( tmp0, tmp2 );
    tmp4 = vec_mergel( tmp0, tmp2 );
    inv0 = vec_mergeh( tmp3, tmp1 );
    //inv1 = vec_perm( tmp3, tmp1, _VECTORMATH_PERM_ZBWX );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	inv1 = _mm_shuffle_ps( tmp3, tmp3, _MM_SHUFFLE(0,3,2,2));
	inv1 = vec_sel(inv1, tmp1, select_y);
    //inv2 = vec_perm( tmp4, tmp1, _VECTORMATH_PERM_XCYX );
	inv2 = _mm_shuffle_ps( tmp4, tmp4, _MM_SHUFFLE(0,1,1,0));
	inv2 = vec_sel(inv2, vec_splat(tmp1, 2), select_y);
    inv0 = vec_mul( inv0, invdet );
    inv1 = vec_mul( inv1, invdet );
	inv2 = vec_mul( inv2, invdet );
    return Matrix3(
        Vector3( inv0 ),
        Vector3( inv1 ),
        Vector3( inv2 )
    );
}

VECTORMATH_FORCE_INLINE const floatInVec determinant( const Matrix3 & mat )
{
    return dot( mat.getCol2(), cross( mat.getCol0(), mat.getCol1() ) );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::operator +( const Matrix3 & mat ) const
{
    return Matrix3(
        ( mCol0 + mat.mCol0 ),
        ( mCol1 + mat.mCol1 ),
        ( mCol2 + mat.mCol2 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::operator -( const Matrix3 & mat ) const
{
    return Matrix3(
        ( mCol0 - mat.mCol0 ),
        ( mCol1 - mat.mCol1 ),
        ( mCol2 - mat.mCol2 )
    );
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::operator +=( const Matrix3 & mat )
{
    *this = *this + mat;
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::operator -=( const Matrix3 & mat )
{
    *this = *this - mat;
    return *this;
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::operator -( ) const
{
    return Matrix3(
        ( -mCol0 ),
        ( -mCol1 ),
        ( -mCol2 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 absPerElem( const Matrix3 & mat )
{
    return Matrix3(
        absPerElem( mat.getCol0() ),
        absPerElem( mat.getCol1() ),
        absPerElem( mat.getCol2() )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::operator *( float scalar ) const
{
    return *this * floatInVec(scalar);
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::operator *( const floatInVec &scalar ) const
{
    return Matrix3(
        ( mCol0 * scalar ),
        ( mCol1 * scalar ),
        ( mCol2 * scalar )
    );
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::operator *=( float scalar )
{
    return *this *= floatInVec(scalar);
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::operator *=( const floatInVec &scalar )
{
    *this = *this * scalar;
    return *this;
}

VECTORMATH_FORCE_INLINE const Matrix3 operator *( float scalar, const Matrix3 & mat )
{
    return floatInVec(scalar) * mat;
}

VECTORMATH_FORCE_INLINE const Matrix3 operator *( const floatInVec &scalar, const Matrix3 & mat )
{
    return mat * scalar;
}

VECTORMATH_FORCE_INLINE const Vector3 Matrix3::operator *( const Vector3 &vec ) const
{
    __m128 res;
    __m128 xxxx, yyyy, zzzz;
    xxxx = vec_splat( vec.get128(), 0 );
    yyyy = vec_splat( vec.get128(), 1 );
    zzzz = vec_splat( vec.get128(), 2 );
    res = vec_mul( mCol0.get128(), xxxx );
    res = vec_madd( mCol1.get128(), yyyy, res );
    res = vec_madd( mCol2.get128(), zzzz, res );
    return Vector3( res );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::operator *( const Matrix3 & mat ) const
{
    return Matrix3(
        ( *this * mat.mCol0 ),
        ( *this * mat.mCol1 ),
        ( *this * mat.mCol2 )
    );
}

VECTORMATH_FORCE_INLINE Matrix3 & Matrix3::operator *=( const Matrix3 & mat )
{
    *this = *this * mat;
    return *this;
}

VECTORMATH_FORCE_INLINE const Matrix3 mulPerElem( const Matrix3 & mat0, const Matrix3 & mat1 )
{
    return Matrix3(
        mulPerElem( mat0.getCol0(), mat1.getCol0() ),
        mulPerElem( mat0.getCol1(), mat1.getCol1() ),
        mulPerElem( mat0.getCol2(), mat1.getCol2() )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::identity( )
{
    return Matrix3(
        Vector3::xAxis( ),
        Vector3::yAxis( ),
        Vector3::zAxis( )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::rotationX( float radians )
{
    return rotationX( floatInVec(radians) );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::rotationX( const floatInVec &radians )
{
    __m128 s, c, res1, res2;
    __m128 zero;
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    zero = _mm_setzero_ps();
    sincosf4( radians.get128(), &s, &c );
    res1 = vec_sel( zero, c, select_y );
    res1 = vec_sel( res1, s, select_z );
    res2 = vec_sel( zero, negatef4(s), select_y );
    res2 = vec_sel( res2, c, select_z );
    return Matrix3(
        Vector3::xAxis( ),
        Vector3( res1 ),
        Vector3( res2 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::rotationY( float radians )
{
    return rotationY( floatInVec(radians) );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::rotationY( const floatInVec &radians )
{
    __m128 s, c, res0, res2;
    __m128 zero;
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    zero = _mm_setzero_ps();
    sincosf4( radians.get128(), &s, &c );
    res0 = vec_sel( zero, c, select_x );
    res0 = vec_sel( res0, negatef4(s), select_z );
    res2 = vec_sel( zero, s, select_x );
    res2 = vec_sel( res2, c, select_z );
    return Matrix3(
        Vector3( res0 ),
        Vector3::yAxis( ),
        Vector3( res2 )
	);
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::rotationZ( float radians )
{
    return rotationZ( floatInVec(radians) );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::rotationZ( const floatInVec &radians )
{
    __m128 s, c, res0, res1;
    __m128 zero;
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
    zero = _mm_setzero_ps();
    sincosf4( radians.get128(), &s, &c );
    res0 = vec_sel( zero, c, select_x );
    res0 = vec_sel( res0, s, select_y );
    res1 = vec_sel( zero, negatef4(s), select_x );
    res1 = vec_sel( res1, c, select_y );
    return Matrix3(
        Vector3( res0 ),
        Vector3( res1 ),
        Vector3::zAxis( )
	);
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::rotationZYX( const Vector3 &radiansXYZ )
{
    __m128 angles, s, negS, c, X0, X1, Y0, Y1, Z0, Z1, tmp;
    angles = Vector4( radiansXYZ, 0.0f ).get128();
    sincosf4( angles, &s, &c );
    negS = negatef4( s );
    Z0 = vec_mergel( c, s );
    Z1 = vec_mergel( negS, c );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_xyz[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0};
    Z1 = vec_and( Z1, _mm_load_ps( (float *)select_xyz ) );
	Y0 = _mm_shuffle_ps( c, negS, _MM_SHUFFLE(0,1,1,1) );
	Y1 = _mm_shuffle_ps( s, c, _MM_SHUFFLE(0,1,1,1) );
    X0 = vec_splat( s, 0 );
    X1 = vec_splat( c, 0 );
    tmp = vec_mul( Z0, Y1 );
    return Matrix3(
        Vector3( vec_mul( Z0, Y0 ) ),
        Vector3( vec_madd( Z1, X1, vec_mul( tmp, X0 ) ) ),
        Vector3( vec_nmsub( Z1, X0, vec_mul( tmp, X1 ) ) )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::rotation( float radians, const Vector3 &unitVec )
{
    return rotation( floatInVec(radians), unitVec );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::rotation( const floatInVec &radians, const Vector3 &unitVec )
{
    __m128 axis, s, c, oneMinusC, axisS, negAxisS, xxxx, yyyy, zzzz, tmp0, tmp1, tmp2;
    axis = unitVec.get128();
    sincosf4( radians.get128(), &s, &c );
    xxxx = vec_splat( axis, 0 );
    yyyy = vec_splat( axis, 1 );
    zzzz = vec_splat( axis, 2 );
    oneMinusC = vec_sub( _mm_set1_ps(1.0f), c );
    axisS = vec_mul( axis, s );
    negAxisS = negatef4( axisS );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    //tmp0 = vec_perm( axisS, negAxisS, _VECTORMATH_PERM_XZBX );
	tmp0 = _mm_shuffle_ps( axisS, axisS, _MM_SHUFFLE(0,0,2,0) );
	tmp0 = vec_sel(tmp0, vec_splat(negAxisS, 1), select_z);
    //tmp1 = vec_perm( axisS, negAxisS, _VECTORMATH_PERM_CXXX );
	tmp1 = vec_sel( vec_splat(axisS, 0), vec_splat(negAxisS, 2), select_x );
    //tmp2 = vec_perm( axisS, negAxisS, _VECTORMATH_PERM_YAXX );
	tmp2 = _mm_shuffle_ps( axisS, axisS, _MM_SHUFFLE(0,0,0,1) );
	tmp2 = vec_sel(tmp2, vec_splat(negAxisS, 0), select_y);
    tmp0 = vec_sel( tmp0, c, select_x );
    tmp1 = vec_sel( tmp1, c, select_y );
    tmp2 = vec_sel( tmp2, c, select_z );
    return Matrix3(
        Vector3( vec_madd( vec_mul( axis, xxxx ), oneMinusC, tmp0 ) ),
        Vector3( vec_madd( vec_mul( axis, yyyy ), oneMinusC, tmp1 ) ),
        Vector3( vec_madd( vec_mul( axis, zzzz ), oneMinusC, tmp2 ) )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::rotation( const Quat &unitQuat )
{
    return Matrix3( unitQuat );
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix3::scale( const Vector3 &scaleVec )
{
    __m128 zero = _mm_setzero_ps();
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    return Matrix3(
        Vector3( vec_sel( zero, scaleVec.get128(), select_x ) ),
        Vector3( vec_sel( zero, scaleVec.get128(), select_y ) ),
        Vector3( vec_sel( zero, scaleVec.get128(), select_z ) )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 appendScale( const Matrix3 & mat, const Vector3 &scaleVec )
{
    return Matrix3(
        ( mat.getCol0() * scaleVec.getX( ) ),
        ( mat.getCol1() * scaleVec.getY( ) ),
        ( mat.getCol2() * scaleVec.getZ( ) )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 prependScale( const Vector3 &scaleVec, const Matrix3 & mat )
{
    return Matrix3(
        mulPerElem( mat.getCol0(), scaleVec ),
        mulPerElem( mat.getCol1(), scaleVec ),
        mulPerElem( mat.getCol2(), scaleVec )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 select( const Matrix3 & mat0, const Matrix3 & mat1, bool select1 )
{
    return Matrix3(
        select( mat0.getCol0(), mat1.getCol0(), select1 ),
        select( mat0.getCol1(), mat1.getCol1(), select1 ),
        select( mat0.getCol2(), mat1.getCol2(), select1 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 select( const Matrix3 & mat0, const Matrix3 & mat1, const boolInVec &select1 )
{
    return Matrix3(
        select( mat0.getCol0(), mat1.getCol0(), select1 ),
        select( mat0.getCol1(), mat1.getCol1(), select1 ),
        select( mat0.getCol2(), mat1.getCol2(), select1 )
    );
}

#ifdef _VECTORMATH_DEBUG

VECTORMATH_FORCE_INLINE void print( const Matrix3 & mat )
{
    print( mat.getRow( 0 ) );
    print( mat.getRow( 1 ) );
    print( mat.getRow( 2 ) );
}

VECTORMATH_FORCE_INLINE void print( const Matrix3 & mat, const char * name )
{
    printf("%s:\n", name);
    print( mat );
}

#endif

VECTORMATH_FORCE_INLINE Matrix4::Matrix4( const Matrix4 & mat )
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
    mCol3 = mat.mCol3;
}

VECTORMATH_FORCE_INLINE Matrix4::Matrix4( float scalar )
{
    mCol0 = Vector4( scalar );
    mCol1 = Vector4( scalar );
    mCol2 = Vector4( scalar );
    mCol3 = Vector4( scalar );
}

VECTORMATH_FORCE_INLINE Matrix4::Matrix4( const floatInVec &scalar )
{
    mCol0 = Vector4( scalar );
    mCol1 = Vector4( scalar );
    mCol2 = Vector4( scalar );
    mCol3 = Vector4( scalar );
}

VECTORMATH_FORCE_INLINE Matrix4::Matrix4( const Transform3 & mat )
{
    mCol0 = Vector4( mat.getCol0(), 0.0f );
    mCol1 = Vector4( mat.getCol1(), 0.0f );
    mCol2 = Vector4( mat.getCol2(), 0.0f );
    mCol3 = Vector4( mat.getCol3(), 1.0f );
}

VECTORMATH_FORCE_INLINE Matrix4::Matrix4( const Vector4 &_col0, const Vector4 &_col1, const Vector4 &_col2, const Vector4 &_col3 )
{
    mCol0 = _col0;
    mCol1 = _col1;
    mCol2 = _col2;
    mCol3 = _col3;
}

VECTORMATH_FORCE_INLINE Matrix4::Matrix4( const Matrix3 & mat, const Vector3 &translateVec )
{
    mCol0 = Vector4( mat.getCol0(), 0.0f );
    mCol1 = Vector4( mat.getCol1(), 0.0f );
    mCol2 = Vector4( mat.getCol2(), 0.0f );
    mCol3 = Vector4( translateVec, 1.0f );
}

VECTORMATH_FORCE_INLINE Matrix4::Matrix4( const Quat &unitQuat, const Vector3 &translateVec )
{
    Matrix3 mat;
    mat = Matrix3( unitQuat );
    mCol0 = Vector4( mat.getCol0(), 0.0f );
    mCol1 = Vector4( mat.getCol1(), 0.0f );
    mCol2 = Vector4( mat.getCol2(), 0.0f );
    mCol3 = Vector4( translateVec, 1.0f );
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::setCol0( const Vector4 &_col0 )
{
    mCol0 = _col0;
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::setCol1( const Vector4 &_col1 )
{
    mCol1 = _col1;
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::setCol2( const Vector4 &_col2 )
{
    mCol2 = _col2;
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::setCol3( const Vector4 &_col3 )
{
    mCol3 = _col3;
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::setCol( int col, const Vector4 &vec )
{
    *(&mCol0 + col) = vec;
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::setRow( int row, const Vector4 &vec )
{
    mCol0.setElem( row, vec.getElem( 0 ) );
    mCol1.setElem( row, vec.getElem( 1 ) );
    mCol2.setElem( row, vec.getElem( 2 ) );
    mCol3.setElem( row, vec.getElem( 3 ) );
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::setElem( int col, int row, float val )
{
    (*this)[col].setElem(row, val);
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::setElem( int col, int row, const floatInVec &val )
{
    Vector4 tmpV3_0;
    tmpV3_0 = this->getCol( col );
    tmpV3_0.setElem( row, val );
    this->setCol( col, tmpV3_0 );
    return *this;
}

VECTORMATH_FORCE_INLINE const floatInVec Matrix4::getElem( int col, int row ) const
{
    return this->getCol( col ).getElem( row );
}

VECTORMATH_FORCE_INLINE const Vector4 Matrix4::getCol0( ) const
{
    return mCol0;
}

VECTORMATH_FORCE_INLINE const Vector4 Matrix4::getCol1( ) const
{
    return mCol1;
}

VECTORMATH_FORCE_INLINE const Vector4 Matrix4::getCol2( ) const
{
    return mCol2;
}

VECTORMATH_FORCE_INLINE const Vector4 Matrix4::getCol3( ) const
{
    return mCol3;
}

VECTORMATH_FORCE_INLINE const Vector4 Matrix4::getCol( int col ) const
{
    return *(&mCol0 + col);
}

VECTORMATH_FORCE_INLINE const Vector4 Matrix4::getRow( int row ) const
{
    return Vector4( mCol0.getElem( row ), mCol1.getElem( row ), mCol2.getElem( row ), mCol3.getElem( row ) );
}

VECTORMATH_FORCE_INLINE Vector4 & Matrix4::operator []( int col )
{
    return *(&mCol0 + col);
}

VECTORMATH_FORCE_INLINE const Vector4 Matrix4::operator []( int col ) const
{
    return *(&mCol0 + col);
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::operator =( const Matrix4 & mat )
{
    mCol0 = mat.mCol0;
    mCol1 = mat.mCol1;
    mCol2 = mat.mCol2;
    mCol3 = mat.mCol3;
    return *this;
}

VECTORMATH_FORCE_INLINE const Matrix4 transpose( const Matrix4 & mat )
{
    __m128 tmp0, tmp1, tmp2, tmp3, res0, res1, res2, res3;
    tmp0 = vec_mergeh( mat.getCol0().get128(), mat.getCol2().get128() );
    tmp1 = vec_mergeh( mat.getCol1().get128(), mat.getCol3().get128() );
    tmp2 = vec_mergel( mat.getCol0().get128(), mat.getCol2().get128() );
    tmp3 = vec_mergel( mat.getCol1().get128(), mat.getCol3().get128() );
    res0 = vec_mergeh( tmp0, tmp1 );
    res1 = vec_mergel( tmp0, tmp1 );
    res2 = vec_mergeh( tmp2, tmp3 );
    res3 = vec_mergel( tmp2, tmp3 );
    return Matrix4(
        Vector4( res0 ),
        Vector4( res1 ),
        Vector4( res2 ),
        Vector4( res3 )
    );
}

// TODO: Tidy
static VM_ATTRIBUTE_ALIGN16 const unsigned int _vmathPNPN[4] = {0x00000000, 0x80000000, 0x00000000, 0x80000000};
static VM_ATTRIBUTE_ALIGN16 const unsigned int _vmathNPNP[4] = {0x80000000, 0x00000000, 0x80000000, 0x00000000};
static VM_ATTRIBUTE_ALIGN16 const float _vmathZERONE[4] = {1.0f, 0.0f, 0.0f, 1.0f};

VECTORMATH_FORCE_INLINE const Matrix4 inverse( const Matrix4 & mat )
{
	__m128 Va,Vb,Vc;
	__m128 r1,r2,r3,tt,tt2;
	__m128 sum,Det,RDet;
	__m128 trns0,trns1,trns2,trns3;

	__m128 _L1 = mat.getCol0().get128();
	__m128 _L2 = mat.getCol1().get128();
	__m128 _L3 = mat.getCol2().get128();
	__m128 _L4 = mat.getCol3().get128();
	// Calculating the minterms for the first line.

	// _mm_ror_ps is just a macro using _mm_shuffle_ps().
	tt = _L4; tt2 = _mm_ror_ps(_L3,1); 
	Vc = _mm_mul_ps(tt2,_mm_ror_ps(tt,0));					// V3'dot V4
	Va = _mm_mul_ps(tt2,_mm_ror_ps(tt,2));					// V3'dot V4"
	Vb = _mm_mul_ps(tt2,_mm_ror_ps(tt,3));					// V3' dot V4^

	r1 = _mm_sub_ps(_mm_ror_ps(Va,1),_mm_ror_ps(Vc,2));		// V3" dot V4^ - V3^ dot V4"
	r2 = _mm_sub_ps(_mm_ror_ps(Vb,2),_mm_ror_ps(Vb,0));		// V3^ dot V4' - V3' dot V4^
	r3 = _mm_sub_ps(_mm_ror_ps(Va,0),_mm_ror_ps(Vc,1));		// V3' dot V4" - V3" dot V4'

	tt = _L2;
	Va = _mm_ror_ps(tt,1);		sum = _mm_mul_ps(Va,r1);
	Vb = _mm_ror_ps(tt,2);		sum = _mm_add_ps(sum,_mm_mul_ps(Vb,r2));
	Vc = _mm_ror_ps(tt,3);		sum = _mm_add_ps(sum,_mm_mul_ps(Vc,r3));

	// Calculating the determinant.
	Det = _mm_mul_ps(sum,_L1);
	Det = _mm_add_ps(Det,_mm_movehl_ps(Det,Det));

	const __m128 Sign_PNPN = _mm_load_ps((float *)_vmathPNPN);
	const __m128 Sign_NPNP = _mm_load_ps((float *)_vmathNPNP);

	__m128 mtL1 = _mm_xor_ps(sum,Sign_PNPN);

	// Calculating the minterms of the second line (using previous results).
	tt = _mm_ror_ps(_L1,1);		sum = _mm_mul_ps(tt,r1);
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r2));
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r3));
	__m128 mtL2 = _mm_xor_ps(sum,Sign_NPNP);

	// Testing the determinant.
	Det = _mm_sub_ss(Det,_mm_shuffle_ps(Det,Det,1));

	// Calculating the minterms of the third line.
	tt = _mm_ror_ps(_L1,1);
	Va = _mm_mul_ps(tt,Vb);									// V1' dot V2"
	Vb = _mm_mul_ps(tt,Vc);									// V1' dot V2^
	Vc = _mm_mul_ps(tt,_L2);								// V1' dot V2

	r1 = _mm_sub_ps(_mm_ror_ps(Va,1),_mm_ror_ps(Vc,2));		// V1" dot V2^ - V1^ dot V2"
	r2 = _mm_sub_ps(_mm_ror_ps(Vb,2),_mm_ror_ps(Vb,0));		// V1^ dot V2' - V1' dot V2^
	r3 = _mm_sub_ps(_mm_ror_ps(Va,0),_mm_ror_ps(Vc,1));		// V1' dot V2" - V1" dot V2'

	tt = _mm_ror_ps(_L4,1);		sum = _mm_mul_ps(tt,r1);
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r2));
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r3));
	__m128 mtL3 = _mm_xor_ps(sum,Sign_PNPN);

	// Dividing is FASTER than rcp_nr! (Because rcp_nr causes many register-memory RWs).
	RDet = _mm_div_ss(_mm_load_ss((float *)&_vmathZERONE), Det); // TODO: just 1.0f?
	RDet = _mm_shuffle_ps(RDet,RDet,0x00);

	// Devide the first 12 minterms with the determinant.
	mtL1 = _mm_mul_ps(mtL1, RDet);
	mtL2 = _mm_mul_ps(mtL2, RDet);
	mtL3 = _mm_mul_ps(mtL3, RDet);

	// Calculate the minterms of the forth line and devide by the determinant.
	tt = _mm_ror_ps(_L3,1);		sum = _mm_mul_ps(tt,r1);
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r2));
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r3));
	__m128 mtL4 = _mm_xor_ps(sum,Sign_NPNP);
	mtL4 = _mm_mul_ps(mtL4, RDet);

	// Now we just have to transpose the minterms matrix.
	trns0 = _mm_unpacklo_ps(mtL1,mtL2);
	trns1 = _mm_unpacklo_ps(mtL3,mtL4);
	trns2 = _mm_unpackhi_ps(mtL1,mtL2);
	trns3 = _mm_unpackhi_ps(mtL3,mtL4);
	_L1 = _mm_movelh_ps(trns0,trns1);
	_L2 = _mm_movehl_ps(trns1,trns0);
	_L3 = _mm_movelh_ps(trns2,trns3);
	_L4 = _mm_movehl_ps(trns3,trns2);

    return Matrix4(
        Vector4( _L1 ),
        Vector4( _L2 ),
        Vector4( _L3 ),
        Vector4( _L4 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 affineInverse( const Matrix4 & mat )
{
    Transform3 affineMat;
    affineMat.setCol0( mat.getCol0().getXYZ( ) );
    affineMat.setCol1( mat.getCol1().getXYZ( ) );
    affineMat.setCol2( mat.getCol2().getXYZ( ) );
    affineMat.setCol3( mat.getCol3().getXYZ( ) );
    return Matrix4( inverse( affineMat ) );
}

VECTORMATH_FORCE_INLINE const Matrix4 orthoInverse( const Matrix4 & mat )
{
    Transform3 affineMat;
    affineMat.setCol0( mat.getCol0().getXYZ( ) );
    affineMat.setCol1( mat.getCol1().getXYZ( ) );
    affineMat.setCol2( mat.getCol2().getXYZ( ) );
    affineMat.setCol3( mat.getCol3().getXYZ( ) );
    return Matrix4( orthoInverse( affineMat ) );
}

VECTORMATH_FORCE_INLINE const floatInVec determinant( const Matrix4 & mat )
{
	__m128 Va,Vb,Vc;
	__m128 r1,r2,r3,tt,tt2;
	__m128 sum,Det;

	__m128 _L1 = mat.getCol0().get128();
	__m128 _L2 = mat.getCol1().get128();
	__m128 _L3 = mat.getCol2().get128();
	__m128 _L4 = mat.getCol3().get128();
	// Calculating the minterms for the first line.

	// _mm_ror_ps is just a macro using _mm_shuffle_ps().
	tt = _L4; tt2 = _mm_ror_ps(_L3,1); 
	Vc = _mm_mul_ps(tt2,_mm_ror_ps(tt,0));					// V3' dot V4
	Va = _mm_mul_ps(tt2,_mm_ror_ps(tt,2));					// V3' dot V4"
	Vb = _mm_mul_ps(tt2,_mm_ror_ps(tt,3));					// V3' dot V4^

	r1 = _mm_sub_ps(_mm_ror_ps(Va,1),_mm_ror_ps(Vc,2));		// V3" dot V4^ - V3^ dot V4"
	r2 = _mm_sub_ps(_mm_ror_ps(Vb,2),_mm_ror_ps(Vb,0));		// V3^ dot V4' - V3' dot V4^
	r3 = _mm_sub_ps(_mm_ror_ps(Va,0),_mm_ror_ps(Vc,1));		// V3' dot V4" - V3" dot V4'

	tt = _L2;
	Va = _mm_ror_ps(tt,1);		sum = _mm_mul_ps(Va,r1);
	Vb = _mm_ror_ps(tt,2);		sum = _mm_add_ps(sum,_mm_mul_ps(Vb,r2));
	Vc = _mm_ror_ps(tt,3);		sum = _mm_add_ps(sum,_mm_mul_ps(Vc,r3));

	// Calculating the determinant.
	Det = _mm_mul_ps(sum,_L1);
	Det = _mm_add_ps(Det,_mm_movehl_ps(Det,Det));

	// Calculating the minterms of the second line (using previous results).
	tt = _mm_ror_ps(_L1,1);		sum = _mm_mul_ps(tt,r1);
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r2));
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r3));

	// Testing the determinant.
	Det = _mm_sub_ss(Det,_mm_shuffle_ps(Det,Det,1));
	return floatInVec(Det, 0);
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::operator +( const Matrix4 & mat ) const
{
    return Matrix4(
        ( mCol0 + mat.mCol0 ),
        ( mCol1 + mat.mCol1 ),
        ( mCol2 + mat.mCol2 ),
        ( mCol3 + mat.mCol3 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::operator -( const Matrix4 & mat ) const
{
    return Matrix4(
        ( mCol0 - mat.mCol0 ),
        ( mCol1 - mat.mCol1 ),
        ( mCol2 - mat.mCol2 ),
        ( mCol3 - mat.mCol3 )
    );
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::operator +=( const Matrix4 & mat )
{
    *this = *this + mat;
    return *this;
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::operator -=( const Matrix4 & mat )
{
    *this = *this - mat;
    return *this;
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::operator -( ) const
{
    return Matrix4(
        ( -mCol0 ),
        ( -mCol1 ),
        ( -mCol2 ),
        ( -mCol3 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 absPerElem( const Matrix4 & mat )
{
    return Matrix4(
        absPerElem( mat.getCol0() ),
        absPerElem( mat.getCol1() ),
        absPerElem( mat.getCol2() ),
        absPerElem( mat.getCol3() )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::operator *( float scalar ) const
{
    return *this * floatInVec(scalar);
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::operator *( const floatInVec &scalar ) const
{
    return Matrix4(
        ( mCol0 * scalar ),
        ( mCol1 * scalar ),
        ( mCol2 * scalar ),
        ( mCol3 * scalar )
    );
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::operator *=( float scalar )
{
    return *this *= floatInVec(scalar);
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::operator *=( const floatInVec &scalar )
{
    *this = *this * scalar;
    return *this;
}

VECTORMATH_FORCE_INLINE const Matrix4 operator *( float scalar, const Matrix4 & mat )
{
    return floatInVec(scalar) * mat;
}

VECTORMATH_FORCE_INLINE const Matrix4 operator *( const floatInVec &scalar, const Matrix4 & mat )
{
    return mat * scalar;
}

VECTORMATH_FORCE_INLINE const Vector4 Matrix4::operator *( const Vector4 &vec ) const
{
    return Vector4(
		_mm_add_ps(
			_mm_add_ps(_mm_mul_ps(mCol0.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(0,0,0,0))), _mm_mul_ps(mCol1.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(1,1,1,1)))),
			_mm_add_ps(_mm_mul_ps(mCol2.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(2,2,2,2))), _mm_mul_ps(mCol3.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(3,3,3,3)))))
		);
}

VECTORMATH_FORCE_INLINE const Vector4 Matrix4::operator *( const Vector3 &vec ) const
{
    return Vector4(
		_mm_add_ps(
			_mm_add_ps(_mm_mul_ps(mCol0.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(0,0,0,0))), _mm_mul_ps(mCol1.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(1,1,1,1)))),
			_mm_mul_ps(mCol2.get128(), _mm_shuffle_ps(vec.get128(), vec.get128(), _MM_SHUFFLE(2,2,2,2))))
		);
}

VECTORMATH_FORCE_INLINE const Vector4 Matrix4::operator *( const Point3 &pnt ) const
{
    return Vector4(
		_mm_add_ps(
			_mm_add_ps(_mm_mul_ps(mCol0.get128(), _mm_shuffle_ps(pnt.get128(), pnt.get128(), _MM_SHUFFLE(0,0,0,0))), _mm_mul_ps(mCol1.get128(), _mm_shuffle_ps(pnt.get128(), pnt.get128(), _MM_SHUFFLE(1,1,1,1)))),
			_mm_add_ps(_mm_mul_ps(mCol2.get128(), _mm_shuffle_ps(pnt.get128(), pnt.get128(), _MM_SHUFFLE(2,2,2,2))), mCol3.get128()))
		);
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::operator *( const Matrix4 & mat ) const
{
    return Matrix4(
        ( *this * mat.mCol0 ),
        ( *this * mat.mCol1 ),
        ( *this * mat.mCol2 ),
        ( *this * mat.mCol3 )
    );
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::operator *=( const Matrix4 & mat )
{
    *this = *this * mat;
    return *this;
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::operator *( const Transform3 & tfrm ) const
{
    return Matrix4(
        ( *this * tfrm.getCol0() ),
        ( *this * tfrm.getCol1() ),
        ( *this * tfrm.getCol2() ),
        ( *this * Point3( tfrm.getCol3() ) )
    );
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::operator *=( const Transform3 & tfrm )
{
    *this = *this * tfrm;
    return *this;
}

VECTORMATH_FORCE_INLINE const Matrix4 mulPerElem( const Matrix4 & mat0, const Matrix4 & mat1 )
{
    return Matrix4(
        mulPerElem( mat0.getCol0(), mat1.getCol0() ),
        mulPerElem( mat0.getCol1(), mat1.getCol1() ),
        mulPerElem( mat0.getCol2(), mat1.getCol2() ),
        mulPerElem( mat0.getCol3(), mat1.getCol3() )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::identity( )
{
    return Matrix4(
        Vector4::xAxis( ),
        Vector4::yAxis( ),
        Vector4::zAxis( ),
        Vector4::wAxis( )
    );
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::setUpper3x3( const Matrix3 & mat3 )
{
    mCol0.setXYZ( mat3.getCol0() );
    mCol1.setXYZ( mat3.getCol1() );
    mCol2.setXYZ( mat3.getCol2() );
    return *this;
}

VECTORMATH_FORCE_INLINE const Matrix3 Matrix4::getUpper3x3( ) const
{
    return Matrix3(
        mCol0.getXYZ( ),
        mCol1.getXYZ( ),
        mCol2.getXYZ( )
    );
}

VECTORMATH_FORCE_INLINE Matrix4 & Matrix4::setTranslation( const Vector3 &translateVec )
{
    mCol3.setXYZ( translateVec );
    return *this;
}

VECTORMATH_FORCE_INLINE const Vector3 Matrix4::getTranslation( ) const
{
    return mCol3.getXYZ( );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::rotationX( float radians )
{
    return rotationX( floatInVec(radians) );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::rotationX( const floatInVec &radians )
{
    __m128 s, c, res1, res2;
    __m128 zero;
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    zero = _mm_setzero_ps();
    sincosf4( radians.get128(), &s, &c );
    res1 = vec_sel( zero, c, select_y );
    res1 = vec_sel( res1, s, select_z );
    res2 = vec_sel( zero, negatef4(s), select_y );
    res2 = vec_sel( res2, c, select_z );
    return Matrix4(
        Vector4::xAxis( ),
        Vector4( res1 ),
        Vector4( res2 ),
        Vector4::wAxis( )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::rotationY( float radians )
{
    return rotationY( floatInVec(radians) );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::rotationY( const floatInVec &radians )
{
    __m128 s, c, res0, res2;
    __m128 zero;
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    zero = _mm_setzero_ps();
    sincosf4( radians.get128(), &s, &c );
    res0 = vec_sel( zero, c, select_x );
    res0 = vec_sel( res0, negatef4(s), select_z );
    res2 = vec_sel( zero, s, select_x );
    res2 = vec_sel( res2, c, select_z );
    return Matrix4(
        Vector4( res0 ),
        Vector4::yAxis( ),
        Vector4( res2 ),
        Vector4::wAxis( )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::rotationZ( float radians )
{
    return rotationZ( floatInVec(radians) );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::rotationZ( const floatInVec &radians )
{
    __m128 s, c, res0, res1;
    __m128 zero;
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
    zero = _mm_setzero_ps();
    sincosf4( radians.get128(), &s, &c );
    res0 = vec_sel( zero, c, select_x );
    res0 = vec_sel( res0, s, select_y );
    res1 = vec_sel( zero, negatef4(s), select_x );
    res1 = vec_sel( res1, c, select_y );
    return Matrix4(
        Vector4( res0 ),
        Vector4( res1 ),
        Vector4::zAxis( ),
        Vector4::wAxis( )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::rotationZYX( const Vector3 &radiansXYZ )
{
    __m128 angles, s, negS, c, X0, X1, Y0, Y1, Z0, Z1, tmp;
    angles = Vector4( radiansXYZ, 0.0f ).get128();
    sincosf4( angles, &s, &c );
    negS = negatef4( s );
    Z0 = vec_mergel( c, s );
    Z1 = vec_mergel( negS, c );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_xyz[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0};
    Z1 = vec_and( Z1, _mm_load_ps( (float *)select_xyz ) );
	Y0 = _mm_shuffle_ps( c, negS, _MM_SHUFFLE(0,1,1,1) );
	Y1 = _mm_shuffle_ps( s, c, _MM_SHUFFLE(0,1,1,1) );
    X0 = vec_splat( s, 0 );
    X1 = vec_splat( c, 0 );
    tmp = vec_mul( Z0, Y1 );
    return Matrix4(
        Vector4( vec_mul( Z0, Y0 ) ),
        Vector4( vec_madd( Z1, X1, vec_mul( tmp, X0 ) ) ),
        Vector4( vec_nmsub( Z1, X0, vec_mul( tmp, X1 ) ) ),
        Vector4::wAxis( )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::rotation( float radians, const Vector3 &unitVec )
{
    return rotation( floatInVec(radians), unitVec );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::rotation( const floatInVec &radians, const Vector3 &unitVec )
{
    __m128 axis, s, c, oneMinusC, axisS, negAxisS, xxxx, yyyy, zzzz, tmp0, tmp1, tmp2;
    axis = unitVec.get128();
    sincosf4( radians.get128(), &s, &c );
    xxxx = vec_splat( axis, 0 );
    yyyy = vec_splat( axis, 1 );
    zzzz = vec_splat( axis, 2 );
    oneMinusC = vec_sub( _mm_set1_ps(1.0f), c );
    axisS = vec_mul( axis, s );
    negAxisS = negatef4( axisS );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    //tmp0 = vec_perm( axisS, negAxisS, _VECTORMATH_PERM_XZBX );
	tmp0 = _mm_shuffle_ps( axisS, axisS, _MM_SHUFFLE(0,0,2,0) );
	tmp0 = vec_sel(tmp0, vec_splat(negAxisS, 1), select_z);
    //tmp1 = vec_perm( axisS, negAxisS, _VECTORMATH_PERM_CXXX );
	tmp1 = vec_sel( vec_splat(axisS, 0), vec_splat(negAxisS, 2), select_x );
    //tmp2 = vec_perm( axisS, negAxisS, _VECTORMATH_PERM_YAXX );
	tmp2 = _mm_shuffle_ps( axisS, axisS, _MM_SHUFFLE(0,0,0,1) );
	tmp2 = vec_sel(tmp2, vec_splat(negAxisS, 0), select_y);
    tmp0 = vec_sel( tmp0, c, select_x );
    tmp1 = vec_sel( tmp1, c, select_y );
    tmp2 = vec_sel( tmp2, c, select_z );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_xyz[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0};
    axis = vec_and( axis, _mm_load_ps( (float *)select_xyz ) );
    tmp0 = vec_and( tmp0, _mm_load_ps( (float *)select_xyz ) );
    tmp1 = vec_and( tmp1, _mm_load_ps( (float *)select_xyz ) );
    tmp2 = vec_and( tmp2, _mm_load_ps( (float *)select_xyz ) );
    return Matrix4(
        Vector4( vec_madd( vec_mul( axis, xxxx ), oneMinusC, tmp0 ) ),
        Vector4( vec_madd( vec_mul( axis, yyyy ), oneMinusC, tmp1 ) ),
        Vector4( vec_madd( vec_mul( axis, zzzz ), oneMinusC, tmp2 ) ),
        Vector4::wAxis( )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::rotation( const Quat &unitQuat )
{
    return Matrix4( Transform3::rotation( unitQuat ) );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::scale( const Vector3 &scaleVec )
{
    __m128 zero = _mm_setzero_ps();
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    return Matrix4(
        Vector4( vec_sel( zero, scaleVec.get128(), select_x ) ),
        Vector4( vec_sel( zero, scaleVec.get128(), select_y ) ),
        Vector4( vec_sel( zero, scaleVec.get128(), select_z ) ),
        Vector4::wAxis( )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 appendScale( const Matrix4 & mat, const Vector3 &scaleVec )
{
    return Matrix4(
        ( mat.getCol0() * scaleVec.getX( ) ),
        ( mat.getCol1() * scaleVec.getY( ) ),
        ( mat.getCol2() * scaleVec.getZ( ) ),
        mat.getCol3()
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 prependScale( const Vector3 &scaleVec, const Matrix4 & mat )
{
    Vector4 scale4;
    scale4 = Vector4( scaleVec, 1.0f );
    return Matrix4(
        mulPerElem( mat.getCol0(), scale4 ),
        mulPerElem( mat.getCol1(), scale4 ),
        mulPerElem( mat.getCol2(), scale4 ),
        mulPerElem( mat.getCol3(), scale4 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::translation( const Vector3 &translateVec )
{
    return Matrix4(
        Vector4::xAxis( ),
        Vector4::yAxis( ),
        Vector4::zAxis( ),
        Vector4( translateVec, 1.0f )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::lookAt( const Point3 &eyePos, const Point3 &lookAtPos, const Vector3 &upVec )
{
    Matrix4 m4EyeFrame;
    Vector3 v3X, v3Y, v3Z;
    v3Y = normalize( upVec );
    v3Z = normalize( ( eyePos - lookAtPos ) );
    v3X = normalize( cross( v3Y, v3Z ) );
    v3Y = cross( v3Z, v3X );
    m4EyeFrame = Matrix4( Vector4( v3X ), Vector4( v3Y ), Vector4( v3Z ), Vector4( eyePos ) );
    return orthoInverse( m4EyeFrame );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::perspective( float fovyRadians, float aspect, float zNear, float zFar )
{
    float f, rangeInv;
    __m128 zero, col0, col1, col2, col3;
    union { __m128 v; float s[4]; } tmp;
    f = tanf( _VECTORMATH_PI_OVER_2 - fovyRadians * 0.5f );
    rangeInv = 1.0f / ( zNear - zFar );
    zero = _mm_setzero_ps();
    tmp.v = zero;
    tmp.s[0] = f / aspect;
    col0 = tmp.v;
    tmp.v = zero;
    tmp.s[1] = f;
    col1 = tmp.v;
    tmp.v = zero;
    tmp.s[2] = ( zNear + zFar ) * rangeInv;
    tmp.s[3] = -1.0f;
    col2 = tmp.v;
    tmp.v = zero;
    tmp.s[2] = zNear * zFar * rangeInv * 2.0f;
    col3 = tmp.v;
    return Matrix4(
        Vector4( col0 ),
        Vector4( col1 ),
        Vector4( col2 ),
        Vector4( col3 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::frustum( float left, float right, float bottom, float top, float zNear, float zFar )
{
    /* function implementation based on code from STIDC SDK:           */
    /* --------------------------------------------------------------  */
    /* PLEASE DO NOT MODIFY THIS SECTION                               */
    /* This prolog section is automatically generated.                 */
    /*                                                                 */
    /* (C)Copyright                                                    */
    /* Sony Computer Entertainment, Inc.,                              */
    /* Toshiba Corporation,                                            */
    /* International Business Machines Corporation,                    */
    /* 2001,2002.                                                      */
    /* S/T/I Confidential Information                                  */
    /* --------------------------------------------------------------  */
    __m128 lbf, rtn;
    __m128 diff, sum, inv_diff;
    __m128 diagonal, column, near2;
    __m128 zero = _mm_setzero_ps();
    union { __m128 v; float s[4]; } l, f, r, n, b, t; // TODO: Union?
    l.s[0] = left;
    f.s[0] = zFar;
    r.s[0] = right;
    n.s[0] = zNear;
    b.s[0] = bottom;
    t.s[0] = top;
    lbf = vec_mergeh( l.v, f.v );
    rtn = vec_mergeh( r.v, n.v );
    lbf = vec_mergeh( lbf, b.v );
    rtn = vec_mergeh( rtn, t.v );
    diff = vec_sub( rtn, lbf );
    sum  = vec_add( rtn, lbf );
    inv_diff = recipf4( diff );
    near2 = vec_splat( n.v, 0 );
    near2 = vec_add( near2, near2 );
    diagonal = vec_mul( near2, inv_diff );
    column = vec_mul( sum, inv_diff );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_w[4] = {0, 0, 0, 0xffffffff};
    return Matrix4(
        Vector4( vec_sel( zero, diagonal, select_x ) ),
        Vector4( vec_sel( zero, diagonal, select_y ) ),
        Vector4( vec_sel( column, _mm_set1_ps(-1.0f), select_w ) ),
        Vector4( vec_sel( zero, vec_mul( diagonal, vec_splat( f.v, 0 ) ), select_z ) )
	);
}

VECTORMATH_FORCE_INLINE const Matrix4 Matrix4::orthographic( float left, float right, float bottom, float top, float zNear, float zFar )
{
    /* function implementation based on code from STIDC SDK:           */
    /* --------------------------------------------------------------  */
    /* PLEASE DO NOT MODIFY THIS SECTION                               */
    /* This prolog section is automatically generated.                 */
    /*                                                                 */
    /* (C)Copyright                                                    */
    /* Sony Computer Entertainment, Inc.,                              */
    /* Toshiba Corporation,                                            */
    /* International Business Machines Corporation,                    */
    /* 2001,2002.                                                      */
    /* S/T/I Confidential Information                                  */
    /* --------------------------------------------------------------  */
    __m128 lbf, rtn;
    __m128 diff, sum, inv_diff, neg_inv_diff;
    __m128 diagonal, column;
    __m128 zero = _mm_setzero_ps();
    union { __m128 v; float s[4]; } l, f, r, n, b, t;
    l.s[0] = left;
    f.s[0] = zFar;
    r.s[0] = right;
    n.s[0] = zNear;
    b.s[0] = bottom;
    t.s[0] = top;
    lbf = vec_mergeh( l.v, f.v );
    rtn = vec_mergeh( r.v, n.v );
    lbf = vec_mergeh( lbf, b.v );
    rtn = vec_mergeh( rtn, t.v );
    diff = vec_sub( rtn, lbf );
    sum  = vec_add( rtn, lbf );
    inv_diff = recipf4( diff );
    neg_inv_diff = negatef4( inv_diff );
    diagonal = vec_add( inv_diff, inv_diff );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_w[4] = {0, 0, 0, 0xffffffff};
    column = vec_mul( sum, vec_sel( neg_inv_diff, inv_diff, select_z ) ); // TODO: no madds with zero
    return Matrix4(
        Vector4( vec_sel( zero, diagonal, select_x ) ),
        Vector4( vec_sel( zero, diagonal, select_y ) ),
        Vector4( vec_sel( zero, diagonal, select_z ) ),
        Vector4( vec_sel( column, _mm_set1_ps(1.0f), select_w ) )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 select( const Matrix4 & mat0, const Matrix4 & mat1, bool select1 )
{
    return Matrix4(
        select( mat0.getCol0(), mat1.getCol0(), select1 ),
        select( mat0.getCol1(), mat1.getCol1(), select1 ),
        select( mat0.getCol2(), mat1.getCol2(), select1 ),
        select( mat0.getCol3(), mat1.getCol3(), select1 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 select( const Matrix4 & mat0, const Matrix4 & mat1, const boolInVec &select1 )
{
    return Matrix4(
        select( mat0.getCol0(), mat1.getCol0(), select1 ),
        select( mat0.getCol1(), mat1.getCol1(), select1 ),
        select( mat0.getCol2(), mat1.getCol2(), select1 ),
        select( mat0.getCol3(), mat1.getCol3(), select1 )
    );
}

#ifdef _VECTORMATH_DEBUG

VECTORMATH_FORCE_INLINE void print( const Matrix4 & mat )
{
    print( mat.getRow( 0 ) );
    print( mat.getRow( 1 ) );
    print( mat.getRow( 2 ) );
    print( mat.getRow( 3 ) );
}

VECTORMATH_FORCE_INLINE void print( const Matrix4 & mat, const char * name )
{
    printf("%s:\n", name);
    print( mat );
}

#endif

VECTORMATH_FORCE_INLINE Transform3::Transform3( const Transform3 & tfrm )
{
    mCol0 = tfrm.mCol0;
    mCol1 = tfrm.mCol1;
    mCol2 = tfrm.mCol2;
    mCol3 = tfrm.mCol3;
}

VECTORMATH_FORCE_INLINE Transform3::Transform3( float scalar )
{
    mCol0 = Vector3( scalar );
    mCol1 = Vector3( scalar );
    mCol2 = Vector3( scalar );
    mCol3 = Vector3( scalar );
}

VECTORMATH_FORCE_INLINE Transform3::Transform3( const floatInVec &scalar )
{
    mCol0 = Vector3( scalar );
    mCol1 = Vector3( scalar );
    mCol2 = Vector3( scalar );
    mCol3 = Vector3( scalar );
}

VECTORMATH_FORCE_INLINE Transform3::Transform3( const Vector3 &_col0, const Vector3 &_col1, const Vector3 &_col2, const Vector3 &_col3 )
{
    mCol0 = _col0;
    mCol1 = _col1;
    mCol2 = _col2;
    mCol3 = _col3;
}

VECTORMATH_FORCE_INLINE Transform3::Transform3( const Matrix3 & tfrm, const Vector3 &translateVec )
{
    this->setUpper3x3( tfrm );
    this->setTranslation( translateVec );
}

VECTORMATH_FORCE_INLINE Transform3::Transform3( const Quat &unitQuat, const Vector3 &translateVec )
{
    this->setUpper3x3( Matrix3( unitQuat ) );
    this->setTranslation( translateVec );
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::setCol0( const Vector3 &_col0 )
{
    mCol0 = _col0;
    return *this;
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::setCol1( const Vector3 &_col1 )
{
    mCol1 = _col1;
    return *this;
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::setCol2( const Vector3 &_col2 )
{
    mCol2 = _col2;
    return *this;
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::setCol3( const Vector3 &_col3 )
{
    mCol3 = _col3;
    return *this;
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::setCol( int col, const Vector3 &vec )
{
    *(&mCol0 + col) = vec;
    return *this;
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::setRow( int row, const Vector4 &vec )
{
    mCol0.setElem( row, vec.getElem( 0 ) );
    mCol1.setElem( row, vec.getElem( 1 ) );
    mCol2.setElem( row, vec.getElem( 2 ) );
    mCol3.setElem( row, vec.getElem( 3 ) );
    return *this;
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::setElem( int col, int row, float val )
{
    (*this)[col].setElem(row, val);
    return *this;
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::setElem( int col, int row, const floatInVec &val )
{
    Vector3 tmpV3_0;
    tmpV3_0 = this->getCol( col );
    tmpV3_0.setElem( row, val );
    this->setCol( col, tmpV3_0 );
    return *this;
}

VECTORMATH_FORCE_INLINE const floatInVec Transform3::getElem( int col, int row ) const
{
    return this->getCol( col ).getElem( row );
}

VECTORMATH_FORCE_INLINE const Vector3 Transform3::getCol0( ) const
{
    return mCol0;
}

VECTORMATH_FORCE_INLINE const Vector3 Transform3::getCol1( ) const
{
    return mCol1;
}

VECTORMATH_FORCE_INLINE const Vector3 Transform3::getCol2( ) const
{
    return mCol2;
}

VECTORMATH_FORCE_INLINE const Vector3 Transform3::getCol3( ) const
{
    return mCol3;
}

VECTORMATH_FORCE_INLINE const Vector3 Transform3::getCol( int col ) const
{
    return *(&mCol0 + col);
}

VECTORMATH_FORCE_INLINE const Vector4 Transform3::getRow( int row ) const
{
    return Vector4( mCol0.getElem( row ), mCol1.getElem( row ), mCol2.getElem( row ), mCol3.getElem( row ) );
}

VECTORMATH_FORCE_INLINE Vector3 & Transform3::operator []( int col )
{
    return *(&mCol0 + col);
}

VECTORMATH_FORCE_INLINE const Vector3 Transform3::operator []( int col ) const
{
    return *(&mCol0 + col);
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::operator =( const Transform3 & tfrm )
{
    mCol0 = tfrm.mCol0;
    mCol1 = tfrm.mCol1;
    mCol2 = tfrm.mCol2;
    mCol3 = tfrm.mCol3;
    return *this;
}

VECTORMATH_FORCE_INLINE const Transform3 inverse( const Transform3 & tfrm )
{
    __m128 inv0, inv1, inv2, inv3;
    __m128 tmp0, tmp1, tmp2, tmp3, tmp4, dot, invdet;
    __m128 xxxx, yyyy, zzzz;
    tmp2 = _vmathVfCross( tfrm.getCol0().get128(), tfrm.getCol1().get128() );
    tmp0 = _vmathVfCross( tfrm.getCol1().get128(), tfrm.getCol2().get128() );
    tmp1 = _vmathVfCross( tfrm.getCol2().get128(), tfrm.getCol0().get128() );
    inv3 = negatef4( tfrm.getCol3().get128() );
    dot = _vmathVfDot3( tmp2, tfrm.getCol2().get128() );
    dot = vec_splat( dot, 0 );
    invdet = recipf4( dot );
    tmp3 = vec_mergeh( tmp0, tmp2 );
    tmp4 = vec_mergel( tmp0, tmp2 );
    inv0 = vec_mergeh( tmp3, tmp1 );
    xxxx = vec_splat( inv3, 0 );
    //inv1 = vec_perm( tmp3, tmp1, _VECTORMATH_PERM_ZBWX );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	inv1 = _mm_shuffle_ps( tmp3, tmp3, _MM_SHUFFLE(0,3,2,2));
	inv1 = vec_sel(inv1, tmp1, select_y);
    //inv2 = vec_perm( tmp4, tmp1, _VECTORMATH_PERM_XCYX );
	inv2 = _mm_shuffle_ps( tmp4, tmp4, _MM_SHUFFLE(0,1,1,0));
	inv2 = vec_sel(inv2, vec_splat(tmp1, 2), select_y);
    yyyy = vec_splat( inv3, 1 );
    zzzz = vec_splat( inv3, 2 );
    inv3 = vec_mul( inv0, xxxx );
    inv3 = vec_madd( inv1, yyyy, inv3 );
    inv3 = vec_madd( inv2, zzzz, inv3 );
    inv0 = vec_mul( inv0, invdet );
    inv1 = vec_mul( inv1, invdet );
    inv2 = vec_mul( inv2, invdet );
    inv3 = vec_mul( inv3, invdet );
    return Transform3(
        Vector3( inv0 ),
        Vector3( inv1 ),
        Vector3( inv2 ),
        Vector3( inv3 )
    );
}

VECTORMATH_FORCE_INLINE const Transform3 orthoInverse( const Transform3 & tfrm )
{
    __m128 inv0, inv1, inv2, inv3;
    __m128 tmp0, tmp1;
    __m128 xxxx, yyyy, zzzz;
    tmp0 = vec_mergeh( tfrm.getCol0().get128(), tfrm.getCol2().get128() );
    tmp1 = vec_mergel( tfrm.getCol0().get128(), tfrm.getCol2().get128() );
    inv3 = negatef4( tfrm.getCol3().get128() );
    inv0 = vec_mergeh( tmp0, tfrm.getCol1().get128() );
    xxxx = vec_splat( inv3, 0 );
    //inv1 = vec_perm( tmp0, tfrm.getCol1().get128(), _VECTORMATH_PERM_ZBWX );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	inv1 = _mm_shuffle_ps( tmp0, tmp0, _MM_SHUFFLE(0,3,2,2));
	inv1 = vec_sel(inv1, tfrm.getCol1().get128(), select_y);
    //inv2 = vec_perm( tmp1, tfrm.getCol1().get128(), _VECTORMATH_PERM_XCYX );
	inv2 = _mm_shuffle_ps( tmp1, tmp1, _MM_SHUFFLE(0,1,1,0));
	inv2 = vec_sel(inv2, vec_splat(tfrm.getCol1().get128(), 2), select_y);
    yyyy = vec_splat( inv3, 1 );
    zzzz = vec_splat( inv3, 2 );
    inv3 = vec_mul( inv0, xxxx );
    inv3 = vec_madd( inv1, yyyy, inv3 );
    inv3 = vec_madd( inv2, zzzz, inv3 );
    return Transform3(
        Vector3( inv0 ),
        Vector3( inv1 ),
        Vector3( inv2 ),
        Vector3( inv3 )
    );
}

VECTORMATH_FORCE_INLINE const Transform3 absPerElem( const Transform3 & tfrm )
{
    return Transform3(
        absPerElem( tfrm.getCol0() ),
        absPerElem( tfrm.getCol1() ),
        absPerElem( tfrm.getCol2() ),
        absPerElem( tfrm.getCol3() )
    );
}

VECTORMATH_FORCE_INLINE const Vector3 Transform3::operator *( const Vector3 &vec ) const
{
    __m128 res;
    __m128 xxxx, yyyy, zzzz;
    xxxx = vec_splat( vec.get128(), 0 );
    yyyy = vec_splat( vec.get128(), 1 );
    zzzz = vec_splat( vec.get128(), 2 );
    res = vec_mul( mCol0.get128(), xxxx );
    res = vec_madd( mCol1.get128(), yyyy, res );
    res = vec_madd( mCol2.get128(), zzzz, res );
    return Vector3( res );
}

VECTORMATH_FORCE_INLINE const Point3 Transform3::operator *( const Point3 &pnt ) const
{
    __m128 tmp0, tmp1, res;
    __m128 xxxx, yyyy, zzzz;
    xxxx = vec_splat( pnt.get128(), 0 );
    yyyy = vec_splat( pnt.get128(), 1 );
    zzzz = vec_splat( pnt.get128(), 2 );
    tmp0 = vec_mul( mCol0.get128(), xxxx );
    tmp1 = vec_mul( mCol1.get128(), yyyy );
    tmp0 = vec_madd( mCol2.get128(), zzzz, tmp0 );
    tmp1 = vec_add( mCol3.get128(), tmp1 );
    res = vec_add( tmp0, tmp1 );
    return Point3( res );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::operator *( const Transform3 & tfrm ) const
{
    return Transform3(
        ( *this * tfrm.mCol0 ),
        ( *this * tfrm.mCol1 ),
        ( *this * tfrm.mCol2 ),
        Vector3( ( *this * Point3( tfrm.mCol3 ) ) )
    );
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::operator *=( const Transform3 & tfrm )
{
    *this = *this * tfrm;
    return *this;
}

VECTORMATH_FORCE_INLINE const Transform3 mulPerElem( const Transform3 & tfrm0, const Transform3 & tfrm1 )
{
    return Transform3(
        mulPerElem( tfrm0.getCol0(), tfrm1.getCol0() ),
        mulPerElem( tfrm0.getCol1(), tfrm1.getCol1() ),
        mulPerElem( tfrm0.getCol2(), tfrm1.getCol2() ),
        mulPerElem( tfrm0.getCol3(), tfrm1.getCol3() )
    );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::identity( )
{
    return Transform3(
        Vector3::xAxis( ),
        Vector3::yAxis( ),
        Vector3::zAxis( ),
        Vector3( 0.0f )
    );
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::setUpper3x3( const Matrix3 & tfrm )
{
    mCol0 = tfrm.getCol0();
    mCol1 = tfrm.getCol1();
    mCol2 = tfrm.getCol2();
    return *this;
}

VECTORMATH_FORCE_INLINE const Matrix3 Transform3::getUpper3x3( ) const
{
    return Matrix3( mCol0, mCol1, mCol2 );
}

VECTORMATH_FORCE_INLINE Transform3 & Transform3::setTranslation( const Vector3 &translateVec )
{
    mCol3 = translateVec;
    return *this;
}

VECTORMATH_FORCE_INLINE const Vector3 Transform3::getTranslation( ) const
{
    return mCol3;
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::rotationX( float radians )
{
    return rotationX( floatInVec(radians) );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::rotationX( const floatInVec &radians )
{
    __m128 s, c, res1, res2;
    __m128 zero;
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    zero = _mm_setzero_ps();
    sincosf4( radians.get128(), &s, &c );
    res1 = vec_sel( zero, c, select_y );
    res1 = vec_sel( res1, s, select_z );
    res2 = vec_sel( zero, negatef4(s), select_y );
    res2 = vec_sel( res2, c, select_z );
    return Transform3(
        Vector3::xAxis( ),
        Vector3( res1 ),
        Vector3( res2 ),
        Vector3( _mm_setzero_ps() )
    );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::rotationY( float radians )
{
    return rotationY( floatInVec(radians) );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::rotationY( const floatInVec &radians )
{
    __m128 s, c, res0, res2;
    __m128 zero;
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    zero = _mm_setzero_ps();
    sincosf4( radians.get128(), &s, &c );
    res0 = vec_sel( zero, c, select_x );
    res0 = vec_sel( res0, negatef4(s), select_z );
    res2 = vec_sel( zero, s, select_x );
    res2 = vec_sel( res2, c, select_z );
    return Transform3(
        Vector3( res0 ),
        Vector3::yAxis( ),
        Vector3( res2 ),
        Vector3( 0.0f )
    );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::rotationZ( float radians )
{
    return rotationZ( floatInVec(radians) );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::rotationZ( const floatInVec &radians )
{
    __m128 s, c, res0, res1;
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
    __m128 zero = _mm_setzero_ps();
    sincosf4( radians.get128(), &s, &c );
    res0 = vec_sel( zero, c, select_x );
    res0 = vec_sel( res0, s, select_y );
    res1 = vec_sel( zero, negatef4(s), select_x );
    res1 = vec_sel( res1, c, select_y );
    return Transform3(
        Vector3( res0 ),
        Vector3( res1 ),
        Vector3::zAxis( ),
        Vector3( 0.0f )
    );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::rotationZYX( const Vector3 &radiansXYZ )
{
    __m128 angles, s, negS, c, X0, X1, Y0, Y1, Z0, Z1, tmp;
    angles = Vector4( radiansXYZ, 0.0f ).get128();
    sincosf4( angles, &s, &c );
    negS = negatef4( s );
    Z0 = vec_mergel( c, s );
    Z1 = vec_mergel( negS, c );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_xyz[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0};
    Z1 = vec_and( Z1, _mm_load_ps( (float *)select_xyz ) );
	Y0 = _mm_shuffle_ps( c, negS, _MM_SHUFFLE(0,1,1,1) );
	Y1 = _mm_shuffle_ps( s, c, _MM_SHUFFLE(0,1,1,1) );
    X0 = vec_splat( s, 0 );
    X1 = vec_splat( c, 0 );
    tmp = vec_mul( Z0, Y1 );
    return Transform3(
        Vector3( vec_mul( Z0, Y0 ) ),
        Vector3( vec_madd( Z1, X1, vec_mul( tmp, X0 ) ) ),
        Vector3( vec_nmsub( Z1, X0, vec_mul( tmp, X1 ) ) ),
        Vector3( 0.0f )
    );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::rotation( float radians, const Vector3 &unitVec )
{
    return rotation( floatInVec(radians), unitVec );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::rotation( const floatInVec &radians, const Vector3 &unitVec )
{
    return Transform3( Matrix3::rotation( radians, unitVec ), Vector3( 0.0f ) );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::rotation( const Quat &unitQuat )
{
    return Transform3( Matrix3( unitQuat ), Vector3( 0.0f ) );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::scale( const Vector3 &scaleVec )
{
    __m128 zero = _mm_setzero_ps();
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    return Transform3(
        Vector3( vec_sel( zero, scaleVec.get128(), select_x ) ),
        Vector3( vec_sel( zero, scaleVec.get128(), select_y ) ),
        Vector3( vec_sel( zero, scaleVec.get128(), select_z ) ),
        Vector3( 0.0f )
    );
}

VECTORMATH_FORCE_INLINE const Transform3 appendScale( const Transform3 & tfrm, const Vector3 &scaleVec )
{
    return Transform3(
        ( tfrm.getCol0() * scaleVec.getX( ) ),
        ( tfrm.getCol1() * scaleVec.getY( ) ),
        ( tfrm.getCol2() * scaleVec.getZ( ) ),
        tfrm.getCol3()
    );
}

VECTORMATH_FORCE_INLINE const Transform3 prependScale( const Vector3 &scaleVec, const Transform3 & tfrm )
{
    return Transform3(
        mulPerElem( tfrm.getCol0(), scaleVec ),
        mulPerElem( tfrm.getCol1(), scaleVec ),
        mulPerElem( tfrm.getCol2(), scaleVec ),
        mulPerElem( tfrm.getCol3(), scaleVec )
    );
}

VECTORMATH_FORCE_INLINE const Transform3 Transform3::translation( const Vector3 &translateVec )
{
    return Transform3(
        Vector3::xAxis( ),
        Vector3::yAxis( ),
        Vector3::zAxis( ),
        translateVec
    );
}

VECTORMATH_FORCE_INLINE const Transform3 select( const Transform3 & tfrm0, const Transform3 & tfrm1, bool select1 )
{
    return Transform3(
        select( tfrm0.getCol0(), tfrm1.getCol0(), select1 ),
        select( tfrm0.getCol1(), tfrm1.getCol1(), select1 ),
        select( tfrm0.getCol2(), tfrm1.getCol2(), select1 ),
        select( tfrm0.getCol3(), tfrm1.getCol3(), select1 )
    );
}

VECTORMATH_FORCE_INLINE const Transform3 select( const Transform3 & tfrm0, const Transform3 & tfrm1, const boolInVec &select1 )
{
    return Transform3(
        select( tfrm0.getCol0(), tfrm1.getCol0(), select1 ),
        select( tfrm0.getCol1(), tfrm1.getCol1(), select1 ),
        select( tfrm0.getCol2(), tfrm1.getCol2(), select1 ),
        select( tfrm0.getCol3(), tfrm1.getCol3(), select1 )
    );
}

#ifdef _VECTORMATH_DEBUG

VECTORMATH_FORCE_INLINE void print( const Transform3 & tfrm )
{
    print( tfrm.getRow( 0 ) );
    print( tfrm.getRow( 1 ) );
    print( tfrm.getRow( 2 ) );
}

VECTORMATH_FORCE_INLINE void print( const Transform3 & tfrm, const char * name )
{
    printf("%s:\n", name);
    print( tfrm );
}

#endif

VECTORMATH_FORCE_INLINE Quat::Quat( const Matrix3 & tfrm )
{
    __m128 res;
    __m128 col0, col1, col2;
    __m128 xx_yy, xx_yy_zz_xx, yy_zz_xx_yy, zz_xx_yy_zz, diagSum, diagDiff;
    __m128 zy_xz_yx, yz_zx_xy, sum, diff;
    __m128 radicand, invSqrt, scale;
    __m128 res0, res1, res2, res3;
    __m128 xx, yy, zz;
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_w[4] = {0, 0, 0, 0xffffffff};

    col0 = tfrm.getCol0().get128();
    col1 = tfrm.getCol1().get128();
    col2 = tfrm.getCol2().get128();

    /* four cases: */
    /* trace > 0 */
    /* else */
    /*    xx largest diagonal element */
    /*    yy largest diagonal element */
    /*    zz largest diagonal element */

    /* compute quaternion for each case */

    xx_yy = vec_sel( col0, col1, select_y );
    //xx_yy_zz_xx = vec_perm( xx_yy, col2, _VECTORMATH_PERM_XYCX );
    //yy_zz_xx_yy = vec_perm( xx_yy, col2, _VECTORMATH_PERM_YCXY );
    //zz_xx_yy_zz = vec_perm( xx_yy, col2, _VECTORMATH_PERM_CXYC );
    xx_yy_zz_xx = _mm_shuffle_ps( xx_yy, xx_yy, _MM_SHUFFLE(0,0,1,0) );
    xx_yy_zz_xx = vec_sel( xx_yy_zz_xx, col2, select_z ); // TODO: Ck
    yy_zz_xx_yy = _mm_shuffle_ps( xx_yy_zz_xx, xx_yy_zz_xx, _MM_SHUFFLE(1,0,2,1) );
    zz_xx_yy_zz = _mm_shuffle_ps( xx_yy_zz_xx, xx_yy_zz_xx, _MM_SHUFFLE(2,1,0,2) );

    diagSum = vec_add( vec_add( xx_yy_zz_xx, yy_zz_xx_yy ), zz_xx_yy_zz );
    diagDiff = vec_sub( vec_sub( xx_yy_zz_xx, yy_zz_xx_yy ), zz_xx_yy_zz );
    radicand = vec_add( vec_sel( diagDiff, diagSum, select_w ), _mm_set1_ps(1.0f) );
 //   invSqrt = rsqrtf4( radicand );
	invSqrt = newtonrapson_rsqrt4( radicand );

	

    zy_xz_yx = vec_sel( col0, col1, select_z );									// zy_xz_yx = 00 01 12 03
    //zy_xz_yx = vec_perm( zy_xz_yx, col2, _VECTORMATH_PERM_ZAYX );
	zy_xz_yx = _mm_shuffle_ps( zy_xz_yx, zy_xz_yx, _MM_SHUFFLE(0,1,2,2) );		// zy_xz_yx = 12 12 01 00
    zy_xz_yx = vec_sel( zy_xz_yx, vec_splat(col2, 0), select_y );				// zy_xz_yx = 12 20 01 00
    yz_zx_xy = vec_sel( col0, col1, select_x );									// yz_zx_xy = 10 01 02 03
    //yz_zx_xy = vec_perm( yz_zx_xy, col2, _VECTORMATH_PERM_BZXX );
	yz_zx_xy = _mm_shuffle_ps( yz_zx_xy, yz_zx_xy, _MM_SHUFFLE(0,0,2,0) );		// yz_zx_xy = 10 02 10 10
	yz_zx_xy = vec_sel( yz_zx_xy, vec_splat(col2, 1), select_x );				// yz_zx_xy = 21 02 10 10

    sum = vec_add( zy_xz_yx, yz_zx_xy );
    diff = vec_sub( zy_xz_yx, yz_zx_xy );

    scale = vec_mul( invSqrt, _mm_set1_ps(0.5f) );

    //res0 = vec_perm( sum, diff, _VECTORMATH_PERM_XZYA );
	res0 = _mm_shuffle_ps( sum, sum, _MM_SHUFFLE(0,1,2,0) );
	res0 = vec_sel( res0, vec_splat(diff, 0), select_w );  // TODO: Ck
    //res1 = vec_perm( sum, diff, _VECTORMATH_PERM_ZXXB );
	res1 = _mm_shuffle_ps( sum, sum, _MM_SHUFFLE(0,0,0,2) );
	res1 = vec_sel( res1, vec_splat(diff, 1), select_w );  // TODO: Ck
    //res2 = vec_perm( sum, diff, _VECTORMATH_PERM_YXXC );
	res2 = _mm_shuffle_ps( sum, sum, _MM_SHUFFLE(0,0,0,1) );
	res2 = vec_sel( res2, vec_splat(diff, 2), select_w );  // TODO: Ck
    res3 = diff;
    res0 = vec_sel( res0, radicand, select_x );
    res1 = vec_sel( res1, radicand, select_y );
    res2 = vec_sel( res2, radicand, select_z );
    res3 = vec_sel( res3, radicand, select_w );
    res0 = vec_mul( res0, vec_splat( scale, 0 ) );
    res1 = vec_mul( res1, vec_splat( scale, 1 ) );
    res2 = vec_mul( res2, vec_splat( scale, 2 ) );
    res3 = vec_mul( res3, vec_splat( scale, 3 ) );

    /* determine case and select answer */

    xx = vec_splat( col0, 0 );
    yy = vec_splat( col1, 1 );
    zz = vec_splat( col2, 2 );
    res = vec_sel( res0, res1, vec_cmpgt( yy, xx ) );
    res = vec_sel( res, res2, vec_and( vec_cmpgt( zz, xx ), vec_cmpgt( zz, yy ) ) );
    res = vec_sel( res, res3, vec_cmpgt( vec_splat( diagSum, 0 ), _mm_setzero_ps() ) );
    mVec128 = res;
}

VECTORMATH_FORCE_INLINE const Matrix3 outer( const Vector3 &tfrm0, const Vector3 &tfrm1 )
{
    return Matrix3(
        ( tfrm0 * tfrm1.getX( ) ),
        ( tfrm0 * tfrm1.getY( ) ),
        ( tfrm0 * tfrm1.getZ( ) )
    );
}

VECTORMATH_FORCE_INLINE const Matrix4 outer( const Vector4 &tfrm0, const Vector4 &tfrm1 )
{
    return Matrix4(
        ( tfrm0 * tfrm1.getX( ) ),
        ( tfrm0 * tfrm1.getY( ) ),
        ( tfrm0 * tfrm1.getZ( ) ),
        ( tfrm0 * tfrm1.getW( ) )
    );
}

VECTORMATH_FORCE_INLINE const Vector3 rowMul( const Vector3 &vec, const Matrix3 & mat )
{
    __m128 tmp0, tmp1, mcol0, mcol1, mcol2, res;
    __m128 xxxx, yyyy, zzzz;
    tmp0 = vec_mergeh( mat.getCol0().get128(), mat.getCol2().get128() );
    tmp1 = vec_mergel( mat.getCol0().get128(), mat.getCol2().get128() );
    xxxx = vec_splat( vec.get128(), 0 );
    mcol0 = vec_mergeh( tmp0, mat.getCol1().get128() );
    //mcol1 = vec_perm( tmp0, mat.getCol1().get128(), _VECTORMATH_PERM_ZBWX );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	mcol1 = _mm_shuffle_ps( tmp0, tmp0, _MM_SHUFFLE(0,3,2,2));
	mcol1 = vec_sel(mcol1, mat.getCol1().get128(), select_y);
    //mcol2 = vec_perm( tmp1, mat.getCol1().get128(), _VECTORMATH_PERM_XCYX );
	mcol2 = _mm_shuffle_ps( tmp1, tmp1, _MM_SHUFFLE(0,1,1,0));
	mcol2 = vec_sel(mcol2, vec_splat(mat.getCol1().get128(), 2), select_y);
    yyyy = vec_splat( vec.get128(), 1 );
    res = vec_mul( mcol0, xxxx );
    zzzz = vec_splat( vec.get128(), 2 );
    res = vec_madd( mcol1, yyyy, res );
    res = vec_madd( mcol2, zzzz, res );
    return Vector3( res );
}

VECTORMATH_FORCE_INLINE const Matrix3 crossMatrix( const Vector3 &vec )
{
    __m128 neg, res0, res1, res2;
    neg = negatef4( vec.get128() );
	VM_ATTRIBUTE_ALIGN16 unsigned int select_x[4] = {0xffffffff, 0, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_y[4] = {0, 0xffffffff, 0, 0};
	VM_ATTRIBUTE_ALIGN16 unsigned int select_z[4] = {0, 0, 0xffffffff, 0};
    //res0 = vec_perm( vec.get128(), neg, _VECTORMATH_PERM_XZBX );
	res0 = _mm_shuffle_ps( vec.get128(), vec.get128(), _MM_SHUFFLE(0,2,2,0) );
	res0 = vec_sel(res0, vec_splat(neg, 1), select_z);
    //res1 = vec_perm( vec.get128(), neg, _VECTORMATH_PERM_CXXX );
	res1 = vec_sel(vec_splat(vec.get128(), 0), vec_splat(neg, 2), select_x);
    //res2 = vec_perm( vec.get128(), neg, _VECTORMATH_PERM_YAXX );
	res2 = _mm_shuffle_ps( vec.get128(), vec.get128(), _MM_SHUFFLE(0,0,1,1) );
	res2 = vec_sel(res2, vec_splat(neg, 0), select_y);
	VM_ATTRIBUTE_ALIGN16 unsigned int filter_x[4] = {0, 0xffffffff, 0xffffffff, 0xffffffff};
	VM_ATTRIBUTE_ALIGN16 unsigned int filter_y[4] = {0xffffffff, 0, 0xffffffff, 0xffffffff};
	VM_ATTRIBUTE_ALIGN16 unsigned int filter_z[4] = {0xffffffff, 0xffffffff, 0, 0xffffffff};
    res0 = vec_and( res0, _mm_load_ps((float *)filter_x ) );
    res1 = vec_and( res1, _mm_load_ps((float *)filter_y ) );
    res2 = vec_and( res2, _mm_load_ps((float *)filter_z ) ); // TODO: Use selects?
    return Matrix3(
        Vector3( res0 ),
        Vector3( res1 ),
        Vector3( res2 )
    );
}

VECTORMATH_FORCE_INLINE const Matrix3 crossMatrixMul( const Vector3 &vec, const Matrix3 & mat )
{
    return Matrix3( cross( vec, mat.getCol0() ), cross( vec, mat.getCol1() ), cross( vec, mat.getCol2() ) );
}

} // namespace Aos
} // namespace Vectormath

#endif
