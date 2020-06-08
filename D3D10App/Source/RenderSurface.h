//--------------------------------------------------------------------------------------
// File: RenderSurface.h
//
// Render target texture class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "DepthStencil.h"
#include "Texture.h"

namespace Core
{

	/// <summary>
	/// Simplifies the usage of render targets
	/// </summary>
	class RenderSurface
	{
	public:
		RenderSurface();

		/// <summary>
		/// Create a render target from an existing 2d texture
		/// </summary>
		/// <param name="pTex">Input Texture</param>
		/// <returns>S_OK if the call succeeded</returns>
		HRESULT CreateFromTexture(ID3D10Texture2D* pTex);
		
		/// <summary>
		/// Create a 2d render target
		/// </summary>
		/// <param name="width">Width of the render target</param>
		/// <param name="height">Height of the render target</param>
		/// <param name="format">Texture format to be used (see DXGI_FORMAT in the DX SDK)</param>
		/// <param name="mipLevels">Number of mipmap levels to create</param>
		/// <param name="pMSAA">Pointer to an MSAA format.  Use NULL for no multisampling</param>
		/// <param name="bRTV">A boolean indicating whether render target views should be generated</param>
		/// <returns>S_OK if the call succeeded</returns> 
		HRESULT Create(UINT width, UINT height, DXGI_FORMAT format, UINT mipLevels,  DXGI_SAMPLE_DESC* pMSAA, bool bRTV = true);


		/// <summary>
		/// Create a an array of 2d render targets
		/// </summary>
		/// <param name="width">Width of the render target</param>
		/// <param name="height">Height of the render target</param>
		/// <param name="format">Array of texture formats to be used.  This must contain arraySize entries. (see DXGI_FORMAT in the DX SDK)</param>
		/// <param name="arraySize">Number of render targets in the array</param>
		/// <param name="mipLevels">Number of mipmap levels to create</param>
		/// <param name="pMSAA">Pointer to an MSAA format.  Use NULL for no multisampling</param>
		/// <param name="bRTV">A boolean indicating whether render target views should be generated</param>
		/// <returns>S_OK if the call succeeded</returns> 
		HRESULT CreateArray(UINT width, UINT height, DXGI_FORMAT* format, UINT arraySize, UINT mipLevels,  DXGI_SAMPLE_DESC* pMSAA, bool bRTV = true);


		/// <summary>
		/// Create a cubemap render target
		/// </summary>
		/// <param name="width">Width of each cubemap face</param>
		/// <param name="height">Height of each cubemap face</param>
		/// <param name="format">Texture format to be used (see DXGI_FORMAT in the DX SDK)</param>
		/// <param name="mipLevels">Number of mipmap levels to create</param>
		/// <returns>S_OK if the call succeeded</returns> 
		HRESULT CreateCube(UINT width, UINT height, DXGI_FORMAT format, UINT mipLevels);

		/// <summary>
		/// Attach an effect variable
		/// </summary>
		/// <remarks>
		/// This method is useful when the texture surface will always use the same effect
		/// variable.  When BindTextures is called, it will use this effect variable to
		/// set the texture.
		/// </remarks>
		/// <param name="pVar">Pointer to an effect SRV that will bind to texture surface</param>
		inline void AttachEffectSRVVariable(ID3D10EffectShaderResourceVariable* pVar){ m_pEffectSRV = pVar; }

		// Attach a depth stencil
		inline void AttachDepthStencil(ID3D10DepthStencilView* pDSV){ m_pDSV = pDSV; }
		inline void AttachDepthStencil(DepthStencil& depth){ m_pDSV = depth.GetDSV(); }

		// Convert to a Texutre object
		inline void ToTexture(Texture& tex, char* name){
			tex.m_szName = name;
			tex.m_pTex = m_pTexSRV[0];
		}

		// Bind the SRV to the effect
		void BindTextures(UINT numResources = 0);

		// Resolve the MSAA target into a regular target
		void Resolve(RenderSurface& target);

		// Generate the mip maps
		inline void GenerateMips()
		{
			g_pd3dDevice->GenerateMips(m_pTexSRV[0]);
		}

		// Grab the frame buffer
		inline void CopyFrameBuffer(ID3D10RenderTargetView* pRTV)
		{
			ID3D10Resource* pRC = NULL;
			pRTV->GetResource(&pRC);
			g_pd3dDevice->CopyResource(GetTex()[0],pRC);
			pRC->Release();
		}

		// Grab the frame buffer
		inline void CopyTexture(RenderSurface& rs)
		{
			g_pd3dDevice->CopyResource(GetTex()[0], rs.GetTex()[0]);
		}

		// Bind the render target
		void BindRenderTarget(UINT numResources = 0);

		// Clear the render targets
		void Clear(UINT index = -1);
		void ClearDSV();

		// Set the clear color
		void SetClearColor(float r, float g, float b, float a);

		// TRUE if an array
		inline bool IsArray(){ return (m_ArraySize>1); }

		// TRUE if cubemap
		inline bool IsCube(){ return m_bCube; }

		// TRUE if created
		inline bool Exists(){ return m_pTex!=NULL; }

		// Free resources
		void Release();

		// Get functions
		inline ID3D10Texture2D** GetTex(){ return m_pTex; }
		inline ID3D10ShaderResourceView** GetSRV(){ return m_pTexSRV; }
		inline ID3D10RenderTargetView** GetRTV(){ return m_pTexRTV; }
		inline ID3D10DepthStencilView* GetDSV(){ return m_pDSV; }
		inline D3D10_VIEWPORT& GetViewport(){ return m_Viewport; }
		inline D3DXMATRIX& GetOrthoMatrix(){ return m_mOrtho; }

	private:
		
		// Textures
		ID3D10Texture2D**				m_pTex;
		ID3D10ShaderResourceView**		m_pTexSRV;	
		ID3D10RenderTargetView**		m_pTexRTV;

		// Effect variables
		ID3D10EffectShaderResourceVariable*	m_pEffectSRV;

		// Depth stencil target
		ID3D10DepthStencilView* m_pDSV;

		// Orthographic projection matrix and viewport
		D3DXMATRIX m_mOrtho;
		D3D10_VIEWPORT m_Viewport;

		//Props
		bool  m_bArray;
		bool  m_bCube;
		UINT  m_ArraySize;
		float m_ClearColor[4];
	};

}