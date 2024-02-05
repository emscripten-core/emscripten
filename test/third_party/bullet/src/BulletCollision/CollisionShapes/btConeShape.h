/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2009 Erwin Coumans  http://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef BT_CONE_MINKOWSKI_H
#define BT_CONE_MINKOWSKI_H

#include "btConvexInternalShape.h"
#include "BulletCollision/BroadphaseCollision/btBroadphaseProxy.h" // for the types

///The btConeShape implements a cone shape primitive, centered around the origin and aligned with the Y axis. The btConeShapeX is aligned around the X axis and btConeShapeZ around the Z axis.
class btConeShape : public btConvexInternalShape

{

	btScalar m_sinAngle;
	btScalar m_radius;
	btScalar m_height;
	int		m_coneIndices[3];
	btVector3 coneLocalSupport(const btVector3& v) const;


public:
	btConeShape (btScalar radius,btScalar height);
	
	virtual btVector3	localGetSupportingVertex(const btVector3& vec) const;
	virtual btVector3	localGetSupportingVertexWithoutMargin(const btVector3& vec) const;
	virtual void	batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const;

	btScalar getRadius() const { return m_radius;}
	btScalar getHeight() const { return m_height;}


	virtual void	calculateLocalInertia(btScalar mass,btVector3& inertia) const
	{
		btTransform identity;
		identity.setIdentity();
		btVector3 aabbMin,aabbMax;
		getAabb(identity,aabbMin,aabbMax);

		btVector3 halfExtents = (aabbMax-aabbMin)*btScalar(0.5);

		btScalar margin = getMargin();

		btScalar lx=btScalar(2.)*(halfExtents.x()+margin);
		btScalar ly=btScalar(2.)*(halfExtents.y()+margin);
		btScalar lz=btScalar(2.)*(halfExtents.z()+margin);
		const btScalar x2 = lx*lx;
		const btScalar y2 = ly*ly;
		const btScalar z2 = lz*lz;
		const btScalar scaledmass = mass * btScalar(0.08333333);

		inertia = scaledmass * (btVector3(y2+z2,x2+z2,x2+y2));

//		inertia.x() = scaledmass * (y2+z2);
//		inertia.y() = scaledmass * (x2+z2);
//		inertia.z() = scaledmass * (x2+y2);
	}


		virtual const char*	getName()const 
		{
			return "Cone";
		}
		
		///choose upAxis index
		void	setConeUpIndex(int upIndex);
		
		int	getConeUpIndex() const
		{
			return m_coneIndices[1];
		}

	virtual void	setLocalScaling(const btVector3& scaling);

};

///btConeShape implements a Cone shape, around the X axis
class btConeShapeX : public btConeShape
{
	public:
		btConeShapeX(btScalar radius,btScalar height);
};

///btConeShapeZ implements a Cone shape, around the Z axis
class btConeShapeZ : public btConeShape
{
	public:
		btConeShapeZ(btScalar radius,btScalar height);
};
#endif //BT_CONE_MINKOWSKI_H

