#ifndef FLOAT_MATH_H

#define FLOAT_MATH_H

#ifdef _WIN32
	#pragma warning(disable : 4324) // disable padding warning
	#pragma warning(disable : 4244) // disable padding warning
	#pragma warning(disable : 4267) //  possible loss of data
	#pragma warning(disable:4530) // Disable the exception disable but used in MSCV Stl warning.
	#pragma warning(disable:4996) //Turn off warnings about deprecated C routines
	#pragma warning(disable:4786) // Disable the "debug name too long" warning
#endif

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


// a set of routines that last you do common 3d math
// operations without any vector, matrix, or quaternion
// classes or templates.
//
// a vector (or point) is a 'float *' to 3 floating point numbers.
// a matrix is a 'float *' to an array of 16 floating point numbers representing a 4x4 transformation matrix compatible with D3D or OGL
// a quaternion is a 'float *' to 4 floats representing a quaternion x,y,z,w

const float FM_PI = 3.141592654f;
const float FM_DEG_TO_RAD = ((2.0f * FM_PI) / 360.0f);
const float FM_RAD_TO_DEG = (360.0f / (2.0f * FM_PI));

void  fm_identity(float *matrix); // set 4x4 matrix to identity.
void  fm_inverseRT(const float *matrix,const float *pos,float *t); // inverse rotate translate the point.
void  fm_eulerMatrix(float ax,float ay,float az,float *matrix); // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)
void  fm_getAABB(unsigned int vcount,const float *points,unsigned int pstride,float *bmin,float *bmax);
void  fm_eulerToQuat(float roll,float pitch,float yaw,float *quat); // convert euler angles to quaternion.
void  fm_quatToMatrix(const float *quat,float *matrix); // convert quaterinion rotation to matrix, translation set to zero.
void  fm_quatRotate(const float *quat,const float *v,float *r); // rotate a vector directly by a quaternion.
void  fm_getTranslation(const float *matrix,float *t);
void  fm_matrixToQuat(const float *matrix,float *quat); // convert the 3x3 portion of a 4x4 matrix into a quaterion as x,y,z,w
float fm_sphereVolume(float radius); // return's the volume of a sphere of this radius (4/3 PI * R cubed )

#endif
