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

#ifndef BT_SOFT_BODY_SOLVER_DATA_H
#define BT_SOFT_BODY_SOLVER_DATA_H

#include "BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h"
#include "vectormath/vmInclude.h"


class btSoftBodyLinkData
{
public:
	/**
	 * Class representing a link as a set of three indices into the vertex array.
	 */
	class LinkNodePair
	{
	public:
		int vertex0;
		int vertex1;

		LinkNodePair()
		{
			vertex0 = 0;
			vertex1 = 0;
		}

		LinkNodePair( int v0, int v1 )
		{
			vertex0 = v0;
			vertex1 = v1;
		}
	};

	/**
	 * Class describing a link for input into the system.
	 */
	class LinkDescription
	{
	protected:
		int m_vertex0;
		int m_vertex1;
		float m_linkLinearStiffness;
		float m_linkStrength;

	public:

		LinkDescription()
		{
			m_vertex0 = 0;
			m_vertex1 = 0;
			m_linkLinearStiffness = 1.0;
			m_linkStrength = 1.0;
		}

		LinkDescription( int newVertex0, int newVertex1, float linkLinearStiffness )
		{
			m_vertex0 = newVertex0;
			m_vertex1 = newVertex1;
			m_linkLinearStiffness = linkLinearStiffness;
			m_linkStrength = 1.0;
		}

		LinkNodePair getVertexPair() const
		{
			LinkNodePair nodes;
			nodes.vertex0 = m_vertex0;
			nodes.vertex1 = m_vertex1;
			return nodes;
		}

		void setVertex0( int vertex )
		{
			m_vertex0 = vertex;
		}

		void setVertex1( int vertex )
		{
			m_vertex1 = vertex;
		}

		void setLinkLinearStiffness( float linearStiffness )
		{
			m_linkLinearStiffness = linearStiffness;
		}

		void setLinkStrength( float strength )
		{
			m_linkStrength = strength;
		}

		int getVertex0() const
		{
			return m_vertex0;
		}

		int getVertex1() const
		{
			return m_vertex1;
		}

		float getLinkStrength() const
		{
			return m_linkStrength;
		}

		float getLinkLinearStiffness() const
		{
			return m_linkLinearStiffness;
		}
	};


protected:
	// NOTE:
	// Vertex reference data is stored relative to global array, not relative to individual cloth.
	// Values must be correct if being passed into single-cloth VBOs or when migrating from one solver
	// to another.

	btAlignedObjectArray< LinkNodePair > m_links; // Vertex pair for the link
	btAlignedObjectArray< float >								m_linkStrength; // Strength of each link
	// (inverseMassA + inverseMassB)/ linear stiffness coefficient
	btAlignedObjectArray< float >								m_linksMassLSC; 
	btAlignedObjectArray< float >								m_linksRestLengthSquared; 
	// Current vector length of link
	btAlignedObjectArray< Vectormath::Aos::Vector3 >			m_linksCLength;
	// 1/(current length * current length * massLSC)
	btAlignedObjectArray< float >								m_linksLengthRatio; 
	btAlignedObjectArray< float >								m_linksRestLength;
	btAlignedObjectArray< float >								m_linksMaterialLinearStiffnessCoefficient;

public:
	btSoftBodyLinkData()
	{
	}

	virtual ~btSoftBodyLinkData()
	{
	}

	virtual void clear()
	{
		m_links.resize(0);
		m_linkStrength.resize(0);
		m_linksMassLSC.resize(0);
		m_linksRestLengthSquared.resize(0);
		m_linksLengthRatio.resize(0);
		m_linksRestLength.resize(0);
		m_linksMaterialLinearStiffnessCoefficient.resize(0);
	}

	int getNumLinks()
	{
		return m_links.size();
	}

	/** Allocate enough space in all link-related arrays to fit numLinks links */
	virtual void createLinks( int numLinks )
	{
		int previousSize = m_links.size();
		int newSize = previousSize + numLinks;

		// Resize all the arrays that store link data
		m_links.resize( newSize );
		m_linkStrength.resize( newSize );
		m_linksMassLSC.resize( newSize );
		m_linksRestLengthSquared.resize( newSize );
		m_linksCLength.resize( newSize );
		m_linksLengthRatio.resize( newSize );
		m_linksRestLength.resize( newSize );
		m_linksMaterialLinearStiffnessCoefficient.resize( newSize );
	}
	
	/** Insert the link described into the correct data structures assuming space has already been allocated by a call to createLinks */
	virtual void setLinkAt( const LinkDescription &link, int linkIndex )
	{
		m_links[linkIndex] = link.getVertexPair();
		m_linkStrength[linkIndex] = link.getLinkStrength();
		m_linksMassLSC[linkIndex] = 0.f;
		m_linksRestLengthSquared[linkIndex] = 0.f;
		m_linksCLength[linkIndex] = Vectormath::Aos::Vector3(0.f, 0.f, 0.f);
		m_linksLengthRatio[linkIndex] = 0.f;
		m_linksRestLength[linkIndex] = 0.f;
		m_linksMaterialLinearStiffnessCoefficient[linkIndex] = link.getLinkLinearStiffness();
	}


	/**
	 * Return true if data is on the accelerator.
	 * The CPU version of this class will return true here because
	 * the CPU is the same as the accelerator.
	 */
	virtual bool onAccelerator()
	{
		return true;
	}
	
	/**
	 * Move data from host memory to the accelerator.
	 * The CPU version will always return that it has moved it.
	 */
	virtual bool moveToAccelerator()
	{
		return true;
	}

	/**
	 * Move data from host memory from the accelerator.
	 * The CPU version will always return that it has moved it.
	 */
	virtual bool moveFromAccelerator()
	{
		return true;
	}



	/**
	 * Return reference to the vertex index pair for link linkIndex as stored on the host.
	 */
	LinkNodePair &getVertexPair( int linkIndex )
	{
		return m_links[linkIndex];
	}

	/** 
	 * Return reference to strength of link linkIndex as stored on the host.
	 */
	float &getStrength( int linkIndex )
	{
		return m_linkStrength[linkIndex];
	}

	/**
	 * Return a reference to the strength of the link corrected for link sorting.
	 * This is important if we are using data on an accelerator which has the data sorted in some fashion.
	 */
	virtual float &getStrengthCorrected( int linkIndex )
	{
		return getStrength( linkIndex );
	}

	/**
	 * Return reference to the rest length of link linkIndex as stored on the host.
	 */
	float &getRestLength( int linkIndex )
	{
		return m_linksRestLength[linkIndex];
	}

	/**
	 * Return reference to linear stiffness coefficient for link linkIndex as stored on the host.
	 */
	float &getLinearStiffnessCoefficient( int linkIndex )
	{
		return m_linksMaterialLinearStiffnessCoefficient[linkIndex];
	}

	/**
	 * Return reference to the MassLSC value for link linkIndex as stored on the host.
	 */
	float &getMassLSC( int linkIndex )
	{
		return m_linksMassLSC[linkIndex];
	}

	/**
	 * Return reference to rest length squared for link linkIndex as stored on the host.
	 */
	float &getRestLengthSquared( int linkIndex )
	{
		return m_linksRestLengthSquared[linkIndex];
	}

	/**
	 * Return reference to current length of link linkIndex as stored on the host.
	 */
	Vectormath::Aos::Vector3 &getCurrentLength( int linkIndex )
	{
		return m_linksCLength[linkIndex];
	}

	 /**
	  * Return the link length ratio from for link linkIndex as stored on the host.
	  */
	 float &getLinkLengthRatio( int linkIndex )
	 {
		 return m_linksLengthRatio[linkIndex];
	 }
};



/**
 * Wrapper for vertex data information.
 * By wrapping it like this we stand a good chance of being able to optimise for storage format easily.
 * It should also help us make sure all the data structures remain consistent.
 */
class btSoftBodyVertexData
{
public:
	/**
	 * Class describing a vertex for input into the system.
	 */
	class VertexDescription
	{
	private:
		Vectormath::Aos::Point3 m_position;
		/** Inverse mass. If this is 0f then the mass was 0 because that simplifies calculations. */
		float m_inverseMass;

	public:
		VertexDescription()
		{	
			m_position = Vectormath::Aos::Point3( 0.f, 0.f, 0.f );
			m_inverseMass = 0.f;
		}

		VertexDescription( const Vectormath::Aos::Point3 &position, float mass )
		{
			m_position = position;
			if( mass > 0.f )
				m_inverseMass = 1.0f/mass;
			else
				m_inverseMass = 0.f;
		}

		void setPosition( const Vectormath::Aos::Point3 &position )
		{
			m_position = position;
		}

		void setInverseMass( float inverseMass )
		{
			m_inverseMass = inverseMass;
		}

		void setMass( float mass )
		{
			if( mass > 0.f )
				m_inverseMass = 1.0f/mass;
			else
				m_inverseMass = 0.f;
		}

		Vectormath::Aos::Point3 getPosition() const
		{
			return m_position;
		}

		float getInverseMass() const
		{
			return m_inverseMass;
		}

		float getMass() const
		{
			if( m_inverseMass == 0.f )
				return 0.f;
			else
				return 1.0f/m_inverseMass;
		}
	};
protected:

	// identifier for the individual cloth
	// For the CPU we don't really need this as we can grab the cloths and iterate over only their vertices
	// For a parallel accelerator knowing on a per-vertex basis which cloth we're part of will help for obtaining
	// per-cloth data
	// For sorting etc it might also be helpful to be able to use in-array data such as this.
	btAlignedObjectArray< int >							m_clothIdentifier;
	btAlignedObjectArray< Vectormath::Aos::Point3 >		m_vertexPosition;			// vertex positions
	btAlignedObjectArray< Vectormath::Aos::Point3 >		m_vertexPreviousPosition;	// vertex positions
	btAlignedObjectArray< Vectormath::Aos::Vector3 >	m_vertexVelocity;			// Velocity
	btAlignedObjectArray< Vectormath::Aos::Vector3 >	m_vertexForceAccumulator;	// Force accumulator
	btAlignedObjectArray< Vectormath::Aos::Vector3 >	m_vertexNormal;				// Normals
	btAlignedObjectArray< float >						m_vertexInverseMass;		// Inverse mass
	btAlignedObjectArray< float >						m_vertexArea;				// Area controlled by the vertex
	btAlignedObjectArray< int >							m_vertexTriangleCount;		// Number of triangles touching this vertex

public:
	btSoftBodyVertexData()
	{
	}

	virtual ~btSoftBodyVertexData()
	{
	}

	virtual void clear()
	{
		m_clothIdentifier.resize(0);
		m_vertexPosition.resize(0);
		m_vertexPreviousPosition.resize(0);
		m_vertexVelocity.resize(0);
		m_vertexForceAccumulator.resize(0);
		m_vertexNormal.resize(0);
		m_vertexInverseMass.resize(0);
		m_vertexArea.resize(0);
		m_vertexTriangleCount.resize(0);
	}

	int getNumVertices()
	{
		return m_vertexPosition.size();
	}

	int getClothIdentifier( int vertexIndex )
	{
		return m_clothIdentifier[vertexIndex];
	}

	void setVertexAt( const VertexDescription &vertex, int vertexIndex )
	{
		m_vertexPosition[vertexIndex] = vertex.getPosition();
		m_vertexPreviousPosition[vertexIndex] = vertex.getPosition();
		m_vertexVelocity[vertexIndex] = Vectormath::Aos::Vector3(0.f, 0.f, 0.f);
		m_vertexForceAccumulator[vertexIndex] = Vectormath::Aos::Vector3(0.f, 0.f, 0.f);
		m_vertexNormal[vertexIndex] = Vectormath::Aos::Vector3(0.f, 0.f, 0.f);
		m_vertexInverseMass[vertexIndex] = vertex.getInverseMass();
		m_vertexArea[vertexIndex] = 0.f;
		m_vertexTriangleCount[vertexIndex] = 0;
	}

	/** 
	 * Create numVertices new vertices for cloth clothIdentifier 
	 * maxVertices allows a buffer zone of extra vertices for alignment or tearing reasons.
	 */
	void createVertices( int numVertices, int clothIdentifier, int maxVertices = 0 )
	{
		int previousSize = m_vertexPosition.size();
		if( maxVertices == 0 )
			maxVertices = numVertices;
		int newSize = previousSize + maxVertices;

		// Resize all the arrays that store vertex data
		m_clothIdentifier.resize( newSize );
		m_vertexPosition.resize( newSize );
		m_vertexPreviousPosition.resize( newSize );
		m_vertexVelocity.resize( newSize );
		m_vertexForceAccumulator.resize( newSize );
		m_vertexNormal.resize( newSize );
		m_vertexInverseMass.resize( newSize );
		m_vertexArea.resize( newSize );
		m_vertexTriangleCount.resize( newSize );

		for( int vertexIndex = previousSize; vertexIndex < newSize; ++vertexIndex )
			m_clothIdentifier[vertexIndex] = clothIdentifier;
		for( int vertexIndex = (previousSize + numVertices); vertexIndex < newSize; ++vertexIndex )
			m_clothIdentifier[vertexIndex] = -1;
	}

	// Get and set methods in header so they can be inlined

	/**
	 * Return a reference to the position of vertex vertexIndex as stored on the host.
	 */
	Vectormath::Aos::Point3 &getPosition( int vertexIndex )
	{
		return m_vertexPosition[vertexIndex];
	}

	Vectormath::Aos::Point3 getPosition( int vertexIndex ) const
	{
		return m_vertexPosition[vertexIndex];
	}

	/**
	 * Return a reference to the previous position of vertex vertexIndex as stored on the host.
	 */
	Vectormath::Aos::Point3 &getPreviousPosition( int vertexIndex )
	{
		return m_vertexPreviousPosition[vertexIndex];
	}

	/**
	 * Return a reference to the velocity of vertex vertexIndex as stored on the host.
	 */
	Vectormath::Aos::Vector3 &getVelocity( int vertexIndex )
	{
		return m_vertexVelocity[vertexIndex];
	}

	/**
	 * Return a reference to the force accumulator of vertex vertexIndex as stored on the host.
	 */
	Vectormath::Aos::Vector3 &getForceAccumulator( int vertexIndex )
	{
		return m_vertexForceAccumulator[vertexIndex];
	}

	/**
	 * Return a reference to the normal of vertex vertexIndex as stored on the host.
	 */
	Vectormath::Aos::Vector3 &getNormal( int vertexIndex )
	{
		return m_vertexNormal[vertexIndex];
	}

	Vectormath::Aos::Vector3 getNormal( int vertexIndex ) const
	{
		return m_vertexNormal[vertexIndex];
	}

	/**
	 * Return a reference to the inverse mass of vertex vertexIndex as stored on the host.
	 */
	float &getInverseMass( int vertexIndex )
	{
		return m_vertexInverseMass[vertexIndex];
	}

	/**
	 * Get access to the area controlled by this vertex.
	 */
	float &getArea( int vertexIndex )
	{
		return m_vertexArea[vertexIndex];
	}

	/**
	 * Get access to the array of how many triangles touch each vertex.
	 */
	int &getTriangleCount( int vertexIndex )
	{
		return m_vertexTriangleCount[vertexIndex];
	}



	/**
	 * Return true if data is on the accelerator.
	 * The CPU version of this class will return true here because
	 * the CPU is the same as the accelerator.
	 */
	virtual bool onAccelerator()
	{
		return true;
	}
	
	/**
	 * Move data from host memory to the accelerator.
	 * The CPU version will always return that it has moved it.
	 */
	virtual bool moveToAccelerator()
	{
		return true;
	}

	/**
	 * Move data from host memory from the accelerator.
	 * The CPU version will always return that it has moved it.
	 */
	virtual bool moveFromAccelerator()
	{
		return true;
	}

	btAlignedObjectArray< Vectormath::Aos::Point3 >	&getVertexPositions()
	{
		return m_vertexPosition;
	}
};


class btSoftBodyTriangleData
{
public:
	/**
	 * Class representing a triangle as a set of three indices into the
	 * vertex array.
	 */
	class TriangleNodeSet
	{
	public:
		int vertex0;
		int vertex1;
		int vertex2;
		int _padding;

		TriangleNodeSet( )
		{
			vertex0 = 0;
			vertex1 = 0;
			vertex2 = 0;
			_padding = -1;
		}

		TriangleNodeSet( int newVertex0, int newVertex1, int newVertex2 )
		{
			vertex0 = newVertex0;
			vertex1 = newVertex1;
			vertex2 = newVertex2;
		}
	};

	class TriangleDescription
	{
	protected:
		int m_vertex0;
		int m_vertex1;
		int m_vertex2;

	public:
		TriangleDescription()
		{
			m_vertex0 = 0;
			m_vertex1 = 0;
			m_vertex2 = 0;
		}

		TriangleDescription( int newVertex0, int newVertex1, int newVertex2 )
		{
			m_vertex0 = newVertex0;
			m_vertex1 = newVertex1;
			m_vertex2 = newVertex2;
		}

		TriangleNodeSet getVertexSet() const
		{
			btSoftBodyTriangleData::TriangleNodeSet nodes;
			nodes.vertex0 = m_vertex0;
			nodes.vertex1 = m_vertex1;
			nodes.vertex2 = m_vertex2;
			return nodes;
		}
	};

protected:
	// NOTE:
	// Vertex reference data is stored relative to global array, not relative to individual cloth.
	// Values must be correct if being passed into single-cloth VBOs or when migrating from one solver
	// to another.
	btAlignedObjectArray< TriangleNodeSet > m_vertexIndices;
	btAlignedObjectArray< float > m_area;
	btAlignedObjectArray< Vectormath::Aos::Vector3 > m_normal;

public:
	btSoftBodyTriangleData()
	{
	}

	virtual ~btSoftBodyTriangleData()
	{

	}

	virtual void clear()
	{
		m_vertexIndices.resize(0);
		m_area.resize(0);
		m_normal.resize(0);
	}

	int getNumTriangles()
	{
		return m_vertexIndices.size();
	}

	virtual void setTriangleAt( const TriangleDescription &triangle, int triangleIndex )
	{
		m_vertexIndices[triangleIndex] = triangle.getVertexSet();
	}

	virtual void createTriangles( int numTriangles )		
	{
		int previousSize = m_vertexIndices.size();
		int newSize = previousSize + numTriangles;

		// Resize all the arrays that store triangle data
		m_vertexIndices.resize( newSize );
		m_area.resize( newSize );
		m_normal.resize( newSize );
	}

	/**
	 * Return the vertex index set for triangle triangleIndex as stored on the host.
	 */
	const TriangleNodeSet &getVertexSet( int triangleIndex )
	{
		return m_vertexIndices[triangleIndex];
	}

	/**
	 * Get access to the triangle area.
	 */
	float &getTriangleArea( int triangleIndex )
	{
		return m_area[triangleIndex];
	}

	/**
	 * Get access to the normal vector for this triangle.
	 */
	Vectormath::Aos::Vector3 &getNormal( int triangleIndex )
	{
		return m_normal[triangleIndex];
	}

	/**
	 * Return true if data is on the accelerator.
	 * The CPU version of this class will return true here because
	 * the CPU is the same as the accelerator.
	 */
	virtual bool onAccelerator()
	{
		return true;
	}
	
	/**
	 * Move data from host memory to the accelerator.
	 * The CPU version will always return that it has moved it.
	 */
	virtual bool moveToAccelerator()
	{
		return true;
	}

	/**
	 * Move data from host memory from the accelerator.
	 * The CPU version will always return that it has moved it.
	 */
	virtual bool moveFromAccelerator()
	{
		return true;
	}
};


#endif // #ifndef BT_SOFT_BODY_SOLVER_DATA_H

