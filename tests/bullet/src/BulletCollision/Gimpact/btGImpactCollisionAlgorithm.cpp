/*
This source file is part of GIMPACT Library.

For the latest info, see http://gimpact.sourceforge.net/

Copyright (c) 2007 Francisco Leon Najera. C.C. 80087371.
email: projectileman@yahoo.com


This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
/*
Author: Francisco Len Nßjera
Concave-Concave Collision

*/

#include "BulletCollision/CollisionDispatch/btManifoldResult.h"
#include "LinearMath/btIDebugDraw.h"
#include "BulletCollision/CollisionDispatch/btCollisionObject.h"
#include "BulletCollision/CollisionShapes/btBoxShape.h"
#include "btGImpactCollisionAlgorithm.h"
#include "btContactProcessing.h"
#include "LinearMath/btQuickprof.h"


//! Class for accessing the plane equation
class btPlaneShape : public btStaticPlaneShape
{
public:

	btPlaneShape(const btVector3& v, float f)
		:btStaticPlaneShape(v,f)
	{
	}

	void get_plane_equation(btVector4 &equation)
	{
		equation[0] = m_planeNormal[0];
		equation[1] = m_planeNormal[1];
		equation[2] = m_planeNormal[2];
		equation[3] = m_planeConstant;
	}


	void get_plane_equation_transformed(const btTransform & trans,btVector4 &equation)
	{
		equation[0] = trans.getBasis().getRow(0).dot(m_planeNormal);
		equation[1] = trans.getBasis().getRow(1).dot(m_planeNormal);
		equation[2] = trans.getBasis().getRow(2).dot(m_planeNormal);
		equation[3] = trans.getOrigin().dot(m_planeNormal) + m_planeConstant;
	}
};



//////////////////////////////////////////////////////////////////////////////////////////////
#ifdef TRI_COLLISION_PROFILING

btClock g_triangle_clock;

float g_accum_triangle_collision_time = 0;
int g_count_triangle_collision = 0;

void bt_begin_gim02_tri_time()
{
	g_triangle_clock.reset();
}

void bt_end_gim02_tri_time()
{
	g_accum_triangle_collision_time += g_triangle_clock.getTimeMicroseconds();
	g_count_triangle_collision++;
}
#endif //TRI_COLLISION_PROFILING
//! Retrieving shapes shapes
/*!
Declared here due of insuficent space on Pool allocators
*/
//!@{
class GIM_ShapeRetriever
{
public:
	btGImpactShapeInterface * m_gim_shape;
	btTriangleShapeEx m_trishape;
	btTetrahedronShapeEx m_tetrashape;

public:
	class ChildShapeRetriever
	{
	public:
		GIM_ShapeRetriever * m_parent;
		virtual btCollisionShape * getChildShape(int index)
		{
			return m_parent->m_gim_shape->getChildShape(index);
		}
		virtual ~ChildShapeRetriever() {}
	};

	class TriangleShapeRetriever:public ChildShapeRetriever
	{
	public:

		virtual btCollisionShape * getChildShape(int index)
		{
			m_parent->m_gim_shape->getBulletTriangle(index,m_parent->m_trishape);
			return &m_parent->m_trishape;
		}
		virtual ~TriangleShapeRetriever() {}
	};

	class TetraShapeRetriever:public ChildShapeRetriever
	{
	public:

		virtual btCollisionShape * getChildShape(int index)
		{
			m_parent->m_gim_shape->getBulletTetrahedron(index,m_parent->m_tetrashape);
			return &m_parent->m_tetrashape;
		}
	};
public:
	ChildShapeRetriever m_child_retriever;
	TriangleShapeRetriever m_tri_retriever;
	TetraShapeRetriever  m_tetra_retriever;
	ChildShapeRetriever * m_current_retriever;

	GIM_ShapeRetriever(btGImpactShapeInterface * gim_shape)
	{
		m_gim_shape = gim_shape;
		//select retriever
		if(m_gim_shape->needsRetrieveTriangles())
		{
			m_current_retriever = &m_tri_retriever;
		}
		else if(m_gim_shape->needsRetrieveTetrahedrons())
		{
			m_current_retriever = &m_tetra_retriever;
		}
		else
		{
			m_current_retriever = &m_child_retriever;
		}

		m_current_retriever->m_parent = this;
	}

	btCollisionShape * getChildShape(int index)
	{
		return m_current_retriever->getChildShape(index);
	}


};



//!@}


#ifdef TRI_COLLISION_PROFILING

//! Gets the average time in miliseconds of tree collisions
float btGImpactCollisionAlgorithm::getAverageTreeCollisionTime()
{
	return btGImpactBoxSet::getAverageTreeCollisionTime();

}

//! Gets the average time in miliseconds of triangle collisions
float btGImpactCollisionAlgorithm::getAverageTriangleCollisionTime()
{
	if(g_count_triangle_collision == 0) return 0;

	float avgtime = g_accum_triangle_collision_time;
	avgtime /= (float)g_count_triangle_collision;

	g_accum_triangle_collision_time = 0;
	g_count_triangle_collision = 0;

	return avgtime;
}

#endif //TRI_COLLISION_PROFILING



btGImpactCollisionAlgorithm::btGImpactCollisionAlgorithm( const btCollisionAlgorithmConstructionInfo& ci, btCollisionObject* body0,btCollisionObject* body1)
: btActivatingCollisionAlgorithm(ci,body0,body1)
{
	m_manifoldPtr = NULL;
	m_convex_algorithm = NULL;
}

btGImpactCollisionAlgorithm::~btGImpactCollisionAlgorithm()
{
	clearCache();
}





void btGImpactCollisionAlgorithm::addContactPoint(btCollisionObject * body0,
				btCollisionObject * body1,
				const btVector3 & point,
				const btVector3 & normal,
				btScalar distance)
{
	m_resultOut->setShapeIdentifiersA(m_part0,m_triface0);
	m_resultOut->setShapeIdentifiersB(m_part1,m_triface1);
	checkManifold(body0,body1);
	m_resultOut->addContactPoint(normal,point,distance);
}


void btGImpactCollisionAlgorithm::shape_vs_shape_collision(
					  btCollisionObject * body0,
					  btCollisionObject * body1,
					  btCollisionShape * shape0,
					  btCollisionShape * shape1)
{

	btCollisionShape* tmpShape0 = body0->getCollisionShape();
	btCollisionShape* tmpShape1 = body1->getCollisionShape();
	
	body0->internalSetTemporaryCollisionShape(shape0);
	body1->internalSetTemporaryCollisionShape(shape1);

	{
		btCollisionAlgorithm* algor = newAlgorithm(body0,body1);
		// post :	checkManifold is called

		m_resultOut->setShapeIdentifiersA(m_part0,m_triface0);
		m_resultOut->setShapeIdentifiersB(m_part1,m_triface1);

		algor->processCollision(body0,body1,*m_dispatchInfo,m_resultOut);

		algor->~btCollisionAlgorithm();
		m_dispatcher->freeCollisionAlgorithm(algor);
	}

	body0->internalSetTemporaryCollisionShape(tmpShape0);
	body1->internalSetTemporaryCollisionShape(tmpShape1);
}

void btGImpactCollisionAlgorithm::convex_vs_convex_collision(
					  btCollisionObject * body0,
					  btCollisionObject * body1,
					  btCollisionShape * shape0,
					  btCollisionShape * shape1)
{

	btCollisionShape* tmpShape0 = body0->getCollisionShape();
	btCollisionShape* tmpShape1 = body1->getCollisionShape();
	
	body0->internalSetTemporaryCollisionShape(shape0);
	body1->internalSetTemporaryCollisionShape(shape1);


	m_resultOut->setShapeIdentifiersA(m_part0,m_triface0);
	m_resultOut->setShapeIdentifiersB(m_part1,m_triface1);

	checkConvexAlgorithm(body0,body1);
	m_convex_algorithm->processCollision(body0,body1,*m_dispatchInfo,m_resultOut);

	body0->internalSetTemporaryCollisionShape(tmpShape0);
	body1->internalSetTemporaryCollisionShape(tmpShape1);

}




void btGImpactCollisionAlgorithm::gimpact_vs_gimpact_find_pairs(
					  const btTransform & trans0,
					  const btTransform & trans1,
					  btGImpactShapeInterface * shape0,
					  btGImpactShapeInterface * shape1,btPairSet & pairset)
{
	if(shape0->hasBoxSet() && shape1->hasBoxSet())
	{
		btGImpactBoxSet::find_collision(shape0->getBoxSet(),trans0,shape1->getBoxSet(),trans1,pairset);
	}
	else
	{
		btAABB boxshape0;
		btAABB boxshape1;
		int i = shape0->getNumChildShapes();

		while(i--)
		{
			shape0->getChildAabb(i,trans0,boxshape0.m_min,boxshape0.m_max);

			int j = shape1->getNumChildShapes();
			while(j--)
			{
				shape1->getChildAabb(i,trans1,boxshape1.m_min,boxshape1.m_max);

				if(boxshape1.has_collision(boxshape0))
				{
					pairset.push_pair(i,j);
				}
			}
		}
	}


}


void btGImpactCollisionAlgorithm::gimpact_vs_shape_find_pairs(
					  const btTransform & trans0,
					  const btTransform & trans1,
					  btGImpactShapeInterface * shape0,
					  btCollisionShape * shape1,
					  btAlignedObjectArray<int> & collided_primitives)
{

	btAABB boxshape;


	if(shape0->hasBoxSet())
	{
		btTransform trans1to0 = trans0.inverse();
		trans1to0 *= trans1;

		shape1->getAabb(trans1to0,boxshape.m_min,boxshape.m_max);

		shape0->getBoxSet()->boxQuery(boxshape, collided_primitives);
	}
	else
	{
		shape1->getAabb(trans1,boxshape.m_min,boxshape.m_max);

		btAABB boxshape0;
		int i = shape0->getNumChildShapes();

		while(i--)
		{
			shape0->getChildAabb(i,trans0,boxshape0.m_min,boxshape0.m_max);

			if(boxshape.has_collision(boxshape0))
			{
				collided_primitives.push_back(i);
			}
		}

	}

}


void btGImpactCollisionAlgorithm::collide_gjk_triangles(btCollisionObject * body0,
				  btCollisionObject * body1,
				  btGImpactMeshShapePart * shape0,
				  btGImpactMeshShapePart * shape1,
				  const int * pairs, int pair_count)
{
	btTriangleShapeEx tri0;
	btTriangleShapeEx tri1;

	shape0->lockChildShapes();
	shape1->lockChildShapes();

	const int * pair_pointer = pairs;

	while(pair_count--)
	{

		m_triface0 = *(pair_pointer);
		m_triface1 = *(pair_pointer+1);
		pair_pointer+=2;



		shape0->getBulletTriangle(m_triface0,tri0);
		shape1->getBulletTriangle(m_triface1,tri1);


		//collide two convex shapes
		if(tri0.overlap_test_conservative(tri1))
		{
			convex_vs_convex_collision(body0,body1,&tri0,&tri1);
		}

	}

	shape0->unlockChildShapes();
	shape1->unlockChildShapes();
}

void btGImpactCollisionAlgorithm::collide_sat_triangles(btCollisionObject * body0,
					  btCollisionObject * body1,
					  btGImpactMeshShapePart * shape0,
					  btGImpactMeshShapePart * shape1,
					  const int * pairs, int pair_count)
{
	btTransform orgtrans0 = body0->getWorldTransform();
	btTransform orgtrans1 = body1->getWorldTransform();

	btPrimitiveTriangle ptri0;
	btPrimitiveTriangle ptri1;
	GIM_TRIANGLE_CONTACT contact_data;

	shape0->lockChildShapes();
	shape1->lockChildShapes();

	const int * pair_pointer = pairs;

	while(pair_count--)
	{

		m_triface0 = *(pair_pointer);
		m_triface1 = *(pair_pointer+1);
		pair_pointer+=2;


		shape0->getPrimitiveTriangle(m_triface0,ptri0);
		shape1->getPrimitiveTriangle(m_triface1,ptri1);

		#ifdef TRI_COLLISION_PROFILING
		bt_begin_gim02_tri_time();
		#endif

		ptri0.applyTransform(orgtrans0);
		ptri1.applyTransform(orgtrans1);


		//build planes
		ptri0.buildTriPlane();
		ptri1.buildTriPlane();
		// test conservative



		if(ptri0.overlap_test_conservative(ptri1))
		{
			if(ptri0.find_triangle_collision_clip_method(ptri1,contact_data))
			{

				int j = contact_data.m_point_count;
				while(j--)
				{

					addContactPoint(body0, body1,
								contact_data.m_points[j],
								contact_data.m_separating_normal,
								-contact_data.m_penetration_depth);
				}
			}
		}

		#ifdef TRI_COLLISION_PROFILING
		bt_end_gim02_tri_time();
		#endif

	}

	shape0->unlockChildShapes();
	shape1->unlockChildShapes();

}


void btGImpactCollisionAlgorithm::gimpact_vs_gimpact(
						btCollisionObject * body0,
					   	btCollisionObject * body1,
					  	btGImpactShapeInterface * shape0,
					  	btGImpactShapeInterface * shape1)
{

	if(shape0->getGImpactShapeType()==CONST_GIMPACT_TRIMESH_SHAPE)
	{
		btGImpactMeshShape * meshshape0 = static_cast<btGImpactMeshShape *>(shape0);
		m_part0 = meshshape0->getMeshPartCount();

		while(m_part0--)
		{
			gimpact_vs_gimpact(body0,body1,meshshape0->getMeshPart(m_part0),shape1);
		}

		return;
	}

	if(shape1->getGImpactShapeType()==CONST_GIMPACT_TRIMESH_SHAPE)
	{
		btGImpactMeshShape * meshshape1 = static_cast<btGImpactMeshShape *>(shape1);
		m_part1 = meshshape1->getMeshPartCount();

		while(m_part1--)
		{

			gimpact_vs_gimpact(body0,body1,shape0,meshshape1->getMeshPart(m_part1));

		}

		return;
	}


	btTransform orgtrans0 = body0->getWorldTransform();
	btTransform orgtrans1 = body1->getWorldTransform();

	btPairSet pairset;

	gimpact_vs_gimpact_find_pairs(orgtrans0,orgtrans1,shape0,shape1,pairset);

	if(pairset.size()== 0) return;

	if(shape0->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART &&
		shape1->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART)
	{
		btGImpactMeshShapePart * shapepart0 = static_cast<btGImpactMeshShapePart * >(shape0);
		btGImpactMeshShapePart * shapepart1 = static_cast<btGImpactMeshShapePart * >(shape1);
		//specialized function
		#ifdef BULLET_TRIANGLE_COLLISION
		collide_gjk_triangles(body0,body1,shapepart0,shapepart1,&pairset[0].m_index1,pairset.size());
		#else
		collide_sat_triangles(body0,body1,shapepart0,shapepart1,&pairset[0].m_index1,pairset.size());
		#endif

		return;
	}

	//general function

	shape0->lockChildShapes();
	shape1->lockChildShapes();

	GIM_ShapeRetriever retriever0(shape0);
	GIM_ShapeRetriever retriever1(shape1);

	bool child_has_transform0 = shape0->childrenHasTransform();
	bool child_has_transform1 = shape1->childrenHasTransform();

	int i = pairset.size();
	while(i--)
	{
		GIM_PAIR * pair = &pairset[i];
		m_triface0 = pair->m_index1;
		m_triface1 = pair->m_index2;
		btCollisionShape * colshape0 = retriever0.getChildShape(m_triface0);
		btCollisionShape * colshape1 = retriever1.getChildShape(m_triface1);

		if(child_has_transform0)
		{
			body0->setWorldTransform(orgtrans0*shape0->getChildTransform(m_triface0));
		}

		if(child_has_transform1)
		{
			body1->setWorldTransform(orgtrans1*shape1->getChildTransform(m_triface1));
		}

		//collide two convex shapes
		convex_vs_convex_collision(body0,body1,colshape0,colshape1);


		if(child_has_transform0)
		{
			body0->setWorldTransform(orgtrans0);
		}

		if(child_has_transform1)
		{
			body1->setWorldTransform(orgtrans1);
		}

	}

	shape0->unlockChildShapes();
	shape1->unlockChildShapes();
}

void btGImpactCollisionAlgorithm::gimpact_vs_shape(btCollisionObject * body0,
				  btCollisionObject * body1,
				  btGImpactShapeInterface * shape0,
				  btCollisionShape * shape1,bool swapped)
{
	if(shape0->getGImpactShapeType()==CONST_GIMPACT_TRIMESH_SHAPE)
	{
		btGImpactMeshShape * meshshape0 = static_cast<btGImpactMeshShape *>(shape0);
		int& part = swapped ? m_part1 : m_part0;
		part = meshshape0->getMeshPartCount();

		while(part--)
		{

			gimpact_vs_shape(body0,
				  body1,
				  meshshape0->getMeshPart(part),
				  shape1,swapped);

		}

		return;
	}

	#ifdef GIMPACT_VS_PLANE_COLLISION
	if(shape0->getGImpactShapeType() == CONST_GIMPACT_TRIMESH_SHAPE_PART &&
		shape1->getShapeType() == STATIC_PLANE_PROXYTYPE)
	{
		btGImpactMeshShapePart * shapepart = static_cast<btGImpactMeshShapePart *>(shape0);
		btStaticPlaneShape * planeshape = static_cast<btStaticPlaneShape * >(shape1);
		gimpacttrimeshpart_vs_plane_collision(body0,body1,shapepart,planeshape,swapped);
		return;
	}

	#endif



	if(shape1->isCompound())
	{
		btCompoundShape * compoundshape = static_cast<btCompoundShape *>(shape1);
		gimpact_vs_compoundshape(body0,body1,shape0,compoundshape,swapped);
		return;
	}
	else if(shape1->isConcave())
	{
		btConcaveShape * concaveshape = static_cast<btConcaveShape *>(shape1);
		gimpact_vs_concave(body0,body1,shape0,concaveshape,swapped);
		return;
	}


	btTransform orgtrans0 = body0->getWorldTransform();

	btTransform orgtrans1 = body1->getWorldTransform();

	btAlignedObjectArray<int> collided_results;

	gimpact_vs_shape_find_pairs(orgtrans0,orgtrans1,shape0,shape1,collided_results);

	if(collided_results.size() == 0) return;


	shape0->lockChildShapes();

	GIM_ShapeRetriever retriever0(shape0);


	bool child_has_transform0 = shape0->childrenHasTransform();


	int i = collided_results.size();

	while(i--)
	{
		int child_index = collided_results[i];
        if(swapped)
    		m_triface1 = child_index;
        else
            m_triface0 = child_index;

		btCollisionShape * colshape0 = retriever0.getChildShape(child_index);

		if(child_has_transform0)
		{
			body0->setWorldTransform(orgtrans0*shape0->getChildTransform(child_index));
		}

		//collide two shapes
		if(swapped)
		{
			shape_vs_shape_collision(body1,body0,shape1,colshape0);
		}
		else
		{
			shape_vs_shape_collision(body0,body1,colshape0,shape1);
		}

		//restore transforms
		if(child_has_transform0)
		{
			body0->setWorldTransform(orgtrans0);
		}

	}

	shape0->unlockChildShapes();

}

void btGImpactCollisionAlgorithm::gimpact_vs_compoundshape(btCollisionObject * body0,
				  btCollisionObject * body1,
				  btGImpactShapeInterface * shape0,
				  btCompoundShape * shape1,bool swapped)
{
	btTransform orgtrans1 = body1->getWorldTransform();

	int i = shape1->getNumChildShapes();
	while(i--)
	{

		btCollisionShape * colshape1 = shape1->getChildShape(i);
		btTransform childtrans1 = orgtrans1*shape1->getChildTransform(i);

		body1->setWorldTransform(childtrans1);

		//collide child shape
		gimpact_vs_shape(body0, body1,
					  shape0,colshape1,swapped);


		//restore transforms
		body1->setWorldTransform(orgtrans1);
	}
}

void btGImpactCollisionAlgorithm::gimpacttrimeshpart_vs_plane_collision(
					  btCollisionObject * body0,
					  btCollisionObject * body1,
					  btGImpactMeshShapePart * shape0,
					  btStaticPlaneShape * shape1,bool swapped)
{


	btTransform orgtrans0 = body0->getWorldTransform();
	btTransform orgtrans1 = body1->getWorldTransform();

	btPlaneShape * planeshape = static_cast<btPlaneShape *>(shape1);
	btVector4 plane;
	planeshape->get_plane_equation_transformed(orgtrans1,plane);

	//test box against plane

	btAABB tribox;
	shape0->getAabb(orgtrans0,tribox.m_min,tribox.m_max);
	tribox.increment_margin(planeshape->getMargin());

	if( tribox.plane_classify(plane)!= BT_CONST_COLLIDE_PLANE) return;

	shape0->lockChildShapes();

	btScalar margin = shape0->getMargin() + planeshape->getMargin();

	btVector3 vertex;
	int vi = shape0->getVertexCount();
	while(vi--)
	{
		shape0->getVertex(vi,vertex);
		vertex = orgtrans0(vertex);

		btScalar distance = vertex.dot(plane) - plane[3] - margin;

		if(distance<0.0)//add contact
		{
			if(swapped)
			{
				addContactPoint(body1, body0,
					vertex,
					-plane,
					distance);
			}
			else
			{
				addContactPoint(body0, body1,
					vertex,
					plane,
					distance);
			}
		}
	}

	shape0->unlockChildShapes();
}




class btGImpactTriangleCallback: public btTriangleCallback
{
public:
	btGImpactCollisionAlgorithm * algorithm;
	btCollisionObject * body0;
	btCollisionObject * body1;
	btGImpactShapeInterface * gimpactshape0;
	bool swapped;
	btScalar margin;

	virtual void processTriangle(btVector3* triangle, int partId, int triangleIndex)
	{
		btTriangleShapeEx tri1(triangle[0],triangle[1],triangle[2]);
		tri1.setMargin(margin);
        if(swapped)
        {
            algorithm->setPart0(partId);
            algorithm->setFace0(triangleIndex);
        }
        else
        {
            algorithm->setPart1(partId);
            algorithm->setFace1(triangleIndex);
        }
		algorithm->gimpact_vs_shape(
							body0,body1,gimpactshape0,&tri1,swapped);
	}
};




void btGImpactCollisionAlgorithm::gimpact_vs_concave(
				  btCollisionObject * body0,
				  btCollisionObject * body1,
				  btGImpactShapeInterface * shape0,
				  btConcaveShape * shape1,bool swapped)
{
	//create the callback
	btGImpactTriangleCallback tricallback;
	tricallback.algorithm = this;
	tricallback.body0 = body0;
	tricallback.body1 = body1;
	tricallback.gimpactshape0 = shape0;
	tricallback.swapped = swapped;
	tricallback.margin = shape1->getMargin();

	//getting the trimesh AABB
	btTransform gimpactInConcaveSpace;

	gimpactInConcaveSpace = body1->getWorldTransform().inverse() * body0->getWorldTransform();

	btVector3 minAABB,maxAABB;
	shape0->getAabb(gimpactInConcaveSpace,minAABB,maxAABB);

	shape1->processAllTriangles(&tricallback,minAABB,maxAABB);

}



void btGImpactCollisionAlgorithm::processCollision (btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
    clearCache();

    m_resultOut = resultOut;
	m_dispatchInfo = &dispatchInfo;
    btGImpactShapeInterface * gimpactshape0;
    btGImpactShapeInterface * gimpactshape1;

	if (body0->getCollisionShape()->getShapeType()==GIMPACT_SHAPE_PROXYTYPE)
	{
		gimpactshape0 = static_cast<btGImpactShapeInterface *>(body0->getCollisionShape());

		if( body1->getCollisionShape()->getShapeType()==GIMPACT_SHAPE_PROXYTYPE )
		{
			gimpactshape1 = static_cast<btGImpactShapeInterface *>(body1->getCollisionShape());

			gimpact_vs_gimpact(body0,body1,gimpactshape0,gimpactshape1);
		}
		else
		{
			gimpact_vs_shape(body0,body1,gimpactshape0,body1->getCollisionShape(),false);
		}

	}
	else if (body1->getCollisionShape()->getShapeType()==GIMPACT_SHAPE_PROXYTYPE )
	{
		gimpactshape1 = static_cast<btGImpactShapeInterface *>(body1->getCollisionShape());

		gimpact_vs_shape(body1,body0,gimpactshape1,body0->getCollisionShape(),true);
	}
}


btScalar btGImpactCollisionAlgorithm::calculateTimeOfImpact(btCollisionObject* body0,btCollisionObject* body1,const btDispatcherInfo& dispatchInfo,btManifoldResult* resultOut)
{
	return 1.f;

}

///////////////////////////////////// REGISTERING ALGORITHM //////////////////////////////////////////////



//! Use this function for register the algorithm externally
void btGImpactCollisionAlgorithm::registerAlgorithm(btCollisionDispatcher * dispatcher)
{

	static btGImpactCollisionAlgorithm::CreateFunc s_gimpact_cf;

	int i;

	for ( i = 0;i < MAX_BROADPHASE_COLLISION_TYPES ;i++ )
	{
		dispatcher->registerCollisionCreateFunc(GIMPACT_SHAPE_PROXYTYPE,i ,&s_gimpact_cf);
	}

	for ( i = 0;i < MAX_BROADPHASE_COLLISION_TYPES ;i++ )
	{
		dispatcher->registerCollisionCreateFunc(i,GIMPACT_SHAPE_PROXYTYPE ,&s_gimpact_cf);
	}

}
