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

#ifndef _VECTORMATH_AOS_CPP_H
#define _VECTORMATH_AOS_CPP_H

#include <math.h>

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
    float mX;
    float mY;
    float mZ;
#ifndef __GNUC__
    float d;
#endif

public:
    // Default constructor; does no initialization
    // 
    inline Vector3( ) { };

    // Copy a 3-D vector
    // 
    inline Vector3( const Vector3 & vec );

    // Construct a 3-D vector from x, y, and z elements
    // 
    inline Vector3( float x, float y, float z );

    // Copy elements from a 3-D point into a 3-D vector
    // 
    explicit inline Vector3( const Point3 & pnt );

    // Set all elements of a 3-D vector to the same scalar value
    // 
    explicit inline Vector3( float scalar );

    // Assign one 3-D vector to another
    // 
    inline Vector3 & operator =( const Vector3 & vec );

    // Set the x element of a 3-D vector
    // 
    inline Vector3 & setX( float x );

    // Set the y element of a 3-D vector
    // 
    inline Vector3 & setY( float y );

    // Set the z element of a 3-D vector
    // 
    inline Vector3 & setZ( float z );

    // Get the x element of a 3-D vector
    // 
    inline float getX( ) const;

    // Get the y element of a 3-D vector
    // 
    inline float getY( ) const;

    // Get the z element of a 3-D vector
    // 
    inline float getZ( ) const;

    // Set an x, y, or z element of a 3-D vector by index
    // 
    inline Vector3 & setElem( int idx, float value );

    // Get an x, y, or z element of a 3-D vector by index
    // 
    inline float getElem( int idx ) const;

    // Subscripting operator to set or get an element
    // 
    inline float & operator []( int idx );

    // Subscripting operator to get an element
    // 
    inline float operator []( int idx ) const;

    // Add two 3-D vectors
    // 
    inline const Vector3 operator +( const Vector3 & vec ) const;

    // Subtract a 3-D vector from another 3-D vector
    // 
    inline const Vector3 operator -( const Vector3 & vec ) const;

    // Add a 3-D vector to a 3-D point
    // 
    inline const Point3 operator +( const Point3 & pnt ) const;

    // Multiply a 3-D vector by a scalar
    // 
    inline const Vector3 operator *( float scalar ) const;

    // Divide a 3-D vector by a scalar
    // 
    inline const Vector3 operator /( float scalar ) const;

    // Perform compound assignment and addition with a 3-D vector
    // 
    inline Vector3 & operator +=( const Vector3 & vec );

    // Perform compound assignment and subtraction by a 3-D vector
    // 
    inline Vector3 & operator -=( const Vector3 & vec );

    // Perform compound assignment and multiplication by a scalar
    // 
    inline Vector3 & operator *=( float scalar );

    // Perform compound assignment and division by a scalar
    // 
    inline Vector3 & operator /=( float scalar );

    // Negate all elements of a 3-D vector
    // 
    inline const Vector3 operator -( ) const;

    // Construct x axis
    // 
    static inline const Vector3 xAxis( );

    // Construct y axis
    // 
    static inline const Vector3 yAxis( );

    // Construct z axis
    // 
    static inline const Vector3 zAxis( );

}
#ifdef __GNUC__
__attribute__ ((aligned(16)))
#endif
;

// Multiply a 3-D vector by a scalar
// 
inline const Vector3 operator *( float scalar, const Vector3 & vec );

// Multiply two 3-D vectors per element
// 
inline const Vector3 mulPerElem( const Vector3 & vec0, const Vector3 & vec1 );

// Divide two 3-D vectors per element
// NOTE: 
// Floating-point behavior matches standard library function divf4.
// 
inline const Vector3 divPerElem( const Vector3 & vec0, const Vector3 & vec1 );

// Compute the reciprocal of a 3-D vector per element
// NOTE: 
// Floating-point behavior matches standard library function recipf4.
// 
inline const Vector3 recipPerElem( const Vector3 & vec );

// Compute the square root of a 3-D vector per element
// NOTE: 
// Floating-point behavior matches standard library function sqrtf4.
// 
inline const Vector3 sqrtPerElem( const Vector3 & vec );

// Compute the reciprocal square root of a 3-D vector per element
// NOTE: 
// Floating-point behavior matches standard library function rsqrtf4.
// 
inline const Vector3 rsqrtPerElem( const Vector3 & vec );

// Compute the absolute value of a 3-D vector per element
// 
inline const Vector3 absPerElem( const Vector3 & vec );

// Copy sign from one 3-D vector to another, per element
// 
inline const Vector3 copySignPerElem( const Vector3 & vec0, const Vector3 & vec1 );

// Maximum of two 3-D vectors per element
// 
inline const Vector3 maxPerElem( const Vector3 & vec0, const Vector3 & vec1 );

// Minimum of two 3-D vectors per element
// 
inline const Vector3 minPerElem( const Vector3 & vec0, const Vector3 & vec1 );

// Maximum element of a 3-D vector
// 
inline float maxElem( const Vector3 & vec );

// Minimum element of a 3-D vector
// 
inline float minElem( const Vector3 & vec );

// Compute the sum of all elements of a 3-D vector
// 
inline float sum( const Vector3 & vec );

// Compute the dot product of two 3-D vectors
// 
inline float dot( const Vector3 & vec0, const Vector3 & vec1 );

// Compute the square of the length of a 3-D vector
// 
inline float lengthSqr( const Vector3 & vec );

// Compute the length of a 3-D vector
// 
inline float length( const Vector3 & vec );

// Normalize a 3-D vector
// NOTE: 
// The result is unpredictable when all elements of vec are at or near zero.
// 
inline const Vector3 normalize( const Vector3 & vec );

// Compute cross product of two 3-D vectors
// 
inline const Vector3 cross( const Vector3 & vec0, const Vector3 & vec1 );

// Outer product of two 3-D vectors
// 
inline const Matrix3 outer( const Vector3 & vec0, const Vector3 & vec1 );

// Pre-multiply a row vector by a 3x3 matrix
// 
inline const Vector3 rowMul( const Vector3 & vec, const Matrix3 & mat );

// Cross-product matrix of a 3-D vector
// 
inline const Matrix3 crossMatrix( const Vector3 & vec );

// Create cross-product matrix and multiply
// NOTE: 
// Faster than separately creating a cross-product matrix and multiplying.
// 
inline const Matrix3 crossMatrixMul( const Vector3 & vec, const Matrix3 & mat );

// Linear interpolation between two 3-D vectors
// NOTE: 
// Does not clamp t between 0 and 1.
// 
inline const Vector3 lerp( float t, const Vector3 & vec0, const Vector3 & vec1 );

// Spherical linear interpolation between two 3-D vectors
// NOTE: 
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
// 
inline const Vector3 slerp( float t, const Vector3 & unitVec0, const Vector3 & unitVec1 );

// Conditionally select between two 3-D vectors
// 
inline const Vector3 select( const Vector3 & vec0, const Vector3 & vec1, bool select1 );

// Load x, y, and z elements from the first three words of a float array.
// 
// 
inline void loadXYZ( Vector3 & vec, const float * fptr );

// Store x, y, and z elements of a 3-D vector in the first three words of a float array.
// Memory area of previous 16 bytes and next 32 bytes from fptr might be accessed
// 
inline void storeXYZ( const Vector3 & vec, float * fptr );

// Load three-half-floats as a 3-D vector
// NOTE: 
// This transformation does not support either denormalized numbers or NaNs.
// 
inline void loadHalfFloats( Vector3 & vec, const unsigned short * hfptr );

// Store a 3-D vector as half-floats. Memory area of previous 16 bytes and next 32 bytes from <code><i>hfptr</i></code> might be accessed.
// NOTE: 
// This transformation does not support either denormalized numbers or NaNs. Memory area of previous 16 bytes and next 32 bytes from hfptr might be accessed.
// 
inline void storeHalfFloats( const Vector3 & vec, unsigned short * hfptr );

#ifdef _VECTORMATH_DEBUG

// Print a 3-D vector
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Vector3 & vec );

// Print a 3-D vector and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Vector3 & vec, const char * name );

#endif

// A 4-D vector in array-of-structures format
//
class Vector4
{
    float mX;
    float mY;
    float mZ;
    float mW;

public:
    // Default constructor; does no initialization
    // 
    inline Vector4( ) { };

    // Copy a 4-D vector
    // 
    inline Vector4( const Vector4 & vec );

    // Construct a 4-D vector from x, y, z, and w elements
    // 
    inline Vector4( float x, float y, float z, float w );

    // Construct a 4-D vector from a 3-D vector and a scalar
    // 
    inline Vector4( const Vector3 & xyz, float w );

    // Copy x, y, and z from a 3-D vector into a 4-D vector, and set w to 0
    // 
    explicit inline Vector4( const Vector3 & vec );

    // Copy x, y, and z from a 3-D point into a 4-D vector, and set w to 1
    // 
    explicit inline Vector4( const Point3 & pnt );

    // Copy elements from a quaternion into a 4-D vector
    // 
    explicit inline Vector4( const Quat & quat );

    // Set all elements of a 4-D vector to the same scalar value
    // 
    explicit inline Vector4( float scalar );

    // Assign one 4-D vector to another
    // 
    inline Vector4 & operator =( const Vector4 & vec );

    // Set the x, y, and z elements of a 4-D vector
    // NOTE: 
    // This function does not change the w element.
    // 
    inline Vector4 & setXYZ( const Vector3 & vec );

    // Get the x, y, and z elements of a 4-D vector
    // 
    inline const Vector3 getXYZ( ) const;

    // Set the x element of a 4-D vector
    // 
    inline Vector4 & setX( float x );

    // Set the y element of a 4-D vector
    // 
    inline Vector4 & setY( float y );

    // Set the z element of a 4-D vector
    // 
    inline Vector4 & setZ( float z );

    // Set the w element of a 4-D vector
    // 
    inline Vector4 & setW( float w );

    // Get the x element of a 4-D vector
    // 
    inline float getX( ) const;

    // Get the y element of a 4-D vector
    // 
    inline float getY( ) const;

    // Get the z element of a 4-D vector
    // 
    inline float getZ( ) const;

    // Get the w element of a 4-D vector
    // 
    inline float getW( ) const;

    // Set an x, y, z, or w element of a 4-D vector by index
    // 
    inline Vector4 & setElem( int idx, float value );

    // Get an x, y, z, or w element of a 4-D vector by index
    // 
    inline float getElem( int idx ) const;

    // Subscripting operator to set or get an element
    // 
    inline float & operator []( int idx );

    // Subscripting operator to get an element
    // 
    inline float operator []( int idx ) const;

    // Add two 4-D vectors
    // 
    inline const Vector4 operator +( const Vector4 & vec ) const;

    // Subtract a 4-D vector from another 4-D vector
    // 
    inline const Vector4 operator -( const Vector4 & vec ) const;

    // Multiply a 4-D vector by a scalar
    // 
    inline const Vector4 operator *( float scalar ) const;

    // Divide a 4-D vector by a scalar
    // 
    inline const Vector4 operator /( float scalar ) const;

    // Perform compound assignment and addition with a 4-D vector
    // 
    inline Vector4 & operator +=( const Vector4 & vec );

    // Perform compound assignment and subtraction by a 4-D vector
    // 
    inline Vector4 & operator -=( const Vector4 & vec );

    // Perform compound assignment and multiplication by a scalar
    // 
    inline Vector4 & operator *=( float scalar );

    // Perform compound assignment and division by a scalar
    // 
    inline Vector4 & operator /=( float scalar );

    // Negate all elements of a 4-D vector
    // 
    inline const Vector4 operator -( ) const;

    // Construct x axis
    // 
    static inline const Vector4 xAxis( );

    // Construct y axis
    // 
    static inline const Vector4 yAxis( );

    // Construct z axis
    // 
    static inline const Vector4 zAxis( );

    // Construct w axis
    // 
    static inline const Vector4 wAxis( );

}
#ifdef __GNUC__
__attribute__ ((aligned(16)))
#endif
;

// Multiply a 4-D vector by a scalar
// 
inline const Vector4 operator *( float scalar, const Vector4 & vec );

// Multiply two 4-D vectors per element
// 
inline const Vector4 mulPerElem( const Vector4 & vec0, const Vector4 & vec1 );

// Divide two 4-D vectors per element
// NOTE: 
// Floating-point behavior matches standard library function divf4.
// 
inline const Vector4 divPerElem( const Vector4 & vec0, const Vector4 & vec1 );

// Compute the reciprocal of a 4-D vector per element
// NOTE: 
// Floating-point behavior matches standard library function recipf4.
// 
inline const Vector4 recipPerElem( const Vector4 & vec );

// Compute the square root of a 4-D vector per element
// NOTE: 
// Floating-point behavior matches standard library function sqrtf4.
// 
inline const Vector4 sqrtPerElem( const Vector4 & vec );

// Compute the reciprocal square root of a 4-D vector per element
// NOTE: 
// Floating-point behavior matches standard library function rsqrtf4.
// 
inline const Vector4 rsqrtPerElem( const Vector4 & vec );

// Compute the absolute value of a 4-D vector per element
// 
inline const Vector4 absPerElem( const Vector4 & vec );

// Copy sign from one 4-D vector to another, per element
// 
inline const Vector4 copySignPerElem( const Vector4 & vec0, const Vector4 & vec1 );

// Maximum of two 4-D vectors per element
// 
inline const Vector4 maxPerElem( const Vector4 & vec0, const Vector4 & vec1 );

// Minimum of two 4-D vectors per element
// 
inline const Vector4 minPerElem( const Vector4 & vec0, const Vector4 & vec1 );

// Maximum element of a 4-D vector
// 
inline float maxElem( const Vector4 & vec );

// Minimum element of a 4-D vector
// 
inline float minElem( const Vector4 & vec );

// Compute the sum of all elements of a 4-D vector
// 
inline float sum( const Vector4 & vec );

// Compute the dot product of two 4-D vectors
// 
inline float dot( const Vector4 & vec0, const Vector4 & vec1 );

// Compute the square of the length of a 4-D vector
// 
inline float lengthSqr( const Vector4 & vec );

// Compute the length of a 4-D vector
// 
inline float length( const Vector4 & vec );

// Normalize a 4-D vector
// NOTE: 
// The result is unpredictable when all elements of vec are at or near zero.
// 
inline const Vector4 normalize( const Vector4 & vec );

// Outer product of two 4-D vectors
// 
inline const Matrix4 outer( const Vector4 & vec0, const Vector4 & vec1 );

// Linear interpolation between two 4-D vectors
// NOTE: 
// Does not clamp t between 0 and 1.
// 
inline const Vector4 lerp( float t, const Vector4 & vec0, const Vector4 & vec1 );

// Spherical linear interpolation between two 4-D vectors
// NOTE: 
// The result is unpredictable if the vectors point in opposite directions.
// Does not clamp t between 0 and 1.
// 
inline const Vector4 slerp( float t, const Vector4 & unitVec0, const Vector4 & unitVec1 );

// Conditionally select between two 4-D vectors
// 
inline const Vector4 select( const Vector4 & vec0, const Vector4 & vec1, bool select1 );

// Load x, y, z, and w elements from the first four words of a float array.
// 
// 
inline void loadXYZW( Vector4 & vec, const float * fptr );

// Store x, y, z, and w elements of a 4-D vector in the first four words of a float array.
// Memory area of previous 16 bytes and next 32 bytes from fptr might be accessed
// 
inline void storeXYZW( const Vector4 & vec, float * fptr );

// Load four-half-floats as a 4-D vector
// NOTE: 
// This transformation does not support either denormalized numbers or NaNs.
// 
inline void loadHalfFloats( Vector4 & vec, const unsigned short * hfptr );

// Store a 4-D vector as half-floats. Memory area of previous 16 bytes and next 32 bytes from <code><i>hfptr</i></code> might be accessed.
// NOTE: 
// This transformation does not support either denormalized numbers or NaNs. Memory area of previous 16 bytes and next 32 bytes from hfptr might be accessed.
// 
inline void storeHalfFloats( const Vector4 & vec, unsigned short * hfptr );

#ifdef _VECTORMATH_DEBUG

// Print a 4-D vector
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Vector4 & vec );

// Print a 4-D vector and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Vector4 & vec, const char * name );

#endif

// A 3-D point in array-of-structures format
//
class Point3
{
    float mX;
    float mY;
    float mZ;
#ifndef __GNUC__
    float d;
#endif

public:
    // Default constructor; does no initialization
    // 
    inline Point3( ) { };

    // Copy a 3-D point
    // 
    inline Point3( const Point3 & pnt );

    // Construct a 3-D point from x, y, and z elements
    // 
    inline Point3( float x, float y, float z );

    // Copy elements from a 3-D vector into a 3-D point
    // 
    explicit inline Point3( const Vector3 & vec );

    // Set all elements of a 3-D point to the same scalar value
    // 
    explicit inline Point3( float scalar );

    // Assign one 3-D point to another
    // 
    inline Point3 & operator =( const Point3 & pnt );

    // Set the x element of a 3-D point
    // 
    inline Point3 & setX( float x );

    // Set the y element of a 3-D point
    // 
    inline Point3 & setY( float y );

    // Set the z element of a 3-D point
    // 
    inline Point3 & setZ( float z );

    // Get the x element of a 3-D point
    // 
    inline float getX( ) const;

    // Get the y element of a 3-D point
    // 
    inline float getY( ) const;

    // Get the z element of a 3-D point
    // 
    inline float getZ( ) const;

    // Set an x, y, or z element of a 3-D point by index
    // 
    inline Point3 & setElem( int idx, float value );

    // Get an x, y, or z element of a 3-D point by index
    // 
    inline float getElem( int idx ) const;

    // Subscripting operator to set or get an element
    // 
    inline float & operator []( int idx );

    // Subscripting operator to get an element
    // 
    inline float operator []( int idx ) const;

    // Subtract a 3-D point from another 3-D point
    // 
    inline const Vector3 operator -( const Point3 & pnt ) const;

    // Add a 3-D point to a 3-D vector
    // 
    inline const Point3 operator +( const Vector3 & vec ) const;

    // Subtract a 3-D vector from a 3-D point
    // 
    inline const Point3 operator -( const Vector3 & vec ) const;

    // Perform compound assignment and addition with a 3-D vector
    // 
    inline Point3 & operator +=( const Vector3 & vec );

    // Perform compound assignment and subtraction by a 3-D vector
    // 
    inline Point3 & operator -=( const Vector3 & vec );

}
#ifdef __GNUC__
__attribute__ ((aligned(16)))
#endif
;

// Multiply two 3-D points per element
// 
inline const Point3 mulPerElem( const Point3 & pnt0, const Point3 & pnt1 );

// Divide two 3-D points per element
// NOTE: 
// Floating-point behavior matches standard library function divf4.
// 
inline const Point3 divPerElem( const Point3 & pnt0, const Point3 & pnt1 );

// Compute the reciprocal of a 3-D point per element
// NOTE: 
// Floating-point behavior matches standard library function recipf4.
// 
inline const Point3 recipPerElem( const Point3 & pnt );

// Compute the square root of a 3-D point per element
// NOTE: 
// Floating-point behavior matches standard library function sqrtf4.
// 
inline const Point3 sqrtPerElem( const Point3 & pnt );

// Compute the reciprocal square root of a 3-D point per element
// NOTE: 
// Floating-point behavior matches standard library function rsqrtf4.
// 
inline const Point3 rsqrtPerElem( const Point3 & pnt );

// Compute the absolute value of a 3-D point per element
// 
inline const Point3 absPerElem( const Point3 & pnt );

// Copy sign from one 3-D point to another, per element
// 
inline const Point3 copySignPerElem( const Point3 & pnt0, const Point3 & pnt1 );

// Maximum of two 3-D points per element
// 
inline const Point3 maxPerElem( const Point3 & pnt0, const Point3 & pnt1 );

// Minimum of two 3-D points per element
// 
inline const Point3 minPerElem( const Point3 & pnt0, const Point3 & pnt1 );

// Maximum element of a 3-D point
// 
inline float maxElem( const Point3 & pnt );

// Minimum element of a 3-D point
// 
inline float minElem( const Point3 & pnt );

// Compute the sum of all elements of a 3-D point
// 
inline float sum( const Point3 & pnt );

// Apply uniform scale to a 3-D point
// 
inline const Point3 scale( const Point3 & pnt, float scaleVal );

// Apply non-uniform scale to a 3-D point
// 
inline const Point3 scale( const Point3 & pnt, const Vector3 & scaleVec );

// Scalar projection of a 3-D point on a unit-length 3-D vector
// 
inline float projection( const Point3 & pnt, const Vector3 & unitVec );

// Compute the square of the distance of a 3-D point from the coordinate-system origin
// 
inline float distSqrFromOrigin( const Point3 & pnt );

// Compute the distance of a 3-D point from the coordinate-system origin
// 
inline float distFromOrigin( const Point3 & pnt );

// Compute the square of the distance between two 3-D points
// 
inline float distSqr( const Point3 & pnt0, const Point3 & pnt1 );

// Compute the distance between two 3-D points
// 
inline float dist( const Point3 & pnt0, const Point3 & pnt1 );

// Linear interpolation between two 3-D points
// NOTE: 
// Does not clamp t between 0 and 1.
// 
inline const Point3 lerp( float t, const Point3 & pnt0, const Point3 & pnt1 );

// Conditionally select between two 3-D points
// 
inline const Point3 select( const Point3 & pnt0, const Point3 & pnt1, bool select1 );

// Load x, y, and z elements from the first three words of a float array.
// 
// 
inline void loadXYZ( Point3 & pnt, const float * fptr );

// Store x, y, and z elements of a 3-D point in the first three words of a float array.
// Memory area of previous 16 bytes and next 32 bytes from fptr might be accessed
// 
inline void storeXYZ( const Point3 & pnt, float * fptr );

// Load three-half-floats as a 3-D point
// NOTE: 
// This transformation does not support either denormalized numbers or NaNs.
// 
inline void loadHalfFloats( Point3 & pnt, const unsigned short * hfptr );

// Store a 3-D point as half-floats. Memory area of previous 16 bytes and next 32 bytes from <code><i>hfptr</i></code> might be accessed.
// NOTE: 
// This transformation does not support either denormalized numbers or NaNs. Memory area of previous 16 bytes and next 32 bytes from hfptr might be accessed.
// 
inline void storeHalfFloats( const Point3 & pnt, unsigned short * hfptr );

#ifdef _VECTORMATH_DEBUG

// Print a 3-D point
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Point3 & pnt );

// Print a 3-D point and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Point3 & pnt, const char * name );

#endif

// A quaternion in array-of-structures format
//
class Quat
{
    float mX;
    float mY;
    float mZ;
    float mW;

public:
    // Default constructor; does no initialization
    // 
    inline Quat( ) { };

    // Copy a quaternion
    // 
    inline Quat( const Quat & quat );

    // Construct a quaternion from x, y, z, and w elements
    // 
    inline Quat( float x, float y, float z, float w );

    // Construct a quaternion from a 3-D vector and a scalar
    // 
    inline Quat( const Vector3 & xyz, float w );

    // Copy elements from a 4-D vector into a quaternion
    // 
    explicit inline Quat( const Vector4 & vec );

    // Convert a rotation matrix to a unit-length quaternion
    // 
    explicit inline Quat( const Matrix3 & rotMat );

    // Set all elements of a quaternion to the same scalar value
    // 
    explicit inline Quat( float scalar );

    // Assign one quaternion to another
    // 
    inline Quat & operator =( const Quat & quat );

    // Set the x, y, and z elements of a quaternion
    // NOTE: 
    // This function does not change the w element.
    // 
    inline Quat & setXYZ( const Vector3 & vec );

    // Get the x, y, and z elements of a quaternion
    // 
    inline const Vector3 getXYZ( ) const;

    // Set the x element of a quaternion
    // 
    inline Quat & setX( float x );

    // Set the y element of a quaternion
    // 
    inline Quat & setY( float y );

    // Set the z element of a quaternion
    // 
    inline Quat & setZ( float z );

    // Set the w element of a quaternion
    // 
    inline Quat & setW( float w );

    // Get the x element of a quaternion
    // 
    inline float getX( ) const;

    // Get the y element of a quaternion
    // 
    inline float getY( ) const;

    // Get the z element of a quaternion
    // 
    inline float getZ( ) const;

    // Get the w element of a quaternion
    // 
    inline float getW( ) const;

    // Set an x, y, z, or w element of a quaternion by index
    // 
    inline Quat & setElem( int idx, float value );

    // Get an x, y, z, or w element of a quaternion by index
    // 
    inline float getElem( int idx ) const;

    // Subscripting operator to set or get an element
    // 
    inline float & operator []( int idx );

    // Subscripting operator to get an element
    // 
    inline float operator []( int idx ) const;

    // Add two quaternions
    // 
    inline const Quat operator +( const Quat & quat ) const;

    // Subtract a quaternion from another quaternion
    // 
    inline const Quat operator -( const Quat & quat ) const;

    // Multiply two quaternions
    // 
    inline const Quat operator *( const Quat & quat ) const;

    // Multiply a quaternion by a scalar
    // 
    inline const Quat operator *( float scalar ) const;

    // Divide a quaternion by a scalar
    // 
    inline const Quat operator /( float scalar ) const;

    // Perform compound assignment and addition with a quaternion
    // 
    inline Quat & operator +=( const Quat & quat );

    // Perform compound assignment and subtraction by a quaternion
    // 
    inline Quat & operator -=( const Quat & quat );

    // Perform compound assignment and multiplication by a quaternion
    // 
    inline Quat & operator *=( const Quat & quat );

    // Perform compound assignment and multiplication by a scalar
    // 
    inline Quat & operator *=( float scalar );

    // Perform compound assignment and division by a scalar
    // 
    inline Quat & operator /=( float scalar );

    // Negate all elements of a quaternion
    // 
    inline const Quat operator -( ) const;

    // Construct an identity quaternion
    // 
    static inline const Quat identity( );

    // Construct a quaternion to rotate between two unit-length 3-D vectors
    // NOTE: 
    // The result is unpredictable if unitVec0 and unitVec1 point in opposite directions.
    // 
    static inline const Quat rotation( const Vector3 & unitVec0, const Vector3 & unitVec1 );

    // Construct a quaternion to rotate around a unit-length 3-D vector
    // 
    static inline const Quat rotation( float radians, const Vector3 & unitVec );

    // Construct a quaternion to rotate around the x axis
    // 
    static inline const Quat rotationX( float radians );

    // Construct a quaternion to rotate around the y axis
    // 
    static inline const Quat rotationY( float radians );

    // Construct a quaternion to rotate around the z axis
    // 
    static inline const Quat rotationZ( float radians );

}
#ifdef __GNUC__
__attribute__ ((aligned(16)))
#endif
;

// Multiply a quaternion by a scalar
// 
inline const Quat operator *( float scalar, const Quat & quat );

// Compute the conjugate of a quaternion
// 
inline const Quat conj( const Quat & quat );

// Use a unit-length quaternion to rotate a 3-D vector
// 
inline const Vector3 rotate( const Quat & unitQuat, const Vector3 & vec );

// Compute the dot product of two quaternions
// 
inline float dot( const Quat & quat0, const Quat & quat1 );

// Compute the norm of a quaternion
// 
inline float norm( const Quat & quat );

// Compute the length of a quaternion
// 
inline float length( const Quat & quat );

// Normalize a quaternion
// NOTE: 
// The result is unpredictable when all elements of quat are at or near zero.
// 
inline const Quat normalize( const Quat & quat );

// Linear interpolation between two quaternions
// NOTE: 
// Does not clamp t between 0 and 1.
// 
inline const Quat lerp( float t, const Quat & quat0, const Quat & quat1 );

// Spherical linear interpolation between two quaternions
// NOTE: 
// Interpolates along the shortest path between orientations.
// Does not clamp t between 0 and 1.
// 
inline const Quat slerp( float t, const Quat & unitQuat0, const Quat & unitQuat1 );

// Spherical quadrangle interpolation
// 
inline const Quat squad( float t, const Quat & unitQuat0, const Quat & unitQuat1, const Quat & unitQuat2, const Quat & unitQuat3 );

// Conditionally select between two quaternions
// 
inline const Quat select( const Quat & quat0, const Quat & quat1, bool select1 );

// Load x, y, z, and w elements from the first four words of a float array.
// 
// 
inline void loadXYZW( Quat & quat, const float * fptr );

// Store x, y, z, and w elements of a quaternion in the first four words of a float array.
// Memory area of previous 16 bytes and next 32 bytes from fptr might be accessed
// 
inline void storeXYZW( const Quat & quat, float * fptr );

#ifdef _VECTORMATH_DEBUG

// Print a quaternion
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Quat & quat );

// Print a quaternion and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Quat & quat, const char * name );

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
    inline Matrix3( ) { };

    // Copy a 3x3 matrix
    // 
    inline Matrix3( const Matrix3 & mat );

    // Construct a 3x3 matrix containing the specified columns
    // 
    inline Matrix3( const Vector3 & col0, const Vector3 & col1, const Vector3 & col2 );

    // Construct a 3x3 rotation matrix from a unit-length quaternion
    // 
    explicit inline Matrix3( const Quat & unitQuat );

    // Set all elements of a 3x3 matrix to the same scalar value
    // 
    explicit inline Matrix3( float scalar );

    // Assign one 3x3 matrix to another
    // 
    inline Matrix3 & operator =( const Matrix3 & mat );

    // Set column 0 of a 3x3 matrix
    // 
    inline Matrix3 & setCol0( const Vector3 & col0 );

    // Set column 1 of a 3x3 matrix
    // 
    inline Matrix3 & setCol1( const Vector3 & col1 );

    // Set column 2 of a 3x3 matrix
    // 
    inline Matrix3 & setCol2( const Vector3 & col2 );

    // Get column 0 of a 3x3 matrix
    // 
    inline const Vector3 getCol0( ) const;

    // Get column 1 of a 3x3 matrix
    // 
    inline const Vector3 getCol1( ) const;

    // Get column 2 of a 3x3 matrix
    // 
    inline const Vector3 getCol2( ) const;

    // Set the column of a 3x3 matrix referred to by the specified index
    // 
    inline Matrix3 & setCol( int col, const Vector3 & vec );

    // Set the row of a 3x3 matrix referred to by the specified index
    // 
    inline Matrix3 & setRow( int row, const Vector3 & vec );

    // Get the column of a 3x3 matrix referred to by the specified index
    // 
    inline const Vector3 getCol( int col ) const;

    // Get the row of a 3x3 matrix referred to by the specified index
    // 
    inline const Vector3 getRow( int row ) const;

    // Subscripting operator to set or get a column
    // 
    inline Vector3 & operator []( int col );

    // Subscripting operator to get a column
    // 
    inline const Vector3 operator []( int col ) const;

    // Set the element of a 3x3 matrix referred to by column and row indices
    // 
    inline Matrix3 & setElem( int col, int row, float val );

    // Get the element of a 3x3 matrix referred to by column and row indices
    // 
    inline float getElem( int col, int row ) const;

    // Add two 3x3 matrices
    // 
    inline const Matrix3 operator +( const Matrix3 & mat ) const;

    // Subtract a 3x3 matrix from another 3x3 matrix
    // 
    inline const Matrix3 operator -( const Matrix3 & mat ) const;

    // Negate all elements of a 3x3 matrix
    // 
    inline const Matrix3 operator -( ) const;

    // Multiply a 3x3 matrix by a scalar
    // 
    inline const Matrix3 operator *( float scalar ) const;

    // Multiply a 3x3 matrix by a 3-D vector
    // 
    inline const Vector3 operator *( const Vector3 & vec ) const;

    // Multiply two 3x3 matrices
    // 
    inline const Matrix3 operator *( const Matrix3 & mat ) const;

    // Perform compound assignment and addition with a 3x3 matrix
    // 
    inline Matrix3 & operator +=( const Matrix3 & mat );

    // Perform compound assignment and subtraction by a 3x3 matrix
    // 
    inline Matrix3 & operator -=( const Matrix3 & mat );

    // Perform compound assignment and multiplication by a scalar
    // 
    inline Matrix3 & operator *=( float scalar );

    // Perform compound assignment and multiplication by a 3x3 matrix
    // 
    inline Matrix3 & operator *=( const Matrix3 & mat );

    // Construct an identity 3x3 matrix
    // 
    static inline const Matrix3 identity( );

    // Construct a 3x3 matrix to rotate around the x axis
    // 
    static inline const Matrix3 rotationX( float radians );

    // Construct a 3x3 matrix to rotate around the y axis
    // 
    static inline const Matrix3 rotationY( float radians );

    // Construct a 3x3 matrix to rotate around the z axis
    // 
    static inline const Matrix3 rotationZ( float radians );

    // Construct a 3x3 matrix to rotate around the x, y, and z axes
    // 
    static inline const Matrix3 rotationZYX( const Vector3 & radiansXYZ );

    // Construct a 3x3 matrix to rotate around a unit-length 3-D vector
    // 
    static inline const Matrix3 rotation( float radians, const Vector3 & unitVec );

    // Construct a rotation matrix from a unit-length quaternion
    // 
    static inline const Matrix3 rotation( const Quat & unitQuat );

    // Construct a 3x3 matrix to perform scaling
    // 
    static inline const Matrix3 scale( const Vector3 & scaleVec );

};
// Multiply a 3x3 matrix by a scalar
// 
inline const Matrix3 operator *( float scalar, const Matrix3 & mat );

// Append (post-multiply) a scale transformation to a 3x3 matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
inline const Matrix3 appendScale( const Matrix3 & mat, const Vector3 & scaleVec );

// Prepend (pre-multiply) a scale transformation to a 3x3 matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
inline const Matrix3 prependScale( const Vector3 & scaleVec, const Matrix3 & mat );

// Multiply two 3x3 matrices per element
// 
inline const Matrix3 mulPerElem( const Matrix3 & mat0, const Matrix3 & mat1 );

// Compute the absolute value of a 3x3 matrix per element
// 
inline const Matrix3 absPerElem( const Matrix3 & mat );

// Transpose of a 3x3 matrix
// 
inline const Matrix3 transpose( const Matrix3 & mat );

// Compute the inverse of a 3x3 matrix
// NOTE: 
// Result is unpredictable when the determinant of mat is equal to or near 0.
// 
inline const Matrix3 inverse( const Matrix3 & mat );

// Determinant of a 3x3 matrix
// 
inline float determinant( const Matrix3 & mat );

// Conditionally select between two 3x3 matrices
// 
inline const Matrix3 select( const Matrix3 & mat0, const Matrix3 & mat1, bool select1 );

#ifdef _VECTORMATH_DEBUG

// Print a 3x3 matrix
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Matrix3 & mat );

// Print a 3x3 matrix and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Matrix3 & mat, const char * name );

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
    inline Matrix4( ) { };

    // Copy a 4x4 matrix
    // 
    inline Matrix4( const Matrix4 & mat );

    // Construct a 4x4 matrix containing the specified columns
    // 
    inline Matrix4( const Vector4 & col0, const Vector4 & col1, const Vector4 & col2, const Vector4 & col3 );

    // Construct a 4x4 matrix from a 3x4 transformation matrix
    // 
    explicit inline Matrix4( const Transform3 & mat );

    // Construct a 4x4 matrix from a 3x3 matrix and a 3-D vector
    // 
    inline Matrix4( const Matrix3 & mat, const Vector3 & translateVec );

    // Construct a 4x4 matrix from a unit-length quaternion and a 3-D vector
    // 
    inline Matrix4( const Quat & unitQuat, const Vector3 & translateVec );

    // Set all elements of a 4x4 matrix to the same scalar value
    // 
    explicit inline Matrix4( float scalar );

    // Assign one 4x4 matrix to another
    // 
    inline Matrix4 & operator =( const Matrix4 & mat );

    // Set the upper-left 3x3 submatrix
    // NOTE: 
    // This function does not change the bottom row elements.
    // 
    inline Matrix4 & setUpper3x3( const Matrix3 & mat3 );

    // Get the upper-left 3x3 submatrix of a 4x4 matrix
    // 
    inline const Matrix3 getUpper3x3( ) const;

    // Set translation component
    // NOTE: 
    // This function does not change the bottom row elements.
    // 
    inline Matrix4 & setTranslation( const Vector3 & translateVec );

    // Get the translation component of a 4x4 matrix
    // 
    inline const Vector3 getTranslation( ) const;

    // Set column 0 of a 4x4 matrix
    // 
    inline Matrix4 & setCol0( const Vector4 & col0 );

    // Set column 1 of a 4x4 matrix
    // 
    inline Matrix4 & setCol1( const Vector4 & col1 );

    // Set column 2 of a 4x4 matrix
    // 
    inline Matrix4 & setCol2( const Vector4 & col2 );

    // Set column 3 of a 4x4 matrix
    // 
    inline Matrix4 & setCol3( const Vector4 & col3 );

    // Get column 0 of a 4x4 matrix
    // 
    inline const Vector4 getCol0( ) const;

    // Get column 1 of a 4x4 matrix
    // 
    inline const Vector4 getCol1( ) const;

    // Get column 2 of a 4x4 matrix
    // 
    inline const Vector4 getCol2( ) const;

    // Get column 3 of a 4x4 matrix
    // 
    inline const Vector4 getCol3( ) const;

    // Set the column of a 4x4 matrix referred to by the specified index
    // 
    inline Matrix4 & setCol( int col, const Vector4 & vec );

    // Set the row of a 4x4 matrix referred to by the specified index
    // 
    inline Matrix4 & setRow( int row, const Vector4 & vec );

    // Get the column of a 4x4 matrix referred to by the specified index
    // 
    inline const Vector4 getCol( int col ) const;

    // Get the row of a 4x4 matrix referred to by the specified index
    // 
    inline const Vector4 getRow( int row ) const;

    // Subscripting operator to set or get a column
    // 
    inline Vector4 & operator []( int col );

    // Subscripting operator to get a column
    // 
    inline const Vector4 operator []( int col ) const;

    // Set the element of a 4x4 matrix referred to by column and row indices
    // 
    inline Matrix4 & setElem( int col, int row, float val );

    // Get the element of a 4x4 matrix referred to by column and row indices
    // 
    inline float getElem( int col, int row ) const;

    // Add two 4x4 matrices
    // 
    inline const Matrix4 operator +( const Matrix4 & mat ) const;

    // Subtract a 4x4 matrix from another 4x4 matrix
    // 
    inline const Matrix4 operator -( const Matrix4 & mat ) const;

    // Negate all elements of a 4x4 matrix
    // 
    inline const Matrix4 operator -( ) const;

    // Multiply a 4x4 matrix by a scalar
    // 
    inline const Matrix4 operator *( float scalar ) const;

    // Multiply a 4x4 matrix by a 4-D vector
    // 
    inline const Vector4 operator *( const Vector4 & vec ) const;

    // Multiply a 4x4 matrix by a 3-D vector
    // 
    inline const Vector4 operator *( const Vector3 & vec ) const;

    // Multiply a 4x4 matrix by a 3-D point
    // 
    inline const Vector4 operator *( const Point3 & pnt ) const;

    // Multiply two 4x4 matrices
    // 
    inline const Matrix4 operator *( const Matrix4 & mat ) const;

    // Multiply a 4x4 matrix by a 3x4 transformation matrix
    // 
    inline const Matrix4 operator *( const Transform3 & tfrm ) const;

    // Perform compound assignment and addition with a 4x4 matrix
    // 
    inline Matrix4 & operator +=( const Matrix4 & mat );

    // Perform compound assignment and subtraction by a 4x4 matrix
    // 
    inline Matrix4 & operator -=( const Matrix4 & mat );

    // Perform compound assignment and multiplication by a scalar
    // 
    inline Matrix4 & operator *=( float scalar );

    // Perform compound assignment and multiplication by a 4x4 matrix
    // 
    inline Matrix4 & operator *=( const Matrix4 & mat );

    // Perform compound assignment and multiplication by a 3x4 transformation matrix
    // 
    inline Matrix4 & operator *=( const Transform3 & tfrm );

    // Construct an identity 4x4 matrix
    // 
    static inline const Matrix4 identity( );

    // Construct a 4x4 matrix to rotate around the x axis
    // 
    static inline const Matrix4 rotationX( float radians );

    // Construct a 4x4 matrix to rotate around the y axis
    // 
    static inline const Matrix4 rotationY( float radians );

    // Construct a 4x4 matrix to rotate around the z axis
    // 
    static inline const Matrix4 rotationZ( float radians );

    // Construct a 4x4 matrix to rotate around the x, y, and z axes
    // 
    static inline const Matrix4 rotationZYX( const Vector3 & radiansXYZ );

    // Construct a 4x4 matrix to rotate around a unit-length 3-D vector
    // 
    static inline const Matrix4 rotation( float radians, const Vector3 & unitVec );

    // Construct a rotation matrix from a unit-length quaternion
    // 
    static inline const Matrix4 rotation( const Quat & unitQuat );

    // Construct a 4x4 matrix to perform scaling
    // 
    static inline const Matrix4 scale( const Vector3 & scaleVec );

    // Construct a 4x4 matrix to perform translation
    // 
    static inline const Matrix4 translation( const Vector3 & translateVec );

    // Construct viewing matrix based on eye position, position looked at, and up direction
    // 
    static inline const Matrix4 lookAt( const Point3 & eyePos, const Point3 & lookAtPos, const Vector3 & upVec );

    // Construct a perspective projection matrix
    // 
    static inline const Matrix4 perspective( float fovyRadians, float aspect, float zNear, float zFar );

    // Construct a perspective projection matrix based on frustum
    // 
    static inline const Matrix4 frustum( float left, float right, float bottom, float top, float zNear, float zFar );

    // Construct an orthographic projection matrix
    // 
    static inline const Matrix4 orthographic( float left, float right, float bottom, float top, float zNear, float zFar );

};
// Multiply a 4x4 matrix by a scalar
// 
inline const Matrix4 operator *( float scalar, const Matrix4 & mat );

// Append (post-multiply) a scale transformation to a 4x4 matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
inline const Matrix4 appendScale( const Matrix4 & mat, const Vector3 & scaleVec );

// Prepend (pre-multiply) a scale transformation to a 4x4 matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
inline const Matrix4 prependScale( const Vector3 & scaleVec, const Matrix4 & mat );

// Multiply two 4x4 matrices per element
// 
inline const Matrix4 mulPerElem( const Matrix4 & mat0, const Matrix4 & mat1 );

// Compute the absolute value of a 4x4 matrix per element
// 
inline const Matrix4 absPerElem( const Matrix4 & mat );

// Transpose of a 4x4 matrix
// 
inline const Matrix4 transpose( const Matrix4 & mat );

// Compute the inverse of a 4x4 matrix
// NOTE: 
// Result is unpredictable when the determinant of mat is equal to or near 0.
// 
inline const Matrix4 inverse( const Matrix4 & mat );

// Compute the inverse of a 4x4 matrix, which is expected to be an affine matrix
// NOTE: 
// This can be used to achieve better performance than a general inverse when the specified 4x4 matrix meets the given restrictions.  The result is unpredictable when the determinant of mat is equal to or near 0.
// 
inline const Matrix4 affineInverse( const Matrix4 & mat );

// Compute the inverse of a 4x4 matrix, which is expected to be an affine matrix with an orthogonal upper-left 3x3 submatrix
// NOTE: 
// This can be used to achieve better performance than a general inverse when the specified 4x4 matrix meets the given restrictions.
// 
inline const Matrix4 orthoInverse( const Matrix4 & mat );

// Determinant of a 4x4 matrix
// 
inline float determinant( const Matrix4 & mat );

// Conditionally select between two 4x4 matrices
// 
inline const Matrix4 select( const Matrix4 & mat0, const Matrix4 & mat1, bool select1 );

#ifdef _VECTORMATH_DEBUG

// Print a 4x4 matrix
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Matrix4 & mat );

// Print a 4x4 matrix and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Matrix4 & mat, const char * name );

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
    inline Transform3( ) { };

    // Copy a 3x4 transformation matrix
    // 
    inline Transform3( const Transform3 & tfrm );

    // Construct a 3x4 transformation matrix containing the specified columns
    // 
    inline Transform3( const Vector3 & col0, const Vector3 & col1, const Vector3 & col2, const Vector3 & col3 );

    // Construct a 3x4 transformation matrix from a 3x3 matrix and a 3-D vector
    // 
    inline Transform3( const Matrix3 & tfrm, const Vector3 & translateVec );

    // Construct a 3x4 transformation matrix from a unit-length quaternion and a 3-D vector
    // 
    inline Transform3( const Quat & unitQuat, const Vector3 & translateVec );

    // Set all elements of a 3x4 transformation matrix to the same scalar value
    // 
    explicit inline Transform3( float scalar );

    // Assign one 3x4 transformation matrix to another
    // 
    inline Transform3 & operator =( const Transform3 & tfrm );

    // Set the upper-left 3x3 submatrix
    // 
    inline Transform3 & setUpper3x3( const Matrix3 & mat3 );

    // Get the upper-left 3x3 submatrix of a 3x4 transformation matrix
    // 
    inline const Matrix3 getUpper3x3( ) const;

    // Set translation component
    // 
    inline Transform3 & setTranslation( const Vector3 & translateVec );

    // Get the translation component of a 3x4 transformation matrix
    // 
    inline const Vector3 getTranslation( ) const;

    // Set column 0 of a 3x4 transformation matrix
    // 
    inline Transform3 & setCol0( const Vector3 & col0 );

    // Set column 1 of a 3x4 transformation matrix
    // 
    inline Transform3 & setCol1( const Vector3 & col1 );

    // Set column 2 of a 3x4 transformation matrix
    // 
    inline Transform3 & setCol2( const Vector3 & col2 );

    // Set column 3 of a 3x4 transformation matrix
    // 
    inline Transform3 & setCol3( const Vector3 & col3 );

    // Get column 0 of a 3x4 transformation matrix
    // 
    inline const Vector3 getCol0( ) const;

    // Get column 1 of a 3x4 transformation matrix
    // 
    inline const Vector3 getCol1( ) const;

    // Get column 2 of a 3x4 transformation matrix
    // 
    inline const Vector3 getCol2( ) const;

    // Get column 3 of a 3x4 transformation matrix
    // 
    inline const Vector3 getCol3( ) const;

    // Set the column of a 3x4 transformation matrix referred to by the specified index
    // 
    inline Transform3 & setCol( int col, const Vector3 & vec );

    // Set the row of a 3x4 transformation matrix referred to by the specified index
    // 
    inline Transform3 & setRow( int row, const Vector4 & vec );

    // Get the column of a 3x4 transformation matrix referred to by the specified index
    // 
    inline const Vector3 getCol( int col ) const;

    // Get the row of a 3x4 transformation matrix referred to by the specified index
    // 
    inline const Vector4 getRow( int row ) const;

    // Subscripting operator to set or get a column
    // 
    inline Vector3 & operator []( int col );

    // Subscripting operator to get a column
    // 
    inline const Vector3 operator []( int col ) const;

    // Set the element of a 3x4 transformation matrix referred to by column and row indices
    // 
    inline Transform3 & setElem( int col, int row, float val );

    // Get the element of a 3x4 transformation matrix referred to by column and row indices
    // 
    inline float getElem( int col, int row ) const;

    // Multiply a 3x4 transformation matrix by a 3-D vector
    // 
    inline const Vector3 operator *( const Vector3 & vec ) const;

    // Multiply a 3x4 transformation matrix by a 3-D point
    // 
    inline const Point3 operator *( const Point3 & pnt ) const;

    // Multiply two 3x4 transformation matrices
    // 
    inline const Transform3 operator *( const Transform3 & tfrm ) const;

    // Perform compound assignment and multiplication by a 3x4 transformation matrix
    // 
    inline Transform3 & operator *=( const Transform3 & tfrm );

    // Construct an identity 3x4 transformation matrix
    // 
    static inline const Transform3 identity( );

    // Construct a 3x4 transformation matrix to rotate around the x axis
    // 
    static inline const Transform3 rotationX( float radians );

    // Construct a 3x4 transformation matrix to rotate around the y axis
    // 
    static inline const Transform3 rotationY( float radians );

    // Construct a 3x4 transformation matrix to rotate around the z axis
    // 
    static inline const Transform3 rotationZ( float radians );

    // Construct a 3x4 transformation matrix to rotate around the x, y, and z axes
    // 
    static inline const Transform3 rotationZYX( const Vector3 & radiansXYZ );

    // Construct a 3x4 transformation matrix to rotate around a unit-length 3-D vector
    // 
    static inline const Transform3 rotation( float radians, const Vector3 & unitVec );

    // Construct a rotation matrix from a unit-length quaternion
    // 
    static inline const Transform3 rotation( const Quat & unitQuat );

    // Construct a 3x4 transformation matrix to perform scaling
    // 
    static inline const Transform3 scale( const Vector3 & scaleVec );

    // Construct a 3x4 transformation matrix to perform translation
    // 
    static inline const Transform3 translation( const Vector3 & translateVec );

};
// Append (post-multiply) a scale transformation to a 3x4 transformation matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
inline const Transform3 appendScale( const Transform3 & tfrm, const Vector3 & scaleVec );

// Prepend (pre-multiply) a scale transformation to a 3x4 transformation matrix
// NOTE: 
// Faster than creating and multiplying a scale transformation matrix.
// 
inline const Transform3 prependScale( const Vector3 & scaleVec, const Transform3 & tfrm );

// Multiply two 3x4 transformation matrices per element
// 
inline const Transform3 mulPerElem( const Transform3 & tfrm0, const Transform3 & tfrm1 );

// Compute the absolute value of a 3x4 transformation matrix per element
// 
inline const Transform3 absPerElem( const Transform3 & tfrm );

// Inverse of a 3x4 transformation matrix
// NOTE: 
// Result is unpredictable when the determinant of the left 3x3 submatrix is equal to or near 0.
// 
inline const Transform3 inverse( const Transform3 & tfrm );

// Compute the inverse of a 3x4 transformation matrix, expected to have an orthogonal upper-left 3x3 submatrix
// NOTE: 
// This can be used to achieve better performance than a general inverse when the specified 3x4 transformation matrix meets the given restrictions.
// 
inline const Transform3 orthoInverse( const Transform3 & tfrm );

// Conditionally select between two 3x4 transformation matrices
// 
inline const Transform3 select( const Transform3 & tfrm0, const Transform3 & tfrm1, bool select1 );

#ifdef _VECTORMATH_DEBUG

// Print a 3x4 transformation matrix
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Transform3 & tfrm );

// Print a 3x4 transformation matrix and an associated string identifier
// NOTE: 
// Function is only defined when _VECTORMATH_DEBUG is defined.
// 
inline void print( const Transform3 & tfrm, const char * name );

#endif

} // namespace Aos
} // namespace Vectormath

#include "vec_aos.h"
#include "quat_aos.h"
#include "mat_aos.h"

#endif
