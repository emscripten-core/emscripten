#include "float_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "planetri.h"

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

static inline float DistToPt(const float *p,const float *plane)
{
	float x = p[0];
	float y = p[1];
	float z = p[2];
	float d = x*plane[0] + y*plane[1] + z*plane[2] + plane[3];
	return d;
}


static PlaneTriResult getSidePlane(const float *p,const float *plane,float epsilon)
{

  float d = DistToPt(p,plane);

  if ( (d+epsilon) > 0 )
		return PTR_FRONT; // it is 'in front' within the provided epsilon value.

  return PTR_BACK;
}

static void add(const float *p,float *dest,unsigned int tstride,unsigned int &pcount)
{
  char *d = (char *) dest;
  d = d + pcount*tstride;
  dest = (float *) d;
  dest[0] = p[0];
  dest[1] = p[1];
  dest[2] = p[2];
  pcount++;
	assert( pcount <= 4 );
}


// assumes that the points are on opposite sides of the plane!
static void intersect(const float *p1,const float *p2,float *split,const float *plane)
{

  float dp1 = DistToPt(p1,plane);

  float dir[3];

  dir[0] = p2[0] - p1[0];
  dir[1] = p2[1] - p1[1];
  dir[2] = p2[2] - p1[2];

  float dot1 = dir[0]*plane[0] + dir[1]*plane[1] + dir[2]*plane[2];
  float dot2 = dp1 - plane[3];

  float    t = -(plane[3] + dot2 ) / dot1;

  split[0] = (dir[0]*t)+p1[0];
  split[1] = (dir[1]*t)+p1[1];
  split[2] = (dir[2]*t)+p1[2];

}

PlaneTriResult planeTriIntersection(const float *plane,    // the plane equation in Ax+By+Cz+D format
                                    const float *triangle, // the source triangle.
                                    unsigned int tstride,  // stride in bytes of the input and output triangles
                                    float        epsilon,  // the co-planer epsilon value.
                                    float       *front,    // the triangle in front of the
                                    unsigned int &fcount,  // number of vertices in the 'front' triangle
                                    float       *back,     // the triangle in back of the plane
                                    unsigned int &bcount) // the number of vertices in the 'back' triangle.
{
  fcount = 0;
  bcount = 0;

  const char *tsource = (const char *) triangle;

  // get the three vertices of the triangle.
  const float *p1     = (const float *) (tsource);
  const float *p2     = (const float *) (tsource+tstride);
  const float *p3     = (const float *) (tsource+tstride*2);


  PlaneTriResult r1   = getSidePlane(p1,plane,epsilon); // compute the side of the plane each vertex is on
  PlaneTriResult r2   = getSidePlane(p2,plane,epsilon);
  PlaneTriResult r3   = getSidePlane(p3,plane,epsilon);

  if ( r1 == r2 && r1 == r3 ) // if all three vertices are on the same side of the plane.
  {
    if ( r1 == PTR_FRONT ) // if all three are in front of the plane, then copy to the 'front' output triangle.
    {
      add(p1,front,tstride,fcount);
      add(p2,front,tstride,fcount);
      add(p3,front,tstride,fcount);
    }
    else
    {
      add(p1,back,tstride,bcount); // if all three are in 'abck' then copy to the 'back' output triangle.
      add(p2,back,tstride,bcount);
      add(p3,back,tstride,bcount);
    }
    return r1; // if all three points are on the same side of the plane return result
  }

  // ok.. we need to split the triangle at the plane.

  // First test ray segment P1 to P2
  if ( r1 == r2 ) // if these are both on the same side...
  {
    if ( r1 == PTR_FRONT )
    {
      add( p1, front, tstride, fcount );
      add( p2, front, tstride, fcount );
    }
    else
    {
      add( p1, back, tstride, bcount );
      add( p2, back, tstride, bcount );
    }
  }
  else
  {
    float split[3]; // split the point
    intersect(p1,p2,split,plane);

    if ( r1 == PTR_FRONT )
    {

      add(p1, front, tstride, fcount );
      add(split, front, tstride, fcount );

      add(split, back, tstride, bcount );
      add(p2, back, tstride, bcount );

    }
    else
    {
      add(p1, back, tstride, bcount );
      add(split, back, tstride, bcount );

      add(split, front, tstride, fcount );
      add(p2, front, tstride, fcount );
    }

  }

  // Next test ray segment P2 to P3
  if ( r2 == r3 ) // if these are both on the same side...
  {
    if ( r3 == PTR_FRONT )
    {
      add( p3, front, tstride, fcount );
    }
    else
    {
      add( p3, back, tstride, bcount );
    }
  }
  else
  {
    float split[3]; // split the point
    intersect(p2,p3,split,plane);

    if ( r3 == PTR_FRONT )
    {
      add(split, front, tstride, fcount );
      add(split, back, tstride, bcount );

      add(p3, front, tstride, fcount );
    }
    else
    {
      add(split, front, tstride, fcount );
      add(split, back, tstride, bcount );

      add(p3, back, tstride, bcount );
    }
  }

  // Next test ray segment P3 to P1
  if ( r3 != r1 ) // if these are both on the same side...
  {
    float split[3]; // split the point

    intersect(p3,p1,split,plane);

    if ( r1 == PTR_FRONT )
    {
      add(split, front, tstride, fcount );
      add(split, back, tstride, bcount );
    }
    else
    {
      add(split, front, tstride, fcount );
      add(split, back, tstride, bcount );
    }
  }



  return PTR_SPLIT;
}
