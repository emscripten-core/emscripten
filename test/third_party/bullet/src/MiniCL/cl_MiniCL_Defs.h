/*
Bullet Continuous Collision Detection and Physics Library, Copyright (c) 2007 Erwin Coumans

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

*/

#include <float.h>
#include <math.h>
#include "LinearMath/btScalar.h"

#include "MiniCL/cl.h"


#define __kernel
#define __global
#define __local
#define get_global_id(a)	__guid_arg
#define get_local_id(a)		((__guid_arg) % gMiniCLNumOutstandingTasks)
#define get_local_size(a)	(gMiniCLNumOutstandingTasks)
#define get_group_id(a)		((__guid_arg) / gMiniCLNumOutstandingTasks)

static unsigned int as_uint(float val) { return *((unsigned int*)&val); }


#define CLK_LOCAL_MEM_FENCE		0x01
#define CLK_GLOBAL_MEM_FENCE	0x02

static void barrier(unsigned int a)
{
	// TODO : implement
}

//ATTRIBUTE_ALIGNED16(struct) float8
struct float8
{
	float s0;
	float s1;
	float s2;
	float s3;
	float s4;
	float s5;
	float s6;
	float s7;

	float8(float scalar)
	{
		s0=s1=s2=s3=s4=s5=s6=s7=scalar;
	}
};

//ATTRIBUTE_ALIGNED16(struct) float4
struct float4
{
	float x,y,z,w;
	float4() {}
	float4(float v) 
	{
		x = y = z = w = v; 
	}
	float4 operator*(const float4& other)
	{
		float4 tmp;
		tmp.x = x*other.x;
		tmp.y = y*other.y;
		tmp.z = z*other.z;
		tmp.w = w*other.w;
		return tmp;
	}

	float4 operator*(const float& other)
	{
		float4 tmp;
		tmp.x = x*other;
		tmp.y = y*other;
		tmp.z = z*other;
		tmp.w = w*other;
		return tmp;
	}

	

	float4& operator+=(const float4& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}

	float4& operator-=(const float4& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}

	float4& operator *=(float scalar)
	{
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
		return (*this);
	}

	
	
	
	
};

static float4 fabs(const float4& a)
{
	float4 tmp;
	tmp.x = a.x < 0.f ? 0.f  : a.x;
	tmp.y = a.y < 0.f ? 0.f  : a.y;
	tmp.z = a.z < 0.f ? 0.f  : a.z;
	tmp.w = a.w < 0.f ? 0.f  : a.w;
	return tmp;
}
static float4 operator+(const float4& a,const float4& b)
{
	float4 tmp;
	tmp.x = a.x + b.x;
	tmp.y = a.y + b.y;
	tmp.z = a.z + b.z;
	tmp.w = a.w + b.w;
	return tmp;
}


static float8 operator+(const float8& a,const float8& b)
{
	float8 tmp(0);
	tmp.s0  = a.s0 + b.s0;
	tmp.s1  = a.s1 + b.s1;
	tmp.s2  = a.s2 + b.s2;
	tmp.s3  = a.s3 + b.s3;
	tmp.s4  = a.s4 + b.s4;
	tmp.s5  = a.s5 + b.s5;
	tmp.s6  = a.s6 + b.s6;
	tmp.s7  = a.s7 + b.s7;
	return tmp;
}


static float4 operator-(const float4& a,const float4& b)
{
	float4 tmp;
	tmp.x = a.x - b.x;
	tmp.y = a.y - b.y;
	tmp.z = a.z - b.z;
	tmp.w = a.w - b.w;
	return tmp;
}

static float8 operator-(const float8& a,const float8& b)
{
	float8 tmp(0);
	tmp.s0  = a.s0 - b.s0;
	tmp.s1  = a.s1 - b.s1;
	tmp.s2  = a.s2 - b.s2;
	tmp.s3  = a.s3 - b.s3;
	tmp.s4  = a.s4 - b.s4;
	tmp.s5  = a.s5 - b.s5;
	tmp.s6  = a.s6 - b.s6;
	tmp.s7  = a.s7 - b.s7;
	return tmp;
}

static float4 operator*(float a,const float4& b)
{
	float4 tmp;
	tmp.x = a * b.x;
	tmp.y = a * b.y;
	tmp.z = a * b.z;
	tmp.w = a * b.w;
	return tmp;
}

static float4 operator/(const float4& b,float a)
{
	float4 tmp;
	tmp.x = b.x/a;
	tmp.y = b.y/a;
	tmp.z = b.z/a;
	tmp.w = b.w/a;
	return tmp;
}




static float dot(const float4&a ,const float4& b)
{
	float4 tmp;
	tmp.x = a.x*b.x;
	tmp.y = a.y*b.y;
	tmp.z = a.z*b.z;
	tmp.w = a.w*b.w;
	return tmp.x+tmp.y+tmp.z+tmp.w;
}

static float length(const float4&a)
{
	float l = sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
	return l;
}

static float4 normalize(const float4&a)
{
	float4 tmp;
	float l = length(a);
	tmp = 1.f/l*a;
	return tmp;
}



static float4 cross(const float4&a ,const float4& b)
{
	float4 tmp;
	tmp.x =  a.y*b.z - a.z*b.y;
	tmp.y = -a.x*b.z + a.z*b.x;
	tmp.z =  a.x*b.y - a.y*b.x;
	tmp.w = 0.f;
	return tmp;
}

static float max(float a, float b) 
{
	return (a >= b) ? a : b;
}


static float min(float a, float b) 
{
	return (a <= b) ? a : b;
}

static float fmax(float a, float b) 
{
	return (a >= b) ? a : b;
}

static float fmin(float a, float b) 
{
	return (a <= b) ? a : b;
}

struct int2
{
	int x,y;
};

struct uint2
{
	unsigned int x,y;
};

//typedef int2 uint2;

typedef unsigned int uint;

struct int4
{
	int x,y,z,w;
};

struct uint4
{
	unsigned int x,y,z,w;
	uint4() {}
	uint4(uint val) { x = y = z = w = val; }
	uint4& operator+=(const uint4& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}
};
static uint4 operator+(const uint4& a,const uint4& b)
{
	uint4 tmp;
	tmp.x = a.x + b.x;
	tmp.y = a.y + b.y;
	tmp.z = a.z + b.z;
	tmp.w = a.w + b.w;
	return tmp;
}
static uint4 operator-(const uint4& a,const uint4& b)
{
	uint4 tmp;
	tmp.x = a.x - b.x;
	tmp.y = a.y - b.y;
	tmp.z = a.z - b.z;
	tmp.w = a.w - b.w;
	return tmp;
}

#define native_sqrt sqrtf
#define native_sin sinf
#define native_cos cosf
#define native_powr powf

#define GUID_ARG ,int __guid_arg
#define GUID_ARG_VAL ,__guid_arg


#define as_int(a) (*((int*)&(a)))

extern "C" int gMiniCLNumOutstandingTasks;
//	extern "C" void __kernel_func();


