/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2007 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef _SPU_PREFERRED_PENETRATION_DIRECTIONS_H
#define _SPU_PREFERRED_PENETRATION_DIRECTIONS_H


#include "BulletCollision/BroadphaseCollision/btBroadphaseProxy.h"

int		spuGetNumPreferredPenetrationDirections(int shapeType, void* shape)
{
	switch (shapeType)
    {
		case TRIANGLE_SHAPE_PROXYTYPE:
		{
			return 2;
			//spu_printf("2\n");
			break;
		}
		default:
			{
#if __ASSERT
        spu_printf("spuGetNumPreferredPenetrationDirections() - Unsupported bound type: %d.\n", shapeType);
#endif // __ASSERT
			}
	}

	return 0;	
}	

void	spuGetPreferredPenetrationDirection(int shapeType, void* shape, int index, btVector3& penetrationVector)
{


	switch (shapeType)
    {
		case TRIANGLE_SHAPE_PROXYTYPE:
		{
			btVector3* vertices = (btVector3*)shape;
			///calcNormal
			penetrationVector = (vertices[1]-vertices[0]).cross(vertices[2]-vertices[0]);
			penetrationVector.normalize();
			if (index)
				penetrationVector *= btScalar(-1.);
			break;
		}
		default:
			{
					
#if __ASSERT
        spu_printf("spuGetNumPreferredPenetrationDirections() - Unsupported bound type: %d.\n", shapeType);
#endif // __ASSERT
			}
	}
		
}

#endif //_SPU_PREFERRED_PENETRATION_DIRECTIONS_H
