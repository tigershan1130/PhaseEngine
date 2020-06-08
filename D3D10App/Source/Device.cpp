//--------------------------------------------------------------------------------------
// File: Device.cpp
//
// Wraps device state management
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Device.h"

namespace Core
{
	ID3D10Buffer* Device::IndexBuffer;
	ID3D10Buffer* Device::VertexBuffer;
	Material* Device::Material;
	Light* Device::Light;
	ID3D10InputLayout* Device::InputLayout;
	D3D10_PRIMITIVE_TOPOLOGY Device::Topology;
	RenderSurface* Device::Cubemap;
	Effect* Device::Effect;
	FrameStatData Device::FrameStats;
	float Device::ClearColor[4];
	ID3D10RenderTargetView* Device::m_pCachedRTV;
	ID3D10DepthStencilView* Device::m_pCachedDSV;
	D3D10_VIEWPORT		   Device::m_CachedVP;


	//--------------------------------------------------------------------------------------
	// Change the active effect
	//--------------------------------------------------------------------------------------
	void Device::SetEffect(Core::Effect& effect)
	{
		Device::Effect = &effect;
	}


	//--------------------------------------------------------------------------------------
	// Changes the input layout
	//--------------------------------------------------------------------------------------
	void Device::SetInputLayout(ID3D10InputLayout* pInputLayout)
	{
		// State check
		if(pInputLayout == Device::InputLayout)
			return;
		Device::InputLayout = pInputLayout;

		g_pd3dDevice->IASetInputLayout(pInputLayout);
	}


	//--------------------------------------------------------------------------------------
	// Sets the primitive topology
	//--------------------------------------------------------------------------------------
	void Device::SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY type)
	{
		// State check
		if(type == Device::Topology)
			return;
		Device::Topology = type;

		g_pd3dDevice->IASetPrimitiveTopology(type);
	}


	//--------------------------------------------------------------------------------------
	// Set a cubemap
	//--------------------------------------------------------------------------------------
	void Device::SetCubeMap(RenderSurface* pCubeMap)
	{
		// Check state
		if(pCubeMap == Device::Cubemap)
			return;
		Device::Cubemap = pCubeMap;
		pCubeMap->BindTextures();
	}


	//--------------------------------------------------------------------------------------
	// Sets the material properties for a shader
	//--------------------------------------------------------------------------------------
	void Device::SetMaterial(Core::Material& material)
	{
		// State check
		if(&material != Device::Material)
		{
			Device::Material=&material;

			// Setup material property effect variables
			Device::Effect->SetMaterial(material);

#ifdef PHASE_DEBUG
			Device::FrameStats.MaterialChanges++;
#endif
		}
	}


	//--------------------------------------------------------------------------------------
	// Set the light properties for a shader
	//--------------------------------------------------------------------------------------
	void Device::SetLight( Core::Light& light)
	{
		// State check
		if(&light != Device::Light)

		{
			Device::Light=&light;

			// Setup light property effect variables
			Device::Effect->SetLight(light);
			
#ifdef PHASE_DEBUG
			Device::FrameStats.LightChanges++;
#endif
		}
	}


	//--------------------------------------------------------------------------------------
	// Change the vertex buffer
	//--------------------------------------------------------------------------------------
	void Device::SetVertexBuffer(ID3D10Buffer* pVB, UINT stride)
	{
		static UINT offset=0;
		if(Device::VertexBuffer != pVB)
		{
			Device::VertexBuffer = pVB;
			g_pd3dDevice->IASetVertexBuffers(0, 1, &pVB, &stride, &offset);
#ifdef PHASE_DEBUG
			Device::FrameStats.VBChanges++;
#endif
		}
	}


	//--------------------------------------------------------------------------------------
	// Change the index buffer
	//--------------------------------------------------------------------------------------
	void Device::SetIndexBuffer(ID3D10Buffer* pIB, DXGI_FORMAT size)
	{
		if(Device::IndexBuffer != pIB)
		{
			Device::IndexBuffer = pIB;
			g_pd3dDevice->IASetIndexBuffer(pIB, size, 0);
#ifdef PHASE_DEBUG
			Device::FrameStats.IBChanges++;
#endif
		}
	}



	//--------------------------------------------------------------------------------------
	// Draw a submesh
	//--------------------------------------------------------------------------------------
	void Device::DrawSubmesh(SubMesh& mesh)
	{
		// Set buffers
		static const UINT offset=0;
		if(mesh.isSkinned)
			SetVertexBuffer(mesh.pVertexBuffer, SkinnedVertex::size);
		else
			SetVertexBuffer(mesh.pVertexBuffer, Vertex::size);
		SetIndexBuffer(mesh.pIndexBuffer, DXGI_FORMAT_R32_UINT);

		// Draw to screen
		g_pd3dDevice->DrawIndexed( mesh.numIndices, mesh.startIndex, 0 );
		Device::FrameStats.PolysDrawn += mesh.numIndices/3;
		Device::FrameStats.DrawCalls++;
	}

	//--------------------------------------------------------------------------------------
	// Draw a submesh a number of times
	//--------------------------------------------------------------------------------------
	void Device::DrawSubmeshInstanced(SubMesh& mesh, int numInstances)
	{
		// Set buffers
		static const UINT offset=0;
		if(mesh.isSkinned)
			SetVertexBuffer(mesh.pVertexBuffer, SkinnedVertex::size);
		else
			SetVertexBuffer(mesh.pVertexBuffer, Vertex::size);
		SetIndexBuffer(mesh.pIndexBuffer, DXGI_FORMAT_R32_UINT);

		// Draw to screen
		g_pd3dDevice->DrawIndexedInstanced( mesh.numIndices, numInstances, mesh.startIndex, 0, 0 );
	}


	//--------------------------------------------------------------------------------------
	// Draw a submesh, position buffer only
	//--------------------------------------------------------------------------------------
	void Device::DrawSubmeshPos(SubMesh& mesh)
	{
		// Set buffers
		static const UINT offset=0;
		SetVertexBuffer(mesh.pPosVertexBuffer, PosVertex::size);
		SetIndexBuffer(mesh.pIndexBuffer, DXGI_FORMAT_R32_UINT);

		// Draw to screen
		g_pd3dDevice->DrawIndexed( mesh.numIndices, mesh.startIndex, 0 );
	}
}