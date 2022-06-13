#include "float_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#pragma warning(disable:4786)

#include <vector>
#include <map>
#include <set>


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




#include "vlookup.h"

namespace Vlookup
{

class VertexPosition
{
public:
  VertexPosition(void) { };
  VertexPosition(const float *p)
  {
  	mPos[0] = p[0];
  	mPos[1] = p[1];
  	mPos[2] = p[2];
  };

	void Set(int index,const float *pos)
	{
		const float * p = &pos[index*3];

		mPos[0]    = p[0];
		mPos[1]    = p[1];
		mPos[2]    = p[2];

	};

  float GetX(void) const { return mPos[0]; };
  float GetY(void) const { return mPos[1]; };
  float GetZ(void) const { return mPos[2]; };

	float mPos[3];
};

typedef std::vector< VertexPosition > VertexVector;

struct Tracker
{
	VertexPosition mFind; // vertice to locate.
	VertexVector  *mList;

	Tracker()
	{
		mList = 0;
	}

	void SetSearch(const VertexPosition& match,VertexVector *list)
	{
		mFind = match;
		mList = list;
	};
};

struct VertexID
{
	int mID;
	Tracker* mTracker;

	VertexID(int ID, Tracker* Tracker)
	{
		mID = ID;
		mTracker = Tracker;
	}
};

class VertexLess
{
public:

	bool operator()(VertexID v1,VertexID v2) const;

private:
	const VertexPosition& Get(VertexID index) const
	{
		if ( index.mID == -1 ) return index.mTracker->mFind;
		VertexVector &vlist = *index.mTracker->mList;
		return vlist[index.mID];
	}
};

template <class Type> class VertexPool
{
public:
	typedef std::set<VertexID, VertexLess > VertexSet;
	typedef std::vector< Type > VertexVector;

	int getVertex(const Type& vtx)
	{
		mTracker.SetSearch(vtx,&mVtxs);
		VertexSet::iterator found;
		found = mVertSet.find( VertexID(-1,&mTracker) );
		if ( found != mVertSet.end() )
		{
			return found->mID;
		}
		int idx = (int)mVtxs.size();
		mVtxs.push_back( vtx );
		mVertSet.insert( VertexID(idx,&mTracker) );
		return idx;
	};


	const float * GetPos(int idx) const
	{
		return mVtxs[idx].mPos;
	}

	const Type& Get(int idx) const
	{
		return mVtxs[idx];
	};

	unsigned int GetSize(void) const
	{
		return mVtxs.size();
	};

	void Clear(int reservesize)  // clear the vertice pool.
	{
		mVertSet.clear();
		mVtxs.clear();
		mVtxs.reserve(reservesize);
	};

	const VertexVector& GetVertexList(void) const { return mVtxs; };

	void Set(const Type& vtx)
	{
		mVtxs.push_back(vtx);
	}

	unsigned int GetVertexCount(void) const
	{
		return mVtxs.size();
	};


	Type * getBuffer(void)
	{
		return &mVtxs[0];
	};

private:
	VertexSet      mVertSet; // ordered list.
	VertexVector   mVtxs;  // set of vertices.
	Tracker        mTracker;
};


bool VertexLess::operator()(VertexID v1,VertexID v2) const
{

	const VertexPosition& a = Get(v1);
	const VertexPosition& b = Get(v2);

  int ixA = (int) (a.GetX()*10000.0f);
  int ixB = (int) (b.GetX()*10000.0f);

	if ( ixA      < ixB      ) return true;
	if ( ixA      > ixB      ) return false;

  int iyA = (int) (a.GetY()*10000.0f);
  int iyB = (int) (b.GetY()*10000.0f);

	if ( iyA      < iyB      ) return true;
	if ( iyA      > iyB      ) return false;

  int izA = (int) (a.GetZ()*10000.0f);
  int izB = (int) (b.GetZ()*10000.0f);

	if ( izA      < izB      ) return true;
	if ( izA      > izB      ) return false;


	return false;
}




}

using namespace Vlookup;

VertexLookup Vl_createVertexLookup(void)
{
  VertexLookup ret = new VertexPool< VertexPosition >;
  return ret;
}

void          Vl_releaseVertexLookup(VertexLookup vlook)
{
  VertexPool< VertexPosition > *vp = (VertexPool< VertexPosition > *) vlook;
  delete vp;
}

unsigned int  Vl_getIndex(VertexLookup vlook,const float *pos)  // get index.
{
  VertexPool< VertexPosition > *vp = (VertexPool< VertexPosition > *) vlook;
  VertexPosition p(pos);
  return vp->getVertex(p);
}

const float * Vl_getVertices(VertexLookup vlook)
{
  VertexPool< VertexPosition > *vp = (VertexPool< VertexPosition > *) vlook;
  return vp->GetPos(0);
}


unsigned int  Vl_getVcount(VertexLookup vlook)
{
  VertexPool< VertexPosition > *vp = (VertexPool< VertexPosition > *) vlook;
  return vp->GetVertexCount();
}
