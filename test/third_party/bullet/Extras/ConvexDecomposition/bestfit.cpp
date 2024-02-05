#include "float_math.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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
// Geometric Tools, Inc.
// http://www.geometrictools.com
// Copyright (c) 1998-2006.  All Rights Reserved
//
// The Wild Magic Library (WM3) source code is supplied under the terms of
// the license agreement
//     http://www.geometrictools.com/License/WildMagic3License.pdf
// and may not be copied or disclosed except in accordance with the terms
// of that agreement.

#include "bestfit.h"

namespace BestFit
{

class Vec3
{
public:
  Vec3(void) { };
  Vec3(float _x,float _y,float _z) { x = _x; y = _y; z = _z; };


  float dot(const Vec3 &v)
  {
    return x*v.x + y*v.y + z*v.z; // the dot product
  }

  float x;
  float y;
  float z;
};


class Eigen
{
public:


  void DecrSortEigenStuff(void)
  {
    Tridiagonal(); //diagonalize the matrix.
    QLAlgorithm(); //
    DecreasingSort();
    GuaranteeRotation();
  }

  void Tridiagonal(void)
  {
    float fM00 = mElement[0][0];
    float fM01 = mElement[0][1];
    float fM02 = mElement[0][2];
    float fM11 = mElement[1][1];
    float fM12 = mElement[1][2];
    float fM22 = mElement[2][2];

    m_afDiag[0] = fM00;
    m_afSubd[2] = 0;
    if (fM02 != (float)0.0)
    {
      float fLength = sqrtf(fM01*fM01+fM02*fM02);
      float fInvLength = ((float)1.0)/fLength;
      fM01 *= fInvLength;
      fM02 *= fInvLength;
      float fQ = ((float)2.0)*fM01*fM12+fM02*(fM22-fM11);
      m_afDiag[1] = fM11+fM02*fQ;
      m_afDiag[2] = fM22-fM02*fQ;
      m_afSubd[0] = fLength;
      m_afSubd[1] = fM12-fM01*fQ;
      mElement[0][0] = (float)1.0;
      mElement[0][1] = (float)0.0;
      mElement[0][2] = (float)0.0;
      mElement[1][0] = (float)0.0;
      mElement[1][1] = fM01;
      mElement[1][2] = fM02;
      mElement[2][0] = (float)0.0;
      mElement[2][1] = fM02;
      mElement[2][2] = -fM01;
      m_bIsRotation = false;
    }
    else
    {
      m_afDiag[1] = fM11;
      m_afDiag[2] = fM22;
      m_afSubd[0] = fM01;
      m_afSubd[1] = fM12;
      mElement[0][0] = (float)1.0;
      mElement[0][1] = (float)0.0;
      mElement[0][2] = (float)0.0;
      mElement[1][0] = (float)0.0;
      mElement[1][1] = (float)1.0;
      mElement[1][2] = (float)0.0;
      mElement[2][0] = (float)0.0;
      mElement[2][1] = (float)0.0;
      mElement[2][2] = (float)1.0;
      m_bIsRotation = true;
    }
  }

  bool QLAlgorithm(void)
  {
    const int iMaxIter = 32;

    for (int i0 = 0; i0 <3; i0++)
    {
      int i1;
      for (i1 = 0; i1 < iMaxIter; i1++)
      {
        int i2;
        for (i2 = i0; i2 <= (3-2); i2++)
        {
          float fTmp = fabsf(m_afDiag[i2]) + fabsf(m_afDiag[i2+1]);
          if ( fabsf(m_afSubd[i2]) + fTmp == fTmp )
            break;
        }
        if (i2 == i0)
        {
          break;
        }

        float fG = (m_afDiag[i0+1] - m_afDiag[i0])/(((float)2.0) * m_afSubd[i0]);
        float fR = sqrtf(fG*fG+(float)1.0);
        if (fG < (float)0.0)
        {
          fG = m_afDiag[i2]-m_afDiag[i0]+m_afSubd[i0]/(fG-fR);
        }
        else
        {
          fG = m_afDiag[i2]-m_afDiag[i0]+m_afSubd[i0]/(fG+fR);
        }
        float fSin = (float)1.0, fCos = (float)1.0, fP = (float)0.0;
        for (int i3 = i2-1; i3 >= i0; i3--)
        {
          float fF = fSin*m_afSubd[i3];
          float fB = fCos*m_afSubd[i3];
          if (fabsf(fF) >= fabsf(fG))
          {
            fCos = fG/fF;
            fR = sqrtf(fCos*fCos+(float)1.0);
            m_afSubd[i3+1] = fF*fR;
            fSin = ((float)1.0)/fR;
            fCos *= fSin;
          }
          else
          {
            fSin = fF/fG;
            fR = sqrtf(fSin*fSin+(float)1.0);
            m_afSubd[i3+1] = fG*fR;
            fCos = ((float)1.0)/fR;
            fSin *= fCos;
          }
          fG = m_afDiag[i3+1]-fP;
          fR = (m_afDiag[i3]-fG)*fSin+((float)2.0)*fB*fCos;
          fP = fSin*fR;
          m_afDiag[i3+1] = fG+fP;
          fG = fCos*fR-fB;
          for (int i4 = 0; i4 < 3; i4++)
          {
            fF = mElement[i4][i3+1];
            mElement[i4][i3+1] = fSin*mElement[i4][i3]+fCos*fF;
            mElement[i4][i3] = fCos*mElement[i4][i3]-fSin*fF;
          }
        }
        m_afDiag[i0] -= fP;
        m_afSubd[i0] = fG;
        m_afSubd[i2] = (float)0.0;
      }
      if (i1 == iMaxIter)
      {
        return false;
      }
    }
    return true;
  }

  void DecreasingSort(void)
  {
    //sort eigenvalues in decreasing order, e[0] >= ... >= e[iSize-1]
    for (int i0 = 0, i1; i0 <= 3-2; i0++)
    {
      // locate maximum eigenvalue
      i1 = i0;
      float fMax = m_afDiag[i1];
      int i2;
      for (i2 = i0+1; i2 < 3; i2++)
      {
        if (m_afDiag[i2] > fMax)
        {
          i1 = i2;
          fMax = m_afDiag[i1];
        }
      }

      if (i1 != i0)
      {
        // swap eigenvalues
        m_afDiag[i1] = m_afDiag[i0];
        m_afDiag[i0] = fMax;
        // swap eigenvectors
        for (i2 = 0; i2 < 3; i2++)
        {
          float fTmp = mElement[i2][i0];
          mElement[i2][i0] = mElement[i2][i1];
          mElement[i2][i1] = fTmp;
          m_bIsRotation = !m_bIsRotation;
        }
      }
    }
  }


  void GuaranteeRotation(void)
  {
    if (!m_bIsRotation)
    {
      // change sign on the first column
      for (int iRow = 0; iRow <3; iRow++)
      {
        mElement[iRow][0] = -mElement[iRow][0];
      }
    }
  }

  float mElement[3][3];
  float m_afDiag[3];
  float m_afSubd[3];
  bool m_bIsRotation;
};

}


using namespace BestFit;


bool getBestFitPlane(unsigned int vcount,
                     const float *points,
                     unsigned int vstride,
                     const float *weights,
                     unsigned int wstride,
                     float *plane)
{
  bool ret = false;

  Vec3 kOrigin(0,0,0);

  float wtotal = 0;

  if ( 1 )
  {
    const char *source  = (const char *) points;
    const char *wsource = (const char *) weights;

    for (unsigned int i=0; i<vcount; i++)
    {

      const float *p = (const float *) source;

      float w = 1;

      if ( wsource )
      {
        const float *ws = (const float *) wsource;
        w = *ws; //
        wsource+=wstride;
      }

      kOrigin.x+=p[0]*w;
      kOrigin.y+=p[1]*w;
      kOrigin.z+=p[2]*w;

      wtotal+=w;

      source+=vstride;
    }
  }

  float recip = 1.0f / wtotal; // reciprocol of total weighting

  kOrigin.x*=recip;
  kOrigin.y*=recip;
  kOrigin.z*=recip;


  float fSumXX=0;
  float fSumXY=0;
  float fSumXZ=0;

  float fSumYY=0;
  float fSumYZ=0;
  float fSumZZ=0;


  if ( 1 )
  {
    const char *source  = (const char *) points;
    const char *wsource = (const char *) weights;

    for (unsigned int i=0; i<vcount; i++)
    {

      const float *p = (const float *) source;

      float w = 1;

      if ( wsource )
      {
        const float *ws = (const float *) wsource;
        w = *ws; //
        wsource+=wstride;
      }

      Vec3 kDiff;

      kDiff.x = w*(p[0] - kOrigin.x); // apply vertex weighting!
      kDiff.y = w*(p[1] - kOrigin.y);
      kDiff.z = w*(p[2] - kOrigin.z);

      fSumXX+= kDiff.x * kDiff.x; // sume of the squares of the differences.
      fSumXY+= kDiff.x * kDiff.y; // sume of the squares of the differences.
      fSumXZ+= kDiff.x * kDiff.z; // sume of the squares of the differences.

      fSumYY+= kDiff.y * kDiff.y;
      fSumYZ+= kDiff.y * kDiff.z;
      fSumZZ+= kDiff.z * kDiff.z;


      source+=vstride;
    }
  }

  fSumXX *= recip;
  fSumXY *= recip;
  fSumXZ *= recip;
  fSumYY *= recip;
  fSumYZ *= recip;
  fSumZZ *= recip;

  // setup the eigensolver
  Eigen kES;

  kES.mElement[0][0] = fSumXX;
  kES.mElement[0][1] = fSumXY;
  kES.mElement[0][2] = fSumXZ;

  kES.mElement[1][0] = fSumXY;
  kES.mElement[1][1] = fSumYY;
  kES.mElement[1][2] = fSumYZ;

  kES.mElement[2][0] = fSumXZ;
  kES.mElement[2][1] = fSumYZ;
  kES.mElement[2][2] = fSumZZ;

  // compute eigenstuff, smallest eigenvalue is in last position
  kES.DecrSortEigenStuff();

  Vec3 kNormal;

  kNormal.x = kES.mElement[0][2];
  kNormal.y = kES.mElement[1][2];
  kNormal.z = kES.mElement[2][2];

  // the minimum energy
  plane[0] = kNormal.x;
  plane[1] = kNormal.y;
  plane[2] = kNormal.z;

  plane[3] = 0 - kNormal.dot(kOrigin);

  return ret;
}



float getBoundingRegion(unsigned int vcount,const float *points,unsigned int pstride,float *bmin,float *bmax) // returns the diagonal distance
{

  const unsigned char *source = (const unsigned char *) points;

	bmin[0] = points[0];
	bmin[1] = points[1];
	bmin[2] = points[2];

	bmax[0] = points[0];
	bmax[1] = points[1];
	bmax[2] = points[2];


  for (unsigned int i=1; i<vcount; i++)
  {
  	source+=pstride;
  	const float *p = (const float *) source;

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

	return sqrtf( dx*dx + dy*dy + dz*dz );

}


bool  overlapAABB(const float *bmin1,const float *bmax1,const float *bmin2,const float *bmax2) // return true if the two AABB's overlap.
{
  if ( bmax2[0] < bmin1[0] ) return false; // if the maximum is less than our minimum on any axis
  if ( bmax2[1] < bmin1[1] ) return false;
  if ( bmax2[2] < bmin1[2] ) return false;

  if ( bmin2[0] > bmax1[0] ) return false; // if the minimum is greater than our maximum on any axis
  if ( bmin2[1] > bmax1[1] ) return false; // if the minimum is greater than our maximum on any axis
  if ( bmin2[2] > bmax1[2] ) return false; // if the minimum is greater than our maximum on any axis


  return true; // the extents overlap
}


