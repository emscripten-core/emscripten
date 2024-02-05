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
#ifndef BT_SOFT_BODY_SOLVER_BUFFER_DX11_H
#define BT_SOFT_BODY_SOLVER_BUFFER_DX11_H

// DX11 support
#include <windows.h>
#include <crtdbg.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <d3dcompiler.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

/**
 * DX11 Buffer that tracks a host buffer on use to ensure size-correctness.
 */
template <typename ElementType> class btDX11Buffer
{
protected:
	ID3D11Device*				m_d3dDevice;
	ID3D11DeviceContext*		m_d3dDeviceContext;

	ID3D11Buffer*               m_Buffer;
	ID3D11ShaderResourceView*   m_SRV;
	ID3D11UnorderedAccessView*  m_UAV;
	btAlignedObjectArray< ElementType >*	m_CPUBuffer;

	// TODO: Separate this from the main class
	// as read back buffers can be shared between buffers
	ID3D11Buffer*               m_readBackBuffer;

	int m_gpuSize;
	bool m_onGPU;

	bool m_readOnlyOnGPU;
	
	bool createBuffer( ID3D11Buffer *preexistingBuffer = 0)
	{
		HRESULT hr = S_OK;

		// Create all CS buffers
		if( preexistingBuffer )
		{
			m_Buffer = preexistingBuffer;
		} else {
			D3D11_BUFFER_DESC buffer_desc;
			ZeroMemory(&buffer_desc, sizeof(buffer_desc));		
			buffer_desc.Usage = D3D11_USAGE_DEFAULT;
			if( m_readOnlyOnGPU )
				buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			else
				buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
			buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
			
			buffer_desc.ByteWidth = m_CPUBuffer->size() * sizeof(ElementType);
			// At a minimum the buffer must exist
			if( buffer_desc.ByteWidth == 0 )
				buffer_desc.ByteWidth = sizeof(ElementType);
			buffer_desc.StructureByteStride = sizeof(ElementType);
			hr = m_d3dDevice->CreateBuffer(&buffer_desc, NULL, &m_Buffer);
			if( FAILED( hr ) )
		        return (hr==S_OK);
		} 

		if( m_readOnlyOnGPU )
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC srvbuffer_desc;
			ZeroMemory(&srvbuffer_desc, sizeof(srvbuffer_desc));
			srvbuffer_desc.Format = DXGI_FORMAT_UNKNOWN;
			srvbuffer_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;

			srvbuffer_desc.Buffer.ElementWidth = m_CPUBuffer->size();
			if( srvbuffer_desc.Buffer.ElementWidth == 0 )
				srvbuffer_desc.Buffer.ElementWidth = 1;
			hr = m_d3dDevice->CreateShaderResourceView(m_Buffer, &srvbuffer_desc, &m_SRV);
			if( FAILED( hr ) )
				return (hr==S_OK);
		} else {
			// Create SRV
			D3D11_SHADER_RESOURCE_VIEW_DESC srvbuffer_desc;
			ZeroMemory(&srvbuffer_desc, sizeof(srvbuffer_desc));
			srvbuffer_desc.Format = DXGI_FORMAT_UNKNOWN;
			srvbuffer_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;

			srvbuffer_desc.Buffer.ElementWidth = m_CPUBuffer->size();
			if( srvbuffer_desc.Buffer.ElementWidth == 0 )
				srvbuffer_desc.Buffer.ElementWidth = 1;
			hr = m_d3dDevice->CreateShaderResourceView(m_Buffer, &srvbuffer_desc, &m_SRV);
			if( FAILED( hr ) )
				return (hr==S_OK);

			// Create UAV
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavbuffer_desc;
			ZeroMemory(&uavbuffer_desc, sizeof(uavbuffer_desc));
			uavbuffer_desc.Format = DXGI_FORMAT_UNKNOWN;
			uavbuffer_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;

			uavbuffer_desc.Buffer.NumElements = m_CPUBuffer->size();
			if( uavbuffer_desc.Buffer.NumElements == 0 )
				uavbuffer_desc.Buffer.NumElements = 1;
			hr = m_d3dDevice->CreateUnorderedAccessView(m_Buffer, &uavbuffer_desc, &m_UAV);
			if( FAILED( hr ) )
				return (hr==S_OK);

			// Create read back buffer
			D3D11_BUFFER_DESC readback_buffer_desc;
			ZeroMemory(&readback_buffer_desc, sizeof(readback_buffer_desc));

			readback_buffer_desc.ByteWidth = m_CPUBuffer->size() * sizeof(ElementType);
			readback_buffer_desc.Usage = D3D11_USAGE_STAGING;
			readback_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			readback_buffer_desc.StructureByteStride = sizeof(ElementType);
			hr = m_d3dDevice->CreateBuffer(&readback_buffer_desc, NULL, &m_readBackBuffer);
			if( FAILED( hr ) )
				return (hr==S_OK);
		}

		m_gpuSize = m_CPUBuffer->size();
		return true;
	}



public:
	btDX11Buffer( ID3D11Device *d3dDevice, ID3D11DeviceContext *d3dDeviceContext, btAlignedObjectArray< ElementType > *CPUBuffer, bool readOnly )
	{
		m_d3dDevice = d3dDevice;
		m_d3dDeviceContext = d3dDeviceContext;
		m_Buffer = 0;
		m_SRV = 0;
		m_UAV = 0;
		m_readBackBuffer = 0;

		m_CPUBuffer = CPUBuffer;

		m_gpuSize = 0;
		m_onGPU = false;

		m_readOnlyOnGPU = readOnly;
	}

	virtual ~btDX11Buffer()
	{
		SAFE_RELEASE(m_Buffer);
		SAFE_RELEASE(m_SRV);
		SAFE_RELEASE(m_UAV);
		SAFE_RELEASE(m_readBackBuffer);
	}

	ID3D11ShaderResourceView* &getSRV()
	{
		return m_SRV;
	}

	ID3D11UnorderedAccessView* &getUAV()
	{
		return m_UAV;
	}

	ID3D11Buffer* &getBuffer()
	{
		return m_Buffer;
	}

	/**
	 * Move the data to the GPU if it is not there already.
	 */
	bool moveToGPU()
	{
		// Reallocate if GPU size is too small
		if( (m_CPUBuffer->size() > m_gpuSize ) )
			m_onGPU = false;
		if( !m_onGPU && m_CPUBuffer->size() > 0 )
		{
			// If the buffer doesn't exist or the CPU-side buffer has changed size, create
			// We should really delete the old one, too, but let's leave that for later
			if( !m_Buffer || (m_CPUBuffer->size() != m_gpuSize) )
			{
				SAFE_RELEASE(m_Buffer);
				SAFE_RELEASE(m_SRV);
				SAFE_RELEASE(m_UAV);
				SAFE_RELEASE(m_readBackBuffer);
				if( !createBuffer() )
				{
					btAssert("Buffer creation failed.");
					return false;
				}
			}

			if( m_gpuSize > 0 )
			{
				D3D11_BOX destRegion;
				destRegion.left = 0;
				destRegion.front = 0;
				destRegion.top = 0;
				destRegion.bottom = 1;
				destRegion.back = 1;
				destRegion.right = (m_CPUBuffer->size())*sizeof(ElementType);
				m_d3dDeviceContext->UpdateSubresource(m_Buffer, 0, &destRegion, &((*m_CPUBuffer)[0]), 0, 0);

				m_onGPU = true;
			}

		}

		return true;
	}

	/**
	 * Move the data back from the GPU if it is on there and isn't read only.
	 */
	bool moveFromGPU()
	{
		if( m_CPUBuffer->size() > 0 )
		{
			if( m_onGPU && !m_readOnlyOnGPU )
			{
				// Copy back
				D3D11_MAPPED_SUBRESOURCE MappedResource = {0}; 
				//m_pd3dImmediateContext->CopyResource(m_phAngVelReadBackBuffer, m_phAngVel);

				D3D11_BOX destRegion;	
				destRegion.left = 0;
				destRegion.front = 0;
				destRegion.top = 0;
				destRegion.bottom = 1;
				destRegion.back = 1;

				destRegion.right = (m_CPUBuffer->size())*sizeof(ElementType);
				m_d3dDeviceContext->CopySubresourceRegion(
					m_readBackBuffer,
					0,
					0,
					0,
					0 ,
					m_Buffer,
					0,
					&destRegion
					);

				m_d3dDeviceContext->Map(m_readBackBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);   
				//memcpy(m_hAngVel, MappedResource.pData, (m_maxObjs * sizeof(float) ));
				memcpy(&((*m_CPUBuffer)[0]), MappedResource.pData, ((m_CPUBuffer->size()) * sizeof(ElementType) ));		
				m_d3dDeviceContext->Unmap(m_readBackBuffer, 0);

				m_onGPU = false;
			}
		}

		return true;
	}


	/**
	 * Copy the data back from the GPU without changing its state to be CPU-side.
	 * Useful if we just want to view it on the host for visualization.
	 */
	bool copyFromGPU()
	{
		if( m_CPUBuffer->size() > 0 )
		{
			if( m_onGPU && !m_readOnlyOnGPU )
			{
				// Copy back
				D3D11_MAPPED_SUBRESOURCE MappedResource = {0}; 

				D3D11_BOX destRegion;	
				destRegion.left = 0;
				destRegion.front = 0;
				destRegion.top = 0;
				destRegion.bottom = 1;
				destRegion.back = 1;

				destRegion.right = (m_CPUBuffer->size())*sizeof(ElementType);
				m_d3dDeviceContext->CopySubresourceRegion(
					m_readBackBuffer,
					0,
					0,
					0,
					0 ,
					m_Buffer,
					0,
					&destRegion
					);

				m_d3dDeviceContext->Map(m_readBackBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);   
				//memcpy(m_hAngVel, MappedResource.pData, (m_maxObjs * sizeof(float) ));
				memcpy(&((*m_CPUBuffer)[0]), MappedResource.pData, ((m_CPUBuffer->size()) * sizeof(ElementType) ));		
				m_d3dDeviceContext->Unmap(m_readBackBuffer, 0);
			}
		}

		return true;
	}

	/**
	 * Call if data has changed on the CPU.
	 * Can then trigger a move to the GPU as necessary.
	 */
	virtual void changedOnCPU()
	{
		m_onGPU = false;
	}
}; // class btDX11Buffer



#endif // #ifndef BT_SOFT_BODY_SOLVER_BUFFER_DX11_H