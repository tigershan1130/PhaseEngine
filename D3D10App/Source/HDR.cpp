//--------------------------------------------------------------------------------------
// File: HDR.cpp
//
// HDR System.  Handles buffer creation and HDR post-processing
//
// Coded by Nate Orr 2008
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "HDR.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Setup the hdr system
	//--------------------------------------------------------------------------------------
	HRESULT HDRSystem::Create(UINT width, UINT height, ID3D10DepthStencilView* pDSV, Effect* pEffect)
	{
		m_Width = width;
		m_Height = height;
		
		// Screen sized hdr buffer
		m_HDRBuffer.Create(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, NULL);

		// Get the max power of 2 number that is <= buffersize/4
		D3DXVECTOR2 bufferSize = D3DXVECTOR2(width/4, height/4);
		int count=0;
		while(bufferSize.x>2.0f && bufferSize.y>2.0f)
		{
			count++;
			bufferSize *= 0.5f;
		}
		
		// Create a series of scaled luminance buffers
		m_HDRScaledLuminance.Allocate(count);
		m_ScaledLuminanceDepth.Allocate(count);
		int pow2 = 2;
		for(int i=count-1; i>=2; i--)
		{
			m_HDRScaledLuminance[i].Create(width/pow2, height/pow2, DXGI_FORMAT_R32G32_FLOAT, 1, NULL);
			m_ScaledLuminanceDepth[i].Create(width/pow2, height/pow2, NULL, false);
			m_HDRScaledLuminance[i].AttachDepthStencil(m_ScaledLuminanceDepth[i]);
			pow2 *= 2;
		}
		m_HDRScaledLuminance[1].Create(4, 4, DXGI_FORMAT_R32G32_FLOAT, 1, NULL);
		m_ScaledLuminanceDepth[1].Create(4, 4, NULL, false);
		m_HDRScaledLuminance[1].AttachDepthStencil(m_ScaledLuminanceDepth[1]);
		m_HDRScaledLuminance[0].Create(2, 2, DXGI_FORMAT_R32G32_FLOAT, 1, NULL);
		m_ScaledLuminanceDepth[0].Create(2, 2, NULL, false);
		m_HDRScaledLuminance[0].AttachDepthStencil(m_ScaledLuminanceDepth[0]);
		

		// Luminance buffers
		m_HDRLuminance[0].Create(1, 1, DXGI_FORMAT_R32G32_FLOAT, 1, NULL);
		m_HDRLuminance[1].Create(1, 1, DXGI_FORMAT_R32G32_FLOAT, 1, NULL);
		m_LuminanceDepth.Create(1, 1, NULL, false);
		m_HDRLuminance[0].AttachDepthStencil(m_LuminanceDepth);
		m_HDRLuminance[1].AttachDepthStencil(m_LuminanceDepth);

		// Bloom buffers
		m_HDRBloom[0].Create(width/2, height/2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, NULL);
		m_HDRBloom[1].Create(width/2, height/2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, NULL);
		m_BloomDepth.Create(width/2, height/2, NULL, false);
		m_HDRBloom[0].AttachDepthStencil(m_BloomDepth);
		m_HDRBloom[1].AttachDepthStencil(m_BloomDepth);

		// Bind depth buffers
		m_HDRBuffer.AttachDepthStencil(pDSV);

		// Bind the effect vars
		BindEffectVariables(pEffect);

		return S_OK;
	}

	//--------------------------------------------------------------------------------------
	// Bind the effect variables
	//--------------------------------------------------------------------------------------
	void HDRSystem::BindEffectVariables(Effect* pEffect)
	{
		// Attach hdr shader variables
		m_pEffect = pEffect;
		m_HDRBuffer.AttachEffectSRVVariable(m_pEffect->HDRVariable);
		m_HDRBloom[0].AttachEffectSRVVariable(m_pEffect->HDRBloomVariable);
		m_HDRBloom[1].AttachEffectSRVVariable(m_pEffect->HDRBloomVariable);
		for(int i=0; i<m_HDRScaledLuminance.Size(); i++)
			m_HDRScaledLuminance[i].AttachEffectSRVVariable(m_pEffect->LuminanceVariable);
		m_HDRLuminance[0].AttachEffectSRVVariable(m_pEffect->AdaptedLuminanceVariable);
		m_HDRLuminance[1].AttachEffectSRVVariable(m_pEffect->AdaptedLuminanceVariable);

		// Build the gaussian blur filters
		ComputeGaussians(m_Width/2, m_Height/2);
	}


	//--------------------------------------------------------------------------------------
	// Free all hdr related resources
	//--------------------------------------------------------------------------------------
	void HDRSystem::Release()
	{
		m_HDRBuffer.Release();
		for(int i=0; i<m_HDRScaledLuminance.Size(); i++)
		{
			m_HDRScaledLuminance[i].Release();
			m_ScaledLuminanceDepth[i].Release();
		}
		m_HDRScaledLuminance.Release();
		m_ScaledLuminanceDepth.Release();
		m_HDRLuminance[0].Release();
		m_HDRLuminance[1].Release();
		m_LuminanceDepth.Release();
		m_HDRBloom[0].Release();
		m_HDRBloom[1].Release();
		m_BloomDepth.Release();
	}


	//--------------------------------------------------------------------------------------
	// Create the final scene image with HDR post-processing
	//--------------------------------------------------------------------------------------
	void HDRSystem::ProcessHDRI(ID3D10RenderTargetView* pBackBuffer)
	{
		// Down-sample the scene to a 1/4 size luminance texture
		int scaledIndex = m_HDRScaledLuminance.Size()-1;
		m_HDRScaledLuminance[scaledIndex].Clear();
		m_HDRScaledLuminance[scaledIndex].BindRenderTarget();
		g_pd3dDevice->RSSetViewports(1, &m_HDRScaledLuminance[scaledIndex].GetViewport());
		m_pEffect->OrthoProjectionVariable->SetMatrix( (float*)&m_HDRScaledLuminance[scaledIndex].GetOrthoMatrix() );		
		m_HDRBuffer.BindTextures();			
		m_pEffect->Pass[PASS_DOWNSCALE]->Apply(0);
		g_pd3dDevice->Draw(6, 0);
		scaledIndex--;

		// Progressively downscale the luminance values
		for(; scaledIndex>=0; scaledIndex--)
		{
			// Set the current scaled target to render to
			m_HDRScaledLuminance[scaledIndex].Clear();
			m_HDRScaledLuminance[scaledIndex].BindRenderTarget();
			g_pd3dDevice->RSSetViewports(1, &m_HDRScaledLuminance[scaledIndex].GetViewport());
			m_pEffect->OrthoProjectionVariable->SetMatrix( (float*)&m_HDRScaledLuminance[scaledIndex].GetOrthoMatrix() );		
			
			// Set the previous scaled target as the texture
			m_HDRScaledLuminance[scaledIndex+1].BindTextures();

			// Continue the downscaling
			m_pEffect->Pass[PASS_DOWNSCALE_LUMINANCE]->Apply(0);
			g_pd3dDevice->Draw(6, 0);
		}

		// Compute the final adapted luminance value
		if(m_CurrentLum==0){ m_CurrentLum=1; m_OldLum=0; }
		else { m_CurrentLum=0; m_OldLum=1; }
		m_HDRLuminance[m_CurrentLum].Clear();
		m_HDRLuminance[m_CurrentLum].BindRenderTarget();
		g_pd3dDevice->RSSetViewports(1, &m_HDRLuminance[m_CurrentLum].GetViewport());
		m_pEffect->OrthoProjectionVariable->SetMatrix( (float*)&m_HDRLuminance[m_CurrentLum].GetOrthoMatrix() );
		m_HDRScaledLuminance[0].BindTextures();		
		m_HDRLuminance[m_OldLum].BindTextures();
		m_pEffect->Pass[PASS_ADAPT]->Apply(0);
		g_pd3dDevice->Draw(6, 0);	

		// Send the bright pixels to the bloom buffer
		/*m_HDRBloom[0].Clear();
		m_HDRBloom[0].BindRenderTarget();
		g_pd3dDevice->RSSetViewports(1, &m_HDRBloom[0].GetViewport());
		m_pEffect->OrthoProjectionVariable->SetMatrix( (float*)&m_HDRBloom[0].GetOrthoMatrix() );
		m_HDRLuminance[m_CurrentLum].BindTextures();
		m_pEffect->Pass[PASS_BRIGHTPASS]->Apply(0);
		g_pd3dDevice->Draw(6, 0);
		
		// Perform a 2-step gaussian blur on the bloom buffer
		m_pEffect->FilterKernelVariable->SetInt(9);
		//for(int i=0; i<4; i++)
		{
			m_HDRBloom[1].Clear();
			m_HDRBloom[1].BindRenderTarget();
			m_pEffect->PostFXVariable->SetResource(m_HDRBloom[0].GetSRV()[0]);
			m_pEffect->Pass[PASS_GAUSSIAN_BLUR_H]->Apply(0);
			g_pd3dDevice->Draw(6, 0);	

			m_HDRBloom[0].Clear();
			m_HDRBloom[0].BindRenderTarget();
			m_pEffect->PostFXVariable->SetResource(m_HDRBloom[1].GetSRV()[0]);
			m_pEffect->Pass[PASS_GAUSSIAN_BLUR_V]->Apply(0);
			g_pd3dDevice->Draw(6, 0);
		}*/
		
		// Tone-map the image, and send the bright pixels to be bloomed	
		g_pd3dDevice->OMSetRenderTargets(1, &pBackBuffer, m_HDRBuffer.GetDSV());
		g_pd3dDevice->RSSetViewports(1, &m_HDRBuffer.GetViewport());
		m_HDRBloom[0].BindTextures();
		m_pEffect->OrthoProjectionVariable->SetMatrix( (float*)&m_HDRBuffer.GetOrthoMatrix() );	
		m_pEffect->Pass[PASS_TONEMAP]->Apply(0);
		g_pd3dDevice->Draw(6, 0);	
	}


	//-----------------------------------------------------------------------------
	// Compute the gaussian offsets and weights, and send them to the shader
	//-----------------------------------------------------------------------------
	void HDRSystem::ComputeGaussians(UINT width, UINT height)
	{
		const int numSamples = 9;
		const float g_BrightThreshold = 0.8f;       // A configurable parameter into the pixel shader
		const float g_GaussMultiplier = 0.4f;       // Default multiplier
		const float g_GaussMean = 0.0f;             // Default mean for gaussian distribution
		const float g_GaussStdDev = 0.2f;           // Default standard deviation for gaussian distribution

		// Configure the horizontal sampling offsets and their weights
		float HBloomWeights[numSamples];
		float HBloomOffsets[numSamples];
		for( int i = 0; i < numSamples; i++ )
		{
			// Compute the offsets. We take 9 samples - 4 either side and one in the middle:
			//     i =  0,  1,  2,  3, 4,  5,  6,  7,  8
			//Offset = -4, -3, -2, -1, 0, +1, +2, +3, +4
			HBloomOffsets[i] = ( static_cast< float >( i ) - 4.0f ) * ( 1.0f / static_cast< float >( width ) );

			// 'x' is just a simple alias to map the [0,8] range down to a [-1,+1]
			float x = ( static_cast< float >( i ) - 4.0f ) / 4.0f;

			// Use a gaussian distribution. Changing the standard-deviation
			// (second parameter) as well as the amplitude (multiplier) gives
			// distinctly different results.
			HBloomWeights[i] = g_GaussMultiplier * Math::ComputeGaussianValue( x, g_GaussMean, g_GaussStdDev );
		}

		// Configure the vertical sampling offsets and their weights
		float VBloomWeights[numSamples];
		float VBloomOffsets[numSamples];
		for( int i = 0; i < numSamples; i++ )
		{
			// Compute the offsets. We take 9 samples - 4 either side and one in the middle:
			//     i =  0,  1,  2,  3, 4,  5,  6,  7,  8
			//Offset = -4, -3, -2, -1, 0, +1, +2, +3, +4
			VBloomOffsets[i] = ( static_cast< float >( i ) - 4.0f ) * ( 1.0f / static_cast< float >( height ) );

			// 'x' is just a simple alias to map the [0,8] range down to a [-1,+1]
			float x = ( static_cast< float >( i ) - 4.0f ) / 4.0f;

			// Use a gaussian distribution. Changing the standard-deviation
			// (second parameter) as well as the amplitude (multiplier) gives
			// distinctly different results.
			VBloomWeights[i] = g_GaussMultiplier * Math::ComputeGaussianValue( x, g_GaussMean, g_GaussStdDev );
		}


		// Send these arrays to the shader
		m_pEffect->GaussianOffsetsH->SetFloatArray(HBloomOffsets, 0, numSamples);
		m_pEffect->GaussianOffsetsV->SetFloatArray(VBloomOffsets, 0, numSamples);
		m_pEffect->GaussianWeightsH->SetFloatArray(HBloomWeights, 0, numSamples);
		m_pEffect->GaussianWeightsV->SetFloatArray(VBloomWeights, 0, numSamples);			
	}


	


}