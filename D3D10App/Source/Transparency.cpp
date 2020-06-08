//--------------------------------------------------------------------------------------
// File: Transparency.cpp
//
// Transparent and refractive mesh rendering
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "Renderer.h"
#include "Log.h"

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Render the transparent meshes
	//--------------------------------------------------------------------------------------
	void Renderer::RenderTransparents()
	{
		if(m_TransparentObjects.Size()==0) return;

		// First sort the transparent meshes in back to front order
		m_TransparentObjects.Sort(&CompareSubMeshDistance);

		// Full lighting accumulation must be performed for back faces first,
		// and then front faces.  Without an accumulation buffer for diffuse
		// and specular terms the blending is repeated for each light (incorrect results).
		// This must be completed once per object, not too efficient but it works correctly
		for(int e=0; e<m_TransparentObjects.Size(); e++)
			RenderTransparentMesh(*m_TransparentObjects[e]);
		m_TransparentObjects.Clear();
	}


	////////////////////////////////////////////////////////////////////////////////////////
	// Render a transparent object using forward rendering
	////////////////////////////////////////////////////////////////////////////////////////
	void Renderer::RenderTransparentMesh(SubMesh& mesh)
	{
		// Bind the depth gbuffer so depth is saved for the sky and transparents
		ID3D10RenderTargetView* pRTVs[] = {
			m_HDRSystem.Buffer().GetRTV()[0],
			m_GBuffer.GetRTV()[DS_DEPTH],
		};
				
		// World matrix
		Device::Effect->WorldMatrixVariable->SetMatrix( (float*)mesh.pWorldMatrix );

		// Set the material
		Device::SetMaterial(*mesh.pMaterial);
		
		// Get the number of lights
#ifdef PHASE_DEBUG
		int numLights = (m_pFocusMesh) ? 4 : m_VisibleLights.Size(); 
#else
		int numLights = m_VisibleLights.Size(); 
#endif
		
		// Backfaces
		if(mesh.pMaterial->IsTwoSidedTransparent())
		{
			// Render the backfaces into the accumulation buffer
			m_LightingBuffer.Clear();
			m_LightingBuffer.BindRenderTarget();

			// Set the framebuffer texture
			if(mesh.pMaterial->IsRefractive())
			{
				m_FrameBuffer.CopyTexture(m_HDRSystem.Buffer());
				m_FrameBuffer.BindTextures();
			}

			for(int i=0; i<numLights; i++)
			{
				// Set light params
#ifdef PHASE_DEBUG
				if(m_pFocusMesh) Device::SetLight( m_FocusLights[i] );
				else Device::SetLight( *m_VisibleLights[i] );
#else
				Device::SetLight( *m_VisibleLights[i] );
#endif

				// Accumulate lighting
				Device::Effect->Pass[PASS_FORWARD_TRANSPARENT_SHADE_BACK]->Apply(0);
				Device::DrawSubmesh(mesh);
			}

			// Restore the backbuffer and set light buffer as the texture
			g_pd3dDevice->OMSetRenderTargets(2, pRTVs, m_DefaultDepth.GetDSV());
			m_LightingBuffer.BindTextures();

			// Use the light buffer to compute the final render
			if(mesh.pMaterial->IsRefractive())
				Device::Effect->Pass[PASS_FORWARD_REFRACT_BACK]->Apply(0);
			else
				Device::Effect->Pass[PASS_FORWARD_TRANSPARENT_FINAL_BACK]->Apply(0);
			Device::DrawSubmesh(mesh);
		}

		// Frontfaces
		{
			// Render the backfaces into the accumulation buffer
			m_LightingBuffer.Clear();
			m_LightingBuffer.BindRenderTarget();

			// Set the framebuffer texture
			if(mesh.pMaterial->IsRefractive())
			{
				m_FrameBuffer.CopyTexture(m_HDRSystem.Buffer());
				m_FrameBuffer.BindTextures();
			}

			for(int i=0; i<numLights; i++)
			{
				// Set light params
#ifdef PHASE_DEBUG
				if(m_pFocusMesh) Device::SetLight( m_FocusLights[i] );
				else Device::SetLight( *m_VisibleLights[i] );
#else
				Device::SetLight( *m_VisibleLights[i] );
#endif

				// Accumulate lighting
				Device::Effect->Pass[PASS_FORWARD_TRANSPARENT_SHADE_FRONT]->Apply(0);
				Device::DrawSubmesh(mesh);
			}

			// Restore the backbuffer and set light buffer as the texture
			g_pd3dDevice->OMSetRenderTargets(2, pRTVs, m_DefaultDepth.GetDSV());
			m_LightingBuffer.BindTextures();

			// Use the light buffer to compute the final render
			if(mesh.pMaterial->IsRefractive())
				Device::Effect->Pass[PASS_FORWARD_REFRACT_FRONT]->Apply(0);
			else
				Device::Effect->Pass[PASS_FORWARD_TRANSPARENT_FINAL_FRONT]->Apply(0);
			Device::DrawSubmesh(mesh);
		}
	}

}