//--------------------------------------------------------------------------------------
// File: States.cpp
//
// Engine state management
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "Renderer.h"

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Re-binds effect variables after recompiling a shader.
	// All render targets must place their attach code in here as
	// well as in their init area or they will not work when
	// the shader is recompiled
	//--------------------------------------------------------------------------------------
	void Renderer::RebindEffectVariables()
	{
		// Shadow stuff
		m_ShadowMapMSAA.AttachDepthStencil(m_ShadowMapDepthMSAA);
		m_ShadowMap.AttachDepthStencil(m_ShadowMapDepth);
		m_ShadowMap.AttachEffectSRVVariable(Device::Effect->ShadowMapVariable);
		m_ShadowMapCube.AttachDepthStencil(m_ShadowMapDepthCube);
		m_ShadowMapCube.AttachEffectSRVVariable(Device::Effect->ShadowMapCubeVariable);
		m_ShadowMapBlur.AttachDepthStencil(m_ShadowMapDepth);
		m_ShadowMapBlur.AttachEffectSRVVariable(Device::Effect->ShadowMapVariable);	

		Device::Effect->ShadowMapSizeVariable->SetFloat(1.0f / (float)ShadowSettings::Resolution);
		Device::Effect->ShadowOrthoProjectionVariable->SetMatrix( (float*)&m_mShadowOrtho );

		// GBuffer
		m_GBuffer.AttachEffectSRVVariable(Device::Effect->GBufferVariable);
		m_LightingBuffer.AttachEffectSRVVariable(Device::Effect->LightBufferVariable);
		m_FrameBuffer.AttachEffectSRVVariable(Device::Effect->FrameBufferVariable);
		m_PostFXBuffer.AttachEffectSRVVariable(Device::Effect->FrameBufferVariable);
		
		D3DXVECTOR4 screenSize(m_Width, m_Height, 1.0f/(float)m_Width, 1.0f/(float)m_Height);
		Device::Effect->ScreenSizeVariable->SetFloatVector( (float*)&screenSize );
		Device::Effect->FarZVariable->SetFloat(m_Camera.GetFarZ());
		Device::Effect->NearZVariable->SetFloat(m_Camera.GetNearZ());
		Device::Effect->OrthoProjectionVariable->SetMatrix( (float*)&m_mOrtho );

		Device::Effect->AmbientVariable->SetFloatVector((float*)&m_Ambient);

		m_SSAOBuffer[0].AttachEffectSRVVariable(Device::Effect->PostFXVariable);
		m_SSAOBuffer[1].AttachEffectSRVVariable(Device::Effect->PostFXVariable);
		
		EncodeMaterialTexture();
		EncodeSSAOTexture(m_Width, m_Height);

		// Cubemaps
		for(int i=0; i<m_Probes.Size(); i++)
		{
			m_Probes[i]->GetSurface().AttachEffectSRVVariable(Device::Effect->ProbeCubeVariable);
			m_Probes[i]->GetSurface().AttachDepthStencil(m_ProbeDepth.GetDSV());
		}

		// Attach hdr shader variables
		m_HDRSystem.BindEffectVariables(Device::Effect);

		m_SkySystem.BindEffectVariables(Device::Effect);
	}

}