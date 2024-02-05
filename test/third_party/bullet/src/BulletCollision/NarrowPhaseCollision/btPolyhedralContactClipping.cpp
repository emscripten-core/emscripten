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
///Separating axis rest based on work from Pierre Terdiman, see
///And contact clipping based on work from Simon Hobbs


#include "btPolyhedralContactClipping.h"
#include "BulletCollision/CollisionShapes/btConvexPolyhedron.h"

#include <float.h> //for FLT_MAX


// Clips a face to the back of a plane
void btPolyhedralContactClipping::clipFace(const btVertexArray& pVtxIn, btVertexArray& ppVtxOut, const btVector3& planeNormalWS,btScalar planeEqWS)
{
	
	int ve;
	btScalar ds, de;
	int numVerts = pVtxIn.size();
	if (numVerts < 2)
		return;

	btVector3 firstVertex=pVtxIn[pVtxIn.size()-1];
	btVector3 endVertex = pVtxIn[0];
	
	ds = planeNormalWS.dot(firstVertex)+planeEqWS;

	for (ve = 0; ve < numVerts; ve++)
	{
		endVertex=pVtxIn[ve];

		de = planeNormalWS.dot(endVertex)+planeEqWS;

		if (ds<0)
		{
			if (de<0)
			{
				// Start < 0, end < 0, so output endVertex
				ppVtxOut.push_back(endVertex);
			}
			else
			{
				// Start < 0, end >= 0, so output intersection
				ppVtxOut.push_back( 	firstVertex.lerp(endVertex,btScalar(ds * 1.f/(ds - de))));
			}
		}
		else
		{
			if (de<0)
			{
				// Start >= 0, end < 0 so output intersection and end
				ppVtxOut.push_back(firstVertex.lerp(endVertex,btScalar(ds * 1.f/(ds - de))));
				ppVtxOut.push_back(endVertex);
			}
		}
		firstVertex = endVertex;
		ds = de;
	}
}
#include <stdio.h>


static bool TestSepAxis(const btConvexPolyhedron& hullA, const btConvexPolyhedron& hullB, const btTransform& transA,const btTransform& transB, const btVector3& sep_axis, float& depth)
{
	float Min0,Max0;
	float Min1,Max1;
	hullA.project(transA,sep_axis, Min0, Max0);
	hullB.project(transB, sep_axis, Min1, Max1);

	if(Max0<Min1 || Max1<Min0)
		return false;

	float d0 = Max0 - Min1;
	assert(d0>=0.0f);
	float d1 = Max1 - Min0;
	assert(d1>=0.0f);
	depth = d0<d1 ? d0:d1;
	return true;
}



static int gActualSATPairTests=0;

inline bool IsAlmostZero(const btVector3& v)
{
	if(fabsf(v.x())>1e-6 || fabsf(v.y())>1e-6 || fabsf(v.z())>1e-6)	return false;
	return true;
}


bool btPolyhedralContactClipping::findSeparatingAxis(	const btConvexPolyhedron& hullA, const btConvexPolyhedron& hullB, const btTransform& transA,const btTransform& transB, btVector3& sep)
{
	gActualSATPairTests++;

#ifdef TEST_INTERNAL_OBJECTS
	const btVector3 c0 = transA * hullA.mLocalCenter;
	const btVector3 c1 = transB * hullB.mLocalCenter;
	const btVector3 DeltaC2 = c0 - c1;
#endif

	float dmin = FLT_MAX;
	int curPlaneTests=0;

	int numFacesA = hullA.m_faces.size();
	// Test normals from hullA
	for(int i=0;i<numFacesA;i++)
	{
		const btVector3 Normal(hullA.m_faces[i].m_plane[0], hullA.m_faces[i].m_plane[1], hullA.m_faces[i].m_plane[2]);
		const btVector3 faceANormalWS = transA.getBasis() * Normal;

		curPlaneTests++;
#ifdef TEST_INTERNAL_OBJECTS
		gExpectedNbTests++;
		if(gUseInternalObject && !TestInternalObjects(transA,transB,DeltaC2, faceANormalWS, hullA, hullB, dmin))
			continue;
		gActualNbTests++;
#endif

		float d;
		if(!TestSepAxis( hullA, hullB, transA,transB, faceANormalWS, d))
			return false;

		if(d<dmin)
		{
			dmin = d;
			sep = faceANormalWS;
		}
	}

	int numFacesB = hullB.m_faces.size();
	// Test normals from hullB
	for(int i=0;i<numFacesB;i++)
	{
		const btVector3 Normal(hullB.m_faces[i].m_plane[0], hullB.m_faces[i].m_plane[1], hullB.m_faces[i].m_plane[2]);
		const btVector3 WorldNormal = transB.getBasis() * Normal;

		curPlaneTests++;
#ifdef TEST_INTERNAL_OBJECTS
		gExpectedNbTests++;
		if(gUseInternalObject && !TestInternalObjects(transA,transB,DeltaC2, WorldNormal, hullA, hullB, dmin))
			continue;
		gActualNbTests++;
#endif

		float d;
		if(!TestSepAxis(hullA, hullB,transA,transB, WorldNormal,d))
			return false;

		if(d<dmin)
		{
			dmin = d;
			sep = WorldNormal;
		}
	}

	btVector3 edgeAstart,edgeAend,edgeBstart,edgeBend;

	int curEdgeEdge = 0;
	// Test edges
	for(int e0=0;e0<hullA.m_uniqueEdges.size();e0++)
	{
		const btVector3 edge0 = hullA.m_uniqueEdges[e0];
		const btVector3 WorldEdge0 = transA.getBasis() * edge0;
		for(int e1=0;e1<hullB.m_uniqueEdges.size();e1++)
		{
			const btVector3 edge1 = hullB.m_uniqueEdges[e1];
			const btVector3 WorldEdge1 = transB.getBasis() * edge1;

			btVector3 Cross = WorldEdge0.cross(WorldEdge1);
			curEdgeEdge++;
			if(!IsAlmostZero(Cross))
			{
				Cross = Cross.normalize();

#ifdef TEST_INTERNAL_OBJECTS
				gExpectedNbTests++;
				if(gUseInternalObject && !TestInternalObjects(transA,transB,DeltaC2, Cross, hullA, hullB, dmin))
					continue;
				gActualNbTests++;
#endif

				float dist;
				if(!TestSepAxis( hullA, hullB, transA,transB, Cross, dist))
					return false;

				if(dist<dmin)
				{
					dmin = dist;
					sep = Cross;
				}
			}
		}

	}

	const btVector3 deltaC = transB.getOrigin() - transA.getOrigin();
	if((deltaC.dot(sep))>0.0f)
		sep = -sep;

	return true;
}

void	btPolyhedralContactClipping::clipFaceAgainstHull(const btVector3& separatingNormal, const btConvexPolyhedron& hullA,  const btTransform& transA, btVertexArray& worldVertsB1, const btScalar minDist, btScalar maxDist,btDiscreteCollisionDetectorInterface::Result& resultOut)
{
	btVertexArray worldVertsB2;
	btVertexArray* pVtxIn = &worldVertsB1;
	btVertexArray* pVtxOut = &worldVertsB2;
	pVtxOut->reserve(pVtxIn->size());

	int closestFaceA=-1;
	{
		btScalar dmin = FLT_MAX;
		for(int face=0;face<hullA.m_faces.size();face++)
		{
			const btVector3 Normal(hullA.m_faces[face].m_plane[0], hullA.m_faces[face].m_plane[1], hullA.m_faces[face].m_plane[2]);
			const btVector3 faceANormalWS = transA.getBasis() * Normal;
		
			btScalar d = faceANormalWS.dot(separatingNormal);
			if (d < dmin)
			{
				dmin = d;
				closestFaceA = face;
			}
		}
	}
	if (closestFaceA<0)
		return;

	const btFace& polyA = hullA.m_faces[closestFaceA];

		// clip polygon to back of planes of all faces of hull A that are adjacent to witness face
	int numContacts = pVtxIn->size();
	int numVerticesA = polyA.m_indices.size();
	for(int e0=0;e0<numVerticesA;e0++)
	{
		/*const btVector3& a = hullA.m_vertices[polyA.m_indices[e0]];
		const btVector3& b = hullA.m_vertices[polyA.m_indices[(e0+1)%numVerticesA]];
		const btVector3 edge0 = a - b;
		const btVector3 WorldEdge0 = transA.getBasis() * edge0;
		*/

		int otherFace = polyA.m_connectedFaces[e0];
		btVector3 localPlaneNormal (hullA.m_faces[otherFace].m_plane[0],hullA.m_faces[otherFace].m_plane[1],hullA.m_faces[otherFace].m_plane[2]);
		btScalar localPlaneEq = hullA.m_faces[otherFace].m_plane[3];

		btVector3 planeNormalWS = transA.getBasis()*localPlaneNormal;
		btScalar planeEqWS=localPlaneEq-planeNormalWS.dot(transA.getOrigin());
		//clip face

		clipFace(*pVtxIn, *pVtxOut,planeNormalWS,planeEqWS);
		btSwap(pVtxIn,pVtxOut);
		pVtxOut->resize(0);
	}



//#define ONLY_REPORT_DEEPEST_POINT

	btVector3 point;
	

	// only keep points that are behind the witness face
	{
		btVector3 localPlaneNormal (polyA.m_plane[0],polyA.m_plane[1],polyA.m_plane[2]);
		btScalar localPlaneEq = polyA.m_plane[3];
		btVector3 planeNormalWS = transA.getBasis()*localPlaneNormal;
		btScalar planeEqWS=localPlaneEq-planeNormalWS.dot(transA.getOrigin());
		for (int i=0;i<pVtxIn->size();i++)
		{
			
			btScalar depth = planeNormalWS.dot(pVtxIn->at(i))+planeEqWS;
			if (depth <=maxDist && depth >=minDist)
			{
				btVector3 point = pVtxIn->at(i);
#ifdef ONLY_REPORT_DEEPEST_POINT
				curMaxDist = depth;
#else
#if 0
				if (depth<-3)
				{
					printf("error in btPolyhedralContactClipping depth = %f\n", depth);
					printf("likely wrong separatingNormal passed in\n");
				} 
#endif				
				resultOut.addContactPoint(separatingNormal,point,depth);
#endif
			}
		}
	}
#ifdef ONLY_REPORT_DEEPEST_POINT
	if (curMaxDist<maxDist)
	{
		resultOut.addContactPoint(separatingNormal,point,curMaxDist);
	}
#endif //ONLY_REPORT_DEEPEST_POINT

}

void	btPolyhedralContactClipping::clipHullAgainstHull(const btVector3& separatingNormal, const btConvexPolyhedron& hullA, const btConvexPolyhedron& hullB, const btTransform& transA,const btTransform& transB, const btScalar minDist, btScalar maxDist,btDiscreteCollisionDetectorInterface::Result& resultOut)
{

	btScalar curMaxDist=maxDist;
	int closestFaceB=-1;

	{
		btScalar dmax = -FLT_MAX;
		for(int face=0;face<hullB.m_faces.size();face++)
		{
			const btVector3 Normal(hullB.m_faces[face].m_plane[0], hullB.m_faces[face].m_plane[1], hullB.m_faces[face].m_plane[2]);
			const btVector3 WorldNormal = transB.getBasis() * Normal;

			btScalar d = WorldNormal.dot(separatingNormal);
			if (d > dmax)
			{
				dmax = d;
				closestFaceB = face;
			}
		}
	}



	if (closestFaceB<0)
	{
		return;
	}



	// setup initial clip face (minimizing face from hull B)
	btVertexArray worldVertsB1;
	{
		const btFace& polyB = hullB.m_faces[closestFaceB];
		const int numVertices = polyB.m_indices.size();
		for(int e0=0;e0<numVertices;e0++)
		{
			const btVector3& b = hullB.m_vertices[polyB.m_indices[e0]];
			worldVertsB1.push_back(transB*b);
		}
	}

	clipFaceAgainstHull(separatingNormal, hullA, transA,worldVertsB1, minDist, maxDist,resultOut);

}