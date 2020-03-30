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

#ifndef _VECTORMATH_VEC_AOS_CPP_H
#define _VECTORMATH_VEC_AOS_CPP_H

//-----------------------------------------------------------------------------
// Constants

#define _VECTORMATH_SLERP_TOL 0.999f

//-----------------------------------------------------------------------------
// Definitions

#ifndef _VECTORMATH_INTERNAL_FUNCTIONS
#define _VECTORMATH_INTERNAL_FUNCTIONS

#endif

namespace Vectormath {
namespace Aos {

inline Vector3::Vector3( const Vector3 & vec )
{
    mX = vec.mX;
    mY = vec.mY;
    mZ = vec.mZ;
}

inline Vector3::Vector3( float _x, float _y, float _z )
{
    mX = _x;
    mY = _y;
    mZ = _z;
}

inline Vector3::Vector3( const Point3 & pnt )
{
    mX = pnt.getX();
    mY = pnt.getY();
    mZ = pnt.getZ();
}

inline Vector3::Vector3( float scalar )
{
    mX = scalar;
    mY = scalar;
    mZ = scalar;
}

inline const Vector3 Vector3::xAxis( )
{
    return Vector3( 1.0f, 0.0f, 0.0f );
}

inline const Vector3 Vector3::yAxis( )
{
    return Vector3( 0.0f, 1.0f, 0.0f );
}

inline const Vector3 Vector3::zAxis( )
{
    return Vector3( 0.0f, 0.0f, 1.0f );
}

inline const Vector3 lerp( float t, const Vector3 & vec0, const Vector3 & vec1 )
{
    return ( vec0 + ( ( vec1 - vec0 ) * t ) );
}

inline const Vector3 slerp( float t, const Vector3 & unitVec0, const Vector3 & unitVec1 )
{
    float recipSinAngle, scale0, scale1, cosAngle, angle;
    cosAngle = dot( unitVec0, unitVec1 );
    if ( cosAngle < _VECTORMATH_SLERP_TOL ) {
        angle = acosf( cosAngle );
        recipSinAngle = ( 1.0f / sinf( angle ) );
        scale0 = ( sinf( ( ( 1.0f - t ) * angle ) ) * recipSinAngle );
        scale1 = ( sinf( ( t * angle ) ) * recipSinAngle );
    } else {
        scale0 = ( 1.0f - t );
        scale1 = t;
    }
    return ( ( unitVec0 * scale0 ) + ( unitVec1 * scale1 ) );
}

inline void loadXYZ( Vector3 & vec, const float * fptr )
{
    vec = Vector3( fptr[0], fptr[1], fptr[2] );
}

inline void storeXYZ( const Vector3 & vec, float * fptr )
{
    fptr[0] = vec.getX();
    fptr[1] = vec.getY();
    fptr[2] = vec.getZ();
}

inline void loadHalfFloats( Vector3 & vec, const unsigned short * hfptr )
{
    union Data32 {
        unsigned int u32;
        float f32;
    };

    for (int i = 0; i < 3; i++) {
        unsigned short fp16 = hfptr[i];
        unsigned int sign = fp16 >> 15;
        unsigned int exponent = (fp16 >> 10) & ((1 << 5) - 1);
        unsigned int mantissa = fp16 & ((1 << 10) - 1);

        if (exponent == 0) {
            // zero
            mantissa = 0;

        } else if (exponent == 31) {
            // infinity or nan -> infinity
            exponent = 255;
	    mantissa = 0;

        } else {
            exponent += 127 - 15;
            mantissa <<= 13;
        }

        Data32 d;
        d.u32 = (sign << 31) | (exponent << 23) | mantissa;
        vec[i] = d.f32;
    }
}

inline void storeHalfFloats( const Vector3 & vec, unsigned short * hfptr )
{
    union Data32 {
        unsigned int u32;
        float f32;
    };

    for (int i = 0; i < 3; i++) {
        Data32 d;
        d.f32 = vec[i];

        unsigned int sign = d.u32 >> 31;
        unsigned int exponent = (d.u32 >> 23) & ((1 << 8) - 1);
        unsigned int mantissa = d.u32 & ((1 << 23) - 1);;

        if (exponent == 0) {
            // zero or denorm -> zero
            mantissa = 0;

        } else if (exponent == 255 && mantissa != 0) {
            // nan -> infinity
            exponent = 31;
            mantissa = 0;

        } else if (exponent >= 127 - 15 + 31) {
            // overflow or infinity -> infinity
            exponent = 31;
            mantissa = 0;

        } else if (exponent <= 127 - 15) {
            // underflow -> zero
            exponent = 0;
            mantissa = 0;

        } else {
            exponent -= 127 - 15;
            mantissa >>= 13;
        }

        hfptr[i] = (unsigned short)((sign << 15) | (exponent << 10) | mantissa);
    }
}

inline Vector3 & Vector3::operator =( const Vector3 & vec )
{
    mX = vec.mX;
    mY = vec.mY;
    mZ = vec.mZ;
    return *this;
}

inline Vector3 & Vector3::setX( float _x )
{
    mX = _x;
    return *this;
}

inline float Vector3::getX( ) const
{
    return mX;
}

inline Vector3 & Vector3::setY( float _y )
{
    mY = _y;
    return *this;
}

inline float Vector3::getY( ) const
{
    return mY;
}

inline Vector3 & Vector3::setZ( float _z )
{
    mZ = _z;
    return *this;
}

inline float Vector3::getZ( ) const
{
    return mZ;
}

inline Vector3 & Vector3::setElem( int idx, float value )
{
    *(&mX + idx) = value;
    return *this;
}

inline float Vector3::getElem( int idx ) const
{
    return *(&mX + idx);
}

inline float & Vector3::operator []( int idx )
{
    return *(&mX + idx);
}

inline float Vector3::operator []( int idx ) const
{
    return *(&mX + idx);
}

inline const Vector3 Vector3::operator +( const Vector3 & vec ) const
{
    return Vector3(
        ( mX + vec.mX ),
        ( mY + vec.mY ),
        ( mZ + vec.mZ )
    );
}

inline const Vector3 Vector3::operator -( const Vector3 & vec ) const
{
    return Vector3(
        ( mX - vec.mX ),
        ( mY - vec.mY ),
        ( mZ - vec.mZ )
    );
}

inline const Point3 Vector3::operator +( const Point3 & pnt ) const
{
    return Point3(
        ( mX + pnt.getX() ),
        ( mY + pnt.getY() ),
        ( mZ + pnt.getZ() )
    );
}

inline const Vector3 Vector3::operator *( float scalar ) const
{
    return Vector3(
        ( mX * scalar ),
        ( mY * scalar ),
        ( mZ * scalar )
    );
}

inline Vector3 & Vector3::operator +=( const Vector3 & vec )
{
    *this = *this + vec;
    return *this;
}

inline Vector3 & Vector3::operator -=( const Vector3 & vec )
{
    *this = *this - vec;
    return *this;
}

inline Vector3 & Vector3::operator *=( float scalar )
{
    *this = *this * scalar;
    return *this;
}

inline const Vector3 Vector3::operator /( float scalar ) const
{
    return Vector3(
        ( mX / scalar ),
        ( mY / scalar ),
        ( mZ / scalar )
    );
}

inline Vector3 & Vector3::operator /=( float scalar )
{
    *this = *this / scalar;
    return *this;
}

inline const Vector3 Vector3::operator -( ) const
{
    return Vector3(
        -mX,
        -mY,
        -mZ
    );
}

inline const Vector3 operator *( float scalar, const Vector3 & vec )
{
    return vec * scalar;
}

inline const Vector3 mulPerElem( const Vector3 & vec0, const Vector3 & vec1 )
{
    return Vector3(
        ( vec0.getX() * vec1.getX() ),
        ( vec0.getY() * vec1.getY() ),
        ( vec0.getZ() * vec1.getZ() )
    );
}

inline const Vector3 divPerElem( const Vector3 & vec0, const Vector3 & vec1 )
{
    return Vector3(
        ( vec0.getX() / vec1.getX() ),
        ( vec0.getY() / vec1.getY() ),
        ( vec0.getZ() / vec1.getZ() )
    );
}

inline const Vector3 recipPerElem( const Vector3 & vec )
{
    return Vector3(
        ( 1.0f / vec.getX() ),
        ( 1.0f / vec.getY() ),
        ( 1.0f / vec.getZ() )
    );
}

inline const Vector3 sqrtPerElem( const Vector3 & vec )
{
    return Vector3(
        sqrtf( vec.getX() ),
        sqrtf( vec.getY() ),
        sqrtf( vec.getZ() )
    );
}

inline const Vector3 rsqrtPerElem( const Vector3 & vec )
{
    return Vector3(
        ( 1.0f / sqrtf( vec.getX() ) ),
        ( 1.0f / sqrtf( vec.getY() ) ),
        ( 1.0f / sqrtf( vec.getZ() ) )
    );
}

inline const Vector3 absPerElem( const Vector3 & vec )
{
    return Vector3(
        fabsf( vec.getX() ),
        fabsf( vec.getY() ),
        fabsf( vec.getZ() )
    );
}

inline const Vector3 copySignPerElem( const Vector3 & vec0, const Vector3 & vec1 )
{
    return Vector3(
        ( vec1.getX() < 0.0f )? -fabsf( vec0.getX() ) : fabsf( vec0.getX() ),
        ( vec1.getY() < 0.0f )? -fabsf( vec0.getY() ) : fabsf( vec0.getY() ),
        ( vec1.getZ() < 0.0f )? -fabsf( vec0.getZ() ) : fabsf( vec0.getZ() )
    );
}

inline const Vector3 maxPerElem( const Vector3 & vec0, const Vector3 & vec1 )
{
    return Vector3(
        (vec0.getX() > vec1.getX())? vec0.getX() : vec1.getX(),
        (vec0.getY() > vec1.getY())? vec0.getY() : vec1.getY(),
        (vec0.getZ() > vec1.getZ())? vec0.getZ() : vec1.getZ()
    );
}

inline float maxElem( const Vector3 & vec )
{
    float result;
    result = (vec.getX() > vec.getY())? vec.getX() : vec.getY();
    result = (vec.getZ() > result)? vec.getZ() : result;
    return result;
}

inline const Vector3 minPerElem( const Vector3 & vec0, const Vector3 & vec1 )
{
    return Vector3(
        (vec0.getX() < vec1.getX())? vec0.getX() : vec1.getX(),
        (vec0.getY() < vec1.getY())? vec0.getY() : vec1.getY(),
        (vec0.getZ() < vec1.getZ())? vec0.getZ() : vec1.getZ()
    );
}

inline float minElem( const Vector3 & vec )
{
    float result;
    result = (vec.getX() < vec.getY())? vec.getX() : vec.getY();
    result = (vec.getZ() < result)? vec.getZ() : result;
    return result;
}

inline float sum( const Vector3 & vec )
{
    float result;
    result = ( vec.getX() + vec.getY() );
    result = ( result + vec.getZ() );
    return result;
}

inline float dot( const Vector3 & vec0, const Vector3 & vec1 )
{
    float result;
    result = ( vec0.getX() * vec1.getX() );
    result = ( result + ( vec0.getY() * vec1.getY() ) );
    result = ( result + ( vec0.getZ() * vec1.getZ() ) );
    return result;
}

inline float lengthSqr( const Vector3 & vec )
{
    float result;
    result = ( vec.getX() * vec.getX() );
    result = ( result + ( vec.getY() * vec.getY() ) );
    result = ( result + ( vec.getZ() * vec.getZ() ) );
    return result;
}

inline float length( const Vector3 & vec )
{
    return ::sqrtf( lengthSqr( vec ) );
}

inline const Vector3 normalize( const Vector3 & vec )
{
    float lenSqr, lenInv;
    lenSqr = lengthSqr( vec );
    lenInv = ( 1.0f / sqrtf( lenSqr ) );
    return Vector3(
        ( vec.getX() * lenInv ),
        ( vec.getY() * lenInv ),
        ( vec.getZ() * lenInv )
    );
}

inline const Vector3 cross( const Vector3 & vec0, const Vector3 & vec1 )
{
    return Vector3(
        ( ( vec0.getY() * vec1.getZ() ) - ( vec0.getZ() * vec1.getY() ) ),
        ( ( vec0.getZ() * vec1.getX() ) - ( vec0.getX() * vec1.getZ() ) ),
        ( ( vec0.getX() * vec1.getY() ) - ( vec0.getY() * vec1.getX() ) )
    );
}

inline const Vector3 select( const Vector3 & vec0, const Vector3 & vec1, bool select1 )
{
    return Vector3(
        ( select1 )? vec1.getX() : vec0.getX(),
        ( select1 )? vec1.getY() : vec0.getY(),
        ( select1 )? vec1.getZ() : vec0.getZ()
    );
}

#ifdef _VECTORMATH_DEBUG

inline void print( const Vector3 & vec )
{
    printf( "( %f %f %f )\n", vec.getX(), vec.getY(), vec.getZ() );
}

inline void print( const Vector3 & vec, const char * name )
{
    printf( "%s: ( %f %f %f )\n", name, vec.getX(), vec.getY(), vec.getZ() );
}

#endif

inline Vector4::Vector4( const Vector4 & vec )
{
    mX = vec.mX;
    mY = vec.mY;
    mZ = vec.mZ;
    mW = vec.mW;
}

inline Vector4::Vector4( float _x, float _y, float _z, float _w )
{
    mX = _x;
    mY = _y;
    mZ = _z;
    mW = _w;
}

inline Vector4::Vector4( const Vector3 & xyz, float _w )
{
    this->setXYZ( xyz );
    this->setW( _w );
}

inline Vector4::Vector4( const Vector3 & vec )
{
    mX = vec.getX();
    mY = vec.getY();
    mZ = vec.getZ();
    mW = 0.0f;
}

inline Vector4::Vector4( const Point3 & pnt )
{
    mX = pnt.getX();
    mY = pnt.getY();
    mZ = pnt.getZ();
    mW = 1.0f;
}

inline Vector4::Vector4( const Quat & quat )
{
    mX = quat.getX();
    mY = quat.getY();
    mZ = quat.getZ();
    mW = quat.getW();
}

inline Vector4::Vector4( float scalar )
{
    mX = scalar;
    mY = scalar;
    mZ = scalar;
    mW = scalar;
}

inline const Vector4 Vector4::xAxis( )
{
    return Vector4( 1.0f, 0.0f, 0.0f, 0.0f );
}

inline const Vector4 Vector4::yAxis( )
{
    return Vector4( 0.0f, 1.0f, 0.0f, 0.0f );
}

inline const Vector4 Vector4::zAxis( )
{
    return Vector4( 0.0f, 0.0f, 1.0f, 0.0f );
}

inline const Vector4 Vector4::wAxis( )
{
    return Vector4( 0.0f, 0.0f, 0.0f, 1.0f );
}

inline const Vector4 lerp( float t, const Vector4 & vec0, const Vector4 & vec1 )
{
    return ( vec0 + ( ( vec1 - vec0 ) * t ) );
}

inline const Vector4 slerp( float t, const Vector4 & unitVec0, const Vector4 & unitVec1 )
{
    float recipSinAngle, scale0, scale1, cosAngle, angle;
    cosAngle = dot( unitVec0, unitVec1 );
    if ( cosAngle < _VECTORMATH_SLERP_TOL ) {
        angle = acosf( cosAngle );
        recipSinAngle = ( 1.0f / sinf( angle ) );
        scale0 = ( sinf( ( ( 1.0f - t ) * angle ) ) * recipSinAngle );
        scale1 = ( sinf( ( t * angle ) ) * recipSinAngle );
    } else {
        scale0 = ( 1.0f - t );
        scale1 = t;
    }
    return ( ( unitVec0 * scale0 ) + ( unitVec1 * scale1 ) );
}

inline void loadXYZW( Vector4 & vec, const float * fptr )
{
    vec = Vector4( fptr[0], fptr[1], fptr[2], fptr[3] );
}

inline void storeXYZW( const Vector4 & vec, float * fptr )
{
    fptr[0] = vec.getX();
    fptr[1] = vec.getY();
    fptr[2] = vec.getZ();
    fptr[3] = vec.getW();
}

inline void loadHalfFloats( Vector4 & vec, const unsigned short * hfptr )
{
    union Data32 {
        unsigned int u32;
        float f32;
    };

    for (int i = 0; i < 4; i++) {
        unsigned short fp16 = hfptr[i];
        unsigned int sign = fp16 >> 15;
        unsigned int exponent = (fp16 >> 10) & ((1 << 5) - 1);
        unsigned int mantissa = fp16 & ((1 << 10) - 1);

        if (exponent == 0) {
            // zero
            mantissa = 0;

        } else if (exponent == 31) {
            // infinity or nan -> infinity
            exponent = 255;
	    mantissa = 0;

        } else {
            exponent += 127 - 15;
            mantissa <<= 13;
        }

        Data32 d;
        d.u32 = (sign << 31) | (exponent << 23) | mantissa;
        vec[i] = d.f32;
    }
}

inline void storeHalfFloats( const Vector4 & vec, unsigned short * hfptr )
{
    union Data32 {
        unsigned int u32;
        float f32;
    };

    for (int i = 0; i < 4; i++) {
        Data32 d;
        d.f32 = vec[i];

        unsigned int sign = d.u32 >> 31;
        unsigned int exponent = (d.u32 >> 23) & ((1 << 8) - 1);
        unsigned int mantissa = d.u32 & ((1 << 23) - 1);;

        if (exponent == 0) {
            // zero or denorm -> zero
            mantissa = 0;

        } else if (exponent == 255 && mantissa != 0) {
            // nan -> infinity
            exponent = 31;
            mantissa = 0;

        } else if (exponent >= 127 - 15 + 31) {
            // overflow or infinity -> infinity
            exponent = 31;
            mantissa = 0;

        } else if (exponent <= 127 - 15) {
            // underflow -> zero
            exponent = 0;
            mantissa = 0;

        } else {
            exponent -= 127 - 15;
            mantissa >>= 13;
        }

        hfptr[i] = (unsigned short)((sign << 15) | (exponent << 10) | mantissa);
    }
}

inline Vector4 & Vector4::operator =( const Vector4 & vec )
{
    mX = vec.mX;
    mY = vec.mY;
    mZ = vec.mZ;
    mW = vec.mW;
    return *this;
}

inline Vector4 & Vector4::setXYZ( const Vector3 & vec )
{
    mX = vec.getX();
    mY = vec.getY();
    mZ = vec.getZ();
    return *this;
}

inline const Vector3 Vector4::getXYZ( ) const
{
    return Vector3( mX, mY, mZ );
}

inline Vector4 & Vector4::setX( float _x )
{
    mX = _x;
    return *this;
}

inline float Vector4::getX( ) const
{
    return mX;
}

inline Vector4 & Vector4::setY( float _y )
{
    mY = _y;
    return *this;
}

inline float Vector4::getY( ) const
{
    return mY;
}

inline Vector4 & Vector4::setZ( float _z )
{
    mZ = _z;
    return *this;
}

inline float Vector4::getZ( ) const
{
    return mZ;
}

inline Vector4 & Vector4::setW( float _w )
{
    mW = _w;
    return *this;
}

inline float Vector4::getW( ) const
{
    return mW;
}

inline Vector4 & Vector4::setElem( int idx, float value )
{
    *(&mX + idx) = value;
    return *this;
}

inline float Vector4::getElem( int idx ) const
{
    return *(&mX + idx);
}

inline float & Vector4::operator []( int idx )
{
    return *(&mX + idx);
}

inline float Vector4::operator []( int idx ) const
{
    return *(&mX + idx);
}

inline const Vector4 Vector4::operator +( const Vector4 & vec ) const
{
    return Vector4(
        ( mX + vec.mX ),
        ( mY + vec.mY ),
        ( mZ + vec.mZ ),
        ( mW + vec.mW )
    );
}

inline const Vector4 Vector4::operator -( const Vector4 & vec ) const
{
    return Vector4(
        ( mX - vec.mX ),
        ( mY - vec.mY ),
        ( mZ - vec.mZ ),
        ( mW - vec.mW )
    );
}

inline const Vector4 Vector4::operator *( float scalar ) const
{
    return Vector4(
        ( mX * scalar ),
        ( mY * scalar ),
        ( mZ * scalar ),
        ( mW * scalar )
    );
}

inline Vector4 & Vector4::operator +=( const Vector4 & vec )
{
    *this = *this + vec;
    return *this;
}

inline Vector4 & Vector4::operator -=( const Vector4 & vec )
{
    *this = *this - vec;
    return *this;
}

inline Vector4 & Vector4::operator *=( float scalar )
{
    *this = *this * scalar;
    return *this;
}

inline const Vector4 Vector4::operator /( float scalar ) const
{
    return Vector4(
        ( mX / scalar ),
        ( mY / scalar ),
        ( mZ / scalar ),
        ( mW / scalar )
    );
}

inline Vector4 & Vector4::operator /=( float scalar )
{
    *this = *this / scalar;
    return *this;
}

inline const Vector4 Vector4::operator -( ) const
{
    return Vector4(
        -mX,
        -mY,
        -mZ,
        -mW
    );
}

inline const Vector4 operator *( float scalar, const Vector4 & vec )
{
    return vec * scalar;
}

inline const Vector4 mulPerElem( const Vector4 & vec0, const Vector4 & vec1 )
{
    return Vector4(
        ( vec0.getX() * vec1.getX() ),
        ( vec0.getY() * vec1.getY() ),
        ( vec0.getZ() * vec1.getZ() ),
        ( vec0.getW() * vec1.getW() )
    );
}

inline const Vector4 divPerElem( const Vector4 & vec0, const Vector4 & vec1 )
{
    return Vector4(
        ( vec0.getX() / vec1.getX() ),
        ( vec0.getY() / vec1.getY() ),
        ( vec0.getZ() / vec1.getZ() ),
        ( vec0.getW() / vec1.getW() )
    );
}

inline const Vector4 recipPerElem( const Vector4 & vec )
{
    return Vector4(
        ( 1.0f / vec.getX() ),
        ( 1.0f / vec.getY() ),
        ( 1.0f / vec.getZ() ),
        ( 1.0f / vec.getW() )
    );
}

inline const Vector4 sqrtPerElem( const Vector4 & vec )
{
    return Vector4(
        sqrtf( vec.getX() ),
        sqrtf( vec.getY() ),
        sqrtf( vec.getZ() ),
        sqrtf( vec.getW() )
    );
}

inline const Vector4 rsqrtPerElem( const Vector4 & vec )
{
    return Vector4(
        ( 1.0f / sqrtf( vec.getX() ) ),
        ( 1.0f / sqrtf( vec.getY() ) ),
        ( 1.0f / sqrtf( vec.getZ() ) ),
        ( 1.0f / sqrtf( vec.getW() ) )
    );
}

inline const Vector4 absPerElem( const Vector4 & vec )
{
    return Vector4(
        fabsf( vec.getX() ),
        fabsf( vec.getY() ),
        fabsf( vec.getZ() ),
        fabsf( vec.getW() )
    );
}

inline const Vector4 copySignPerElem( const Vector4 & vec0, const Vector4 & vec1 )
{
    return Vector4(
        ( vec1.getX() < 0.0f )? -fabsf( vec0.getX() ) : fabsf( vec0.getX() ),
        ( vec1.getY() < 0.0f )? -fabsf( vec0.getY() ) : fabsf( vec0.getY() ),
        ( vec1.getZ() < 0.0f )? -fabsf( vec0.getZ() ) : fabsf( vec0.getZ() ),
        ( vec1.getW() < 0.0f )? -fabsf( vec0.getW() ) : fabsf( vec0.getW() )
    );
}

inline const Vector4 maxPerElem( const Vector4 & vec0, const Vector4 & vec1 )
{
    return Vector4(
        (vec0.getX() > vec1.getX())? vec0.getX() : vec1.getX(),
        (vec0.getY() > vec1.getY())? vec0.getY() : vec1.getY(),
        (vec0.getZ() > vec1.getZ())? vec0.getZ() : vec1.getZ(),
        (vec0.getW() > vec1.getW())? vec0.getW() : vec1.getW()
    );
}

inline float maxElem( const Vector4 & vec )
{
    float result;
    result = (vec.getX() > vec.getY())? vec.getX() : vec.getY();
    result = (vec.getZ() > result)? vec.getZ() : result;
    result = (vec.getW() > result)? vec.getW() : result;
    return result;
}

inline const Vector4 minPerElem( const Vector4 & vec0, const Vector4 & vec1 )
{
    return Vector4(
        (vec0.getX() < vec1.getX())? vec0.getX() : vec1.getX(),
        (vec0.getY() < vec1.getY())? vec0.getY() : vec1.getY(),
        (vec0.getZ() < vec1.getZ())? vec0.getZ() : vec1.getZ(),
        (vec0.getW() < vec1.getW())? vec0.getW() : vec1.getW()
    );
}

inline float minElem( const Vector4 & vec )
{
    float result;
    result = (vec.getX() < vec.getY())? vec.getX() : vec.getY();
    result = (vec.getZ() < result)? vec.getZ() : result;
    result = (vec.getW() < result)? vec.getW() : result;
    return result;
}

inline float sum( const Vector4 & vec )
{
    float result;
    result = ( vec.getX() + vec.getY() );
    result = ( result + vec.getZ() );
    result = ( result + vec.getW() );
    return result;
}

inline float dot( const Vector4 & vec0, const Vector4 & vec1 )
{
    float result;
    result = ( vec0.getX() * vec1.getX() );
    result = ( result + ( vec0.getY() * vec1.getY() ) );
    result = ( result + ( vec0.getZ() * vec1.getZ() ) );
    result = ( result + ( vec0.getW() * vec1.getW() ) );
    return result;
}

inline float lengthSqr( const Vector4 & vec )
{
    float result;
    result = ( vec.getX() * vec.getX() );
    result = ( result + ( vec.getY() * vec.getY() ) );
    result = ( result + ( vec.getZ() * vec.getZ() ) );
    result = ( result + ( vec.getW() * vec.getW() ) );
    return result;
}

inline float length( const Vector4 & vec )
{
    return ::sqrtf( lengthSqr( vec ) );
}

inline const Vector4 normalize( const Vector4 & vec )
{
    float lenSqr, lenInv;
    lenSqr = lengthSqr( vec );
    lenInv = ( 1.0f / sqrtf( lenSqr ) );
    return Vector4(
        ( vec.getX() * lenInv ),
        ( vec.getY() * lenInv ),
        ( vec.getZ() * lenInv ),
        ( vec.getW() * lenInv )
    );
}

inline const Vector4 select( const Vector4 & vec0, const Vector4 & vec1, bool select1 )
{
    return Vector4(
        ( select1 )? vec1.getX() : vec0.getX(),
        ( select1 )? vec1.getY() : vec0.getY(),
        ( select1 )? vec1.getZ() : vec0.getZ(),
        ( select1 )? vec1.getW() : vec0.getW()
    );
}

#ifdef _VECTORMATH_DEBUG

inline void print( const Vector4 & vec )
{
    printf( "( %f %f %f %f )\n", vec.getX(), vec.getY(), vec.getZ(), vec.getW() );
}

inline void print( const Vector4 & vec, const char * name )
{
    printf( "%s: ( %f %f %f %f )\n", name, vec.getX(), vec.getY(), vec.getZ(), vec.getW() );
}

#endif

inline Point3::Point3( const Point3 & pnt )
{
    mX = pnt.mX;
    mY = pnt.mY;
    mZ = pnt.mZ;
}

inline Point3::Point3( float _x, float _y, float _z )
{
    mX = _x;
    mY = _y;
    mZ = _z;
}

inline Point3::Point3( const Vector3 & vec )
{
    mX = vec.getX();
    mY = vec.getY();
    mZ = vec.getZ();
}

inline Point3::Point3( float scalar )
{
    mX = scalar;
    mY = scalar;
    mZ = scalar;
}

inline const Point3 lerp( float t, const Point3 & pnt0, const Point3 & pnt1 )
{
    return ( pnt0 + ( ( pnt1 - pnt0 ) * t ) );
}

inline void loadXYZ( Point3 & pnt, const float * fptr )
{
    pnt = Point3( fptr[0], fptr[1], fptr[2] );
}

inline void storeXYZ( const Point3 & pnt, float * fptr )
{
    fptr[0] = pnt.getX();
    fptr[1] = pnt.getY();
    fptr[2] = pnt.getZ();
}

inline void loadHalfFloats( Point3 & vec, const unsigned short * hfptr )
{
    union Data32 {
        unsigned int u32;
        float f32;
    };

    for (int i = 0; i < 3; i++) {
        unsigned short fp16 = hfptr[i];
        unsigned int sign = fp16 >> 15;
        unsigned int exponent = (fp16 >> 10) & ((1 << 5) - 1);
        unsigned int mantissa = fp16 & ((1 << 10) - 1);

        if (exponent == 0) {
            // zero
            mantissa = 0;

        } else if (exponent == 31) {
            // infinity or nan -> infinity
            exponent = 255;
	    mantissa = 0;

        } else {
            exponent += 127 - 15;
            mantissa <<= 13;
        }

        Data32 d;
        d.u32 = (sign << 31) | (exponent << 23) | mantissa;
        vec[i] = d.f32;
    }
}

inline void storeHalfFloats( const Point3 & vec, unsigned short * hfptr )
{
    union Data32 {
        unsigned int u32;
        float f32;
    };

    for (int i = 0; i < 3; i++) {
        Data32 d;
        d.f32 = vec[i];

        unsigned int sign = d.u32 >> 31;
        unsigned int exponent = (d.u32 >> 23) & ((1 << 8) - 1);
        unsigned int mantissa = d.u32 & ((1 << 23) - 1);;

        if (exponent == 0) {
            // zero or denorm -> zero
            mantissa = 0;

        } else if (exponent == 255 && mantissa != 0) {
            // nan -> infinity
            exponent = 31;
            mantissa = 0;

        } else if (exponent >= 127 - 15 + 31) {
            // overflow or infinity -> infinity
            exponent = 31;
            mantissa = 0;

        } else if (exponent <= 127 - 15) {
            // underflow -> zero
            exponent = 0;
            mantissa = 0;

        } else {
            exponent -= 127 - 15;
            mantissa >>= 13;
        }

        hfptr[i] = (unsigned short)((sign << 15) | (exponent << 10) | mantissa);
    }
}

inline Point3 & Point3::operator =( const Point3 & pnt )
{
    mX = pnt.mX;
    mY = pnt.mY;
    mZ = pnt.mZ;
    return *this;
}

inline Point3 & Point3::setX( float _x )
{
    mX = _x;
    return *this;
}

inline float Point3::getX( ) const
{
    return mX;
}

inline Point3 & Point3::setY( float _y )
{
    mY = _y;
    return *this;
}

inline float Point3::getY( ) const
{
    return mY;
}

inline Point3 & Point3::setZ( float _z )
{
    mZ = _z;
    return *this;
}

inline float Point3::getZ( ) const
{
    return mZ;
}

inline Point3 & Point3::setElem( int idx, float value )
{
    *(&mX + idx) = value;
    return *this;
}

inline float Point3::getElem( int idx ) const
{
    return *(&mX + idx);
}

inline float & Point3::operator []( int idx )
{
    return *(&mX + idx);
}

inline float Point3::operator []( int idx ) const
{
    return *(&mX + idx);
}

inline const Vector3 Point3::operator -( const Point3 & pnt ) const
{
    return Vector3(
        ( mX - pnt.mX ),
        ( mY - pnt.mY ),
        ( mZ - pnt.mZ )
    );
}

inline const Point3 Point3::operator +( const Vector3 & vec ) const
{
    return Point3(
        ( mX + vec.getX() ),
        ( mY + vec.getY() ),
        ( mZ + vec.getZ() )
    );
}

inline const Point3 Point3::operator -( const Vector3 & vec ) const
{
    return Point3(
        ( mX - vec.getX() ),
        ( mY - vec.getY() ),
        ( mZ - vec.getZ() )
    );
}

inline Point3 & Point3::operator +=( const Vector3 & vec )
{
    *this = *this + vec;
    return *this;
}

inline Point3 & Point3::operator -=( const Vector3 & vec )
{
    *this = *this - vec;
    return *this;
}

inline const Point3 mulPerElem( const Point3 & pnt0, const Point3 & pnt1 )
{
    return Point3(
        ( pnt0.getX() * pnt1.getX() ),
        ( pnt0.getY() * pnt1.getY() ),
        ( pnt0.getZ() * pnt1.getZ() )
    );
}

inline const Point3 divPerElem( const Point3 & pnt0, const Point3 & pnt1 )
{
    return Point3(
        ( pnt0.getX() / pnt1.getX() ),
        ( pnt0.getY() / pnt1.getY() ),
        ( pnt0.getZ() / pnt1.getZ() )
    );
}

inline const Point3 recipPerElem( const Point3 & pnt )
{
    return Point3(
        ( 1.0f / pnt.getX() ),
        ( 1.0f / pnt.getY() ),
        ( 1.0f / pnt.getZ() )
    );
}

inline const Point3 sqrtPerElem( const Point3 & pnt )
{
    return Point3(
        sqrtf( pnt.getX() ),
        sqrtf( pnt.getY() ),
        sqrtf( pnt.getZ() )
    );
}

inline const Point3 rsqrtPerElem( const Point3 & pnt )
{
    return Point3(
        ( 1.0f / sqrtf( pnt.getX() ) ),
        ( 1.0f / sqrtf( pnt.getY() ) ),
        ( 1.0f / sqrtf( pnt.getZ() ) )
    );
}

inline const Point3 absPerElem( const Point3 & pnt )
{
    return Point3(
        fabsf( pnt.getX() ),
        fabsf( pnt.getY() ),
        fabsf( pnt.getZ() )
    );
}

inline const Point3 copySignPerElem( const Point3 & pnt0, const Point3 & pnt1 )
{
    return Point3(
        ( pnt1.getX() < 0.0f )? -fabsf( pnt0.getX() ) : fabsf( pnt0.getX() ),
        ( pnt1.getY() < 0.0f )? -fabsf( pnt0.getY() ) : fabsf( pnt0.getY() ),
        ( pnt1.getZ() < 0.0f )? -fabsf( pnt0.getZ() ) : fabsf( pnt0.getZ() )
    );
}

inline const Point3 maxPerElem( const Point3 & pnt0, const Point3 & pnt1 )
{
    return Point3(
        (pnt0.getX() > pnt1.getX())? pnt0.getX() : pnt1.getX(),
        (pnt0.getY() > pnt1.getY())? pnt0.getY() : pnt1.getY(),
        (pnt0.getZ() > pnt1.getZ())? pnt0.getZ() : pnt1.getZ()
    );
}

inline float maxElem( const Point3 & pnt )
{
    float result;
    result = (pnt.getX() > pnt.getY())? pnt.getX() : pnt.getY();
    result = (pnt.getZ() > result)? pnt.getZ() : result;
    return result;
}

inline const Point3 minPerElem( const Point3 & pnt0, const Point3 & pnt1 )
{
    return Point3(
        (pnt0.getX() < pnt1.getX())? pnt0.getX() : pnt1.getX(),
        (pnt0.getY() < pnt1.getY())? pnt0.getY() : pnt1.getY(),
        (pnt0.getZ() < pnt1.getZ())? pnt0.getZ() : pnt1.getZ()
    );
}

inline float minElem( const Point3 & pnt )
{
    float result;
    result = (pnt.getX() < pnt.getY())? pnt.getX() : pnt.getY();
    result = (pnt.getZ() < result)? pnt.getZ() : result;
    return result;
}

inline float sum( const Point3 & pnt )
{
    float result;
    result = ( pnt.getX() + pnt.getY() );
    result = ( result + pnt.getZ() );
    return result;
}

inline const Point3 scale( const Point3 & pnt, float scaleVal )
{
    return mulPerElem( pnt, Point3( scaleVal ) );
}

inline const Point3 scale( const Point3 & pnt, const Vector3 & scaleVec )
{
    return mulPerElem( pnt, Point3( scaleVec ) );
}

inline float projection( const Point3 & pnt, const Vector3 & unitVec )
{
    float result;
    result = ( pnt.getX() * unitVec.getX() );
    result = ( result + ( pnt.getY() * unitVec.getY() ) );
    result = ( result + ( pnt.getZ() * unitVec.getZ() ) );
    return result;
}

inline float distSqrFromOrigin( const Point3 & pnt )
{
    return lengthSqr( Vector3( pnt ) );
}

inline float distFromOrigin( const Point3 & pnt )
{
    return length( Vector3( pnt ) );
}

inline float distSqr( const Point3 & pnt0, const Point3 & pnt1 )
{
    return lengthSqr( ( pnt1 - pnt0 ) );
}

inline float dist( const Point3 & pnt0, const Point3 & pnt1 )
{
    return length( ( pnt1 - pnt0 ) );
}

inline const Point3 select( const Point3 & pnt0, const Point3 & pnt1, bool select1 )
{
    return Point3(
        ( select1 )? pnt1.getX() : pnt0.getX(),
        ( select1 )? pnt1.getY() : pnt0.getY(),
        ( select1 )? pnt1.getZ() : pnt0.getZ()
    );
}

#ifdef _VECTORMATH_DEBUG

inline void print( const Point3 & pnt )
{
    printf( "( %f %f %f )\n", pnt.getX(), pnt.getY(), pnt.getZ() );
}

inline void print( const Point3 & pnt, const char * name )
{
    printf( "%s: ( %f %f %f )\n", name, pnt.getX(), pnt.getY(), pnt.getZ() );
}

#endif

} // namespace Aos
} // namespace Vectormath

#endif
