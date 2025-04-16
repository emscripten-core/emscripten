#include "float_math.h"
#include "meshvolume.h"

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

inline float det(const float *p1,const float *p2,const float *p3)
{
  return  p1[0]*p2[1]*p3[2] + p2[0]*p3[1]*p1[2] + p3[0]*p1[1]*p2[2] -p1[0]*p3[1]*p2[2] - p2[0]*p1[1]*p3[2] - p3[0]*p2[1]*p1[2];
}

float computeMeshVolume(const float *vertices,unsigned int tcount,const unsigned int *indices)
{
	float volume = 0;

	for (unsigned int i=0; i<tcount; i++,indices+=3)
	{

		const float *p1 = &vertices[ indices[0]*3 ];
		const float *p2 = &vertices[ indices[1]*3 ];
		const float *p3 = &vertices[ indices[2]*3 ];

		volume+=det(p1,p2,p3); // compute the volume of the tetrahedran relative to the origin.
	}

	volume*=(1.0f/6.0f);
	if ( volume < 0 ) 
		volume*=-1;
	return volume;
}


inline void CrossProduct(const float *a,const float *b,float *cross)
{
	cross[0] = a[1]*b[2] - a[2]*b[1];
	cross[1] = a[2]*b[0] - a[0]*b[2];
	cross[2] = a[0]*b[1] - a[1]*b[0];
}

inline float DotProduct(const float *a,const float *b)
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

inline float tetVolume(const float *p0,const float *p1,const float *p2,const float *p3)
{
	float a[3];
	float b[3];
	float c[3];

  a[0] = p1[0] - p0[0];
  a[1] = p1[1] - p0[1];
  a[2] = p1[2] - p0[2];

	b[0] = p2[0] - p0[0];
	b[1] = p2[1] - p0[1];
	b[2] = p2[2] - p0[2];

  c[0] = p3[0] - p0[0];
  c[1] = p3[1] - p0[1];
  c[2] = p3[2] - p0[2];

  float cross[3];

  CrossProduct( b, c, cross );

	float volume = DotProduct( a, cross );

  if ( volume < 0 )
   return -volume;

  return volume;
}

inline float det(const float *p0,const float *p1,const float *p2,const float *p3)
{
  return  p1[0]*p2[1]*p3[2] + p2[0]*p3[1]*p1[2] + p3[0]*p1[1]*p2[2] -p1[0]*p3[1]*p2[2] - p2[0]*p1[1]*p3[2] - p3[0]*p2[1]*p1[2];
}

float computeMeshVolume2(const float *vertices,unsigned int tcount,const unsigned int *indices)
{
	float volume = 0;

	const float *p0 = vertices;
	for (unsigned int i=0; i<tcount; i++,indices+=3)
	{

		const float *p1 = &vertices[ indices[0]*3 ];
		const float *p2 = &vertices[ indices[1]*3 ];
		const float *p3 = &vertices[ indices[2]*3 ];

		volume+=tetVolume(p0,p1,p2,p3); // compute the volume of the tetrahdren relative to the root vertice
	}

  return volume * (1.0f / 6.0f );
}

