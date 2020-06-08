//--------------------------------------------------------------------------------------
// File: HDR.h
//
// HDR System.  Handles buffer creation and HDR post-processing
//
// Coded by Nate Orr 2008
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once
#include "stdafx.h"
#include "Effect.h"

namespace Core
{

	class HDRSystem
	{
	public:
		HDRSystem(){
			m_CurrentLum = 1;
			m_OldLum = 0;
		}

		// Create the final scene image with HDR post-processing
		void ProcessHDRI(ID3D10RenderTargetView* pBackBuffer);			
		
		// Setup the hdr system
		HRESULT Create(UINT width, UINT height, ID3D10DepthStencilView* pDSV, Effect* pEffect);

		// Bind the effect variables
		void BindEffectVariables(Effect* pEffect);
		
		// Free all hdr related resources
		void Release();		

		inline RenderSurface& Buffer(){ return m_HDRBuffer; }
		
	private:

		// Compute the gaussian offsets and weights, and send them to the shader
		void ComputeGaussians(UINT width, UINT height); 
		
		RenderSurface				m_HDRBuffer;			// RT to store the 128-bit hdr scene rendering
		Array<RenderSurface>		m_HDRScaledLuminance;	// Stores the luminance data, each scaled RT is 1/2 the original size
		Array<DepthStencil>			m_ScaledLuminanceDepth;
		RenderSurface				m_HDRLuminance[2];		// Luminance texture
		DepthStencil				m_LuminanceDepth;
		RenderSurface				m_HDRBloom[2];			// 1/4 size bloom render targets
		DepthStencil				m_BloomDepth;
		int							m_CurrentLum;			// Current luminance texture
		int							m_OldLum;				// Lum from last frame
		Effect*						m_pEffect;				// Parent effect
		UINT						m_Width;				// Buffer size
		UINT						m_Height;				// Buffer size
	};

}