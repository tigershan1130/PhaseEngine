//--------------------------------------------------------------------------------------
// File: Shadows.cpp
//
// Implementation of variance shadow maps with msaa
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "Renderer.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Create the shadow map cache
	//--------------------------------------------------------------------------------------
	HRESULT Renderer::InitShadowSystem(UINT resolution, UINT samples, UINT quality, UINT mips)
	{
		HRESULT hr;
		ShadowSettings::Resolution=resolution;
		ShadowSettings::MSAA.Count=samples;
		ShadowSettings::MSAA.Quality=quality;
		ShadowSettings::MipLevels=mips;
		ShadowSettings::Viewport.Height = ShadowSettings::Resolution;
		ShadowSettings::Viewport.Width = ShadowSettings::Resolution;
		ShadowSettings::Viewport.MinDepth = 0;
		ShadowSettings::Viewport.MaxDepth = 1.0f;
		ShadowSettings::Viewport.TopLeftX = 0;
		ShadowSettings::Viewport.TopLeftY = 0;

		// Shdaow map texel size for the shader
		Device::Effect->ShadowMapSizeVariable->SetFloat(1.0f / (float)ShadowSettings::Resolution);

		// Create depth stencils
		V_RETURN(m_ShadowMapDepth.Create(ShadowSettings::Resolution,ShadowSettings::Resolution, NULL, false));
		V_RETURN(m_ShadowMapDepthMSAA.Create(ShadowSettings::Resolution,ShadowSettings::Resolution, &ShadowSettings::MSAA, false));
		V_RETURN(m_ShadowMapDepthCube.CreateCube(ShadowSettings::Resolution, ShadowSettings::Resolution));

		// Create the MSAA enabled resources
		V_RETURN(m_ShadowMapMSAA.Create(ShadowSettings::Resolution, ShadowSettings::Resolution, DXGI_FORMAT_R32G32_FLOAT, 1, &ShadowSettings::MSAA));
		

		// Initialize the shadowmaps
		V_RETURN(m_ShadowMap.Create(ShadowSettings::Resolution, ShadowSettings::Resolution, DXGI_FORMAT_R32G32_FLOAT, ShadowSettings::MipLevels, NULL));
		V_RETURN(m_ShadowMapCube.CreateCube(ShadowSettings::Resolution, ShadowSettings::Resolution, DXGI_FORMAT_R32G32_FLOAT, ShadowSettings::MipLevels));
		

		// Create a render target to blur shadow maps
		V_RETURN(m_ShadowMapBlur.Create(ShadowSettings::Resolution, ShadowSettings::Resolution, DXGI_FORMAT_R32G32_FLOAT, 1, NULL ));
			

		// Initialize the shadow projection matrices
		D3DXMatrixPerspectiveFovLH( &m_mShadowProjSpot, D3DX_PI*0.75f, 1.0f, 0.01f, 1000.0f);
		D3DXMatrixPerspectiveFovLH( &m_mShadowProjCube, D3DX_PI*0.5f, 1.0f, 0.01f, 1000.0f);

		// Create an orthoganal and scaled projection matrix for
		// shadow map rendering
		D3DXMATRIXA16 mScaleScreen,mOrtho2D;
		D3DXMatrixScaling(&mScaleScreen, (float)ShadowSettings::Resolution/2.0f, (float)ShadowSettings::Resolution/2.0f, 0);	
		D3DXMatrixOrthoLH(&mOrtho2D, (float)ShadowSettings::Resolution, (float)ShadowSettings::Resolution, 0.0f, 1.0f);
		m_mShadowOrtho = mScaleScreen * mOrtho2D;
		Device::Effect->ShadowOrthoProjectionVariable->SetMatrix( (float*)&m_mShadowOrtho );

		// Bind effect vars
		m_ShadowMapMSAA.AttachDepthStencil(m_ShadowMapDepthMSAA);
		m_ShadowMap.AttachDepthStencil(m_ShadowMapDepth);
		m_ShadowMap.AttachEffectSRVVariable(Device::Effect->ShadowMapVariable);
		m_ShadowMapCube.AttachDepthStencil(m_ShadowMapDepthCube);
		m_ShadowMapCube.AttachEffectSRVVariable(Device::Effect->ShadowMapCubeVariable);
		m_ShadowMapBlur.AttachDepthStencil(m_ShadowMapDepth);
		m_ShadowMapBlur.AttachEffectSRVVariable(Device::Effect->ShadowMapVariable);

		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	// Release shadow system objects
	//--------------------------------------------------------------------------------------
	void Renderer::ReleaseShadowSystem()
	{
		m_ShadowMap.Release();
		m_ShadowMapMSAA.Release();
		m_ShadowMapCube.Release();
		m_ShadowMapDepth.Release();
		m_ShadowMapDepthMSAA.Release();
		m_ShadowMapDepthCube.Release();
		m_ShadowMapBlur.Release();
	}


	////////////////////////////////////////////////////////////////////////////////////////
	// Renders the light shadow maps.  Shadow maps are rendered using
	// Variance Shadow Mapping with MSAA, a seperable box filter, mip-mapping
	// as well as anisotropic texture filtering.  Light bleeding reduction is applied
	// in the shader to reduce artifacts.
	////////////////////////////////////////////////////////////////////////////////////////
	void Renderer::RenderShadowMap(Light& light)
	{
		// Save the old viewport
		static D3D10_VIEWPORT OldVP;
		static UINT cRT = 1;
		g_pd3dDevice->RSGetViewports( &cRT, &OldVP );

		// Set the viewport for rendering to shadow maps
		g_pd3dDevice->RSSetViewports( 1, &ShadowSettings::Viewport );

		//
		// Render the shadow map
		//
		if(light.Type == Light::LIGHT_SPOT)
		{
			// Build the view matrix and frustum
			D3DXMatrixLookAtLH( &light.matrix, 
				&light.GetPos(), 
				&D3DXVECTOR3(light.GetPos() + light.GetDir()), 
				&D3DXVECTOR3(0.0f, 1.0f, 0.0f) );
			light.frustum.Build(&light.matrix,&m_mShadowProjSpot);
			Device::Effect->LightMatrixVariable->SetMatrix((float*)&light.matrix);
			Device::Effect->ShadowMatrixVariable->SetMatrix((float*)m_mShadowProjSpot);

			// Bind the MSAA targets
			if(ShadowSettings::MSAA.Count>1)
			{
				m_ShadowMapMSAA.Clear();
				m_ShadowMapMSAA.ClearDSV();
				m_ShadowMapMSAA.BindRenderTarget();
			}
			else
			{
				m_ShadowMap.Clear();
				m_ShadowMap.ClearDSV();
				m_ShadowMap.BindRenderTarget();
			}

			// Render the scene into the shadow map		
			for(int e=0; e<light.MeshList.Size(); e++)
			{
				// Check the mesh against the light frustum
				MeshObject& mesh = *light.MeshList[e];
				if(!light.frustum.CheckSphere(mesh.GetPos(), mesh.GetRadius()))
					continue;

				// Set the effect variables only when the material changes
				Device::Effect->WorldMatrixVariable->SetMatrix( (float*)&mesh.GetWorldMatrix() );

				// Update the animation matrices
				if(mesh.IsSkinned())
				{
					Device::Effect->AnimMatricesVariable->SetMatrixArray((float*)mesh.GetAnimMatrices(), 0, mesh.GetNumAnimMatrices());

					// Set the input layout
					Device::SetInputLayout( SkinnedVertex::pInputLayout );
				}
				else
					Device::SetInputLayout( Vertex::pInputLayout );

				// Render the submeshes
				for(int k=0; k<mesh.GetNumSubMesh(); k++)
				{
					if(mesh.GetSubMesh(k)->pMaterial->IsTransparent() || mesh.GetSubMesh(k)->pMaterial->IsRefractive())
						continue;
					
					if( mesh.IsSkinned() )	
						Device::Effect->Pass[PASS_SHADOWMAP_ANIM]->Apply(0);
					else
						Device::Effect->Pass[PASS_SHADOWMAP]->Apply(0);

					// Render the mesh				
					Device::DrawSubmesh(*mesh.GetSubMesh(k));
				}				
			}		

			// Resolve the MSAA surface
			if(ShadowSettings::MSAA.Count>1)
				m_ShadowMapMSAA.Resolve(m_ShadowMap);

			// Now blur the shadow map with a separable box filter
			if(light.ShadowQuality>1)
			{
				Device::Effect->FilterKernelVariable->SetInt( light.ShadowQuality );
				m_ShadowMapBlur.BindRenderTarget();
				Device::Effect->PostFXVariable->SetResource(m_ShadowMap.GetSRV()[0]);
				UINT offset = 0;
				SetRenderToQuad();
				Device::Effect->Pass[PASS_BOX_BLUR_H]->Apply(0);
				g_pd3dDevice->Draw(6,0);
				m_ShadowMap.BindRenderTarget();
				Device::Effect->PostFXVariable->SetResource(m_ShadowMapBlur.GetSRV()[0]);
				Device::Effect->Pass[PASS_BOX_BLUR_V]->Apply(0);
				g_pd3dDevice->Draw(6,0);
			}

			// Generate mip maps for the shadow maps
			if(ShadowSettings::MipLevels>1)
				m_ShadowMap.GenerateMips();
		}
		else if(light.Type == Light::LIGHT_POINT)
		{
			// Render each face of the cube map for point lights
			D3DXVECTOR3 vLookDir;
			D3DXVECTOR3 vUpDir;
			D3DXMATRIX mView;
			Device::Effect->ShadowMatrixVariable->SetMatrix((float*)m_mShadowProjCube);
			for(int i=0; i<6; i++)
			{
				// Clear and set the render target/depth buffer
				m_ShadowMapCube.Clear(i);
				m_ShadowMapCube.ClearDSV();
				m_ShadowMapCube.BindRenderTarget(i);

				// Set the view matrix for this face
				switch( i )
				{
				case D3DCUBEMAP_FACE_POSITIVE_X:
					vLookDir = D3DXVECTOR3( 1.0f, 0.0f, 0.0f );
					vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
					break;
				case D3DCUBEMAP_FACE_NEGATIVE_X:
					vLookDir = D3DXVECTOR3(-1.0f, 0.0f, 0.0f );
					vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
					break;
				case D3DCUBEMAP_FACE_POSITIVE_Y:
					vLookDir = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
					vUpDir   = D3DXVECTOR3( 0.0f, 0.0f,-1.0f );
					break;
				case D3DCUBEMAP_FACE_NEGATIVE_Y:
					vLookDir = D3DXVECTOR3( 0.0f,-1.0f, 0.0f );
					vUpDir   = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
					break;
				case D3DCUBEMAP_FACE_POSITIVE_Z:
					vLookDir = D3DXVECTOR3( 0.0f, 0.0f, 1.0f );
					vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
					break;
				case D3DCUBEMAP_FACE_NEGATIVE_Z:
					vLookDir = D3DXVECTOR3( 0.0f, 0.0f,-1.0f );
					vUpDir   = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
					break;
				}

				// Set the view transform for this cubemap face
				D3DXMatrixLookAtLH( &mView, &light.GetPos(), &(light.GetPos()+vLookDir), &vUpDir );
				Device::Effect->LightMatrixVariable->SetMatrix((float*)&mView);
				light.frustum.Build(&mView, &m_mShadowProjCube);

				// Render the scene into the shadow map	cube face	
				for(int e=0; e<light.MeshList.Size(); e++)
				{
					// Check the mesh against the light frustum
					MeshObject& mesh = *light.MeshList[e];
					if(!light.frustum.CheckSphere(mesh.GetPos(), mesh.GetRadius()))
						continue;

					// Set the effect variables only when the material changes
					Device::Effect->WorldMatrixVariable->SetMatrix( (float*)&mesh.GetWorldMatrix() );

					// Update the animation matrices
					if(mesh.IsSkinned())
					{
						Device::Effect->AnimMatricesVariable->SetMatrixArray((float*)mesh.GetAnimMatrices(), 0, mesh.GetNumAnimMatrices());

						// Set the input layout
						Device::SetInputLayout( SkinnedVertex::pInputLayout );
					}
					else
						Device::SetInputLayout( Vertex::pInputLayout );

					// Render the submeshes
					for(int k=0; k<mesh.GetNumSubMesh(); k++)
					{
						if(mesh.GetSubMesh(k)->pMaterial->IsTransparent() || mesh.GetSubMesh(k)->pMaterial->IsRefractive())
							continue;

						if( mesh.IsSkinned() )	
							Device::Effect->Pass[PASS_SHADOWMAP_ANIM]->Apply(0);
						else
							Device::Effect->Pass[PASS_SHADOWMAP]->Apply(0);

						// Render the mesh				
						Device::DrawSubmesh(*mesh.GetSubMesh(k));
					}		
				}
			}

			// Generate mip maps for the shadow maps
			if(ShadowSettings::MipLevels>1)
				m_ShadowMapCube.GenerateMips();
		}

		// Restore the old viewport
		g_pd3dDevice->RSSetViewports( 1, &OldVP );

		// Restore the old target
		m_HDRSystem.Buffer().BindRenderTarget();

		Device::SetInputLayout( Vertex::pInputLayout );

		if(light.IsShadowed)
		{
			if(light.Type == Light::LIGHT_POINT)
				m_ShadowMapCube.BindTextures();
			else
				m_ShadowMap.BindTextures();
		}
	}

}