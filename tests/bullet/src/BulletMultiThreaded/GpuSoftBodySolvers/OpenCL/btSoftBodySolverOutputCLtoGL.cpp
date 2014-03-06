#include "btSoftBodySolverOutputCLtoGL.h"
#include <stdio.h> //@todo: remove the debugging printf at some stage
#include "btSoftBodySolver_OpenCL.h"
#include "BulletSoftBody/btSoftBodySolverVertexBuffer.h"
#include "btSoftBodySolverVertexBuffer_OpenGL.h"
#include "BulletSoftBody/btSoftBody.h"

#if (0)//CL_VERSION_1_1 == 1)
 //OpenCL 1.1 kernels use float3
#define MSTRINGIFY(A) #A
static char* OutputToVertexArrayCLString =
#include "OpenCLC/OutputToVertexArray.cl"
#else
////OpenCL 1.0 kernels don't use float3
#define MSTRINGIFY(A) #A
static char* OutputToVertexArrayCLString =
#include "OpenCLC10/OutputToVertexArray.cl"
#endif //CL_VERSION_1_1

	
#define RELEASE_CL_KERNEL(kernelName) {if( kernelName ){ clReleaseKernel( kernelName ); kernelName = 0; }}

static const size_t workGroupSize = 128;

void btSoftBodySolverOutputCLtoGL::copySoftBodyToVertexBuffer( const btSoftBody * const softBody, btVertexBufferDescriptor *vertexBuffer )
{

	btSoftBodySolver *solver = softBody->getSoftBodySolver();
	btAssert( solver->getSolverType() == btSoftBodySolver::CL_SOLVER || solver->getSolverType() == btSoftBodySolver::CL_SIMD_SOLVER );
	btOpenCLSoftBodySolver *dxSolver = static_cast< btOpenCLSoftBodySolver * >( solver );
	checkInitialized();
	btOpenCLAcceleratedSoftBodyInterface* currentCloth = dxSolver->findSoftBodyInterface( softBody );
	btSoftBodyVertexDataOpenCL &vertexData( dxSolver->m_vertexData );	

	const int firstVertex = currentCloth->getFirstVertex();
	const int lastVertex = firstVertex + currentCloth->getNumVertices();

	if( vertexBuffer->getBufferType() == btVertexBufferDescriptor::OPENGL_BUFFER ) {		

		const btOpenGLInteropVertexBufferDescriptor *openGLVertexBuffer = static_cast< btOpenGLInteropVertexBufferDescriptor* >(vertexBuffer);						
		cl_int ciErrNum = CL_SUCCESS;    

		cl_mem clBuffer = openGLVertexBuffer->getBuffer();		
		cl_kernel outputKernel = outputToVertexArrayWithNormalsKernel;
		if( !vertexBuffer->hasNormals() )
			outputKernel = outputToVertexArrayWithoutNormalsKernel;

		ciErrNum = clEnqueueAcquireGLObjects(m_cqCommandQue, 1, &clBuffer, 0, 0, NULL);
		if( ciErrNum != CL_SUCCESS )
		{
			btAssert( 0 &&  "clEnqueueAcquireGLObjects(copySoftBodyToVertexBuffer)");
		}

		int numVertices = currentCloth->getNumVertices();

		ciErrNum = clSetKernelArg(outputKernel, 0, sizeof(int), &firstVertex );
		ciErrNum = clSetKernelArg(outputKernel, 1, sizeof(int), &numVertices );
		ciErrNum = clSetKernelArg(outputKernel, 2, sizeof(cl_mem), (void*)&clBuffer );
		if( vertexBuffer->hasVertexPositions() )
		{
			int vertexOffset = vertexBuffer->getVertexOffset();
			int vertexStride = vertexBuffer->getVertexStride();
			ciErrNum = clSetKernelArg(outputKernel, 3, sizeof(int), &vertexOffset );
			ciErrNum = clSetKernelArg(outputKernel, 4, sizeof(int), &vertexStride );
			ciErrNum = clSetKernelArg(outputKernel, 5, sizeof(cl_mem), (void*)&vertexData.m_clVertexPosition.m_buffer );

		}
		if( vertexBuffer->hasNormals() )
		{
			int normalOffset = vertexBuffer->getNormalOffset();
			int normalStride = vertexBuffer->getNormalStride();
			ciErrNum = clSetKernelArg(outputKernel, 6, sizeof(int), &normalOffset );
			ciErrNum = clSetKernelArg(outputKernel, 7, sizeof(int), &normalStride );
			ciErrNum = clSetKernelArg(outputKernel, 8, sizeof(cl_mem), (void*)&vertexData.m_clVertexNormal.m_buffer );

		}
		size_t	numWorkItems = workGroupSize*((vertexData.getNumVertices() + (workGroupSize-1)) / workGroupSize);
		ciErrNum = clEnqueueNDRangeKernel(m_cqCommandQue, outputKernel, 1, NULL, &numWorkItems, &workGroupSize,0 ,0 ,0);
		if( ciErrNum != CL_SUCCESS ) 
		{
			btAssert( 0 &&  "enqueueNDRangeKernel(copySoftBodyToVertexBuffer)");
		}

		ciErrNum = clEnqueueReleaseGLObjects(m_cqCommandQue, 1, &clBuffer, 0, 0, 0);
		if( ciErrNum != CL_SUCCESS )
		{
			btAssert( 0 &&  "clEnqueueReleaseGLObjects(copySoftBodyToVertexBuffer)");
		}
	} else {
		btAssert( "Undefined output for this solver output" == false );
	}

	// clFinish in here may not be the best thing. It's possible that we should have a waitForFrameComplete function.
	clFinish(m_cqCommandQue);

} // btSoftBodySolverOutputCLtoGL::outputToVertexBuffers

bool btSoftBodySolverOutputCLtoGL::buildShaders()
{
	// Ensure current kernels are released first
	releaseKernels();

	bool returnVal = true;

	if( m_shadersInitialized )
		return true;
	
	outputToVertexArrayWithNormalsKernel = clFunctions.compileCLKernelFromString( OutputToVertexArrayCLString, "OutputToVertexArrayWithNormalsKernel" );
	outputToVertexArrayWithoutNormalsKernel = clFunctions.compileCLKernelFromString( OutputToVertexArrayCLString, "OutputToVertexArrayWithoutNormalsKernel" );


	if( returnVal )
		m_shadersInitialized = true;

	return returnVal;
} // btSoftBodySolverOutputCLtoGL::buildShaders

void btSoftBodySolverOutputCLtoGL::releaseKernels()
{
	RELEASE_CL_KERNEL( outputToVertexArrayWithNormalsKernel );
	RELEASE_CL_KERNEL( outputToVertexArrayWithoutNormalsKernel );

	m_shadersInitialized = false;
} // btSoftBodySolverOutputCLtoGL::releaseKernels

bool btSoftBodySolverOutputCLtoGL::checkInitialized()
{
	if( !m_shadersInitialized )
		if( buildShaders() )
			m_shadersInitialized = true;

	return m_shadersInitialized;
}