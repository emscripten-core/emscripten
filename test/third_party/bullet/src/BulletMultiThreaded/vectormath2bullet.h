/*
   Copyright (C) 2006, 2007 Sony Computer Entertainment Inc.
   All rights reserved.

   Redistribution and use in source and binary forms,
   with or without modification, are permitted provided that the
   following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Sony Computer Entertainment Inc nor the names
      of its contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BT_AOS_VECTORMATH_BULLET_CONVERT_H
#define BT_AOS_VECTORMATH_BULLET_CONVERT_H

#include "PlatformDefinitions.h"
#include "LinearMath/btVector3.h"
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btMatrix3x3.h"

inline Vectormath::Aos::Vector3	getVmVector3(const btVector3& bulletVec)
{
	return Vectormath::Aos::Vector3(bulletVec.getX(),bulletVec.getY(),bulletVec.getZ());
}

inline btVector3 getBtVector3(const Vectormath::Aos::Vector3& vmVec)
{
	return btVector3(vmVec.getX(),vmVec.getY(),vmVec.getZ());
}
inline btVector3 getBtVector3(const Vectormath::Aos::Point3& vmVec)
{
	return btVector3(vmVec.getX(),vmVec.getY(),vmVec.getZ());
}

inline Vectormath::Aos::Quat	getVmQuat(const btQuaternion& bulletQuat)
{
	Vectormath::Aos::Quat vmQuat(bulletQuat.getX(),bulletQuat.getY(),bulletQuat.getZ(),bulletQuat.getW());
	return vmQuat;
}

inline btQuaternion	getBtQuat(const Vectormath::Aos::Quat& vmQuat)
{
	return btQuaternion (vmQuat.getX(),vmQuat.getY(),vmQuat.getZ(),vmQuat.getW());
}

inline Vectormath::Aos::Matrix3	getVmMatrix3(const btMatrix3x3& btMat)
{
	Vectormath::Aos::Matrix3 mat(
		getVmVector3(btMat.getColumn(0)),
		getVmVector3(btMat.getColumn(1)),
		getVmVector3(btMat.getColumn(2)));
		return mat;
}


#endif //BT_AOS_VECTORMATH_BULLET_CONVERT_H
