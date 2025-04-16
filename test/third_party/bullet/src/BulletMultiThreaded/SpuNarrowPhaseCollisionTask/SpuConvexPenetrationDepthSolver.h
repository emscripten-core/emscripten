
/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#ifndef SPU_CONVEX_PENETRATION_DEPTH_H
#define SPU_CONVEX_PENETRATION_DEPTH_H



class btStackAlloc;
class btIDebugDraw;
#include "BulletCollision/NarrowphaseCollision/btConvexPenetrationDepthSolver.h"

#include "LinearMath/btTransform.h"


///ConvexPenetrationDepthSolver provides an interface for penetration depth calculation.
class SpuConvexPenetrationDepthSolver : public btConvexPenetrationDepthSolver
{
public:	
	
	virtual ~SpuConvexPenetrationDepthSolver() {};
	virtual bool calcPenDepth( SpuVoronoiSimplexSolver& simplexSolver,
	        void* convexA,void* convexB,int shapeTypeA, int shapeTypeB, float marginA, float marginB,
            btTransform& transA,const btTransform& transB,
			btVector3& v, btVector3& pa, btVector3& pb,
			class btIDebugDraw* debugDraw,btStackAlloc* stackAlloc,
			struct SpuConvexPolyhedronVertexData* convexVertexDataA,
			struct SpuConvexPolyhedronVertexData* convexVertexDataB
			) const = 0;


};



#endif //SPU_CONVEX_PENETRATION_DEPTH_H

