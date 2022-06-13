#include "float_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <vector>

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

#include "concavity.h"
#include "raytri.h"
#include "bestfit.h"
#include "cd_hull.h"
#include "meshvolume.h"
#include "cd_vector.h"
#include "splitplane.h"
#include "ConvexDecomposition.h"


#define WSCALE 4
#define CONCAVE_THRESH 0.05f

namespace ConvexDecomposition
{

unsigned int getDebugColor(void)
{
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

  return color;

}

class Wpoint
{
public:
  Wpoint(const Vector3d &p,float w)
  {
    mPoint = p;
    mWeight = w;
  }

  Vector3d mPoint;
  float           mWeight;
};

typedef std::vector< Wpoint > WpointVector;


static inline float DistToPt(const float *p,const float *plane)
{
	float x = p[0];
	float y = p[1];
	float z = p[2];
	float d = x*plane[0] + y*plane[1] + z*plane[2] + plane[3];
	return d;
}


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


class CTri
{
public:
	CTri(void) { };

  CTri(const float *p1,const float *p2,const float *p3,unsigned int i1,unsigned int i2,unsigned int i3)
  {
    mProcessed = 0;
    mI1 = i1;
    mI2 = i2;
    mI3 = i3;

  	mP1.Set(p1);
  	mP2.Set(p2);
  	mP3.Set(p3);

  	mPlaneD = mNormal.ComputePlane(mP1,mP2,mP3);
	}

  float Facing(const CTri &t)
  {
		float d = mNormal.Dot(t.mNormal);
		return d;
  }

  // clip this line segment against this triangle.
  bool clip(const Vector3d &start,Vector3d &end) const
  {
    Vector3d sect;

    bool hit = lineIntersectsTriangle(start.Ptr(), end.Ptr(), mP1.Ptr(), mP2.Ptr(), mP3.Ptr(), sect.Ptr() );

    if ( hit )
    {
      end = sect;
    }
    return hit;
  }

	bool Concave(const Vector3d &p,float &distance,Vector3d &n) const
	{
		n.NearestPointInTriangle(p,mP1,mP2,mP3);
		distance = p.Distance(n);
		return true;
	}

	void addTri(unsigned int *indices,unsigned int i1,unsigned int i2,unsigned int i3,unsigned int &tcount) const
	{
		indices[tcount*3+0] = i1;
		indices[tcount*3+1] = i2;
		indices[tcount*3+2] = i3;
		tcount++;
	}

	float getVolume(ConvexDecompInterface *callback) const
	{
		unsigned int indices[8*3];


    unsigned int tcount = 0;

    addTri(indices,0,1,2,tcount);
    addTri(indices,3,4,5,tcount);

    addTri(indices,0,3,4,tcount);
    addTri(indices,0,4,1,tcount);

    addTri(indices,1,4,5,tcount);
    addTri(indices,1,5,2,tcount);

    addTri(indices,0,3,5,tcount);
    addTri(indices,0,5,2,tcount);

    const float *vertices = mP1.Ptr();

		if ( callback )
		{
			unsigned int color = getDebugColor();

#if 0
  		Vector3d d1 = mNear1;
  		Vector3d d2 = mNear2;
	  	Vector3d d3 = mNear3;

  		callback->ConvexDebugPoint(mP1.Ptr(),0.01f,0x00FF00);
  		callback->ConvexDebugPoint(mP2.Ptr(),0.01f,0x00FF00);
  		callback->ConvexDebugPoint(mP3.Ptr(),0.01f,0x00FF00);
  		callback->ConvexDebugPoint(d1.Ptr(),0.01f,0xFF0000);
  		callback->ConvexDebugPoint(d2.Ptr(),0.01f,0xFF0000);
  		callback->ConvexDebugPoint(d3.Ptr(),0.01f,0xFF0000);

  		callback->ConvexDebugTri(mP1.Ptr(), d1.Ptr(),  d1.Ptr(),0x00FF00);
  		callback->ConvexDebugTri(mP2.Ptr(), d2.Ptr(),  d2.Ptr(),0x00FF00);
  		callback->ConvexDebugTri(mP3.Ptr(), d3.Ptr(),  d3.Ptr(),0x00FF00);

#else
			for (unsigned int i=0; i<tcount; i++)
			{
				unsigned int i1 = indices[i*3+0];
				unsigned int i2 = indices[i*3+1];
				unsigned int i3 = indices[i*3+2];

				const float *p1 = &vertices[ i1*3 ];
				const float *p2 = &vertices[ i2*3 ];
				const float *p3 = &vertices[ i3*3 ];

				callback->ConvexDebugTri(p1,p2,p3,color);

			}
#endif
		}

		float v = computeMeshVolume(mP1.Ptr(), tcount, indices );

		return v;

	}

	float raySect(const Vector3d &p,const Vector3d &dir,Vector3d &sect) const
	{
		float plane[4];

    plane[0] = mNormal.x;
    plane[1] = mNormal.y;
    plane[2] = mNormal.z;
    plane[3] = mPlaneD;

		Vector3d dest = p+dir*100000;

    intersect( p.Ptr(), dest.Ptr(), sect.Ptr(), plane );

    return sect.Distance(p); // return the intersection distance.

	}

  float planeDistance(const Vector3d &p) const
  {
		float plane[4];

    plane[0] = mNormal.x;
    plane[1] = mNormal.y;
    plane[2] = mNormal.z;
    plane[3] = mPlaneD;

		return DistToPt( p.Ptr(), plane );

  }

	bool samePlane(const CTri &t) const
	{
		const float THRESH = 0.001f;
    float dd = fabsf( t.mPlaneD - mPlaneD );
    if ( dd > THRESH ) return false;
    dd = fabsf( t.mNormal.x - mNormal.x );
    if ( dd > THRESH ) return false;
    dd = fabsf( t.mNormal.y - mNormal.y );
    if ( dd > THRESH ) return false;
    dd = fabsf( t.mNormal.z - mNormal.z );
    if ( dd > THRESH ) return false;
    return true;
	}

	bool hasIndex(unsigned int i) const
	{
		if ( i == mI1 || i == mI2 || i == mI3 ) return true;
		return false;
	}

  bool sharesEdge(const CTri &t) const
  {
    bool ret = false;
    unsigned int count = 0;

		if ( t.hasIndex(mI1) ) count++;
	  if ( t.hasIndex(mI2) ) count++;
		if ( t.hasIndex(mI3) ) count++;

    if ( count >= 2 ) ret = true;

    return ret;
  }

  void debug(unsigned int color,ConvexDecompInterface *callback)
  {
    callback->ConvexDebugTri( mP1.Ptr(), mP2.Ptr(), mP3.Ptr(), color );
    callback->ConvexDebugTri( mP1.Ptr(), mP1.Ptr(), mNear1.Ptr(), 0xFF0000 );
    callback->ConvexDebugTri( mP2.Ptr(), mP2.Ptr(), mNear2.Ptr(), 0xFF0000 );
    callback->ConvexDebugTri( mP2.Ptr(), mP3.Ptr(), mNear3.Ptr(), 0xFF0000 );
    callback->ConvexDebugPoint( mNear1.Ptr(), 0.01f, 0xFF0000 );
    callback->ConvexDebugPoint( mNear2.Ptr(), 0.01f, 0xFF0000 );
    callback->ConvexDebugPoint( mNear3.Ptr(), 0.01f, 0xFF0000 );
  }

  float area(void)
  {
		float a = mConcavity*mP1.Area(mP2,mP3);
    return a;
  }

  void addWeighted(WpointVector &list,ConvexDecompInterface *callback)
  {

    Wpoint p1(mP1,mC1);
    Wpoint p2(mP2,mC2);
    Wpoint p3(mP3,mC3);

		Vector3d d1 = mNear1 - mP1;
		Vector3d d2 = mNear2 - mP2;
		Vector3d d3 = mNear3 - mP3;

		d1*=WSCALE;
		d2*=WSCALE;
		d3*=WSCALE;

		d1 = d1 + mP1;
		d2 = d2 + mP2;
 	  d3 = d3 + mP3;

    Wpoint p4(d1,mC1);
    Wpoint p5(d2,mC2);
    Wpoint p6(d3,mC3);

    list.push_back(p1);
    list.push_back(p2);
    list.push_back(p3);

    list.push_back(p4);
    list.push_back(p5);
    list.push_back(p6);

#if 0
		callback->ConvexDebugPoint(mP1.Ptr(),0.01f,0x00FF00);
		callback->ConvexDebugPoint(mP2.Ptr(),0.01f,0x00FF00);
		callback->ConvexDebugPoint(mP3.Ptr(),0.01f,0x00FF00);
		callback->ConvexDebugPoint(d1.Ptr(),0.01f,0xFF0000);
		callback->ConvexDebugPoint(d2.Ptr(),0.01f,0xFF0000);
		callback->ConvexDebugPoint(d3.Ptr(),0.01f,0xFF0000);

		callback->ConvexDebugTri(mP1.Ptr(), d1.Ptr(),  d1.Ptr(),0x00FF00);
		callback->ConvexDebugTri(mP2.Ptr(), d2.Ptr(),  d2.Ptr(),0x00FF00);
		callback->ConvexDebugTri(mP3.Ptr(), d3.Ptr(),  d3.Ptr(),0x00FF00);

		Vector3d np1 = mP1 + mNormal*0.05f;
		Vector3d np2 = mP2 + mNormal*0.05f;
		Vector3d np3 = mP3 + mNormal*0.05f;

		callback->ConvexDebugTri(mP1.Ptr(), np1.Ptr(), np1.Ptr(), 0xFF00FF );
		callback->ConvexDebugTri(mP2.Ptr(), np2.Ptr(), np2.Ptr(), 0xFF00FF );
		callback->ConvexDebugTri(mP3.Ptr(), np3.Ptr(), np3.Ptr(), 0xFF00FF );

		callback->ConvexDebugPoint( np1.Ptr(), 0.01F, 0XFF00FF );
		callback->ConvexDebugPoint( np2.Ptr(), 0.01F, 0XFF00FF );
		callback->ConvexDebugPoint( np3.Ptr(), 0.01F, 0XFF00FF );

#endif



  }

  Vector3d	mP1;
  Vector3d	mP2;
  Vector3d	mP3;
  Vector3d mNear1;
  Vector3d mNear2;
  Vector3d mNear3;
  Vector3d mNormal;
  float           mPlaneD;
  float           mConcavity;
  float           mC1;
  float           mC2;
  float           mC3;
  unsigned int    mI1;
  unsigned int    mI2;
  unsigned int    mI3;
  int             mProcessed; // already been added...
};

typedef std::vector< CTri > CTriVector;

bool featureMatch(CTri &m,const CTriVector &tris,ConvexDecompInterface *callback,const CTriVector &input_mesh)
{

  bool ret = false;

  float neardot = 0.707f;

  m.mConcavity = 0;

	//gLog->Display("*********** FEATURE MATCH *************\r\n");
	//gLog->Display("Plane: %0.4f,%0.4f,%0.4f   %0.4f\r\n", m.mNormal.x, m.mNormal.y, m.mNormal.z, m.mPlaneD );
	//gLog->Display("*********************************************\r\n");

	CTriVector::const_iterator i;

	CTri nearest;


	for (i=tris.begin(); i!=tris.end(); ++i)
	{
		const CTri &t = (*i);


  	//gLog->Display("   HullPlane: %0.4f,%0.4f,%0.4f   %0.4f\r\n", t.mNormal.x, t.mNormal.y, t.mNormal.z, t.mPlaneD );

		if ( t.samePlane(m) )
		{
			//gLog->Display("*** PLANE MATCH!!!\r\n");
			ret = false;
			break;
		}

	  float dot = t.mNormal.Dot(m.mNormal);

	  if ( dot > neardot )
	  {

      float d1 = t.planeDistance( m.mP1 );
      float d2 = t.planeDistance( m.mP2 );
      float d3 = t.planeDistance( m.mP3 );

      if ( d1 > 0.001f || d2 > 0.001f || d3 > 0.001f ) // can't be near coplaner!
      {

  	  	neardot = dot;

        Vector3d n1,n2,n3;

        t.raySect( m.mP1, m.mNormal, m.mNear1 );
        t.raySect( m.mP2, m.mNormal, m.mNear2 );
        t.raySect( m.mP3, m.mNormal, m.mNear3 );

				nearest = t;

	  		ret = true;
		  }

	  }
	}

	if ( ret )
	{
    if ( 0 )
    {
      CTriVector::const_iterator i;
      for (i=input_mesh.begin(); i!=input_mesh.end(); ++i)
      {
        const CTri &c = (*i);
        if ( c.mI1 != m.mI1 && c.mI2 != m.mI2 && c.mI3 != m.mI3 )
        {
  			  c.clip( m.mP1, m.mNear1 );
				  c.clip( m.mP2, m.mNear2 );
				  c.clip( m.mP3, m.mNear3 );
        }
      }
    }

	  //gLog->Display("*********************************************\r\n");
  	//gLog->Display("   HullPlaneNearest: %0.4f,%0.4f,%0.4f   %0.4f\r\n", nearest.mNormal.x, nearest.mNormal.y, nearest.mNormal.z, nearest.mPlaneD );

		m.mC1 = m.mP1.Distance( m.mNear1 );
		m.mC2 = m.mP2.Distance( m.mNear2 );
		m.mC3 = m.mP3.Distance( m.mNear3 );

		m.mConcavity = m.mC1;

		if ( m.mC2 > m.mConcavity ) m.mConcavity = m.mC2;
		if ( m.mC3 > m.mConcavity ) m.mConcavity = m.mC3;

    #if 0
		callback->ConvexDebugTri( m.mP1.Ptr(), m.mP2.Ptr(), m.mP3.Ptr(), 0x00FF00 );
		callback->ConvexDebugTri( m.mNear1.Ptr(), m.mNear2.Ptr(), m.mNear3.Ptr(), 0xFF0000 );

		callback->ConvexDebugTri( m.mP1.Ptr(), m.mP1.Ptr(), m.mNear1.Ptr(), 0xFFFF00 );
		callback->ConvexDebugTri( m.mP2.Ptr(), m.mP2.Ptr(), m.mNear2.Ptr(), 0xFFFF00 );
		callback->ConvexDebugTri( m.mP3.Ptr(), m.mP3.Ptr(), m.mNear3.Ptr(), 0xFFFF00 );
    #endif

	}
	else
	{
		//gLog->Display("No match\r\n");
	}

	//gLog->Display("*********************************************\r\n");
	return ret;
}

bool isFeatureTri(CTri &t,CTriVector &flist,float fc,ConvexDecompInterface *callback,unsigned int color)
{
  bool ret = false;

  if ( t.mProcessed == 0 ) // if not already processed
  {

    float c = t.mConcavity / fc; // must be within 80% of the concavity of the parent.

    if ( c > 0.85f )
    {
      // see if this triangle is a 'feature' triangle.  Meaning it shares an
      // edge with any existing feature triangle and is within roughly the same
      // concavity of the parent.
			if ( flist.size() )
			{
			  CTriVector::iterator i;
			  for (i=flist.begin(); i!=flist.end(); ++i)
			  {
				  CTri &ftri = (*i);
				  if ( ftri.sharesEdge(t) )
				  {
					  t.mProcessed = 2; // it is now part of a feature.
					  flist.push_back(t); // add it to the feature list.
//					  callback->ConvexDebugTri( t.mP1.Ptr(), t.mP2.Ptr(),t.mP3.Ptr(), color );
					  ret = true;
					  break;
          }
				}
			}
			else
			{
				t.mProcessed = 2;
				flist.push_back(t); // add it to the feature list.
//				callback->ConvexDebugTri( t.mP1.Ptr(), t.mP2.Ptr(),t.mP3.Ptr(), color );
				ret = true;
			}
    }
    else
    {
      t.mProcessed = 1; // eliminated for this feature, but might be valid for the next one..
    }

  }
  return ret;
}

float computeConcavity(unsigned int vcount,
                       const float *vertices,
                       unsigned int tcount,
                       const unsigned int *indices,
                       ConvexDecompInterface *callback,
                       float *plane,      // plane equation to split on
                       float &volume)
{


	float cret = 0;
	volume = 1;

	HullResult  result;
  HullLibrary hl;
  HullDesc    desc;

	desc.mMaxFaces = 256;
	desc.mMaxVertices = 256;
	desc.SetHullFlag(QF_TRIANGLES);


  desc.mVcount       = vcount;
  desc.mVertices     = vertices;
  desc.mVertexStride = sizeof(float)*3;

  HullError ret = hl.CreateConvexHull(desc,result);

  if ( ret == QE_OK )
  {
#if 0
		float bmin[3];
		float bmax[3];

		float dx = bmax[0] - bmin[0];
		float dy = bmax[1] - bmin[1];
		float dz = bmax[2] - bmin[2];

		Vector3d center;

		center.x = bmin[0] + dx*0.5f;
		center.y = bmin[1] + dy*0.5f;
		center.z = bmin[2] + dz*0.5f;
#endif

		volume = computeMeshVolume2( result.mOutputVertices, result.mNumFaces, result.mIndices );

#if 1
		// ok..now..for each triangle on the original mesh..
		// we extrude the points to the nearest point on the hull.
		const unsigned int *source = result.mIndices;

		CTriVector tris;

    for (unsigned int i=0; i<result.mNumFaces; i++)
    {
    	unsigned int i1 = *source++;
    	unsigned int i2 = *source++;
    	unsigned int i3 = *source++;

    	const float *p1 = &result.mOutputVertices[i1*3];
    	const float *p2 = &result.mOutputVertices[i2*3];
    	const float *p3 = &result.mOutputVertices[i3*3];

//			callback->ConvexDebugTri(p1,p2,p3,0xFFFFFF);

			CTri t(p1,p2,p3,i1,i2,i3); //
			tris.push_back(t);
		}

    // we have not pre-computed the plane equation for each triangle in the convex hull..

		float totalVolume = 0;

		CTriVector ftris; // 'feature' triangles.

		const unsigned int *src = indices;


    float maxc=0;


		if ( 1 )
		{
      CTriVector input_mesh;
      if ( 1 )
      {
		    const unsigned int *src = indices;
  			for (unsigned int i=0; i<tcount; i++)
  			{

      		unsigned int i1 = *src++;
      		unsigned int i2 = *src++;
      		unsigned int i3 = *src++;

      		const float *p1 = &vertices[i1*3];
      		const float *p2 = &vertices[i2*3];
      		const float *p3 = &vertices[i3*3];

   				CTri t(p1,p2,p3,i1,i2,i3);
          input_mesh.push_back(t);
        }
      }

      CTri  maxctri;

			for (unsigned int i=0; i<tcount; i++)
			{

    		unsigned int i1 = *src++;
    		unsigned int i2 = *src++;
    		unsigned int i3 = *src++;

    		const float *p1 = &vertices[i1*3];
    		const float *p2 = &vertices[i2*3];
    		const float *p3 = &vertices[i3*3];

 				CTri t(p1,p2,p3,i1,i2,i3);

				featureMatch(t, tris, callback, input_mesh );

				if ( t.mConcavity > CONCAVE_THRESH )
				{

          if ( t.mConcavity > maxc )
          {
            maxc = t.mConcavity;
            maxctri = t;
          }

  				float v = t.getVolume(0);
  				totalVolume+=v;
   				ftris.push_back(t);
   			}

			}
		}

	  if ( ftris.size()  && 0 )
	  {

      // ok..now we extract the triangles which form the maximum concavity.
      CTriVector major_feature;
      float maxarea = 0;

      while ( maxc > CONCAVE_THRESH  )
      {

        unsigned int color = getDebugColor();  //

        CTriVector flist;

        bool found;

        float totalarea = 0;

        do
        {
          found = false;
          CTriVector::iterator i;
          for (i=ftris.begin(); i!=ftris.end(); ++i)
          {
            CTri &t = (*i);
            if ( isFeatureTri(t,flist,maxc,callback,color) )
            {
              found = true;
              totalarea+=t.area();
            }
  				}
        } while ( found );


        if ( totalarea > maxarea )
        {
          major_feature = flist;
          maxarea = totalarea;
        }

        maxc = 0;

        for (unsigned int i=0; i<ftris.size(); i++)
        {
          CTri &t = ftris[i];
          if ( t.mProcessed != 2 )
          {
            t.mProcessed = 0;
            if ( t.mConcavity > maxc )
            {
              maxc = t.mConcavity;
            }
          }
        }
      }

      unsigned int color = getDebugColor();

      WpointVector list;
      for (unsigned int i=0; i<major_feature.size(); ++i)
      {
        major_feature[i].addWeighted(list,callback);
        major_feature[i].debug(color,callback);
      }

      getBestFitPlane( list.size(), &list[0].mPoint.x, sizeof(Wpoint), &list[0].mWeight, sizeof(Wpoint), plane );

	  	computeSplitPlane( vcount, vertices, tcount, indices, callback, plane );


		}
	  else
	  {
	  	computeSplitPlane( vcount, vertices, tcount, indices, callback, plane );
	  }
#endif

		cret = totalVolume;

	  hl.ReleaseResult(result);
  }


	return cret;
}


}
