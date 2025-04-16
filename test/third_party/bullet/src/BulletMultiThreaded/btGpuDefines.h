/*
Bullet Continuous Collision Detection and Physics Library, http://bulletphysics.org
Copyright (C) 2006, 2009 Sony Computer Entertainment Inc. 

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/



// definitions for "GPU on CPU" code


#ifndef BT_GPU_DEFINES_H
#define BT_GPU_DEFINES_H

typedef unsigned int uint;

struct int2
{
	int x, y;
};

struct uint2
{
	unsigned int x, y;
};

struct int3
{
	int x, y, z;
};

struct uint3
{
	unsigned int x, y, z;
};

struct float4
{
	float x, y, z, w;
};

struct float3
{
	float x, y, z;
};


#define BT_GPU___device__ inline
#define BT_GPU___devdata__
#define BT_GPU___constant__
#define BT_GPU_max(a, b) ((a) > (b) ? (a) : (b))
#define BT_GPU_min(a, b) ((a) < (b) ? (a) : (b))
#define BT_GPU_params s3DGridBroadphaseParams
#define BT_GPU___mul24(a, b) ((a)*(b))
#define BT_GPU___global__ inline
#define BT_GPU___shared__ static
#define BT_GPU___syncthreads()
#define CUDART_PI_F SIMD_PI

static inline uint2 bt3dGrid_make_uint2(unsigned int x, unsigned int y)
{
  uint2 t; t.x = x; t.y = y; return t;
}
#define BT_GPU_make_uint2(x, y) bt3dGrid_make_uint2(x, y)

static inline int3 bt3dGrid_make_int3(int x, int y, int z)
{
  int3 t; t.x = x; t.y = y; t.z = z; return t;
}
#define BT_GPU_make_int3(x, y, z) bt3dGrid_make_int3(x, y, z)

static inline float3 bt3dGrid_make_float3(float x, float y, float z)
{
  float3 t; t.x = x; t.y = y; t.z = z; return t;
}
#define BT_GPU_make_float3(x, y, z) bt3dGrid_make_float3(x, y, z)

static inline float3 bt3dGrid_make_float34(float4 f)
{
  float3 t; t.x = f.x; t.y = f.y; t.z = f.z; return t;
}
#define BT_GPU_make_float34(f) bt3dGrid_make_float34(f)

static inline float3 bt3dGrid_make_float31(float f)
{
  float3 t; t.x = t.y = t.z = f; return t;
}
#define BT_GPU_make_float31(x) bt3dGrid_make_float31(x)

static inline float4 bt3dGrid_make_float42(float3 v, float f)
{
  float4 t; t.x = v.x; t.y = v.y; t.z = v.z; t.w = f; return t;
}
#define BT_GPU_make_float42(a, b) bt3dGrid_make_float42(a, b) 

static inline float4 bt3dGrid_make_float44(float a, float b, float c, float d)
{
  float4 t; t.x = a; t.y = b; t.z = c; t.w = d; return t;
}
#define BT_GPU_make_float44(a, b, c, d) bt3dGrid_make_float44(a, b, c, d) 

inline int3 operator+(int3 a, int3 b)
{
    return bt3dGrid_make_int3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline float4 operator+(const float4& a, const float4& b)
{
	float4 r; r.x = a.x+b.x; r.y = a.y+b.y; r.z = a.z+b.z; r.w = a.w+b.w; return r;
}
inline float4 operator*(const float4& a, float fact)
{
	float4 r; r.x = a.x*fact; r.y = a.y*fact; r.z = a.z*fact; r.w = a.w*fact; return r;
}
inline float4 operator*(float fact, float4& a)
{
	return (a * fact);
}
inline float4& operator*=(float4& a, float fact)
{
	a = fact * a;
	return a;
}
inline float4& operator+=(float4& a, const float4& b)
{
	a = a + b;
	return a;
}

inline float3 operator+(const float3& a, const float3& b)
{
	float3 r; r.x = a.x+b.x; r.y = a.y+b.y; r.z = a.z+b.z; return r;
}
inline float3 operator-(const float3& a, const float3& b)
{
	float3 r; r.x = a.x-b.x; r.y = a.y-b.y; r.z = a.z-b.z; return r;
}
static inline float bt3dGrid_dot(float3& a, float3& b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
}
#define BT_GPU_dot(a,b) bt3dGrid_dot(a,b)

static inline float bt3dGrid_dot4(float4& a, float4& b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w;
}
#define BT_GPU_dot4(a,b) bt3dGrid_dot4(a,b)

static inline float3 bt3dGrid_cross(const float3& a, const float3& b)
{
	float3 r; r.x = a.y*b.z-a.z*b.y; r.y = -a.x*b.z+a.z*b.x; r.z = a.x*b.y-a.y*b.x;	return r;
}
#define BT_GPU_cross(a,b) bt3dGrid_cross(a,b)


inline float3 operator*(const float3& a, float fact)
{
	float3 r; r.x = a.x*fact; r.y = a.y*fact; r.z = a.z*fact; return r;
}


inline float3& operator+=(float3& a, const float3& b)
{
	a = a + b;
	return a;
}
inline float3& operator-=(float3& a, const float3& b)
{
	a = a - b;
	return a;
}
inline float3& operator*=(float3& a, float fact)
{
	a = a * fact;
	return a;
}
inline float3 operator-(const float3& v)
{
	float3 r; r.x = -v.x; r.y = -v.y; r.z = -v.z; return r;
}


#define BT_GPU_FETCH(a, b) a[b]
#define BT_GPU_FETCH4(a, b) a[b]
#define BT_GPU_PREF(func) btGpu_##func
#define BT_GPU_SAFE_CALL(func) func
#define BT_GPU_Memset memset
#define BT_GPU_MemcpyToSymbol(a, b, c) memcpy(&a, b, c)
#define BT_GPU_BindTexture(a, b, c, d)
#define BT_GPU_UnbindTexture(a)

static uint2 s_blockIdx, s_blockDim, s_threadIdx;
#define BT_GPU_blockIdx s_blockIdx
#define BT_GPU_blockDim s_blockDim
#define BT_GPU_threadIdx s_threadIdx
#define BT_GPU_EXECKERNEL(numb, numt, kfunc, args) {s_blockDim.x=numt;for(int nb=0;nb<numb;nb++){s_blockIdx.x=nb;for(int nt=0;nt<numt;nt++){s_threadIdx.x=nt;kfunc args;}}}

#define BT_GPU_CHECK_ERROR(s)


#endif //BT_GPU_DEFINES_H
