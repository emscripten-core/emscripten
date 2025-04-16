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

#ifndef BT_ACCELERATED_SOFT_BODY_DX11_SOLVER_H
#define BT_ACCELERATED_SOFT_BODY_DX11_SOLVER_H


#include "vectormath/vmInclude.h"
#include "BulletSoftBody/btSoftBodySolvers.h"
#include "btSoftBodySolverVertexBuffer_DX11.h"
#include "btSoftBodySolverLinkData_DX11.h"
#include "btSoftBodySolverVertexData_DX11.h"
#include "btSoftBodySolverTriangleData_DX11.h"



class DXFunctions
{
public:
	
	typedef HRESULT (WINAPI * CompileFromMemoryFunc)(LPCSTR,SIZE_T,LPCSTR,const D3D10_SHADER_MACRO*,LPD3D10INCLUDE,LPCSTR,LPCSTR,UINT,UINT,ID3DX11ThreadPump*,ID3D10Blob**,ID3D10Blob**,HRESULT*);

	ID3D11Device *		 m_dx11Device;
	ID3D11DeviceContext* m_dx11Context;
	CompileFromMemoryFunc m_dx11CompileFromMemory;

	DXFunctions(ID3D11Device *dx11Device, ID3D11DeviceContext* dx11Context, CompileFromMemoryFunc dx11CompileFromMemory) :
		m_dx11Device( dx11Device ),
		m_dx11Context( dx11Context ),
		m_dx11CompileFromMemory( dx11CompileFromMemory )
	{

	}

	class KernelDesc
	{
	protected:
		

	public:
		ID3D11ComputeShader* kernel;
		ID3D11Buffer* constBuffer;

		KernelDesc()
		{
			kernel = 0;
			constBuffer = 0;
		}

		virtual ~KernelDesc()
		{
			// TODO: this should probably destroy its kernel but we need to be careful
			// in case KernelDescs are copied
		}
	}; 

	/**
	 * Compile a compute shader kernel from a string and return the appropriate KernelDesc object.
	 */
	KernelDesc compileComputeShaderFromString( const char* shaderString, const char* shaderName, int constBufferSize, D3D10_SHADER_MACRO *compileMacros = 0 );

};

class btDX11SoftBodySolver : public btSoftBodySolver
{
protected:
	/**
	 * Entry in the collision shape array.
	 * Specifies the shape type, the transform matrix and the necessary details of the collisionShape.
	 */
	struct CollisionShapeDescription
	{
		Vectormath::Aos::Transform3 shapeTransform;
		Vectormath::Aos::Vector3 linearVelocity;
		Vectormath::Aos::Vector3 angularVelocity;

		int softBodyIdentifier;
		int collisionShapeType;
	
		// Both needed for capsule
		float radius;
		float halfHeight;
		
		float margin;
		float friction;

		CollisionShapeDescription()
		{
			collisionShapeType = 0;
			margin = 0;
			friction = 0;
		}
	};

	struct UIntVector3
	{
		UIntVector3()
		{
			x = 0;
			y = 0;
			z = 0;
			_padding = 0;
		}
		
		UIntVector3( unsigned int x_, unsigned int y_, unsigned int z_ )
		{
			x = x_;
			y = y_;
			z = z_;
			_padding = 0;
		}
			
		unsigned int x;
		unsigned int y;
		unsigned int z;
		unsigned int _padding;
	};



public:
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

		int getMaxLinks()
		{
			return m_maxLinks;
		}

		int getNumLinks()
		{
			return m_numLinks;
		}

		int getFirstLink()
		{
			return m_firstLink;
		}

		btSoftBody* getSoftBody()
		{
			return m_softBody;
		}

	};

	
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





	struct PrepareLinksCB
	{		
		int numLinks;
		int padding0;
		int padding1;
		int padding2;
	};

	struct SolvePositionsFromLinksKernelCB
	{		
		int startLink;
		int numLinks;
		float kst;
		float ti;
	};

	struct IntegrateCB
	{
		int numNodes;
		float solverdt;
		int padding1;
		int padding2;
	};

	struct UpdatePositionsFromVelocitiesCB
	{
		int numNodes;
		float solverSDT;
		int padding1;
		int padding2;
	};

	struct UpdateVelocitiesFromPositionsWithoutVelocitiesCB
	{
		int numNodes;
		float isolverdt;
		int padding1;
		int padding2;
	};

	struct UpdateVelocitiesFromPositionsWithVelocitiesCB
	{
		int numNodes;
		float isolverdt;
		int padding1;
		int padding2;
	};

	struct UpdateSoftBodiesCB
	{
		int numNodes;
		int startFace;
		int numFaces;
		float epsilon;
	};


	struct ApplyForcesCB
	{
		unsigned int numNodes;
		float solverdt;
		float epsilon;
		int padding3;
	};

	struct AddVelocityCB
	{
		int startNode;
		int lastNode;
		float velocityX;
		float velocityY;
		float velocityZ;
		int padding1;
		int padding2;
		int padding3;
	};

	struct VSolveLinksCB
	{
		int startLink;
		int numLinks;
		float kst;
		int padding;
	};

	struct ComputeBoundsCB
	{
		int numNodes;
		int numSoftBodies;
		int padding1;
		int padding2;
	};

	struct SolveCollisionsAndUpdateVelocitiesCB
	{
		unsigned int numNodes;
		float isolverdt;
		int padding0;
		int padding1;
	};

	


protected:
	ID3D11Device *		 m_dx11Device;
	ID3D11DeviceContext* m_dx11Context;
	
	DXFunctions dxFunctions;
public:
	/** Link data for all cloths. Note that this will be sorted batch-wise for efficient computation and m_linkAddresses will maintain the addressing. */
	btSoftBodyLinkDataDX11 m_linkData;
	btSoftBodyVertexDataDX11 m_vertexData;
	btSoftBodyTriangleDataDX11 m_triangleData;

protected:

	/** Variable to define whether we need to update solver constants on the next iteration */
	bool m_updateSolverConstants;

	bool m_shadersInitialized;

	/** 
	 * Cloths owned by this solver.
	 * Only our cloths are in this array.
	 */
	btAlignedObjectArray< btAcceleratedSoftBodyInterface * > m_softBodySet;

	/** Acceleration value to be applied to all non-static vertices in the solver. 
	 * Index n is cloth n, array sized by number of cloths in the world not the solver. 
	 */
	btAlignedObjectArray< Vectormath::Aos::Vector3 >	m_perClothAcceleration;
	btDX11Buffer<Vectormath::Aos::Vector3>				m_dx11PerClothAcceleration;

	/** Wind velocity to be applied normal to all non-static vertices in the solver. 
	 * Index n is cloth n, array sized by number of cloths in the world not the solver. 
	 */
	btAlignedObjectArray< Vectormath::Aos::Vector3 >	m_perClothWindVelocity;
	btDX11Buffer<Vectormath::Aos::Vector3>				m_dx11PerClothWindVelocity;

	/** Velocity damping factor */
	btAlignedObjectArray< float >						m_perClothDampingFactor;
	btDX11Buffer<float>									m_dx11PerClothDampingFactor;

	/** Velocity correction coefficient */
	btAlignedObjectArray< float >						m_perClothVelocityCorrectionCoefficient;
	btDX11Buffer<float>									m_dx11PerClothVelocityCorrectionCoefficient;

	/** Lift parameter for wind effect on cloth. */
	btAlignedObjectArray< float >						m_perClothLiftFactor;
	btDX11Buffer<float>									m_dx11PerClothLiftFactor;
	
	/** Drag parameter for wind effect on cloth. */
	btAlignedObjectArray< float >						m_perClothDragFactor;
	btDX11Buffer<float>									m_dx11PerClothDragFactor;

	/** Density of the medium in which each cloth sits */
	btAlignedObjectArray< float >						m_perClothMediumDensity;
	btDX11Buffer<float>									m_dx11PerClothMediumDensity;

	
	/** 
	 * Collision shape details: pair of index of first collision shape for the cloth and number of collision objects.
	 */
	btAlignedObjectArray< CollisionObjectIndices >		m_perClothCollisionObjects;
	btDX11Buffer<CollisionObjectIndices>				m_dx11PerClothCollisionObjects;

	/** 
	 * Collision shapes being passed across to the cloths in this solver.
	 */
	btAlignedObjectArray< CollisionShapeDescription >	m_collisionObjectDetails;
	btDX11Buffer< CollisionShapeDescription >			m_dx11CollisionObjectDetails;

	/** 
	 * Minimum bounds for each cloth.
	 * Updated by GPU and returned for use by broad phase.
	 * These are int vectors as a reminder that they store the int representation of a float, not a float.
	 * Bit 31 is inverted - is floats are stored with int-sortable values.
	 */
	btAlignedObjectArray< UIntVector3 >	m_perClothMinBounds;
	btDX11Buffer< UIntVector3 >			m_dx11PerClothMinBounds;

	/** 
	 * Maximum bounds for each cloth.
	 * Updated by GPU and returned for use by broad phase.
	 * These are int vectors as a reminder that they store the int representation of a float, not a float.
	 * Bit 31 is inverted - is floats are stored with int-sortable values.
	 */
	btAlignedObjectArray< UIntVector3 >	m_perClothMaxBounds;
	btDX11Buffer< UIntVector3 >			m_dx11PerClothMaxBounds;

	
	/** 
	 * Friction coefficient for each cloth
	 */
	btAlignedObjectArray< float >	m_perClothFriction;
	btDX11Buffer< float >			m_dx11PerClothFriction;

	DXFunctions::KernelDesc		prepareLinksKernel;
	DXFunctions::KernelDesc		solvePositionsFromLinksKernel;
	DXFunctions::KernelDesc		vSolveLinksKernel;
	DXFunctions::KernelDesc		integrateKernel;
	DXFunctions::KernelDesc		addVelocityKernel;
	DXFunctions::KernelDesc		updatePositionsFromVelocitiesKernel;
	DXFunctions::KernelDesc		updateVelocitiesFromPositionsWithoutVelocitiesKernel;
	DXFunctions::KernelDesc		updateVelocitiesFromPositionsWithVelocitiesKernel;
	DXFunctions::KernelDesc		solveCollisionsAndUpdateVelocitiesKernel;
	DXFunctions::KernelDesc		resetNormalsAndAreasKernel;
	DXFunctions::KernelDesc		normalizeNormalsAndAreasKernel;
	DXFunctions::KernelDesc		computeBoundsKernel;
	DXFunctions::KernelDesc		updateSoftBodiesKernel;

	DXFunctions::KernelDesc		applyForcesKernel;


	/**
	 * Integrate motion on the solver.
	 */
	virtual void integrate( float solverdt );
	float computeTriangleArea( 
		const Vectormath::Aos::Point3 &vertex0,
		const Vectormath::Aos::Point3 &vertex1,
		const Vectormath::Aos::Point3 &vertex2 );


	virtual bool buildShaders();

	void resetNormalsAndAreas( int numVertices );

	void normalizeNormalsAndAreas( int numVertices );

	void executeUpdateSoftBodies( int firstTriangle, int numTriangles );

	void prepareCollisionConstraints();

	Vectormath::Aos::Vector3 ProjectOnAxis( const Vectormath::Aos::Vector3 &v, const Vectormath::Aos::Vector3 &a );

	void ApplyClampedForce( float solverdt, const Vectormath::Aos::Vector3 &force, const Vectormath::Aos::Vector3 &vertexVelocity, float inverseMass, Vectormath::Aos::Vector3 &vertexForce );

	virtual void applyForces( float solverdt );
	
	virtual void updateConstants( float timeStep );
	int findSoftBodyIndex( const btSoftBody* const softBody );

	//////////////////////////////////////
	// Kernel dispatches
	virtual void prepareLinks();

	void updatePositionsFromVelocities( float solverdt );
	void solveLinksForPosition( int startLink, int numLinks, float kst, float ti );
	void solveLinksForVelocity( int startLink, int numLinks, float kst );
	
	void updateVelocitiesFromPositionsWithVelocities( float isolverdt );
	void updateVelocitiesFromPositionsWithoutVelocities( float isolverdt );
	void computeBounds( );
	void solveCollisionsAndUpdateVelocities( float isolverdt );

	// End kernel dispatches
	/////////////////////////////////////

	void updateBounds();

	
	void releaseKernels();

public:
	btDX11SoftBodySolver(ID3D11Device * dx11Device, ID3D11DeviceContext* dx11Context, DXFunctions::CompileFromMemoryFunc dx11CompileFromMemory = &D3DX11CompileFromMemory);

	virtual ~btDX11SoftBodySolver();
	
	
	virtual SolverTypes getSolverType() const
	{
		return DX_SOLVER;
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

	virtual void processCollision( btSoftBody*, btSoftBody* );

};



/** 
 * Class to manage movement of data from a solver to a given target.
 * This version is the DX to CPU version.
 */
class btSoftBodySolverOutputDXtoCPU : public btSoftBodySolverOutput
{
protected:

public:
	btSoftBodySolverOutputDXtoCPU()
	{
	}

	/** Output current computed vertex data to the vertex buffers for all cloths in the solver. */
	virtual void copySoftBodyToVertexBuffer( const btSoftBody * const softBody, btVertexBufferDescriptor *vertexBuffer );
};

/** 
 * Class to manage movement of data from a solver to a given target.
 * This version is the DX to DX version and subclasses DX to CPU so that it works for that too.
 */
class btSoftBodySolverOutputDXtoDX : public btSoftBodySolverOutputDXtoCPU
{
protected:
	struct OutputToVertexArrayCB
	{
		int startNode;
		int numNodes;
		int positionOffset;
		int positionStride;
		
		int normalOffset;	
		int normalStride;
		int padding1;
		int padding2;
	};
	
	DXFunctions dxFunctions;
	DXFunctions::KernelDesc outputToVertexArrayWithNormalsKernel;
	DXFunctions::KernelDesc outputToVertexArrayWithoutNormalsKernel;

	
	bool m_shadersInitialized;

	bool checkInitialized();
	bool buildShaders();
	void releaseKernels();

public:
	btSoftBodySolverOutputDXtoDX(ID3D11Device *dx11Device, ID3D11DeviceContext* dx11Context, DXFunctions::CompileFromMemoryFunc dx11CompileFromMemory = &D3DX11CompileFromMemory) :
	  dxFunctions( dx11Device, dx11Context, dx11CompileFromMemory )
	{
		m_shadersInitialized = false;
	}

	~btSoftBodySolverOutputDXtoDX()
	{
		releaseKernels();
	}

	/** Output current computed vertex data to the vertex buffers for all cloths in the solver. */
	virtual void copySoftBodyToVertexBuffer( const btSoftBody * const softBody, btVertexBufferDescriptor *vertexBuffer );
};

#endif // #ifndef BT_ACCELERATED_SOFT_BODY_DX11_SOLVER_H


