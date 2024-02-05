/*
Copyright (c) 2003-2006 Gino van den Bergen / Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/



#ifndef BT_SIMD__QUATERNION_H_
#define BT_SIMD__QUATERNION_H_


#include "btVector3.h"
#include "btQuadWord.h"

/**@brief The btQuaternion implements quaternion to perform linear algebra rotations in combination with btMatrix3x3, btVector3 and btTransform. */
class btQuaternion : public btQuadWord {
public:
  /**@brief No initialization constructor */
	btQuaternion() {}

	//		template <typename btScalar>
	//		explicit Quaternion(const btScalar *v) : Tuple4<btScalar>(v) {}
  /**@brief Constructor from scalars */
	btQuaternion(const btScalar& x, const btScalar& y, const btScalar& z, const btScalar& w) 
		: btQuadWord(x, y, z, w) 
	{}
  /**@brief Axis angle Constructor
   * @param axis The axis which the rotation is around
   * @param angle The magnitude of the rotation around the angle (Radians) */
	btQuaternion(const btVector3& axis, const btScalar& angle) 
	{ 
		setRotation(axis, angle); 
	}
  /**@brief Constructor from Euler angles
   * @param yaw Angle around Y unless BT_EULER_DEFAULT_ZYX defined then Z
   * @param pitch Angle around X unless BT_EULER_DEFAULT_ZYX defined then Y
   * @param roll Angle around Z unless BT_EULER_DEFAULT_ZYX defined then X */
	btQuaternion(const btScalar& yaw, const btScalar& pitch, const btScalar& roll)
	{ 
#ifndef BT_EULER_DEFAULT_ZYX
		setEuler(yaw, pitch, roll); 
#else
		setEulerZYX(yaw, pitch, roll); 
#endif 
	}
  /**@brief Set the rotation using axis angle notation 
   * @param axis The axis around which to rotate
   * @param angle The magnitude of the rotation in Radians */
	void setRotation(const btVector3& axis, const btScalar& angle)
	{
		btScalar d = axis.length();
		btAssert(d != btScalar(0.0));
		btScalar s = btSin(angle * btScalar(0.5)) / d;
		setValue(axis.x() * s, axis.y() * s, axis.z() * s, 
			btCos(angle * btScalar(0.5)));
	}
  /**@brief Set the quaternion using Euler angles
   * @param yaw Angle around Y
   * @param pitch Angle around X
   * @param roll Angle around Z */
	void setEuler(const btScalar& yaw, const btScalar& pitch, const btScalar& roll)
	{
		btScalar halfYaw = btScalar(yaw) * btScalar(0.5);  
		btScalar halfPitch = btScalar(pitch) * btScalar(0.5);  
		btScalar halfRoll = btScalar(roll) * btScalar(0.5);  
		btScalar cosYaw = btCos(halfYaw);
		btScalar sinYaw = btSin(halfYaw);
		btScalar cosPitch = btCos(halfPitch);
		btScalar sinPitch = btSin(halfPitch);
		btScalar cosRoll = btCos(halfRoll);
		btScalar sinRoll = btSin(halfRoll);
		setValue(cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,
			cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw,
			sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw,
			cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw);
	}
  /**@brief Set the quaternion using euler angles 
   * @param yaw Angle around Z
   * @param pitch Angle around Y
   * @param roll Angle around X */
	void setEulerZYX(const btScalar& yaw, const btScalar& pitch, const btScalar& roll)
	{
		btScalar halfYaw = btScalar(yaw) * btScalar(0.5);  
		btScalar halfPitch = btScalar(pitch) * btScalar(0.5);  
		btScalar halfRoll = btScalar(roll) * btScalar(0.5);  
		btScalar cosYaw = btCos(halfYaw);
		btScalar sinYaw = btSin(halfYaw);
		btScalar cosPitch = btCos(halfPitch);
		btScalar sinPitch = btSin(halfPitch);
		btScalar cosRoll = btCos(halfRoll);
		btScalar sinRoll = btSin(halfRoll);
		setValue(sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw, //x
                         cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw, //y
                         cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw, //z
                         cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw); //formerly yzx
	}
  /**@brief Add two quaternions
   * @param q The quaternion to add to this one */
	SIMD_FORCE_INLINE	btQuaternion& operator+=(const btQuaternion& q)
	{
		m_floats[0] += q.x(); m_floats[1] += q.y(); m_floats[2] += q.z(); m_floats[3] += q.m_floats[3];
		return *this;
	}

  /**@brief Subtract out a quaternion
   * @param q The quaternion to subtract from this one */
	btQuaternion& operator-=(const btQuaternion& q) 
	{
		m_floats[0] -= q.x(); m_floats[1] -= q.y(); m_floats[2] -= q.z(); m_floats[3] -= q.m_floats[3];
		return *this;
	}

  /**@brief Scale this quaternion
   * @param s The scalar to scale by */
	btQuaternion& operator*=(const btScalar& s)
	{
		m_floats[0] *= s; m_floats[1] *= s; m_floats[2] *= s; m_floats[3] *= s;
		return *this;
	}

  /**@brief Multiply this quaternion by q on the right
   * @param q The other quaternion 
   * Equivilant to this = this * q */
	btQuaternion& operator*=(const btQuaternion& q)
	{
		setValue(m_floats[3] * q.x() + m_floats[0] * q.m_floats[3] + m_floats[1] * q.z() - m_floats[2] * q.y(),
			m_floats[3] * q.y() + m_floats[1] * q.m_floats[3] + m_floats[2] * q.x() - m_floats[0] * q.z(),
			m_floats[3] * q.z() + m_floats[2] * q.m_floats[3] + m_floats[0] * q.y() - m_floats[1] * q.x(),
			m_floats[3] * q.m_floats[3] - m_floats[0] * q.x() - m_floats[1] * q.y() - m_floats[2] * q.z());
		return *this;
	}
  /**@brief Return the dot product between this quaternion and another
   * @param q The other quaternion */
	btScalar dot(const btQuaternion& q) const
	{
		return m_floats[0] * q.x() + m_floats[1] * q.y() + m_floats[2] * q.z() + m_floats[3] * q.m_floats[3];
	}

  /**@brief Return the length squared of the quaternion */
	btScalar length2() const
	{
		return dot(*this);
	}

  /**@brief Return the length of the quaternion */
	btScalar length() const
	{
		return btSqrt(length2());
	}

  /**@brief Normalize the quaternion 
   * Such that x^2 + y^2 + z^2 +w^2 = 1 */
	btQuaternion& normalize() 
	{
		return *this /= length();
	}

  /**@brief Return a scaled version of this quaternion
   * @param s The scale factor */
	SIMD_FORCE_INLINE btQuaternion
	operator*(const btScalar& s) const
	{
		return btQuaternion(x() * s, y() * s, z() * s, m_floats[3] * s);
	}


  /**@brief Return an inversely scaled versionof this quaternion
   * @param s The inverse scale factor */
	btQuaternion operator/(const btScalar& s) const
	{
		btAssert(s != btScalar(0.0));
		return *this * (btScalar(1.0) / s);
	}

  /**@brief Inversely scale this quaternion
   * @param s The scale factor */
	btQuaternion& operator/=(const btScalar& s) 
	{
		btAssert(s != btScalar(0.0));
		return *this *= btScalar(1.0) / s;
	}

  /**@brief Return a normalized version of this quaternion */
	btQuaternion normalized() const 
	{
		return *this / length();
	} 
  /**@brief Return the angle between this quaternion and the other 
   * @param q The other quaternion */
	btScalar angle(const btQuaternion& q) const 
	{
		btScalar s = btSqrt(length2() * q.length2());
		btAssert(s != btScalar(0.0));
		return btAcos(dot(q) / s);
	}
  /**@brief Return the angle of rotation represented by this quaternion */
	btScalar getAngle() const 
	{
		btScalar s = btScalar(2.) * btAcos(m_floats[3]);
		return s;
	}

	/**@brief Return the axis of the rotation represented by this quaternion */
	btVector3 getAxis() const
	{
		btScalar s_squared = btScalar(1.) - btPow(m_floats[3], btScalar(2.));
		if (s_squared < btScalar(10.) * SIMD_EPSILON) //Check for divide by zero
			return btVector3(1.0, 0.0, 0.0);  // Arbitrary
		btScalar s = btSqrt(s_squared);
		return btVector3(m_floats[0] / s, m_floats[1] / s, m_floats[2] / s);
	}

	/**@brief Return the inverse of this quaternion */
	btQuaternion inverse() const
	{
		return btQuaternion(-m_floats[0], -m_floats[1], -m_floats[2], m_floats[3]);
	}

  /**@brief Return the sum of this quaternion and the other 
   * @param q2 The other quaternion */
	SIMD_FORCE_INLINE btQuaternion
	operator+(const btQuaternion& q2) const
	{
		const btQuaternion& q1 = *this;
		return btQuaternion(q1.x() + q2.x(), q1.y() + q2.y(), q1.z() + q2.z(), q1.m_floats[3] + q2.m_floats[3]);
	}

  /**@brief Return the difference between this quaternion and the other 
   * @param q2 The other quaternion */
	SIMD_FORCE_INLINE btQuaternion
	operator-(const btQuaternion& q2) const
	{
		const btQuaternion& q1 = *this;
		return btQuaternion(q1.x() - q2.x(), q1.y() - q2.y(), q1.z() - q2.z(), q1.m_floats[3] - q2.m_floats[3]);
	}

  /**@brief Return the negative of this quaternion 
   * This simply negates each element */
	SIMD_FORCE_INLINE btQuaternion operator-() const
	{
		const btQuaternion& q2 = *this;
		return btQuaternion( - q2.x(), - q2.y(),  - q2.z(),  - q2.m_floats[3]);
	}
  /**@todo document this and it's use */
	SIMD_FORCE_INLINE btQuaternion farthest( const btQuaternion& qd) const 
	{
		btQuaternion diff,sum;
		diff = *this - qd;
		sum = *this + qd;
		if( diff.dot(diff) > sum.dot(sum) )
			return qd;
		return (-qd);
	}

	/**@todo document this and it's use */
	SIMD_FORCE_INLINE btQuaternion nearest( const btQuaternion& qd) const 
	{
		btQuaternion diff,sum;
		diff = *this - qd;
		sum = *this + qd;
		if( diff.dot(diff) < sum.dot(sum) )
			return qd;
		return (-qd);
	}


  /**@brief Return the quaternion which is the result of Spherical Linear Interpolation between this and the other quaternion
   * @param q The other quaternion to interpolate with 
   * @param t The ratio between this and q to interpolate.  If t = 0 the result is this, if t=1 the result is q.
   * Slerp interpolates assuming constant velocity.  */
	btQuaternion slerp(const btQuaternion& q, const btScalar& t) const
	{
		btScalar theta = angle(q);
		if (theta != btScalar(0.0))
		{
			btScalar d = btScalar(1.0) / btSin(theta);
			btScalar s0 = btSin((btScalar(1.0) - t) * theta);
			btScalar s1 = btSin(t * theta);   
                        if (dot(q) < 0) // Take care of long angle case see http://en.wikipedia.org/wiki/Slerp
                          return btQuaternion((m_floats[0] * s0 + -q.x() * s1) * d,
                                              (m_floats[1] * s0 + -q.y() * s1) * d,
                                              (m_floats[2] * s0 + -q.z() * s1) * d,
                                              (m_floats[3] * s0 + -q.m_floats[3] * s1) * d);
                        else
                          return btQuaternion((m_floats[0] * s0 + q.x() * s1) * d,
                                              (m_floats[1] * s0 + q.y() * s1) * d,
                                              (m_floats[2] * s0 + q.z() * s1) * d,
                                              (m_floats[3] * s0 + q.m_floats[3] * s1) * d);
                        
		}
		else
		{
			return *this;
		}
	}

	static const btQuaternion&	getIdentity()
	{
		static const btQuaternion identityQuat(btScalar(0.),btScalar(0.),btScalar(0.),btScalar(1.));
		return identityQuat;
	}

	SIMD_FORCE_INLINE const btScalar& getW() const { return m_floats[3]; }

	
};


/**@brief Return the negative of a quaternion */
SIMD_FORCE_INLINE btQuaternion
operator-(const btQuaternion& q)
{
	return btQuaternion(-q.x(), -q.y(), -q.z(), -q.w());
}



/**@brief Return the product of two quaternions */
SIMD_FORCE_INLINE btQuaternion
operator*(const btQuaternion& q1, const btQuaternion& q2) {
	return btQuaternion(q1.w() * q2.x() + q1.x() * q2.w() + q1.y() * q2.z() - q1.z() * q2.y(),
		q1.w() * q2.y() + q1.y() * q2.w() + q1.z() * q2.x() - q1.x() * q2.z(),
		q1.w() * q2.z() + q1.z() * q2.w() + q1.x() * q2.y() - q1.y() * q2.x(),
		q1.w() * q2.w() - q1.x() * q2.x() - q1.y() * q2.y() - q1.z() * q2.z()); 
}

SIMD_FORCE_INLINE btQuaternion
operator*(const btQuaternion& q, const btVector3& w)
{
	return btQuaternion( q.w() * w.x() + q.y() * w.z() - q.z() * w.y(),
		q.w() * w.y() + q.z() * w.x() - q.x() * w.z(),
		q.w() * w.z() + q.x() * w.y() - q.y() * w.x(),
		-q.x() * w.x() - q.y() * w.y() - q.z() * w.z()); 
}

SIMD_FORCE_INLINE btQuaternion
operator*(const btVector3& w, const btQuaternion& q)
{
	return btQuaternion( w.x() * q.w() + w.y() * q.z() - w.z() * q.y(),
		w.y() * q.w() + w.z() * q.x() - w.x() * q.z(),
		w.z() * q.w() + w.x() * q.y() - w.y() * q.x(),
		-w.x() * q.x() - w.y() * q.y() - w.z() * q.z()); 
}

/**@brief Calculate the dot product between two quaternions */
SIMD_FORCE_INLINE btScalar 
dot(const btQuaternion& q1, const btQuaternion& q2) 
{ 
	return q1.dot(q2); 
}


/**@brief Return the length of a quaternion */
SIMD_FORCE_INLINE btScalar
length(const btQuaternion& q) 
{ 
	return q.length(); 
}

/**@brief Return the angle between two quaternions*/
SIMD_FORCE_INLINE btScalar
angle(const btQuaternion& q1, const btQuaternion& q2) 
{ 
	return q1.angle(q2); 
}

/**@brief Return the inverse of a quaternion*/
SIMD_FORCE_INLINE btQuaternion
inverse(const btQuaternion& q) 
{
	return q.inverse();
}

/**@brief Return the result of spherical linear interpolation betwen two quaternions 
 * @param q1 The first quaternion
 * @param q2 The second quaternion 
 * @param t The ration between q1 and q2.  t = 0 return q1, t=1 returns q2 
 * Slerp assumes constant velocity between positions. */
SIMD_FORCE_INLINE btQuaternion
slerp(const btQuaternion& q1, const btQuaternion& q2, const btScalar& t) 
{
	return q1.slerp(q2, t);
}

SIMD_FORCE_INLINE btVector3 
quatRotate(const btQuaternion& rotation, const btVector3& v) 
{
	btQuaternion q = rotation * v;
	q *= rotation.inverse();
	return btVector3(q.getX(),q.getY(),q.getZ());
}

SIMD_FORCE_INLINE btQuaternion 
shortestArcQuat(const btVector3& v0, const btVector3& v1) // Game Programming Gems 2.10. make sure v0,v1 are normalized
{
	btVector3 c = v0.cross(v1);
	btScalar  d = v0.dot(v1);

	if (d < -1.0 + SIMD_EPSILON)
	{
		btVector3 n,unused;
		btPlaneSpace1(v0,n,unused);
		return btQuaternion(n.x(),n.y(),n.z(),0.0f); // just pick any vector that is orthogonal to v0
	}

	btScalar  s = btSqrt((1.0f + d) * 2.0f);
	btScalar rs = 1.0f / s;

	return btQuaternion(c.getX()*rs,c.getY()*rs,c.getZ()*rs,s * 0.5f);
}

SIMD_FORCE_INLINE btQuaternion 
shortestArcQuatNormalize2(btVector3& v0,btVector3& v1)
{
	v0.normalize();
	v1.normalize();
	return shortestArcQuat(v0,v1);
}

#endif //BT_SIMD__QUATERNION_H_




