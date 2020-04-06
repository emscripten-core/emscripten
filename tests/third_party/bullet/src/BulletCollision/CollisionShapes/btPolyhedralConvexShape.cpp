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

#include "BulletCollision/CollisionShapes/btPolyhedralConvexShape.h"
#include "btConvexPolyhedron.h"
#include "LinearMath/btConvexHullComputer.h"
#include <new>

btPolyhedralConvexShape::btPolyhedralConvexShape() :btConvexInternalShape(),
m_polyhedron(0)
{

}

btPolyhedralConvexShape::~btPolyhedralConvexShape()
{
	if (m_polyhedron)
	{
		btAlignedFree(m_polyhedron);
	}
}

bool	btPolyhedralConvexShape::initializePolyhedralFeatures()
{
	if (m_polyhedron)
		btAlignedFree(m_polyhedron);
	
	void* mem = btAlignedAlloc(sizeof(btConvexPolyhedron),16);
	m_polyhedron = new (mem) btConvexPolyhedron;

	btAlignedObjectArray<btVector3> tmpVertices;
	for (int i=0;i<getNumVertices();i++)
	{
		btVector3& newVertex = tmpVertices.expand();
		getVertex(i,newVertex);
	}

	btConvexHullComputer conv;
	conv.compute(&tmpVertices[0].getX(), sizeof(btVector3),tmpVertices.size(),0.f,0.f);

	

	btAlignedObjectArray<btVector3> faceNormals;
	int numFaces = conv.faces.size();
	faceNormals.resize(numFaces);
	btConvexHullComputer* convexUtil = &conv;

	
	
	m_polyhedron->m_faces.resize(numFaces);
	int numVertices = convexUtil->vertices.size();
	m_polyhedron->m_vertices.resize(numVertices);
	for (int p=0;p<numVertices;p++)
	{
		m_polyhedron->m_vertices[p] = convexUtil->vertices[p];
	}

	for (int i=0;i<numFaces;i++)
	{
		int face = convexUtil->faces[i];
		//printf("face=%d\n",face);
		const btConvexHullComputer::Edge*  firstEdge = &convexUtil->edges[face];
		const btConvexHullComputer::Edge*  edge = firstEdge;

		btVector3 edges[3];
		int numEdges = 0;
		//compute face normals

		btScalar maxCross2 = 0.f;
		int chosenEdge = -1;

		do
		{
			
			int src = edge->getSourceVertex();
			m_polyhedron->m_faces[i].m_indices.push_back(src);
			int targ = edge->getTargetVertex();
			btVector3 wa = convexUtil->vertices[src];

			btVector3 wb = convexUtil->vertices[targ];
			btVector3 newEdge = wb-wa;
			newEdge.normalize();
			if (numEdges<2)
				edges[numEdges++] = newEdge;

			edge = edge->getNextEdgeOfFace();
		} while (edge!=firstEdge);

		btScalar planeEq = 1e30f;

		
		if (numEdges==2)
		{
			faceNormals[i] = edges[0].cross(edges[1]);
			faceNormals[i].normalize();
			m_polyhedron->m_faces[i].m_plane[0] = -faceNormals[i].getX();
			m_polyhedron->m_faces[i].m_plane[1] = -faceNormals[i].getY();
			m_polyhedron->m_faces[i].m_plane[2] = -faceNormals[i].getZ();
			m_polyhedron->m_faces[i].m_plane[3] = planeEq;

		}
		else
		{
			btAssert(0);//degenerate?
			faceNormals[i].setZero();
		}

		for (int v=0;v<m_polyhedron->m_faces[i].m_indices.size();v++)
		{
			btScalar eq = m_polyhedron->m_vertices[m_polyhedron->m_faces[i].m_indices[v]].dot(faceNormals[i]);
			if (planeEq>eq)
			{
				planeEq=eq;
			}
		}
		m_polyhedron->m_faces[i].m_plane[3] = planeEq;
	}


	if (m_polyhedron->m_faces.size() && conv.vertices.size())
	{

		for (int f=0;f<m_polyhedron->m_faces.size();f++)
		{
			
			btVector3 planeNormal(m_polyhedron->m_faces[f].m_plane[0],m_polyhedron->m_faces[f].m_plane[1],m_polyhedron->m_faces[f].m_plane[2]);
			btScalar planeEq = m_polyhedron->m_faces[f].m_plane[3];

			btVector3 supVec = localGetSupportingVertex(-planeNormal);

			if (supVec.dot(planeNormal)<planeEq)
			{
				m_polyhedron->m_faces[f].m_plane[0] *= -1;
				m_polyhedron->m_faces[f].m_plane[1] *= -1;
				m_polyhedron->m_faces[f].m_plane[2] *= -1;
				m_polyhedron->m_faces[f].m_plane[3] *= -1;
				int numVerts = m_polyhedron->m_faces[f].m_indices.size();
				for (int v=0;v<numVerts/2;v++)
				{
					btSwap(m_polyhedron->m_faces[f].m_indices[v],m_polyhedron->m_faces[f].m_indices[numVerts-1-v]);
				}
			}
		}
	}

	

	m_polyhedron->initialize();

	return true;
}


btVector3	btPolyhedralConvexShape::localGetSupportingVertexWithoutMargin(const btVector3& vec0)const
{


	btVector3 supVec(0,0,0);
#ifndef __SPU__
	int i;
	btScalar maxDot(btScalar(-BT_LARGE_FLOAT));

	btVector3 vec = vec0;
	btScalar lenSqr = vec.length2();
	if (lenSqr < btScalar(0.0001))
	{
		vec.setValue(1,0,0);
	} else
	{
		btScalar rlen = btScalar(1.) / btSqrt(lenSqr );
		vec *= rlen;
	}

	btVector3 vtx;
	btScalar newDot;

	for (i=0;i<getNumVertices();i++)
	{
		getVertex(i,vtx);
		newDot = vec.dot(vtx);
		if (newDot > maxDot)
		{
			maxDot = newDot;
			supVec = vtx;
		}
	}

	
#endif //__SPU__
	return supVec;
}



void	btPolyhedralConvexShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const
{
#ifndef __SPU__
	int i;

	btVector3 vtx;
	btScalar newDot;

	for (i=0;i<numVectors;i++)
	{
		supportVerticesOut[i][3] = btScalar(-BT_LARGE_FLOAT);
	}

	for (int j=0;j<numVectors;j++)
	{
	
		const btVector3& vec = vectors[j];

		for (i=0;i<getNumVertices();i++)
		{
			getVertex(i,vtx);
			newDot = vec.dot(vtx);
			if (newDot > supportVerticesOut[j][3])
			{
				//WARNING: don't swap next lines, the w component would get overwritten!
				supportVerticesOut[j] = vtx;
				supportVerticesOut[j][3] = newDot;
			}
		}
	}
#endif //__SPU__
}



void	btPolyhedralConvexShape::calculateLocalInertia(btScalar mass,btVector3& inertia) const
{
#ifndef __SPU__
	//not yet, return box inertia

	btScalar margin = getMargin();

	btTransform ident;
	ident.setIdentity();
	btVector3 aabbMin,aabbMax;
	getAabb(ident,aabbMin,aabbMax);
	btVector3 halfExtents = (aabbMax-aabbMin)*btScalar(0.5);

	btScalar lx=btScalar(2.)*(halfExtents.x()+margin);
	btScalar ly=btScalar(2.)*(halfExtents.y()+margin);
	btScalar lz=btScalar(2.)*(halfExtents.z()+margin);
	const btScalar x2 = lx*lx;
	const btScalar y2 = ly*ly;
	const btScalar z2 = lz*lz;
	const btScalar scaledmass = mass * btScalar(0.08333333);

	inertia = scaledmass * (btVector3(y2+z2,x2+z2,x2+y2));
#endif //__SPU__
}



void	btPolyhedralConvexAabbCachingShape::setLocalScaling(const btVector3& scaling)
{
	btConvexInternalShape::setLocalScaling(scaling);
	recalcLocalAabb();
}

btPolyhedralConvexAabbCachingShape::btPolyhedralConvexAabbCachingShape()
:btPolyhedralConvexShape(),
m_localAabbMin(1,1,1),
m_localAabbMax(-1,-1,-1),
m_isLocalAabbValid(false)
{
}

void btPolyhedralConvexAabbCachingShape::getAabb(const btTransform& trans,btVector3& aabbMin,btVector3& aabbMax) const
{
	getNonvirtualAabb(trans,aabbMin,aabbMax,getMargin());
}

void	btPolyhedralConvexAabbCachingShape::recalcLocalAabb()
{
	m_isLocalAabbValid = true;
	
	#if 1
	static const btVector3 _directions[] =
	{
		btVector3( 1.,  0.,  0.),
		btVector3( 0.,  1.,  0.),
		btVector3( 0.,  0.,  1.),
		btVector3( -1., 0.,  0.),
		btVector3( 0., -1.,  0.),
		btVector3( 0.,  0., -1.)
	};
	
	btVector3 _supporting[] =
	{
		btVector3( 0., 0., 0.),
		btVector3( 0., 0., 0.),
		btVector3( 0., 0., 0.),
		btVector3( 0., 0., 0.),
		btVector3( 0., 0., 0.),
		btVector3( 0., 0., 0.)
	};
	
	batchedUnitVectorGetSupportingVertexWithoutMargin(_directions, _supporting, 6);
	
	for ( int i = 0; i < 3; ++i )
	{
		m_localAabbMax[i] = _supporting[i][i] + m_collisionMargin;
		m_localAabbMin[i] = _supporting[i + 3][i] - m_collisionMargin;
	}
	
	#else

	for (int i=0;i<3;i++)
	{
		btVector3 vec(btScalar(0.),btScalar(0.),btScalar(0.));
		vec[i] = btScalar(1.);
		btVector3 tmp = localGetSupportingVertex(vec);
		m_localAabbMax[i] = tmp[i]+m_collisionMargin;
		vec[i] = btScalar(-1.);
		tmp = localGetSupportingVertex(vec);
		m_localAabbMin[i] = tmp[i]-m_collisionMargin;
	}
	#endif
}




