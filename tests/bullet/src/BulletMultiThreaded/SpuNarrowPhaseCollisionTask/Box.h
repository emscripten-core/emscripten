/*
   Copyright (C) 2006, 2008 Sony Computer Entertainment Inc.
   All rights reserved.

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

*/

#ifndef __BOX_H__
#define __BOX_H__


#ifndef PE_REF
#define PE_REF(a) a&
#endif

#include <math.h>


#include "../PlatformDefinitions.h"




enum FeatureType { F, E, V };

//----------------------------------------------------------------------------
// Box
//----------------------------------------------------------------------------
///The Box is an internal class used by the boxBoxDistance calculation.
class Box
{
public:
	vmVector3 mHalf;

	inline Box()
	{}
	inline Box(PE_REF(vmVector3) half_);
	inline Box(float hx, float hy, float hz);

	inline void Set(PE_REF(vmVector3) half_);
	inline void Set(float hx, float hy, float hz);

	inline vmVector3 GetAABB(const vmMatrix3& rotation) const;
};

inline
Box::Box(PE_REF(vmVector3) half_)
{
	Set(half_);
}

inline
Box::Box(float hx, float hy, float hz)
{
	Set(hx, hy, hz);
}

inline
void
Box::Set(PE_REF(vmVector3) half_)
{
	mHalf = half_;
}

inline
void
Box::Set(float hx, float hy, float hz)
{
	mHalf = vmVector3(hx, hy, hz);
}

inline
vmVector3
Box::GetAABB(const vmMatrix3& rotation) const
{
	return absPerElem(rotation) * mHalf;
}

//-------------------------------------------------------------------------------------------------
// BoxPoint
//-------------------------------------------------------------------------------------------------

///The BoxPoint class is an internally used class to contain feature information for boxBoxDistance calculation.
class BoxPoint
{
public:
	BoxPoint() : localPoint(0.0f) {}

	vmPoint3      localPoint;
	FeatureType featureType;
	int         featureIdx;

	inline void setVertexFeature(int plusX, int plusY, int plusZ);
	inline void setEdgeFeature(int dim0, int plus0, int dim1, int plus1);
	inline void setFaceFeature(int dim, int plus);

	inline void getVertexFeature(int & plusX, int & plusY, int & plusZ) const;
	inline void getEdgeFeature(int & dim0, int & plus0, int & dim1, int & plus1) const;
	inline void getFaceFeature(int & dim, int & plus) const;
};

inline
void
BoxPoint::setVertexFeature(int plusX, int plusY, int plusZ)
{
	featureType = V;
	featureIdx = plusX << 2 | plusY << 1 | plusZ;
}

inline
void
BoxPoint::setEdgeFeature(int dim0, int plus0, int dim1, int plus1)
{
	featureType = E;

	if (dim0 > dim1) {
		featureIdx = plus1 << 5 | dim1 << 3 | plus0 << 2 | dim0;
	} else {
		featureIdx = plus0 << 5 | dim0 << 3 | plus1 << 2 | dim1;
	}
}

inline
void
BoxPoint::setFaceFeature(int dim, int plus)
{
	featureType = F;
	featureIdx = plus << 2 | dim;
}

inline
void
BoxPoint::getVertexFeature(int & plusX, int & plusY, int & plusZ) const
{
	plusX = featureIdx >> 2;
	plusY = featureIdx >> 1 & 1;
	plusZ = featureIdx & 1;
}

inline
void
BoxPoint::getEdgeFeature(int & dim0, int & plus0, int & dim1, int & plus1) const
{
	plus0 = featureIdx >> 5;
	dim0 = featureIdx >> 3 & 3;
	plus1 = featureIdx >> 2 & 1;
	dim1 = featureIdx & 3;
}

inline
void
BoxPoint::getFaceFeature(int & dim, int & plus) const
{
	plus = featureIdx >> 2;
	dim = featureIdx & 3;
}

#endif /* __BOX_H__ */
