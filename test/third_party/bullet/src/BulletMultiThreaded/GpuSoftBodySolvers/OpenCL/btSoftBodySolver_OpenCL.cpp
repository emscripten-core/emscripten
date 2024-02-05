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
#include <stdio.h> //@todo: remove the debugging printf at some stage
#include "btSoftBodySolver_OpenCL.h"
#include "BulletSoftBody/btSoftBodySolverVertexBuffer.h"
#include "BulletSoftBody/btSoftBody.h"
#include "BulletCollision/CollisionShapes/btCapsuleShape.h"
#include "LinearMath/btQuickprof.h"
#include <limits.h>

#ifdef USE_MINICL
	#include "MiniCL/cl.h"
#else //USE_MINICL
	#ifdef __APPLE__
		#include <OpenCL/OpenCL.h>
	#else
		#include <CL/cl.h>
	#endif //__APPLE__
#endif//USE_MINICL

#define BT_DEFAULT_WORKGROUPSIZE 128



#define RELEASE_CL_KERNEL(kernelName) {if( kernelName ){ clReleaseKernel( kernelName ); kernelName = 0; }}


//CL_VERSION_1_1 seems broken on NVidia SDK so just disable it

#if (0)//CL_VERSION_1_1 == 1)
 //OpenCL 1.1 kernels use float3
#define MSTRINGIFY(A) #A
static const char* PrepareLinksCLString = 
#include "OpenCLC/PrepareLinks.cl"
static const char* UpdatePositionsFromVelocitiesCLString = 
#include "OpenCLC/UpdatePositionsFromVelocities.cl"
static const char* SolvePositionsCLString = 
#include "OpenCLC/SolvePositions.cl"
static const char* UpdateNodesCLString = 
#include "OpenCLC/UpdateNodes.cl"
static const char* UpdatePositionsCLString = 
#include "OpenCLC/UpdatePositions.cl"
static const char* UpdateConstantsCLString = 
#include "OpenCLC/UpdateConstants.cl"
static const char* IntegrateCLString = 
#include "OpenCLC/Integrate.cl"
static const char* ApplyForcesCLString = 
#include "OpenCLC/ApplyForces.cl"
static const char* UpdateNormalsCLString = 
#include "OpenCLC/UpdateNormals.cl"
static const char* VSolveLinksCLString = 
#include "OpenCLC/VSolveLinks.cl"
static const char* SolveCollisionsAndUpdateVelocitiesCLString =
#include "OpenCLC/SolveCollisionsAndUpdateVelocities.cl"
#else
////OpenCL 1.0 kernels don't use float3
#define MSTRINGIFY(A) #A
static const char* PrepareLinksCLString = 
#include "OpenCLC10/PrepareLinks.cl"
static const char* UpdatePositionsFromVelocitiesCLString = 
#include "OpenCLC10/UpdatePositionsFromVelocities.cl"
static const char* SolvePositionsCLString = 
#include "OpenCLC10/SolvePositions.cl"
static const char* UpdateNodesCLString = 
#include "OpenCLC10/UpdateNodes.cl"
static const char* UpdatePositionsCLString = 
#include "OpenCLC10/UpdatePositions.cl"
static const char* UpdateConstantsCLString = 
#include "OpenCLC10/UpdateConstants.cl"
static const char* IntegrateCLString = 
#include "OpenCLC10/Integrate.cl"
static const char* ApplyForcesCLString = 
#include "OpenCLC10/ApplyForces.cl"
static const char* UpdateNormalsCLString = 
#include "OpenCLC10/UpdateNormals.cl"
static const char* VSolveLinksCLString = 
#include "OpenCLC10/VSolveLinks.cl"
static const char* SolveCollisionsAndUpdateVelocitiesCLString =
#include "OpenCLC10/SolveCollisionsAndUpdateVelocities.cl"
#endif //CL_VERSION_1_1


btSoftBodyVertexDataOpenCL::btSoftBodyVertexDataOpenCL( cl_command_queue queue, cl_context ctx) :
    m_queue(queue),
	m_clClothIdentifier( queue, ctx, &m_clothIdentifier, false ),
	m_clVertexPosition( queue, ctx, &m_vertexPosition, false ),
	m_clVertexPreviousPosition( queue, ctx, &m_vertexPreviousPosition, false ),
	m_clVertexVelocity( queue, ctx, &m_vertexVelocity, false ),
	m_clVertexForceAccumulator( queue, ctx, &m_vertexForceAccumulator, false ),
	m_clVertexNormal( queue, ctx, &m_vertexNormal, false ),
	m_clVertexInverseMass( queue, ctx, &m_vertexInverseMass, false ),
	m_clVertexArea( queue, ctx, &m_vertexArea, false ),
	m_clVertexTriangleCount( queue, ctx, &m_vertexTriangleCount, false )
{
}

btSoftBodyVertexDataOpenCL::~btSoftBodyVertexDataOpenCL()
{

}

bool btSoftBodyVertexDataOpenCL::onAccelerator()
{
	return m_onGPU;
}

bool btSoftBodyVertexDataOpenCL::moveToAccelerator()
{
	bool success = true;
	success = success && m_clClothIdentifier.moveToGPU();
	success = success && m_clVertexPosition.moveToGPU();
	success = success && m_clVertexPreviousPosition.moveToGPU();
	success = success && m_clVertexVelocity.moveToGPU();
	success = success && m_clVertexForceAccumulator.moveToGPU();
	success = success && m_clVertexNormal.moveToGPU();
	success = success && m_clVertexInverseMass.moveToGPU();
	success = success && m_clVertexArea.moveToGPU();
	success = success && m_clVertexTriangleCount.moveToGPU();

	if( success )
		m_onGPU = true;

	return success;
}

bool btSoftBodyVertexDataOpenCL::moveFromAccelerator()
{
	bool success = true;
	success = success && m_clClothIdentifier.moveFromGPU();
	success = success && m_clVertexPosition.moveFromGPU();
	success = success && m_clVertexPreviousPosition.moveFromGPU();
	success = success && m_clVertexVelocity.moveFromGPU();
	success = success && m_clVertexForceAccumulator.moveFromGPU();
	success = success && m_clVertexNormal.moveFromGPU();
	success = success && m_clVertexInverseMass.moveFromGPU();
	success = success && m_clVertexArea.moveFromGPU();
	success = success && m_clVertexTriangleCount.moveFromGPU();

	if( success )
		m_onGPU = true;

	return success;
}




btSoftBodyLinkDataOpenCL::btSoftBodyLinkDataOpenCL(cl_command_queue queue,  cl_context ctx) 
:m_cqCommandQue(queue),
	m_clLinks( queue, ctx, &m_links, false ),
	m_clLinkStrength( queue, ctx, &m_linkStrength, false ),
	m_clLinksMassLSC( queue, ctx, &m_linksMassLSC, false ),
	m_clLinksRestLengthSquared( queue, ctx, &m_linksRestLengthSquared, false ),
	m_clLinksCLength( queue, ctx, &m_linksCLength, false ),
	m_clLinksLengthRatio( queue, ctx, &m_linksLengthRatio, false ),
	m_clLinksRestLength( queue, ctx, &m_linksRestLength, false ),
	m_clLinksMaterialLinearStiffnessCoefficient( queue, ctx, &m_linksMaterialLinearStiffnessCoefficient, false )
{
}

btSoftBodyLinkDataOpenCL::~btSoftBodyLinkDataOpenCL()
{
}

static Vectormath::Aos::Vector3 toVector3( const btVector3 &vec )
{
	Vectormath::Aos::Vector3 outVec( vec.getX(), vec.getY(), vec.getZ() );
	return outVec;
}

/** Allocate enough space in all link-related arrays to fit numLinks links */
void btSoftBodyLinkDataOpenCL::createLinks( int numLinks )
{
	int previousSize = m_links.size();
	int newSize = previousSize + numLinks;

	btSoftBodyLinkData::createLinks( numLinks );

	// Resize the link addresses array as well
	m_linkAddresses.resize( newSize );
}

/** Insert the link described into the correct data structures assuming space has already been allocated by a call to createLinks */
void btSoftBodyLinkDataOpenCL::setLinkAt( 
	const LinkDescription &link, 
	int linkIndex )
{
	btSoftBodyLinkData::setLinkAt( link, linkIndex );

	// Set the link index correctly for initialisation
	m_linkAddresses[linkIndex] = linkIndex;
}

bool btSoftBodyLinkDataOpenCL::onAccelerator()
{
	return m_onGPU;
}

bool btSoftBodyLinkDataOpenCL::moveToAccelerator()
{
	bool success = true;
	success = success && m_clLinks.moveToGPU();
	success = success && m_clLinkStrength.moveToGPU();
	success = success && m_clLinksMassLSC.moveToGPU();
	success = success && m_clLinksRestLengthSquared.moveToGPU();
	success = success && m_clLinksCLength.moveToGPU();
	success = success && m_clLinksLengthRatio.moveToGPU();
	success = success && m_clLinksRestLength.moveToGPU();
	success = success && m_clLinksMaterialLinearStiffnessCoefficient.moveToGPU();

	if( success ) {
		m_onGPU = true;
	}

	return success;
}

bool btSoftBodyLinkDataOpenCL::moveFromAccelerator()
{
	bool success = true;
	success = success && m_clLinks.moveFromGPU();
	success = success && m_clLinkStrength.moveFromGPU();
	success = success && m_clLinksMassLSC.moveFromGPU();
	success = success && m_clLinksRestLengthSquared.moveFromGPU();
	success = success && m_clLinksCLength.moveFromGPU();
	success = success && m_clLinksLengthRatio.moveFromGPU();
	success = success && m_clLinksRestLength.moveFromGPU();
	success = success && m_clLinksMaterialLinearStiffnessCoefficient.moveFromGPU();

	if( success ) {
		m_onGPU = false;
	}

	return success;
}

/**
 * Generate (and later update) the batching for the entire link set.
 * This redoes a lot of work because it batches the entire set when each cloth is inserted.
 * In theory we could delay it until just before we need the cloth.
 * It's a one-off overhead, though, so that is a later optimisation.
 */
void btSoftBodyLinkDataOpenCL::generateBatches()
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
		m_batchStartLengths.resize(batchCounts.size());
		m_batchStartLengths[0] = BatchPair(0, 0);

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
	btAlignedObjectArray<LinkNodePair>									m_links_Backup(m_links);
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


} // void generateBatches()





btSoftBodyTriangleDataOpenCL::btSoftBodyTriangleDataOpenCL( cl_command_queue queue , cl_context ctx) : 
    m_queue( queue ),
	m_clVertexIndices( queue, ctx, &m_vertexIndices, false ),
	m_clArea( queue, ctx, &m_area, false ),
	m_clNormal( queue, ctx, &m_normal, false )
{
}

btSoftBodyTriangleDataOpenCL::~btSoftBodyTriangleDataOpenCL()
{
}

/** Allocate enough space in all link-related arrays to fit numLinks links */
void btSoftBodyTriangleDataOpenCL::createTriangles( int numTriangles )
{
	int previousSize = getNumTriangles();
	int newSize = previousSize + numTriangles;

	btSoftBodyTriangleData::createTriangles( numTriangles );

	// Resize the link addresses array as well
	m_triangleAddresses.resize( newSize );
}

/** Insert the link described into the correct data structures assuming space has already been allocated by a call to createLinks */
void btSoftBodyTriangleDataOpenCL::setTriangleAt( const btSoftBodyTriangleData::TriangleDescription &triangle, int triangleIndex )
{
	btSoftBodyTriangleData::setTriangleAt( triangle, triangleIndex );

	m_triangleAddresses[triangleIndex] = triangleIndex;
}

bool btSoftBodyTriangleDataOpenCL::onAccelerator()
{
	return m_onGPU;
}

bool btSoftBodyTriangleDataOpenCL::moveToAccelerator()
{
	bool success = true;
	success = success && m_clVertexIndices.moveToGPU();
	success = success && m_clArea.moveToGPU();
	success = success && m_clNormal.moveToGPU();

	if( success )
		m_onGPU = true;

	return success;
}

bool btSoftBodyTriangleDataOpenCL::moveFromAccelerator()
{
	bool success = true;
	success = success && m_clVertexIndices.moveFromGPU();
	success = success && m_clArea.moveFromGPU();
	success = success && m_clNormal.moveFromGPU();

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
void btSoftBodyTriangleDataOpenCL::generateBatches()
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
	m_batchStartLengths[0] = btSomePair(0,0);


	int sum = 0;
	for( int batchIndex = 0; batchIndex < batchCounts.size(); ++batchIndex )
	{
		m_batchStartLengths[batchIndex].first = sum;
		m_batchStartLengths[batchIndex].second = batchCounts[batchIndex];
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
		int newLocation = m_batchStartLengths[batch].first + batchCounts[batch];

		batchCounts[batch] = batchCounts[batch] + 1;
		m_vertexIndices[newLocation] = m_vertexIndices_Backup[triangleLocation];
		m_area[newLocation] = m_area_Backup[triangleLocation];
		m_normal[newLocation] = m_normal_Backup[triangleLocation];

		// Update the locations array to account for the moved entry
		m_triangleAddresses[triangleIndex] = newLocation;
	}
} // btSoftBodyTriangleDataOpenCL::generateBatches







btOpenCLSoftBodySolver::btOpenCLSoftBodySolver(cl_command_queue queue, cl_context ctx) :
	m_linkData(queue, ctx),
	m_vertexData(queue, ctx),
	m_triangleData(queue, ctx),
	clFunctions(queue, ctx),
	m_clPerClothAcceleration(queue, ctx, &m_perClothAcceleration, true ),
	m_clPerClothWindVelocity(queue, ctx, &m_perClothWindVelocity, true ),
	m_clPerClothDampingFactor(queue,ctx, &m_perClothDampingFactor, true ),
	m_clPerClothVelocityCorrectionCoefficient(queue, ctx,&m_perClothVelocityCorrectionCoefficient, true ),
	m_clPerClothLiftFactor(queue, ctx,&m_perClothLiftFactor, true ),
	m_clPerClothDragFactor(queue, ctx,&m_perClothDragFactor, true ),
	m_clPerClothMediumDensity(queue, ctx,&m_perClothMediumDensity, true ),
	m_clPerClothCollisionObjects( queue, ctx, &m_perClothCollisionObjects, true ),
	m_clCollisionObjectDetails( queue, ctx, &m_collisionObjectDetails, true ),
	m_clPerClothFriction( queue, ctx, &m_perClothFriction, false ),
	m_cqCommandQue( queue ),
	m_cxMainContext(ctx),
	m_defaultWorkGroupSize(BT_DEFAULT_WORKGROUPSIZE)
{
	// Initial we will clearly need to update solver constants
	// For now this is global for the cloths linked with this solver - we should probably make this body specific 
	// for performance in future once we understand more clearly when constants need to be updated
	m_updateSolverConstants = true;

	m_shadersInitialized = false;

	prepareLinksKernel = 0;
	solvePositionsFromLinksKernel = 0;
	updateConstantsKernel = 0;
	integrateKernel = 0;
	addVelocityKernel = 0;
	updatePositionsFromVelocitiesKernel = 0;
	updateVelocitiesFromPositionsWithoutVelocitiesKernel = 0;
	updateVelocitiesFromPositionsWithVelocitiesKernel = 0;
	vSolveLinksKernel = 0;
	solveCollisionsAndUpdateVelocitiesKernel = 0;
	resetNormalsAndAreasKernel = 0;
	resetNormalsAndAreasKernel = 0;
	normalizeNormalsAndAreasKernel = 0;
	outputToVertexArrayKernel = 0;
	applyForcesKernel = 0;
}

btOpenCLSoftBodySolver::~btOpenCLSoftBodySolver()
{
	releaseKernels();
}

void btOpenCLSoftBodySolver::releaseKernels()
{
	RELEASE_CL_KERNEL( prepareLinksKernel );
	RELEASE_CL_KERNEL( solvePositionsFromLinksKernel );
	RELEASE_CL_KERNEL( updateConstantsKernel );
	RELEASE_CL_KERNEL( integrateKernel );
	RELEASE_CL_KERNEL( addVelocityKernel );
	RELEASE_CL_KERNEL( updatePositionsFromVelocitiesKernel );
	RELEASE_CL_KERNEL( updateVelocitiesFromPositionsWithoutVelocitiesKernel );
	RELEASE_CL_KERNEL( updateVelocitiesFromPositionsWithVelocitiesKernel );
	RELEASE_CL_KERNEL( vSolveLinksKernel );
	RELEASE_CL_KERNEL( solveCollisionsAndUpdateVelocitiesKernel );
	RELEASE_CL_KERNEL( resetNormalsAndAreasKernel );
	RELEASE_CL_KERNEL( normalizeNormalsAndAreasKernel );
	RELEASE_CL_KERNEL( outputToVertexArrayKernel );
	RELEASE_CL_KERNEL( applyForcesKernel );

	m_shadersInitialized = false;
}

void btOpenCLSoftBodySolver::copyBackToSoftBodies()
{
	// Move the vertex data back to the host first
	m_vertexData.moveFromAccelerator();

	// Loop over soft bodies, copying all the vertex positions back for each body in turn
	for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
	{
		btOpenCLAcceleratedSoftBodyInterface *softBodyInterface = m_softBodySet[ softBodyIndex ];
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
} // btOpenCLSoftBodySolver::copyBackToSoftBodies

void btOpenCLSoftBodySolver::optimize( btAlignedObjectArray< btSoftBody * > &softBodies, bool forceUpdate )
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
			btOpenCLAcceleratedSoftBodyInterface *newSoftBody = new btOpenCLAcceleratedSoftBodyInterface( softBody );
			m_softBodySet.push_back( newSoftBody );

			m_perClothAcceleration.push_back( toVector3(softBody->getWorldInfo()->m_gravity) );
			m_perClothDampingFactor.push_back(softBody->m_cfg.kDP);
			m_perClothVelocityCorrectionCoefficient.push_back( softBody->m_cfg.kVCF );
			m_perClothLiftFactor.push_back( softBody->m_cfg.kLF );
			m_perClothDragFactor.push_back( softBody->m_cfg.kDG );
			m_perClothMediumDensity.push_back(softBody->getWorldInfo()->air_density);
			// Simple init values. Actually we'll put 0 and -1 into them at the appropriate time
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


btSoftBodyLinkData &btOpenCLSoftBodySolver::getLinkData()
{
	// TODO: Consider setting link data to "changed" here
	return m_linkData;
}

btSoftBodyVertexData &btOpenCLSoftBodySolver::getVertexData()
{
	// TODO: Consider setting vertex data to "changed" here
	return m_vertexData;
}

btSoftBodyTriangleData &btOpenCLSoftBodySolver::getTriangleData()
{
	// TODO: Consider setting triangle data to "changed" here
	return m_triangleData;
}

void btOpenCLSoftBodySolver::resetNormalsAndAreas( int numVertices )
{
	cl_int ciErrNum;
	ciErrNum = clSetKernelArg(resetNormalsAndAreasKernel, 0, sizeof(numVertices), (void*)&numVertices); //oclCHECKERROR(ciErrNum, CL_SUCCESS);
	ciErrNum = clSetKernelArg(resetNormalsAndAreasKernel, 1, sizeof(cl_mem), (void*)&m_vertexData.m_clVertexNormal.m_buffer);//oclCHECKERROR(ciErrNum, CL_SUCCESS);
	ciErrNum = clSetKernelArg(resetNormalsAndAreasKernel,  2, sizeof(cl_mem), (void*)&m_vertexData.m_clVertexArea.m_buffer); //oclCHECKERROR(ciErrNum, CL_SUCCESS);
	size_t numWorkItems = m_defaultWorkGroupSize*((numVertices + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);

	if (numWorkItems)
	{
		ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue, resetNormalsAndAreasKernel, 1, NULL, &numWorkItems, &m_defaultWorkGroupSize, 0,0,0 );

		if( ciErrNum != CL_SUCCESS )
		{
			btAssert( 0 && "enqueueNDRangeKernel(resetNormalsAndAreasKernel)" );
		}
	}

}

void btOpenCLSoftBodySolver::normalizeNormalsAndAreas( int numVertices )
{

	cl_int ciErrNum;

	ciErrNum = clSetKernelArg(normalizeNormalsAndAreasKernel, 0, sizeof(int),(void*) &numVertices);
	ciErrNum = clSetKernelArg(normalizeNormalsAndAreasKernel, 1, sizeof(cl_mem), &m_vertexData.m_clVertexTriangleCount.m_buffer);
	ciErrNum = clSetKernelArg(normalizeNormalsAndAreasKernel, 2, sizeof(cl_mem), &m_vertexData.m_clVertexNormal.m_buffer);
	ciErrNum = clSetKernelArg(normalizeNormalsAndAreasKernel, 3, sizeof(cl_mem), &m_vertexData.m_clVertexArea.m_buffer);
	size_t	numWorkItems = m_defaultWorkGroupSize*((numVertices + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);
	if (numWorkItems)
	{
		ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue, normalizeNormalsAndAreasKernel, 1, NULL, &numWorkItems, &m_defaultWorkGroupSize, 0,0,0);
		if( ciErrNum != CL_SUCCESS ) 
		{
			btAssert( 0 && "enqueueNDRangeKernel(normalizeNormalsAndAreasKernel)");
		}
	}

}

void btOpenCLSoftBodySolver::executeUpdateSoftBodies( int firstTriangle, int numTriangles )
{

	cl_int ciErrNum;
	ciErrNum = clSetKernelArg(updateSoftBodiesKernel, 0, sizeof(int), (void*) &firstTriangle);
	ciErrNum = clSetKernelArg(updateSoftBodiesKernel, 1, sizeof(int), &numTriangles);
	ciErrNum = clSetKernelArg(updateSoftBodiesKernel, 2, sizeof(cl_mem), &m_triangleData.m_clVertexIndices.m_buffer);
	ciErrNum = clSetKernelArg(updateSoftBodiesKernel, 3, sizeof(cl_mem), &m_vertexData.m_clVertexPosition.m_buffer);
	ciErrNum = clSetKernelArg(updateSoftBodiesKernel, 4, sizeof(cl_mem), &m_vertexData.m_clVertexNormal.m_buffer);
	ciErrNum = clSetKernelArg(updateSoftBodiesKernel, 5, sizeof(cl_mem), &m_vertexData.m_clVertexArea.m_buffer);
	ciErrNum = clSetKernelArg(updateSoftBodiesKernel, 6, sizeof(cl_mem), &m_triangleData.m_clNormal.m_buffer);
	ciErrNum = clSetKernelArg(updateSoftBodiesKernel, 7, sizeof(cl_mem), &m_triangleData.m_clArea.m_buffer);

	size_t numWorkItems = m_defaultWorkGroupSize*((numTriangles + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);
	ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue, updateSoftBodiesKernel, 1, NULL, &numWorkItems, &m_defaultWorkGroupSize,0,0,0);
	if( ciErrNum != CL_SUCCESS ) 
	{
		btAssert( 0 &&  "enqueueNDRangeKernel(normalizeNormalsAndAreasKernel)");
	}

}

void btOpenCLSoftBodySolver::updateSoftBodies()
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

		int startTriangle = m_triangleData.m_batchStartLengths[batchIndex].first;
		int numTriangles = m_triangleData.m_batchStartLengths[batchIndex].second;

		executeUpdateSoftBodies( startTriangle, numTriangles );
	}


	normalizeNormalsAndAreas( numVertices );
} // updateSoftBodies


Vectormath::Aos::Vector3 btOpenCLSoftBodySolver::ProjectOnAxis( const Vectormath::Aos::Vector3 &v, const Vectormath::Aos::Vector3 &a )
{
	return a*Vectormath::Aos::dot(v, a);
}

void btOpenCLSoftBodySolver::ApplyClampedForce( float solverdt, const Vectormath::Aos::Vector3 &force, const Vectormath::Aos::Vector3 &vertexVelocity, float inverseMass, Vectormath::Aos::Vector3 &vertexForce )
{
	float dtInverseMass = solverdt*inverseMass;
	if( Vectormath::Aos::lengthSqr(force * dtInverseMass) > Vectormath::Aos::lengthSqr(vertexVelocity) )
	{
		vertexForce -= ProjectOnAxis( vertexVelocity, normalize( force ) )/dtInverseMass;
	} else {
		vertexForce += force;
	}
}

void btOpenCLSoftBodySolver::applyForces( float solverdt )
{	

	// Ensure data is on accelerator
	m_vertexData.moveToAccelerator();
	m_clPerClothAcceleration.moveToGPU();
	m_clPerClothLiftFactor.moveToGPU();
	m_clPerClothDragFactor.moveToGPU();
	m_clPerClothMediumDensity.moveToGPU();
	m_clPerClothWindVelocity.moveToGPU();			

	cl_int ciErrNum ;
	int numVerts = m_vertexData.getNumVertices();
	ciErrNum = clSetKernelArg(applyForcesKernel, 0, sizeof(int), &numVerts);
	ciErrNum = clSetKernelArg(applyForcesKernel, 1, sizeof(float), &solverdt);
	float fl = FLT_EPSILON;
	ciErrNum = clSetKernelArg(applyForcesKernel, 2, sizeof(float), &fl);
	ciErrNum = clSetKernelArg(applyForcesKernel, 3, sizeof(cl_mem), &m_vertexData.m_clClothIdentifier.m_buffer);
	ciErrNum = clSetKernelArg(applyForcesKernel, 4, sizeof(cl_mem), &m_vertexData.m_clVertexNormal.m_buffer);
	ciErrNum = clSetKernelArg(applyForcesKernel, 5, sizeof(cl_mem), &m_vertexData.m_clVertexArea.m_buffer);
	ciErrNum = clSetKernelArg(applyForcesKernel, 6, sizeof(cl_mem), &m_vertexData.m_clVertexInverseMass.m_buffer);
	ciErrNum = clSetKernelArg(applyForcesKernel, 7, sizeof(cl_mem), &m_clPerClothLiftFactor.m_buffer);
	ciErrNum = clSetKernelArg(applyForcesKernel, 8 ,sizeof(cl_mem), &m_clPerClothDragFactor.m_buffer);
	ciErrNum = clSetKernelArg(applyForcesKernel, 9, sizeof(cl_mem), &m_clPerClothWindVelocity.m_buffer);
	ciErrNum = clSetKernelArg(applyForcesKernel,10, sizeof(cl_mem), &m_clPerClothAcceleration.m_buffer);
	ciErrNum = clSetKernelArg(applyForcesKernel,11, sizeof(cl_mem), &m_clPerClothMediumDensity.m_buffer);
	ciErrNum = clSetKernelArg(applyForcesKernel,12, sizeof(cl_mem), &m_vertexData.m_clVertexForceAccumulator.m_buffer);
	ciErrNum = clSetKernelArg(applyForcesKernel,13, sizeof(cl_mem), &m_vertexData.m_clVertexVelocity.m_buffer);
	size_t numWorkItems = m_defaultWorkGroupSize*((m_vertexData.getNumVertices() + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);
	if (numWorkItems)
	{
		ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue,applyForcesKernel, 1, NULL, &numWorkItems, &m_defaultWorkGroupSize, 0,0,0);
		if( ciErrNum != CL_SUCCESS ) 
		{
			btAssert( 0 &&  "enqueueNDRangeKernel(applyForcesKernel)");
		}
	}

}

/**
 * Integrate motion on the solver.
 */
void btOpenCLSoftBodySolver::integrate( float solverdt )
{
	

	// Ensure data is on accelerator
	m_vertexData.moveToAccelerator();

	cl_int ciErrNum;
	int numVerts = m_vertexData.getNumVertices();
	ciErrNum = clSetKernelArg(integrateKernel, 0, sizeof(int), &numVerts);
	ciErrNum = clSetKernelArg(integrateKernel, 1, sizeof(float), &solverdt);
	ciErrNum = clSetKernelArg(integrateKernel, 2, sizeof(cl_mem), &m_vertexData.m_clVertexInverseMass.m_buffer);
	ciErrNum = clSetKernelArg(integrateKernel, 3, sizeof(cl_mem), &m_vertexData.m_clVertexPosition.m_buffer);
	ciErrNum = clSetKernelArg(integrateKernel, 4, sizeof(cl_mem), &m_vertexData.m_clVertexVelocity.m_buffer);
	ciErrNum = clSetKernelArg(integrateKernel, 5, sizeof(cl_mem), &m_vertexData.m_clVertexPreviousPosition.m_buffer);
	ciErrNum = clSetKernelArg(integrateKernel, 6, sizeof(cl_mem), &m_vertexData.m_clVertexForceAccumulator.m_buffer);

	size_t numWorkItems = m_defaultWorkGroupSize*((m_vertexData.getNumVertices() + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);
	if (numWorkItems)
	{
		ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue,integrateKernel, 1, NULL, &numWorkItems, &m_defaultWorkGroupSize,0,0,0);
		if( ciErrNum != CL_SUCCESS )
		{
			btAssert( 0 &&  "enqueueNDRangeKernel(integrateKernel)");
		}
	}

}

float btOpenCLSoftBodySolver::computeTriangleArea( 
	const Vectormath::Aos::Point3 &vertex0,
	const Vectormath::Aos::Point3 &vertex1,
	const Vectormath::Aos::Point3 &vertex2 )
{
	Vectormath::Aos::Vector3 a = vertex1 - vertex0;
	Vectormath::Aos::Vector3 b = vertex2 - vertex0;
	Vectormath::Aos::Vector3 crossProduct = cross(a, b);
	float area = length( crossProduct );
	return area;
}


void btOpenCLSoftBodySolver::updateBounds()
{	
	for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
	{
		btVector3 minBound(-1e30,-1e30,-1e30), maxBound(1e30,1e30,1e30);
		m_softBodySet[softBodyIndex]->updateBounds( minBound, maxBound );
	}

} // btOpenCLSoftBodySolver::updateBounds


void btOpenCLSoftBodySolver::updateConstants( float timeStep )
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

}

class QuickSortCompare
{
	public:

	bool operator() ( const CollisionShapeDescription& a, const CollisionShapeDescription& b )
	{
		return ( a.softBodyIdentifier < b.softBodyIdentifier );
	}
};


/**
 * Sort the collision object details array and generate indexing into it for the per-cloth collision object array.
 */
void btOpenCLSoftBodySolver::prepareCollisionConstraints()
{
	// First do a simple sort on the collision objects
	btAlignedObjectArray<int> numObjectsPerClothPrefixSum;
	btAlignedObjectArray<int> numObjectsPerCloth;
	numObjectsPerCloth.resize( m_softBodySet.size(), 0 );
	numObjectsPerClothPrefixSum.resize( m_softBodySet.size(), 0 );


	
	m_collisionObjectDetails.quickSort( QuickSortCompare() );

	if (!m_perClothCollisionObjects.size())
		return;

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
	
} // btOpenCLSoftBodySolver::prepareCollisionConstraints



void btOpenCLSoftBodySolver::solveConstraints( float solverdt )
{

	using Vectormath::Aos::Vector3;
	using Vectormath::Aos::Point3;
	using Vectormath::Aos::lengthSqr;
	using Vectormath::Aos::dot;

	// Prepare links
	int numLinks = m_linkData.getNumLinks();
	int numVertices = m_vertexData.getNumVertices();

	float kst = 1.f;
	float ti = 0.f;


	m_clPerClothDampingFactor.moveToGPU();
	m_clPerClothVelocityCorrectionCoefficient.moveToGPU();


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

}


//////////////////////////////////////
// Kernel dispatches
void btOpenCLSoftBodySolver::prepareLinks()
{

	cl_int ciErrNum;
	int numLinks = m_linkData.getNumLinks();
	ciErrNum = clSetKernelArg(prepareLinksKernel,0, sizeof(int), &numLinks);
	ciErrNum = clSetKernelArg(prepareLinksKernel,1, sizeof(cl_mem), &m_linkData.m_clLinks.m_buffer);
	ciErrNum = clSetKernelArg(prepareLinksKernel,2, sizeof(cl_mem), &m_linkData.m_clLinksMassLSC.m_buffer);
	ciErrNum = clSetKernelArg(prepareLinksKernel,3, sizeof(cl_mem), &m_vertexData.m_clVertexPreviousPosition.m_buffer);
	ciErrNum = clSetKernelArg(prepareLinksKernel,4, sizeof(cl_mem), &m_linkData.m_clLinksLengthRatio.m_buffer);
	ciErrNum = clSetKernelArg(prepareLinksKernel,5, sizeof(cl_mem), &m_linkData.m_clLinksCLength.m_buffer);

	size_t	numWorkItems = m_defaultWorkGroupSize*((m_linkData.getNumLinks() + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);
	ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue,prepareLinksKernel, 1 , NULL, &numWorkItems, &m_defaultWorkGroupSize,0,0,0);
	if( ciErrNum != CL_SUCCESS ) 
	{
		btAssert( 0 &&  "enqueueNDRangeKernel(prepareLinksKernel)");
	}

}

void btOpenCLSoftBodySolver::updatePositionsFromVelocities( float solverdt )
{

	cl_int ciErrNum;
	int numVerts = m_vertexData.getNumVertices();
	ciErrNum = clSetKernelArg(updatePositionsFromVelocitiesKernel,0, sizeof(int), &numVerts);
	ciErrNum = clSetKernelArg(updatePositionsFromVelocitiesKernel,1, sizeof(float), &solverdt);
	ciErrNum = clSetKernelArg(updatePositionsFromVelocitiesKernel,2, sizeof(cl_mem), &m_vertexData.m_clVertexVelocity.m_buffer);
	ciErrNum = clSetKernelArg(updatePositionsFromVelocitiesKernel,3, sizeof(cl_mem), &m_vertexData.m_clVertexPreviousPosition.m_buffer);
	ciErrNum = clSetKernelArg(updatePositionsFromVelocitiesKernel,4, sizeof(cl_mem), &m_vertexData.m_clVertexPosition.m_buffer);

	size_t	numWorkItems = m_defaultWorkGroupSize*((m_vertexData.getNumVertices() + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);
	ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue,updatePositionsFromVelocitiesKernel, 1, NULL, &numWorkItems,&m_defaultWorkGroupSize,0,0,0);
	if( ciErrNum != CL_SUCCESS ) 
	{
		btAssert( 0 &&  "enqueueNDRangeKernel(updatePositionsFromVelocitiesKernel)");
	}

}

void btOpenCLSoftBodySolver::solveLinksForPosition( int startLink, int numLinks, float kst, float ti )
{

	cl_int ciErrNum;
	ciErrNum = clSetKernelArg(solvePositionsFromLinksKernel,0, sizeof(int), &startLink);
	ciErrNum = clSetKernelArg(solvePositionsFromLinksKernel,1, sizeof(int), &numLinks);
	ciErrNum = clSetKernelArg(solvePositionsFromLinksKernel,2, sizeof(float), &kst);
	ciErrNum = clSetKernelArg(solvePositionsFromLinksKernel,3, sizeof(float), &ti);
	ciErrNum = clSetKernelArg(solvePositionsFromLinksKernel,4, sizeof(cl_mem), &m_linkData.m_clLinks.m_buffer);
	ciErrNum = clSetKernelArg(solvePositionsFromLinksKernel,5, sizeof(cl_mem), &m_linkData.m_clLinksMassLSC.m_buffer);
	ciErrNum = clSetKernelArg(solvePositionsFromLinksKernel,6, sizeof(cl_mem), &m_linkData.m_clLinksRestLengthSquared.m_buffer);
	ciErrNum = clSetKernelArg(solvePositionsFromLinksKernel,7, sizeof(cl_mem), &m_vertexData.m_clVertexInverseMass.m_buffer);
	ciErrNum = clSetKernelArg(solvePositionsFromLinksKernel,8, sizeof(cl_mem), &m_vertexData.m_clVertexPosition.m_buffer);

	size_t	numWorkItems = m_defaultWorkGroupSize*((numLinks + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);
	ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue,solvePositionsFromLinksKernel,1,NULL,&numWorkItems,&m_defaultWorkGroupSize,0,0,0);
	if( ciErrNum!= CL_SUCCESS ) 
	{
		btAssert( 0 &&  "enqueueNDRangeKernel(solvePositionsFromLinksKernel)");
	}

} // solveLinksForPosition


void btOpenCLSoftBodySolver::solveLinksForVelocity( int startLink, int numLinks, float kst )
{

	cl_int ciErrNum;
	ciErrNum = clSetKernelArg(vSolveLinksKernel, 0, sizeof(int), &startLink);
	ciErrNum = clSetKernelArg(vSolveLinksKernel, 1, sizeof(int), &numLinks);
	ciErrNum = clSetKernelArg(vSolveLinksKernel, 2, sizeof(cl_mem), &m_linkData.m_clLinks.m_buffer);
	ciErrNum = clSetKernelArg(vSolveLinksKernel, 3, sizeof(cl_mem), &m_linkData.m_clLinksLengthRatio.m_buffer);
	ciErrNum = clSetKernelArg(vSolveLinksKernel, 4, sizeof(cl_mem), &m_linkData.m_clLinksCLength.m_buffer);
	ciErrNum = clSetKernelArg(vSolveLinksKernel, 5, sizeof(cl_mem), &m_vertexData.m_clVertexInverseMass.m_buffer);
	ciErrNum = clSetKernelArg(vSolveLinksKernel, 6, sizeof(cl_mem), &m_vertexData.m_clVertexVelocity.m_buffer);

	size_t	numWorkItems = m_defaultWorkGroupSize*((numLinks + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);
	ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue,vSolveLinksKernel,1,NULL,&numWorkItems, &m_defaultWorkGroupSize,0,0,0);
	if( ciErrNum != CL_SUCCESS ) 
	{
		btAssert( 0 &&  "enqueueNDRangeKernel(vSolveLinksKernel)");
	}

}

void btOpenCLSoftBodySolver::updateVelocitiesFromPositionsWithVelocities( float isolverdt )
{

	cl_int ciErrNum;
	int numVerts = m_vertexData.getNumVertices();
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithVelocitiesKernel,0, sizeof(int), &numVerts);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithVelocitiesKernel, 1, sizeof(float), &isolverdt);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithVelocitiesKernel, 2, sizeof(cl_mem), &m_vertexData.m_clVertexPosition.m_buffer);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithVelocitiesKernel, 3, sizeof(cl_mem), &m_vertexData.m_clVertexPreviousPosition.m_buffer);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithVelocitiesKernel, 4, sizeof(cl_mem), &m_vertexData.m_clClothIdentifier.m_buffer);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithVelocitiesKernel, 5, sizeof(cl_mem), &m_clPerClothVelocityCorrectionCoefficient.m_buffer);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithVelocitiesKernel, 6, sizeof(cl_mem), &m_clPerClothDampingFactor.m_buffer);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithVelocitiesKernel, 7, sizeof(cl_mem), &m_vertexData.m_clVertexVelocity.m_buffer);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithVelocitiesKernel, 8, sizeof(cl_mem), &m_vertexData.m_clVertexForceAccumulator.m_buffer);

	size_t	numWorkItems = m_defaultWorkGroupSize*((m_vertexData.getNumVertices() + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);
	ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue,updateVelocitiesFromPositionsWithVelocitiesKernel, 1, NULL, &numWorkItems, &m_defaultWorkGroupSize,0,0,0);
	if( ciErrNum != CL_SUCCESS ) 
	{
		btAssert( 0 &&  "enqueueNDRangeKernel(updateVelocitiesFromPositionsWithVelocitiesKernel)");
	}


} // updateVelocitiesFromPositionsWithVelocities

void btOpenCLSoftBodySolver::updateVelocitiesFromPositionsWithoutVelocities( float isolverdt )
{

	cl_int ciErrNum;
	int numVerts = m_vertexData.getNumVertices();
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithoutVelocitiesKernel, 0, sizeof(int), &numVerts);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithoutVelocitiesKernel, 1, sizeof(float), &isolverdt);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithoutVelocitiesKernel, 2, sizeof(cl_mem),&m_vertexData.m_clVertexPosition.m_buffer);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithoutVelocitiesKernel, 3, sizeof(cl_mem),&m_vertexData.m_clVertexPreviousPosition.m_buffer);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithoutVelocitiesKernel, 4, sizeof(cl_mem),&m_vertexData.m_clClothIdentifier.m_buffer);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithoutVelocitiesKernel, 5, sizeof(cl_mem),&m_clPerClothDampingFactor.m_buffer);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithoutVelocitiesKernel, 6, sizeof(cl_mem),&m_vertexData.m_clVertexVelocity.m_buffer);
	ciErrNum = clSetKernelArg(updateVelocitiesFromPositionsWithoutVelocitiesKernel, 7, sizeof(cl_mem),&m_vertexData.m_clVertexForceAccumulator.m_buffer);

	size_t	numWorkItems = m_defaultWorkGroupSize*((m_vertexData.getNumVertices() + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);
	ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue,updateVelocitiesFromPositionsWithoutVelocitiesKernel, 1, NULL, &numWorkItems, &m_defaultWorkGroupSize,0,0,0);
	if( ciErrNum != CL_SUCCESS ) 
	{
		btAssert( 0 &&  "enqueueNDRangeKernel(updateVelocitiesFromPositionsWithoutVelocitiesKernel)");
	}

} // updateVelocitiesFromPositionsWithoutVelocities



void btOpenCLSoftBodySolver::solveCollisionsAndUpdateVelocities( float isolverdt )
{

	// Copy kernel parameters to GPU
	m_vertexData.moveToAccelerator();
	m_clPerClothFriction.moveToGPU();
	m_clPerClothDampingFactor.moveToGPU();
	m_clPerClothCollisionObjects.moveToGPU();
	m_clCollisionObjectDetails.moveToGPU();


	cl_int ciErrNum;
	int numVerts = m_vertexData.getNumVertices();
	ciErrNum = clSetKernelArg(solveCollisionsAndUpdateVelocitiesKernel, 0, sizeof(int), &numVerts);
	ciErrNum = clSetKernelArg(solveCollisionsAndUpdateVelocitiesKernel, 1, sizeof(int), &isolverdt);
	ciErrNum = clSetKernelArg(solveCollisionsAndUpdateVelocitiesKernel, 2, sizeof(cl_mem),&m_vertexData.m_clClothIdentifier.m_buffer);
	ciErrNum = clSetKernelArg(solveCollisionsAndUpdateVelocitiesKernel, 3, sizeof(cl_mem),&m_vertexData.m_clVertexPreviousPosition.m_buffer);
	ciErrNum = clSetKernelArg(solveCollisionsAndUpdateVelocitiesKernel, 4, sizeof(cl_mem),&m_clPerClothFriction.m_buffer);
	ciErrNum = clSetKernelArg(solveCollisionsAndUpdateVelocitiesKernel, 5, sizeof(cl_mem),&m_clPerClothDampingFactor.m_buffer);
	ciErrNum = clSetKernelArg(solveCollisionsAndUpdateVelocitiesKernel, 6, sizeof(cl_mem),&m_clPerClothCollisionObjects.m_buffer);
	ciErrNum = clSetKernelArg(solveCollisionsAndUpdateVelocitiesKernel, 7, sizeof(cl_mem),&m_clCollisionObjectDetails.m_buffer);
	ciErrNum = clSetKernelArg(solveCollisionsAndUpdateVelocitiesKernel, 8, sizeof(cl_mem),&m_vertexData.m_clVertexForceAccumulator.m_buffer);
	ciErrNum = clSetKernelArg(solveCollisionsAndUpdateVelocitiesKernel, 9, sizeof(cl_mem),&m_vertexData.m_clVertexVelocity.m_buffer);
	ciErrNum = clSetKernelArg(solveCollisionsAndUpdateVelocitiesKernel, 10, sizeof(cl_mem),&m_vertexData.m_clVertexPosition.m_buffer);

	size_t	numWorkItems = m_defaultWorkGroupSize*((m_vertexData.getNumVertices() + (m_defaultWorkGroupSize-1)) / m_defaultWorkGroupSize);
	if (numWorkItems)
	{
		ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue,solveCollisionsAndUpdateVelocitiesKernel, 1, NULL, &numWorkItems, &m_defaultWorkGroupSize,0,0,0);
		if( ciErrNum != CL_SUCCESS ) 
		{
			btAssert( 0 &&  "enqueueNDRangeKernel(updateVelocitiesFromPositionsWithoutVelocitiesKernel)");
		}
	}

} // btOpenCLSoftBodySolver::updateVelocitiesFromPositionsWithoutVelocities



// End kernel dispatches
/////////////////////////////////////


void btSoftBodySolverOutputCLtoCPU::copySoftBodyToVertexBuffer( const btSoftBody * const softBody, btVertexBufferDescriptor *vertexBuffer )
{

	btSoftBodySolver *solver = softBody->getSoftBodySolver();
	btAssert( solver->getSolverType() == btSoftBodySolver::CL_SOLVER || solver->getSolverType() == btSoftBodySolver::CL_SIMD_SOLVER );
	btOpenCLSoftBodySolver *dxSolver = static_cast< btOpenCLSoftBodySolver * >( solver );

	btOpenCLAcceleratedSoftBodyInterface* currentCloth = dxSolver->findSoftBodyInterface( softBody );
	btSoftBodyVertexDataOpenCL &vertexData( dxSolver->m_vertexData );
	

	const int firstVertex = currentCloth->getFirstVertex();
	const int lastVertex = firstVertex + currentCloth->getNumVertices();

	if( vertexBuffer->getBufferType() == btVertexBufferDescriptor::CPU_BUFFER )
	{		
		const btCPUVertexBufferDescriptor *cpuVertexBuffer = static_cast< btCPUVertexBufferDescriptor* >(vertexBuffer);						
		float *basePointer = cpuVertexBuffer->getBasePointer();						

		vertexData.m_clVertexPosition.copyFromGPU();
		vertexData.m_clVertexNormal.copyFromGPU();

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

} // btSoftBodySolverOutputCLtoCPU::outputToVertexBuffers



cl_kernel CLFunctions::compileCLKernelFromString( const char* kernelSource, const char* kernelName, const char* additionalMacros )
{
	printf("compiling kernelName: %s ",kernelName);
	cl_kernel kernel;
	cl_int ciErrNum;
	size_t program_length = strlen(kernelSource);

	cl_program m_cpProgram = clCreateProgramWithSource(m_cxMainContext, 1, (const char**)&kernelSource, &program_length, &ciErrNum);
//	oclCHECKERROR(ciErrNum, CL_SUCCESS);
		
    // Build the program with 'mad' Optimization option

	
#ifdef MAC
	char* flags = "-cl-mad-enable -DMAC -DGUID_ARG";
#else
	//const char* flags = "-DGUID_ARG= -fno-alias";
	const char* flags = "-DGUID_ARG= ";
#endif

	char* compileFlags = new char[strlen(additionalMacros) + strlen(flags) + 5];
	sprintf(compileFlags, "%s %s", flags, additionalMacros);
    ciErrNum = clBuildProgram(m_cpProgram, 0, NULL, compileFlags, NULL, NULL);
    if (ciErrNum != CL_SUCCESS)
    {
		size_t numDevices;
		clGetProgramInfo( m_cpProgram, CL_PROGRAM_DEVICES, 0, 0, &numDevices );
		cl_device_id *devices = new cl_device_id[numDevices];
		clGetProgramInfo( m_cpProgram, CL_PROGRAM_DEVICES, numDevices, devices, &numDevices );
        for( int i = 0; i < 2; ++i )
		{
			char *build_log;
			size_t ret_val_size;
			clGetProgramBuildInfo(m_cpProgram, devices[i], CL_PROGRAM_BUILD_LOG, 0, NULL, &ret_val_size);
			build_log = new char[ret_val_size+1];
			clGetProgramBuildInfo(m_cpProgram, devices[i], CL_PROGRAM_BUILD_LOG, ret_val_size, build_log, NULL);
    
			// to be carefully, terminate with \0
			// there's no information in the reference whether the string is 0 terminated or not
			build_log[ret_val_size] = '\0';
        

			printf("Error in clBuildProgram, Line %u in file %s, Log: \n%s\n !!!\n\n", __LINE__, __FILE__, build_log);
			delete[] build_log;
		}
		btAssert(0);
        exit(0);
    }
	
	
    // Create the kernel
    kernel = clCreateKernel(m_cpProgram, kernelName, &ciErrNum);
    if (ciErrNum != CL_SUCCESS)
    {
        printf("Error in clCreateKernel, Line %u in file %s !!!\n\n", __LINE__, __FILE__);
		btAssert(0);
		exit(0);
    }

	printf("ready. \n");
	delete [] compileFlags;
	return kernel;

}

void btOpenCLSoftBodySolver::predictMotion( float timeStep )
{
	// Clear the collision shape array for the next frame
	// Ensure that the DX11 ones are moved off the device so they will be updated correctly
	m_clCollisionObjectDetails.changedOnCPU();
	m_clPerClothCollisionObjects.changedOnCPU();
	m_collisionObjectDetails.clear();
	
	{
		BT_PROFILE("perClothWindVelocity");
		// Fill the force arrays with current acceleration data etc
		m_perClothWindVelocity.resize( m_softBodySet.size() );
		for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
		{
			btSoftBody *softBody = m_softBodySet[softBodyIndex]->getSoftBody();
			
			m_perClothWindVelocity[softBodyIndex] = toVector3(softBody->getWindVelocity());
		}
	}
	{
		BT_PROFILE("changedOnCPU");
		m_clPerClothWindVelocity.changedOnCPU();
	}

	{
		BT_PROFILE("applyForces");
		// Apply forces that we know about to the cloths
		applyForces(  timeStep * getTimeScale() );
	}

	{
		BT_PROFILE("integrate");
		// Itegrate motion for all soft bodies dealt with by the solver
		integrate( timeStep * getTimeScale() );
	}

	{
		BT_PROFILE("updateBounds");
		updateBounds();
	}
	// End prediction work for solvers
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

void btOpenCLAcceleratedSoftBodyInterface::updateBounds( const btVector3 &lowerBound, const btVector3 &upperBound )
{
	float scalarMargin = this->getSoftBody()->getCollisionShape()->getMargin();
	btVector3 vectorMargin( scalarMargin, scalarMargin, scalarMargin );
	m_softBody->m_bounds[0] = lowerBound - vectorMargin;
	m_softBody->m_bounds[1] = upperBound + vectorMargin;
}  // btOpenCLSoftBodySolver::btDX11AcceleratedSoftBodyInterface::updateBounds

void btOpenCLSoftBodySolver::processCollision( btSoftBody*, btSoftBody* )
{

}

// Add the collision object to the set to deal with for a particular soft body
void btOpenCLSoftBodySolver::processCollision( btSoftBody *softBody, btCollisionObject* collisionObject )
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
			newCollisionShapeDescription.upAxis = capsule->getUpAxis();
			newCollisionShapeDescription.friction = friction;
			btRigidBody* body = static_cast< btRigidBody* >( collisionObject );
			newCollisionShapeDescription.linearVelocity = toVector3(body->getLinearVelocity());
			newCollisionShapeDescription.angularVelocity = toVector3(body->getAngularVelocity());
			m_collisionObjectDetails.push_back( newCollisionShapeDescription );

		} else {
			btAssert(0 && "Unsupported collision shape type\n");
		}
	} else {
		btAssert("Unknown soft body");
	}
} // btOpenCLSoftBodySolver::processCollision





btOpenCLAcceleratedSoftBodyInterface* btOpenCLSoftBodySolver::findSoftBodyInterface( const btSoftBody* const softBody )
{
	for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
	{
		btOpenCLAcceleratedSoftBodyInterface* softBodyInterface = m_softBodySet[softBodyIndex];
		if( softBodyInterface->getSoftBody() == softBody )
			return softBodyInterface;
	}
	return 0;
}


int btOpenCLSoftBodySolver::findSoftBodyIndex( const btSoftBody* const softBody )
{
	for( int softBodyIndex = 0; softBodyIndex < m_softBodySet.size(); ++softBodyIndex )
	{
		btOpenCLAcceleratedSoftBodyInterface* softBodyInterface = m_softBodySet[softBodyIndex];
		if( softBodyInterface->getSoftBody() == softBody )
			return softBodyIndex;
	}
	return 1;
}

bool btOpenCLSoftBodySolver::checkInitialized()
{
	if( !m_shadersInitialized )
		if( buildShaders() )
			m_shadersInitialized = true;

	return m_shadersInitialized;
}

bool btOpenCLSoftBodySolver::buildShaders()
{
	// Ensure current kernels are released first
	releaseKernels();

	bool returnVal = true;

	if( m_shadersInitialized )
		return true;
	
	prepareLinksKernel = clFunctions.compileCLKernelFromString( PrepareLinksCLString, "PrepareLinksKernel" );
	updatePositionsFromVelocitiesKernel = clFunctions.compileCLKernelFromString( UpdatePositionsFromVelocitiesCLString, "UpdatePositionsFromVelocitiesKernel" );
	solvePositionsFromLinksKernel = clFunctions.compileCLKernelFromString( SolvePositionsCLString, "SolvePositionsFromLinksKernel" );
	updateVelocitiesFromPositionsWithVelocitiesKernel = clFunctions.compileCLKernelFromString( UpdateNodesCLString, "updateVelocitiesFromPositionsWithVelocitiesKernel" );
	updateVelocitiesFromPositionsWithoutVelocitiesKernel = clFunctions.compileCLKernelFromString( UpdatePositionsCLString, "updateVelocitiesFromPositionsWithoutVelocitiesKernel" );
	solveCollisionsAndUpdateVelocitiesKernel = clFunctions.compileCLKernelFromString( SolveCollisionsAndUpdateVelocitiesCLString, "SolveCollisionsAndUpdateVelocitiesKernel" );
	integrateKernel = clFunctions.compileCLKernelFromString( IntegrateCLString, "IntegrateKernel" );
	applyForcesKernel = clFunctions.compileCLKernelFromString( ApplyForcesCLString, "ApplyForcesKernel" );

	// TODO: Rename to UpdateSoftBodies
	resetNormalsAndAreasKernel = clFunctions.compileCLKernelFromString( UpdateNormalsCLString, "ResetNormalsAndAreasKernel" );
	normalizeNormalsAndAreasKernel = clFunctions.compileCLKernelFromString( UpdateNormalsCLString, "NormalizeNormalsAndAreasKernel" );
	updateSoftBodiesKernel = clFunctions.compileCLKernelFromString( UpdateNormalsCLString, "UpdateSoftBodiesKernel" );


	if( returnVal )
		m_shadersInitialized = true;

	return returnVal;
}
