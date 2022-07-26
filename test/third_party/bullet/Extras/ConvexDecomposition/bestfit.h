#ifndef BEST_FIT_H

#define BEST_FIT_H

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


// This routine was released in 'snippet' form
// by John W. Ratcliff mailto:jratcliff@infiniplex.net
// on March 22, 2006.
//
// This routine computes the 'best fit' plane equation to
// a set of input data points with an optional per vertex
// weighting component.
//
// The implementation for this was lifted directly from
// David Eberly's Magic Software implementation.

// computes the best fit plane to a collection of data points.
// returns the plane equation as A,B,C,D format. (Ax+By+Cz+D)

bool getBestFitPlane(unsigned int vcount,     // number of input data points
                     const float *points,     // starting address of points array.
                     unsigned int vstride,    // stride between input points.
                     const float *weights,    // *optional point weighting values.
                     unsigned int wstride,    // weight stride for each vertex.
                     float *plane);


float getBoundingRegion(unsigned int vcount,const float *points,unsigned int pstride,float *bmin,float *bmax); // returns the diagonal distance
bool  overlapAABB(const float *bmin1,const float *bmax1,const float *bmin2,const float *bmax2); // return true if the two AABB's overlap.

#endif
