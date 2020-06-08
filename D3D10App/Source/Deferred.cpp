//--------------------------------------------------------------------------------------
// File: Deferred.cpp
//
// Implementation of the Phase Deferred rendering system
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "Renderer.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Render a mesh into the GBuffers, or postpone for transparent rendering
	//--------------------------------------------------------------------------------------
	void Renderer::RenderMeshDeferred(SubMesh& mesh)
	{
		// Make sure it's visible
		if(!Device::IsMeshVisible(mesh, m_Camera))
			return;

		// Render transparent and refractive meshes later
		if(mesh.pMaterial->IsRefractive() || mesh.pMaterial->IsTransparent())
		{
			m_TransparentObjects.Add(&mesh);
			return;
		}

		// World matrix
		Device::Effect->WorldMatrixVariable->SetMatrix( (float*)mesh.pWorldMatrix );
		Device::Effect->OldWorldMatrixVariable->SetMatrix( (float*)mesh.pWorldMatrixPrev );

		if(mesh.isSkinned)
		{
			Device::Effect->AnimMatricesVariable->SetMatrixArray((float*)((D3DXMATRIX*)*mesh.pAnimMatrices), 0, mesh.pAnimMatrices->Size());

			// Set the input layout
			Device::SetInputLayout( SkinnedVertex::pInputLayout );
		}
		else
			Device::SetInputLayout( Vertex::pInputLayout );

		// Set the material
		Device::SetMaterial(*mesh.pMaterial);

		// Setup the effect pass
		if(mesh.pCubeMap && mesh.pMaterial->IsReflective())
		{
			Device::SetCubeMap(mesh.pCubeMap);
			Device::Effect->Pass[PASS_GBUFFER_CUBEMAP]->Apply(0);
		}
		else
		{
			if(mesh.isSkinned)
				Device::Effect->Pass[PASS_GBUFFER_ANIM]->Apply(0);
			else
				Device::Effect->Pass[PASS_GBUFFER]->Apply(0);
		}

		// Render the mesh
		Device::DrawSubmesh(mesh);
	}



	//--------------------------------------------------------------------------------------
	// Render using deferred shading.  First a set of GBuffers are filled with scene data, 
	// such as position, normal, color and material properties.  Then during lighting passes
	// this data is read in from the textures to shade the scene.  A final ambient and light
	// accumulation pass puts the final color to the screen.
	//--------------------------------------------------------------------------------------
	void Renderer::RenderSceneDeferred()
	{
		// Set the gbuffer render targets
		m_GBuffer.Clear(DS_COLOR);
		m_GBuffer.Clear(DS_NORMAL);
		float depthClear[] = {m_Camera.GetFarZ(), m_Camera.GetFarZ(), m_Camera.GetFarZ(), m_Camera.GetFarZ()};
		g_pd3dDevice->ClearRenderTargetView(m_GBuffer.GetRTV()[DS_DEPTH], depthClear);
		m_GBuffer.ClearDSV();
		m_GBuffer.BindRenderTarget();

		//
		// Render to the gbuffers
		//

	   
		// Normal meshes
		for(int i=0; i<m_RenderList.Size(); i++)
			RenderMeshDeferred(*m_RenderList[i]);


		// Terrain
		Device::SetInputLayout(PosVertex::pInputLayout);
		RenderTerrain(m_Camera.GetFrustum(), Device::Effect->Pass[PASS_GBUFFER_TERRAIN]);
		Device::SetInputLayout(Vertex::pInputLayout);

		// Restore the hdr buffer as the render target
		m_HDRSystem.Buffer().BindRenderTarget();
	}		

	//--------------------------------------------------------------------------------------
	// Performs shading on the GBuffers by rendering light geometry
	//--------------------------------------------------------------------------------------
	void Renderer::ShadeSceneDeferred()
	{
		// Set the GBuffer textures
		m_GBuffer.BindTextures();

		//
		// Run a fullscreen ambient pass
		//
		UINT offset = 0;
		SetRenderToQuad();	

		// First fill SSAO buffer
		/*m_SSAOBuffer[0].BindRenderTarget();
		g_pd3dDevice->RSSetViewports(1, &m_SSAOBuffer[0].GetViewport());
		Device::Effect->OrthoProjectionVariable->SetMatrix( (float*)&m_SSAOBuffer[0].GetOrthoMatrix() );	
		Device::Effect->Pass[PASS_AMBIENT_OCCLUSION]->Apply(0);
		g_pd3dDevice->Draw(6, 0);

		// Blur it
		Device::Effect->FilterKernelVariable->SetInt(3);
		for(int i=0; i<3; i++)
		{
			m_SSAOBuffer[1].BindRenderTarget();
			m_SSAOBuffer[0].BindTextures();
			Device::Effect->Pass[PASS_BOX_BLUR_H]->Apply(0);
			g_pd3dDevice->Draw(6, 0);
			m_SSAOBuffer[0].BindRenderTarget();
			m_SSAOBuffer[1].BindTextures();
			Device::Effect->Pass[PASS_BOX_BLUR_V]->Apply(0);
			g_pd3dDevice->Draw(6, 0);
		}


		// Combine back with frame buffer for ambient
		m_HDRSystem.Buffer().BindRenderTarget();
		g_pd3dDevice->RSSetViewports(1, &m_HDRSystem.Buffer().GetViewport());
		Device::Effect->OrthoProjectionVariable->SetMatrix( (float*)&m_HDRSystem.Buffer().GetOrthoMatrix() );	
		m_SSAOBuffer[0].BindTextures();*/
		Device::Effect->Pass[PASS_AMBIENT]->Apply(0);
		g_pd3dDevice->Draw(6, 0);

		// Update the sun direction
		m_Sun.SetDir(m_SkySystem.GetSunDirection());
		
		m_VisibleLights.Clear();
		D3DXVECTOR3 r;
		D3DXMATRIX mS,mT,mR;
		for(int i=0; i<m_Lights.Size(); i++)
		{
			// Make a reference to avoid many pointer derefs on the function calls
			Light& light = *m_Lights[i];

			// Update the light's position
			light.UpdatePosition();

			// Set the light effect vars
			Device::SetLight(light);

			// Direction lights always visible
			if( light.Type==Light::LIGHT_DIRECTIONAL)
			{
				// Set the pass
				Device::Effect->Pass[PASS_SHADE_FULL]->Apply(0);
				// Render a quad
				SetRenderToQuad();	
				g_pd3dDevice->Draw(6, 0);
				// Add to visible lights list
				m_VisibleLights.Add(m_Lights[i]);
			}

			// Perform an occlusion test for the lights sphere volume mesh
			else if( light.Type==Light::LIGHT_POINT)
			{		
				// Check if the light is in the view frustum
				if(!IsLightVisible(light))
					continue;
				// Render the shadow map
				if(light.IsShadowed)
					RenderShadowMap(light);

				// Build the world matrix for the light geometry
				D3DXMatrixTranslation(&mT, light.GetPos().x, light.GetPos().y, light.GetPos().z );
				D3DXMatrixScaling(&mS, light.GetRange(), light.GetRange(), light.GetRange());
				light.worldMatrix = mS * mT;
				Device::Effect->WorldMatrixVariable->SetMatrix((float*)&light.worldMatrix);

				// Render the light geometry
				Device::Effect->Pass[PASS_SHADE]->Apply(0);
				Device::DrawSubmesh(*m_SphereMesh.GetSubMesh(0));

				// Add to visible lights list
				m_VisibleLights.Add(m_Lights[i]);
			}
			// Perform an occlusion test for the lights cone volume mesh
			else if( light.Type==Light::LIGHT_SPOT)
			{
				// Check if the light is in the view frustum
				if(!IsLightVisible(light))
					continue;

				// Render the shadow map
				if(light.IsShadowed)
					RenderShadowMap(light);

				// Build the world matrix for the light geometry
				D3DXMatrixScaling(&mS, light.GetRange(), light.GetRange(), light.GetRange());
				Math::BuildRotationMatrix(light.GetDir(), mR);
				r = light.GetPos() + light.GetDir()*0.5f*light.GetRange();
				D3DXMatrixTranslation(&mT, r.x, r.y, r.z);
				light.worldMatrix = mS * mR * mT;
				Device::Effect->WorldMatrixVariable->SetMatrix((float*)&light.worldMatrix);

				// Render the light geometry
				Device::Effect->Pass[PASS_SHADE]->Apply(0);
				Device::DrawSubmesh(*m_ConeMesh.GetSubMesh(0));

				// Add to visible lights list
				m_VisibleLights.Add(m_Lights[i]);
			}
		}
	}


	//--------------------------------------------------------------------------------------
	// Render only the focus mesh using deferred shading.  
	//--------------------------------------------------------------------------------------
#ifdef PHASE_DEBUG
	void Renderer::RenderFocusMesh()
	{
		// Set the gbuffer render targets
		m_GBuffer.Clear();
		m_GBuffer.ClearDSV();
		m_GBuffer.BindRenderTarget();

		//
		// Render to the gbuffers
		//
		// Pass the matrix pallete to the shader (for skinned meshes)
		if(m_pFocusMesh->IsSkinned())
		{
			Device::Effect->AnimMatricesVariable->SetMatrixArray((float*)m_pFocusMesh->GetAnimMatrices(), 0, m_pFocusMesh->GetNumAnimMatrices());

			// Set the input layout
			Device::SetInputLayout( SkinnedVertex::pInputLayout );
		}
		else
			Device::SetInputLayout( Vertex::pInputLayout );

		// Build the correct world matrix, using the camera view angles to rotate the mesh
		m_pFocusMesh->SetRot( -m_Camera.GetXDeg(), -m_Camera.GetYDeg(), 0);

		// Render it to the GBuffers
		for(int i=0; i<m_pFocusMesh->GetNumSubMesh(); i++)
			RenderMeshDeferred(*m_pFocusMesh->GetSubMesh(i));

		//
		// Shading phase
		//
		Device::RestoreCachedTargets();
		m_GBuffer.BindTextures();

		//
		// Run a fullscreen ambient pass
		//
		Device::Effect->Pass[PASS_AMBIENT]->Apply(0);
		UINT offset=0;
		SetRenderToQuad();	
		g_pd3dDevice->Draw(6, 0);


		//
		// Render using directional lights
		//
		for(int i=0; i<4; i++)
		{
			Device::SetLight(m_FocusLights[i]);
			Device::Effect->Pass[PASS_SHADE_FULL]->Apply(0);
			g_pd3dDevice->Draw(6, 0);
		}
	}		
#endif

}