/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#include "BulletMultiThreaded/GpuSoftBodySolvers/CPU/btSoftBodySolverData.h"
#include "btSoftBodySolverBuffer_DX11.h"


#ifndef BT_SOFT_BHODY_SOLVER_VERTEX_DATA_DX11_H
#define BT_SOFT_BHODY_SOLVER_VERTEX_DATA_DX11_H

class btSoftBodyLinkData;
class btSoftBodyLinkData::LinkDescription;

struct ID3D11Device;
struct ID3D11DeviceContext;

class btSoftBodyVertexDataDX11 : public btSoftBodyVertexData
{
protected:
	bool				m_onGPU;
	ID3D11Device		*m_d3dDevice;
	ID3D11DeviceContext *m_d3dDeviceContext;

public:
	btDX11Buffer<int>										m_dx11ClothIdentifier;
	btDX11Buffer<Vectormath::Aos::Point3>					m_dx11VertexPosition;
	btDX11Buffer<Vectormath::Aos::Point3>					m_dx11VertexPreviousPosition;
	btDX11Buffer<Vectormath::Aos::Vector3>				m_dx11VertexVelocity;
	btDX11Buffer<Vectormath::Aos::Vector3>				m_dx11VertexForceAccumulator;
	btDX11Buffer<Vectormath::Aos::Vector3>				m_dx11VertexNormal;
	btDX11Buffer<float>									m_dx11VertexInverseMass;
	btDX11Buffer<float>									m_dx11VertexArea;
	btDX11Buffer<int>										m_dx11VertexTriangleCount;


	//ID3D11Buffer*               readBackBuffer;

public:
	btSoftBodyVertexDataDX11( ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dDeviceContext );
	virtual ~btSoftBodyVertexDataDX11();

	virtual bool onAccelerator();
	virtual bool moveToAccelerator();

	virtual bool moveFromAccelerator();
};


#endif // #ifndef BT_SOFT_BHODY_SOLVER_VERTEX_DATA_DX11_H

