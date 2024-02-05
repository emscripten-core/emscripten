#ifndef VLOOKUP_H

#define VLOOKUP_H


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


// CodeSnippet provided by John W. Ratcliff
// on March 23, 2006.
//
// mailto: jratcliff@infiniplex.net
//
// Personal website: http://jratcliffscarab.blogspot.com
// Coding Website:   http://codesuppository.blogspot.com
// FundRaising Blog: http://amillionpixels.blogspot.com
// Fundraising site: http://www.amillionpixels.us
// New Temple Site:  http://newtemple.blogspot.com
//
// This snippet shows how to 'hide' the complexity of
// the STL by wrapping some useful piece of functionality
// around a handful of discrete API calls.
//
// This API allows you to create an indexed triangle list
// from a collection of raw input triangles.  Internally
// it uses an STL set to build the lookup table very rapidly.
//
// Here is how you would use it to build an indexed triangle
// list from a raw list of triangles.
//
// (1) create a 'VertexLookup' interface by calling
//
//     VertexLook vl = Vl_createVertexLookup();
//
// (2) For each vertice in each triangle call:
//
//     unsigned int i1 = Vl_getIndex(vl,p1);
//     unsigned int i2 = Vl_getIndex(vl,p2);
//     unsigned int i3 = Vl_getIndex(vl,p3);
//
//     save the 3 indices into your triangle list array.
//
// (3) Get the vertex array by calling:
//
//     const float *vertices = Vl_getVertices(vl);
//
// (4) Get the number of vertices so you can copy them into
//     your own buffer.
//     unsigned int vcount = Vl_getVcount(vl);
//
// (5) Release the VertexLookup interface when you are done with it.
//     Vl_releaseVertexLookup(vl);
//
// Teaches the following lessons:
//
//    How to wrap the complexity of STL and C++ classes around a
//    simple API interface.
//
//    How to use an STL set and custom comparator operator for
//    a complex data type.
//
//    How to create a template class.
//
//    How to achieve significant performance improvements by
//    taking advantage of built in STL containers in just
//    a few lines of code.
//
//    You could easily modify this code to support other vertex
//    formats with any number of interpolants.
//
//    Hide C++ classes from the rest of your application by
//    keeping them in the CPP and wrapping them in a namespace
// Uses an STL set to create an index table for a bunch of vertex positions
// used typically to re-index a collection of raw triangle data.


typedef void * VertexLookup;

VertexLookup  Vl_createVertexLookup(void);
void          Vl_releaseVertexLookup(VertexLookup vlook);

unsigned int  Vl_getIndex(VertexLookup vlook,const float *pos);  // get index.
const float * Vl_getVertices(VertexLookup vlook);
unsigned int  Vl_getVcount(VertexLookup vlook);


#endif
