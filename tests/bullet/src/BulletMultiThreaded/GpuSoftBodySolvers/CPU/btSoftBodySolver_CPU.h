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

#ifndef BT_ACCELERATED_SOFT_BODY_CPU_SOLVER_H
#define BT_ACCELERATED_SOFT_BODY_CPU_SOLVER_H

#include "vectormath/vmInclude.h"
#include "BulletSoftBody/btSoftBodySolvers.h"
#include "BulletSoftBody/btSoftBodySolverVertexBuffer.h"
#include "BulletMultiThreaded/GpuSoftBodySolvers/CPU/btSoftBodySolverData.h"

struct btCPUCollisionShapeDescription
{
	int softBodyIdentifier;
	int collisionShapeType;
	Vectormath::Aos::Transform3 shapeTransform;
	union
	{
		struct Sphere
		{
			float radius;
		} sphere;
		struct Capsule
		{
			float radius;
			float halfHeight;
			int upAxis;
		} capsule;
	} shapeInformation;
	
	float margin;
	float friction;
	Vectormath::Aos::Vector3 linearVelocity;
	Vectormath::Aos::Vector3 angularVelocity;

	btCPUCollisionShapeDescription()
	{
		collisionShapeType = 0;
		margin = 0;
		friction = 0;
	}
};

class btCPUSoftBodySolver : public btSoftBodySolver
{
protected:
	/**
	 * Entry in the collision shape array.
	 * Specifies the shape type, the transform matrix and the necessary details of the collisionShape.
	 */


	// Public because output classes need it. This is a better encapsulation to break in the short term
	// Than having the solvers themselves need dependencies on DX, CL etc unnecessarily
public:
	
	struct CollisionObjectIndices
	{
		CollisionObjectIndices( int f, int e )
		{
			firstObject = f;
			endObject = e;
		}

		int firstObject;
		int endObject;
	};

	/**
	 * SoftBody class to maintain information about a soft body instance
	 * within a solver.
	 * This data addresses the main solver arrays.
	 */
	class btAcceleratedSoftBodyInterface
	{
	protected:
		/** Current number of vertices that are part of this cloth */
		int m_numVertices;
		/** Maximum number of vertices allocated to be part of this cloth */
		int m_maxVertices;
		/** Current number of triangles that are part of this cloth */
		int m_numTriangles;
		/** Maximum number of triangles allocated to be part of this cloth */
		int m_maxTriangles;
		/** Index of first vertex in the world allocated to this cloth */
		int m_firstVertex;
		/** Index of first triangle in the world allocated to this cloth */
		int m_firstTriangle;
		/** Index of first link in the world allocated to this cloth */
		int m_firstLink;
		/** Maximum number of links allocated to this cloth */
		int m_maxLinks;
		/** Current number of links allocated to this cloth */
		int m_numLinks;

		/** The actual soft body this data represents */
		btSoftBody *m_softBody;


	public:
		btAcceleratedSoftBodyInterface( btSoftBody *softBody ) :
		  m_softBody( softBody )
		{
			m_numVertices = 0;
			m_maxVertices = 0;
			m_numTriangles = 0;
			m_maxTriangles = 0;
			m_firstVertex = 0;
			m_firstTriangle = 0;
			m_firstLink = 0;
			m_maxLinks = 0;
			m_numLinks = 0;
		}
		int getNumVertices() const
		{
			return m_numVertices;
		}

		int getNumTriangles() const
		{
			return m_numTriangles;
		}

		int getMaxVertices() const
		{
			return m_maxVertices;
		}

		int getMaxTriangles() const
		{
			return m_maxTriangles;
		}

		int getFirstVertex() const
		{
			return m_firstVertex;
		}

		int getFirstTriangle() const
		{
			return m_firstTriangle;
		}

		/**
		 * Update the bounds in the btSoftBody object
		 */
		void updateBounds( const btVector3 &lowerBound, const btVector3 &upperBound );

		// TODO: All of these set functions will have to do checks and
		// update the world because restructuring of the arrays will be necessary
		// Reasonable use of "friend"?
		void setNumVertices( int numVertices )
		{
			m_numVertices = numVertices;
		}	
		
		void setNumTriangles( int numTriangles )
		{
			m_numTriangles = numTriangles;
		}

		void setMaxVertices( int maxVertices )
		{
			m_maxVertices = maxVertices;
		}

		void setMaxTriangles( int maxTriangles )
		{
			m_maxTriangles = maxTriangles;
		}

		void setFirstVertex( int firstVertex )
		{
			m_firstVertex = firstVertex;
		}

		void setFirstTriangle( int firstTriangle )
		{
			m_firstTriangle = firstTriangle;
		}

		void setMaxLinks( int maxLinks )
		{
			m_maxLinks = maxLinks;
		}

		void setNumLinks( int numLinks )
		{
			m_numLinks = numLinks;
		}

		void setFirstLink( int firstLink )
		{
			m_firstLink = firstLink;
		}

		int getMaxLinks() const
		{
			return m_maxLinks;
		}

		int getNumLinks() const
		{
			return m_numLinks;
		}

		int getFirstLink() const
		{
			return m_firstLink;
		}

		btSoftBody* getSoftBody()
		{
			return m_softBody;
		}

		const btSoftBody* const getSoftBody() const
		{
			return m_softBody;
		}
	};
	
	btSoftBodyLinkData m_linkData;
	btSoftBodyVertexData m_vertexData;
	btSoftBodyTriangleData m_triangleData;

protected:



		
	/** Variable to define whether we need to update solver constants on the next iteration */
	bool m_updateSolverConstants;

	/** 
	 * Cloths owned by this solver.
	 * Only our cloths are in this array.
	 */
	btAlignedObjectArray< btAcceleratedSoftBodyInterface * > m_softBodySet;
	
	/** Acceleration value to be applied to all non-static vertices in the solver. 
	 * Index n is cloth n, array sized by number of cloths in the world not the solver. 
	 */
	btAlignedObjectArray< Vectormath::Aos::Vector3 > m_perClothAcceleration;

	/** Wind velocity to be applied normal to all non-static vertices in the solver. 
	 * Index n is cloth n, array sized by number of cloths in the world not the solver. 
	 */
	btAlignedObjectArray< Vectormath::Aos::Vector3 > m_perClothWindVelocity;

	/** Velocity damping factor */
	btAlignedObjectArray< float > m_perClothDampingFactor;

	/** Velocity correction coefficient */
	btAlignedObjectArray< float > m_perClothVelocityCorrectionCoefficient;

	/** Lift parameter for wind effect on cloth. */
	btAlignedObjectArray< float > m_perClothLiftFactor;
	
	/** Drag parameter for wind effect on cloth. */
	btAlignedObjectArray< float > m_perClothDragFactor;

	/** Density of the medium in which each cloth sits */
	btAlignedObjectArray< float > m_perClothMediumDensity;

	/** 
	 * Collision shape details: pair of index of first collision shape for the cloth and number of collision objects.
	 */
	btAlignedObjectArray< CollisionObjectIndices > m_perClothCollisionObjects;

	/** 
	 * Collision shapes being passed across to the cloths in this solver.
	 */
	btAlignedObjectArray< btCPUCollisionShapeDescription > m_collisionObjectDetails;


	void prepareCollisionConstraints();

	Vectormath::Aos::Vector3 ProjectOnAxis( const Vectormath::Aos::Vector3 &v, const Vectormath::Aos::Vector3 &a );

	void ApplyClampedForce( float solverdt, const Vectormath::Aos::Vector3 &force, const Vectormath::Aos::Vector3 &vertexVelocity, float inverseMass, Vectormath::Aos::Vector3 &vertexForce );
	
	float computeTriangleArea( 
		const Vectormath::Aos::Point3 &vertex0,
		const Vectormath::Aos::Point3 &vertex1,
		const Vectormath::Aos::Point3 &vertex2 );

	void applyForces( float solverdt );
	void integrate( float solverdt );
	void updateConstants( float timeStep );
	int findSoftBodyIndex( const btSoftBody* const softBody );
	
	/** Update the bounds of the soft body objects in the solver */
	void updateBounds();


public:
	btCPUSoftBodySolver();
	
	virtual ~btCPUSoftBodySolver();

	
	virtual SolverTypes getSolverType() const
	{
		return CPU_SOLVER;
	}


	virtual btSoftBodyLinkData &getLinkData();

	virtual btSoftBodyVertexData &getVertexData();

	virtual btSoftBodyTriangleData &getTriangleData();



	btAcceleratedSoftBodyInterface *findSoftBodyInterface( const btSoftBody* const softBody );
	const btAcceleratedSoftBodyInterface * const findSoftBodyInterface( const btSoftBody* const softBody ) const;



	virtual bool checkInitialized();

	virtual void updateSoftBodies( );

	virtual void optimize( btAlignedObjectArray< btSoftBody * > &softBodies , bool forceUpdate=false);

	virtual void copyBackToSoftBodies();

	virtual void solveConstraints( float solverdt );

	virtual void predictMotion( float solverdt );

	virtual void processCollision( btSoftBody *, btCollisionObject* );

	virtual void processCollision( btSoftBody*, btSoftBody *);

};


/** 
 * Class to manage movement of data from a solver to a given target.
 * This version is the CPU to CPU generic version.
 */
class btSoftBodySolverOutputCPUtoCPU : public btSoftBodySolverOutput
{
protected:

public:
	btSoftBodySolverOutputCPUtoCPU()
	{
	}

	/** Output current computed vertex data to the vertex buffers for all cloths in the solver. */
	virtual void copySoftBodyToVertexBuffer( const btSoftBody * const softBody, btVertexBufferDescriptor *vertexBuffer );
};

#endif // #ifndef BT_ACCELERATED_SOFT_BODY_CPU_SOLVER_H