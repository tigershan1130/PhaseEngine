//--------------------------------------------------------------------------------------
// File: Device.h
//
// Wraps device state management
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "RenderSurface.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "MeshObject.h"
#include "Effect.h"

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Frame stats
	//--------------------------------------------------------------------------------------
	struct FrameStatData
	{
		int MaterialChanges;
		int VBChanges;
		int IBChanges;
		int DrawCalls;
		int LightChanges;
		int PolysDrawn;
		int PolysProcessed;
		float ElapsedTime;
		int ProcessedMessages;
		int FPS;
		FrameStatData(){
			MaterialChanges=IBChanges=VBChanges=DrawCalls=FPS=LightChanges=PolysProcessed=PolysDrawn=ProcessedMessages=0;
		}

		inline void Reset(){
			MaterialChanges=IBChanges=VBChanges=DrawCalls=LightChanges=PolysProcessed=PolysDrawn=ProcessedMessages=0;
		}
	};
	
	
	
	//--------------------------------------------------------------------------------------
	// High level state management
	//--------------------------------------------------------------------------------------
	class Device
	{
	public:
		static Core::Material* Material;
		static Core::Light* Light;
		static ID3D10InputLayout* InputLayout;
		static D3D10_PRIMITIVE_TOPOLOGY Topology;
		static RenderSurface* Cubemap;
		static RenderSurface* ShadowMap;
		static RenderSurface* CubeShadowMap;
		static ID3D10Buffer* VertexBuffer;
		static ID3D10Buffer* IndexBuffer;
		static Effect* Effect;
		static FrameStatData FrameStats;
		static float ClearColor[4];

		static inline void Reset(){
			Material = NULL;
			Light = NULL;
			InputLayout = NULL;
			Cubemap = NULL;
			Topology = D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED;
			VertexBuffer = NULL;
			IndexBuffer = NULL;
			FrameStats.Reset();
		}


		//--------------------------------------------------------------------------------------
		// Caches the current RTV and DSV
		//--------------------------------------------------------------------------------------
		static inline void CacheRenderTarget(){ 
			g_pd3dDevice->OMGetRenderTargets(1, &m_pCachedRTV, &m_pCachedDSV);
			UINT cRT = 1;
			g_pd3dDevice->RSGetViewports(&cRT, &m_CachedVP);
		}

		//--------------------------------------------------------------------------------------
		// Restores the cached RTV and DSV
		//--------------------------------------------------------------------------------------
		static inline void RestoreCachedTargets(){	
			g_pd3dDevice->OMSetRenderTargets(1, &m_pCachedRTV, m_pCachedDSV); 
			g_pd3dDevice->RSSetViewports(1, &m_CachedVP);
		}

		//--------------------------------------------------------------------------------------
		// Clears the cached targets to the value of ClearColor
		//--------------------------------------------------------------------------------------
		static inline void ClearCachedTargets(){
			g_pd3dDevice->ClearRenderTargetView( m_pCachedRTV, ClearColor );
			g_pd3dDevice->ClearDepthStencilView( m_pCachedDSV, D3D10_CLEAR_DEPTH, 1.0, 0 );
		}

		//--------------------------------------------------------------------------------------
		// Sets the color used to the clear the backbuffer
		//--------------------------------------------------------------------------------------
		static inline void SetClearColor(float r, float g, float b, float a=0){
			ClearColor[0] = r; ClearColor[1] = g; ClearColor[2] = b; ClearColor[3]=a;
		}

		//--------------------------------------------------------------------------------------
		// Checks if a mesh within the camera view.  This does not take occulsion into account
		//--------------------------------------------------------------------------------------
		static inline bool IsMeshVisible(MeshObject& mesh, Camera& camera){
			return (camera.GetFrustum().CheckSphere( mesh.GetPos(), mesh.GetRadius()));
		}


		//--------------------------------------------------------------------------------------
		// Checks if a submesh within the camera view.  This does not take occulsion into account
		//--------------------------------------------------------------------------------------
		static inline bool IsMeshVisible(SubMesh& mesh, Camera& camera){
			return (camera.GetFrustum().CheckSphere( *mesh.pWorldPosition+mesh.localPosition, mesh.boundRadius));
		}


		//--------------------------------------------------------------------------------------
		// Render states
		//--------------------------------------------------------------------------------------
		static void SetEffect(Core::Effect& effect);
		static void SetInputLayout(ID3D10InputLayout* pInputLayout);
		static void SetMaterial(Core::Material& pMat);
		static void SetLight( Core::Light& light);
		static void SetVertexBuffer(ID3D10Buffer* pVB, UINT stride);
		static void SetIndexBuffer(ID3D10Buffer* pIB, DXGI_FORMAT size);
		static void SetCubeMap(RenderSurface* pCubeMap);
		static void SetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY type);



		//--------------------------------------------------------------------------------------
		// Draw a submesh
		//--------------------------------------------------------------------------------------
		static void DrawSubmesh(SubMesh& mesh);
		static void DrawSubmeshInstanced(SubMesh& mesh, int numInstances);
		static void DrawSubmeshPos(SubMesh& mesh);

	private:

		static ID3D10RenderTargetView* m_pCachedRTV;
		static ID3D10DepthStencilView* m_pCachedDSV;
		static D3D10_VIEWPORT		   m_CachedVP;			
	};
}