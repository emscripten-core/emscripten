// https://github.com/floooh/sokol-samples/blob/master/html5/HandmadeMath.h
/*
  HandmadeMath.h v1.2.0
  
  This is a single header file with a bunch of useful functions for
  basic game math operations.
  
  =============================================================================
  
  You MUST
  
     #define HANDMADE_MATH_IMPLEMENTATION
     
  in EXACTLY one C or C++ file that includes this header, BEFORE the
  include, like this:
  
     #define HANDMADE_MATH_IMPLEMENTATION
     #include "HandmadeMath.h"
     
  All other files should just #include "HandmadeMath.h" without the #define.
  
  =============================================================================
  
  For overloaded and operator overloaded versions of the base C functions,
  you MUST
  
     #define HANDMADE_MATH_CPP_MODE
     
  in EXACTLY one C or C++ file that includes this header, BEFORE the
  include, like this:
  
     #define HANDMADE_MATH_IMPLEMENTATION
     #define HANDMADE_MATH_CPP_MODE
     #include "HandmadeMath.h"
     
  All other files should just #include "HandmadeMath.h" without the #define.
  
  =============================================================================
  
  To disable SSE intrinsics, you MUST
  
  #define HANDMADE_MATH_NO_SSE
  
  in EXACTLY one C or C++ file that includes this header, BEFORE the
  include, like this:
  
     #define HANDMADE_MATH_IMPLEMENTATION
     #define HANDMADE_MATH_CPP_MODE
     #define HANDMADE_MATH_NO_SSE
     #include "HandmadeMath.h"
     
     or
     
     #define HANDMADE_MATH_IMPLEMENTATION
     #define HANDMADE_MATH_NO_SSE
     #include "HandmadeMath.h"
     
  =============================================================================
  
  To disable inlining functions, you MUST
  
  #define HANDMADE_MATH_NO_INLINE
  
  in EXACTLY one C or C++ file that includes this header, BEFORE the
  include, like this:
  
     #define HANDMADE_MATH_IMPLEMENTATION
     #define HANDMADE_MATH_CPP_MODE
     #define HANDMADE_MATH_NO_INLINE
     #include "HandmadeMath.h"
     
  All other files should just #include "HandmadeMath.h" without the #define.
  
  =============================================================================
  
  To use HandmadeMath without the CRT, you MUST 
  
     #define HMM_SINF MySinF
     #define HMM_COSF MyCosF
     #define HMM_TANF MyTanF
     #define HMM_SQRTF MySqrtF
     #define HMM_EXPF MyExpF
     #define HMM_LOGF MyLogF
     #define HMM_ACOSF MyACosF
     #define HMM_ATANF MyATanF
     #define HMM_ATAN2F MYATan2F
     
  Provide your own implementations of SinF, CosF, TanF, ACosF, ATanF, ATan2F, 
  ExpF, and LogF in EXACTLY one C or C++ file that includes this header,
  BEFORE the include, like this:     
  
     #define HMM_SINF MySinF
     #define HMM_COSF MyCosF
     #define HMM_TANF MyTanF
     #define HMM_SQRTF MySqrtF
     #define HMM_EXPF MyExpF
     #define HMM_LOGF MyLogF
     #define HMM_ACOSF MyACosF
     #define HMM_ATANF MyATanF
     #define HMM_ATAN2F MyATan2F
     #define HANDMADE_MATH_IMPLEMENTATION
     #define HANDMADE_MATH_CPP_MODE
     #include "HandmadeMath.h"
     
  If you do not define all five of these, HandmadeMath.h will use the
  versions of these functions that are provided by the CRT.
  
  =============================================================================
  
  Version History:
      0.2 (*) Updated documentation
          (*) Better C compliance
          (*) Prefix all handmade math functions 
          (*) Better operator overloading
      0.2a
          (*) Prefixed Macros
      0.2b
          (*) Disabled warning 4201 on MSVC as it is legal is C11
          (*) Removed the f at the end of HMM_PI to get 64bit precision
      0.3
          (*) Added +=, -=, *=, /= for hmm_vec2, hmm_vec3, hmm_vec4
      0.4
          (*) SSE Optimized HMM_SqrtF
          (*) SSE Optimized HMM_RSqrtF
          (*) Removed CRT
      0.5
          (*) Added scalar multiplication and division for vectors
              and matrices
          (*) Added matrix subtraction and += for hmm_mat4
          (*) Reconciled all headers and implementations
          (*) Tidied up, and filled in a few missing operators
      0.5.1
          (*) Ensured column-major order for matrices throughout
          (*) Fixed HMM_Translate producing row-major matrices
      0.5.2
          (*) Fixed SSE code in HMM_SqrtF
          (*) Fixed SSE code in HMM_RSqrtF
      0.6
          (*) Added Unit testing
          (*) Made HMM_Power faster
          (*) Fixed possible efficiency problem with HMM_Normalize 
          (*) RENAMED HMM_LengthSquareRoot to HMM_LengthSquared
          (*) RENAMED HMM_RSqrtF to HMM_RSquareRootF
          (*) RENAMED HMM_SqrtF to HMM_SquareRootF
          (*) REMOVED Inner function (user should use Dot now)
          (*) REMOVED HMM_FastInverseSquareRoot function declaration
      0.7 
          (*) REMOVED HMM_LengthSquared in HANDMADE_MATH_IMPLEMENTATION (should
              use HMM_LengthSquaredVec3, or HANDMADE_MATH_CPP_MODE for function
              overloaded version)
          (*) REMOVED HMM_Length in HANDMADE_MATH_IMPLEMENTATION (should use
              HMM_LengthVec3, HANDMADE_MATH_CPP_MODE for function
              overloaded version)
          (*) REMOVED HMM_Normalize in HANDMADE_MATH_IMPLEMENTATION (should use
              HMM_NormalizeVec3, or HANDMADE_MATH_CPP_MODE for function
              overloaded version)
          (*) Added HMM_LengthSquaredVec2
          (*) Added HMM_LengthSquaredVec4
          (*) Addd HMM_LengthVec2
          (*) Added HMM_LengthVec4
          (*) Added HMM_NormalizeVec2
          (*) Added HMM_NormalizeVec4
     1.0
          (*) Lots of testing!
     1.1
          (*) Quaternion support
          (*) Added type hmm_quaternion
          (*) Added HMM_Quaternion
          (*) Added HMM_QuaternionV4
          (*) Added HMM_AddQuaternion
          (*) Added HMM_SubtractQuaternion
          (*) Added HMM_MultiplyQuaternion
          (*) Added HMM_MultiplyQuaternionF
          (*) Added HMM_DivideQuaternionF
          (*) Added HMM_InverseQuaternion
          (*) Added HMM_DotQuaternion
          (*) Added HMM_NormalizeQuaternion
          (*) Added HMM_Slerp
          (*) Added HMM_QuaternionToMat4
          (*) Added HMM_QuaternionFromAxisAngle
     1.1.1
          (*) Resolved compiler warnings on gcc and g++
     1.1.2
          (*) Fixed invalid HMMDEF's in the function definitions
     1.1.3
          (*) Fixed compile error in C mode
     1.1.4
          (*) Fixed SSE being included on platforms that don't support it
          (*) Fixed divide-by-zero errors when normalizing zero vectors.
     1.1.5
          (*) Add Width and Height to HMM_Vec2
          (*) Made it so you can supply your own SqrtF 
     1.2.0
          (*) Added equality functions for HMM_Vec2, HMM_Vec3, and HMM_Vec4.
              (*) Added HMM_EqualsVec2, HMM_EqualsVec3, and HMM_EqualsVec4
              (*) Added C++ overloaded HMM_Equals for all three
              (*) Added C++ == and != operators for all three
          (*) SSE'd HMM_MultiplyMat4 (this is _WAY_ faster)
          (*) SSE'd HMM_Transpose
          
  LICENSE
  
  This software is in the public domain. Where that dedication is not
  recognized, you are granted a perpetual, irrevocable license to copy,
  distribute, and modify this file as you see fit.
  
  CREDITS
  
  Written by Zakary Strange (zak@handmade.network && @strangezak)
  
  Functionality:
   Matt Mascarenhas (@miblo_)
   Aleph
   FieryDrake (@fierydrake)
   Gingerbill (@TheGingerBill)
   Ben Visness (@bvisness) 
   Trinton Bullard (@Peliex_Dev)
   
  Fixes:
   Jeroen van Rijn (@J_vanRijn)
   Kiljacken (@Kiljacken)
   Insofaras (@insofaras)
   Daniel Gibson (@DanielGibson)
*/


/* let's figure out if SSE is really available (unless disabled anyway)
   (it isn't on non-x86/x86_64 platforms or even x86 without explicit SSE support)
   => only use "#ifdef HANDMADE_MATH__USE_SSE" to check for SSE support below this block! */
#ifndef HANDMADE_MATH_NO_SSE

# ifdef _MSC_VER
   /* MSVC supports SSE in amd64 mode or _M_IX86_FP >= 1 (2 means SSE2) */
#  if defined(_M_AMD64) || ( defined(_M_IX86_FP) && _M_IX86_FP >= 1 )
#   define HANDMADE_MATH__USE_SSE 1
#  endif
# else /* not MSVC, probably GCC, clang, icc or something that doesn't support SSE anyway */
#  ifdef __SSE__ /* they #define __SSE__ if it's supported */
#   define HANDMADE_MATH__USE_SSE 1
#  endif /*  __SSE__ */
# endif /* not _MSC_VER */

#endif /* #ifndef HANDMADE_MATH_NO_SSE */

#include <stdint.h> // This is for types

#ifdef HANDMADE_MATH__USE_SSE
#include <xmmintrin.h>
#endif

#ifndef HANDMADE_MATH_H
#define HANDMADE_MATH_H

#ifdef _MSC_VER
#pragma warning(disable:4201)
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef HANDMADE_MATH_STATIC
#define HMMDEF static
#else
#define HMMDEF extern
#endif

#ifdef HANDMADE_MATH_NO_INLINE
#define HINLINE
#elif _MSC_VER && !__INTEL_COMPILER
#define HINLINE __inline
#else
#define HINLINE inline
#endif

#if !defined(HMM_SINF) || !defined(HMM_COSF) || !defined(HMM_TANF) || \
    !defined(HMM_SQRTF) || !defined(HMM_EXPF) || !defined(HMM_LOGF) || \
    !defined(HMM_ACOSF) || !defined(HMM_ATANF)|| !defined(HMM_ATAN2F)
#include <math.h>    
#endif
    
#ifndef HMM_SINF
#define HMM_SINF sinf
#endif    
        
#ifndef HMM_COSF
#define HMM_COSF cosf
#endif    
        
#ifndef HMM_TANF
#define HMM_TANF tanf
#endif        

#ifndef HMM_SQRTF
#define HMM_SQRTF sqrtf
#endif    
    
#ifndef HMM_EXPF
#define HMM_EXPF expf
#endif

#ifndef HMM_LOGF
#define HMM_LOGF logf
#endif

#ifndef HMM_ACOSF
#define HMM_ACOSF acosf
#endif

#ifndef HMM_ATANF
#define HMM_ATANF atanf
#endif

#ifndef HMM_ATAN2F
#define HMM_ATAN2F atan2f
#endif

#define HMM_PI32 3.14159265359f
#define HMM_PI 3.14159265358979323846

#define HMM_MIN(a, b) (a) > (b) ? (b) : (a)
#define HMM_MAX(a, b) (a) < (b) ? (b) : (a)
#define HMM_ABS(a) ((a) > 0 ? (a) : -(a))
#define HMM_MOD(a, m) ((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m))
#define HMM_SQUARE(x) ((x) * (x))

typedef union hmm_vec2
{
    struct
    {
        float X, Y;
    };

    struct
    {
        float U, V;
    };

    struct
    {
        float Left, Right;
    };
    
    struct
    {
        float Width, Height;
    };

    float Elements[2];
} hmm_vec2;

typedef union hmm_vec3
{
    struct
    {
        float X, Y, Z;
    };

    struct
    {
        float U, V, W;
    };

    struct
    {
        float R, G, B;
    };

    struct
    {
        hmm_vec2 XY;
        float Ignored0_;
    };

    struct
    {
        float Ignored1_;
        hmm_vec2 YZ;
    };

    struct
    {
        hmm_vec2 UV;
        float Ignored2_;
    };

    struct
    {
        float Ignored3_;
        hmm_vec2 VW;
    };

    float Elements[3];
} hmm_vec3;

typedef union hmm_vec4
{
    struct
    {
        union
        {
            hmm_vec3 XYZ;
            struct
            {
                float X, Y, Z;
            };
        };

        float W;
    };
    struct
    {
        union
        {
            hmm_vec3 RGB;
            struct
            {
                float R, G, B;
            };
        };

        float A;
    };

    struct
    {
        hmm_vec2 XY;
        float Ignored0_;
        float Ignored1_;
    };

    struct
    {
        float Ignored2_;
        hmm_vec2 YZ;
        float Ignored3_;
    };

    struct
    {
        float Ignored4_;
        float Ignored5_;
        hmm_vec2 ZW;
    };

    float Elements[4];
} hmm_vec4;

typedef union hmm_mat4
{
    float Elements[4][4];
    
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 Rows[4];
#endif
} hmm_mat4;

typedef union hmm_quaternion
{
    struct
    {
        union
        {
            hmm_vec3 XYZ;
            struct
            {
                float X, Y, Z;
            };
        };
        
        float W;
    };
    
    float Elements[4];
} hmm_quaternion;

typedef int32_t hmm_bool;

typedef hmm_vec2 hmm_v2;
typedef hmm_vec3 hmm_v3;
typedef hmm_vec4 hmm_v4;
typedef hmm_mat4 hmm_m4;    

HMMDEF float HMM_SinF(float Angle);
HMMDEF float HMM_TanF(float Angle);
HMMDEF float HMM_ATanF(float Theta);
HMMDEF float HMM_ATan2F(float Theta, float Theta2);
HMMDEF float HMM_CosF(float Angle);
HMMDEF float HMM_ACosF(float Theta);
HMMDEF float HMM_ExpF(float Float);
HMMDEF float HMM_LogF(float Float);

HMMDEF float HMM_ToRadians(float Degrees);
HMMDEF float HMM_SquareRootF(float Float);
HMMDEF float HMM_RSquareRootF(float Float);

HMMDEF float HMM_LengthSquaredVec2(hmm_vec2 A);
HMMDEF float HMM_LengthSquaredVec3(hmm_vec3 A);
HMMDEF float HMM_LengthSquaredVec4(hmm_vec4 A);

HMMDEF float HMM_LengthVec2(hmm_vec2 A);    
HMMDEF float HMM_LengthVec3(hmm_vec3 A);    
HMMDEF float HMM_LengthVec4(hmm_vec4 A);    

HMMDEF float HMM_Power(float Base, int Exponent);
HMMDEF float HMM_PowerF(float Base, float Exponent);
HMMDEF float HMM_Lerp(float A, float Time, float B);
HMMDEF float HMM_Clamp(float Min, float Value, float Max);

HMMDEF hmm_vec2 HMM_NormalizeVec2(hmm_vec2 A);
HMMDEF hmm_vec3 HMM_NormalizeVec3(hmm_vec3 A);
HMMDEF hmm_vec4 HMM_NormalizeVec4(hmm_vec4 A);

HMMDEF float HMM_DotVec2(hmm_vec2 VecOne, hmm_vec2 VecTwo);
HMMDEF float HMM_DotVec3(hmm_vec3 VecOne, hmm_vec3 VecTwo);
HMMDEF float HMM_DotVec4(hmm_vec4 VecOne, hmm_vec4 VecTwo);

HMMDEF hmm_vec3 HMM_Cross(hmm_vec3 VecOne, hmm_vec3 VecTwo);

HMMDEF hmm_vec2 HMM_Vec2(float X, float Y);
HMMDEF hmm_vec2 HMM_Vec2i(int X, int Y);
HMMDEF hmm_vec3 HMM_Vec3(float X, float Y, float Z);
HMMDEF hmm_vec3 HMM_Vec3i(int X, int Y, int Z);
HMMDEF hmm_vec4 HMM_Vec4(float X, float Y, float Z, float W);
HMMDEF hmm_vec4 HMM_Vec4i(int X, int Y, int Z, int W);
HMMDEF hmm_vec4 HMM_Vec4v(hmm_vec3 Vector, float W);

HMMDEF hmm_vec2 HMM_AddVec2(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec3 HMM_AddVec3(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec4 HMM_AddVec4(hmm_vec4 Left, hmm_vec4 Right);

HMMDEF hmm_vec2 HMM_SubtractVec2(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec3 HMM_SubtractVec3(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec4 HMM_SubtractVec4(hmm_vec4 Left, hmm_vec4 Right);

HMMDEF hmm_vec2 HMM_MultiplyVec2(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec2 HMM_MultiplyVec2f(hmm_vec2 Left, float Right);
HMMDEF hmm_vec3 HMM_MultiplyVec3(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec3 HMM_MultiplyVec3f(hmm_vec3 Left, float Right);
HMMDEF hmm_vec4 HMM_MultiplyVec4(hmm_vec4 Left, hmm_vec4 Right);
HMMDEF hmm_vec4 HMM_MultiplyVec4f(hmm_vec4 Left, float Right);

HMMDEF hmm_vec2 HMM_DivideVec2(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec2 HMM_DivideVec2f(hmm_vec2 Left, float Right);
HMMDEF hmm_vec3 HMM_DivideVec3(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec3 HMM_DivideVec3f(hmm_vec3 Left, float Right);
HMMDEF hmm_vec4 HMM_DivideVec4(hmm_vec4 Left, hmm_vec4 Right);
HMMDEF hmm_vec4 HMM_DivideVec4f(hmm_vec4 Left, float Right);

HMMDEF hmm_bool HMM_EqualsVec2(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_bool HMM_EqualsVec3(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_bool HMM_EqualsVec4(hmm_vec4 Left, hmm_vec4 Right);

HMMDEF hmm_mat4 HMM_Mat4(void);
HMMDEF hmm_mat4 HMM_Mat4d(float Diagonal);
HMMDEF hmm_mat4 HMM_AddMat4(hmm_mat4 Left, hmm_mat4 Right);
HMMDEF hmm_mat4 HMM_SubtractMat4(hmm_mat4 Left, hmm_mat4 Right);

#ifdef HANDMADE_MATH__USE_SSE
HMMDEF __m128 HMM_LinearCombineSSE(__m128 Left, hmm_mat4 Right);
#endif 

HMMDEF hmm_mat4 HMM_MultiplyMat4(hmm_mat4 Left, hmm_mat4 Right);
HMMDEF hmm_mat4 HMM_MultiplyMat4f(hmm_mat4 Matrix, float Scalar);
HMMDEF hmm_vec4 HMM_MultiplyMat4ByVec4(hmm_mat4 Matrix, hmm_vec4 Vector);
HMMDEF hmm_mat4 HMM_DivideMat4f(hmm_mat4 Matrix, float Scalar);

HMMDEF hmm_mat4 HMM_Transpose(hmm_mat4 Matrix);

HMMDEF hmm_mat4 HMM_Orthographic(float Left, float Right, float Bottom, float Top, float Near, float Far);
HMMDEF hmm_mat4 HMM_Perspective(float FOV, float AspectRatio, float Near, float Far);

HMMDEF hmm_mat4 HMM_Translate(hmm_vec3 Translation);
HMMDEF hmm_mat4 HMM_Rotate(float Angle, hmm_vec3 Axis);
HMMDEF hmm_mat4 HMM_Scale(hmm_vec3 Scale);

HMMDEF hmm_mat4 HMM_LookAt(hmm_vec3 Eye, hmm_vec3 Center, hmm_vec3 Up);

HMMDEF hmm_quaternion HMM_Quaternion(float X, float Y, float Z, float W);
HMMDEF hmm_quaternion HMM_QuaternionV4(hmm_vec4 Vector);
HMMDEF hmm_quaternion HMM_AddQuaternion(hmm_quaternion Left, hmm_quaternion Right);
HMMDEF hmm_quaternion HMM_SubtractQuaternion(hmm_quaternion Left, hmm_quaternion Right);
HMMDEF hmm_quaternion HMM_MultiplyQuaternion(hmm_quaternion Left, hmm_quaternion Right);
HMMDEF hmm_quaternion HMM_MultiplyQuaternionF(hmm_quaternion Left, float Multiplicative);
HMMDEF hmm_quaternion HMM_DivideQuaternionF(hmm_quaternion Left, float Dividend);
HMMDEF hmm_quaternion HMM_InverseQuaternion(hmm_quaternion Left);
HMMDEF float HMM_DotQuaternion(hmm_quaternion Left, hmm_quaternion Right);
HMMDEF hmm_quaternion HMM_NormalizeQuaternion(hmm_quaternion Left);
HMMDEF hmm_quaternion HMM_NLerp(hmm_quaternion Left, float Time, hmm_quaternion Right);
HMMDEF hmm_quaternion HMM_Slerp(hmm_quaternion Left, float Time, hmm_quaternion Right);
HMMDEF hmm_mat4 HMM_QuaternionToMat4(hmm_quaternion Left);
HMMDEF hmm_quaternion HMM_QuaternionFromAxisAngle(hmm_vec3 Axis, float AngleOfRotation);

#ifdef __cplusplus
}
#endif

#ifdef HANDMADE_MATH_CPP_MODE

HMMDEF float HMM_Length(hmm_vec2 A);
HMMDEF float HMM_Length(hmm_vec3 A);
HMMDEF float HMM_Length(hmm_vec4 A);

HMMDEF float HMM_LengthSquared(hmm_vec2 A);
HMMDEF float HMM_LengthSquared(hmm_vec3 A);
HMMDEF float HMM_LengthSquared(hmm_vec4 A);

HMMDEF hmm_vec2 HMM_Normalize(hmm_vec2 A);
HMMDEF hmm_vec3 HMM_Normalize(hmm_vec3 A);
HMMDEF hmm_vec4 HMM_Normalize(hmm_vec4 A);
HMMDEF hmm_quaternion HMM_Normalize(hmm_quaternion A);

HMMDEF float HMM_Dot(hmm_vec2 VecOne, hmm_vec2 VecTwo);
HMMDEF float HMM_Dot(hmm_vec3 VecOne, hmm_vec3 VecTwo);
HMMDEF float HMM_Dot(hmm_vec4 VecOne, hmm_vec4 VecTwo);
HMMDEF float HMM_Dot(hmm_quaternion QuatOne, hmm_quaternion QuatTwo);

HMMDEF hmm_vec2 HMM_Add(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec3 HMM_Add(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec4 HMM_Add(hmm_vec4 Left, hmm_vec4 Right);
HMMDEF hmm_mat4 HMM_Add(hmm_mat4 Left, hmm_mat4 Right);
HMMDEF hmm_quaternion HMM_Add(hmm_quaternion Left, hmm_quaternion Right);

HMMDEF hmm_vec2 HMM_Subtract(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec3 HMM_Subtract(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec4 HMM_Subtract(hmm_vec4 Left, hmm_vec4 Right);
HMMDEF hmm_mat4 HMM_Subtract(hmm_mat4 Left, hmm_mat4 Right);
HMMDEF hmm_quaternion HMM_Subtract(hmm_quaternion Left, hmm_quaternion Right);

HMMDEF hmm_vec2 HMM_Multiply(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec2 HMM_Multiply(hmm_vec2 Left, float Right);
HMMDEF hmm_vec3 HMM_Multiply(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec3 HMM_Multiply(hmm_vec3 Left, float Right);
HMMDEF hmm_vec4 HMM_Multiply(hmm_vec4 Left, hmm_vec4 Right);
HMMDEF hmm_vec4 HMM_Multiply(hmm_vec4 Left, float Right);
HMMDEF hmm_mat4 HMM_Multiply(hmm_mat4 Left, hmm_mat4 Right);
HMMDEF hmm_mat4 HMM_Multiply(hmm_mat4 Left, float Right);
HMMDEF hmm_vec4 HMM_Multiply(hmm_mat4 Matrix, hmm_vec4 Vector);
HMMDEF hmm_quaternion HMM_Multiply(hmm_quaternion Left, hmm_quaternion Right);
HMMDEF hmm_quaternion HMM_Multiply(hmm_quaternion Left, float Right);

HMMDEF hmm_vec2 HMM_Divide(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec2 HMM_Divide(hmm_vec2 Left, float Right);
HMMDEF hmm_vec3 HMM_Divide(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec3 HMM_Divide(hmm_vec3 Left, float Right);
HMMDEF hmm_vec4 HMM_Divide(hmm_vec4 Left, hmm_vec4 Right);
HMMDEF hmm_vec4 HMM_Divide(hmm_vec4 Left, float Right);
HMMDEF hmm_mat4 HMM_Divide(hmm_mat4 Left, float Right);
HMMDEF hmm_quaternion HMM_Divide(hmm_quaternion Left, hmm_quaternion Right);
HMMDEF hmm_quaternion HMM_Divide(hmm_quaternion Left, float Right);

HMMDEF hmm_bool HMM_Equals(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_bool HMM_Equals(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_bool HMM_Equals(hmm_vec4 Left, hmm_vec4 Right);

HMMDEF hmm_vec2 operator+(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec3 operator+(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec4 operator+(hmm_vec4 Left, hmm_vec4 Right);
HMMDEF hmm_mat4 operator+(hmm_mat4 Left, hmm_mat4 Right);
HMMDEF hmm_quaternion operator+(hmm_quaternion Left, hmm_quaternion Right);

HMMDEF hmm_vec2 operator-(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec3 operator-(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec4 operator-(hmm_vec4 Left, hmm_vec4 Right);
HMMDEF hmm_mat4 operator-(hmm_mat4 Left, hmm_mat4 Right);
HMMDEF hmm_quaternion operator-(hmm_quaternion Left, hmm_quaternion Right);

HMMDEF hmm_vec2 operator*(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec3 operator*(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec4 operator*(hmm_vec4 Left, hmm_vec4 Right);
HMMDEF hmm_mat4 operator*(hmm_mat4 Left, hmm_mat4 Right);
HMMDEF hmm_quaternion operator*(hmm_quaternion Left, hmm_quaternion Right);

HMMDEF hmm_vec2 operator*(hmm_vec2 Left, float Right);
HMMDEF hmm_vec3 operator*(hmm_vec3 Left, float Right);
HMMDEF hmm_vec4 operator*(hmm_vec4 Left, float Right);
HMMDEF hmm_mat4 operator*(hmm_mat4 Left, float Right);
HMMDEF hmm_quaternion operator*(hmm_quaternion Left, float Right);

HMMDEF hmm_vec2 operator*(float Left, hmm_vec2 Right);
HMMDEF hmm_vec3 operator*(float Left, hmm_vec3 Right);
HMMDEF hmm_vec4 operator*(float Left, hmm_vec4 Right);
HMMDEF hmm_mat4 operator*(float Left, hmm_mat4 Right);
HMMDEF hmm_quaternion operator*(float Left, hmm_quaternion Right);

HMMDEF hmm_vec4 operator*(hmm_mat4 Matrix, hmm_vec4 Vector);

HMMDEF hmm_vec2 operator/(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_vec3 operator/(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_vec4 operator/(hmm_vec4 Left, hmm_vec4 Right);

HMMDEF hmm_vec2 operator/(hmm_vec2 Left, float Right);
HMMDEF hmm_vec3 operator/(hmm_vec3 Left, float Right);
HMMDEF hmm_vec4 operator/(hmm_vec4 Left, float Right);
HMMDEF hmm_mat4 operator/(hmm_mat4 Left, float Right);
HMMDEF hmm_quaternion operator/(hmm_quaternion Left, float Right);

HMMDEF hmm_vec2 &operator+=(hmm_vec2 &Left, hmm_vec2 Right);
HMMDEF hmm_vec3 &operator+=(hmm_vec3 &Left, hmm_vec3 Right);
HMMDEF hmm_vec4 &operator+=(hmm_vec4 &Left, hmm_vec4 Right);
HMMDEF hmm_mat4 &operator+=(hmm_mat4 &Left, hmm_mat4 Right);
HMMDEF hmm_quaternion &operator+=(hmm_quaternion &Left, hmm_quaternion Right);

HMMDEF hmm_vec2 &operator-=(hmm_vec2 &Left, hmm_vec2 Right);
HMMDEF hmm_vec3 &operator-=(hmm_vec3 &Left, hmm_vec3 Right);
HMMDEF hmm_vec4 &operator-=(hmm_vec4 &Left, hmm_vec4 Right);
HMMDEF hmm_mat4 &operator-=(hmm_mat4 &Left, hmm_mat4 Right);
HMMDEF hmm_quaternion &operator-=(hmm_quaternion &Left, hmm_quaternion Right);

HMMDEF hmm_vec2 &operator*=(hmm_vec2 &Left, hmm_vec2 Right);
HMMDEF hmm_vec3 &operator*=(hmm_vec3 &Left, hmm_vec3 Right);
HMMDEF hmm_vec4 &operator*=(hmm_vec4 &Left, hmm_vec4 Right);

HMMDEF hmm_vec2 &operator*=(hmm_vec2 &Left, float Right);
HMMDEF hmm_vec3 &operator*=(hmm_vec3 &Left, float Right);
HMMDEF hmm_vec4 &operator*=(hmm_vec4 &Left, float Right);
HMMDEF hmm_mat4 &operator*=(hmm_mat4 &Left, float Right);
HMMDEF hmm_quaternion &operator*=(hmm_quaternion &Left, float Right);

HMMDEF hmm_vec2 &operator/=(hmm_vec2 &Left, hmm_vec2 Right);
HMMDEF hmm_vec3 &operator/=(hmm_vec3 &Left, hmm_vec3 Right);
HMMDEF hmm_vec4 &operator/=(hmm_vec4 &Left, hmm_vec4 Right);

HMMDEF hmm_vec2 &operator/=(hmm_vec2 &Left, float Right);
HMMDEF hmm_vec3 &operator/=(hmm_vec3 &Left, float Right);
HMMDEF hmm_vec4 &operator/=(hmm_vec4 &Left, float Right);
HMMDEF hmm_mat4 &operator/=(hmm_mat4 &Left, float Right);
HMMDEF hmm_quaternion &operator/=(hmm_quaternion &Left, float Right);

HMMDEF hmm_bool operator==(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_bool operator==(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_bool operator==(hmm_vec4 Left, hmm_vec4 Right);

HMMDEF hmm_bool operator!=(hmm_vec2 Left, hmm_vec2 Right);
HMMDEF hmm_bool operator!=(hmm_vec3 Left, hmm_vec3 Right);
HMMDEF hmm_bool operator!=(hmm_vec4 Left, hmm_vec4 Right);

#endif /* HANDMADE_MATH_CPP */

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif /* HANDMADE_MATH_H */

#ifdef HANDMADE_MATH_IMPLEMENTATION

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wmissing-braces"
#endif

HINLINE float
HMM_SinF(float Angle)
{
    float Result = 0.0f;

    Result = HMM_SINF(Angle);
    return (Result);
}

HINLINE float
HMM_CosF(float Angle)
{
    float Result = 0.0f;

    Result = HMM_COSF(Angle);
    return (Result);
}

HINLINE float
HMM_TanF(float Radians)
{
    float Result = 0.0f;

    Result = HMM_TANF(Radians);
    return (Result);
}

HINLINE float
HMM_ACosF(float Radians)
{
    float Result = 0.0f;

    Result = HMM_ACOSF(Radians);
    return (Result);
}

HINLINE float
HMM_ATanF(float Radians)
{
    float Result = 0.0f;

    Result = HMM_ATANF(Radians);
    return (Result);
}

HINLINE float
HMM_Atan2F(float Left, float Right)
{
    float Result = 0.0f;

    Result = HMM_ATAN2F(Left, Right);
    return (Result);
}

HINLINE float
HMM_ExpF(float Float)
{
    float Result = 0.0f;

    Result = HMM_EXPF(Float);
    return (Result);
}

HINLINE float
HMM_LogF(float Float)
{
    float Result = 0.0f;

    Result = HMM_LOGF(Float);
    return (Result);
}

HINLINE float
HMM_ToRadians(float Degrees)
{
    float Result = 0.0f;

    Result = Degrees * (HMM_PI32 / 180.0f);
    return (Result);
}

HINLINE float
HMM_SquareRootF(float Value)
{
    float Result = 0.0f;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 In = _mm_set_ss(Value);
    __m128 Out = _mm_sqrt_ss(In);
    Result = _mm_cvtss_f32(Out);
#else
    Result = HMM_SQRTF(Value);
#endif 

    return(Result);
}

HINLINE float
HMM_RSquareRootF(float Value)
{
    float Result = 0.0f;

#ifdef HANDMADE_MATH__USE_SSE
    __m128 In = _mm_set_ss(Value);
    __m128 Out = _mm_rsqrt_ss(In);
    Result = _mm_cvtss_f32(Out);
#else
    Result = 1.0f/HMM_SquareRootF(Value);
#endif

    return(Result);
}

HINLINE float
HMM_LengthSquaredVec2(hmm_vec2 A)
{
    float Result = 0.0f;
    
    Result = HMM_DotVec2(A, A);
    
    return(Result);
}

HINLINE float
HMM_LengthSquaredVec3(hmm_vec3 A)
{
    float Result = 0.0f;

    Result = HMM_DotVec3(A, A);
    
    return (Result);
}

HINLINE float 
HMM_LengthSquaredVec4(hmm_vec4 A)
{
    float Result = 0.0f;
    
    Result = HMM_DotVec4(A, A);
    
    return(Result);
}

HINLINE float
HMM_LengthVec2(hmm_vec2 A)
{
    float Result = 0.0f;
    
    Result = HMM_SquareRootF(HMM_LengthSquaredVec2(A));
    
    return(Result);
}

HINLINE float
HMM_LengthVec3(hmm_vec3 A)
{
    float Result = 0.0f;

    Result = HMM_SquareRootF(HMM_LengthSquaredVec3(A));
    
    return (Result);
}

HINLINE float 
HMM_LengthVec4(hmm_vec4 A)
{
    float Result = 0.0f;
    
    Result = HMM_SquareRootF(HMM_LengthSquaredVec4(A));
    
    return(Result);
}

HINLINE float
HMM_Power(float Base, int Exponent)
{
    float Result = 1.0f;
    float Mul = Exponent < 0 ? 1.f / Base : Base;
    unsigned int X = Exponent < 0 ? -Exponent : Exponent;
    while (X)
    {
        if (X & 1)
        {
            Result *= Mul;
        }
        
        Mul *= Mul;
        X >>= 1;
    }
    
    return (Result);
}

HINLINE float
HMM_PowerF(float Base, float Exponent)
{
    return HMM_EXPF(Exponent * HMM_LOGF(Base));
}

HINLINE float
HMM_Lerp(float A, float Time, float B)
{
    float Result = 0;

    Result = (1.0f - Time) * A + Time * B;
    return (Result);
}

HINLINE float
HMM_Clamp(float Min, float Value, float Max)
{
    float Result = Value;

    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }

    return (Result);
}

HINLINE hmm_vec2
HMM_NormalizeVec2(hmm_vec2 A)
{
    hmm_vec2 Result = {0};

    float VectorLength = HMM_LengthVec2(A);
    
    /* NOTE(kiljacken): We need a zero check to not divide-by-zero */
    if (VectorLength != 0.0f)
    {
        Result.X = A.X * (1.0f / VectorLength);
        Result.Y = A.Y * (1.0f / VectorLength);
    }
    
    return (Result);
}

HINLINE hmm_vec3
HMM_NormalizeVec3(hmm_vec3 A)
{
    hmm_vec3 Result = {0};

    float VectorLength = HMM_LengthVec3(A);
    
    /* NOTE(kiljacken): We need a zero check to not divide-by-zero */
    if (VectorLength != 0.0f)
    {
        Result.X = A.X * (1.0f / VectorLength);
        Result.Y = A.Y * (1.0f / VectorLength);
        Result.Z = A.Z * (1.0f / VectorLength);
    }
    
    return (Result);
}

HINLINE hmm_vec4
HMM_NormalizeVec4(hmm_vec4 A)
{
    hmm_vec4 Result = {0};

    float VectorLength = HMM_LengthVec4(A);
    
    /* NOTE(kiljacken): We need a zero check to not divide-by-zero */
    if (VectorLength != 0.0f)
    {
        Result.X = A.X * (1.0f / VectorLength);
        Result.Y = A.Y * (1.0f / VectorLength);
        Result.Z = A.Z * (1.0f / VectorLength);
        Result.W = A.W * (1.0f / VectorLength);
    }
    
    return (Result);
}

HINLINE float
HMM_DotVec2(hmm_vec2 VecOne, hmm_vec2 VecTwo)
{
    float Result = 0.0f;

    Result = (VecOne.X * VecTwo.X) + (VecOne.Y * VecTwo.Y);
    
    return (Result);
}

HINLINE float
HMM_DotVec3(hmm_vec3 VecOne, hmm_vec3 VecTwo)
{
    float Result = 0.0f;

    Result = (VecOne.X * VecTwo.X) + (VecOne.Y * VecTwo.Y) + (VecOne.Z * VecTwo.Z);
    
    return (Result);
}

HINLINE float
HMM_DotVec4(hmm_vec4 VecOne, hmm_vec4 VecTwo)
{
    float Result = 0.0f;

    Result = (VecOne.X * VecTwo.X) + (VecOne.Y * VecTwo.Y) + (VecOne.Z * VecTwo.Z) + (VecOne.W * VecTwo.W);
    
    return (Result);
}

HINLINE hmm_vec3
HMM_Cross(hmm_vec3 VecOne, hmm_vec3 VecTwo)
{
    hmm_vec3 Result = {0};

    Result.X = (VecOne.Y * VecTwo.Z) - (VecOne.Z * VecTwo.Y);
    Result.Y = (VecOne.Z * VecTwo.X) - (VecOne.X * VecTwo.Z);
    Result.Z = (VecOne.X * VecTwo.Y) - (VecOne.Y * VecTwo.X);

    return (Result);
}

HINLINE hmm_vec2
HMM_Vec2(float X, float Y)
{
    hmm_vec2 Result = {0};

    Result.X = X;
    Result.Y = Y;

    return (Result);
}

HINLINE hmm_vec2
HMM_Vec2i(int X, int Y)
{
    hmm_vec2 Result = {0};

    Result.X = (float)X;
    Result.Y = (float)Y;

    return (Result);
}

HINLINE hmm_vec3
HMM_Vec3(float X, float Y, float Z)
{
    hmm_vec3 Result = {0};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;

    return (Result);
}

HINLINE hmm_vec3
HMM_Vec3i(int X, int Y, int Z)
{
    hmm_vec3 Result = {0};

    Result.X = (float)X;
    Result.Y = (float)Y;
    Result.Z = (float)Z;

    return (Result);
}

HINLINE hmm_vec4
HMM_Vec4(float X, float Y, float Z, float W)
{
    hmm_vec4 Result = {0};

    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;

    return (Result);
}

HINLINE hmm_vec4
HMM_Vec4i(int X, int Y, int Z, int W)
{
    hmm_vec4 Result = {0};

    Result.X = (float)X;
    Result.Y = (float)Y;
    Result.Z = (float)Z;
    Result.W = (float)W;

    return (Result);
}

HINLINE hmm_vec4
HMM_Vec4v(hmm_vec3 Vector, float W)
{
    hmm_vec4 Result = {0};

    Result.XYZ = Vector;
    Result.W = W;

    return (Result);
}

HINLINE hmm_vec2
HMM_AddVec2(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;

    return (Result);
}

HINLINE hmm_vec3
HMM_AddVec3(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;

    return (Result);
}

HINLINE hmm_vec4
HMM_AddVec4(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    Result.W = Left.W + Right.W;

    return (Result);
}

HINLINE hmm_vec2
HMM_SubtractVec2(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;

    return (Result);
}

HINLINE hmm_vec3
HMM_SubtractVec3(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;

    return (Result);
}

HINLINE hmm_vec4
HMM_SubtractVec4(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    Result.W = Left.W - Right.W;

    return (Result);
}

HINLINE hmm_vec2
HMM_MultiplyVec2(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;

    return (Result);
}

HINLINE hmm_vec2
HMM_MultiplyVec2f(hmm_vec2 Left, float Right)
{
    hmm_vec2 Result = {0};

    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;

    return (Result);
}

HINLINE hmm_vec3
HMM_MultiplyVec3(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    Result.Z = Left.Z * Right.Z;

    return (Result);
}

HINLINE hmm_vec3
HMM_MultiplyVec3f(hmm_vec3 Left, float Right)
{
    hmm_vec3 Result = {0};

    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    Result.Z = Left.Z * Right;

    return (Result);
}

HINLINE hmm_vec4
HMM_MultiplyVec4(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    Result.Z = Left.Z * Right.Z;
    Result.W = Left.W * Right.W;

    return (Result);
}

HINLINE hmm_vec4
HMM_MultiplyVec4f(hmm_vec4 Left, float Right)
{
    hmm_vec4 Result = {0};

    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    Result.Z = Left.Z * Right;
    Result.W = Left.W * Right;

    return (Result);
}

HINLINE hmm_vec2
HMM_DivideVec2(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;

    return (Result);
}

HINLINE hmm_vec2
HMM_DivideVec2f(hmm_vec2 Left, float Right)
{
    hmm_vec2 Result = {0};

    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;

    return (Result);
}

HINLINE hmm_vec3
HMM_DivideVec3(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    Result.Z = Left.Z / Right.Z;

    return (Result);
}

HINLINE hmm_vec3
HMM_DivideVec3f(hmm_vec3 Left, float Right)
{
    hmm_vec3 Result = {0};

    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    Result.Z = Left.Z / Right;

    return (Result);
}

HINLINE hmm_vec4
HMM_DivideVec4(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    Result.Z = Left.Z / Right.Z;
    Result.W = Left.W / Right.W;

    return (Result);
}

HINLINE hmm_vec4
HMM_DivideVec4f(hmm_vec4 Left, float Right)
{
    hmm_vec4 Result = {0};

    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    Result.Z = Left.Z / Right;
    Result.W = Left.W / Right;

    return (Result);
}

HINLINE hmm_bool
HMM_EqualsVec2(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_bool Result = 0;

    Result = (Left.X == Right.X && Left.Y == Right.Y);

    return (Result);
}

HINLINE hmm_bool
HMM_EqualsVec3(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_bool Result = 0;

    Result = (Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z);

    return (Result);
}

HINLINE hmm_bool
HMM_EqualsVec4(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_bool Result = 0;

    Result = (Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z && Left.W == Right.W);

    return (Result);
}

HINLINE hmm_mat4
HMM_Mat4(void)
{
    hmm_mat4 Result = {0};

    return (Result);
}

HINLINE hmm_mat4
HMM_Mat4d(float Diagonal)
{
    hmm_mat4 Result = HMM_Mat4();

    Result.Elements[0][0] = Diagonal;
    Result.Elements[1][1] = Diagonal;
    Result.Elements[2][2] = Diagonal;
    Result.Elements[3][3] = Diagonal;

    return (Result);
}

HINLINE hmm_mat4
HMM_AddMat4(hmm_mat4 Left, hmm_mat4 Right)
{
    hmm_mat4 Result = HMM_Mat4();

    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Left.Elements[Columns][Rows] + Right.Elements[Columns][Rows];
        }
    }

    return (Result);
}

HINLINE hmm_mat4
HMM_SubtractMat4(hmm_mat4 Left, hmm_mat4 Right)
{
    hmm_mat4 Result = HMM_Mat4();

    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Left.Elements[Columns][Rows] - Right.Elements[Columns][Rows];
        }
    }

    return (Result);
}

#ifdef HANDMADE_MATH__USE_SSE
HINLINE __m128
HMM_LinearCombineSSE(__m128 Left, hmm_mat4 Right)
{
    __m128 Result = {};
    Result = _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0x00), Right.Rows[0]);
    Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0x55), Right.Rows[1]));
    Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0xaa), Right.Rows[2]));
    Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0xff), Right.Rows[3]));
    
    return(Result);
}
#endif

HINLINE hmm_mat4
HMM_MultiplyMat4(hmm_mat4 Left, hmm_mat4 Right)
{
    hmm_mat4 Result = HMM_Mat4();

#ifdef HANDMADE_MATH__USE_SSE
    
    hmm_mat4 TransposedLeft = HMM_Transpose(Left);
    hmm_mat4 TransposedRight = HMM_Transpose(Right);

    Result.Rows[0] = HMM_LinearCombineSSE(TransposedLeft.Rows[0], TransposedRight);
    Result.Rows[1] = HMM_LinearCombineSSE(TransposedLeft.Rows[1], TransposedRight);
    Result.Rows[2] = HMM_LinearCombineSSE(TransposedLeft.Rows[2], TransposedRight);
    Result.Rows[3] = HMM_LinearCombineSSE(TransposedLeft.Rows[3], TransposedRight);       
    
    Result = HMM_Transpose(Result);
    
#else
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            float Sum = 0;
            int CurrentMatrice;
            for(CurrentMatrice = 0; CurrentMatrice < 4; ++CurrentMatrice)
            {
                Sum += Left.Elements[CurrentMatrice][Rows] * Right.Elements[Columns][CurrentMatrice];
            }

            Result.Elements[Columns][Rows] = Sum;
        }
    }
#endif 
    return (Result);
}

HINLINE hmm_mat4
HMM_MultiplyMat4f(hmm_mat4 Matrix, float Scalar)
{
    hmm_mat4 Result = HMM_Mat4();

    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Matrix.Elements[Columns][Rows] * Scalar;
        }
    }

    return (Result);
}

HINLINE hmm_vec4
HMM_MultiplyMat4ByVec4(hmm_mat4 Matrix, hmm_vec4 Vector)
{
    hmm_vec4 Result = {0};
    
    int Columns, Rows;
    for(Rows = 0; Rows < 4; ++Rows)
    {
        float Sum = 0;
        for(Columns = 0; Columns < 4; ++Columns)
        {
            Sum += Matrix.Elements[Columns][Rows] * Vector.Elements[Columns];
        }
        
        Result.Elements[Rows] = Sum;
    }
    
    return (Result);
}

HINLINE hmm_mat4
HMM_DivideMat4f(hmm_mat4 Matrix, float Scalar)
{
    hmm_mat4 Result = HMM_Mat4();

    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Matrix.Elements[Columns][Rows] / Scalar;
        }
    }

    return (Result);
}

HINLINE hmm_mat4
HMM_Transpose(hmm_mat4 Matrix)
{
    hmm_mat4 Result = HMM_Mat4();

#ifdef HANDMADE_MATH__USE_SSE
    Result = Matrix;
    
    _MM_TRANSPOSE4_PS(Result.Rows[0], Result.Rows[1], Result.Rows[2], Result.Rows[3]);    
#else 
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Rows][Columns] = Matrix.Elements[Columns][Rows];
        }
    }
#endif 
        
    return (Result);
}

HINLINE hmm_mat4
HMM_Orthographic(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
    hmm_mat4 Result = HMM_Mat4d(1.0f);

    Result.Elements[0][0] = 2.0f / (Right - Left);
    Result.Elements[1][1] = 2.0f / (Top - Bottom);
    Result.Elements[2][2] = 2.0f / (Near - Far);

    Result.Elements[3][0] = (Left + Right) / (Left - Right);
    Result.Elements[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.Elements[3][2] = (Far + Near) / (Near - Far);

    return (Result);
}

HINLINE hmm_mat4
HMM_Perspective(float FOV, float AspectRatio, float Near, float Far)
{
    hmm_mat4 Result = HMM_Mat4d(1.0f);

    float TanThetaOver2 = HMM_TanF(FOV * (HMM_PI32 / 360.0f));
    
    Result.Elements[0][0] = 1.0f / TanThetaOver2;
    Result.Elements[1][1] = AspectRatio / TanThetaOver2;
    Result.Elements[2][3] = -1.0f;
    Result.Elements[2][2] = (Near + Far) / (Near - Far);
    Result.Elements[3][2] = (2.0f * Near * Far) / (Near - Far);
    Result.Elements[3][3] = 0.0f;

    return (Result);
}

HINLINE hmm_mat4
HMM_Translate(hmm_vec3 Translation)
{
    hmm_mat4 Result = HMM_Mat4d(1.0f);

    Result.Elements[3][0] = Translation.X;
    Result.Elements[3][1] = Translation.Y;
    Result.Elements[3][2] = Translation.Z;

    return (Result);
}

HINLINE hmm_mat4
HMM_Rotate(float Angle, hmm_vec3 Axis)
{
    hmm_mat4 Result = HMM_Mat4d(1.0f);
    
    Axis = HMM_NormalizeVec3(Axis);
    
    float SinTheta = HMM_SinF(HMM_ToRadians(Angle));
    float CosTheta = HMM_CosF(HMM_ToRadians(Angle));
    float CosValue = 1.0f - CosTheta;
    
    Result.Elements[0][0] = (Axis.X * Axis.X * CosValue) + CosTheta;
    Result.Elements[0][1] = (Axis.X * Axis.Y * CosValue) + (Axis.Z * SinTheta);
    Result.Elements[0][2] = (Axis.X * Axis.Z * CosValue) - (Axis.Y * SinTheta);
    
    Result.Elements[1][0] = (Axis.Y * Axis.X * CosValue) - (Axis.Z * SinTheta);
    Result.Elements[1][1] = (Axis.Y * Axis.Y * CosValue) + CosTheta;
    Result.Elements[1][2] = (Axis.Y * Axis.Z * CosValue) + (Axis.X * SinTheta);
    
    Result.Elements[2][0] = (Axis.Z * Axis.X * CosValue) + (Axis.Y * SinTheta);
    Result.Elements[2][1] = (Axis.Z * Axis.Y * CosValue) - (Axis.X * SinTheta);
    Result.Elements[2][2] = (Axis.Z * Axis.Z * CosValue) + CosTheta;
    
    return (Result);
}

HINLINE hmm_mat4
HMM_Scale(hmm_vec3 Scale)
{
    hmm_mat4 Result = HMM_Mat4d(1.0f);

    Result.Elements[0][0] = Scale.X;
    Result.Elements[1][1] = Scale.Y;
    Result.Elements[2][2] = Scale.Z;

    return (Result);
}

HINLINE hmm_mat4
HMM_LookAt(hmm_vec3 Eye, hmm_vec3 Center, hmm_vec3 Up)
{
    hmm_mat4 Result = {0};

    hmm_vec3 F = HMM_NormalizeVec3(HMM_SubtractVec3(Center, Eye));
    hmm_vec3 S = HMM_NormalizeVec3(HMM_Cross(F, Up));
    hmm_vec3 U = HMM_Cross(S, F);

    Result.Elements[0][0] = S.X;
    Result.Elements[0][1] = U.X;
    Result.Elements[0][2] = -F.X;

    Result.Elements[1][0] = S.Y;
    Result.Elements[1][1] = U.Y;
    Result.Elements[1][2] = -F.Y;

    Result.Elements[2][0] = S.Z;
    Result.Elements[2][1] = U.Z;
    Result.Elements[2][2] = -F.Z;

    Result.Elements[3][0] = -HMM_DotVec3(S, Eye);
    Result.Elements[3][1] = -HMM_DotVec3(U, Eye);
    Result.Elements[3][2] = HMM_DotVec3(F, Eye);
    Result.Elements[3][3] = 1.0f;

    return (Result);
}


HINLINE hmm_quaternion 
HMM_Quaternion(float X, float Y, float Z, float W)
{
    hmm_quaternion Result = {0};
    
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;
    
    return(Result);
}

HINLINE hmm_quaternion 
HMM_QuaternionV4(hmm_vec4 Vector)
{
    hmm_quaternion Result = {0};
    
    Result.X = Vector.X;
    Result.Y = Vector.Y;
    Result.Z = Vector.Z;
    Result.W = Vector.W;
    
    return(Result);    
}

HINLINE hmm_quaternion
HMM_AddQuaternion(hmm_quaternion Left, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    Result.W = Left.W + Right.W;

    return(Result);
}

HINLINE hmm_quaternion
HMM_SubtractQuaternion(hmm_quaternion Left, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    Result.W = Left.W - Right.W;

    return(Result);
}

HINLINE hmm_quaternion
HMM_MultiplyQuaternion(hmm_quaternion Left, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result.X = (Left.X * Right.W) + (Left.Y * Right.Z) - (Left.Z * Right.Y) + (Left.W * Right.X);
    Result.Y = (-Left.X * Right.Z) + (Left.Y * Right.W) + (Left.Z * Right.X) + (Left.W * Right.Y);
    Result.Z = (Left.X * Right.Y) - (Left.Y * Right.X) + (Left.Z * Right.W) + (Left.W * Right.Z);
    Result.W = (-Left.X * Right.X) - (Left.Y * Right.Y) - (Left.Z * Right.Z) + (Left.W * Right.W);

    return(Result);
}

HINLINE hmm_quaternion
HMM_MultiplyQuaternionF(hmm_quaternion Left, float Multiplicative)
{
    hmm_quaternion Result = {0};

    Result.X = Left.X * Multiplicative;
    Result.Y = Left.Y * Multiplicative;
    Result.Z = Left.Z * Multiplicative;
    Result.W = Left.W * Multiplicative;

    return(Result);
}

HINLINE hmm_quaternion
HMM_DivideQuaternionF(hmm_quaternion Left, float Dividend)
{
    hmm_quaternion Result = {0};

    Result.X = Left.X / Dividend;
    Result.Y = Left.Y / Dividend;
    Result.Z = Left.Z / Dividend;
    Result.W = Left.W / Dividend;

    return(Result);
}

HINLINE hmm_quaternion
HMM_InverseQuaternion(hmm_quaternion Left)
{
    hmm_quaternion Conjugate = {0};
    hmm_quaternion Result = {0};
    float Norm = 0;
    float NormSquared = 0;

    Conjugate.X = -Left.X;
    Conjugate.Y = -Left.Y;
    Conjugate.Z = -Left.Z;
    Conjugate.W = Left.W;

    Norm = HMM_SquareRootF(HMM_DotQuaternion(Left, Left));
    NormSquared = Norm * Norm;

    Result.X = Conjugate.X / NormSquared;
    Result.Y = Conjugate.Y / NormSquared;
    Result.Z = Conjugate.Z / NormSquared;
    Result.W = Conjugate.W / NormSquared;

    return(Result);
}

HINLINE float
HMM_DotQuaternion(hmm_quaternion Left, hmm_quaternion Right)
{
    float Result = 0.0f;

    Result = (Left.X * Right.X) + (Left.Y * Right.Y) + (Left.Z * Right.Z) + (Left.W * Right.W);

    return(Result);
}

HINLINE hmm_quaternion
HMM_NormalizeQuaternion(hmm_quaternion Left)
{
    hmm_quaternion Result = {0};

    float Length = HMM_SquareRootF(HMM_DotQuaternion(Left, Left));
    Result = HMM_DivideQuaternionF(Left, Length);

    return(Result);
}

HINLINE hmm_quaternion
HMM_NLerp(hmm_quaternion Left, float Time, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result.X = HMM_Lerp(Left.X, Time, Right.X);
    Result.Y = HMM_Lerp(Left.Y, Time, Right.Y);
    Result.Z = HMM_Lerp(Left.Z, Time, Right.Z);
    Result.W = HMM_Lerp(Left.W, Time, Right.W);

    Result = HMM_NormalizeQuaternion(Result);

    return(Result);
}

HINLINE hmm_quaternion
HMM_Slerp(hmm_quaternion Left, float Time, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};
    hmm_quaternion QuaternionLeft = {0};
    hmm_quaternion QuaternionRight = {0};

    float Cos_Theta = HMM_DotQuaternion(Left, Right);
    float Angle = HMM_ACosF(Cos_Theta);
    
    float S1 = HMM_SinF((1.0f - Time) * Angle);
    float S2 = HMM_SinF(Time * Angle);
    float Is = 1.0f / HMM_SinF(Angle);

    QuaternionLeft = HMM_MultiplyQuaternionF(Left, S1);
    QuaternionRight = HMM_MultiplyQuaternionF(Right, S2);

    Result = HMM_AddQuaternion(QuaternionLeft, QuaternionRight);
    Result = HMM_MultiplyQuaternionF(Result, Is);

    return(Result);
}

HINLINE hmm_mat4
HMM_QuaternionToMat4(hmm_quaternion Left)
{
    hmm_mat4 Result = {0};
    Result = HMM_Mat4d(1);

    hmm_quaternion NormalizedQuaternion = HMM_NormalizeQuaternion(Left);
    
    float XX, YY, ZZ,
          XY, XZ, YZ,
          WX, WY, WZ;

    XX = NormalizedQuaternion.X * NormalizedQuaternion.X;
    YY = NormalizedQuaternion.Y * NormalizedQuaternion.Y;
    ZZ = NormalizedQuaternion.Z * NormalizedQuaternion.Z;
    XY = NormalizedQuaternion.X * NormalizedQuaternion.Y;
    XZ = NormalizedQuaternion.X * NormalizedQuaternion.Z;
    YZ = NormalizedQuaternion.Y * NormalizedQuaternion.Z;
    WX = NormalizedQuaternion.W * NormalizedQuaternion.X;
    WY = NormalizedQuaternion.W * NormalizedQuaternion.Y;
    WZ = NormalizedQuaternion.W * NormalizedQuaternion.Z;

    Result.Elements[0][0] = 1.0f - 2.0f * (YY + ZZ);
    Result.Elements[0][1] = 2.0f * (XY + WZ);
    Result.Elements[0][2] = 2.0f * (XZ - WY);

    Result.Elements[1][0] = 2.0f * (XY - WZ);
    Result.Elements[1][1] = 1.0f - 2.0f * (XX + ZZ);
    Result.Elements[1][2] = 2.0f * (YZ + WX);

    Result.Elements[2][0] = 2.0f * (XZ + WY);
    Result.Elements[2][1] = 2.0f * (YZ - WX);
    Result.Elements[2][2] = 1.0f - 2.0f * (XX + YY);

    return(Result);
}

HINLINE hmm_quaternion
HMM_QuaternionFromAxisAngle(hmm_vec3 Axis, float AngleOfRotation)
{
    hmm_quaternion Result = {0};
    float AxisNorm = 0;
    float SineOfRotation = 0;
    hmm_vec3 RotatedVector = {0};

    AxisNorm = HMM_SquareRootF(HMM_DotVec3(Axis, Axis));
    SineOfRotation = HMM_SinF(AngleOfRotation / 2.0f);
    RotatedVector = HMM_MultiplyVec3f(Axis, SineOfRotation);

    Result.W = HMM_CosF(AngleOfRotation / 2.0f);
    Result.XYZ = HMM_DivideVec3f(RotatedVector, AxisNorm);

    return(Result);
}

#ifdef HANDMADE_MATH_CPP_MODE

HINLINE float 
HMM_Length(hmm_vec2 A)
{
    float Result = 0.0f;
    
    Result = HMM_LengthVec2(A);
    
    return(Result);
}

HINLINE float 
HMM_Length(hmm_vec3 A)
{
    float Result = 0.0f;
    
    Result = HMM_LengthVec3(A);
    
    return(Result);
}

HINLINE float 
HMM_Length(hmm_vec4 A)
{
    float Result = 0.0f;
    
    Result = HMM_LengthVec4(A);
    
    return(Result);
}

HINLINE float 
HMM_LengthSquared(hmm_vec2 A)
{
    float Result = 0.0f;
    
    Result = HMM_LengthSquaredVec2(A);
    
    return(Result);
}

HINLINE float 
HMM_LengthSquared(hmm_vec3 A)
{
    float Result = 0.0f;
    
    Result = HMM_LengthSquaredVec3(A);
    
    return(Result);
}

HINLINE float 
HMM_LengthSquared(hmm_vec4 A)
{
    float Result = 0.0f;
    
    Result = HMM_LengthSquaredVec4(A);
    
    return(Result);
}

HINLINE hmm_vec2 
HMM_Normalize(hmm_vec2 A)
{
    hmm_vec2 Result = {0};
    
    Result = HMM_NormalizeVec2(A);    
    
    return(Result);
}

HINLINE hmm_vec3 
HMM_Normalize(hmm_vec3 A)
{
    hmm_vec3 Result = {0};
    
    Result = HMM_NormalizeVec3(A);
    
    return(Result);
}

HINLINE hmm_vec4 
HMM_Normalize(hmm_vec4 A)
{
    hmm_vec4 Result = {0};
    
    Result = HMM_NormalizeVec4(A);
    
    return(Result);
}

HINLINE hmm_quaternion
HMM_Normalize(hmm_quaternion A)
{
    hmm_quaternion Result = {0};

    Result = HMM_NormalizeQuaternion(A);

    return(Result);
}

HINLINE float 
HMM_Dot(hmm_vec2 VecOne, hmm_vec2 VecTwo)
{
    float Result = 0;
    
    Result = HMM_DotVec2(VecOne, VecTwo);
    
    return(Result);
}

HINLINE float 
HMM_Dot(hmm_vec3 VecOne, hmm_vec3 VecTwo)
{
    float Result = 0;
    
    Result = HMM_DotVec3(VecOne, VecTwo);
    
    return(Result);
}

HINLINE float 
HMM_Dot(hmm_vec4 VecOne, hmm_vec4 VecTwo)
{
    float Result = 0;
    
    Result = HMM_DotVec4(VecOne, VecTwo);
    
    return(Result);
}

HINLINE float
HMM_Dot(hmm_quaternion QuatOne, hmm_quaternion QuatTwo)
{
    float Result = 0;

    Result = HMM_DotQuaternion(QuatOne, QuatTwo);

    return(Result);
}

HINLINE hmm_vec2
HMM_Add(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_AddVec2(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
HMM_Add(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_AddVec3(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
HMM_Add(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_AddVec4(Left, Right);
    return (Result);
}

HINLINE hmm_mat4
HMM_Add(hmm_mat4 Left, hmm_mat4 Right)
{
    hmm_mat4 Result = {0};

    Result = HMM_AddMat4(Left, Right);
    return (Result);
}

HINLINE hmm_quaternion
HMM_Add(hmm_quaternion Left, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_AddQuaternion(Left, Right);
    return(Result);
}

HINLINE hmm_vec2
HMM_Subtract(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_SubtractVec2(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
HMM_Subtract(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_SubtractVec3(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
HMM_Subtract(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_SubtractVec4(Left, Right);
    return (Result);
}

HINLINE hmm_mat4
HMM_Subtract(hmm_mat4 Left, hmm_mat4 Right)
{
    hmm_mat4 Result = {0};

    Result = HMM_SubtractMat4(Left, Right);
    return (Result);
}

HINLINE hmm_quaternion
HMM_Subtract(hmm_quaternion Left, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_SubtractQuaternion(Left, Right);
    return (Result);
}

HINLINE hmm_vec2
HMM_Multiply(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_MultiplyVec2(Left, Right);
    return (Result);
}

HINLINE hmm_vec2
HMM_Multiply(hmm_vec2 Left, float Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_MultiplyVec2f(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
HMM_Multiply(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_MultiplyVec3(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
HMM_Multiply(hmm_vec3 Left, float Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_MultiplyVec3f(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
HMM_Multiply(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_MultiplyVec4(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
HMM_Multiply(hmm_vec4 Left, float Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_MultiplyVec4f(Left, Right);
    return (Result);
}

HINLINE hmm_mat4
HMM_Multiply(hmm_mat4 Left, hmm_mat4 Right)
{
    hmm_mat4 Result = {0};

    Result = HMM_MultiplyMat4(Left, Right);
    return (Result);
}

HINLINE hmm_mat4
HMM_Multiply(hmm_mat4 Left, float Right)
{
    hmm_mat4 Result = {0};

    Result = HMM_MultiplyMat4f(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
HMM_Multiply(hmm_mat4 Matrix, hmm_vec4 Vector)
{
    hmm_vec4 Result = {0};

    Result = HMM_MultiplyMat4ByVec4(Matrix, Vector);
    return (Result);
}

HINLINE hmm_quaternion
HMM_Multiply(hmm_quaternion Left, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_MultiplyQuaternion(Left, Right);
    return (Result);
}

HINLINE hmm_quaternion
HMM_Multiply(hmm_quaternion Left, float Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_MultiplyQuaternionF(Left, Right);
    return (Result);
}

HINLINE hmm_quaternion
HMM_Multiply(float Left, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_MultiplyQuaternionF(Right, Left);
    return (Result);
}

HINLINE hmm_vec2
HMM_Divide(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_DivideVec2(Left, Right);
    return (Result);
}

HINLINE hmm_vec2
HMM_Divide(hmm_vec2 Left, float Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_DivideVec2f(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
HMM_Divide(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_DivideVec3(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
HMM_Divide(hmm_vec3 Left, float Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_DivideVec3f(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
HMM_Divide(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_DivideVec4(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
HMM_Divide(hmm_vec4 Left, float Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_DivideVec4f(Left, Right);
    return (Result);
}

HINLINE hmm_mat4
HMM_Divide(hmm_mat4 Left, float Right)
{
    hmm_mat4 Result = {0};

    Result = HMM_DivideMat4f(Left, Right);
    return (Result);
}

HINLINE hmm_quaternion
HMM_Divide(hmm_quaternion Left, float Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_DivideQuaternionF(Left, Right);
    return (Result);
}

HINLINE hmm_bool
HMM_Equals(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_bool Result = 0;

    Result = HMM_EqualsVec2(Left, Right);
    return (Result);
}

HINLINE hmm_bool
HMM_Equals(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_bool Result = 0;

    Result = HMM_EqualsVec3(Left, Right);
    return (Result);
}

HINLINE hmm_bool
HMM_Equals(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_bool Result = 0;

    Result = HMM_EqualsVec4(Left, Right);
    return (Result);
}

HINLINE hmm_vec2
operator+(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_Add(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
operator+(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_Add(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
operator+(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_Add(Left, Right);
    return (Result);
}

HINLINE hmm_mat4
operator+(hmm_mat4 Left, hmm_mat4 Right)
{
    hmm_mat4 Result = {0};

    Result = HMM_Add(Left, Right);
    return (Result);
}

HINLINE hmm_quaternion
operator+(hmm_quaternion Left, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_Add(Left, Right);
    return (Result);
}

HINLINE hmm_vec2
operator-(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_Subtract(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
operator-(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_Subtract(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
operator-(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_Subtract(Left, Right);
    return (Result);
}

HINLINE hmm_mat4
operator-(hmm_mat4 Left, hmm_mat4 Right)
{
    hmm_mat4 Result = {0};

    Result = HMM_Subtract(Left, Right);
    return (Result);
}

HINLINE hmm_quaternion
operator-(hmm_quaternion Left, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_Subtract(Left, Right);
    return (Result);
}

HINLINE hmm_vec2
operator*(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_Multiply(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
operator*(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_Multiply(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
operator*(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = HMM_Multiply(Left, Right);

    return (Result);
}

HINLINE hmm_vec2
operator*(hmm_vec2 Left, float Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_Multiply(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
operator*(hmm_vec3 Left, float Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_Multiply(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
operator*(hmm_vec4 Left, float Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_Multiply(Left, Right);
    return (Result);
}

HINLINE hmm_mat4
operator*(hmm_mat4 Left, float Right)
{
    hmm_mat4 Result = {0};

    Result = HMM_Multiply(Left, Right);
    return (Result);
}

HINLINE hmm_vec2
operator*(float Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_Multiply(Right, Left);
    return (Result);
}

HINLINE hmm_vec3
operator*(float Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_Multiply(Right, Left);
    return (Result);
}

HINLINE hmm_vec4
operator*(float Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_Multiply(Right, Left);
    return (Result);
}

HINLINE hmm_mat4
operator*(float Left, hmm_mat4 Right)
{
    hmm_mat4 Result = {0};

    Result = HMM_Multiply(Right, Left);
    return (Result);
}

HINLINE hmm_mat4
operator*(hmm_mat4 Left, hmm_mat4 Right)
{
    hmm_mat4 Result = {0};

    Result = HMM_Multiply(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
operator*(hmm_mat4 Matrix, hmm_vec4 Vector)
{
    hmm_vec4 Result = {0};

    Result = HMM_Multiply(Matrix, Vector);
    return (Result);
}

HINLINE hmm_quaternion
operator*(hmm_quaternion Left, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_Multiply(Left, Right);
    return (Result);
}

HINLINE hmm_quaternion
operator*(hmm_quaternion Left, float Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_Multiply(Left, Right);
    return (Result);
}

HINLINE hmm_quaternion
operator*(float Left, hmm_quaternion Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_Multiply(Right, Left);
    return (Result);
}

HINLINE hmm_vec2
operator/(hmm_vec2 Left, hmm_vec2 Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_Divide(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
operator/(hmm_vec3 Left, hmm_vec3 Right)
{
    hmm_vec3 Result = {0};
    
    Result = HMM_Divide(Left, Right);

    return (Result);
}

HINLINE hmm_vec4
operator/(hmm_vec4 Left, hmm_vec4 Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_Divide(Left, Right);
    return (Result);
}

HINLINE hmm_vec2
operator/(hmm_vec2 Left, float Right)
{
    hmm_vec2 Result = {0};

    Result = HMM_Divide(Left, Right);
    return (Result);
}

HINLINE hmm_vec3
operator/(hmm_vec3 Left, float Right)
{
    hmm_vec3 Result = {0};

    Result = HMM_Divide(Left, Right);
    return (Result);
}

HINLINE hmm_vec4
operator/(hmm_vec4 Left, float Right)
{
    hmm_vec4 Result = {0};

    Result = HMM_Divide(Left, Right);
    return (Result);
}

HINLINE hmm_mat4
operator/(hmm_mat4 Left, float Right)
{
    hmm_mat4 Result = {0};

    Result = HMM_Divide(Left, Right);
    return (Result);
}

HINLINE hmm_quaternion
operator/(hmm_quaternion Left, float Right)
{
    hmm_quaternion Result = {0};

    Result = HMM_Divide(Left, Right);
    return (Result);
}

HINLINE hmm_vec2 &
operator+=(hmm_vec2 &Left, hmm_vec2 Right)
{    
    return (Left = Left + Right);
}

HINLINE hmm_vec3 &
operator+=(hmm_vec3 &Left, hmm_vec3 Right)
{    
    return (Left = Left + Right);
}

HINLINE hmm_vec4 &
operator+=(hmm_vec4 &Left, hmm_vec4 Right)
{    
    return (Left = Left + Right);
}

HINLINE hmm_mat4 &
operator+=(hmm_mat4 &Left, hmm_mat4 Right)
{
    return (Left = Left + Right);
}

HINLINE hmm_quaternion &
operator+=(hmm_quaternion &Left, hmm_quaternion Right)
{
    return (Left = Left + Right);
}

HINLINE hmm_vec2 &
operator-=(hmm_vec2 &Left, hmm_vec2 Right)
{    
    return (Left = Left - Right);
}

HINLINE hmm_vec3 &
operator-=(hmm_vec3 &Left, hmm_vec3 Right)
{    
    return (Left = Left - Right);
}

HINLINE hmm_vec4 &
operator-=(hmm_vec4 &Left, hmm_vec4 Right)
{    
    return (Left = Left - Right);
}

HINLINE hmm_mat4 &
operator-=(hmm_mat4 &Left, hmm_mat4 Right)
{
    return (Left = Left - Right);
}

HINLINE hmm_quaternion &
operator-=(hmm_quaternion &Left, hmm_quaternion Right)
{
    return (Left = Left - Right);
}

HINLINE hmm_vec2 &
operator/=(hmm_vec2 &Left, hmm_vec2 Right)
{    
    return (Left = Left / Right);
}

HINLINE hmm_vec3 &
operator/=(hmm_vec3 &Left, hmm_vec3 Right)
{    
    return (Left = Left / Right);
}

HINLINE hmm_vec4 &
operator/=(hmm_vec4 &Left, hmm_vec4 Right)
{    
    return (Left = Left / Right);
}

HINLINE hmm_vec2 &
operator/=(hmm_vec2 &Left, float Right)
{
    return (Left = Left / Right);
}

HINLINE hmm_vec3 &
operator/=(hmm_vec3 &Left, float Right)
{
    return (Left = Left / Right);
}

HINLINE hmm_vec4 &
operator/=(hmm_vec4 &Left, float Right)
{
    return (Left = Left / Right);
}

HINLINE hmm_mat4 &
operator/=(hmm_mat4 &Left, float Right)
{
    return (Left = Left / Right);
}

HINLINE hmm_quaternion &
operator/=(hmm_quaternion &Left, float Right)
{
    return (Left = Left / Right);
}

HINLINE hmm_vec2 &
operator*=(hmm_vec2 &Left, hmm_vec2 Right)
{    
    return (Left = Left * Right);
}

HINLINE hmm_vec3 &
operator*=(hmm_vec3 &Left, hmm_vec3 Right)
{    
    return (Left = Left * Right);
}

HINLINE hmm_vec4 &
operator*=(hmm_vec4 &Left, hmm_vec4 Right)
{    
    return (Left = Left * Right);
}

HINLINE hmm_vec2 &
operator*=(hmm_vec2 &Left, float Right)
{
    return (Left = Left * Right);
}

HINLINE hmm_vec3 &
operator*=(hmm_vec3 &Left, float Right)
{
    return (Left = Left * Right);
}

HINLINE hmm_vec4 &
operator*=(hmm_vec4 &Left, float Right)
{
    return (Left = Left * Right);
}

HINLINE hmm_mat4 &
operator*=(hmm_mat4 &Left, float Right)
{
    return (Left = Left * Right);
}

HINLINE hmm_quaternion &
operator*=(hmm_quaternion &Left, float Right)
{
    return (Left = Left * Right);
}

HINLINE hmm_bool
operator==(hmm_vec2 Left, hmm_vec2 Right)
{
    return HMM_EqualsVec2(Left, Right);    
}

HINLINE hmm_bool
operator==(hmm_vec3 Left, hmm_vec3 Right)
{
    return HMM_EqualsVec3(Left, Right);    
}

HINLINE hmm_bool
operator==(hmm_vec4 Left, hmm_vec4 Right)
{
    return HMM_EqualsVec4(Left, Right);    
}


HINLINE hmm_bool 
operator!=(hmm_vec2 Left, hmm_vec2 Right)
{
    return !HMM_EqualsVec2(Left, Right);    
}

HINLINE hmm_bool 
operator!=(hmm_vec3 Left, hmm_vec3 Right)
{
    return !HMM_EqualsVec3(Left, Right);    
}

HINLINE hmm_bool 
operator!=(hmm_vec4 Left, hmm_vec4 Right)
{
    return !HMM_EqualsVec4(Left, Right);    
}

#endif /* HANDMADE_MATH_CPP_MODE */

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif /* HANDMADE_MATH_IMPLEMENTATION */

