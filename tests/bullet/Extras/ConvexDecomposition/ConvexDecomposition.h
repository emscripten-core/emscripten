#ifndef CONVEX_DECOMPOSITION_H

#define CONVEX_DECOMPOSITION_H

/*----------------------------------------------------------------------
Copyright (c) 2004 Open Dynamics Framework Group
www.physicstools.org
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided
that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions
and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

Neither the name of the Open Dynamics Framework Group nor the names of its contributors may
be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE INTEL OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

// http://codesuppository.blogspot.com
//
// mailto: jratcliff@infiniplex.net
//
// http://www.amillionpixels.us
//


#ifdef _WIN32
#include <memory.h> //memcpy
#endif
#include <string.h>
#include <stdio.h>
#include "LinearMath/btAlignedObjectArray.h"



extern unsigned int MAXDEPTH ;
extern float CONCAVE_PERCENT ;
extern float MERGE_PERCENT   ;


typedef btAlignedObjectArray< unsigned int > UintVector;



namespace ConvexDecomposition
{

	class ConvexResult
	{
	public:
		ConvexResult(void)
		{
			mHullVcount = 0;
			mHullVertices = 0;
			mHullTcount = 0;
			mHullIndices = 0;
		}

		ConvexResult(unsigned int hvcount,const float *hvertices,unsigned int htcount,const unsigned int *hindices)
		{
			mHullVcount = hvcount;
			if ( mHullVcount )
			{
				mHullVertices = new float[mHullVcount*sizeof(float)*3];
				memcpy(mHullVertices, hvertices, sizeof(float)*3*mHullVcount );
			}
			else
			{
				mHullVertices = 0;
			}

			mHullTcount = htcount;

			if ( mHullTcount )
			{
				mHullIndices = new unsigned int[sizeof(unsigned int)*mHullTcount*3];
				memcpy(mHullIndices,hindices, sizeof(unsigned int)*mHullTcount*3 );
			}
			else
			{
				mHullIndices = 0;
			}

		}

		ConvexResult(const ConvexResult &r)
		{
			mHullVcount = r.mHullVcount;
			if ( mHullVcount )
			{
				mHullVertices = new float[mHullVcount*sizeof(float)*3];
				memcpy(mHullVertices, r.mHullVertices, sizeof(float)*3*mHullVcount );
			}
			else
			{
				mHullVertices = 0;
			}
			mHullTcount = r.mHullTcount;
			if ( mHullTcount )
			{
				mHullIndices = new unsigned int[sizeof(unsigned int)*mHullTcount*3];
				memcpy(mHullIndices, r.mHullIndices, sizeof(unsigned int)*mHullTcount*3 );
			}
			else
			{
				mHullIndices = 0;
			}
		}

		~ConvexResult(void)
		{
			delete [] mHullVertices;
			delete [] mHullIndices;
		}

		// the convex hull.
		unsigned int		    mHullVcount;
		float *						  mHullVertices;
		unsigned  int       mHullTcount;
		unsigned int			 *mHullIndices;

		float               mHullVolume;		    // the volume of the convex hull.

		float               mOBBSides[3];			  // the width, height and breadth of the best fit OBB
		float               mOBBCenter[3];      // the center of the OBB
		float               mOBBOrientation[4]; // the quaternion rotation of the OBB.
		float               mOBBTransform[16];  // the 4x4 transform of the OBB.
		float               mOBBVolume;         // the volume of the OBB

		float               mSphereRadius;      // radius and center of best fit sphere
		float               mSphereCenter[3];
		float               mSphereVolume;      // volume of the best fit sphere



	};

	class ConvexDecompInterface
	{
	public:
		virtual ~ConvexDecompInterface() {};
		virtual void ConvexDebugTri(const float *p1,const float *p2,const float *p3,unsigned int color) { };
		virtual void ConvexDebugPoint(const float *p,float dist,unsigned int color) { };
		virtual void ConvexDebugBound(const float *bmin,const float *bmax,unsigned int color) { };
		virtual void ConvexDebugOBB(const float *sides, const float *matrix,unsigned int color) { };

		virtual void ConvexDecompResult(ConvexResult &result) = 0;



	};

	// just to avoid passing a zillion parameters to the method the
	// options are packed into this descriptor.
	class DecompDesc
	{
	public:
		DecompDesc(void)
		{
			mVcount = 0;
			mVertices = 0;
			mTcount   = 0;
			mIndices  = 0;
			mDepth    = 5;
			mCpercent = 5;
			mPpercent = 5;
			mMaxVertices = 32;
			mSkinWidth   = 0;
			mCallback    = 0;
		}

		// describes the input triangle.
		unsigned	int	mVcount;   // the number of vertices in the source mesh.
		const float  *mVertices; // start of the vertex position array.  Assumes a stride of 3 floats.
		unsigned int  mTcount;   // the number of triangles in the source mesh.
		unsigned int *mIndices;  // the indexed triangle list array (zero index based)

		// options
		unsigned int  mDepth;    // depth to split, a maximum of 10, generally not over 7.
		float         mCpercent; // the concavity threshold percentage.  0=20 is reasonable.
		float         mPpercent; // the percentage volume conservation threshold to collapse hulls. 0-30 is reasonable.

		// hull output limits.
		unsigned int  mMaxVertices; // maximum number of vertices in the output hull. Recommended 32 or less.
		float         mSkinWidth;   // a skin width to apply to the output hulls.

		ConvexDecompInterface *mCallback; // the interface to receive back the results.

	};

	// perform approximate convex decomposition on a mesh.
	unsigned int performConvexDecomposition(const DecompDesc &desc); // returns the number of hulls produced.


	void calcConvexDecomposition(unsigned int           vcount,
		const float           *vertices,
		unsigned int           tcount,
		const unsigned int    *indices,
		ConvexDecompInterface *callback,
		float                  masterVolume,
		unsigned int           depth);


}


#endif
