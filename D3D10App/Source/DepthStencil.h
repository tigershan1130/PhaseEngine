//--------------------------------------------------------------------------------------
// File: DepthStencil.h
//
// Depth stencil texture class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once


namespace Core
{

	// No clue why this is needed, visual studio is retarded
	extern ID3D10Device* g_pd3dDevice;

	// Manages a depth stencil view
	//  Typically used with the RenderSurface class
	class DepthStencil
	{
	public:
		DepthStencil();

		//
		// Create the depth targets
		//
		HRESULT Create(UINT width, UINT height, DXGI_SAMPLE_DESC* pMSAA, bool asTexture);
		HRESULT CreateCube(UINT width, UINT height);

		// Attach an effect variable to bind this as a texture
		inline void AttachEffectSRVVariable(ID3D10EffectShaderResourceVariable* pVar){ m_pEffectSRV = pVar; }

		// Bind the SRV to the effect
		inline void BindTextures(){ m_pEffectSRV->SetResource(m_pSRV);	}
		
		// Clear the SRV
		inline void UnbindTextures(){ m_pEffectSRV->SetResource(NULL);	}

		// Clear the dsv
		inline void Clear(){ g_pd3dDevice->ClearDepthStencilView(m_pDSV, D3D10_CLEAR_DEPTH, 1.0, 0 ); }


		// Free resources
		void Release();

		// Get functions
		inline ID3D10ShaderResourceView* GetSRV(){ return m_pSRV; }
		inline ID3D10DepthStencilView* GetDSV(){ return m_pDSV; }

	private:

		// Textures
		ID3D10Texture2D*				m_pTex;
		ID3D10ShaderResourceView*		m_pSRV;	
		ID3D10DepthStencilView*			m_pDSV;
		
		// Effect variables
		ID3D10EffectShaderResourceVariable*	m_pEffectSRV;
	};

}