#include "float_math.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <float.h>

/*----------------------------------------------------------------------
		Copyright (c) 2004 Open Dynamics Framework Group
					www.physicstools.org
		All rights reserved.

		Redistribution and use in source and binary forms, with or without modification, are permitted provided
		that the following conditions are met:

		Redistributions of source code must retain the above copyright notice, this list of conditions
		and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.

		Neither the name of the Open Dynamics Framework Group nor the names of its contributors may
		be used to endorse or promote products derived from this software without specific prior written permission.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES,
		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
		DISCLAIMED. IN NO EVENT SHALL THE INTEL OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
		LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
		IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
		THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

// http://codesuppository.blogspot.com
//
// mailto: jratcliff@infiniplex.net
//
// http://www.amillionpixels.us
//

#include "cd_hull.h"

using namespace ConvexDecomposition;

/*----------------------------------------------------------------------
		Copyright (c) 2004 Open Dynamics Framework Group
					www.physicstools.org
		All rights reserved.

		Redistribution and use in source and binary forms, with or without modification, are permitted provided
		that the following conditions are met:

		Redistributions of source code must retain the above copyright notice, this list of conditions
		and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.

		Neither the name of the Open Dynamics Framework Group nor the names of its contributors may
		be used to endorse or promote products derived from this software without specific prior written permission.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES,
		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
		DISCLAIMED. IN NO EVENT SHALL THE INTEL OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
		LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
		IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
		THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

#define PI 3.14159264f

//*****************************************************
//*****************************************************
//********* Stan Melax's vector math template needed
//********* to use his hull building code.
//*****************************************************
//*****************************************************

#define DEG2RAD (PI / 180.0f)
#define RAD2DEG (180.0f / PI)
#define SQRT_OF_2 (1.4142135f)
#define OFFSET(Class,Member)  (((char*) (&(((Class*)NULL)-> Member )))- ((char*)NULL))

namespace ConvexDecomposition
{


int    argmin(float a[],int n);
float  sqr(float a);
float  clampf(float a) ;
float  Round(float a,float precision);
float  Interpolate(const float &f0,const float &f1,float alpha) ;

template <class T>
void Swap(T &a,T &b)
{
	T tmp = a;
	a=b;
	b=tmp;
}



template <class T>
T Max(const T &a,const T &b)
{
	return (a>b)?a:b;
}

template <class T>
T Min(const T &a,const T &b) 
{
	return (a<b)?a:b;
}

//----------------------------------

class int3  
{
public:
	int x,y,z;
	int3(){};
	int3(int _x,int _y, int _z){x=_x;y=_y;z=_z;}
	const int& operator[](int i) const {return (&x)[i];}
	int& operator[](int i) {return (&x)[i];}
};


//-------- 2D --------

class float2
{
public:
	float x,y;
	float2(){x=0;y=0;};
	float2(float _x,float _y){x=_x;y=_y;}
	float& operator[](int i) {assert(i>=0&&i<2);return ((float*)this)[i];}
	const float& operator[](int i) const {assert(i>=0&&i<2);return ((float*)this)[i];}
};
inline float2 operator-( const float2& a, const float2& b ){return float2(a.x-b.x,a.y-b.y);}
inline float2 operator+( const float2& a, const float2& b ){return float2(a.x+b.x,a.y+b.y);}

//--------- 3D ---------

class float3 // 3D
{
	public:
	float x,y,z;
	float3(){x=0;y=0;z=0;};
	float3(float _x,float _y,float _z){x=_x;y=_y;z=_z;};
	//operator float *() { return &x;};
	float& operator[](int i) {assert(i>=0&&i<3);return ((float*)this)[i];}
	const float& operator[](int i) const {assert(i>=0&&i<3);return ((float*)this)[i];}
#	ifdef PLUGIN_3DSMAX
	float3(const Point3 &p):x(p.x),y(p.y),z(p.z){}
	operator Point3(){return *((Point3*)this);}
#	endif
};


float3& operator+=( float3 &a, const float3& b );
float3& operator-=( float3 &a ,const float3& b );
float3& operator*=( float3 &v ,const float s );
float3& operator/=( float3 &v, const float s );

float  magnitude( const float3& v );
float3 normalize( const float3& v );
float3 safenormalize(const float3 &v);
float3 vabs(const float3 &v);
float3 operator+( const float3& a, const float3& b );
float3 operator-( const float3& a, const float3& b );
float3 operator-( const float3& v );
float3 operator*( const float3& v, const float s );
float3 operator*( const float s, const float3& v );
float3 operator/( const float3& v, const float s );
inline int operator==( const float3 &a, const float3 &b ) { return (a.x==b.x && a.y==b.y && a.z==b.z); }
inline int operator!=( const float3 &a, const float3 &b ) { return (a.x!=b.x || a.y!=b.y || a.z!=b.z); }
// due to ambiguity and inconsistent standards ther are no overloaded operators for mult such as va*vb.
float  dot( const float3& a, const float3& b );
float3 cmul( const float3 &a, const float3 &b);
float3 cross( const float3& a, const float3& b );
float3 Interpolate(const float3 &v0,const float3 &v1,float alpha);
float3 Round(const float3& a,float precision);
float3	VectorMax(const float3 &a, const float3 &b);
float3	VectorMin(const float3 &a, const float3 &b);



class float3x3
{
	public:
	float3 x,y,z;  // the 3 rows of the Matrix
	float3x3(){}
	float3x3(float xx,float xy,float xz,float yx,float yy,float yz,float zx,float zy,float zz):x(xx,xy,xz),y(yx,yy,yz),z(zx,zy,zz){}
	float3x3(float3 _x,float3 _y,float3 _z):x(_x),y(_y),z(_z){}
	float3&       operator[](int i)       {assert(i>=0&&i<3);return (&x)[i];}
	const float3& operator[](int i) const {assert(i>=0&&i<3);return (&x)[i];}
	float&        operator()(int r, int c)       {assert(r>=0&&r<3&&c>=0&&c<3);return ((&x)[r])[c];}
	const float&  operator()(int r, int c) const {assert(r>=0&&r<3&&c>=0&&c<3);return ((&x)[r])[c];}
}; 
float3x3 Transpose( const float3x3& m );
float3   operator*( const float3& v  , const float3x3& m  );
float3   operator*( const float3x3& m , const float3& v   );
float3x3 operator*( const float3x3& m , const float& s   );
float3x3 operator*( const float3x3& ma, const float3x3& mb );
float3x3 operator/( const float3x3& a, const float& s ) ;
float3x3 operator+( const float3x3& a, const float3x3& b );
float3x3 operator-( const float3x3& a, const float3x3& b );
float3x3 &operator+=( float3x3& a, const float3x3& b );
float3x3 &operator-=( float3x3& a, const float3x3& b );
float3x3 &operator*=( float3x3& a, const float& s );
float    Determinant(const float3x3& m );
float3x3 Inverse(const float3x3& a);  // its just 3x3 so we simply do that cofactor method


//-------- 4D Math --------

class float4
{
public:
	float x,y,z,w;
	float4(){x=0;y=0;z=0;w=0;};
	float4(float _x,float _y,float _z,float _w){x=_x;y=_y;z=_z;w=_w;}
	float4(const float3 &v,float _w){x=v.x;y=v.y;z=v.z;w=_w;}
	//operator float *() { return &x;};
	float& operator[](int i) {assert(i>=0&&i<4);return ((float*)this)[i];}
	const float& operator[](int i) const {assert(i>=0&&i<4);return ((float*)this)[i];}
	const float3& xyz() const { return *((float3*)this);}
	float3&       xyz()       { return *((float3*)this);}
};


struct D3DXMATRIX;

class float4x4
{
	public:
	float4 x,y,z,w;  // the 4 rows
	float4x4(){}
	float4x4(const float4 &_x, const float4 &_y, const float4 &_z, const float4 &_w):x(_x),y(_y),z(_z),w(_w){}
	float4x4(float m00, float m01, float m02, float m03,
						float m10, float m11, float m12, float m13,
				float m20, float m21, float m22, float m23, 
				float m30, float m31, float m32, float m33 )
			:x(m00,m01,m02,m03),y(m10,m11,m12,m13),z(m20,m21,m22,m23),w(m30,m31,m32,m33){}
	float&       operator()(int r, int c)       {assert(r>=0&&r<4&&c>=0&&c<4);return ((&x)[r])[c];}
	const float& operator()(int r, int c) const {assert(r>=0&&r<4&&c>=0&&c<4);return ((&x)[r])[c];}
		operator       float* ()       {return &x.x;}
		operator const float* () const {return &x.x;}
	operator       struct D3DXMATRIX* ()       { return (struct D3DXMATRIX*) this;}
	operator const struct D3DXMATRIX* () const { return (struct D3DXMATRIX*) this;}
};


int     operator==( const float4 &a, const float4 &b );
float4 Homogenize(const float3 &v3,const float &w=1.0f); // Turns a 3D float3 4D vector4 by appending w
float4 cmul( const float4 &a, const float4 &b);
float4 operator*( const float4 &v, float s);
float4 operator*( float s, const float4 &v);
float4 operator+( const float4 &a, const float4 &b);
float4 operator-( const float4 &a, const float4 &b);
float4x4 operator*( const float4x4& a, const float4x4& b );
float4 operator*( const float4& v, const float4x4& m );
float4x4 Inverse(const float4x4 &m);
float4x4 MatrixRigidInverse(const float4x4 &m);
float4x4 MatrixTranspose(const float4x4 &m);
float4x4 MatrixPerspectiveFov(float fovy, float Aspect, float zn, float zf );
float4x4 MatrixTranslation(const float3 &t);
float4x4 MatrixRotationZ(const float angle_radians);
float4x4 MatrixLookAt(const float3& eye, const float3& at, const float3& up);
int     operator==( const float4x4 &a, const float4x4 &b );


//-------- Quaternion ------------

class Quaternion :public float4
{
 public:
	Quaternion() { x = y = z = 0.0f; w = 1.0f; }
	Quaternion( float3 v, float t ) { v = normalize(v); w = cosf(t/2.0f); v = v*sinf(t/2.0f); x = v.x; y = v.y; z = v.z; }
	Quaternion(float _x, float _y, float _z, float _w){x=_x;y=_y;z=_z;w=_w;}
	float angle() const { return acosf(w)*2.0f; }
	float3 axis() const { float3 a(x,y,z); if(fabsf(angle())<0.0000001f) return float3(1,0,0); return a*(1/sinf(angle()/2.0f)); }
	float3 xdir() const { return float3( 1-2*(y*y+z*z),  2*(x*y+w*z),  2*(x*z-w*y) ); }
	float3 ydir() const { return float3(   2*(x*y-w*z),1-2*(x*x+z*z),  2*(y*z+w*x) ); }
	float3 zdir() const { return float3(   2*(x*z+w*y),  2*(y*z-w*x),1-2*(x*x+y*y) ); }
	float3x3 getmatrix() const { return float3x3( xdir(), ydir(), zdir() ); }
	operator float3x3() { return getmatrix(); }
	void Normalize();
};

Quaternion& operator*=(Quaternion& a, float s );
Quaternion	operator*( const Quaternion& a, float s );
Quaternion	operator*( const Quaternion& a, const Quaternion& b);
Quaternion	operator+( const Quaternion& a, const Quaternion& b );
Quaternion	normalize( Quaternion a );
float		dot( const Quaternion &a, const Quaternion &b );
float3		operator*( const Quaternion& q, const float3& v );
float3		operator*( const float3& v, const Quaternion& q );
Quaternion	slerp( Quaternion a, const Quaternion& b, float interp );
Quaternion  Interpolate(const Quaternion &q0,const Quaternion &q1,float alpha); 
Quaternion  RotationArc(float3 v0, float3 v1 );  // returns quat q where q*v0=v1
Quaternion  Inverse(const Quaternion &q);
float4x4     MatrixFromQuatVec(const Quaternion &q, const float3 &v);


//------ Euler Angle -----

Quaternion YawPitchRoll( float yaw, float pitch, float roll );
float Yaw( const Quaternion& q );
float Pitch( const Quaternion& q );
float Roll( Quaternion q );
float Yaw( const float3& v );
float Pitch( const float3& v );


//------- Plane ----------

class Plane
{
	public:
	float3	normal;
	float	dist;   // distance below origin - the D from plane equasion Ax+By+Cz+D=0
			Plane(const float3 &n,float d):normal(n),dist(d){}
			Plane():normal(),dist(0){}
	void	Transform(const float3 &position, const Quaternion &orientation);
};

inline Plane PlaneFlip(const Plane &plane){return Plane(-plane.normal,-plane.dist);}
inline int operator==( const Plane &a, const Plane &b ) { return (a.normal==b.normal && a.dist==b.dist); }
inline int coplanar( const Plane &a, const Plane &b ) { return (a==b || a==PlaneFlip(b)); }


//--------- Utility Functions ------

float3  PlaneLineIntersection(const Plane &plane, const float3 &p0, const float3 &p1);
float3  PlaneProject(const Plane &plane, const float3 &point);
float3  LineProject(const float3 &p0, const float3 &p1, const float3 &a);  // projects a onto infinite line p0p1
float   LineProjectTime(const float3 &p0, const float3 &p1, const float3 &a);
float3  ThreePlaneIntersection(const Plane &p0,const Plane &p1, const Plane &p2);
int     PolyHit(const float3 *vert,const int n,const float3 &v0, const float3 &v1, float3 *impact=NULL, float3 *normal=NULL);
int     BoxInside(const float3 &p,const float3 &bmin, const float3 &bmax) ;
int     BoxIntersect(const float3 &v0, const float3 &v1, const float3 &bmin, const float3 &bmax, float3 *impact);
float   DistanceBetweenLines(const float3 &ustart, const float3 &udir, const float3 &vstart, const float3 &vdir, float3 *upoint=NULL, float3 *vpoint=NULL);
float3  TriNormal(const float3 &v0, const float3 &v1, const float3 &v2);
float3  NormalOf(const float3 *vert, const int n);
Quaternion VirtualTrackBall(const float3 &cop, const float3 &cor, const float3 &dir0, const float3 &dir1);


float   sqr(float a) {return a*a;}
float   clampf(float a) {return Min(1.0f,Max(0.0f,a));}


float Round(float a,float precision)
{
	return floorf(0.5f+a/precision)*precision;
}


float Interpolate(const float &f0,const float &f1,float alpha)
{
	return f0*(1-alpha) + f1*alpha;
}


int     argmin(float a[],int n)
{
	int r=0;
	for(int i=1;i<n;i++) 
		{
		if(a[i]<a[r]) 
				{
			r = i;
		}
	}
	return r;
}



//------------ float3 (3D) --------------



float3 operator+( const float3& a, const float3& b ) 
{
	return float3(a.x+b.x, a.y+b.y, a.z+b.z); 
}


float3 operator-( const float3& a, const float3& b )
{
	return float3( a.x-b.x, a.y-b.y, a.z-b.z );
}


float3 operator-( const float3& v )                     
{
	return float3( -v.x, -v.y, -v.z );
}


float3 operator*( const float3& v, float s )      
{
	return float3( v.x*s, v.y*s, v.z*s );
}


float3 operator*( float s, const float3& v )      
{
	return float3( v.x*s, v.y*s, v.z*s ); 
}


float3 operator/( const float3& v, float s )
{ 
	return v*(1.0f/s); 
}

float  dot( const float3& a, const float3& b )    
{
	return a.x*b.x + a.y*b.y + a.z*b.z; 
}

float3 cmul( const float3 &v1, const float3 &v2) 
{ 
	return float3(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z); 
}


float3 cross( const float3& a, const float3& b )
{
		return float3( a.y*b.z - a.z*b.y,
									 a.z*b.x - a.x*b.z,
									 a.x*b.y - a.y*b.x );
}




float3& operator+=( float3& a , const float3& b )
{
		a.x += b.x;
		a.y += b.y;
		a.z += b.z;
		return a;
}


float3& operator-=( float3& a , const float3& b )
{
		a.x -= b.x;
		a.y -= b.y;
		a.z -= b.z;
		return a;
}


float3& operator*=(float3& v , float s )
{
		v.x *= s;
		v.y *= s;
		v.z *= s;
		return v;
}


float3& operator/=(float3& v , float s )
{
		float sinv = 1.0f / s;
		v.x *= sinv;
		v.y *= sinv;
		v.z *= sinv;
		return v;
}

float3 vabs(const float3 &v)
{
	return float3(fabsf(v.x),fabsf(v.y),fabsf(v.z));
}


float magnitude( const float3& v )
{
		return sqrtf(sqr(v.x) + sqr( v.y)+ sqr(v.z));
}



float3 normalize( const float3 &v )
{
	// this routine, normalize, is ok, provided magnitude works!!
		float d=magnitude(v);
		if (d==0)
		{
		printf("Cant normalize ZERO vector\n");
		assert(0);// yes this could go here
		d=0.1f;
	}
	d = 1/d;
	return float3(v.x*d,v.y*d,v.z*d);
}

float3 safenormalize(const float3 &v)
{
	if(magnitude(v)<=0.0f)
	{
		return float3(1,0,0);
	}
	return normalize(v);
}

float3 Round(const float3 &a,float precision)
{
	return float3(Round(a.x,precision),Round(a.y,precision),Round(a.z,precision));
}


float3 Interpolate(const float3 &v0,const float3 &v1,float alpha) 
{
	return v0*(1-alpha) + v1*alpha;
}

float3 VectorMin(const float3 &a,const float3 &b)
{
	return float3(Min(a.x,b.x),Min(a.y,b.y),Min(a.z,b.z));
}
float3 VectorMax(const float3 &a,const float3 &b)
{
	return float3(Max(a.x,b.x),Max(a.y,b.y),Max(a.z,b.z));
}

// the statement v1*v2 is ambiguous since there are 3 types
// of vector multiplication
//  - componantwise (for example combining colors)
//  - dot product
//  - cross product
// Therefore we never declare/implement this function.
// So we will never see:  float3 operator*(float3 a,float3 b) 




//------------ float3x3 ---------------
float Determinant(const float3x3 &m)
{
	return  m.x.x*m.y.y*m.z.z + m.y.x*m.z.y*m.x.z + m.z.x*m.x.y*m.y.z
			 -m.x.x*m.z.y*m.y.z - m.y.x*m.x.y*m.z.z - m.z.x*m.y.y*m.x.z ;
}

float3x3 Inverse(const float3x3 &a)
{
	float3x3 b;
	float d=Determinant(a);
	assert(d!=0);
	for(int i=0;i<3;i++) 
		{
		for(int j=0;j<3;j++) 
				{
			int i1=(i+1)%3;
			int i2=(i+2)%3;
			int j1=(j+1)%3;
			int j2=(j+2)%3;
			// reverse indexs i&j to take transpose
			b[j][i] = (a[i1][j1]*a[i2][j2]-a[i1][j2]*a[i2][j1])/d;
		}
	}
	// Matrix check=a*b; // Matrix 'check' should be the identity (or close to it)
	return b;
}


float3x3 Transpose( const float3x3& m )
{
	return float3x3( float3(m.x.x,m.y.x,m.z.x),
					float3(m.x.y,m.y.y,m.z.y),
					float3(m.x.z,m.y.z,m.z.z));
}


float3 operator*(const float3& v , const float3x3 &m ) { 
	return float3((m.x.x*v.x + m.y.x*v.y + m.z.x*v.z), 
					(m.x.y*v.x + m.y.y*v.y + m.z.y*v.z), 
					(m.x.z*v.x + m.y.z*v.y + m.z.z*v.z));
}
float3 operator*(const float3x3 &m,const float3& v  ) {
	return float3(dot(m.x,v),dot(m.y,v),dot(m.z,v));
}


float3x3 operator*( const float3x3& a, const float3x3& b )
{ 
	return float3x3(a.x*b,a.y*b,a.z*b); 
}

float3x3 operator*( const float3x3& a, const float& s )  
{ 
	return float3x3(a.x*s, a.y*s ,a.z*s); 
}
float3x3 operator/( const float3x3& a, const float& s )  
{ 
	float t=1/s;
	return float3x3(a.x*t, a.y*t ,a.z*t); 
}
float3x3 operator+( const float3x3& a, const float3x3& b )
{
	return float3x3(a.x+b.x, a.y+b.y, a.z+b.z);
}
float3x3 operator-( const float3x3& a, const float3x3& b )
{
	return float3x3(a.x-b.x, a.y-b.y, a.z-b.z);
}
float3x3 &operator+=( float3x3& a, const float3x3& b )
{
	a.x+=b.x;
	a.y+=b.y;
	a.z+=b.z;
	return a;
}
float3x3 &operator-=( float3x3& a, const float3x3& b )
{
	a.x-=b.x;
	a.y-=b.y;
	a.z-=b.z;
	return a;
}
float3x3 &operator*=( float3x3& a, const float& s )
{
	a.x*=s;
	a.y*=s;
	a.z*=s;
	return a;
}



float3 ThreePlaneIntersection(const Plane &p0,const Plane &p1, const Plane &p2){
	float3x3 mp =Transpose(float3x3(p0.normal,p1.normal,p2.normal));
	float3x3 mi = Inverse(mp);
	float3 b(p0.dist,p1.dist,p2.dist);
	return -b * mi;
}


//--------------- 4D ----------------

float4   operator*( const float4&   v, const float4x4& m )
{
	return v.x*m.x + v.y*m.y + v.z*m.z + v.w*m.w; // yes this actually works
}

int operator==( const float4 &a, const float4 &b ) 
{
	return (a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w); 
}


//  Dont implement m*v for now, since that might confuse us
//  All our transforms are based on multiplying the "row" vector on the left
//float4   operator*(const float4x4& m , const float4&   v )
//{
//	return float4(dot(v,m.x),dot(v,m.y),dot(v,m.z),dot(v,m.w));
//}



float4 cmul( const float4 &a, const float4 &b) 
{
	return float4(a.x*b.x,a.y*b.y,a.z*b.z,a.w*b.w);
}


float4 operator*( const float4 &v, float s) 
{
	return float4(v.x*s,v.y*s,v.z*s,v.w*s);
}


float4 operator*( float s, const float4 &v) 
{
	return float4(v.x*s,v.y*s,v.z*s,v.w*s);
}


float4 operator+( const float4 &a, const float4 &b) 
{
	return float4(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w);
}



float4 operator-( const float4 &a, const float4 &b) 
{
	return float4(a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w);
}


float4 Homogenize(const float3 &v3,const float &w)
{
	return float4(v3.x,v3.y,v3.z,w);
}



float4x4 operator*( const float4x4& a, const float4x4& b )
{
	return float4x4(a.x*b,a.y*b,a.z*b,a.w*b);
}

float4x4 MatrixTranspose(const float4x4 &m)
{
	return float4x4(
		m.x.x, m.y.x, m.z.x, m.w.x,
		m.x.y, m.y.y, m.z.y, m.w.y,
		m.x.z, m.y.z, m.z.z, m.w.z,
		m.x.w, m.y.w, m.z.w, m.w.w );
}

float4x4 MatrixRigidInverse(const float4x4 &m)
{
	float4x4 trans_inverse = MatrixTranslation(-m.w.xyz());
	float4x4 rot   = m;
	rot.w = float4(0,0,0,1);
	return trans_inverse * MatrixTranspose(rot);
}


float4x4 MatrixPerspectiveFov(float fovy, float aspect, float zn, float zf )
{
	float h = 1.0f/tanf(fovy/2.0f); // view space height
	float w = h / aspect ;  // view space width
	return float4x4(
		w, 0, 0             ,   0,
		0, h, 0             ,   0,
		0, 0, zf/(zn-zf)    ,  -1,
		0, 0, zn*zf/(zn-zf) ,   0 );
}



float4x4 MatrixLookAt(const float3& eye, const float3& at, const float3& up)
{
	float4x4 m;
	m.w.w = 1.0f;
	m.w.xyz() = eye;
	m.z.xyz() = normalize(eye-at);
	m.x.xyz() = normalize(cross(up,m.z.xyz()));
	m.y.xyz() = cross(m.z.xyz(),m.x.xyz());
	return MatrixRigidInverse(m);
}


float4x4 MatrixTranslation(const float3 &t)
{
	return float4x4(
		1,  0,  0,  0,
		0,  1,  0,  0,
		0,  0,  1,  0,
		t.x,t.y,t.z,1 );
}


float4x4 MatrixRotationZ(const float angle_radians)
{
	float s =  sinf(angle_radians);
	float c =  cosf(angle_radians);
	return float4x4(
		c,  s,  0,  0,
		-s, c,  0,  0,
		0,  0,  1,  0,
		0,  0,  0,  1 );
}



int operator==( const float4x4 &a, const float4x4 &b )
{
	return (a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w);
}


float4x4 Inverse(const float4x4 &m)
{
	float4x4 d;
	float *dst = &d.x.x;
	float tmp[12]; /* temp array for pairs */
	float src[16]; /* array of transpose source matrix */
	float det; /* determinant */
	/* transpose matrix */
	for ( int i = 0; i < 4; i++) {
		src[i] = m(i,0) ;
		src[i + 4] = m(i,1);
		src[i + 8] = m(i,2);
		src[i + 12] = m(i,3); 
	}
	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0]  = src[10] * src[15];
	tmp[1]  = src[11] * src[14];
	tmp[2]  = src[9] * src[15];
	tmp[3]  = src[11] * src[13];
	tmp[4]  = src[9] * src[14];
	tmp[5]  = src[10] * src[13];
	tmp[6]  = src[8] * src[15];
	tmp[7]  = src[11] * src[12];
	tmp[8]  = src[8] * src[14];
	tmp[9]  = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];
	/* calculate first 8 elements (cofactors) */
	dst[0]  = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst[1]  = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst[2]  = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst[3]  = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst[4]  = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst[5]  = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst[6]  = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst[7]  = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0]  = src[2]*src[7];
	tmp[1]  = src[3]*src[6];
	tmp[2]  = src[1]*src[7];
	tmp[3]  = src[3]*src[5];
	tmp[4]  = src[1]*src[6];
	tmp[5]  = src[2]*src[5];
	tmp[6]  = src[0]*src[7];
	tmp[7]  = src[3]*src[4];
	tmp[8]  = src[0]*src[6];
	tmp[9]  = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];
	/* calculate second 8 elements (cofactors) */
	dst[8]  = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst[9]  = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
	/* calculate determinant */
	det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];
	/* calculate matrix inverse */
	det = 1/det;
	for ( int j = 0; j < 16; j++)
	dst[j] *= det;
	return d;
}


//--------- Quaternion --------------

Quaternion operator*( const Quaternion& a, const Quaternion& b )
{
	Quaternion c;
	c.w = a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z;
	c.x = a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y; 
	c.y = a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x; 
	c.z = a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w; 
	return c;
}


Quaternion operator*( const Quaternion& a, float b )
{
	return Quaternion(a.x*b, a.y*b, a.z*b ,a.w*b);
}

Quaternion  Inverse(const Quaternion &q)
{
	return Quaternion(-q.x,-q.y,-q.z,q.w);
}

Quaternion& operator*=( Quaternion& q, const float s )
{
		q.x *= s;
		q.y *= s;
		q.z *= s;
		q.w *= s;
		return q;
}
void Quaternion::Normalize()
{
	float m = sqrtf(sqr(w)+sqr(x)+sqr(y)+sqr(z));
	if(m<0.000000001f) {
		w=1.0f;
		x=y=z=0.0f;
		return;
	}
	(*this) *= (1.0f/m);
}

float3 operator*( const Quaternion& q, const float3& v )
{
	// The following is equivalent to:
	//return (q.getmatrix() * v);
	float qx2 = q.x*q.x;
	float qy2 = q.y*q.y;
	float qz2 = q.z*q.z;

	float qxqy = q.x*q.y;
	float qxqz = q.x*q.z;
	float qxqw = q.x*q.w;
	float qyqz = q.y*q.z;
	float qyqw = q.y*q.w;
	float qzqw = q.z*q.w;
	return float3(
		(1-2*(qy2+qz2))*v.x + (2*(qxqy-qzqw))*v.y + (2*(qxqz+qyqw))*v.z ,
		(2*(qxqy+qzqw))*v.x + (1-2*(qx2+qz2))*v.y + (2*(qyqz-qxqw))*v.z ,
		(2*(qxqz-qyqw))*v.x + (2*(qyqz+qxqw))*v.y + (1-2*(qx2+qy2))*v.z  );
}

float3 operator*( const float3& v, const Quaternion& q )
{
	assert(0);  // must multiply with the quat on the left
	return float3(0.0f,0.0f,0.0f);
}

Quaternion operator+( const Quaternion& a, const Quaternion& b )
{
	return Quaternion(a.x+b.x, a.y+b.y, a.z+b.z, a.w+b.w);
}

float dot( const Quaternion &a,const Quaternion &b )
{
	return  (a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z);
}

Quaternion normalize( Quaternion a )
{
	float m = sqrtf(sqr(a.w)+sqr(a.x)+sqr(a.y)+sqr(a.z));
	if(m<0.000000001) 
		{    
		a.w=1;
		a.x=a.y=a.z=0;
		return a;
	}
	return a * (1/m);
}

Quaternion slerp( Quaternion a, const Quaternion& b, float interp )
{
	if(dot(a,b) <0.0) 
		{
		a.w=-a.w;
		a.x=-a.x;
		a.y=-a.y;
		a.z=-a.z;
	}
	float d = dot(a,b);
	if(d>=1.0) {
		return a;
	}
	float theta = acosf(d);
	if(theta==0.0f) { return(a);}
	return a*(sinf(theta-interp*theta)/sinf(theta)) + b*(sinf(interp*theta)/sinf(theta));
}


Quaternion Interpolate(const Quaternion &q0,const Quaternion &q1,float alpha) {
	return slerp(q0,q1,alpha);
}


Quaternion YawPitchRoll( float yaw, float pitch, float roll ) 
{
	roll  *= DEG2RAD;
	yaw   *= DEG2RAD;
	pitch *= DEG2RAD;
	return Quaternion(float3(0.0f,0.0f,1.0f),yaw)*Quaternion(float3(1.0f,0.0f,0.0f),pitch)*Quaternion(float3(0.0f,1.0f,0.0f),roll);
}

float Yaw( const Quaternion& q )
{
	float3 v;
	v=q.ydir();
	return (v.y==0.0&&v.x==0.0) ? 0.0f: atan2f(-v.x,v.y)*RAD2DEG;
}

float Pitch( const Quaternion& q )
{
	float3 v;
	v=q.ydir();
	return atan2f(v.z,sqrtf(sqr(v.x)+sqr(v.y)))*RAD2DEG;
}

float Roll( Quaternion q )
{
	q = Quaternion(float3(0.0f,0.0f,1.0f),-Yaw(q)*DEG2RAD)  *q;
	q = Quaternion(float3(1.0f,0.0f,0.0f),-Pitch(q)*DEG2RAD)  *q;
	return atan2f(-q.xdir().z,q.xdir().x)*RAD2DEG;
}

float Yaw( const float3& v )
{
	return (v.y==0.0&&v.x==0.0) ? 0.0f: atan2f(-v.x,v.y)*RAD2DEG;
}

float Pitch( const float3& v )
{
	return atan2f(v.z,sqrtf(sqr(v.x)+sqr(v.y)))*RAD2DEG;
}


//------------- Plane --------------


void Plane::Transform(const float3 &position, const Quaternion &orientation) {
	//   Transforms the plane to the space defined by the 
	//   given position/orientation.
	float3 newnormal;
	float3 origin;

	newnormal = Inverse(orientation)*normal;
	origin = Inverse(orientation)*(-normal*dist - position);

	normal = newnormal;
	dist = -dot(newnormal, origin);
}




//--------- utility functions -------------

//        RotationArc()
// Given two vectors v0 and v1 this function
// returns quaternion q where q*v0==v1.
// Routine taken from game programming gems.
Quaternion RotationArc(float3 v0,float3 v1){
	Quaternion q;
	v0 = normalize(v0);  // Comment these two lines out if you know its not needed.
	v1 = normalize(v1);  // If vector is already unit length then why do it again?
	float3  c = cross(v0,v1);
	float   d = dot(v0,v1);
	if(d<=-1.0f) { return Quaternion(1,0,0,0);} // 180 about x axis
	float   s = sqrtf((1+d)*2);
	q.x = c.x / s;
	q.y = c.y / s;
	q.z = c.z / s;
	q.w = s /2.0f;
	return q;
}


float4x4 MatrixFromQuatVec(const Quaternion &q, const float3 &v) 
{
	// builds a 4x4 transformation matrix based on orientation q and translation v 
	float qx2 = q.x*q.x;
	float qy2 = q.y*q.y;
	float qz2 = q.z*q.z;

	float qxqy = q.x*q.y;
	float qxqz = q.x*q.z;
	float qxqw = q.x*q.w;
	float qyqz = q.y*q.z;
	float qyqw = q.y*q.w;
	float qzqw = q.z*q.w;

	return float4x4(
		1-2*(qy2+qz2),  
		2*(qxqy+qzqw),
		2*(qxqz-qyqw),  
		0            ,  
		2*(qxqy-qzqw),  
		1-2*(qx2+qz2),
		2*(qyqz+qxqw),  
		0            ,  
		2*(qxqz+qyqw),  
		2*(qyqz-qxqw),  
		1-2*(qx2+qy2),  
		0    , 
		 v.x ,
		 v.y ,
		 v.z ,
		 1.0f );
}


float3 PlaneLineIntersection(const Plane &plane, const float3 &p0, const float3 &p1)
{
	// returns the point where the line p0-p1 intersects the plane n&d
		float3 dif;
		dif = p1-p0;
				float dn= dot(plane.normal,dif);
				float t = -(plane.dist+dot(plane.normal,p0) )/dn;
				return p0 + (dif*t);
}

float3 PlaneProject(const Plane &plane, const float3 &point)
{
	return point - plane.normal * (dot(point,plane.normal)+plane.dist);
}

float3 LineProject(const float3 &p0, const float3 &p1, const float3 &a)
{
	float3 w;
	w = p1-p0;
	float t= dot(w,(a-p0)) / (sqr(w.x)+sqr(w.y)+sqr(w.z));
	return p0+ w*t;
}


float LineProjectTime(const float3 &p0, const float3 &p1, const float3 &a)
{
	float3 w;
	w = p1-p0;
	float t= dot(w,(a-p0)) / (sqr(w.x)+sqr(w.y)+sqr(w.z));
	return t;
}



float3 TriNormal(const float3 &v0, const float3 &v1, const float3 &v2)
{
	// return the normal of the triangle
	// inscribed by v0, v1, and v2
	float3 cp=cross(v1-v0,v2-v1);
	float m=magnitude(cp);
	if(m==0) return float3(1,0,0);
	return cp*(1.0f/m);
}



int BoxInside(const float3 &p, const float3 &bmin, const float3 &bmax) 
{
	return (p.x >= bmin.x && p.x <=bmax.x && 
			p.y >= bmin.y && p.y <=bmax.y && 
			p.z >= bmin.z && p.z <=bmax.z );
}


int BoxIntersect(const float3 &v0, const float3 &v1, const float3 &bmin, const float3 &bmax,float3 *impact)
{
	if(BoxInside(v0,bmin,bmax))
		{
				*impact=v0;
				return 1;
		}
	if(v0.x<=bmin.x && v1.x>=bmin.x) 
		{
		float a = (bmin.x-v0.x)/(v1.x-v0.x);
		//v.x = bmin.x;
		float vy =  (1-a) *v0.y + a*v1.y;
		float vz =  (1-a) *v0.z + a*v1.z;
		if(vy>=bmin.y && vy<=bmax.y && vz>=bmin.z && vz<=bmax.z) 
				{
			impact->x = bmin.x;
			impact->y = vy;
			impact->z = vz;
			return 1;
		}
	}
	else if(v0.x >= bmax.x  &&  v1.x <= bmax.x) 
		{
		float a = (bmax.x-v0.x)/(v1.x-v0.x);
		//v.x = bmax.x;
		float vy =  (1-a) *v0.y + a*v1.y;
		float vz =  (1-a) *v0.z + a*v1.z;
		if(vy>=bmin.y && vy<=bmax.y && vz>=bmin.z && vz<=bmax.z) 
				{
			impact->x = bmax.x;
			impact->y = vy;
			impact->z = vz;
			return 1;
		}
	}
	if(v0.y<=bmin.y && v1.y>=bmin.y) 
		{
		float a = (bmin.y-v0.y)/(v1.y-v0.y);
		float vx =  (1-a) *v0.x + a*v1.x;
		//v.y = bmin.y;
		float vz =  (1-a) *v0.z + a*v1.z;
		if(vx>=bmin.x && vx<=bmax.x && vz>=bmin.z && vz<=bmax.z) 
				{
			impact->x = vx;
			impact->y = bmin.y;
			impact->z = vz;
			return 1;
		}
	}
	else if(v0.y >= bmax.y  &&  v1.y <= bmax.y) 
		{
		float a = (bmax.y-v0.y)/(v1.y-v0.y);
		float vx =  (1-a) *v0.x + a*v1.x;
		// vy = bmax.y;
		float vz =  (1-a) *v0.z + a*v1.z;
		if(vx>=bmin.x && vx<=bmax.x && vz>=bmin.z && vz<=bmax.z)
				{
			impact->x = vx;
			impact->y = bmax.y;
			impact->z = vz;
			return 1;
		}
	}
	if(v0.z<=bmin.z && v1.z>=bmin.z) 
		{
		float a = (bmin.z-v0.z)/(v1.z-v0.z);
		float vx =  (1-a) *v0.x + a*v1.x;
		float vy =  (1-a) *v0.y + a*v1.y;
		// v.z = bmin.z;
		if(vy>=bmin.y && vy<=bmax.y && vx>=bmin.x && vx<=bmax.x) 
				{
			impact->x = vx;
			impact->y = vy;
			impact->z = bmin.z;
			return 1;
		}
	}
	else if(v0.z >= bmax.z  &&  v1.z <= bmax.z) 
		{
		float a = (bmax.z-v0.z)/(v1.z-v0.z);
		float vx =  (1-a) *v0.x + a*v1.x;
		float vy =  (1-a) *v0.y + a*v1.y;
		// v.z = bmax.z;
		if(vy>=bmin.y && vy<=bmax.y && vx>=bmin.x && vx<=bmax.x) 
				{
			impact->x = vx;
			impact->y = vy;
			impact->z = bmax.z;
			return 1;
		}
	}
	return 0;
}


float DistanceBetweenLines(const float3 &ustart, const float3 &udir, const float3 &vstart, const float3 &vdir, float3 *upoint, float3 *vpoint)
{
	float3 cp;
	cp = normalize(cross(udir,vdir));

	float distu = -dot(cp,ustart);
	float distv = -dot(cp,vstart);
	float dist = (float)fabs(distu-distv);
	if(upoint) 
		{
		Plane plane;
		plane.normal = normalize(cross(vdir,cp));
		plane.dist = -dot(plane.normal,vstart);
		*upoint = PlaneLineIntersection(plane,ustart,ustart+udir);
	}
	if(vpoint) 
		{
		Plane plane;
		plane.normal = normalize(cross(udir,cp));
		plane.dist = -dot(plane.normal,ustart);
		*vpoint = PlaneLineIntersection(plane,vstart,vstart+vdir);
	}
	return dist;
}


Quaternion VirtualTrackBall(const float3 &cop, const float3 &cor, const float3 &dir1, const float3 &dir2) 
{
	// routine taken from game programming gems.
	// Implement track ball functionality to spin stuf on the screen
	//  cop   center of projection
	//  cor   center of rotation
	//  dir1  old mouse direction 
	//  dir2  new mouse direction
	// pretend there is a sphere around cor.  Then find the points
	// where dir1 and dir2 intersect that sphere.  Find the
	// rotation that takes the first point to the second.
	float m;
	// compute plane 
	float3 nrml = cor - cop;
	float fudgefactor = 1.0f/(magnitude(nrml) * 0.25f); // since trackball proportional to distance from cop
	nrml = normalize(nrml);
	float dist = -dot(nrml,cor);
	float3 u= PlaneLineIntersection(Plane(nrml,dist),cop,cop+dir1);
	u=u-cor;
	u=u*fudgefactor;
	m= magnitude(u);
	if(m>1)
		{
				u/=m;
		}
	else 
		{
		u=u - (nrml * sqrtf(1-m*m));
	}
	float3 v= PlaneLineIntersection(Plane(nrml,dist),cop,cop+dir2);
	v=v-cor;
	v=v*fudgefactor;
	m= magnitude(v);
	if(m>1) 
		{
				v/=m;
		}
	else 
		{
		v=v - (nrml * sqrtf(1-m*m));
	}
	return RotationArc(u,v);
}


int countpolyhit=0;
int PolyHit(const float3 *vert, const int n, const float3 &v0, const float3 &v1, float3 *impact, float3 *normal)
{
	countpolyhit++;
	int i;
	float3 nrml(0,0,0);
	for(i=0;i<n;i++) 
		{
		int i1=(i+1)%n;
		int i2=(i+2)%n;
		nrml = nrml + cross(vert[i1]-vert[i],vert[i2]-vert[i1]);
	}

	float m = magnitude(nrml);
	if(m==0.0)
		{
				return 0;
		}
	nrml = nrml * (1.0f/m);
	float dist = -dot(nrml,vert[0]);
	float d0,d1;
	if((d0=dot(v0,nrml)+dist) <0  ||  (d1=dot(v1,nrml)+dist) >0) 
		{        
				return 0;
		}

	float3 the_point; 
	// By using the cached plane distances d0 and d1
	// we can optimize the following:
	//     the_point = planelineintersection(nrml,dist,v0,v1);
	float a = d0/(d0-d1);
	the_point = v0*(1-a) + v1*a;


	int inside=1;
	for(int j=0;inside && j<n;j++) 
		{
			// let inside = 0 if outside
			float3 pp1,pp2,side;
			pp1 = vert[j] ;
			pp2 = vert[(j+1)%n];
			side = cross((pp2-pp1),(the_point-pp1));
			inside = (dot(nrml,side) >= 0.0);
	}
	if(inside) 
		{
		if(normal){*normal=nrml;}
		if(impact){*impact=the_point;}
	}
	return inside;
}

//**************************************************************************
//**************************************************************************
//*** Stan Melax's array template, needed to compile his hull generation code
//**************************************************************************
//**************************************************************************

template <class Type> class ArrayRet;
template <class Type> class Array {
	public:
				Array(int s=0);
				Array(Array<Type> &array);
				Array(ArrayRet<Type> &array);
				~Array();
	void		allocate(int s);
	void		SetSize(int s);
	void		Pack();
	Type&		Add(Type);
	void		AddUnique(Type);
	int 		Contains(Type);
	void		Insert(Type,int);
	int			IndexOf(Type);
	void		Remove(Type);
	void		DelIndex(int i);
	Type *		element;
	int			count;
	int			array_size;
	const Type	&operator[](int i) const { assert(i>=0 && i<count);  return element[i]; }
	Type		&operator[](int i)  { assert(i>=0 && i<count);  return element[i]; }
	Type		&Pop() { assert(count); count--;  return element[count]; }
	Array<Type> &operator=(Array<Type> &array);
	Array<Type> &operator=(ArrayRet<Type> &array);
	// operator ArrayRet<Type> &() { return *(ArrayRet<Type> *)this;} // this worked but i suspect could be dangerous
};

template <class Type> class ArrayRet:public Array<Type>
{
};

template <class Type> Array<Type>::Array(int s)
{
	count=0;
	array_size = 0;
	element = NULL;
	if(s) 
	{
		allocate(s);
	}
}


template <class Type> Array<Type>::Array(Array<Type> &array)
{
	count=0;
	array_size = 0;
	element = NULL;
	for(int i=0;i<array.count;i++)
	{
		Add(array[i]);
	}
}


template <class Type> Array<Type>::Array(ArrayRet<Type> &array)
{
	*this = array;
}
template <class Type> Array<Type> &Array<Type>::operator=(ArrayRet<Type> &array)
{
	count=array.count;
	array_size = array.array_size;
	element = array.element;
	array.element=NULL;
	array.count=0;
	array.array_size=0;
	return *this;
}


template <class Type> Array<Type> &Array<Type>::operator=(Array<Type> &array)
{
	count=0;
	for(int i=0;i<array.count;i++)
	{
		Add(array[i]);
	}
	return *this;
}

template <class Type> Array<Type>::~Array()
{
	if (element != NULL)
	{
	  free(element);
	}
	count=0;array_size=0;element=NULL;
}

template <class Type> void Array<Type>::allocate(int s)
{
	assert(s>0);
	assert(s>=count);
	Type *old = element;
	array_size =s;
	element = (Type *) malloc( sizeof(Type)*array_size);
	assert(element);
	for(int i=0;i<count;i++)
	{
		element[i]=old[i];
	}
	if(old)
	{
		free(old);
	}
}

template <class Type> void Array<Type>::SetSize(int s)
{
	if(s==0)
	{
		if(element)
		{
			free(element);
			element = NULL;
		}
 	  array_size = s;
	}
	else
	{
		allocate(s);
	}
	count=s;
}

template <class Type> void Array<Type>::Pack()
{
	allocate(count);
}

template <class Type> Type& Array<Type>::Add(Type t)
{
	assert(count<=array_size);
	if(count==array_size)
	{
		allocate((array_size)?array_size *2:16);
	}
	element[count++] = t;
	return element[count-1];
}

template <class Type> int Array<Type>::Contains(Type t)
{
	int i;
	int found=0;
	for(i=0;i<count;i++)
	{
		if(element[i] == t) found++;
	}
	return found;
}

template <class Type> void Array<Type>::AddUnique(Type t)
{
	if(!Contains(t)) Add(t);
}


template <class Type> void Array<Type>::DelIndex(int i)
{
	assert(i<count);
	count--;
	while(i<count)
	{
		element[i] = element[i+1];
		i++;
	}
}

template <class Type> void Array<Type>::Remove(Type t)
{
	int i;
	for(i=0;i<count;i++)
	{
		if(element[i] == t)
		{
			break;
		}
	}
	assert(i<count); // assert object t is in the array.
	DelIndex(i);
	for(i=0;i<count;i++)
	{
		assert(element[i] != t);
	}
}

template <class Type> void Array<Type>::Insert(Type t,int k)
{
	int i=count;
	Add(t); // to allocate space
	while(i>k)
	{
		element[i]=element[i-1];
		i--;
	}
	assert(i==k);
	element[k]=t;
}


template <class Type> int Array<Type>::IndexOf(Type t)
{
	int i;
	for(i=0;i<count;i++)
	{
		if(element[i] == t)
		{
			return i;
		}
	}
	assert(0);
	return -1;
}



//*********************************************************************
//*********************************************************************
//********  Hull header
//*********************************************************************
//*********************************************************************

class PHullResult
{
public:

	PHullResult(void)
	{
		mVcount = 0;
		mIndexCount = 0;
		mFaceCount = 0;
		mVertices = 0;
		mIndices  = 0;
	}

	unsigned int mVcount;
	unsigned int mIndexCount;
	unsigned int mFaceCount;
	float       *mVertices;
	unsigned int *mIndices;
};


#define REAL3 float3
#define REAL  float

#define COPLANAR   (0)
#define UNDER      (1)
#define OVER       (2)
#define SPLIT      (OVER|UNDER)
#define PAPERWIDTH (0.001f)

float planetestepsilon = PAPERWIDTH;


class ConvexH 
{
  public:
	class HalfEdge
	{
	  public:
		short ea;         // the other half of the edge (index into edges list)
		unsigned char v;  // the vertex at the start of this edge (index into vertices list)
		unsigned char p;  // the facet on which this edge lies (index into facets list)
		HalfEdge(){}
		HalfEdge(short _ea,unsigned char _v, unsigned char _p):ea(_ea),v(_v),p(_p){}
	};
	Array<REAL3> vertices;
	Array<HalfEdge> edges;
	Array<Plane>  facets;
	ConvexH(int vertices_size,int edges_size,int facets_size);
};

typedef ConvexH::HalfEdge HalfEdge;

ConvexH::ConvexH(int vertices_size,int edges_size,int facets_size)
	:vertices(vertices_size)
	,edges(edges_size)
	,facets(facets_size)
{
	vertices.count=vertices_size;
	edges.count   = edges_size;
	facets.count  = facets_size;
}

ConvexH *ConvexHDup(ConvexH *src) {
	ConvexH *dst = new ConvexH(src->vertices.count,src->edges.count,src->facets.count);
	memcpy(dst->vertices.element,src->vertices.element,sizeof(float3)*src->vertices.count);
	memcpy(dst->edges.element,src->edges.element,sizeof(HalfEdge)*src->edges.count);
	memcpy(dst->facets.element,src->facets.element,sizeof(Plane)*src->facets.count);
	return dst;
}


int PlaneTest(const Plane &p, const REAL3 &v) {
	REAL a  = dot(v,p.normal)+p.dist;
	int   flag = (a>planetestepsilon)?OVER:((a<-planetestepsilon)?UNDER:COPLANAR);
	return flag;
}

int SplitTest(ConvexH &convex,const Plane &plane) {
	int flag=0;
	for(int i=0;i<convex.vertices.count;i++) {
		flag |= PlaneTest(plane,convex.vertices[i]);
	}
	return flag;
}

class VertFlag
{
public:
	unsigned char planetest;
	unsigned char junk;
	unsigned char undermap;
	unsigned char overmap;
};
class EdgeFlag 
{
public:
	unsigned char planetest;
	unsigned char fixes;
	short undermap;
	short overmap;
};
class PlaneFlag
{
public:
	unsigned char undermap;
	unsigned char overmap;
};
class Coplanar{
public:
	unsigned short ea;
	unsigned char v0;
	unsigned char v1;
};

int AssertIntact(ConvexH &convex) {
	int i;
	int estart=0;
	for(i=0;i<convex.edges.count;i++) {
		if(convex.edges[estart].p!= convex.edges[i].p) {
			estart=i;
		}
		int inext = i+1;
		if(inext>= convex.edges.count || convex.edges[inext].p != convex.edges[i].p) {
			inext = estart;
		}
		assert(convex.edges[inext].p == convex.edges[i].p);
		int nb = convex.edges[i].ea;
		assert(nb!=255);
		if(nb==255 || nb==-1) return 0;
		assert(nb!=-1);
		assert(i== convex.edges[nb].ea);
	}
	for(i=0;i<convex.edges.count;i++) {
		assert(COPLANAR==PlaneTest(convex.facets[convex.edges[i].p],convex.vertices[convex.edges[i].v]));
		if(COPLANAR!=PlaneTest(convex.facets[convex.edges[i].p],convex.vertices[convex.edges[i].v])) return 0;
		if(convex.edges[estart].p!= convex.edges[i].p) {
			estart=i;
		}
		int i1 = i+1;
		if(i1>= convex.edges.count || convex.edges[i1].p != convex.edges[i].p) {
			i1 = estart;
		}
		int i2 = i1+1;
		if(i2>= convex.edges.count || convex.edges[i2].p != convex.edges[i].p) {
			i2 = estart;
		}
		if(i==i2) continue; // i sliced tangent to an edge and created 2 meaningless edges
		REAL3 localnormal = TriNormal(convex.vertices[convex.edges[i ].v],
			                           convex.vertices[convex.edges[i1].v],
			                           convex.vertices[convex.edges[i2].v]);
		assert(dot(localnormal,convex.facets[convex.edges[i].p].normal)>0);
		if(dot(localnormal,convex.facets[convex.edges[i].p].normal)<=0)return 0;
	}
	return 1;
}

// back to back quads
ConvexH *test_btbq() {
	ConvexH *convex = new ConvexH(4,8,2);
	convex->vertices[0] = REAL3(0,0,0);
	convex->vertices[1] = REAL3(1,0,0);
	convex->vertices[2] = REAL3(1,1,0);
	convex->vertices[3] = REAL3(0,1,0);
	convex->facets[0] = Plane(REAL3(0,0,1),0);
	convex->facets[1] = Plane(REAL3(0,0,-1),0);
	convex->edges[0]  = HalfEdge(7,0,0);
	convex->edges[1]  = HalfEdge(6,1,0);
	convex->edges[2]  = HalfEdge(5,2,0);
	convex->edges[3]  = HalfEdge(4,3,0);

	convex->edges[4]  = HalfEdge(3,0,1);
	convex->edges[5]  = HalfEdge(2,3,1);
	convex->edges[6]  = HalfEdge(1,2,1);
	convex->edges[7]  = HalfEdge(0,1,1);
	AssertIntact(*convex);
	return convex;
}
ConvexH *test_cube() {
	ConvexH *convex = new ConvexH(8,24,6);
	convex->vertices[0] = REAL3(0,0,0);
	convex->vertices[1] = REAL3(0,0,1);
	convex->vertices[2] = REAL3(0,1,0);
	convex->vertices[3] = REAL3(0,1,1);
	convex->vertices[4] = REAL3(1,0,0);
	convex->vertices[5] = REAL3(1,0,1);
	convex->vertices[6] = REAL3(1,1,0);
	convex->vertices[7] = REAL3(1,1,1);

	convex->facets[0] = Plane(REAL3(-1,0,0),0);
	convex->facets[1] = Plane(REAL3(1,0,0),-1);
	convex->facets[2] = Plane(REAL3(0,-1,0),0);
	convex->facets[3] = Plane(REAL3(0,1,0),-1);
	convex->facets[4] = Plane(REAL3(0,0,-1),0);
	convex->facets[5] = Plane(REAL3(0,0,1),-1);

	convex->edges[0 ] = HalfEdge(11,0,0);
	convex->edges[1 ] = HalfEdge(23,1,0);
	convex->edges[2 ] = HalfEdge(15,3,0);
	convex->edges[3 ] = HalfEdge(16,2,0);

	convex->edges[4 ] = HalfEdge(13,6,1);
	convex->edges[5 ] = HalfEdge(21,7,1);
	convex->edges[6 ] = HalfEdge( 9,5,1);
	convex->edges[7 ] = HalfEdge(18,4,1);

	convex->edges[8 ] = HalfEdge(19,0,2);
	convex->edges[9 ] = HalfEdge( 6,4,2);
	convex->edges[10] = HalfEdge(20,5,2);
	convex->edges[11] = HalfEdge( 0,1,2);

	convex->edges[12] = HalfEdge(22,3,3);
	convex->edges[13] = HalfEdge( 4,7,3);
	convex->edges[14] = HalfEdge(17,6,3);
	convex->edges[15] = HalfEdge( 2,2,3);

	convex->edges[16] = HalfEdge( 3,0,4);
	convex->edges[17] = HalfEdge(14,2,4);
	convex->edges[18] = HalfEdge( 7,6,4);
	convex->edges[19] = HalfEdge( 8,4,4);
	
	convex->edges[20] = HalfEdge(10,1,5);
	convex->edges[21] = HalfEdge( 5,5,5);
	convex->edges[22] = HalfEdge(12,7,5);
	convex->edges[23] = HalfEdge( 1,3,5);

	
	return convex;
}
ConvexH *ConvexHMakeCube(const REAL3 &bmin, const REAL3 &bmax) {
	ConvexH *convex = test_cube();
	convex->vertices[0] = REAL3(bmin.x,bmin.y,bmin.z);
	convex->vertices[1] = REAL3(bmin.x,bmin.y,bmax.z);
	convex->vertices[2] = REAL3(bmin.x,bmax.y,bmin.z);
	convex->vertices[3] = REAL3(bmin.x,bmax.y,bmax.z);
	convex->vertices[4] = REAL3(bmax.x,bmin.y,bmin.z);
	convex->vertices[5] = REAL3(bmax.x,bmin.y,bmax.z);
	convex->vertices[6] = REAL3(bmax.x,bmax.y,bmin.z);
	convex->vertices[7] = REAL3(bmax.x,bmax.y,bmax.z);

	convex->facets[0] = Plane(REAL3(-1,0,0), bmin.x);
	convex->facets[1] = Plane(REAL3(1,0,0), -bmax.x);
	convex->facets[2] = Plane(REAL3(0,-1,0), bmin.y);
	convex->facets[3] = Plane(REAL3(0,1,0), -bmax.y);
	convex->facets[4] = Plane(REAL3(0,0,-1), bmin.z);
	convex->facets[5] = Plane(REAL3(0,0,1), -bmax.z);
	return convex;
}
ConvexH *ConvexHCrop(ConvexH &convex,const Plane &slice)
{
	int i;
	int vertcountunder=0;
	int vertcountover =0;
	Array<int> vertscoplanar;  // existing vertex members of convex that are coplanar
	vertscoplanar.count=0;
	Array<int> edgesplit;  // existing edges that members of convex that cross the splitplane
	edgesplit.count=0;

	assert(convex.edges.count<480);

	EdgeFlag  edgeflag[512];
	VertFlag  vertflag[256];
	PlaneFlag planeflag[128];
	HalfEdge  tmpunderedges[512];
	Plane	  tmpunderplanes[128];
	Coplanar coplanaredges[512];
	int coplanaredges_num=0;

	Array<REAL3> createdverts;
	// do the side-of-plane tests
	for(i=0;i<convex.vertices.count;i++) {
		vertflag[i].planetest = PlaneTest(slice,convex.vertices[i]);
		if(vertflag[i].planetest == COPLANAR) {
			// ? vertscoplanar.Add(i);
			vertflag[i].undermap = vertcountunder++;
			vertflag[i].overmap  = vertcountover++;
		}
		else if(vertflag[i].planetest == UNDER)	{
			vertflag[i].undermap = vertcountunder++;
		}
		else {
			assert(vertflag[i].planetest == OVER);
			vertflag[i].overmap  = vertcountover++;
			vertflag[i].undermap = 255; // for debugging purposes
		}
	}
	int vertcountunderold = vertcountunder; // for debugging only

	int under_edge_count =0;
	int underplanescount=0;
	int e0=0;

	for(int currentplane=0; currentplane<convex.facets.count; currentplane++) {
		int estart =e0;
		int enextface = 0;
		int planeside = 0;
		int e1 = e0+1;
		int vout=-1;
		int vin =-1;
		int coplanaredge = -1;
		do{

			if(e1 >= convex.edges.count || convex.edges[e1].p!=currentplane) {
				enextface = e1;
				e1=estart;
			}
			HalfEdge &edge0 = convex.edges[e0];
			HalfEdge &edge1 = convex.edges[e1];
			HalfEdge &edgea = convex.edges[edge0.ea];


			planeside |= vertflag[edge0.v].planetest;
			//if((vertflag[edge0.v].planetest & vertflag[edge1.v].planetest)  == COPLANAR) {
			//	assert(ecop==-1);
			//	ecop=e;
			//}


			if(vertflag[edge0.v].planetest == OVER && vertflag[edge1.v].planetest == OVER){
				// both endpoints over plane
				edgeflag[e0].undermap  = -1;
			}
			else if((vertflag[edge0.v].planetest | vertflag[edge1.v].planetest)  == UNDER) {
				// at least one endpoint under, the other coplanar or under
				
				edgeflag[e0].undermap = under_edge_count;
				tmpunderedges[under_edge_count].v = vertflag[edge0.v].undermap;
				tmpunderedges[under_edge_count].p = underplanescount;
				if(edge0.ea < e0) {
					// connect the neighbors
					assert(edgeflag[edge0.ea].undermap !=-1);
					tmpunderedges[under_edge_count].ea = edgeflag[edge0.ea].undermap;
					tmpunderedges[edgeflag[edge0.ea].undermap].ea = under_edge_count;
				}
				under_edge_count++;
			}
			else if((vertflag[edge0.v].planetest | vertflag[edge1.v].planetest)  == COPLANAR) {
				// both endpoints coplanar 
				// must check a 3rd point to see if UNDER
				int e2 = e1+1;
				if(e2>=convex.edges.count || convex.edges[e2].p!=currentplane) {
					e2 = estart;
				}
				assert(convex.edges[e2].p==currentplane);
				HalfEdge &edge2 = convex.edges[e2];
				if(vertflag[edge2.v].planetest==UNDER) {
					
					edgeflag[e0].undermap = under_edge_count;
					tmpunderedges[under_edge_count].v = vertflag[edge0.v].undermap;
					tmpunderedges[under_edge_count].p = underplanescount;
					tmpunderedges[under_edge_count].ea = -1;
					// make sure this edge is added to the "coplanar" list
					coplanaredge = under_edge_count;
					vout = vertflag[edge0.v].undermap;
					vin  = vertflag[edge1.v].undermap;
					under_edge_count++;
				}
				else {
					edgeflag[e0].undermap = -1;
				}
			}
			else if(vertflag[edge0.v].planetest == UNDER && vertflag[edge1.v].planetest == OVER) {
				// first is under 2nd is over 
				
				edgeflag[e0].undermap = under_edge_count;
				tmpunderedges[under_edge_count].v = vertflag[edge0.v].undermap;
				tmpunderedges[under_edge_count].p = underplanescount;
				if(edge0.ea < e0) {
					assert(edgeflag[edge0.ea].undermap !=-1);
					// connect the neighbors
					tmpunderedges[under_edge_count].ea = edgeflag[edge0.ea].undermap;
					tmpunderedges[edgeflag[edge0.ea].undermap].ea = under_edge_count;
					vout = tmpunderedges[edgeflag[edge0.ea].undermap].v;
				}
				else {
					Plane &p0 = convex.facets[edge0.p];
					Plane &pa = convex.facets[edgea.p];
					createdverts.Add(ThreePlaneIntersection(p0,pa,slice));
					//createdverts.Add(PlaneProject(slice,PlaneLineIntersection(slice,convex.vertices[edge0.v],convex.vertices[edgea.v])));
					//createdverts.Add(PlaneLineIntersection(slice,convex.vertices[edge0.v],convex.vertices[edgea.v]));
					vout = vertcountunder++;
				}
				under_edge_count++;
				/// hmmm something to think about: i might be able to output this edge regarless of 
				// wheter or not we know v-in yet.  ok i;ll try this now:
				tmpunderedges[under_edge_count].v = vout;
				tmpunderedges[under_edge_count].p = underplanescount;
				tmpunderedges[under_edge_count].ea = -1;
				coplanaredge = under_edge_count;
				under_edge_count++;

				if(vin!=-1) {
					// we previously processed an edge  where we came under
					// now we know about vout as well

					// ADD THIS EDGE TO THE LIST OF EDGES THAT NEED NEIGHBOR ON PARTITION PLANE!!
				}

			}
			else if(vertflag[edge0.v].planetest == COPLANAR && vertflag[edge1.v].planetest == OVER) {
				// first is coplanar 2nd is over 
				
				edgeflag[e0].undermap = -1;
				vout = vertflag[edge0.v].undermap;
				// I hate this but i have to make sure part of this face is UNDER before ouputting this vert
				int k=estart;
				assert(edge0.p == currentplane);
				while(!(planeside&UNDER) && k<convex.edges.count && convex.edges[k].p==edge0.p) {
					planeside |= vertflag[convex.edges[k].v].planetest;
					k++;
				}
				if(planeside&UNDER){
					tmpunderedges[under_edge_count].v = vout;
					tmpunderedges[under_edge_count].p = underplanescount;
					tmpunderedges[under_edge_count].ea = -1;
					coplanaredge = under_edge_count; // hmmm should make a note of the edge # for later on
					under_edge_count++;
					
				}
			}
			else if(vertflag[edge0.v].planetest == OVER && vertflag[edge1.v].planetest == UNDER) {
				// first is over next is under 
				// new vertex!!!
				assert(vin==-1);
				if(e0<edge0.ea) {
					Plane &p0 = convex.facets[edge0.p];
					Plane &pa = convex.facets[edgea.p];
					createdverts.Add(ThreePlaneIntersection(p0,pa,slice));
					//createdverts.Add(PlaneLineIntersection(slice,convex.vertices[edge0.v],convex.vertices[edgea.v]));
					//createdverts.Add(PlaneProject(slice,PlaneLineIntersection(slice,convex.vertices[edge0.v],convex.vertices[edgea.v])));
					vin = vertcountunder++;
				}
				else {
					// find the new vertex that was created by edge[edge0.ea]
					int nea = edgeflag[edge0.ea].undermap;
					assert(tmpunderedges[nea].p==tmpunderedges[nea+1].p);
					vin = tmpunderedges[nea+1].v;
					assert(vin < vertcountunder);
					assert(vin >= vertcountunderold);   // for debugging only
				}
				if(vout!=-1) {
					// we previously processed an edge  where we went over
					// now we know vin too
					// ADD THIS EDGE TO THE LIST OF EDGES THAT NEED NEIGHBOR ON PARTITION PLANE!!
				}
				// output edge
				tmpunderedges[under_edge_count].v = vin;
				tmpunderedges[under_edge_count].p = underplanescount;
				edgeflag[e0].undermap = under_edge_count;
				if(e0>edge0.ea) {
					assert(edgeflag[edge0.ea].undermap !=-1);
					// connect the neighbors
					tmpunderedges[under_edge_count].ea = edgeflag[edge0.ea].undermap;
					tmpunderedges[edgeflag[edge0.ea].undermap].ea = under_edge_count;
				}
				assert(edgeflag[e0].undermap == under_edge_count);
				under_edge_count++;
			}
			else if(vertflag[edge0.v].planetest == OVER && vertflag[edge1.v].planetest == COPLANAR) {
				// first is over next is coplanar 
				
				edgeflag[e0].undermap = -1;
				vin = vertflag[edge1.v].undermap;
				assert(vin!=-1);
				if(vout!=-1) {
					// we previously processed an edge  where we came under
					// now we know both endpoints
					// ADD THIS EDGE TO THE LIST OF EDGES THAT NEED NEIGHBOR ON PARTITION PLANE!!
				}

			}
			else {
				assert(0);
			}
			

			e0=e1;
			e1++; // do the modulo at the beginning of the loop

		} while(e0!=estart) ;
		e0 = enextface;
		if(planeside&UNDER) {
			planeflag[currentplane].undermap = underplanescount;
			tmpunderplanes[underplanescount] = convex.facets[currentplane];
			underplanescount++;
		}
		else {
			planeflag[currentplane].undermap = 0;
		}
		if(vout>=0 && (planeside&UNDER)) {
			assert(vin>=0);
			assert(coplanaredge>=0);
			assert(coplanaredge!=511);
			coplanaredges[coplanaredges_num].ea = coplanaredge;
			coplanaredges[coplanaredges_num].v0 = vin;
			coplanaredges[coplanaredges_num].v1 = vout;
			coplanaredges_num++;
		}
	}

	// add the new plane to the mix:
	if(coplanaredges_num>0) {
		tmpunderplanes[underplanescount++]=slice;
	}
	for(i=0;i<coplanaredges_num-1;i++) {
		if(coplanaredges[i].v1 != coplanaredges[i+1].v0) {
			int j = 0;
			for(j=i+2;j<coplanaredges_num;j++) {
				if(coplanaredges[i].v1 == coplanaredges[j].v0) {
					Coplanar tmp = coplanaredges[i+1];
					coplanaredges[i+1] = coplanaredges[j];
					coplanaredges[j] = tmp;
					break;
				}
			}
			if(j>=coplanaredges_num)
			{
				assert(j<coplanaredges_num);
				return NULL;
			}
		}
	}
	ConvexH *punder = new ConvexH(vertcountunder,under_edge_count+coplanaredges_num,underplanescount);
	ConvexH &under = *punder;
	int k=0;
	for(i=0;i<convex.vertices.count;i++) {
		if(vertflag[i].planetest != OVER){
			under.vertices[k++] = convex.vertices[i];
		}
	}
	i=0;
	while(k<vertcountunder) {
		under.vertices[k++] = createdverts[i++];
	}
	assert(i==createdverts.count);

	for(i=0;i<coplanaredges_num;i++) {
		under.edges[under_edge_count+i].p  = underplanescount-1;
		under.edges[under_edge_count+i].ea = coplanaredges[i].ea;
		tmpunderedges[coplanaredges[i].ea].ea = under_edge_count+i;
		under.edges[under_edge_count+i].v  = coplanaredges[i].v0;
	}
	
	memcpy(under.edges.element,tmpunderedges,sizeof(HalfEdge)*under_edge_count);
	memcpy(under.facets.element,tmpunderplanes,sizeof(Plane)*underplanescount);
	return punder;
}



static int candidateplane(Plane *planes,int planes_count,ConvexH *convex,float epsilon)
{
	int p = 0 ;
	REAL md= 0 ;
	int i;
	for(i=0;i<planes_count;i++)
	{
		REAL d=0;
		for(int j=0;j<convex->vertices.count;j++)
		{
			d = Max(d,dot(convex->vertices[j],planes[i].normal)+planes[i].dist);
		}
		if(i==0 || d>md)
		{
			p=i;
			md=d;
		}
	}
	return (md>epsilon)?p:-1;
}

template<class T>
inline int maxdir(const T *p,int count,const T &dir)
{
	assert(count);
	int m=0;
	float currDotm = dot(p[0], dir);
	for(int i=1;i<count;i++)
	{
		const float currDoti =  dot(p[i], dir);
		if(currDoti > currDotm) 
		{
			currDotm = currDoti;
			m=i;
		}
	}
	return m;
}


template<class T>
int maxdirfiltered(const T *p,int count,const T &dir,Array<int> &allow)
{
	assert(count);
	int m=-1;
	float currDotm = dot(p[0], dir);
	for(int i=0;i<count;i++) 
	{
		if(allow[i])
		{
			if(m==-1 )
			{
				m=i;
			}
			else 
			{
				const float currDoti = dot(p[i], dir);
				if (currDoti>currDotm)
				{
					currDotm = currDoti;
					m=i;
				}
			}
		}
	}
	assert(m!=-1);
	return m;
} 

float3 orth(const float3 &v)
{
	float3 a=cross(v,float3(0,0,1));
	float3 b=cross(v,float3(0,1,0));
	return normalize((magnitude(a)>magnitude(b))?a:b);
}


template<class T>
int maxdirsterid(const T *p,int count,const T &dir,Array<int> &allow)
{
	int m=-1;
	while(m==-1)
	{
		m = maxdirfiltered(p,count,dir,allow);
		if(allow[m]==3) return m;
		T u = orth(dir);
		T v = cross(u,dir);
		int ma=-1;
		for(float x = 0.0f ; x<= 360.0f ; x+= 45.0f)
		{
			float s = sinf(DEG2RAD*(x));
			float c = cosf(DEG2RAD*(x));
			int mb = maxdirfiltered(p,count,dir+(u*s+v*c)*0.025f,allow);
			if(ma==m && mb==m)
			{
				allow[m]=3;
				return m;
			}
			if(ma!=-1 && ma!=mb)  // Yuck - this is really ugly
			{
				int mc = ma;
				for(float xx = x-40.0f ; xx <= x ; xx+= 5.0f)
				{
					float s = sinf(DEG2RAD*(xx));
					float c = cosf(DEG2RAD*(xx));
					int md = maxdirfiltered(p,count,dir+(u*s+v*c)*0.025f,allow);
					if(mc==m && md==m)
					{
						allow[m]=3;
						return m;
					}
					mc=md;
				}
			}
			ma=mb;
		}
		allow[m]=0;
		m=-1;
	}
	assert(0);
	return m;
} 




int operator ==(const int3 &a,const int3 &b) 
{
	for(int i=0;i<3;i++) 
	{
		if(a[i]!=b[i]) return 0;
	}
	return 1;
}

int3 roll3(int3 a) 
{
	int tmp=a[0];
	a[0]=a[1];
	a[1]=a[2];
	a[2]=tmp;
	return a;
}
int isa(const int3 &a,const int3 &b) 
{
	return ( a==b || roll3(a)==b || a==roll3(b) );
}
int b2b(const int3 &a,const int3 &b) 
{
	return isa(a,int3(b[2],b[1],b[0]));
}
int above(float3* vertices,const int3& t, const float3 &p, float epsilon) 
{
	float3 n=TriNormal(vertices[t[0]],vertices[t[1]],vertices[t[2]]);
	return (dot(n,p-vertices[t[0]]) > epsilon); // EPSILON???
}
int hasedge(const int3 &t, int a,int b)
{
	for(int i=0;i<3;i++)
	{
		int i1= (i+1)%3;
		if(t[i]==a && t[i1]==b) return 1;
	}
	return 0;
}
int hasvert(const int3 &t, int v)
{
	return (t[0]==v || t[1]==v || t[2]==v) ;
}
int shareedge(const int3 &a,const int3 &b)
{
	int i;
	for(i=0;i<3;i++)
	{
		int i1= (i+1)%3;
		if(hasedge(a,b[i1],b[i])) return 1;
	}
	return 0;
}

class btHullTriangle;

//Array<btHullTriangle*> tris;

class btHullTriangle : public int3
{
public:
	int3 n;
	int id;
	int vmax;
	float rise;
	Array<btHullTriangle*>* tris;
	btHullTriangle(int a,int b,int c, Array<btHullTriangle*>* pTris):int3(a,b,c),n(-1,-1,-1)
	{
		tris = pTris;
		id = tris->count;
		tris->Add(this);
		vmax=-1;
		rise = 0.0f;
	}
	~btHullTriangle()
	{
		assert((*tris)[id]==this);
		(*tris)[id]=NULL;
	}
	int &neib(int a,int b);
};


int &btHullTriangle::neib(int a,int b)
{
	static int er=-1;
	int i;
	for(i=0;i<3;i++) 
	{
		int i1=(i+1)%3;
		int i2=(i+2)%3;
		if((*this)[i]==a && (*this)[i1]==b) return n[i2];
		if((*this)[i]==b && (*this)[i1]==a) return n[i2];
	}
	assert(0);
	return er;
}
void b2bfix(btHullTriangle* s,btHullTriangle*t, Array<btHullTriangle*>& tris)
{
	int i;
	for(i=0;i<3;i++) 
	{
		int i1=(i+1)%3;
		int i2=(i+2)%3;
		int a = (*s)[i1];
		int b = (*s)[i2];
		assert(tris[s->neib(a,b)]->neib(b,a) == s->id);
		assert(tris[t->neib(a,b)]->neib(b,a) == t->id);
		tris[s->neib(a,b)]->neib(b,a) = t->neib(b,a);
		tris[t->neib(b,a)]->neib(a,b) = s->neib(a,b);
	}
}

void removeb2b(btHullTriangle* s,btHullTriangle*t, Array<btHullTriangle*>& tris)
{
	b2bfix(s,t, tris);
	delete s;
	delete t;
}

void checkit(btHullTriangle *t, Array<btHullTriangle*>& tris)
{
	int i;
	assert(tris[t->id]==t);
	for(i=0;i<3;i++)
	{
		int i1=(i+1)%3;
		int i2=(i+2)%3;
		int a = (*t)[i1];
		int b = (*t)[i2];
		assert(a!=b);
		assert( tris[t->n[i]]->neib(b,a) == t->id);
	}
}
void extrude(btHullTriangle *t0,int v, Array<btHullTriangle*>& tris)
{
	int3 t= *t0;
	int n = tris.count;
	btHullTriangle* ta = new btHullTriangle(v,t[1],t[2], &tris);
	ta->n = int3(t0->n[0],n+1,n+2);
	tris[t0->n[0]]->neib(t[1],t[2]) = n+0;
	btHullTriangle* tb = new btHullTriangle(v,t[2],t[0], &tris);
	tb->n = int3(t0->n[1],n+2,n+0);
	tris[t0->n[1]]->neib(t[2],t[0]) = n+1;
	btHullTriangle* tc = new btHullTriangle(v,t[0],t[1], &tris);
	tc->n = int3(t0->n[2],n+0,n+1);
	tris[t0->n[2]]->neib(t[0],t[1]) = n+2;
	checkit(ta, tris);
	checkit(tb, tris);
	checkit(tc, tris);
	if(hasvert(*tris[ta->n[0]],v)) removeb2b(ta,tris[ta->n[0]], tris);
	if(hasvert(*tris[tb->n[0]],v)) removeb2b(tb,tris[tb->n[0]], tris);
	if(hasvert(*tris[tc->n[0]],v)) removeb2b(tc,tris[tc->n[0]], tris);
	delete t0;

}

btHullTriangle *extrudable(float epsilon, Array<btHullTriangle*>& tris)
{
	int i;
	btHullTriangle *t=NULL;
	for(i=0;i<tris.count;i++)
	{
		if(!t || (tris[i] && t->rise<tris[i]->rise))
		{
			t = tris[i];
		}
	}
	return (t->rise >epsilon)?t:NULL ;
}

class int4
{
public:
	int x,y,z,w;
	int4(){};
	int4(int _x,int _y, int _z,int _w){x=_x;y=_y;z=_z;w=_w;}
	const int& operator[](int i) const {return (&x)[i];}
	int& operator[](int i) {return (&x)[i];}
};



int4 FindSimplex(float3 *verts,int verts_count,Array<int> &allow)
{
	float3 basis[3];
	basis[0] = float3( 0.01f, 0.02f, 1.0f );      
	int p0 = maxdirsterid(verts,verts_count, basis[0],allow);   
	int	p1 = maxdirsterid(verts,verts_count,-basis[0],allow);
	basis[0] = verts[p0]-verts[p1];
	if(p0==p1 || basis[0]==float3(0,0,0)) 
		return int4(-1,-1,-1,-1);
	basis[1] = cross(float3(     1, 0.02f, 0),basis[0]);
	basis[2] = cross(float3(-0.02f,     1, 0),basis[0]);
	basis[1] = normalize( (magnitude(basis[1])>magnitude(basis[2])) ? basis[1]:basis[2]);
	int p2 = maxdirsterid(verts,verts_count,basis[1],allow);
	if(p2 == p0 || p2 == p1)
	{
		p2 = maxdirsterid(verts,verts_count,-basis[1],allow);
	}
	if(p2 == p0 || p2 == p1) 
		return int4(-1,-1,-1,-1);
	basis[1] = verts[p2] - verts[p0];
	basis[2] = normalize(cross(basis[1],basis[0]));
	int p3 = maxdirsterid(verts,verts_count,basis[2],allow);
	if(p3==p0||p3==p1||p3==p2) p3 = maxdirsterid(verts,verts_count,-basis[2],allow);
	if(p3==p0||p3==p1||p3==p2) 
		return int4(-1,-1,-1,-1);
	assert(!(p0==p1||p0==p2||p0==p3||p1==p2||p1==p3||p2==p3));
	if(dot(verts[p3]-verts[p0],cross(verts[p1]-verts[p0],verts[p2]-verts[p0])) <0) {Swap(p2,p3);}
	return int4(p0,p1,p2,p3);
}

int calchullgen(float3 *verts,int verts_count, int vlimit,Array<btHullTriangle*>& tris)
{
	if(verts_count <4) return 0;
	if(vlimit==0) vlimit=1000000000;
	int j;
	float3 bmin(*verts),bmax(*verts);
	Array<int> isextreme(verts_count);
	Array<int> allow(verts_count);
	for(j=0;j<verts_count;j++) 
	{
		allow.Add(1);
		isextreme.Add(0);
		bmin = VectorMin(bmin,verts[j]);
		bmax = VectorMax(bmax,verts[j]);
	}
	float epsilon = magnitude(bmax-bmin) * 0.001f;


	int4 p = FindSimplex(verts,verts_count,allow);
	if(p.x==-1) return 0; // simplex failed



	float3 center = (verts[p[0]]+verts[p[1]]+verts[p[2]]+verts[p[3]]) /4.0f;  // a valid interior point
	btHullTriangle *t0 = new btHullTriangle(p[2],p[3],p[1], &tris); t0->n=int3(2,3,1);
	btHullTriangle *t1 = new btHullTriangle(p[3],p[2],p[0], &tris); t1->n=int3(3,2,0);
	btHullTriangle *t2 = new btHullTriangle(p[0],p[1],p[3], &tris); t2->n=int3(0,1,3);
	btHullTriangle *t3 = new btHullTriangle(p[1],p[0],p[2], &tris); t3->n=int3(1,0,2);
	isextreme[p[0]]=isextreme[p[1]]=isextreme[p[2]]=isextreme[p[3]]=1;
	checkit(t0, tris);checkit(t1, tris);checkit(t2, tris);checkit(t3, tris);

	for(j=0;j<tris.count;j++)
	{
		btHullTriangle *t=tris[j];
		assert(t);
		assert(t->vmax<0);
		float3 n=TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
		t->vmax = maxdirsterid(verts,verts_count,n,allow);
		t->rise = dot(n,verts[t->vmax]-verts[(*t)[0]]);
	}
	btHullTriangle *te;
	vlimit-=4;
	while(vlimit >0 && (te=extrudable(epsilon, tris)))
	{
		int3 ti=*te;
		int v=te->vmax;
		assert(!isextreme[v]);  // wtf we've already done this vertex
		isextreme[v]=1;
		//if(v==p0 || v==p1 || v==p2 || v==p3) continue; // done these already
		j=tris.count;
		while(j--) {
			if(!tris[j]) continue;
			int3 t=*tris[j];
			if(above(verts,t,verts[v],0.01f*epsilon)) 
			{
				extrude(tris[j],v, tris);
			}
		}
		// now check for those degenerate cases where we have a flipped triangle or a really skinny triangle
		j=tris.count;
		while(j--)
		{
			if(!tris[j]) continue;
			if(!hasvert(*tris[j],v)) break;
			int3 nt=*tris[j];
			if(above(verts,nt,center,0.01f*epsilon)  || magnitude(cross(verts[nt[1]]-verts[nt[0]],verts[nt[2]]-verts[nt[1]]))< epsilon*epsilon*0.1f )
			{
				btHullTriangle *nb = tris[tris[j]->n[0]];
				assert(nb);assert(!hasvert(*nb,v));assert(nb->id<j);
				extrude(nb,v, tris);
				j=tris.count; 
			}
		} 
		j=tris.count;
		while(j--)
		{
			btHullTriangle *t=tris[j];
			if(!t) continue;
			if(t->vmax>=0) break;
			float3 n=TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
			t->vmax = maxdirsterid(verts,verts_count,n,allow);
			if(isextreme[t->vmax]) 
			{
				t->vmax=-1; // already done that vertex - algorithm needs to be able to terminate.
			}
			else
			{
				t->rise = dot(n,verts[t->vmax]-verts[(*t)[0]]);
			}
		}
		vlimit --;
	}
	return 1;
}

int calchull(float3 *verts,int verts_count, int *&tris_out, int &tris_count,int vlimit, Array<btHullTriangle*>& tris) 
{
	int rc=calchullgen(verts,verts_count,  vlimit, tris) ;
	if(!rc) return 0;
	Array<int> ts;
	for(int i=0;i<tris.count;i++)if(tris[i])
	{
		for(int j=0;j<3;j++)ts.Add((*tris[i])[j]);
		delete tris[i];
	}
	tris_count = ts.count/3;
	tris_out   = ts.element;
	ts.element=NULL; ts.count=ts.array_size=0;
	tris.count=0;
	return 1;
}

int calchullpbev(float3 *verts,int verts_count,int vlimit, Array<Plane> &planes,float bevangle, Array<btHullTriangle*>& tris) 
{
	int i,j;
	planes.count=0;
	int rc = calchullgen(verts,verts_count,vlimit, tris);
	if(!rc) return 0;
	for(i=0;i<tris.count;i++)if(tris[i])
	{
		Plane p;
		btHullTriangle *t = tris[i];
		p.normal = TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
		p.dist   = -dot(p.normal, verts[(*t)[0]]);
		planes.Add(p);
		for(j=0;j<3;j++)
		{
			if(t->n[j]<t->id) continue;
			btHullTriangle *s = tris[t->n[j]];
			REAL3 snormal = TriNormal(verts[(*s)[0]],verts[(*s)[1]],verts[(*s)[2]]);
			if(dot(snormal,p.normal)>=cos(bevangle*DEG2RAD)) continue;
			REAL3 n = normalize(snormal+p.normal);
			planes.Add(Plane(n,-dot(n,verts[maxdir(verts,verts_count,n)])));
		}
	}

	for(i=0;i<tris.count;i++)if(tris[i])
	{
		delete tris[i]; // delete tris[i];
	}
	tris.count=0;
	return 1;
}

int overhull(Plane *planes,int planes_count,float3 *verts, int verts_count,int maxplanes, 
			 float3 *&verts_out, int &verts_count_out,  int *&faces_out, int &faces_count_out ,float inflate)
{
	int i,j;
	if(verts_count <4) return 0;
	maxplanes = Min(maxplanes,planes_count);
	float3 bmin(verts[0]),bmax(verts[0]);
	for(i=0;i<verts_count;i++) 
	{
		bmin = VectorMin(bmin,verts[i]);
		bmax = VectorMax(bmax,verts[i]);
	}
//	float diameter = magnitude(bmax-bmin);
//	inflate *=diameter;   // RELATIVE INFLATION
	bmin -= float3(inflate,inflate,inflate);
	bmax += float3(inflate,inflate,inflate);
	for(i=0;i<planes_count;i++)
	{
		planes[i].dist -= inflate;
	}
	float3 emin = bmin; // VectorMin(bmin,float3(0,0,0));
	float3 emax = bmax; // VectorMax(bmax,float3(0,0,0));
	float epsilon  = magnitude(emax-emin) * 0.025f;
	planetestepsilon = magnitude(emax-emin) * PAPERWIDTH;
	// todo: add bounding cube planes to force bevel. or try instead not adding the diameter expansion ??? must think.
	// ConvexH *convex = ConvexHMakeCube(bmin - float3(diameter,diameter,diameter),bmax+float3(diameter,diameter,diameter));
	ConvexH *c = ConvexHMakeCube(REAL3(bmin),REAL3(bmax)); 
	int k;
	while(maxplanes-- && (k=candidateplane(planes,planes_count,c,epsilon))>=0)
	{
		ConvexH *tmp = c;
		c = ConvexHCrop(*tmp,planes[k]);
		if(c==NULL) {c=tmp; break;} // might want to debug this case better!!!
		if(!AssertIntact(*c)) {c=tmp; break;} // might want to debug this case better too!!!
		delete tmp;
	}

	assert(AssertIntact(*c));
	//return c;
	faces_out = (int*)malloc(sizeof(int)*(1+c->facets.count+c->edges.count));     // new int[1+c->facets.count+c->edges.count];
	faces_count_out=0;
	i=0;
	faces_out[faces_count_out++]=-1;
	k=0;
	while(i<c->edges.count)
	{
		j=1;
		while(j+i<c->edges.count && c->edges[i].p==c->edges[i+j].p) { j++; }
		faces_out[faces_count_out++]=j;
		while(j--)
		{
			faces_out[faces_count_out++] = c->edges[i].v;
			i++;
		}
		k++;
	}
	faces_out[0]=k; // number of faces.
	assert(k==c->facets.count);
	assert(faces_count_out == 1+c->facets.count+c->edges.count);
	verts_out = c->vertices.element; // new float3[c->vertices.count]; 
	verts_count_out = c->vertices.count;
	for(i=0;i<c->vertices.count;i++)
	{
		verts_out[i] = float3(c->vertices[i]);
	}
	c->vertices.count=c->vertices.array_size=0;	c->vertices.element=NULL;
	delete c;
	return 1;
}

int overhullv(float3 *verts, int verts_count,int maxplanes, 
			 float3 *&verts_out, int &verts_count_out,  int *&faces_out, int &faces_count_out ,float inflate,float bevangle,int vlimit, Array<btHullTriangle*>& tris)
{
	if(!verts_count) return 0;
	extern int calchullpbev(float3 *verts,int verts_count,int vlimit, Array<Plane> &planes,float bevangle, Array<btHullTriangle*>& tris) ;
	Array<Plane> planes;
	int rc=calchullpbev(verts,verts_count,vlimit,planes,bevangle, tris) ;
	if(!rc) return 0;
	return overhull(planes.element,planes.count,verts,verts_count,maxplanes,verts_out,verts_count_out,faces_out,faces_count_out,inflate);
}


bool ComputeHull(unsigned int vcount,const float *vertices,PHullResult &result,unsigned int vlimit,float inflate, Array<btHullTriangle*>& arrtris)
{

	int index_count;
	int *faces;
	float3 *verts_out;
	int     verts_count_out;

	if(inflate==0.0f)
	{
		int  *tris_out;
		int    tris_count;
		int ret = calchull( (float3 *) vertices, (int) vcount, tris_out, tris_count, vlimit, arrtris );
		if(!ret) return false;
		result.mIndexCount = (unsigned int) (tris_count*3);
		result.mFaceCount  = (unsigned int) tris_count;
		result.mVertices   = (float*) vertices;
		result.mVcount     = (unsigned int) vcount;
		result.mIndices    = (unsigned int *) tris_out;
		return true;
	}

	int ret = overhullv((float3*)vertices,vcount,35,verts_out,verts_count_out,faces,index_count,inflate,120.0f,vlimit, arrtris);
	if(!ret) return false;

	Array<int3> tris;
	int n=faces[0];
	int k=1;
	for(int i=0;i<n;i++)
	{
		int pn = faces[k++];
		for(int j=2;j<pn;j++) tris.Add(int3(faces[k],faces[k+j-1],faces[k+j]));
		k+=pn;
	}
	assert(tris.count == index_count-1-(n*3));

	result.mIndexCount = (unsigned int) (tris.count*3);
	result.mFaceCount  = (unsigned int) tris.count;
	result.mVertices   = (float*) verts_out;
	result.mVcount     = (unsigned int) verts_count_out;
	result.mIndices    = (unsigned int *) tris.element;
	tris.element=NULL; tris.count = tris.array_size=0;

	return true;
}


void ReleaseHull(PHullResult &result)
{
	if ( result.mIndices )
  {
	  free(result.mIndices);
	}

	result.mVcount = 0;
	result.mIndexCount = 0;
	result.mIndices = 0;
	result.mVertices = 0;
	result.mIndices  = 0;
}


//*********************************************************************
//*********************************************************************
//********  HullLib header
//*********************************************************************
//*********************************************************************

//*********************************************************************
//*********************************************************************
//********  HullLib implementation
//*********************************************************************
//*********************************************************************

HullError HullLibrary::CreateConvexHull(const HullDesc       &desc,           // describes the input request
																					HullResult           &result)         // contains the resulst
{
	HullError ret = QE_FAIL;


	PHullResult hr;

	unsigned int vcount = desc.mVcount;
	if ( vcount < 8 ) vcount = 8;

	float *vsource  = (float *) malloc( sizeof(float)*vcount*3);


	float scale[3];

	unsigned int ovcount;

	bool ok = CleanupVertices(desc.mVcount,desc.mVertices, desc.mVertexStride, ovcount, vsource, desc.mNormalEpsilon, scale ); // normalize point cloud, remove duplicates!

	if ( ok )
	{


		if ( 1 ) // scale vertices back to their original size.
		{
			for (unsigned int i=0; i<ovcount; i++)
			{
				float *v = &vsource[i*3];
				v[0]*=scale[0];
				v[1]*=scale[1];
				v[2]*=scale[2];
			}
		}

		float skinwidth = 0;
		if ( desc.HasHullFlag(QF_SKIN_WIDTH) )
			skinwidth = desc.mSkinWidth;

		Array<btHullTriangle*> tris;
		ok = ComputeHull(ovcount,vsource,hr,desc.mMaxVertices,skinwidth, tris);

		if ( ok )
		{

			// re-index triangle mesh so it refers to only used vertices, rebuild a new vertex table.
			float *vscratch = (float *) malloc( sizeof(float)*hr.mVcount*3);
			BringOutYourDead(hr.mVertices,hr.mVcount, vscratch, ovcount, hr.mIndices, hr.mIndexCount );

			ret = QE_OK;

			if ( desc.HasHullFlag(QF_TRIANGLES) ) // if he wants the results as triangle!
			{
				result.mPolygons          = false;
				result.mNumOutputVertices = ovcount;
				result.mOutputVertices    = (float *)malloc( sizeof(float)*ovcount*3);
				result.mNumFaces          = hr.mFaceCount;
				result.mNumIndices        = hr.mIndexCount;

				result.mIndices           = (unsigned int *) malloc( sizeof(unsigned int)*hr.mIndexCount);

				memcpy(result.mOutputVertices, vscratch, sizeof(float)*3*ovcount );

  			if ( desc.HasHullFlag(QF_REVERSE_ORDER) )
				{

					const unsigned int *source = hr.mIndices;
								unsigned int *dest   = result.mIndices;

					for (unsigned int i=0; i<hr.mFaceCount; i++)
					{
						dest[0] = source[2];
						dest[1] = source[1];
						dest[2] = source[0];
						dest+=3;
						source+=3;
					}

				}
				else
				{
					memcpy(result.mIndices, hr.mIndices, sizeof(unsigned int)*hr.mIndexCount);
				}
			}
			else
			{
				result.mPolygons          = true;
				result.mNumOutputVertices = ovcount;
				result.mOutputVertices    = (float *)malloc( sizeof(float)*ovcount*3);
				result.mNumFaces          = hr.mFaceCount;
				result.mNumIndices        = hr.mIndexCount+hr.mFaceCount;
				result.mIndices           = (unsigned int *) malloc( sizeof(unsigned int)*result.mNumIndices);
				memcpy(result.mOutputVertices, vscratch, sizeof(float)*3*ovcount );

				if ( 1 )
				{
					const unsigned int *source = hr.mIndices;
								unsigned int *dest   = result.mIndices;
					for (unsigned int i=0; i<hr.mFaceCount; i++)
					{
						dest[0] = 3;
						if ( desc.HasHullFlag(QF_REVERSE_ORDER) )
						{
							dest[1] = source[2];
							dest[2] = source[1];
							dest[3] = source[0];
						}
						else
						{
							dest[1] = source[0];
							dest[2] = source[1];
							dest[3] = source[2];
						}

						dest+=4;
						source+=3;
					}
				}
			}
			ReleaseHull(hr);
			if ( vscratch )
			{
				free(vscratch);
			}
		}
	}

	if ( vsource )
	{
		free(vsource);
	}


	return ret;
}



HullError HullLibrary::ReleaseResult(HullResult &result) // release memory allocated for this result, we are done with it.
{
	if ( result.mOutputVertices )
	{
		free(result.mOutputVertices);
		result.mOutputVertices = 0;
	}
	if ( result.mIndices )
	{
		free(result.mIndices);
		result.mIndices = 0;
	}
	return QE_OK;
}


static void addPoint(unsigned int &vcount,float *p,float x,float y,float z)
{
	float *dest = &p[vcount*3];
	dest[0] = x;
	dest[1] = y;
	dest[2] = z;
	vcount++;
}


float GetDist(float px,float py,float pz,const float *p2)
{

	float dx = px - p2[0];
	float dy = py - p2[1];
	float dz = pz - p2[2];

	return dx*dx+dy*dy+dz*dz;
}



bool  HullLibrary::CleanupVertices(unsigned int svcount,
																const float *svertices,
																unsigned int stride,
																unsigned int &vcount,       // output number of vertices
																float *vertices,                 // location to store the results.
																float  normalepsilon,
																float *scale)
{
	if ( svcount == 0 ) return false;


#define EPSILON 0.000001f /* close enough to consider two floating point numbers to be 'the same'. */

	vcount = 0;

	float recip[3];

	if ( scale )
	{
		scale[0] = 1;
		scale[1] = 1;
		scale[2] = 1;
	}

	float bmin[3] = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
	float bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

	const char *vtx = (const char *) svertices;

	if ( 1 )
	{
		for (unsigned int i=0; i<svcount; i++)
		{
			const float *p = (const float *) vtx;

			vtx+=stride;

			for (int j=0; j<3; j++)
			{
				if ( p[j] < bmin[j] ) bmin[j] = p[j];
				if ( p[j] > bmax[j] ) bmax[j] = p[j];
			}
		}
	}

	float dx = bmax[0] - bmin[0];
	float dy = bmax[1] - bmin[1];
	float dz = bmax[2] - bmin[2];

	float center[3];

	center[0] = dx*0.5f + bmin[0];
	center[1] = dy*0.5f + bmin[1];
	center[2] = dz*0.5f + bmin[2];

	if ( dx < EPSILON || dy < EPSILON || dz < EPSILON || svcount < 3 )
	{

		float len = FLT_MAX;

		if ( dx > EPSILON && dx < len ) len = dx;
		if ( dy > EPSILON && dy < len ) len = dy;
		if ( dz > EPSILON && dz < len ) len = dz;

		if ( len == FLT_MAX )
		{
			dx = dy = dz = 0.01f; // one centimeter
		}
		else
		{
			if ( dx < EPSILON ) dx = len * 0.05f; // 1/5th the shortest non-zero edge.
			if ( dy < EPSILON ) dy = len * 0.05f;
			if ( dz < EPSILON ) dz = len * 0.05f;
		}

		float x1 = center[0] - dx;
		float x2 = center[0] + dx;

		float y1 = center[1] - dy;
		float y2 = center[1] + dy;

		float z1 = center[2] - dz;
		float z2 = center[2] + dz;

		addPoint(vcount,vertices,x1,y1,z1);
		addPoint(vcount,vertices,x2,y1,z1);
		addPoint(vcount,vertices,x2,y2,z1);
		addPoint(vcount,vertices,x1,y2,z1);
		addPoint(vcount,vertices,x1,y1,z2);
		addPoint(vcount,vertices,x2,y1,z2);
		addPoint(vcount,vertices,x2,y2,z2);
		addPoint(vcount,vertices,x1,y2,z2);

		return true; // return cube


	}
	else
	{
		if ( scale )
		{
			scale[0] = dx;
			scale[1] = dy;
			scale[2] = dz;

			recip[0] = 1 / dx;
			recip[1] = 1 / dy;
			recip[2] = 1 / dz;

			center[0]*=recip[0];
			center[1]*=recip[1];
			center[2]*=recip[2];

		}

	}



	vtx = (const char *) svertices;

	for (unsigned int i=0; i<svcount; i++)
	{

		const float *p = (const float *)vtx;
		vtx+=stride;

		float px = p[0];
		float py = p[1];
		float pz = p[2];

		if ( scale )
		{
			px = px*recip[0]; // normalize
			py = py*recip[1]; // normalize
			pz = pz*recip[2]; // normalize
		}

		if ( 1 )
		{
			unsigned int j;

			for (j=0; j<vcount; j++)
			{
				float *v = &vertices[j*3];

				float x = v[0];
				float y = v[1];
				float z = v[2];

				float dx = fabsf(x - px );
				float dy = fabsf(y - py );
				float dz = fabsf(z - pz );

				if ( dx < normalepsilon && dy < normalepsilon && dz < normalepsilon )
				{
					// ok, it is close enough to the old one
					// now let us see if it is further from the center of the point cloud than the one we already recorded.
					// in which case we keep this one instead.

					float dist1 = GetDist(px,py,pz,center);
					float dist2 = GetDist(v[0],v[1],v[2],center);

					if ( dist1 > dist2 )
					{
						v[0] = px;
						v[1] = py;
						v[2] = pz;
					}

					break;
				}
			}

			if ( j == vcount )
			{
				float *dest = &vertices[vcount*3];
				dest[0] = px;
				dest[1] = py;
				dest[2] = pz;
				vcount++;
			}
		}
	}

	// ok..now make sure we didn't prune so many vertices it is now invalid.
	if ( 1 )
	{
		float bmin[3] = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
		float bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		for (unsigned int i=0; i<vcount; i++)
		{
			const float *p = &vertices[i*3];
			for (int j=0; j<3; j++)
			{
				if ( p[j] < bmin[j] ) bmin[j] = p[j];
				if ( p[j] > bmax[j] ) bmax[j] = p[j];
			}
		}

		float dx = bmax[0] - bmin[0];
		float dy = bmax[1] - bmin[1];
		float dz = bmax[2] - bmin[2];

		if ( dx < EPSILON || dy < EPSILON || dz < EPSILON || vcount < 3)
		{
			float cx = dx*0.5f + bmin[0];
			float cy = dy*0.5f + bmin[1];
			float cz = dz*0.5f + bmin[2];

			float len = FLT_MAX;

			if ( dx >= EPSILON && dx < len ) len = dx;
			if ( dy >= EPSILON && dy < len ) len = dy;
			if ( dz >= EPSILON && dz < len ) len = dz;

			if ( len == FLT_MAX )
			{
				dx = dy = dz = 0.01f; // one centimeter
			}
			else
			{
				if ( dx < EPSILON ) dx = len * 0.05f; // 1/5th the shortest non-zero edge.
				if ( dy < EPSILON ) dy = len * 0.05f;
				if ( dz < EPSILON ) dz = len * 0.05f;
			}

			float x1 = cx - dx;
			float x2 = cx + dx;

			float y1 = cy - dy;
			float y2 = cy + dy;

			float z1 = cz - dz;
			float z2 = cz + dz;

			vcount = 0; // add box

			addPoint(vcount,vertices,x1,y1,z1);
			addPoint(vcount,vertices,x2,y1,z1);
			addPoint(vcount,vertices,x2,y2,z1);
			addPoint(vcount,vertices,x1,y2,z1);
			addPoint(vcount,vertices,x1,y1,z2);
			addPoint(vcount,vertices,x2,y1,z2);
			addPoint(vcount,vertices,x2,y2,z2);
			addPoint(vcount,vertices,x1,y2,z2);

			return true;
		}
	}

	return true;
}

void HullLibrary::BringOutYourDead(const float *verts,unsigned int vcount, float *overts,unsigned int &ocount,unsigned int *indices,unsigned indexcount)
{
	unsigned int *used = (unsigned int *)malloc(sizeof(unsigned int)*vcount);
	memset(used,0,sizeof(unsigned int)*vcount);

	ocount = 0;

	for (unsigned int i=0; i<indexcount; i++)
	{
		unsigned int v = indices[i]; // original array index

		assert( v >= 0 && v < vcount );

		if ( used[v] ) // if already remapped
		{
			indices[i] = used[v]-1; // index to new array
		}
		else
		{

			indices[i] = ocount;      // new index mapping

			overts[ocount*3+0] = verts[v*3+0]; // copy old vert to new vert array
			overts[ocount*3+1] = verts[v*3+1];
			overts[ocount*3+2] = verts[v*3+2];

			ocount++; // increment output vert count

			assert( ocount >=0 && ocount <= vcount );

			used[v] = ocount; // assign new index remapping
		}
	}

	free(used);
}

}
