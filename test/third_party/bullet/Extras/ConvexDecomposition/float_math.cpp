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

void fm_inverseRT(const float *matrix,const float *pos,float *t) // inverse rotate translate the point.
{

	float _x = pos[0] - matrix[3*4+0];
	float _y = pos[1] - matrix[3*4+1];
	float _z = pos[2] - matrix[3*4+2];

	// Multiply inverse-translated source vector by inverted rotation transform

	t[0] = (matrix[0*4+0] * _x) + (matrix[0*4+1] * _y) + (matrix[0*4+2] * _z);
	t[1] = (matrix[1*4+0] * _x) + (matrix[1*4+1] * _y) + (matrix[1*4+2] * _z);
	t[2] = (matrix[2*4+0] * _x) + (matrix[2*4+1] * _y) + (matrix[2*4+2] * _z);

}


void fm_identity(float *matrix) // set 4x4 matrix to identity.
{
	matrix[0*4+0] = 1;
	matrix[1*4+1] = 1;
	matrix[2*4+2] = 1;
	matrix[3*4+3] = 1;

	matrix[1*4+0] = 0;
	matrix[2*4+0] = 0;
	matrix[3*4+0] = 0;

	matrix[0*4+1] = 0;
	matrix[2*4+1] = 0;
	matrix[3*4+1] = 0;

	matrix[0*4+2] = 0;
	matrix[1*4+2] = 0;
	matrix[3*4+2] = 0;

	matrix[0*4+3] = 0;
	matrix[1*4+3] = 0;
	matrix[2*4+3] = 0;

}

void fm_eulerMatrix(float ax,float ay,float az,float *matrix) // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)
{
  float quat[4];
  fm_eulerToQuat(ax,ay,az,quat);
  fm_quatToMatrix(quat,matrix);
}

void fm_getAABB(unsigned int vcount,const float *points,unsigned int pstride,float *bmin,float *bmax)
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
}


void fm_eulerToQuat(float roll,float pitch,float yaw,float *quat) // convert euler angles to quaternion.
{
	roll  *= 0.5f;
	pitch *= 0.5f;
	yaw   *= 0.5f;

	float cr = cosf(roll);
	float cp = cosf(pitch);
	float cy = cosf(yaw);

	float sr = sinf(roll);
	float sp = sinf(pitch);
	float sy = sinf(yaw);

	float cpcy = cp * cy;
	float spsy = sp * sy;
	float spcy = sp * cy;
	float cpsy = cp * sy;

	quat[0]   = ( sr * cpcy - cr * spsy);
	quat[1]   = ( cr * spcy + sr * cpsy);
	quat[2]   = ( cr * cpsy - sr * spcy);
	quat[3]   = cr * cpcy + sr * spsy;
}

void fm_quatToMatrix(const float *quat,float *matrix) // convert quaterinion rotation to matrix, zeros out the translation component.
{

	float xx = quat[0]*quat[0];
	float yy = quat[1]*quat[1];
	float zz = quat[2]*quat[2];
	float xy = quat[0]*quat[1];
	float xz = quat[0]*quat[2];
	float yz = quat[1]*quat[2];
	float wx = quat[3]*quat[0];
	float wy = quat[3]*quat[1];
	float wz = quat[3]*quat[2];

	matrix[0*4+0] = 1 - 2 * ( yy + zz );
	matrix[1*4+0] =     2 * ( xy - wz );
	matrix[2*4+0] =     2 * ( xz + wy );

	matrix[0*4+1] =     2 * ( xy + wz );
	matrix[1*4+1] = 1 - 2 * ( xx + zz );
	matrix[2*4+1] =     2 * ( yz - wx );

	matrix[0*4+2] =     2 * ( xz - wy );
	matrix[1*4+2] =     2 * ( yz + wx );
	matrix[2*4+2] = 1 - 2 * ( xx + yy );

	matrix[3*4+0] = matrix[3*4+1] = matrix[3*4+2] = 0.0f;
	matrix[0*4+3] = matrix[1*4+3] = matrix[2*4+3] = 0.0f;
	matrix[3*4+3] = 1.0f;

}


void fm_quatRotate(const float *quat,const float *v,float *r) // rotate a vector directly by a quaternion.
{
  float left[4];

	left[0] =   quat[3]*v[0] + quat[1]*v[2] - v[1]*quat[2];
	left[1] =   quat[3]*v[1] + quat[2]*v[0] - v[2]*quat[0];
	left[2] =   quat[3]*v[2] + quat[0]*v[1] - v[0]*quat[1];
	left[3] = - quat[0]*v[0] - quat[1]*v[1] - quat[2]*v[2];

	r[0] = (left[3]*-quat[0]) + (quat[3]*left[0]) + (left[1]*-quat[2]) - (-quat[1]*left[2]);
	r[1] = (left[3]*-quat[1]) + (quat[3]*left[1]) + (left[2]*-quat[0]) - (-quat[2]*left[0]);
	r[2] = (left[3]*-quat[2]) + (quat[3]*left[2]) + (left[0]*-quat[1]) - (-quat[0]*left[1]);

}


void fm_getTranslation(const float *matrix,float *t)
{
	t[0] = matrix[3*4+0];
	t[1] = matrix[3*4+1];
	t[2] = matrix[3*4+2];
}

void fm_matrixToQuat(const float *matrix,float *quat) // convert the 3x3 portion of a 4x4 matrix into a quaterion as x,y,z,w
{

	float tr = matrix[0*4+0] + matrix[1*4+1] + matrix[2*4+2];

	// check the diagonal

	if (tr > 0.0f )
	{
		float s = (float) sqrt ( (double) (tr + 1.0f) );
		quat[3] = s * 0.5f;
		s = 0.5f / s;
		quat[0] = (matrix[1*4+2] - matrix[2*4+1]) * s;
		quat[1] = (matrix[2*4+0] - matrix[0*4+2]) * s;
		quat[2] = (matrix[0*4+1] - matrix[1*4+0]) * s;

	}
	else
	{
		// diagonal is negative
		int nxt[3] = {1, 2, 0};
		float  qa[4];

		int i = 0;

		if (matrix[1*4+1] > matrix[0*4+0]) i = 1;
		if (matrix[2*4+2] > matrix[i*4+i]) i = 2;

		int j = nxt[i];
		int k = nxt[j];

		float s = sqrtf ( ((matrix[i*4+i] - (matrix[j*4+j] + matrix[k*4+k])) + 1.0f) );

		qa[i] = s * 0.5f;

		if (s != 0.0f ) s = 0.5f / s;

		qa[3] = (matrix[j*4+k] - matrix[k*4+j]) * s;
		qa[j] = (matrix[i*4+j] + matrix[j*4+i]) * s;
		qa[k] = (matrix[i*4+k] + matrix[k*4+i]) * s;

		quat[0] = qa[0];
		quat[1] = qa[1];
		quat[2] = qa[2];
		quat[3] = qa[3];
	}


}


float fm_sphereVolume(float radius) // return's the volume of a sphere of this radius (4/3 PI * R cubed )
{
	return (4.0f / 3.0f ) * FM_PI * radius * radius * radius;
}
