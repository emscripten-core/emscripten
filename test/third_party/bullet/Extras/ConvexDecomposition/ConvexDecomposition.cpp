#include "float_math.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


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

#include "ConvexDecomposition.h"
#include "cd_vector.h"
#include "cd_hull.h"
#include "bestfit.h"
#include "planetri.h"
#include "vlookup.h"
#include "splitplane.h"
#include "meshvolume.h"
#include "concavity.h"
#include "bestfitobb.h"
#include "float_math.h"
#include "fitsphere.h"

#define SHOW_MESH 0
#define MAKE_MESH 1


using namespace ConvexDecomposition;



namespace ConvexDecomposition
{

class FaceTri
{
public:
	FaceTri(void) { };
  FaceTri(const float *vertices,unsigned int i1,unsigned int i2,unsigned int i3)
  {
  	mP1.Set( &vertices[i1*3] );
  	mP2.Set( &vertices[i2*3] );
  	mP3.Set( &vertices[i3*3] );
  }

  Vector3d	mP1;
  Vector3d	mP2;
  Vector3d	mP3;
  Vector3d mNormal;

};


void addTri(VertexLookup vl,UintVector &list,const Vector3d &p1,const Vector3d &p2,const Vector3d &p3)
{
  unsigned int i1 = Vl_getIndex(vl, p1.Ptr() );
  unsigned int i2 = Vl_getIndex(vl, p2.Ptr() );
  unsigned int i3 = Vl_getIndex(vl, p3.Ptr() );

  // do *not* process degenerate triangles!

  if ( i1 != i2 && i1 != i3 && i2 != i3 )
  {
    list.push_back(i1);
    list.push_back(i2);
    list.push_back(i3);
  }
}


void calcConvexDecomposition(unsigned int           vcount,
                                const float           *vertices,
                                unsigned int           tcount,
                                const unsigned int    *indices,
                                ConvexDecompInterface *callback,
                                float                  masterVolume,
                                unsigned int           depth)

{

  float plane[4];

  bool split = false;


  if ( depth < MAXDEPTH )
  {

		float volume;
		float c = computeConcavity( vcount, vertices, tcount, indices, callback, plane, volume );

    if ( depth == 0 )
    {
      masterVolume = volume;
    }

		float percent = (c*100.0f)/masterVolume;

		if ( percent > CONCAVE_PERCENT ) // if great than 5% of the total volume is concave, go ahead and keep splitting.
		{
      split = true;
    }

  }

  if ( depth >= MAXDEPTH || !split )
  {

#if 1

    HullResult result;
    HullLibrary hl;
    HullDesc   desc;

  	desc.SetHullFlag(QF_TRIANGLES);

    desc.mVcount       = vcount;
    desc.mVertices     = vertices;
    desc.mVertexStride = sizeof(float)*3;

    HullError ret = hl.CreateConvexHull(desc,result);

    if ( ret == QE_OK )
    {

			ConvexResult r(result.mNumOutputVertices, result.mOutputVertices, result.mNumFaces, result.mIndices);


			callback->ConvexDecompResult(r);
    }


#else

		static unsigned int colors[8] =
		{
			0xFF0000,
		  0x00FF00,
			0x0000FF,
			0xFFFF00,
			0x00FFFF,
			0xFF00FF,
			0xFFFFFF,
			0xFF8040
		};

		static int count = 0;

		count++;

		if ( count == 8 ) count = 0;

		assert( count >= 0 && count < 8 );

		unsigned int color = colors[count];

    const unsigned int *source = indices;

    for (unsigned int i=0; i<tcount; i++)
    {

      unsigned int i1 = *source++;
      unsigned int i2 = *source++;
      unsigned int i3 = *source++;

			FaceTri t(vertices, i1, i2, i3 );

      callback->ConvexDebugTri( t.mP1.Ptr(), t.mP2.Ptr(), t.mP3.Ptr(), color );

    }
#endif

    hl.ReleaseResult (result);
    return;

  }

  UintVector ifront;
  UintVector iback;

  VertexLookup vfront = Vl_createVertexLookup();
  VertexLookup vback  = Vl_createVertexLookup();


	bool showmesh = false;
  #if SHOW_MESH
  showmesh = true;
  #endif

	if ( 0 )
	{
		showmesh = true;
	  for (float x=-1; x<1; x+=0.10f)
		{
		  for (float y=0; y<1; y+=0.10f)
			{
			  for (float z=-1; z<1; z+=0.04f)
				{
				  float d = x*plane[0] + y*plane[1] + z*plane[2] + plane[3];
					Vector3d p(x,y,z);
				  if ( d >= 0 )
					  callback->ConvexDebugPoint(p.Ptr(), 0.02f, 0x00FF00);
				  else
					  callback->ConvexDebugPoint(p.Ptr(), 0.02f, 0xFF0000);
				}
			}
		}
	}

	if ( 1 )
	{
		// ok..now we are going to 'split' all of the input triangles against this plane!
		const unsigned int *source = indices;
		for (unsigned int i=0; i<tcount; i++)
		{
			unsigned int i1 = *source++;
			unsigned int i2 = *source++;
			unsigned int i3 = *source++;

			FaceTri t(vertices, i1, i2, i3 );

			Vector3d front[4];
			Vector3d back[4];

			unsigned int fcount=0;
			unsigned int bcount=0;

			PlaneTriResult result;

		  result = planeTriIntersection(plane,t.mP1.Ptr(),sizeof(Vector3d),0.00001f,front[0].Ptr(),fcount,back[0].Ptr(),bcount );

			if( fcount > 4 || bcount > 4 )
			{
		    result = planeTriIntersection(plane,t.mP1.Ptr(),sizeof(Vector3d),0.00001f,front[0].Ptr(),fcount,back[0].Ptr(),bcount );
			}

			switch ( result )
			{
				case PTR_FRONT:

					assert( fcount == 3 );

          if ( showmesh )
            callback->ConvexDebugTri( front[0].Ptr(), front[1].Ptr(), front[2].Ptr(), 0x00FF00 );

          #if MAKE_MESH

          addTri( vfront, ifront, front[0], front[1], front[2] );


          #endif

					break;
				case PTR_BACK:
					assert( bcount == 3 );

          if ( showmesh )
  					callback->ConvexDebugTri( back[0].Ptr(), back[1].Ptr(), back[2].Ptr(), 0xFFFF00 );

          #if MAKE_MESH

          addTri( vback, iback, back[0], back[1], back[2] );

          #endif

					break;
				case PTR_SPLIT:

					assert( fcount >= 3 && fcount <= 4);
					assert( bcount >= 3 && bcount <= 4);

          #if MAKE_MESH

          addTri( vfront, ifront, front[0], front[1], front[2] );
          addTri( vback, iback, back[0], back[1], back[2] );


          if ( fcount == 4 )
          {
            addTri( vfront, ifront, front[0], front[2], front[3] );
          }

          if ( bcount == 4  )
          {
            addTri( vback, iback, back[0], back[2], back[3] );
          }

          #endif

          if ( showmesh )
          {
  					callback->ConvexDebugTri( front[0].Ptr(), front[1].Ptr(), front[2].Ptr(), 0x00D000 );
  					callback->ConvexDebugTri( back[0].Ptr(), back[1].Ptr(), back[2].Ptr(), 0xD0D000 );

  					if ( fcount == 4 )
  					{
  						callback->ConvexDebugTri( front[0].Ptr(), front[2].Ptr(), front[3].Ptr(), 0x00D000 );
  					}
  					if ( bcount == 4 )
  					{
  						callback->ConvexDebugTri( back[0].Ptr(), back[2].Ptr(), back[3].Ptr(), 0xD0D000 );
  					}
  				}

					break;
			}
		}

    // ok... here we recursively call
    if ( ifront.size() )
    {
      unsigned int vcount   = Vl_getVcount(vfront);
      const float *vertices = Vl_getVertices(vfront);
      unsigned int tcount   = ifront.size()/3;

      calcConvexDecomposition(vcount, vertices, tcount, &ifront[0], callback, masterVolume, depth+1);

    }

    ifront.clear();

    Vl_releaseVertexLookup(vfront);

    if ( iback.size() )
    {
      unsigned int vcount   = Vl_getVcount(vback);
      const float *vertices = Vl_getVertices(vback);
      unsigned int tcount   = iback.size()/3;

      calcConvexDecomposition(vcount, vertices, tcount, &iback[0], callback, masterVolume, depth+1);

    }

    iback.clear();
    Vl_releaseVertexLookup(vback);

	}
}




}
