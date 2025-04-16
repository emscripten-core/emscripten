#include "float_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <float.h>
#include <math.h>


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

#include "splitplane.h"
#include "ConvexDecomposition.h"
#include "cd_vector.h"
#include "cd_hull.h"
#include "cd_wavefront.h"
#include "bestfit.h"
#include "planetri.h"
#include "vlookup.h"
#include "meshvolume.h"

namespace ConvexDecomposition
{

static void computePlane(const float *A,const float *B,const float *C,float *plane)
{

	float vx = (B[0] - C[0]);
	float vy = (B[1] - C[1]);
	float vz = (B[2] - C[2]);

	float wx = (A[0] - B[0]);
	float wy = (A[1] - B[1]);
	float wz = (A[2] - B[2]);

	float vw_x = vy * wz - vz * wy;
	float vw_y = vz * wx - vx * wz;
	float vw_z = vx * wy - vy * wx;

	float mag = sqrtf((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

	if ( mag < 0.000001f )
	{
		mag = 0;
	}
	else
	{
		mag = 1.0f/mag;
	}

	float x = vw_x * mag;
	float y = vw_y * mag;
	float z = vw_z * mag;


	float D = 0.0f - ((x*A[0])+(y*A[1])+(z*A[2]));

  plane[0] = x;
  plane[1] = y;
  plane[2] = z;
  plane[3] = D;

}

class Rect3d
{
public:
  Rect3d(void) { };

  Rect3d(const float *bmin,const float *bmax)
  {

    mMin[0] = bmin[0];
    mMin[1] = bmin[1];
    mMin[2] = bmin[2];

    mMax[0] = bmax[0];
    mMax[1] = bmax[1];
    mMax[2] = bmax[2];

  }

  void SetMin(const float *bmin)
  {
    mMin[0] = bmin[0];
    mMin[1] = bmin[1];
    mMin[2] = bmin[2];
  }

  void SetMax(const float *bmax)
  {
    mMax[0] = bmax[0];
    mMax[1] = bmax[1];
    mMax[2] = bmax[2];
  }

	void SetMin(float x,float y,float z)
	{
		mMin[0] = x;
		mMin[1] = y;
		mMin[2] = z;
	}

	void SetMax(float x,float y,float z)
	{
		mMax[0] = x;
		mMax[1] = y;
		mMax[2] = z;
	}

  float mMin[3];
  float mMax[3];
};

void splitRect(unsigned int axis,
						   const Rect3d &source,
							 Rect3d &b1,
							 Rect3d &b2,
							 const float *midpoint)
{
	switch ( axis )
	{
		case 0:
			b1.SetMin(source.mMin);
			b1.SetMax( midpoint[0], source.mMax[1], source.mMax[2] );

			b2.SetMin( midpoint[0], source.mMin[1], source.mMin[2] );
			b2.SetMax(source.mMax);

			break;
		case 1:
			b1.SetMin(source.mMin);
			b1.SetMax( source.mMax[0], midpoint[1], source.mMax[2] );

			b2.SetMin( source.mMin[0], midpoint[1], source.mMin[2] );
			b2.SetMax(source.mMax);

			break;
		case 2:
			b1.SetMin(source.mMin);
			b1.SetMax( source.mMax[0], source.mMax[1], midpoint[2] );

			b2.SetMin( source.mMin[0], source.mMin[1], midpoint[2] );
			b2.SetMax(source.mMax);

			break;
	}
}

bool computeSplitPlane(unsigned int vcount,
                       const float *vertices,
                       unsigned int tcount,
                       const unsigned int *indices,
                       ConvexDecompInterface *callback,
                       float *plane)
{
  float bmin[3] = { 1e9, 1e9, 1e9 };
  float bmax[3] = { -1e9, -1e9, -1e9 };

 	for (unsigned int i=0; i<vcount; i++)
 	{
    const float *p = &vertices[i*3];

 		if ( p[0] < bmin[0] ) bmin[0] = p[0];
 		if ( p[1] < bmin[1] ) bmin[1] = p[1];
 		if ( p[2] < bmin[2] ) bmin[2] = p[2];

 		if ( p[0] > bmax[0] ) bmax[0] = p[0];
 		if ( p[1] > bmax[1] ) bmax[1] = p[1];
 		if ( p[2] > bmax[2] ) bmax[2] = p[2];

  }

  float dx = bmax[0] - bmin[0];
  float dy = bmax[1] - bmin[1];
  float dz = bmax[2] - bmin[2];


	float laxis = dx;

	unsigned int axis = 0;

	if ( dy > dx )
	{
		axis = 1;
		laxis = dy;
	}

	if ( dz > dx && dz > dy )
	{
		axis = 2;
		laxis = dz;
	}

  float p1[3];
  float p2[3];
  float p3[3];

  p3[0] = p2[0] = p1[0] = bmin[0] + dx*0.5f;
  p3[1] = p2[1] = p1[1] = bmin[1] + dy*0.5f;
  p3[2] = p2[2] = p1[2] = bmin[2] + dz*0.5f;

  Rect3d b(bmin,bmax);

  Rect3d b1,b2;

  splitRect(axis,b,b1,b2,p1);


//  callback->ConvexDebugBound(b1.mMin,b1.mMax,0x00FF00);
//  callback->ConvexDebugBound(b2.mMin,b2.mMax,0xFFFF00);

  switch ( axis )
  {
    case 0:
      p2[1] = bmin[1];
      p2[2] = bmin[2];

      if ( dz > dy )
      {
        p3[1] = bmax[1];
        p3[2] = bmin[2];
      }
      else
      {
        p3[1] = bmin[1];
        p3[2] = bmax[2];
      }

      break;
    case 1:
      p2[0] = bmin[0];
      p2[2] = bmin[2];

      if ( dx > dz )
      {
        p3[0] = bmax[0];
        p3[2] = bmin[2];
      }
      else
      {
        p3[0] = bmin[0];
        p3[2] = bmax[2];
      }

      break;
    case 2:
      p2[0] = bmin[0];
      p2[1] = bmin[1];

      if ( dx > dy )
      {
        p3[0] = bmax[0];
        p3[1] = bmin[1];
      }
      else
      {
        p3[0] = bmin[0];
        p3[1] = bmax[1];
      }

      break;
  }

//  callback->ConvexDebugTri(p1,p2,p3,0xFF0000);

	computePlane(p1,p2,p3,plane);

  return true;

}


}
