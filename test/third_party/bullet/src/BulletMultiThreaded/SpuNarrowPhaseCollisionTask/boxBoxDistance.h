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


#ifndef __BOXBOXDISTANCE_H__
#define __BOXBOXDISTANCE_H__


#include "Box.h"


//---------------------------------------------------------------------------
// boxBoxDistance:
//
// description:
//    this computes info that can be used for the collision response of two boxes.  when the boxes
//    do not overlap, the points are set to the closest points of the boxes, and a positive
//    distance between them is returned.  if the boxes do overlap, a negative distance is returned
//    and the points are set to two points that would touch after the boxes are translated apart.
//    the contact normal gives the direction to repel or separate the boxes when they touch or
//    overlap (it's being approximated here as one of the 15 "separating axis" directions).
//
// returns:
//    positive or negative distance between two boxes.
//
// args:
//    vmVector3& normal: set to a unit contact normal pointing from box A to box B.
//
//    BoxPoint& boxPointA, BoxPoint& boxPointB:
//       set to a closest point or point of penetration on each box.
//
//    Box boxA, Box boxB:
//       boxes, represented as 3 half-widths
//
//    const vmTransform3& transformA, const vmTransform3& transformB:
//       box transformations, in world coordinates
//
//    float distanceThreshold:
//       the algorithm will exit early if it finds that the boxes are more distant than this
//       threshold, and not compute a contact normal or points.  if this distance returned
//       exceeds the threshold, all the other output data may not have been computed.  by
//       default, this is set to MAX_FLOAT so it will have no effect.
//
//---------------------------------------------------------------------------

float
boxBoxDistance(vmVector3& normal, BoxPoint& boxPointA, BoxPoint& boxPointB,
			   PE_REF(Box) boxA, const vmTransform3 & transformA, PE_REF(Box) boxB,
			   const vmTransform3 & transformB,
			   float distanceThreshold = FLT_MAX );

#endif /* __BOXBOXDISTANCE_H__ */
