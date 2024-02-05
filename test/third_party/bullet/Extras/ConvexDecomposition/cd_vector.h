#ifndef CD_VECTOR_H

#define CD_VECTOR_H

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


#pragma warning(disable:4786)

#include <math.h>
#include <float.h>
#include <vector>

namespace ConvexDecomposition
{


const float DEG_TO_RAD = ((2.0f * 3.14152654f) / 360.0f);
const float RAD_TO_DEG = (360.0f / (2.0f * 3.141592654f));

class Vector3d
{
public:
	Vector3d(void) { };  // null constructor, does not inialize point.

	Vector3d(const Vector3d &a) // constructor copies existing vector.
	{
		x = a.x;
		y = a.y;
		z = a.z;
	};

	Vector3d(float a,float b,float c) // construct with initial point.
	{
		x = a;
		y = b;
		z = c;
	};

	Vector3d(const float *t)
	{
		x = t[0];
		y = t[1];
		z = t[2];
	};

	Vector3d(const int *t)
	{
		x = t[0];
		y = t[1];
		z = t[2];
	};

	bool operator==(const Vector3d &a) const
	{
		return( a.x == x && a.y == y && a.z == z );
	};

	bool operator!=(const Vector3d &a) const
	{
		return( a.x != x || a.y != y || a.z != z );
	};

// Operators
		Vector3d& operator = (const Vector3d& A)          // ASSIGNMENT (=)
			{ x=A.x; y=A.y; z=A.z;
				return(*this);  };

		Vector3d operator + (const Vector3d& A) const     // ADDITION (+)
			{ Vector3d Sum(x+A.x, y+A.y, z+A.z);
				return(Sum); };

		Vector3d operator - (const Vector3d& A) const     // SUBTRACTION (-)
			{ Vector3d Diff(x-A.x, y-A.y, z-A.z);
				return(Diff); };

		Vector3d operator * (const float s) const       // MULTIPLY BY SCALAR (*)
			{ Vector3d Scaled(x*s, y*s, z*s);
				return(Scaled); };


		Vector3d operator + (const float s) const       // ADD CONSTANT TO ALL 3 COMPONENTS (*)
			{ Vector3d Scaled(x+s, y+s, z+s);
				return(Scaled); };




		Vector3d operator / (const float s) const       // DIVIDE BY SCALAR (/)
		{
			float r = 1.0f / s;
				Vector3d Scaled(x*r, y*r, z*r);
				return(Scaled);
		};

		void operator /= (float A)             // ACCUMULATED VECTOR ADDITION (/=)
			{ x/=A; y/=A; z/=A; };

		void operator += (const Vector3d A)             // ACCUMULATED VECTOR ADDITION (+=)
			{ x+=A.x; y+=A.y; z+=A.z; };
		void operator -= (const Vector3d A)             // ACCUMULATED VECTOR SUBTRACTION (+=)
			{ x-=A.x; y-=A.y; z-=A.z; };
		void operator *= (const float s)        // ACCUMULATED SCALAR MULTIPLICATION (*=) (bpc 4/24/2000)
			{x*=s; y*=s; z*=s;}

		void operator += (const float A)             // ACCUMULATED VECTOR ADDITION (+=)
			{ x+=A; y+=A; z+=A; };


		Vector3d operator - (void) const                // NEGATION (-)
			{ Vector3d Negated(-x, -y, -z);
				return(Negated); };

		float operator [] (const int i) const         // ALLOWS VECTOR ACCESS AS AN ARRAY.
			{ return( (i==0)?x:((i==1)?y:z) ); };
		float & operator [] (const int i)
			{ return( (i==0)?x:((i==1)?y:z) ); };
//

	// accessor methods.
	float GetX(void) const { return x; };
	float GetY(void) const { return y; };
	float GetZ(void) const { return z; };

	float X(void) const { return x; };
	float Y(void) const { return y; };
	float Z(void) const { return z; };

	void SetX(float t)   { x   = t; };
	void SetY(float t)   { y   = t; };
	void SetZ(float t)   { z   = t; };

	bool IsSame(const Vector3d &v,float epsilon) const
	{
		float dx = fabsf( x - v.x );
		if ( dx > epsilon ) return false;
		float dy = fabsf( y - v.y );
		if ( dy > epsilon ) return false;
		float dz = fabsf( z - v.z );
		if ( dz > epsilon ) return false;
		return true;
	}


	float ComputeNormal(const Vector3d &A,
										 const Vector3d &B,
										 const Vector3d &C)
	{
		float vx,vy,vz,wx,wy,wz,vw_x,vw_y,vw_z,mag;

		vx = (B.x - C.x);
		vy = (B.y - C.y);
		vz = (B.z - C.z);

		wx = (A.x - B.x);
		wy = (A.y - B.y);
		wz = (A.z - B.z);

		vw_x = vy * wz - vz * wy;
		vw_y = vz * wx - vx * wz;
		vw_z = vx * wy - vy * wx;

		mag = sqrtf((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

		if ( mag < 0.000001f )
		{
			mag = 0;
		}
		else
		{
			mag = 1.0f/mag;
		}

		x = vw_x * mag;
		y = vw_y * mag;
		z = vw_z * mag;

		return mag;
	}


	void ScaleSumScale(float c0,float c1,const Vector3d &pos)
	{
		x = (x*c0) + (pos.x*c1);
		y = (y*c0) + (pos.y*c1);
		z = (z*c0) + (pos.z*c1);
	}

	void SwapYZ(void)
	{
		float t = y;
		y = z;
		z = t;
	};

	void Get(float *v) const
	{
		v[0] = x;
		v[1] = y;
		v[2] = z;
	};

	void Set(const int *p)
	{
		x = (float) p[0];
		y = (float) p[1];
		z = (float) p[2];
	}

	void Set(const float *p)
	{
		x = (float) p[0];
		y = (float) p[1];
		z = (float) p[2];
	}


	void Set(float a,float b,float c)
	{
		x = a;
		y = b;
		z = c;
	};

	void Zero(void)
	{
		x = y = z = 0;
	};

	const float* Ptr() const { return &x; }
	float* Ptr() { return &x; }


// return -(*this).
	Vector3d negative(void) const
	{
		Vector3d result;
		result.x = -x;
		result.y = -y;
		result.z = -z;
		return result;
	}

	float Magnitude(void) const
	{
		return float(sqrt(x * x + y * y + z * z));
	};

	float FastMagnitude(void) const
	{
		return float(sqrtf(x * x + y * y + z * z));
	};

	float FasterMagnitude(void) const
	{
		return float(sqrtf(x * x + y * y + z * z));
	};

	void Lerp(const Vector3d& from,const Vector3d& to,float slerp)
	{
		x = ((to.x - from.x) * slerp) + from.x;
		y = ((to.y - from.y) * slerp) + from.y;
		z = ((to.z - from.z) * slerp) + from.z;
	};

	// Highly specialized interpolate routine.  Will compute the interpolated position
	// shifted forward or backwards along the ray defined between (from) and (to).
	// Reason for existance is so that when a bullet collides with a wall, for
	// example, you can generate a graphic effect slightly *before* it hit the
	// wall so that the effect doesn't sort into the wall itself.
	void Interpolate(const Vector3d &from,const Vector3d &to,float offset)
	{
		x = to.x-from.x;
		y = to.y-from.y;
		z = to.z-from.z;
		float d = sqrtf( x*x + y*y + z*z );
		float recip = 1.0f / d;
		x*=recip;
		y*=recip;
		z*=recip; // normalize vector
		d+=offset; // shift along ray
		x = x*d + from.x;
		y = y*d + from.y;
		z = z*d + from.z;
	};

	bool BinaryEqual(const Vector3d &p) const
	{
		const int *source = (const int *) &x;
		const int *dest   = (const int *) &p.x;

		if ( source[0] == dest[0] &&
				 source[1] == dest[1] &&
				 source[2] == dest[2] ) return true;

		return false;
	};

	/*bool BinaryEqual(const Vector3d<int> &p) const
	{
		if ( x == p.x && y == p.y && z == p.z ) return true;
		return false;
	}
	*/



/** Computes the reflection vector between two vectors.*/
	void Reflection(const Vector3d &a,const Vector3d &b)// compute reflection vector.
	{
		Vector3d c;
		Vector3d d;

		float dot = a.Dot(b) * 2.0f;

		c = b * dot;

		d = c - a;

		x = -d.x;
		y = -d.y;
		z = -d.z;
	};

	void AngleAxis(float angle,const Vector3d& axis)
	{
		x = axis.x*angle;
		y = axis.y*angle;
		z = axis.z*angle;
	};

	float Length(void) const          // length of vector.
	{
		return float(sqrt( x*x + y*y + z*z ));
	};


	float ComputePlane(const Vector3d &A,
										 const Vector3d &B,
										 const Vector3d &C)
	{
		float vx,vy,vz,wx,wy,wz,vw_x,vw_y,vw_z,mag;

		vx = (B.x - C.x);
		vy = (B.y - C.y);
		vz = (B.z - C.z);

		wx = (A.x - B.x);
		wy = (A.y - B.y);
		wz = (A.z - B.z);

		vw_x = vy * wz - vz * wy;
		vw_y = vz * wx - vx * wz;
		vw_z = vx * wy - vy * wx;

		mag = sqrtf((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

		if ( mag < 0.000001f )
		{
			mag = 0;
		}
		else
		{
			mag = 1.0f/mag;
		}

		x = vw_x * mag;
		y = vw_y * mag;
		z = vw_z * mag;


		float D = 0.0f - ((x*A.x)+(y*A.y)+(z*A.z));

		return D;
	}


	float FastLength(void) const          // length of vector.
	{
		return float(sqrtf( x*x + y*y + z*z ));
	};
	

	float FasterLength(void) const          // length of vector.
	{
		return float(sqrtf( x*x + y*y + z*z ));
	};

	float Length2(void) const         // squared distance, prior to square root.
	{
		float l2 = x*x+y*y+z*z;
		return l2;
	};

	float Distance(const Vector3d &a) const   // distance between two points.
	{
		Vector3d d(a.x-x,a.y-y,a.z-z);
		return d.Length();
	}

	float FastDistance(const Vector3d &a) const   // distance between two points.
	{
		Vector3d d(a.x-x,a.y-y,a.z-z);
		return d.FastLength();
	}
	
	float FasterDistance(const Vector3d &a) const   // distance between two points.
	{
		Vector3d d(a.x-x,a.y-y,a.z-z);
		return d.FasterLength();
	}


	float DistanceXY(const Vector3d &a) const
	{
	float dx = a.x - x;
	float dy = a.y - y;
		float dist = dx*dx + dy*dy;
	return dist;
	}

	float Distance2(const Vector3d &a) const  // squared distance.
	{
		float dx = a.x - x;
		float dy = a.y - y;
		float dz = a.z - z;
		return dx*dx + dy*dy + dz*dz;
	};

	float Partial(const Vector3d &p) const
	{
		return (x*p.y) - (p.x*y);
	}

	float Area(const Vector3d &p1,const Vector3d &p2) const
	{
		float A = Partial(p1);
		A+= p1.Partial(p2);
		A+= p2.Partial(*this);
		return A*0.5f;
	}

	inline float Normalize(void)       // normalize to a unit vector, returns distance.
	{
		float d = sqrtf( static_cast< float >( x*x + y*y + z*z ) );
		if ( d > 0 )
		{
			float r = 1.0f / d;
			x *= r;
			y *= r;
			z *= r;
		}
		else
		{
			x = y = z = 1;
		}
		return d;
	};

	inline float FastNormalize(void)       // normalize to a unit vector, returns distance.
	{
		float d = sqrt( static_cast< float >( x*x + y*y + z*z ) );
		if ( d > 0 )
		{
			float r = 1.0f / d;
			x *= r;
			y *= r;
			z *= r;
		}
		else
		{
			x = y = z = 1;
		}
		return d;
	};

	inline float FasterNormalize(void)       // normalize to a unit vector, returns distance.
	{
		float d = sqrtf( static_cast< float >( x*x + y*y + z*z ) );
		if ( d > 0 )
		{
			float r = 1.0f / d;
			x *= r;
			y *= r;
			z *= r;
		}
		else
		{
			x = y = z = 1;
		}
		return d;
	};




	float Dot(const Vector3d &a) const        // computes dot product.
	{
		return (x * a.x + y * a.y + z * a.z );
	};


	Vector3d Cross( const Vector3d& other ) const
	{
		Vector3d result( y*other.z - z*other.y,  z*other.x - x*other.z,  x*other.y - y*other.x );

		return result;
	}

	void Cross(const Vector3d &a,const Vector3d &b)  // cross two vectors result in this one.
	{
		x = a.y*b.z - a.z*b.y;
		y = a.z*b.x - a.x*b.z;
		z = a.x*b.y - a.y*b.x;
	};

	/******************************************/
	// Check if next edge (b to c) turns inward
	//
	//    Edge from a to b is already in face
	//    Edge from b to c is being considered for addition to face
	/******************************************/
	bool Concave(const Vector3d& a,const Vector3d& b)
	{
		float vx,vy,vz,wx,wy,wz,vw_x,vw_y,vw_z,mag,nx,ny,nz,mag_a,mag_b;

		wx = b.x - a.x;
		wy = b.y - a.y;
		wz = b.z - a.z;

		mag_a = (float) sqrtf((wx * wx) + (wy * wy) + (wz * wz));

		vx = x - b.x;
		vy = y - b.y;
		vz = z - b.z;

		mag_b = (float) sqrtf((vx * vx) + (vy * vy) + (vz * vz));

		vw_x = (vy * wz) - (vz * wy);
		vw_y = (vz * wx) - (vx * wz);
		vw_z = (vx * wy) - (vy * wx);

		mag = (float) sqrtf((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

		// Check magnitude of cross product, which is a sine function
		// i.e., mag (a x b) = mag (a) * mag (b) * sin (theta);
		// If sin (theta) small, then angle between edges is very close to
		// 180, which we may want to call a concavity.	Setting the
		// CONCAVITY_TOLERANCE value greater than about 0.01 MAY cause
		// face consolidation to get stuck on particular face.	Most meshes
		// convert properly with a value of 0.0

		if (mag/(mag_a*mag_b) <= 0.0f )	return true;

		mag = 1.0f / mag;

		nx = vw_x * mag;
		ny = vw_y * mag;
		nz = vw_z * mag;

		// Dot product of tri normal with cross product result will
		// yield positive number if edges are convex (+1.0 if two tris
		// are coplanar), negative number if edges are concave (-1.0 if
		// two tris are coplanar.)

		mag = ( x * nx) + ( y * ny) + ( z * nz);

		if (mag > 0.0f ) return false;

		return(true);
	};

	bool PointTestXY(const Vector3d &i,const Vector3d &j) const
	{
		if (((( i.y <= y ) && ( y  < j.y )) ||
				 (( j.y <= y ) && ( y  < i.y ))) &&
					( x < (j.x - i.x) * (y - i.y) / (j.y - i.y) + i.x)) return true;
		return false;
	}

	// test to see if this point is inside the triangle specified by
	// these three points on the X/Y plane.
	bool PointInTriXY(const Vector3d &p1,
									const Vector3d &p2,
									const Vector3d &p3) const
	{
		float ax  = p3.x - p2.x;
		float ay  = p3.y - p2.y;
		float bx  = p1.x - p3.x;
		float by  = p1.y - p3.y;
		float cx  = p2.x - p1.x;
		float cy  = p2.y - p1.y;
		float apx = x - p1.x;
		float apy = y - p1.y;
		float bpx = x - p2.x;
		float bpy = y - p2.y;
		float cpx = x - p3.x;
		float cpy = y - p3.y;

		float aCROSSbp = ax*bpy - ay*bpx;
		float cCROSSap = cx*apy - cy*apx;
		float bCROSScp = bx*cpy - by*cpx;

		return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
	};

	// test to see if this point is inside the triangle specified by
	// these three points on the X/Y plane.
	bool PointInTriYZ(const Vector3d &p1,
									const Vector3d &p2,
									const Vector3d &p3) const
	{
		float ay  = p3.y - p2.y;
		float az  = p3.z - p2.z;
		float by  = p1.y - p3.y;
		float bz  = p1.z - p3.z;
		float cy  = p2.y - p1.y;
		float cz  = p2.z - p1.z;
		float apy = y - p1.y;
		float apz = z - p1.z;
		float bpy = y - p2.y;
		float bpz = z - p2.z;
		float cpy = y - p3.y;
		float cpz = z - p3.z;

		float aCROSSbp = ay*bpz - az*bpy;
		float cCROSSap = cy*apz - cz*apy;
		float bCROSScp = by*cpz - bz*cpy;

		return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
	};


	// test to see if this point is inside the triangle specified by
	// these three points on the X/Y plane.
	bool PointInTriXZ(const Vector3d &p1,
									const Vector3d &p2,
									const Vector3d &p3) const
	{
		float az  = p3.z - p2.z;
		float ax  = p3.x - p2.x;
		float bz  = p1.z - p3.z;
		float bx  = p1.x - p3.x;
		float cz  = p2.z - p1.z;
		float cx  = p2.x - p1.x;
		float apz = z - p1.z;
		float apx = x - p1.x;
		float bpz = z - p2.z;
		float bpx = x - p2.x;
		float cpz = z - p3.z;
		float cpx = x - p3.x;

		float aCROSSbp = az*bpx - ax*bpz;
		float cCROSSap = cz*apx - cx*apz;
		float bCROSScp = bz*cpx - bx*cpz;

		return ((aCROSSbp >= 0.0f) && (bCROSScp >= 0.0f) && (cCROSSap >= 0.0f));
	};

	// Given a point and a line (defined by two points), compute the closest point
	// in the line.  (The line is treated as infinitely long.)
	void NearestPointInLine(const Vector3d &point,
													const Vector3d &line0,
													const Vector3d &line1)
	{
		Vector3d &nearestPoint = *this;
		Vector3d lineDelta     = line1 - line0;

		// Handle degenerate lines
		if ( lineDelta == Vector3d(0, 0, 0) )
		{
			nearestPoint = line0;
		}
		else
		{
			float delta = (point-line0).Dot(lineDelta) / (lineDelta).Dot(lineDelta);
			nearestPoint = line0 + lineDelta*delta;
		}
	}

	// Given a point and a line segment (defined by two points), compute the closest point
	// in the line.  Cap the point at the endpoints of the line segment.
	void NearestPointInLineSegment(const Vector3d &point,
																 const Vector3d &line0,
																 const Vector3d &line1)
	{
		Vector3d &nearestPoint = *this;
		Vector3d lineDelta     = line1 - line0;

		// Handle degenerate lines
		if ( lineDelta == Vector3d(0, 0, 0) )
		{
			nearestPoint = line0;
		}
		else
		{
			float delta = (point-line0).Dot(lineDelta) / (lineDelta).Dot(lineDelta);

			// Clamp the point to conform to the segment's endpoints
			if ( delta < 0 )
				delta = 0;
			else if ( delta > 1 )
				delta = 1;

			nearestPoint = line0 + lineDelta*delta;
		}
	}

	// Given a point and a plane (defined by three points), compute the closest point
	// in the plane.  (The plane is unbounded.)
	void NearestPointInPlane(const Vector3d &point,
													 const Vector3d &triangle0,
													 const Vector3d &triangle1,
													 const Vector3d &triangle2)
	{
		Vector3d &nearestPoint = *this;
		Vector3d lineDelta0    = triangle1 - triangle0;
		Vector3d lineDelta1    = triangle2 - triangle0;
		Vector3d pointDelta    = point - triangle0;
		Vector3d normal;

		// Get the normal of the polygon (doesn't have to be a unit vector)
		normal.Cross(lineDelta0, lineDelta1);

		float delta = normal.Dot(pointDelta) / normal.Dot(normal);
		nearestPoint = point - normal*delta;
	}

	// Given a point and a plane (defined by a coplanar point and a normal), compute the closest point
	// in the plane.  (The plane is unbounded.)
	void NearestPointInPlane(const Vector3d &point,
													 const Vector3d &planePoint,
													 const Vector3d &planeNormal)
	{
		Vector3d &nearestPoint = *this;
		Vector3d pointDelta    = point - planePoint;

		float delta = planeNormal.Dot(pointDelta) / planeNormal.Dot(planeNormal);
		nearestPoint = point - planeNormal*delta;
	}

	// Given a point and a triangle (defined by three points), compute the closest point
	// in the triangle.  Clamp the point so it's confined to the area of the triangle.
	void NearestPointInTriangle(const Vector3d &point,
															const Vector3d &triangle0,
															const Vector3d &triangle1,
															const Vector3d &triangle2)
	{
		static const Vector3d zeroVector(0, 0, 0);

		Vector3d &nearestPoint = *this;

		Vector3d lineDelta0 = triangle1 - triangle0;
		Vector3d lineDelta1 = triangle2 - triangle0;

		// Handle degenerate triangles
		if ( (lineDelta0 == zeroVector) || (lineDelta1 == zeroVector) )
		{
			nearestPoint.NearestPointInLineSegment(point, triangle1, triangle2);
		}
		else if ( lineDelta0 == lineDelta1 )
		{
			nearestPoint.NearestPointInLineSegment(point, triangle0, triangle1);
		}

		else
		{
			Vector3d axis[3];
			axis[0].NearestPointInLine(triangle0, triangle1, triangle2);
			axis[1].NearestPointInLine(triangle1, triangle0, triangle2);
			axis[2].NearestPointInLine(triangle2, triangle0, triangle1);

			float axisDot[3];
			axisDot[0] = (triangle0-axis[0]).Dot(point-axis[0]);
			axisDot[1] = (triangle1-axis[1]).Dot(point-axis[1]);
			axisDot[2] = (triangle2-axis[2]).Dot(point-axis[2]);

			bool            bForce         = true;
			float            bestMagnitude2 = 0;
			float            closeMagnitude2;
			Vector3d closePoint;

			if ( axisDot[0] < 0 )
			{
				closePoint.NearestPointInLineSegment(point, triangle1, triangle2);
				closeMagnitude2 = point.Distance2(closePoint);
				if ( bForce || (bestMagnitude2 > closeMagnitude2) )
				{
					bForce         = false;
					bestMagnitude2 = closeMagnitude2;
					nearestPoint   = closePoint;
				}
			}
			if ( axisDot[1] < 0 )
			{
				closePoint.NearestPointInLineSegment(point, triangle0, triangle2);
				closeMagnitude2 = point.Distance2(closePoint);
				if ( bForce || (bestMagnitude2 > closeMagnitude2) )
				{
					bForce         = false;
					bestMagnitude2 = closeMagnitude2;
					nearestPoint   = closePoint;
				}
			}
			if ( axisDot[2] < 0 )
			{
				closePoint.NearestPointInLineSegment(point, triangle0, triangle1);
				closeMagnitude2 = point.Distance2(closePoint);
				if ( bForce || (bestMagnitude2 > closeMagnitude2) )
				{
					bForce         = false;
					bestMagnitude2 = closeMagnitude2;
					nearestPoint   = closePoint;
				}
			}

			// If bForce is true at this point, it means the nearest point lies
			// inside the triangle; use the nearest-point-on-a-plane equation
			if ( bForce )
			{
				Vector3d normal;

				// Get the normal of the polygon (doesn't have to be a unit vector)
				normal.Cross(lineDelta0, lineDelta1);

				Vector3d pointDelta = point - triangle0;
				float delta = normal.Dot(pointDelta) / normal.Dot(normal);

				nearestPoint = point - normal*delta;
			}
		}
	}


//private:

	float x;
	float y;
	float z;
};


class Vector2d
{
public:
	Vector2d(void) { };  // null constructor, does not inialize point.

	Vector2d(const Vector2d &a) // constructor copies existing vector.
	{
		x = a.x;
		y = a.y;
	};

	Vector2d(const float *t)
	{
		x = t[0];
		y = t[1];
	};


	Vector2d(float a,float b) // construct with initial point.
	{
		x = a;
		y = b;
	};

	const float* Ptr() const { return &x; }
	float* Ptr() { return &x; }

	Vector2d & operator+=(const Vector2d &a) // += operator.
	{
		x+=a.x;
		y+=a.y;
		return *this;
	};

	Vector2d & operator-=(const Vector2d &a)
	{
		x-=a.x;
		y-=a.y;
		return *this;
	};

	Vector2d & operator*=(const Vector2d &a)
	{
		x*=a.x;
		y*=a.y;
		return *this;
	};

	Vector2d & operator/=(const Vector2d &a)
	{
		x/=a.x;
		y/=a.y;
		return *this;
	};

	bool operator==(const Vector2d &a) const
	{
		if ( a.x == x && a.y == y ) return true;
		return false;
	};

	bool operator!=(const Vector2d &a) const
	{
		if ( a.x != x || a.y != y ) return true;
		return false;
	};

	Vector2d operator+(Vector2d a) const
	{
		a.x+=x;
		a.y+=y;
		return a;
	};

	Vector2d operator-(Vector2d a) const
	{
		a.x = x-a.x;
		a.y = y-a.y;
		return a;
	};

	Vector2d operator - (void) const
	{
		return negative();
	};

	Vector2d operator*(Vector2d a) const
	{
		a.x*=x;
		a.y*=y;
		return a;
	};

	Vector2d operator*(float c) const
	{
		Vector2d a;
		
		a.x = x * c;
		a.y = y * c;

		return a;
	};

	Vector2d operator/(Vector2d a) const
	{
		a.x = x/a.x;
		a.y = y/a.y;
		return a;
	};


	float Dot(const Vector2d &a) const        // computes dot product.
	{
		return (x * a.x + y * a.y );
	};

	float GetX(void) const { return x; };
	float GetY(void) const { return y; };

	void SetX(float t) { x   = t; };
	void SetY(float t) { y   = t; };

	void Set(float a,float b)
	{
		x = a;
		y = b;
	};

	void Zero(void)
	{
		x = y = 0;
	};

	Vector2d negative(void) const
	{
		Vector2d result;
		result.x = -x;
		result.y = -y;
		return result;
	}

	float magnitude(void) const
	{
		return (float) sqrtf(x * x + y * y );
	}

	float fastmagnitude(void) const
	{
		return (float) sqrtf(x * x + y * y );
	}
	
	float fastermagnitude(void) const
	{
		return (float) sqrtf( x * x + y * y );
	}

	void Reflection(Vector2d &a,Vector2d &b); // compute reflection vector.

	float Length(void) const          // length of vector.
	{
		return float(sqrtf( x*x + y*y ));
	};

	float FastLength(void) const          // length of vector.
	{
		return float(sqrtf( x*x + y*y ));
	};

	float FasterLength(void) const          // length of vector.
	{
		return float(sqrtf( x*x + y*y ));
	};

	float Length2(void)        // squared distance, prior to square root.
	{
		return x*x+y*y;
	}

	float Distance(const Vector2d &a) const   // distance between two points.
	{
		float dx = a.x - x;
		float dy = a.y - y;
		float d  = dx*dx+dy*dy;
		return sqrtf(d);
	};

	float FastDistance(const Vector2d &a) const   // distance between two points.
	{
		float dx = a.x - x;
		float dy = a.y - y;
		float d  = dx*dx+dy*dy;
		return sqrtf(d);
	};

	float FasterDistance(const Vector2d &a) const   // distance between two points.
	{
		float dx = a.x - x;
		float dy = a.y - y;
		float d  = dx*dx+dy*dy;
		return sqrtf(d);
	};

	float Distance2(Vector2d &a) // squared distance.
	{
		float dx = a.x - x;
		float dy = a.y - y;
		return dx*dx + dy *dy;
	};

	void Lerp(const Vector2d& from,const Vector2d& to,float slerp)
	{
		x = ((to.x - from.x)*slerp) + from.x;
		y = ((to.y - from.y)*slerp) + from.y;
	};


	void Cross(const Vector2d &a,const Vector2d &b)  // cross two vectors result in this one.
	{
		x = a.y*b.x - a.x*b.y;
		y = a.x*b.x - a.x*b.x;
	};

	float Normalize(void)       // normalize to a unit vector, returns distance.
	{
		float l = Length();
		if ( l != 0 )
		{
			l = float( 1 ) / l;
			x*=l;
			y*=l;
		}
		else
		{
			x = y = 0;
		}
		return l;
	};

	float FastNormalize(void)       // normalize to a unit vector, returns distance.
	{
		float l = FastLength();
		if ( l != 0 )
		{
			l = float( 1 ) / l;
			x*=l;
			y*=l;
		}
		else
		{
			x = y = 0;
		}
		return l;
	};

	float FasterNormalize(void)       // normalize to a unit vector, returns distance.
	{
		float l = FasterLength();
		if ( l != 0 )
		{
			l = float( 1 ) / l;
			x*=l;
			y*=l;
		}
		else
		{
			x = y = 0;
		}
		return l;
	};


	float x;
	float y;
};

class Line
{
public:
	Line(const Vector3d &from,const Vector3d &to)
	{
		mP1 = from;
		mP2 = to;
	};
	// JWR  Test for the intersection of two lines.

	bool Intersect(const Line& src,Vector3d &sect);
private:
	Vector3d mP1;
	Vector3d mP2;

};


typedef std::vector< Vector3d > Vector3dVector;
typedef std::vector< Vector2d > Vector2dVector;

inline  Vector3d operator * (float s, const Vector3d &v )
{ 
	Vector3d	Scaled(v.x*s, v.y*s, v.z*s);
	return(Scaled); 
}

inline  Vector2d	operator * (float s, const Vector2d &v )
 { 
	 Vector2d	Scaled(v.x*s, v.y*s);
	return(Scaled); 
 }

}

#endif
