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

#include "BulletCollision/CollisionShapes/btTriangleIndexVertexArray.h"
#include "vectormath/vmInclude.h"

#include "btSoftBodySolver_DX11.h"
#include "btSoftBodySolverVertexBuffer_DX11.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"

#define MSTRINGIFY(A) #A
static char* PrepareLinksHLSLString = 
#include "HLSL/PrepareLinks.hlsl"
static char* UpdatePositionsFromVelocitiesHLSLString = 
#include "HLSL/UpdatePositionsFromVelocities.hlsl"
static char* SolvePositionsHLSLString = 
#include "HLSL/SolvePositions.hlsl"
static char* UpdateNodesHLSLString = 
#include "HLSL/UpdateNodes.hlsl"
static char* UpdatePositionsHLSLString = 
#include "HLSL/UpdatePositions.hlsl"
static char* UpdateConstantsHLSLString = 
#include "HLSL/UpdateConstants.hlsl"
static char* IntegrateHLSLString = 
#include "HLSL/Integrate.hlsl"
static char* ApplyForcesHLSLString = 
#include "HLSL/ApplyForces.hlsl"
static char* UpdateNormalsHLSLString = 
#include "HLSL/UpdateNormals.hlsl"
static char* OutputToVertexArrayHLSLString = 
#include "HLSL/OutputToVertexArray.hlsl"
static char* VSolveLinksHLSLString = 
#include "HLSL/VSolveLinks.hlsl"
static char* ComputeBoundsHLSLString = 
#include "HLSL/ComputeBounds.hlsl"
static char* SolveCollisionsAndUpdateVelocitiesHLSLString =
#include "HLSL/SolveCollisionsAndUpdateVelocities.hlsl"


btSoftBodyLinkDataDX11::btSoftBodyLinkDataDX11( ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dDeviceContext ) : 
		m_dx11Links( d3dDevice, d3dDeviceContext, &m_links, false ),
		m_dx11LinkStrength( d3dDevice, d3dDeviceContext, &m_linkStrength, false ),
		m_dx11LinksMassLSC( d3dDevice, d3dDeviceContext, &m_linksMassLSC, false ),
		m_dx11LinksRestLengthSquared( d3dDevice, d3dDeviceContext, &m_linksRestLengthSquared, false ),
		m_dx11LinksCLength( d3dDevice, d3dDeviceContext, &m_linksCLength, false ),
		m_dx11LinksLengthRatio( d3dDevice, d3dDeviceContext, &m_linksLengthRatio, false ),
		m_dx11LinksRestLength( d3dDevice, d3dDeviceContext, &m_linksRestLength, false ),
		m_dx11LinksMaterialLinearStiffnessCoefficient( d3dDevice, d3dDeviceContext, &m_linksMaterialLinearStiffnessCoefficient, false )
{
	m_d3dDevice = d3dDevice;
	m_d3dDeviceContext = d3dDeviceContext;
}

btSoftBodyLinkDataDX11::~btSoftBodyLinkDataDX11()
{
}

static Vectormath::Aos::Vector3 toVector3( const btVector3 &vec )
{
	Vectormath::Aos::Vector3 outVec( vec.getX(), vec.getY(), vec.getZ() );
	return outVec;
}

void btSoftBodyLinkDataDX11::createLinks( int numLinks )
{
	int previousSize = m_links.size();
	int newSize = previousSize + numLinks;

	btSoftBodyLinkData::createLinks( numLinks );

	// Resize the link addresses array as well
	m_linkAddresses.resize( newSize );
}

void btSoftBodyLinkDataDX11::setLinkAt( const btSoftBodyLinkData::LinkDescription &link, int linkIndex )
{
	btSoftBodyLinkData::setLinkAt( link, linkIndex );

	// Set the link index correctly for initialisation
	m_linkAddresses[linkIndex] = linkIndex;
}

bool btSoftBodyLinkDataDX11::onAccelerator()
{
	return m_onGPU;
}

bool btSoftBodyLinkDataDX11::moveToAccelerator()
{
	bool success = true;
	success = success && m_dx11Links.moveToGPU();
	success = success && m_dx11LinkStrength.moveToGPU();
	success = success && m_dx11LinksMassLSC.moveToGPU();
	success = success && m_dx11LinksRestLengthSquared.moveToGPU();
	success = success && m_dx11LinksCLength.moveToGPU();
	success = success && m_dx11LinksLengthRatio.moveToGPU();
	success = success && m_dx11LinksRestLength.moveToGPU();
	success = success && m_dx11LinksMaterialLinearStiffnessCoefficient.moveToGPU();

	if( success )
		m_onGPU = true;

	return success;
}

bool btSoftBodyLinkDataDX11::moveFromAccelerator()
{
	bool success = true;
	success = success && m_dx11Links.moveFromGPU();
	success = success && m_dx11LinkStrength.moveFromGPU();
	success = success && m_dx11LinksMassLSC.moveFromGPU();
	success = success && m_dx11LinksRestLengthSquared.moveFromGPU();
	success = success && m_dx11LinksCLength.moveFromGPU();
	success = success && m_dx11LinksLengthRatio.moveFromGPU();
	success = success && m_dx11LinksRestLength.moveFromGPU();
	success = success && m_dx11LinksMaterialLinearStiffnessCoefficient.moveFromGPU();

	if( success )
		m_onGPU = false;

	return success;
}

void btSoftBodyLinkDataDX11::generateBatches()
{
	int numLinks = getNumLinks();

	// Do the graph colouring here temporarily
	btAlignedObjectArray< int > batchValues;
	batchValues.resize( numLinks, 0 );

	// Find the maximum vertex value internally for now
	int maxVertex = 0;
	for( int linkIndex = 0; linkIndex < numLinks; ++linkIndex )
	{
		int vertex0 = getVertexPair(linkIndex).vertex0;
		int vertex1 = getVertexPair(linkIndex).vertex1;
		if( vertex0 > maxVertex )
			maxVertex = vertex0;
		if( vertex1 > maxVertex )
			maxVertex = vertex1;
	}
	int numVertices = maxVertex + 1;

	// Set of lists, one for each node, specifying which colours are connected
	// to that node.
	// No two edges into a node can share a colour.
	btAlignedObjectArray< btAlignedObjectArray< int > > vertexConnectedColourLists;
	vertexConnectedColourLists.resize(numVertices);



	// Simple algorithm that chooses the lowest batch number
	// that none of the links attached to either of the connected 
	// nodes is in
	for( int linkIndex = 0; linkIndex < numLinks; ++linkIndex )
	{				
		int linkLocation = m_linkAddresses[linkIndex];

		int vertex0 = getVertexPair(linkLocation).vertex0;
		int vertex1 = getVertexPair(linkLocation).vertex1;

		// Get the two node colour lists
		btAlignedObjectArray< int > &colourListVertex0( vertexConnectedColourLists[vertex0] );
		btAlignedObjectArray< int > &colourListVertex1( vertexConnectedColourLists[vertex1] );

		// Choose the minimum colour that is in neither list
		int colour = 0;
		while( colourListVertex0.findLinearSearch(colour) != colourListVertex0.size() || colourListVertex1.findLinearSearch(colour) != colourListVertex1.size()  )
			++colour;
		// i should now be the minimum colour in neither list
		// Add to the two lists so that future edges don't share
		// And store the colour against this edge

		colourListVertex0.push_back(colour);
		colourListVertex1.push_back(colour);
		batchValues[linkIndex] = colour;
	}

	// Check the colour counts
	btAlignedObjectArray< int > batchCounts;
	for( int i = 0; i < numLinks; ++i )
	{
		int batch = batchValues[i];
		if( batch >= batchCounts.size() )
			batchCounts.push_back(1);
		else
			++(batchCounts[batch]);
	}

	m_batchStartLengths.resize(batchCounts.size());
	if( m_batchStartLengths.size() > 0 )
	{
		m_batchStartLengths[0] = BatchPair( 0, 0 );

		int sum = 0;
		for( int batchIndex = 0; batchIndex < batchCounts.size(); ++batchIndex )
		{
			m_batchStartLengths[batchIndex].start = sum;
			m_batchStartLengths[batchIndex].length = batchCounts[batchIndex];
			sum += batchCounts[batchIndex];
		}
	}

	/////////////////////////////
	// Sort data based on batches

	// Create source arrays by copying originals
	btAlignedObjectArray<btSoftBodyLinkData::LinkNodePair>				m_links_Backup(m_links);
	btAlignedObjectArray<float>											m_linkStrength_Backup(m_linkStrength);
	btAlignedObjectArray<float>											m_linksMassLSC_Backup(m_linksMassLSC);
	btAlignedObjectArray<float>											m_linksRestLengthSquared_Backup(m_linksRestLengthSquared);
	btAlignedObjectArray<Vectormath::Aos::Vector3>						m_linksCLength_Backup(m_linksCLength);
	btAlignedObjectArray<float>											m_linksLengthRatio_Backup(m_linksLengthRatio);
	btAlignedObjectArray<float>											m_linksRestLength_Backup(m_linksRestLength);
	btAlignedObjectArray<float>											m_linksMaterialLinearStiffnessCoefficient_Backup(m_linksMaterialLinearStiffnessCoefficient);


	for( int batch = 0; batch < batchCounts.size(); ++batch )
		batchCounts[batch] = 0;

	// Do sort as single pass into destination arrays	
	for( int linkIndex = 0; linkIndex < numLinks; ++linkIndex )
	{
		// To maintain locations run off the original link locations rather than the current position.
		// It's not cache efficient, but as we run this rarely that should not matter.
		// It's faster than searching the link location array for the current location and then updating it.
		// The other alternative would be to unsort before resorting, but this is equivalent to doing that.
		int linkLocation = m_linkAddresses[linkIndex];

		// Obtain batch and calculate target location for the
		// next element in that batch, incrementing the batch counter
		// afterwards
		int batch = batchValues[linkIndex];
		int newLocation = m_batchStartLengths[batch].start + batchCounts[batch];

		batchCounts[batch] = batchCounts[batch] + 1;
		m_links[newLocation] = m_links_Backup[linkLocation];
#if 1
		m_linkStrength[newLocation] = m_linkStrength_Backup[linkLocation];
		m_linksMassLSC[newLocation] = m_linksMassLSC_Backup[linkLocation];
		m_linksRestLengthSquared[newLocation] = m_linksRestLengthSquared_Backup[linkLocation];
		m_linksLengthRatio[newLocation] = m_linksLengthRatio_Backup[linkLocation];
		m_linksRestLength[newLocation] = m_linksRestLength_Backup[linkLocation];
		m_linksMaterialLinearStiffnessCoefficient[newLocation] = m_linksMaterialLinearStiffnessCoefficient_Backup[linkLocation];
#endif
		// Update the locations array to account for the moved entry
		m_linkAddresses[linkIndex] = newLocation;
	}
} // void btSoftBodyLinkDataDX11::generateBatches()



btSoftBodyVertexDataDX11::btSoftBodyVertexDataDX11( ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dDeviceContext ) : 
	m_dx11ClothIdentifier( d3dDevice, d3dDeviceContext, &m_clothIdentifier, false ),
	m_dx11VertexPosition( d3dDevice, d3dDeviceContext, &m_vertexPosition, false ),
	m_dx11VertexPreviousPosition( d3dDevice, d3dDeviceContext, &m_vertexPreviousPosition, false ),
	m_dx11VertexVelocity( d3dDevice, d3dDeviceContext, &m_vertexVelocity, false ),
	m_dx11VertexForceAccumulator( d3dDevice, d3dDeviceContext, &m_vertexForceAccumulator, false ),
	m_dx11VertexNormal( d3dDevice, d3dDeviceContext, &m_vertexNormal, false ),
	m_dx11VertexInverseMass( d3dDevice, d3dDeviceContext, &m_vertexInverseMass, false ),
	m_dx11VertexArea( d3dDevice, d3dDeviceContext, &m_vertexArea, false ),
	m_dx11VertexTriangleCount( d3dDevice, d3dDeviceContext, &m_vertexTriangleCount, false )
{
	m_d3dDevice = d3dDevice;
	m_d3dDeviceContext = d3dDeviceContext;
}

btSoftBodyVertexDataDX11::~btSoftBodyVertexDataDX11()
{

}

bool btSoftBodyVertexDataDX11::onAccelerator()
{
	return m_onGPU;
}

bool btSoftBodyVertexDataDX11::moveToAccelerator()
{
	bool success = true;
	success = success && m_dx11ClothIdentifier.moveToGPU();
	success = success && m_dx11VertexPosition.moveToGPU();
	success = success && m_dx11VertexPreviousPosition.moveToGPU();
	success = success && m_dx11VertexVelocity.moveToGPU();
	success = success && m_dx11VertexForceAccumulator.moveToGPU();
	success = success && m_dx11VertexNormal.moveToGPU();
	success = success && m_dx11VertexInverseMass.moveToGPU();
	success = success && m_dx11VertexArea.moveToGPU();
	success = success && m_dx11VertexTriangleCount.moveToGPU();

	if( success )
		m_onGPU = true;

	return success;
}

bool btSoftBodyVertexDataDX11::moveFromAccelerator()
{
	bool success = true;
	success = success && m_dx11ClothIdentifier.moveFromGPU();
	success = success && m_dx11VertexPosition.moveFromGPU();
	success = success && m_dx11VertexPreviousPosition.moveFromGPU();
	success = success && m_dx11VertexVelocity.moveFromGPU();
	success = success && m_dx11VertexForceAccumulator.moveFromGPU();
	success = success && m_dx11VertexNormal.moveFromGPU();
	success = success && m_dx11VertexInverseMass.moveFromGPU();
	success = success && m_dx11VertexArea.moveFromGPU();
	success = success && m_dx11VertexTriangleCount.moveFromGPU();

	if( success )
		m_onGPU = true;

	return success;
}


btSoftBodyTriangleDataDX11::btSoftBodyTriangleDataDX11( ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dDeviceContext ) : 
	m_dx11VertexIndices( d3dDevice, d3dDeviceContext, &m_vertexIndices, false ),
	m_dx11Area( d3dDevice, d3dDeviceContext, &m_area, false ),
	m_dx11Normal( d3dDevice, d3dDeviceContext, &m_normal, false )
{
	m_d3dDevice = d3dDevice;
	m_d3dDeviceContext = d3dDeviceContext;
}

btSoftBodyTriangleDataDX11::~btSoftBodyTriangleDataDX11()
{

}


/** Allocate enough space in all link-related arrays to fit numLinks links */
void btSoftBodyTriangleDataDX11::createTriangles( int numTriangles )
{
	int previousSize = getNumTriangles();
	int newSize = previousSize + numTriangles;

	btSoftBodyTriangleData::createTriangles( numTriangles );

	// Resize the link addresses array as well
	m_triangleAddresses.resize( newSize );
}

/** Insert the link described into the correct data structures assuming space has already been allocated by a call to createLinks */
void btSoftBodyTriangleDataDX11::setTriangleAt( const btSoftBodyTriangleData::TriangleDescription &triangle, int triangleIndex )
{
	btSoftBodyTriangleData::setTriangleAt( triangle, triangleIndex );

	m_triangleAddresses[triangleIndex] = triangleIndex;
}

bool btSoftBodyTriangleDataDX11::onAccelerator()
{
	return m_onGPU;
}

bool btSoftBodyTriangleDataDX11::moveToAccelerator()
{
	bool success = true;
	success = success && m_dx11VertexIndices.moveToGPU();
	success = success && m_dx11Area.moveToGPU();
	success = success && m_dx11Normal.moveToGPU();

	if( success )
		m_onGPU = true;

	return success;
}

bool btSoftBodyTriangleDataDX11::moveFromAccelerator()
{
	bool success = true;
	success = success && m_dx11VertexIndices.moveFromGPU();
	success = success && m_dx11Area.moveFromGPU();
	success = success && m_dx11Normal.moveFromGPU();

	if( success )
		m_onGPU = true;

	return success;
}

/**
 * Generate (and later update) the batching for the entire triangle set.
 * This redoes a lot of work because it batches the entire set when each cloth is inserted.
 * In theory we could delay it until just before we need the cloth.
 * It's a one-off overhead, though, so that is a later optimisation.
 */
void btSoftBodyTriangleDataDX11::generateBatches()
{
	int numTriangles = getNumTriangles();
	if( numTriangles == 0 )
		return;

	// Do the graph colouring here temporarily
	btAlignedObjectArray< int > batchValues;
	batchValues.resize( numTriangles );

	// Find the maximum vertex value internally for now
	int maxVertex = 0;
	for( int triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex )
	{
		int vertex0 = getVertexSet(triangleIndex).vertex0;
		int vertex1 = getVertexSet(triangleIndex).vertex1;
		int vertex2 = getVertexSet(triangleIndex).vertex2;
		
		if( vertex0 > maxVertex )
			maxVertex = vertex0;
		if( vertex1 > maxVertex )
			maxVertex = vertex1;
		if( vertex2 > maxVertex )
			maxVertex = vertex2;
	}
	int numVertices = maxVertex + 1;

	// Set of lists, one for each node, specifying which colours are connected
	// to that node.
	// No two edges into a node can share a colour.
	btAlignedObjectArray< btAlignedObjectArray< int > > vertexConnectedColourLists;
	vertexConnectedColourLists.resize(numVertices);


	//std::cout << "\n";
	// Simple algorithm that chooses the lowest batch number
	// that none of the faces attached to either of the connected 
	// nodes is in
	for( int triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex )
	{
		// To maintain locations run off the original link locations rather than the current position.
		// It's not cache efficient, but as we run this rarely that should not matter.
		// It's faster than searching the link location array for the current location and then updating it.
		// The other alternative would be to unsort before resorting, but this is equivalent to doing that.
		int triangleLocation = m_triangleAddresses[triangleIndex];

		int vertex0 = getVertexSet(triangleLocation).vertex0;
		int vertex1 = getVertexSet(triangleLocation).vertex1;
		int vertex2 = getVertexSet(triangleLocation).vertex2;

		// Get the three node colour lists
		btAlignedObjectArray< int > &colourListVertex0( vertexConnectedColourLists[vertex0] );
		btAlignedObjectArray< int > &colourListVertex1( vertexConnectedColourLists[vertex1] );
		btAlignedObjectArray< int > &colourListVertex2( vertexConnectedColourLists[vertex2] );

		// Choose the minimum colour that is in none of the lists
		int colour = 0;
		while( 
			colourListVertex0.findLinearSearch(colour) != colourListVertex0.size() || 
			colourListVertex1.findLinearSearch(colour) != colourListVertex1.size() ||
			colourListVertex2.findLinearSearch(colour) != colourListVertex2.size() )
		{
			++colour;
		}
		// i should now be the minimum colour in neither list
		// Add to the three lists so that future edges don't share
		// And store the colour against this face
		colourListVertex0.push_back(colour);
		colourListVertex1.push_back(colour);
		colourListVertex2.push_back(colour);

		batchValues[triangleIndex] = colour;
	}


	// Check the colour counts
	btAlignedObjectArray< int > batchCounts;
	for( int i = 0; i < numTriangles; ++i )
	{
		int batch = batchValues[i];
		if( batch >= batchCounts.size() )
			batchCounts.push_back(1);
		else
			++(batchCounts[batch]);
	}


	m_batchStartLengths.resize(batchCounts.size());
	m_batchStartLengths[0] = BatchPair( 0, 0 );


	int sum = 0;
	for( int batchIndex = 0; batchIndex < batchCounts.size(); ++batchIndex )
	{
		m_batchStartLengths[batchIndex].start = sum;
		m_batchStartLengths[batchIndex].length = batchCounts[batchIndex];
		sum += batchCounts[batchIndex];
	}
	
	/////////////////////////////
	// Sort data based on batches
	
	// Create source arrays by copying originals
	btAlignedObjectArray<btSoftBodyTriangleData::TriangleNodeSet>							m_vertexIndices_Backup(m_vertexIndices);
	btAlignedObjectArray<float>										m_area_Backup(m_area);
	btAlignedObjectArray<Vectormath::Aos::Vector3>					m_normal_Backup(m_normal);


	for( int batch = 0; batch < batchCounts.size(); ++batch )
		batchCounts[batch] = 0;

	// Do sort as single pass into destination arrays	
	for( int triangleIndex = 0; triangleIndex < numTriangles; ++triangleIndex )
	{
		// To maintain locations run off the original link locations rather than the current position.
		// It's not cache efficient, but as we run this rarely that should not matter.
		// It's faster than searching the link location array for the current location and then updating it.
		// The other alternative would be to unsort before resorting, but this is equivalent to doing that.
		int triangleLocation = m_triangleAddresses[triangleIndex];

		// Obtain batch and calculate target location for the
		// next element in that batch, incrementing the batch counter
		// afterwards
		int batch = batchValues[triangleIndex];
		int newLocation = m_batchStartLengths[batch].start + batchCounts[batch];

		batchCounts[batch] = batchCounts[batch] + 1;
		m_vertexIndices[newLocation] = m_vertexIndices_Backup[triangleLocation];
		m_area[newLocation] = m_area_Backup[triangleLocation];
		m_normal[newLocation] = m_normal_Backup[triangleLocation];

		// Update the locations array to account for the moved entry
		m_triangleAddresses[triangleIndex] = newLocation;
	}
} // btSoftBodyTriangleDataDX11::generateBatches












btDX11SoftBodySolver::btDX11SoftBodySolver(ID3D11Device * dx11Device, ID3D11DeviceContext* dx11Context, DXFunctions::CompileFromMemoryFunc dx11CompileFromMemory) :
	m_dx11Device( dx11Device ),
	m_dx11Context( dx11Context ),
	dxFunctions( m_dx11Device, m_dx11Context, dx11CompileFromMemory ),
	m_linkData(m_dx11Device, m_dx11Context),
	m_vertexData(m_dx11Device, m_dx11Context),
	m_triangleData(m_dx11Device, m_dx11Context),
	m_dx11PerClothAcceleration( m_dx11Device, m_dx11Context, &m_perClothAcceleration, true ),
	m_dx11PerClothWindVelocity( m_dx11Device, m_dx11Context, &m_perClothWindVelocity, true ),
	m_dx11PerClothDampingFactor( m_dx11Device, m_dx11Context, &m_perClothDampingFactor, true ),
	m_dx11PerClothVelocityCorrectionCoefficient( m_dx11Device, m_dx11Context, &m_perClothVelocityCorrectionCoefficient, true ),
	m_dx11PerClothLiftFactor( m_dx11Device, m_dx11Context, &m_perClothLiftFactor, true ),
	m_dx11PerClothDragFactor( m_dx11Device, m_dx11Context, &m_perClothDragFactor, true ),
	m_dx11PerClothMediumDensity( m_dx11Device, m_dx11Context, &m_perClothMediumDensity, true ),
	m_dx11PerClothCollisionObjects( m_dx11Device, m_dx11Context, &m_perClothCollisionObjects, true ),
	m_dx11CollisionObjectDetails( m_dx11Device, m_dx11Context, &m_collisionObjectDetails, true ),
	m_dx11PerClothMinBounds( m_dx11Device, m_dx11Context, &m_perClothMinBounds, false ),
	m_dx11PerClothMaxBounds( m_dx11Device, m_dx11Context, &m_perClothMaxBounds, false ),
	m_dx11PerClothFriction( m_dx11Device, m_dx11Context, &m_perClothFriction, false )
{
	// Initial we will clearly need to update solver constants
	// For now this is global for the cloths linked with this solver - we should probably make this body specific 
	// for performance in future once we understand more clearly when constants need to be updated
	m_updateSolverConstants = true;

	m_shadersInitialized = false;
}

btDX11SoftBodySolver::~btDX11SoftBodySolver()
{	
	releaseKernels();
}

void btDX11SoftBodySolver::releaseKernels()
{
	
	SAFE_RELEASE( prepareLinksKernel.kernel );
	SAFE_RELEASE( prepareLinksKernel.constBuffer );
	SAFE_RELEASE( integrateKernel.kernel );
	SAFE_RELEASE( integrateKernel.constBuffer );
	SAFE_RELEASE( integrateKernel.kernel );
	SAFE_RELEASE( solvePositionsFromLinksKernel.constBuffer );
	SAFE_RELEASE( solvePositionsFromLinksKernel.kernel );
	SAFE_RELEASE( updatePositionsFromVelocitiesKernel.constBuffer );
	SAFE_RELEASE( updatePositionsFromVelocitiesKernel.kernel );
	SAFE_RELEASE( updateVelocitiesFromPositionsWithoutVelocitiesKernel.constBuffer );
	SAFE_RELEASE( updateVelocitiesFromPositionsWithoutVelocitiesKernel.kernel );
	SAFE_RELEASE( updateVelocitiesFromPositionsWithVelocitiesKernel.constBuffer );
	SAFE_RELEASE( updateVelocitiesFromPositionsWithVelocitiesKernel.kernel );
	SAFE_RELEASE( resetNormalsAndAreasKernel.constBuffer );
	SAFE_RELEASE( resetNormalsAndAreasKernel.kernel );
	SAFE_RELEASE( normalizeNormalsAndAreasKernel.constBuffer );
	SAFE_RELEASE( normalizeNormalsAndAreasKernel.kernel );
	SAFE_RELEASE( updateSoftBodiesKernel.constBuffer );
	SAFE_RELEASE( updateSoftBodiesKernel.kernel );
	SAFE_RELEASE( solveCollisionsAndUpdateVelocitiesKernel.kernel );
	SAFE_RELEASE( solveCollisionsAndUpdateVelocitiesKernel.constBuffer );
	SAFE_RELEASE( computeBoundsKernel.kernel );
	SAFE_RELEASE( computeBoundsKernel.constBuffer );
	SAFE_RELEASE( vSolveLinksKernel.kernel );
	SAFE_RELEASE( vSolveLinksKernel.constBuffer );

	SAFE_RELEASE( addVelocityKernel.constBuffer );
	SAFE_RELEASE( addVelocityKernel.kernel );
	SAFE_RELEASE( applyForcesKernel.constBuffer );
	SAFE_RELEASE( applyForcesKernel.kernel );

	m_shadersInitialized = false;
}


void btDX11SoftBodySolver::copyBackToSoftBodies()
{
	// Move the vertex data back to the host first
	m_vertexData.moveFromAccelerator();

	// Loop over soft bodies, copying all the vertex positions back for each body in turn
	for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
	{
		btAcceleratedSoftBodyInterface *softBodyInterface = m_softBodySet[ softBodyIndex ];
		btSoftBody *softBody = softBodyInterface->getSoftBody();

		int firstVertex = softBodyInterface->getFirstVertex();
		int numVertices = softBodyInterface->getNumVertices();

		// Copy vertices from solver back into the softbody
		for( int vertex = 0; vertex < numVertices; ++vertex )
		{
			using Vectormath::Aos::Point3;
			Point3 vertexPosition( getVertexData().getVertexPositions()[firstVertex + vertex] );

			softBody->m_nodes[vertex].m_x.setX( vertexPosition.getX() );
			softBody->m_nodes[vertex].m_x.setY( vertexPosition.getY() );
			softBody->m_nodes[vertex].m_x.setZ( vertexPosition.getZ() );

			softBody->m_nodes[vertex].m_n.setX( vertexPosition.getX() );
			softBody->m_nodes[vertex].m_n.setY( vertexPosition.getY() );
			softBody->m_nodes[vertex].m_n.setZ( vertexPosition.getZ() );
		}
	}
} // btDX11SoftBodySolver::copyBackToSoftBodies


void btDX11SoftBodySolver::optimize( btAlignedObjectArray< btSoftBody * > &softBodies, bool forceUpdate )
{
	if( forceUpdate || m_softBodySet.size() != softBodies.size() )
	{
		// Have a change in the soft body set so update, reloading all the data
		getVertexData().clear();
		getTriangleData().clear();
		getLinkData().clear();
		m_softBodySet.resize(0);


		for( int softBodyIndex = 0; softBodyIndex < softBodies.size(); ++softBodyIndex )
		{
			btSoftBody *softBody = softBodies[ softBodyIndex ];
			using Vectormath::Aos::Matrix3;
			using Vectormath::Aos::Point3;

			// Create SoftBody that will store the information within the solver
			btAcceleratedSoftBodyInterface *newSoftBody = new btAcceleratedSoftBodyInterface( softBody );
			m_softBodySet.push_back( newSoftBody );

			m_perClothAcceleration.push_back( toVector3(softBody->getWorldInfo()->m_gravity) );
			m_perClothDampingFactor.push_back(softBody->m_cfg.kDP);
			m_perClothVelocityCorrectionCoefficient.push_back( softBody->m_cfg.kVCF );
			m_perClothLiftFactor.push_back( softBody->m_cfg.kLF );
			m_perClothDragFactor.push_back( softBody->m_cfg.kDG );
			m_perClothMediumDensity.push_back(softBody->getWorldInfo()->air_density);
			// Simple init values. Actually we'll put 0 and -1 into them at the appropriate time
			m_perClothMinBounds.push_back( UIntVector3( 0, 0, 0 ) );
			m_perClothMaxBounds.push_back( UIntVector3( UINT_MAX, UINT_MAX, UINT_MAX ) );
			m_perClothFriction.push_back( softBody->getFriction() );
			m_perClothCollisionObjects.push_back( CollisionObjectIndices(-1, -1) );

			// Add space for new vertices and triangles in the default solver for now
			// TODO: Include space here for tearing too later
			int firstVertex = getVertexData().getNumVertices();
			int numVertices = softBody->m_nodes.size();
			int maxVertices = numVertices;
			// Allocate space for new vertices in all the vertex arrays
			getVertexData().createVertices( maxVertices, softBodyIndex );

			int firstTriangle = getTriangleData().getNumTriangles();
			int numTriangles = softBody->m_faces.size();
			int maxTriangles = numTriangles;
			getTriangleData().createTriangles( maxTriangles );

			// Copy vertices from softbody into the solver
			for( int vertex = 0; vertex < numVertices; ++vertex )
			{
				Point3 multPoint(softBody->m_nodes[vertex].m_x.getX(), softBody->m_nodes[vertex].m_x.getY(), softBody->m_nodes[vertex].m_x.getZ());
				btSoftBodyVertexData::VertexDescription desc;

				// TODO: Position in the softbody might be pre-transformed
				// or we may need to adapt for the pose.
				//desc.setPosition( cloth.getMeshTransform()*multPoint );
				desc.setPosition( multPoint );

				float vertexInverseMass = softBody->m_nodes[vertex].m_im;
				desc.setInverseMass(vertexInverseMass);
				getVertexData().setVertexAt( desc, firstVertex + vertex );
			}

			// Copy triangles similarly
			// We're assuming here that vertex indices are based on the firstVertex rather than the entire scene
			for( int triangle = 0; triangle < numTriangles; ++triangle )
			{
				// Note that large array storage is relative to the array not to the cloth
				// So we need to add firstVertex to each value
				int vertexIndex0 = (softBody->m_faces[triangle].m_n[0] - &(softBody->m_nodes[0]));
				int vertexIndex1 = (softBody->m_faces[triangle].m_n[1] - &(softBody->m_nodes[0]));
				int vertexIndex2 = (softBody->m_faces[triangle].m_n[2] - &(softBody->m_nodes[0]));
				btSoftBodyTriangleData::TriangleDescription newTriangle(vertexIndex0 + firstVertex, vertexIndex1 + firstVertex, vertexIndex2 + firstVertex);
				getTriangleData().setTriangleAt( newTriangle, firstTriangle + triangle );
				
				// Increase vertex triangle counts for this triangle		
				getVertexData().getTriangleCount(newTriangle.getVertexSet().vertex0)++;
				getVertexData().getTriangleCount(newTriangle.getVertexSet().vertex1)++;
				getVertexData().getTriangleCount(newTriangle.getVertexSet().vertex2)++;
			}

			int firstLink = getLinkData().getNumLinks();
			int numLinks = softBody->m_links.size();
			int maxLinks = numLinks;
			
			// Allocate space for the links
			getLinkData().createLinks( numLinks );

			// Add the links
			for( int link = 0; link < numLinks; ++link )
			{
				int vertexIndex0 = softBody->m_links[link].m_n[0] - &(softBody->m_nodes[0]);
				int vertexIndex1 = softBody->m_links[link].m_n[1] - &(softBody->m_nodes[0]);

				btSoftBodyLinkData::LinkDescription newLink(vertexIndex0 + firstVertex, vertexIndex1 + firstVertex, softBody->m_links[link].m_material->m_kLST);
				newLink.setLinkStrength(1.f);
				getLinkData().setLinkAt(newLink, firstLink + link);
			}
			
			newSoftBody->setFirstVertex( firstVertex );
			newSoftBody->setFirstTriangle( firstTriangle );
			newSoftBody->setNumVertices( numVertices );
			newSoftBody->setMaxVertices( maxVertices );
			newSoftBody->setNumTriangles( numTriangles );
			newSoftBody->setMaxTriangles( maxTriangles );
			newSoftBody->setFirstLink( firstLink );
			newSoftBody->setNumLinks( numLinks );
		}



		updateConstants(0.f);


		m_linkData.generateBatches();		
		m_triangleData.generateBatches();
	}
}


btSoftBodyLinkData &btDX11SoftBodySolver::getLinkData()
{
	// TODO: Consider setting link data to "changed" here
	return m_linkData;
}

btSoftBodyVertexData &btDX11SoftBodySolver::getVertexData()
{
	// TODO: Consider setting vertex data to "changed" here
	return m_vertexData;
}

btSoftBodyTriangleData &btDX11SoftBodySolver::getTriangleData()
{
	// TODO: Consider setting triangle data to "changed" here
	return m_triangleData;
}

bool btDX11SoftBodySolver::checkInitialized()
{
	if( !m_shadersInitialized )
		if( buildShaders() )
			m_shadersInitialized = true;

	return m_shadersInitialized;
}

void btDX11SoftBodySolver::resetNormalsAndAreas( int numVertices )
{
	// No need to batch link solver, it is entirely parallel
	// Copy kernel parameters to GPU
	UpdateSoftBodiesCB constBuffer;
	
	constBuffer.numNodes = numVertices;
	constBuffer.epsilon = FLT_EPSILON;
	
	// Todo: factor this out. Number of nodes is static and sdt might be, too, we can update this just once on setup
	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( integrateKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(UpdateSoftBodiesCB) );	
	m_dx11Context->Unmap( integrateKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &integrateKernel.constBuffer );

	// Set resources and dispatch
	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_vertexData.m_dx11VertexNormal.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &(m_vertexData.m_dx11VertexArea.getUAV()), NULL );

	// Execute the kernel
	m_dx11Context->CSSetShader( resetNormalsAndAreasKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numNodes + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks, 1, 1 );

	{
		// Tidy up 
		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	
} // btDX11SoftBodySolver::resetNormalsAndAreas

void btDX11SoftBodySolver::normalizeNormalsAndAreas( int numVertices )
{
	// No need to batch link solver, it is entirely parallel
	// Copy kernel parameters to GPU
	UpdateSoftBodiesCB constBuffer;
	
	constBuffer.numNodes = numVertices;
	constBuffer.epsilon = FLT_EPSILON;
	
	// Todo: factor this out. Number of nodes is static and sdt might be, too, we can update this just once on setup
	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( integrateKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(UpdateSoftBodiesCB) );	
	m_dx11Context->Unmap( integrateKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &integrateKernel.constBuffer );

	// Set resources and dispatch	
	m_dx11Context->CSSetShaderResources( 2, 1, &(m_vertexData.m_dx11VertexTriangleCount.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_vertexData.m_dx11VertexNormal.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &(m_vertexData.m_dx11VertexArea.getUAV()), NULL );

	// Execute the kernel
	m_dx11Context->CSSetShader( normalizeNormalsAndAreasKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numNodes + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks, 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 2, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	
} // btDX11SoftBodySolver::normalizeNormalsAndAreas

void btDX11SoftBodySolver::executeUpdateSoftBodies( int firstTriangle, int numTriangles )
{
	// No need to batch link solver, it is entirely parallel
	// Copy kernel parameters to GPU
	UpdateSoftBodiesCB constBuffer;
	
	constBuffer.startFace = firstTriangle;
	constBuffer.numFaces = numTriangles;
	
	// Todo: factor this out. Number of nodes is static and sdt might be, too, we can update this just once on setup
	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( updateSoftBodiesKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(UpdateSoftBodiesCB) );	
	m_dx11Context->Unmap( updateSoftBodiesKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &updateSoftBodiesKernel.constBuffer );

	// Set resources and dispatch	
	m_dx11Context->CSSetShaderResources( 0, 1, &(m_triangleData.m_dx11VertexIndices.getSRV()) );
	m_dx11Context->CSSetShaderResources( 1, 1, &(m_vertexData.m_dx11VertexPosition.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_vertexData.m_dx11VertexNormal.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &(m_vertexData.m_dx11VertexArea.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 2, 1, &(m_triangleData.m_dx11Normal.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 3, 1, &(m_triangleData.m_dx11Area.getUAV()), NULL );

	// Execute the kernel
	m_dx11Context->CSSetShader( updateSoftBodiesKernel.kernel, NULL, 0 );

	int	numBlocks = (numTriangles + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks, 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 4, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	
} // btDX11SoftBodySolver::executeUpdateSoftBodies

void btDX11SoftBodySolver::updateSoftBodies()
{
	using namespace Vectormath::Aos;


	int numVertices = m_vertexData.getNumVertices();
	int numTriangles = m_triangleData.getNumTriangles();

	// Ensure data is on accelerator
	m_vertexData.moveToAccelerator();
	m_triangleData.moveToAccelerator();

	resetNormalsAndAreas( numVertices );


	// Go through triangle batches so updates occur correctly
	for( int batchIndex = 0; batchIndex < m_triangleData.m_batchStartLengths.size(); ++batchIndex )
	{

		int startTriangle = m_triangleData.m_batchStartLengths[batchIndex].start;
		int numTriangles = m_triangleData.m_batchStartLengths[batchIndex].length;

		executeUpdateSoftBodies( startTriangle, numTriangles );
	}


	normalizeNormalsAndAreas( numVertices );
	

} // btDX11SoftBodySolver::updateSoftBodies


Vectormath::Aos::Vector3 btDX11SoftBodySolver::ProjectOnAxis( const Vectormath::Aos::Vector3 &v, const Vectormath::Aos::Vector3 &a )
{
	return a*Vectormath::Aos::dot(v, a);
}

void btDX11SoftBodySolver::ApplyClampedForce( float solverdt, const Vectormath::Aos::Vector3 &force, const Vectormath::Aos::Vector3 &vertexVelocity, float inverseMass, Vectormath::Aos::Vector3 &vertexForce )
{
	float dtInverseMass = solverdt*inverseMass;
	if( Vectormath::Aos::lengthSqr(force * dtInverseMass) > Vectormath::Aos::lengthSqr(vertexVelocity) )
	{
		vertexForce -= ProjectOnAxis( vertexVelocity, normalize( force ) )/dtInverseMass;
	} else {
		vertexForce += force;
	}
}

void btDX11SoftBodySolver::applyForces( float solverdt )
{		
	using namespace Vectormath::Aos;


	// Ensure data is on accelerator
	m_vertexData.moveToAccelerator();
	m_dx11PerClothAcceleration.moveToGPU();
	m_dx11PerClothLiftFactor.moveToGPU();
	m_dx11PerClothDragFactor.moveToGPU();
	m_dx11PerClothMediumDensity.moveToGPU();
	m_dx11PerClothWindVelocity.moveToGPU();

	// No need to batch link solver, it is entirely parallel
	// Copy kernel parameters to GPU
	ApplyForcesCB constBuffer;
	
	constBuffer.numNodes = m_vertexData.getNumVertices();
	constBuffer.solverdt = solverdt;
	constBuffer.epsilon = FLT_EPSILON;
	
	// Todo: factor this out. Number of nodes is static and sdt might be, too, we can update this just once on setup
	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( integrateKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(ApplyForcesCB) );	
	m_dx11Context->Unmap( integrateKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &integrateKernel.constBuffer );

	// Set resources and dispatch	
	m_dx11Context->CSSetShaderResources( 0, 1, &(m_vertexData.m_dx11ClothIdentifier.getSRV()) );
	m_dx11Context->CSSetShaderResources( 1, 1, &(m_vertexData.m_dx11VertexNormal.getSRV()) );
	m_dx11Context->CSSetShaderResources( 2, 1, &(m_vertexData.m_dx11VertexArea.getSRV()) );
	m_dx11Context->CSSetShaderResources( 3, 1, &(m_vertexData.m_dx11VertexInverseMass.getSRV()) );
	m_dx11Context->CSSetShaderResources( 4, 1, &(m_dx11PerClothLiftFactor.getSRV()) );
	m_dx11Context->CSSetShaderResources( 5, 1, &(m_dx11PerClothDragFactor.getSRV()) );
	m_dx11Context->CSSetShaderResources( 6, 1, &(m_dx11PerClothWindVelocity.getSRV()) );
	m_dx11Context->CSSetShaderResources( 7, 1, &(m_dx11PerClothAcceleration.getSRV()) );
	m_dx11Context->CSSetShaderResources( 8, 1, &(m_dx11PerClothMediumDensity.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_vertexData.m_dx11VertexForceAccumulator.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &(m_vertexData.m_dx11VertexVelocity.getUAV()), NULL );

	// Execute the kernel
	m_dx11Context->CSSetShader( applyForcesKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numNodes + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks, 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 0, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 1, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 2, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 3, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 4, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 5, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 6, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 7, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 8, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	
} // btDX11SoftBodySolver::applyForces

/**
 * Integrate motion on the solver.
 */
void btDX11SoftBodySolver::integrate( float solverdt )
{
	// TEMPORARY COPIES
	m_vertexData.moveToAccelerator();

	// No need to batch link solver, it is entirely parallel
	// Copy kernel parameters to GPU
	IntegrateCB constBuffer;
	
	constBuffer.numNodes = m_vertexData.getNumVertices();
	constBuffer.solverdt = solverdt;
	
	// Todo: factor this out. Number of nodes is static and sdt might be, too, we can update this just once on setup
	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( integrateKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(IntegrateCB) );	
	m_dx11Context->Unmap( integrateKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &integrateKernel.constBuffer );

	// Set resources and dispatch
	m_dx11Context->CSSetShaderResources( 0, 1, &(m_vertexData.m_dx11VertexInverseMass.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_vertexData.m_dx11VertexPosition.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &(m_vertexData.m_dx11VertexVelocity.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 2, 1, &(m_vertexData.m_dx11VertexPreviousPosition.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 3, 1, &(m_vertexData.m_dx11VertexForceAccumulator.getUAV()), NULL );

	// Execute the kernel
	m_dx11Context->CSSetShader( integrateKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numNodes + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks, 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 0, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 2, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 3, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	
} // btDX11SoftBodySolver::integrate

float btDX11SoftBodySolver::computeTriangleArea( 
	const Vectormath::Aos::Point3 &vertex0,
	const Vectormath::Aos::Point3 &vertex1,
	const Vectormath::Aos::Point3 &vertex2 )
{
	Vectormath::Aos::Vector3 a = vertex1 - vertex0;
	Vectormath::Aos::Vector3 b = vertex2 - vertex0;
	Vectormath::Aos::Vector3 crossProduct = cross(a, b);
	float area = length( crossProduct );
	return area;
} // btDX11SoftBodySolver::computeTriangleArea


void btDX11SoftBodySolver::updateBounds()
{	
	using Vectormath::Aos::Point3;
	// Interpretation structure for float and int
	
	struct FPRep {
		unsigned int mantissa  : 23;
		unsigned int exponent : 8;
		unsigned int sign    : 1;
	};
	union FloatAsInt
	{
		float floatValue;
		int intValue;
		unsigned int uintValue;
		FPRep fpRep;
	};

	
	// Update bounds array to min and max int values to allow easy atomics
	for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
	{
		m_perClothMinBounds[softBodyIndex] = UIntVector3( UINT_MAX, UINT_MAX, UINT_MAX );
		m_perClothMaxBounds[softBodyIndex] = UIntVector3( 0, 0, 0 );
	}
	
	m_dx11PerClothMinBounds.moveToGPU();
	m_dx11PerClothMaxBounds.moveToGPU();


	computeBounds( );


	m_dx11PerClothMinBounds.moveFromGPU();
	m_dx11PerClothMaxBounds.moveFromGPU();


	
	for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
	{
		UIntVector3 minBoundUInt = m_perClothMinBounds[softBodyIndex];
		UIntVector3 maxBoundUInt = m_perClothMaxBounds[softBodyIndex];
				
		// Convert back to float
		FloatAsInt fai;

		btVector3 minBound;
		fai.uintValue = minBoundUInt.x;
	    fai.uintValue ^= (((fai.uintValue >> 31) - 1) | 0x80000000);
		minBound.setX( fai.floatValue );
		fai.uintValue = minBoundUInt.y;
		fai.uintValue ^= (((fai.uintValue >> 31) - 1) | 0x80000000);
		minBound.setY( fai.floatValue );
		fai.uintValue = minBoundUInt.z;
		fai.uintValue ^= (((fai.uintValue >> 31) - 1) | 0x80000000);
		minBound.setZ( fai.floatValue );

		btVector3 maxBound;
		fai.uintValue = maxBoundUInt.x;
		fai.uintValue ^= (((fai.uintValue >> 31) - 1) | 0x80000000);
		maxBound.setX( fai.floatValue );
		fai.uintValue = maxBoundUInt.y;
		fai.uintValue ^= (((fai.uintValue >> 31) - 1) | 0x80000000);
		maxBound.setY( fai.floatValue );
		fai.uintValue = maxBoundUInt.z;
		fai.uintValue ^= (((fai.uintValue >> 31) - 1) | 0x80000000);
		maxBound.setZ( fai.floatValue );
		
		// And finally assign to the soft body
		m_softBodySet[softBodyIndex]->updateBounds( minBound, maxBound );
	}
}

void btDX11SoftBodySolver::updateConstants( float timeStep )
{
	using namespace Vectormath::Aos;

	if( m_updateSolverConstants )
	{
		m_updateSolverConstants = false;

		// Will have to redo this if we change the structure (tear, maybe) or various other possible changes

		// Initialise link constants
		const int numLinks = m_linkData.getNumLinks();
		for( int linkIndex = 0; linkIndex < numLinks; ++linkIndex )
		{
			btSoftBodyLinkData::LinkNodePair &vertices( m_linkData.getVertexPair(linkIndex) );
			m_linkData.getRestLength(linkIndex) = length((m_vertexData.getPosition( vertices.vertex0 ) - m_vertexData.getPosition( vertices.vertex1 )));
			float invMass0 = m_vertexData.getInverseMass(vertices.vertex0);
			float invMass1 = m_vertexData.getInverseMass(vertices.vertex1);
			float linearStiffness = m_linkData.getLinearStiffnessCoefficient(linkIndex);
			float massLSC = (invMass0 + invMass1)/linearStiffness;
			m_linkData.getMassLSC(linkIndex) = massLSC;
			float restLength = m_linkData.getRestLength(linkIndex);
			float restLengthSquared = restLength*restLength;
			m_linkData.getRestLengthSquared(linkIndex) = restLengthSquared;
		}
	}
} // btDX11SoftBodySolver::updateConstants

/**
 * Sort the collision object details array and generate indexing into it for the per-cloth collision object array.
 */
void btDX11SoftBodySolver::prepareCollisionConstraints()
{
	// First do a simple sort on the collision objects
	btAlignedObjectArray<int> numObjectsPerClothPrefixSum;
	btAlignedObjectArray<int> numObjectsPerCloth;
	numObjectsPerCloth.resize( m_softBodySet.size(), 0 );
	numObjectsPerClothPrefixSum.resize( m_softBodySet.size(), 0 );


	class QuickSortCompare
	{
		public:

		bool operator() ( const CollisionShapeDescription& a, const CollisionShapeDescription& b )
		{
			return ( a.softBodyIdentifier < b.softBodyIdentifier );
		}
	};

	QuickSortCompare comparator;
	m_collisionObjectDetails.quickSort( comparator );

	// Generating indexing for perClothCollisionObjects
	// First clear the previous values with the "no collision object for cloth" constant
	for( int clothIndex = 0; clothIndex < m_perClothCollisionObjects.size(); ++clothIndex )
	{
		m_perClothCollisionObjects[clothIndex].firstObject = -1;
		m_perClothCollisionObjects[clothIndex].endObject = -1;
	}
	int currentCloth = 0;
	int startIndex = 0;
	for( int collisionObject = 0; collisionObject < m_collisionObjectDetails.size(); ++collisionObject )
	{
		int nextCloth = m_collisionObjectDetails[collisionObject].softBodyIdentifier;
		if( nextCloth != currentCloth )
		{	
			// Changed cloth in the array
			// Set the end index and the range is what we need for currentCloth
			m_perClothCollisionObjects[currentCloth].firstObject = startIndex;
			m_perClothCollisionObjects[currentCloth].endObject = collisionObject;
			currentCloth = nextCloth;
			startIndex = collisionObject;
		}
	}

	// And update last cloth	
	m_perClothCollisionObjects[currentCloth].firstObject = startIndex;
	m_perClothCollisionObjects[currentCloth].endObject =  m_collisionObjectDetails.size();
	
} // btDX11SoftBodySolver::prepareCollisionConstraints


void btDX11SoftBodySolver::solveConstraints( float solverdt )
{

	//std::cerr << "'GPU' solve constraints\n";
	using Vectormath::Aos::Vector3;
	using Vectormath::Aos::Point3;
	using Vectormath::Aos::lengthSqr;
	using Vectormath::Aos::dot;

	// Prepare links
	int numLinks = m_linkData.getNumLinks();
	int numVertices = m_vertexData.getNumVertices();

	float kst = 1.f;
	float ti = 0.f;


	m_dx11PerClothDampingFactor.moveToGPU();
	m_dx11PerClothVelocityCorrectionCoefficient.moveToGPU();


	// Ensure data is on accelerator
	m_linkData.moveToAccelerator();
	m_vertexData.moveToAccelerator();


	prepareLinks();	

	for( int iteration = 0; iteration < m_numberOfVelocityIterations ; ++iteration )
	{
		for( int i = 0; i < m_linkData.m_batchStartLengths.size(); ++i )
		{
			int startLink = m_linkData.m_batchStartLengths[i].start;
			int numLinks = m_linkData.m_batchStartLengths[i].length;

			solveLinksForVelocity( startLink, numLinks, kst );
		}
	}

	
	prepareCollisionConstraints();

	// Compute new positions from velocity
	// Also update the previous position so that our position computation is now based on the new position from the velocity solution
	// rather than based directly on the original positions
	if( m_numberOfVelocityIterations > 0 )
	{
		updateVelocitiesFromPositionsWithVelocities( 1.f/solverdt );
	} else {
		updateVelocitiesFromPositionsWithoutVelocities( 1.f/solverdt );
	}


	// Solve drift
	for( int iteration = 0; iteration < m_numberOfPositionIterations ; ++iteration )
	{
		for( int i = 0; i < m_linkData.m_batchStartLengths.size(); ++i )
		{
			int startLink = m_linkData.m_batchStartLengths[i].start;
			int numLinks = m_linkData.m_batchStartLengths[i].length;

			solveLinksForPosition( startLink, numLinks, kst, ti );
		}
		
	} // for( int iteration = 0; iteration < m_numberOfPositionIterations ; ++iteration )

	// At this point assume that the force array is blank - we will overwrite it
	solveCollisionsAndUpdateVelocities( 1.f/solverdt );
} // btDX11SoftBodySolver::solveConstraints




//////////////////////////////////////
// Kernel dispatches
void btDX11SoftBodySolver::prepareLinks()
{
	// No need to batch link solver, it is entirely parallel
	// Copy kernel parameters to GPU
	PrepareLinksCB constBuffer;
	
	constBuffer.numLinks = m_linkData.getNumLinks();
	
	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( prepareLinksKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(PrepareLinksCB) );	
	m_dx11Context->Unmap( prepareLinksKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &prepareLinksKernel.constBuffer );

	// Set resources and dispatch
	m_dx11Context->CSSetShaderResources( 0, 1, &(m_linkData.m_dx11Links.getSRV()) );
	m_dx11Context->CSSetShaderResources( 1, 1, &(m_linkData.m_dx11LinksMassLSC.getSRV()) );
	m_dx11Context->CSSetShaderResources( 2, 1, &(m_vertexData.m_dx11VertexPreviousPosition.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_linkData.m_dx11LinksLengthRatio.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &(m_linkData.m_dx11LinksCLength.getUAV()), NULL );

	// Execute the kernel
	m_dx11Context->CSSetShader( prepareLinksKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numLinks + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks , 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 0, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 1, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 2, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &pUAViewNULL, NULL );
		
		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}
} // btDX11SoftBodySolver::prepareLinks


void btDX11SoftBodySolver::updatePositionsFromVelocities( float solverdt )
{
	// No need to batch link solver, it is entirely parallel
	// Copy kernel parameters to GPU
	UpdatePositionsFromVelocitiesCB constBuffer;
	
	constBuffer.numNodes = m_vertexData.getNumVertices();
	constBuffer.solverSDT = solverdt;
	
	// Todo: factor this out. Number of nodes is static and sdt might be, too, we can update this just once on setup
	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( updatePositionsFromVelocitiesKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(UpdatePositionsFromVelocitiesCB) );	
	m_dx11Context->Unmap( updatePositionsFromVelocitiesKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &updatePositionsFromVelocitiesKernel.constBuffer );

	// Set resources and dispatch			
	m_dx11Context->CSSetShaderResources( 0, 1, &(m_vertexData.m_dx11VertexVelocity.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_vertexData.m_dx11VertexPreviousPosition.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &(m_vertexData.m_dx11VertexPosition.getUAV()), NULL );

	// Execute the kernel
	m_dx11Context->CSSetShader( updatePositionsFromVelocitiesKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numNodes + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks, 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 0, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	
} // btDX11SoftBodySolver::updatePositionsFromVelocities

void btDX11SoftBodySolver::solveLinksForPosition( int startLink, int numLinks, float kst, float ti )
{
	// Copy kernel parameters to GPU
	SolvePositionsFromLinksKernelCB constBuffer;

	// Set the first link of the batch
	// and the batch size
	constBuffer.startLink = startLink;
	constBuffer.numLinks = numLinks;

	constBuffer.kst = kst;
	constBuffer.ti = ti;
	
	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( solvePositionsFromLinksKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(SolvePositionsFromLinksKernelCB) );	
	m_dx11Context->Unmap( solvePositionsFromLinksKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &solvePositionsFromLinksKernel.constBuffer );

	// Set resources and dispatch
	m_dx11Context->CSSetShaderResources( 0, 1, &(m_linkData.m_dx11Links.getSRV()) );
	m_dx11Context->CSSetShaderResources( 1, 1, &(m_linkData.m_dx11LinksMassLSC.getSRV()) );
	m_dx11Context->CSSetShaderResources( 2, 1, &(m_linkData.m_dx11LinksRestLengthSquared.getSRV()) );
	m_dx11Context->CSSetShaderResources( 3, 1, &(m_vertexData.m_dx11VertexInverseMass.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_vertexData.m_dx11VertexPosition.getUAV()), NULL );

	// Execute the kernel
	m_dx11Context->CSSetShader( solvePositionsFromLinksKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numLinks + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks , 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 0, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 1, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 2, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 3, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	
	
} // btDX11SoftBodySolver::solveLinksForPosition

void btDX11SoftBodySolver::solveLinksForVelocity( int startLink, int numLinks, float kst )
{
	// Copy kernel parameters to GPU
	VSolveLinksCB constBuffer;

	// Set the first link of the batch
	// and the batch size

	constBuffer.startLink = startLink;
	constBuffer.numLinks = numLinks;
	constBuffer.kst = kst;
	
	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( vSolveLinksKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(VSolveLinksCB) );	
	m_dx11Context->Unmap( vSolveLinksKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &vSolveLinksKernel.constBuffer );

	// Set resources and dispatch
	m_dx11Context->CSSetShaderResources( 0, 1, &(m_linkData.m_dx11Links.getSRV()) );
	m_dx11Context->CSSetShaderResources( 1, 1, &(m_linkData.m_dx11LinksLengthRatio.getSRV()) );
	m_dx11Context->CSSetShaderResources( 2, 1, &(m_linkData.m_dx11LinksCLength.getSRV()) );
	m_dx11Context->CSSetShaderResources( 3, 1, &(m_vertexData.m_dx11VertexInverseMass.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_vertexData.m_dx11VertexVelocity.getUAV()), NULL );

	// Execute the kernel
	m_dx11Context->CSSetShader( vSolveLinksKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numLinks + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks , 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 0, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 1, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 2, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 3, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	
} // btDX11SoftBodySolver::solveLinksForVelocity


void btDX11SoftBodySolver::updateVelocitiesFromPositionsWithVelocities( float isolverdt )
{
	// Copy kernel parameters to GPU
	UpdateVelocitiesFromPositionsWithVelocitiesCB constBuffer;

	// Set the first link of the batch
	// and the batch size
	constBuffer.numNodes = m_vertexData.getNumVertices();
	constBuffer.isolverdt = isolverdt;

	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( updateVelocitiesFromPositionsWithVelocitiesKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(UpdateVelocitiesFromPositionsWithVelocitiesCB) );	
	m_dx11Context->Unmap( updateVelocitiesFromPositionsWithVelocitiesKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &updateVelocitiesFromPositionsWithVelocitiesKernel.constBuffer );

	// Set resources and dispatch
	m_dx11Context->CSSetShaderResources( 0, 1, &(m_vertexData.m_dx11VertexPosition.getSRV()) );
	m_dx11Context->CSSetShaderResources( 1, 1, &(m_vertexData.m_dx11VertexPreviousPosition.getSRV()) );
	m_dx11Context->CSSetShaderResources( 2, 1, &(m_vertexData.m_dx11ClothIdentifier.getSRV()) );
	m_dx11Context->CSSetShaderResources( 3, 1, &(m_dx11PerClothVelocityCorrectionCoefficient.getSRV()) );
	m_dx11Context->CSSetShaderResources( 4, 1, &(m_dx11PerClothDampingFactor.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_vertexData.m_dx11VertexVelocity.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &(m_vertexData.m_dx11VertexForceAccumulator.getUAV()), NULL );


	// Execute the kernel
	m_dx11Context->CSSetShader( updateVelocitiesFromPositionsWithVelocitiesKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numNodes + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks , 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 0, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 1, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 2, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 3, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 4, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	

} // btDX11SoftBodySolver::updateVelocitiesFromPositionsWithVelocities

void btDX11SoftBodySolver::updateVelocitiesFromPositionsWithoutVelocities( float isolverdt )
{
	// Copy kernel parameters to GPU
	UpdateVelocitiesFromPositionsWithoutVelocitiesCB constBuffer;

	// Set the first link of the batch
	// and the batch size
	constBuffer.numNodes = m_vertexData.getNumVertices();
	constBuffer.isolverdt = isolverdt;

	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( updateVelocitiesFromPositionsWithoutVelocitiesKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(UpdateVelocitiesFromPositionsWithoutVelocitiesCB) );	
	m_dx11Context->Unmap( updateVelocitiesFromPositionsWithoutVelocitiesKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &updateVelocitiesFromPositionsWithoutVelocitiesKernel.constBuffer );

	// Set resources and dispatch
	m_dx11Context->CSSetShaderResources( 0, 1, &(m_vertexData.m_dx11VertexPosition.getSRV()) );
	m_dx11Context->CSSetShaderResources( 1, 1, &(m_vertexData.m_dx11VertexPreviousPosition.getSRV()) );
	m_dx11Context->CSSetShaderResources( 2, 1, &(m_vertexData.m_dx11ClothIdentifier.getSRV()) );
	m_dx11Context->CSSetShaderResources( 3, 1, &(m_dx11PerClothDampingFactor.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_vertexData.m_dx11VertexVelocity.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &(m_vertexData.m_dx11VertexForceAccumulator.getUAV()), NULL );


	// Execute the kernel
	m_dx11Context->CSSetShader( updateVelocitiesFromPositionsWithoutVelocitiesKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numNodes + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks , 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 0, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 1, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 2, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 3, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	

} // btDX11SoftBodySolver::updateVelocitiesFromPositionsWithoutVelocities


void btDX11SoftBodySolver::computeBounds( )
{
	ComputeBoundsCB constBuffer;
	m_vertexData.moveToAccelerator();

	// Set the first link of the batch
	// and the batch size
	constBuffer.numNodes = m_vertexData.getNumVertices();
	constBuffer.numSoftBodies = m_softBodySet.size();

	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( computeBoundsKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(ComputeBoundsCB) );	
	m_dx11Context->Unmap( computeBoundsKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &computeBoundsKernel.constBuffer );

	// Set resources and dispatch
	m_dx11Context->CSSetShaderResources( 0, 1, &(m_vertexData.m_dx11ClothIdentifier.getSRV()) );
	m_dx11Context->CSSetShaderResources( 1, 1, &(m_vertexData.m_dx11VertexPosition.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_dx11PerClothMinBounds.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &(m_dx11PerClothMaxBounds.getUAV()), NULL );
	
	// Execute the kernel
	m_dx11Context->CSSetShader( computeBoundsKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numNodes + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks , 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 0, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 1, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	
}

void btDX11SoftBodySolver::solveCollisionsAndUpdateVelocities( float isolverdt )
{

	// Copy kernel parameters to GPU
	m_vertexData.moveToAccelerator();
	m_dx11PerClothFriction.moveToGPU();
	m_dx11PerClothDampingFactor.moveToGPU();
	m_dx11PerClothCollisionObjects.moveToGPU();
	m_dx11CollisionObjectDetails.moveToGPU();

	SolveCollisionsAndUpdateVelocitiesCB constBuffer;

	// Set the first link of the batch
	// and the batch size
	constBuffer.numNodes = m_vertexData.getNumVertices();
	constBuffer.isolverdt = isolverdt;


	D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
	m_dx11Context->Map( solveCollisionsAndUpdateVelocitiesKernel.constBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
	memcpy( MappedResource.pData, &constBuffer, sizeof(SolveCollisionsAndUpdateVelocitiesCB) );	
	m_dx11Context->Unmap( solveCollisionsAndUpdateVelocitiesKernel.constBuffer, 0 );
	m_dx11Context->CSSetConstantBuffers( 0, 1, &solveCollisionsAndUpdateVelocitiesKernel.constBuffer );

	// Set resources and dispatch
	m_dx11Context->CSSetShaderResources( 0, 1, &(m_vertexData.m_dx11ClothIdentifier.getSRV()) );	
	m_dx11Context->CSSetShaderResources( 1, 1, &(m_vertexData.m_dx11VertexPreviousPosition.getSRV()) );
	m_dx11Context->CSSetShaderResources( 2, 1, &(m_dx11PerClothFriction.getSRV()) );
	m_dx11Context->CSSetShaderResources( 3, 1, &(m_dx11PerClothDampingFactor.getSRV()) );
	m_dx11Context->CSSetShaderResources( 4, 1, &(m_dx11PerClothCollisionObjects.getSRV()) );
	m_dx11Context->CSSetShaderResources( 5, 1, &(m_dx11CollisionObjectDetails.getSRV()) );

	m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(m_vertexData.m_dx11VertexForceAccumulator.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &(m_vertexData.m_dx11VertexVelocity.getUAV()), NULL );
	m_dx11Context->CSSetUnorderedAccessViews( 2, 1, &(m_vertexData.m_dx11VertexPosition.getUAV()), NULL );
	
	// Execute the kernel
	m_dx11Context->CSSetShader( solveCollisionsAndUpdateVelocitiesKernel.kernel, NULL, 0 );

	int	numBlocks = (constBuffer.numNodes + (128-1)) / 128;
	m_dx11Context->Dispatch(numBlocks , 1, 1 );

	{
		// Tidy up 
		ID3D11ShaderResourceView* pViewNULL = NULL;
		m_dx11Context->CSSetShaderResources( 0, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 1, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 2, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 3, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 4, 1, &pViewNULL );
		m_dx11Context->CSSetShaderResources( 5, 1, &pViewNULL );

		ID3D11UnorderedAccessView* pUAViewNULL = NULL;
		m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 1, 1, &pUAViewNULL, NULL );
		m_dx11Context->CSSetUnorderedAccessViews( 2, 1, &pUAViewNULL, NULL );

		ID3D11Buffer *pBufferNull = NULL;
		m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
	}	

} // btDX11SoftBodySolver::solveCollisionsAndUpdateVelocities

// End kernel dispatches
/////////////////////////////////////














btDX11SoftBodySolver::btAcceleratedSoftBodyInterface *btDX11SoftBodySolver::findSoftBodyInterface( const btSoftBody* const softBody )
{
	for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
	{
		btAcceleratedSoftBodyInterface *softBodyInterface = m_softBodySet[softBodyIndex];
		if( softBodyInterface->getSoftBody() == softBody )
			return softBodyInterface;
	}
	return 0;
}

const btDX11SoftBodySolver::btAcceleratedSoftBodyInterface * const btDX11SoftBodySolver::findSoftBodyInterface( const btSoftBody* const softBody ) const
{
	for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
	{
		btAcceleratedSoftBodyInterface *softBodyInterface = m_softBodySet[softBodyIndex];
		if( softBodyInterface->getSoftBody() == softBody )
			return softBodyInterface;
	}
	return 0;
}

int btDX11SoftBodySolver::findSoftBodyIndex( const btSoftBody* const softBody )
{
	for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
	{
		btAcceleratedSoftBodyInterface *softBodyInterface = m_softBodySet[softBodyIndex];
		if( softBodyInterface->getSoftBody() == softBody )
			return softBodyIndex;
	}
	return 1;
}


void btSoftBodySolverOutputDXtoCPU::copySoftBodyToVertexBuffer( const btSoftBody * const softBody, btVertexBufferDescriptor *vertexBuffer )
{
	

	btSoftBodySolver *solver = softBody->getSoftBodySolver();
	btAssert( solver->getSolverType() == btSoftBodySolver::DX_SOLVER || solver->getSolverType() == btSoftBodySolver::DX_SIMD_SOLVER );
	btDX11SoftBodySolver *dxSolver = static_cast< btDX11SoftBodySolver * >( solver );

	btDX11SoftBodySolver::btAcceleratedSoftBodyInterface * currentCloth = dxSolver->findSoftBodyInterface( softBody );
	btSoftBodyVertexDataDX11 &vertexData( dxSolver->m_vertexData );
	

	const int firstVertex = currentCloth->getFirstVertex();
	const int lastVertex = firstVertex + currentCloth->getNumVertices();

	if( vertexBuffer->getBufferType() == btVertexBufferDescriptor::CPU_BUFFER )
	{		
		// If we're doing a CPU-buffer copy must copy the data back to the host first
		vertexData.m_dx11VertexPosition.copyFromGPU();
		vertexData.m_dx11VertexNormal.copyFromGPU();

		const int firstVertex = currentCloth->getFirstVertex();
		const int lastVertex = firstVertex + currentCloth->getNumVertices();
		const btCPUVertexBufferDescriptor *cpuVertexBuffer = static_cast< btCPUVertexBufferDescriptor* >(vertexBuffer);						
		float *basePointer = cpuVertexBuffer->getBasePointer();						

		if( vertexBuffer->hasVertexPositions() )
		{
			const int vertexOffset = cpuVertexBuffer->getVertexOffset();
			const int vertexStride = cpuVertexBuffer->getVertexStride();
			float *vertexPointer = basePointer + vertexOffset;

			for( int vertexIndex = firstVertex; vertexIndex < lastVertex; ++vertexIndex )
			{
				Vectormath::Aos::Point3 position = vertexData.getPosition(vertexIndex);
				*(vertexPointer + 0) = position.getX();
				*(vertexPointer + 1) = position.getY();
				*(vertexPointer + 2) = position.getZ();
				vertexPointer += vertexStride;
			}
		}
		if( vertexBuffer->hasNormals() )
		{
			const int normalOffset = cpuVertexBuffer->getNormalOffset();
			const int normalStride = cpuVertexBuffer->getNormalStride();
			float *normalPointer = basePointer + normalOffset;

			for( int vertexIndex = firstVertex; vertexIndex < lastVertex; ++vertexIndex )
			{
				Vectormath::Aos::Vector3 normal = vertexData.getNormal(vertexIndex);
				*(normalPointer + 0) = normal.getX();
				*(normalPointer + 1) = normal.getY();
				*(normalPointer + 2) = normal.getZ();
				normalPointer += normalStride;
			}
		}
	} 
} // btDX11SoftBodySolver::outputToVertexBuffers



bool btSoftBodySolverOutputDXtoDX::checkInitialized()
{
	if( !m_shadersInitialized )
		if( buildShaders() )
			m_shadersInitialized = true;

	return m_shadersInitialized;
}

void btSoftBodySolverOutputDXtoDX::releaseKernels()
{
	SAFE_RELEASE( outputToVertexArrayWithNormalsKernel.constBuffer );
	SAFE_RELEASE( outputToVertexArrayWithNormalsKernel.kernel );
	SAFE_RELEASE( outputToVertexArrayWithoutNormalsKernel.constBuffer );
	SAFE_RELEASE( outputToVertexArrayWithoutNormalsKernel.kernel );

	m_shadersInitialized = false;
}


bool btSoftBodySolverOutputDXtoDX::buildShaders()
{
	// Ensure current kernels are released first
	releaseKernels();

	bool returnVal = true;

	if( m_shadersInitialized )
		return true;
	

	outputToVertexArrayWithNormalsKernel = dxFunctions.compileComputeShaderFromString( OutputToVertexArrayHLSLString, "OutputToVertexArrayWithNormalsKernel", sizeof(OutputToVertexArrayCB) );
	if( !outputToVertexArrayWithNormalsKernel.constBuffer)
		returnVal = false;
	outputToVertexArrayWithoutNormalsKernel = dxFunctions.compileComputeShaderFromString( OutputToVertexArrayHLSLString, "OutputToVertexArrayWithoutNormalsKernel", sizeof(OutputToVertexArrayCB) );
	if( !outputToVertexArrayWithoutNormalsKernel.constBuffer )
		returnVal = false;


	if( returnVal )
		m_shadersInitialized = true;

	return returnVal;
}


void btSoftBodySolverOutputDXtoDX::copySoftBodyToVertexBuffer( const btSoftBody * const softBody, btVertexBufferDescriptor *vertexBuffer )
{
	

	btSoftBodySolver *solver = softBody->getSoftBodySolver();
	btAssert( solver->getSolverType() == btSoftBodySolver::DX_SOLVER || solver->getSolverType() == btSoftBodySolver::DX_SIMD_SOLVER );
	btDX11SoftBodySolver *dxSolver = static_cast< btDX11SoftBodySolver * >( solver );
	checkInitialized();
	btDX11SoftBodySolver::btAcceleratedSoftBodyInterface * currentCloth = dxSolver->findSoftBodyInterface( softBody );
	btSoftBodyVertexDataDX11 &vertexData( dxSolver->m_vertexData );


	const int firstVertex = currentCloth->getFirstVertex();
	const int lastVertex = firstVertex + currentCloth->getNumVertices();

	if( vertexBuffer->getBufferType() == btVertexBufferDescriptor::CPU_BUFFER )
	{		
		btSoftBodySolverOutputDXtoDX::copySoftBodyToVertexBuffer( softBody, vertexBuffer );
	} else 	if( vertexBuffer->getBufferType() == btVertexBufferDescriptor::DX11_BUFFER )
	{
		// Do a DX11 copy shader DX to DX copy

		const btDX11VertexBufferDescriptor *dx11VertexBuffer = static_cast< btDX11VertexBufferDescriptor* >(vertexBuffer);	

		// No need to batch link solver, it is entirely parallel
		// Copy kernel parameters to GPU
		OutputToVertexArrayCB constBuffer;
		ID3D11ComputeShader* outputToVertexArrayShader = outputToVertexArrayWithoutNormalsKernel.kernel;
		ID3D11Buffer* outputToVertexArrayConstBuffer = outputToVertexArrayWithoutNormalsKernel.constBuffer;
		
		constBuffer.startNode = firstVertex;
		constBuffer.numNodes = currentCloth->getNumVertices();
		constBuffer.positionOffset = vertexBuffer->getVertexOffset();
		constBuffer.positionStride = vertexBuffer->getVertexStride();
		if( vertexBuffer->hasNormals() )
		{
			constBuffer.normalOffset = vertexBuffer->getNormalOffset();
			constBuffer.normalStride = vertexBuffer->getNormalStride();
			outputToVertexArrayShader = outputToVertexArrayWithNormalsKernel.kernel;
			outputToVertexArrayConstBuffer = outputToVertexArrayWithNormalsKernel.constBuffer;
		}	
		
		// TODO: factor this out. Number of nodes is static and sdt might be, too, we can update this just once on setup
		D3D11_MAPPED_SUBRESOURCE MappedResource = {0};
		dxFunctions.m_dx11Context->Map( outputToVertexArrayConstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
		memcpy( MappedResource.pData, &constBuffer, sizeof(OutputToVertexArrayCB) );	
		dxFunctions.m_dx11Context->Unmap( outputToVertexArrayConstBuffer, 0 );
		dxFunctions.m_dx11Context->CSSetConstantBuffers( 0, 1, &outputToVertexArrayConstBuffer );

		// Set resources and dispatch
		dxFunctions.m_dx11Context->CSSetShaderResources( 0, 1, &(vertexData.m_dx11VertexPosition.getSRV()) );
		dxFunctions.m_dx11Context->CSSetShaderResources( 1, 1, &(vertexData.m_dx11VertexNormal.getSRV()) );

		ID3D11UnorderedAccessView* dx11UAV = dx11VertexBuffer->getDX11UAV();
		dxFunctions.m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &(dx11UAV), NULL );

		// Execute the kernel
		dxFunctions.m_dx11Context->CSSetShader( outputToVertexArrayShader, NULL, 0 );

		int	numBlocks = (constBuffer.numNodes + (128-1)) / 128;
		dxFunctions.m_dx11Context->Dispatch(numBlocks, 1, 1 );

		{
			// Tidy up 
			ID3D11ShaderResourceView* pViewNULL = NULL;
			dxFunctions.m_dx11Context->CSSetShaderResources( 0, 1, &pViewNULL );
			dxFunctions.m_dx11Context->CSSetShaderResources( 1, 1, &pViewNULL );

			ID3D11UnorderedAccessView* pUAViewNULL = NULL;
			dxFunctions.m_dx11Context->CSSetUnorderedAccessViews( 0, 1, &pUAViewNULL, NULL );

			ID3D11Buffer *pBufferNull = NULL;
			dxFunctions.m_dx11Context->CSSetConstantBuffers( 0, 1, &pBufferNull );
		}	
	}
} // btDX11SoftBodySolver::outputToVertexBuffers




DXFunctions::KernelDesc DXFunctions::compileComputeShaderFromString( const char* shaderString, const char* shaderName, int constBufferSize, D3D10_SHADER_MACRO *compileMacros )
{
	const char *cs5String = "cs_5_0";

	HRESULT hr = S_OK;
	ID3DBlob* pErrorBlob = NULL;
	ID3DBlob* pBlob = NULL;
	ID3D11ComputeShader*		kernelPointer = 0;

	hr = m_dx11CompileFromMemory( 
		shaderString,
		strlen(shaderString),
		shaderName,
		compileMacros,
		NULL,
		shaderName,
		cs5String,
		D3D10_SHADER_ENABLE_STRICTNESS,
		NULL,
		NULL,
		&pBlob,
		&pErrorBlob,
		NULL
		);

	if( FAILED(hr) )
	{
		if( pErrorBlob ) {
			btAssert( "Compilation of compute shader failed\n" );
			char *debugString = (char*)pErrorBlob->GetBufferPointer();
			OutputDebugStringA( debugString );
		}
	
		SAFE_RELEASE( pErrorBlob );
		SAFE_RELEASE( pBlob );    

		DXFunctions::KernelDesc descriptor;
		descriptor.kernel = 0;
		descriptor.constBuffer = 0;
		return descriptor;
	}    

	// Create the Compute Shader
	hr = m_dx11Device->CreateComputeShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), NULL, &kernelPointer );
	if( FAILED( hr ) )
	{
		DXFunctions::KernelDesc descriptor;
		descriptor.kernel = 0;
		descriptor.constBuffer = 0;
		return descriptor;
	}

	ID3D11Buffer* constBuffer = 0;
	if( constBufferSize > 0 )
	{
		// Create the constant buffer
		D3D11_BUFFER_DESC constant_buffer_desc;
		ZeroMemory(&constant_buffer_desc, sizeof(constant_buffer_desc));
		constant_buffer_desc.ByteWidth = constBufferSize;
		constant_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		constant_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constant_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		m_dx11Device->CreateBuffer(&constant_buffer_desc, NULL, &constBuffer);
		if( FAILED( hr ) )
		{
			KernelDesc descriptor;
			descriptor.kernel = 0;
			descriptor.constBuffer = 0;
			return descriptor;
		}
	}

	SAFE_RELEASE( pErrorBlob );
	SAFE_RELEASE( pBlob );

	DXFunctions::KernelDesc descriptor;
	descriptor.kernel = kernelPointer;
	descriptor.constBuffer = constBuffer;
	return descriptor;
} // compileComputeShader



bool btDX11SoftBodySolver::buildShaders()
{
	// Ensure current kernels are released first
	releaseKernels();

	bool returnVal = true;

	if( m_shadersInitialized )
		return true;

	prepareLinksKernel = dxFunctions.compileComputeShaderFromString( PrepareLinksHLSLString, "PrepareLinksKernel", sizeof(PrepareLinksCB) );
	if( !prepareLinksKernel.constBuffer )
		returnVal = false;
	updatePositionsFromVelocitiesKernel = dxFunctions.compileComputeShaderFromString( UpdatePositionsFromVelocitiesHLSLString, "UpdatePositionsFromVelocitiesKernel", sizeof(UpdatePositionsFromVelocitiesCB) );
	if( !updatePositionsFromVelocitiesKernel.constBuffer )
		returnVal = false;
	solvePositionsFromLinksKernel = dxFunctions.compileComputeShaderFromString( SolvePositionsHLSLString, "SolvePositionsFromLinksKernel", sizeof(SolvePositionsFromLinksKernelCB) );
	if( !updatePositionsFromVelocitiesKernel.constBuffer )
		returnVal = false;
	vSolveLinksKernel = dxFunctions.compileComputeShaderFromString( VSolveLinksHLSLString, "VSolveLinksKernel", sizeof(VSolveLinksCB) );
	if( !vSolveLinksKernel.constBuffer )
		returnVal = false;
	updateVelocitiesFromPositionsWithVelocitiesKernel = dxFunctions.compileComputeShaderFromString( UpdateNodesHLSLString, "updateVelocitiesFromPositionsWithVelocitiesKernel", sizeof(UpdateVelocitiesFromPositionsWithVelocitiesCB) );
	if( !updateVelocitiesFromPositionsWithVelocitiesKernel.constBuffer )
		returnVal = false;
	updateVelocitiesFromPositionsWithoutVelocitiesKernel = dxFunctions.compileComputeShaderFromString( UpdatePositionsHLSLString, "updateVelocitiesFromPositionsWithoutVelocitiesKernel", sizeof(UpdateVelocitiesFromPositionsWithoutVelocitiesCB) );
	if( !updateVelocitiesFromPositionsWithoutVelocitiesKernel.constBuffer )
		returnVal = false;
	integrateKernel = dxFunctions.compileComputeShaderFromString( IntegrateHLSLString, "IntegrateKernel", sizeof(IntegrateCB) );
	if( !integrateKernel.constBuffer )
		returnVal = false;
	applyForcesKernel = dxFunctions.compileComputeShaderFromString( ApplyForcesHLSLString, "ApplyForcesKernel", sizeof(ApplyForcesCB) );
	if( !applyForcesKernel.constBuffer )
		returnVal = false;
	solveCollisionsAndUpdateVelocitiesKernel = dxFunctions.compileComputeShaderFromString( SolveCollisionsAndUpdateVelocitiesHLSLString, "SolveCollisionsAndUpdateVelocitiesKernel", sizeof(SolveCollisionsAndUpdateVelocitiesCB) );
	if( !solveCollisionsAndUpdateVelocitiesKernel.constBuffer )
		returnVal = false;

	// TODO: Rename to UpdateSoftBodies
	resetNormalsAndAreasKernel = dxFunctions.compileComputeShaderFromString( UpdateNormalsHLSLString, "ResetNormalsAndAreasKernel", sizeof(UpdateSoftBodiesCB) );
	if( !resetNormalsAndAreasKernel.constBuffer )
		returnVal = false;
	normalizeNormalsAndAreasKernel = dxFunctions.compileComputeShaderFromString( UpdateNormalsHLSLString, "NormalizeNormalsAndAreasKernel", sizeof(UpdateSoftBodiesCB) );
	if( !normalizeNormalsAndAreasKernel.constBuffer )
		returnVal = false;
	updateSoftBodiesKernel = dxFunctions.compileComputeShaderFromString( UpdateNormalsHLSLString, "UpdateSoftBodiesKernel", sizeof(UpdateSoftBodiesCB) );
	if( !updateSoftBodiesKernel.constBuffer )
		returnVal = false;

	computeBoundsKernel = dxFunctions.compileComputeShaderFromString( ComputeBoundsHLSLString, "ComputeBoundsKernel", sizeof(ComputeBoundsCB) );
	if( !computeBoundsKernel.constBuffer )
		returnVal = false;



	if( returnVal )
		m_shadersInitialized = true;

	return returnVal;
}


static Vectormath::Aos::Transform3 toTransform3( const btTransform &transform )
{
	Vectormath::Aos::Transform3 outTransform;
	outTransform.setCol(0, toVector3(transform.getBasis().getColumn(0)));
	outTransform.setCol(1, toVector3(transform.getBasis().getColumn(1)));
	outTransform.setCol(2, toVector3(transform.getBasis().getColumn(2)));
	outTransform.setCol(3, toVector3(transform.getOrigin()));
	return outTransform;	
}


void btDX11SoftBodySolver::btAcceleratedSoftBodyInterface::updateBounds( const btVector3 &lowerBound, const btVector3 &upperBound )
{
	float scalarMargin = this->getSoftBody()->getCollisionShape()->getMargin();
	btVector3 vectorMargin( scalarMargin, scalarMargin, scalarMargin );
	m_softBody->m_bounds[0] = lowerBound - vectorMargin;
	m_softBody->m_bounds[1] = upperBound + vectorMargin;
}

void btDX11SoftBodySolver::processCollision( btSoftBody*, btSoftBody* )
{

}

// Add the collision object to the set to deal with for a particular soft body
void btDX11SoftBodySolver::processCollision( btSoftBody *softBody, btCollisionObject* collisionObject )
{
	int softBodyIndex = findSoftBodyIndex( softBody );

	if( softBodyIndex >= 0 )
	{
		btCollisionShape *collisionShape = collisionObject->getCollisionShape();
		float friction = collisionObject->getFriction();
		int shapeType = collisionShape->getShapeType();
		if( shapeType == CAPSULE_SHAPE_PROXYTYPE )
		{
			// Add to the list of expected collision objects
			CollisionShapeDescription newCollisionShapeDescription;
			newCollisionShapeDescription.softBodyIdentifier = softBodyIndex;
			newCollisionShapeDescription.collisionShapeType = shapeType;
			// TODO: May need to transpose this matrix either here or in HLSL
			newCollisionShapeDescription.shapeTransform = toTransform3(collisionObject->getWorldTransform());
			btCapsuleShape *capsule = static_cast<btCapsuleShape*>( collisionShape );
			newCollisionShapeDescription.radius = capsule->getRadius();
			newCollisionShapeDescription.halfHeight = capsule->getHalfHeight();
			newCollisionShapeDescription.margin = capsule->getMargin();
			newCollisionShapeDescription.friction = friction;
			btRigidBody* body = static_cast< btRigidBody* >( collisionObject );
			newCollisionShapeDescription.linearVelocity = toVector3(body->getLinearVelocity());
			newCollisionShapeDescription.angularVelocity = toVector3(body->getAngularVelocity());
			m_collisionObjectDetails.push_back( newCollisionShapeDescription );

		} else {
			btAssert("Unsupported collision shape type\n");
		}
	} else {
		btAssert("Unknown soft body");
	}
} // btDX11SoftBodySolver::processCollision



void btDX11SoftBodySolver::predictMotion( float timeStep )
{
	// Clear the collision shape array for the next frame
	// Ensure that the DX11 ones are moved off the device so they will be updated correctly
	m_dx11CollisionObjectDetails.changedOnCPU();
	m_dx11PerClothCollisionObjects.changedOnCPU();
	m_collisionObjectDetails.clear();

	// Fill the force arrays with current acceleration data etc
	m_perClothWindVelocity.resize( m_softBodySet.size() );
	for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
	{
		btSoftBody *softBody = m_softBodySet[softBodyIndex]->getSoftBody();
		
		m_perClothWindVelocity[softBodyIndex] = toVector3(softBody->getWindVelocity());
	}
	m_dx11PerClothWindVelocity.changedOnCPU();

	// Apply forces that we know about to the cloths
	applyForces(  timeStep * getTimeScale() );

	// Itegrate motion for all soft bodies dealt with by the solver
	integrate( timeStep * getTimeScale() );

	// Update bounds
	// Will update the bounds for all softBodies being dealt with by the solver and 
	// set the values in the btSoftBody object
	updateBounds();

	// End prediction work for solvers
}

