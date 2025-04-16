/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2011 Advanced Micro Devices, Inc.  http://bulletphysics.org

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


///This file was written by Erwin Coumans


#ifndef _BT_POLYHEDRAL_FEATURES_H
#define _BT_POLYHEDRAL_FEATURES_H

#include "LinearMath/btTransform.h"
#include "LinearMath/btAlignedObjectArray.h"


struct btFace
{
	btAlignedObjectArray<int>	m_indices;
	btAlignedObjectArray<int>	m_connectedFaces;
	float	m_plane[4];
};


class btConvexPolyhedron
{
	public:
	btConvexPolyhedron();
	virtual	~btConvexPolyhedron();

	btAlignedObjectArray<btVector3>	m_vertices;
	btAlignedObjectArray<btFace>	m_faces;
	btAlignedObjectArray<btVector3> m_uniqueEdges;
	btVector3		m_localCenter;

	void	initialize();

	void project(const btTransform& trans, const btVector3& dir, float& min, float& max) const;
};

	
#endif //_BT_POLYHEDRAL_FEATURES_H


