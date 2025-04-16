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

#ifndef _VECTORMATH_QUAT_AOS_CPP_H
#define _VECTORMATH_QUAT_AOS_CPP_H

//-----------------------------------------------------------------------------
// Definitions

#ifndef _VECTORMATH_INTERNAL_FUNCTIONS
#define _VECTORMATH_INTERNAL_FUNCTIONS

#endif

namespace Vectormath {
namespace Aos {

inline Quat::Quat( const Quat & quat )
{
    mX = quat.mX;
    mY = quat.mY;
    mZ = quat.mZ;
    mW = quat.mW;
}

inline Quat::Quat( float _x, float _y, float _z, float _w )
{
    mX = _x;
    mY = _y;
    mZ = _z;
    mW = _w;
}

inline Quat::Quat( const Vector3 & xyz, float _w )
{
    this->setXYZ( xyz );
    this->setW( _w );
}

inline Quat::Quat( const Vector4 & vec )
{
    mX = vec.getX();
    mY = vec.getY();
    mZ = vec.getZ();
    mW = vec.getW();
}

inline Quat::Quat( float scalar )
{
    mX = scalar;
    mY = scalar;
    mZ = scalar;
    mW = scalar;
}

inline const Quat Quat::identity( )
{
    return Quat( 0.0f, 0.0f, 0.0f, 1.0f );
}

inline const Quat lerp( float t, const Quat & quat0, const Quat & quat1 )
{
    return ( quat0 + ( ( quat1 - quat0 ) * t ) );
}

inline const Quat slerp( float t, const Quat & unitQuat0, const Quat & unitQuat1 )
{
    Quat start;
    float recipSinAngle, scale0, scale1, cosAngle, angle;
    cosAngle = dot( unitQuat0, unitQuat1 );
    if ( cosAngle < 0.0f ) {
        cosAngle = -cosAngle;
        start = ( -unitQuat0 );
    } else {
        start = unitQuat0;
    }
    if ( cosAngle < _VECTORMATH_SLERP_TOL ) {
        angle = acosf( cosAngle );
        recipSinAngle = ( 1.0f / sinf( angle ) );
        scale0 = ( sinf( ( ( 1.0f - t ) * angle ) ) * recipSinAngle );
        scale1 = ( sinf( ( t * angle ) ) * recipSinAngle );
    } else {
        scale0 = ( 1.0f - t );
        scale1 = t;
    }
    return ( ( start * scale0 ) + ( unitQuat1 * scale1 ) );
}

inline const Quat squad( float t, const Quat & unitQuat0, const Quat & unitQuat1, const Quat & unitQuat2, const Quat & unitQuat3 )
{
    Quat tmp0, tmp1;
    tmp0 = slerp( t, unitQuat0, unitQuat3 );
    tmp1 = slerp( t, unitQuat1, unitQuat2 );
    return slerp( ( ( 2.0f * t ) * ( 1.0f - t ) ), tmp0, tmp1 );
}

inline void loadXYZW( Quat & quat, const float * fptr )
{
    quat = Quat( fptr[0], fptr[1], fptr[2], fptr[3] );
}

inline void storeXYZW( const Quat & quat, float * fptr )
{
    fptr[0] = quat.getX();
    fptr[1] = quat.getY();
    fptr[2] = quat.getZ();
    fptr[3] = quat.getW();
}

inline Quat & Quat::operator =( const Quat & quat )
{
    mX = quat.mX;
    mY = quat.mY;
    mZ = quat.mZ;
    mW = quat.mW;
    return *this;
}

inline Quat & Quat::setXYZ( const Vector3 & vec )
{
    mX = vec.getX();
    mY = vec.getY();
    mZ = vec.getZ();
    return *this;
}

inline const Vector3 Quat::getXYZ( ) const
{
    return Vector3( mX, mY, mZ );
}

inline Quat & Quat::setX( float _x )
{
    mX = _x;
    return *this;
}

inline float Quat::getX( ) const
{
    return mX;
}

inline Quat & Quat::setY( float _y )
{
    mY = _y;
    return *this;
}

inline float Quat::getY( ) const
{
    return mY;
}

inline Quat & Quat::setZ( float _z )
{
    mZ = _z;
    return *this;
}

inline float Quat::getZ( ) const
{
    return mZ;
}

inline Quat & Quat::setW( float _w )
{
    mW = _w;
    return *this;
}

inline float Quat::getW( ) const
{
    return mW;
}

inline Quat & Quat::setElem( int idx, float value )
{
    *(&mX + idx) = value;
    return *this;
}

inline float Quat::getElem( int idx ) const
{
    return *(&mX + idx);
}

inline float & Quat::operator []( int idx )
{
    return *(&mX + idx);
}

inline float Quat::operator []( int idx ) const
{
    return *(&mX + idx);
}

inline const Quat Quat::operator +( const Quat & quat ) const
{
    return Quat(
        ( mX + quat.mX ),
        ( mY + quat.mY ),
        ( mZ + quat.mZ ),
        ( mW + quat.mW )
    );
}

inline const Quat Quat::operator -( const Quat & quat ) const
{
    return Quat(
        ( mX - quat.mX ),
        ( mY - quat.mY ),
        ( mZ - quat.mZ ),
        ( mW - quat.mW )
    );
}

inline const Quat Quat::operator *( float scalar ) const
{
    return Quat(
        ( mX * scalar ),
        ( mY * scalar ),
        ( mZ * scalar ),
        ( mW * scalar )
    );
}

inline Quat & Quat::operator +=( const Quat & quat )
{
    *this = *this + quat;
    return *this;
}

inline Quat & Quat::operator -=( const Quat & quat )
{
    *this = *this - quat;
    return *this;
}

inline Quat & Quat::operator *=( float scalar )
{
    *this = *this * scalar;
    return *this;
}

inline const Quat Quat::operator /( float scalar ) const
{
    return Quat(
        ( mX / scalar ),
        ( mY / scalar ),
        ( mZ / scalar ),
        ( mW / scalar )
    );
}

inline Quat & Quat::operator /=( float scalar )
{
    *this = *this / scalar;
    return *this;
}

inline const Quat Quat::operator -( ) const
{
    return Quat(
        -mX,
        -mY,
        -mZ,
        -mW
    );
}

inline const Quat operator *( float scalar, const Quat & quat )
{
    return quat * scalar;
}

inline float dot( const Quat & quat0, const Quat & quat1 )
{
    float result;
    result = ( quat0.getX() * quat1.getX() );
    result = ( result + ( quat0.getY() * quat1.getY() ) );
    result = ( result + ( quat0.getZ() * quat1.getZ() ) );
    result = ( result + ( quat0.getW() * quat1.getW() ) );
    return result;
}

inline float norm( const Quat & quat )
{
    float result;
    result = ( quat.getX() * quat.getX() );
    result = ( result + ( quat.getY() * quat.getY() ) );
    result = ( result + ( quat.getZ() * quat.getZ() ) );
    result = ( result + ( quat.getW() * quat.getW() ) );
    return result;
}

inline float length( const Quat & quat )
{
    return ::sqrtf( norm( quat ) );
}

inline const Quat normalize( const Quat & quat )
{
    float lenSqr, lenInv;
    lenSqr = norm( quat );
    lenInv = ( 1.0f / sqrtf( lenSqr ) );
    return Quat(
        ( quat.getX() * lenInv ),
        ( quat.getY() * lenInv ),
        ( quat.getZ() * lenInv ),
        ( quat.getW() * lenInv )
    );
}

inline const Quat Quat::rotation( const Vector3 & unitVec0, const Vector3 & unitVec1 )
{
    float cosHalfAngleX2, recipCosHalfAngleX2;
    cosHalfAngleX2 = sqrtf( ( 2.0f * ( 1.0f + dot( unitVec0, unitVec1 ) ) ) );
    recipCosHalfAngleX2 = ( 1.0f / cosHalfAngleX2 );
    return Quat( ( cross( unitVec0, unitVec1 ) * recipCosHalfAngleX2 ), ( cosHalfAngleX2 * 0.5f ) );
}

inline const Quat Quat::rotation( float radians, const Vector3 & unitVec )
{
    float s, c, angle;
    angle = ( radians * 0.5f );
    s = sinf( angle );
    c = cosf( angle );
    return Quat( ( unitVec * s ), c );
}

inline const Quat Quat::rotationX( float radians )
{
    float s, c, angle;
    angle = ( radians * 0.5f );
    s = sinf( angle );
    c = cosf( angle );
    return Quat( s, 0.0f, 0.0f, c );
}

inline const Quat Quat::rotationY( float radians )
{
    float s, c, angle;
    angle = ( radians * 0.5f );
    s = sinf( angle );
    c = cosf( angle );
    return Quat( 0.0f, s, 0.0f, c );
}

inline const Quat Quat::rotationZ( float radians )
{
    float s, c, angle;
    angle = ( radians * 0.5f );
    s = sinf( angle );
    c = cosf( angle );
    return Quat( 0.0f, 0.0f, s, c );
}

inline const Quat Quat::operator *( const Quat & quat ) const
{
    return Quat(
        ( ( ( ( mW * quat.mX ) + ( mX * quat.mW ) ) + ( mY * quat.mZ ) ) - ( mZ * quat.mY ) ),
        ( ( ( ( mW * quat.mY ) + ( mY * quat.mW ) ) + ( mZ * quat.mX ) ) - ( mX * quat.mZ ) ),
        ( ( ( ( mW * quat.mZ ) + ( mZ * quat.mW ) ) + ( mX * quat.mY ) ) - ( mY * quat.mX ) ),
        ( ( ( ( mW * quat.mW ) - ( mX * quat.mX ) ) - ( mY * quat.mY ) ) - ( mZ * quat.mZ ) )
    );
}

inline Quat & Quat::operator *=( const Quat & quat )
{
    *this = *this * quat;
    return *this;
}

inline const Vector3 rotate( const Quat & quat, const Vector3 & vec )
{
    float tmpX, tmpY, tmpZ, tmpW;
    tmpX = ( ( ( quat.getW() * vec.getX() ) + ( quat.getY() * vec.getZ() ) ) - ( quat.getZ() * vec.getY() ) );
    tmpY = ( ( ( quat.getW() * vec.getY() ) + ( quat.getZ() * vec.getX() ) ) - ( quat.getX() * vec.getZ() ) );
    tmpZ = ( ( ( quat.getW() * vec.getZ() ) + ( quat.getX() * vec.getY() ) ) - ( quat.getY() * vec.getX() ) );
    tmpW = ( ( ( quat.getX() * vec.getX() ) + ( quat.getY() * vec.getY() ) ) + ( quat.getZ() * vec.getZ() ) );
    return Vector3(
        ( ( ( ( tmpW * quat.getX() ) + ( tmpX * quat.getW() ) ) - ( tmpY * quat.getZ() ) ) + ( tmpZ * quat.getY() ) ),
        ( ( ( ( tmpW * quat.getY() ) + ( tmpY * quat.getW() ) ) - ( tmpZ * quat.getX() ) ) + ( tmpX * quat.getZ() ) ),
        ( ( ( ( tmpW * quat.getZ() ) + ( tmpZ * quat.getW() ) ) - ( tmpX * quat.getY() ) ) + ( tmpY * quat.getX() ) )
    );
}

inline const Quat conj( const Quat & quat )
{
    return Quat( -quat.getX(), -quat.getY(), -quat.getZ(), quat.getW() );
}

inline const Quat select( const Quat & quat0, const Quat & quat1, bool select1 )
{
    return Quat(
        ( select1 )? quat1.getX() : quat0.getX(),
        ( select1 )? quat1.getY() : quat0.getY(),
        ( select1 )? quat1.getZ() : quat0.getZ(),
        ( select1 )? quat1.getW() : quat0.getW()
    );
}

#ifdef _VECTORMATH_DEBUG

inline void print( const Quat & quat )
{
    printf( "( %f %f %f %f )\n", quat.getX(), quat.getY(), quat.getZ(), quat.getW() );
}

inline void print( const Quat & quat, const char * name )
{
    printf( "%s: ( %f %f %f %f )\n", name, quat.getX(), quat.getY(), quat.getZ(), quat.getW() );
}

#endif

} // namespace Aos
} // namespace Vectormath

#endif
