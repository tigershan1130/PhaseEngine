//--------------------------------------------------------------------------------------
// File: Renderer.cpp
//
// Implementation of the Phase rendering system
//
// Includes deferred renderer, transparent mesh forward renderer and shadow mapping
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "Renderer.h"

namespace Core
{
	// Static variables
#ifdef PHASE_DEBUG
	Camera::CAMERA_TYPE Renderer::CameraCache::Type;
	D3DXVECTOR3 Renderer::CameraCache::Position;
	D3DXVECTOR2 Renderer::CameraCache::Direction;
	D3DXVECTOR3 Renderer::CameraCache::MeshRotation;
#endif
	
	UINT Renderer::ShadowSettings::Resolution;
	DXGI_SAMPLE_DESC Renderer::ShadowSettings::MSAA;
	UINT Renderer::ShadowSettings::MipLevels;
	D3D10_VIEWPORT Renderer::ShadowSettings::Viewport;
	UINT Renderer::ProbeSettings::resolution;
	DXGI_FORMAT Renderer::ProbeSettings::format;
	UINT Renderer::ProbeSettings::miplevels;

	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	Renderer::Renderer()
	{
		Device::FrameStats.FPS=0;				
		m_Width=0;				
		m_Height=0;				
		Device::FrameStats.PolysDrawn=0;	
		m_TotalPolyThisScene=0;	
#ifdef PHASE_DEBUG
		m_pFocusMesh = NULL;
		m_bDemoMode = false;
		m_pDebugLineOutlineVB = NULL;
		m_pDebugLineVB = NULL;
		m_pDebugLineBoxVB = NULL;
		m_pLightTex = NULL;
#endif
		m_pFont = NULL;
		m_pSprite = NULL;
		m_pTxtHelper = NULL;
		m_pQuadVertexBuffer = NULL;
		m_pPredicate = NULL;
		m_pTerrain = NULL;

		m_pMaterialTex = NULL;
		m_pMaterialSRV = NULL;
		m_pRandomTex = NULL;
		m_pRandomSRV = NULL;
		m_CurVelocity = 0;
		
		m_qOffset=0;
	}


	//--------------------------------------------------------------------------------------
	// Renders the scene
	//--------------------------------------------------------------------------------------
	void Renderer::Render()
	{
		OnFrameMove();
		ClearFrame();
		OnFrameRender();
		Present();
	}


	//--------------------------------------------------------------------------------------
	// Updates the system
	//--------------------------------------------------------------------------------------
	void Renderer::OnFrameMove()
	{
		Device::FrameStats.PolysDrawn=0;
		Device::FrameStats = FrameStatData();
		
		// Get the mouse position
		GetCursorPos(&m_MousePos);
		ScreenToClient(m_hWnd, &m_MousePos);
		
		// Reset any engine states
		Device::Reset();

		// Update the time scale
		static DWORD lastFrame=0, thisFrame=timeGetTime();
		static float fAbsTime=0, fFrames=0, fps=60;
		
		lastFrame = thisFrame;
		thisFrame = timeGetTime();
		float frameTime = (thisFrame-lastFrame)*0.001f;
		Device::FrameStats.ElapsedTime = frameTime;
		Device::Effect->ElapsedTimeVariable->SetFloat(frameTime);
		g_TimeScale = 60.0f * frameTime; 

		// Find the fps
		fAbsTime += fabs(frameTime);
		fFrames++;
		if( fAbsTime >= 1.0f )
		{
			fps = fFrames / fAbsTime;
			fAbsTime = 0;
			fFrames = 0;
		}
		Device::FrameStats.FPS = fps;

		// Camera update
		m_Camera.Update(false);

		// Update variables that change once per frame
		m_mOldViewProj = m_mViewProj;
		Device::Effect->OldViewProjectionVariable->SetMatrix( (float*)&m_mOldViewProj );
		m_mViewProj = m_Camera.GetViewMatrix()*m_Camera.GetProjMatrix();
		Device::Effect->ViewProjectionVariable->SetMatrix( (float*)&m_mViewProj );

		// Set the old world matrices for the meshes
		for(int i=0; i<m_MeshObjects.Size(); i++)
			m_MeshObjects[i]->SetOldWorldMatrix( m_MeshObjects[i]->GetWorldMatrix() );

		// Camera position and direction
		Device::Effect->CameraPosVariable->SetFloatVector( (float*)&m_Camera.GetPos() );
		Device::Effect->CameraDirVariable->SetFloatVector( (float*)&m_Camera.GetView() );

		// Update the frustum corners
		m_QuadVerts[0].normal = m_Camera.GetFrustumCorner(Camera::FRUSTUM_TOP_RIGHT);
		m_QuadVerts[1].normal = m_Camera.GetFrustumCorner(Camera::FRUSTUM_BOTTOM_RIGHT);
		m_QuadVerts[2].normal = m_Camera.GetFrustumCorner(Camera::FRUSTUM_BOTTOM_LEFT);
		m_QuadVerts[3].normal = m_Camera.GetFrustumCorner(Camera::FRUSTUM_BOTTOM_LEFT);
		m_QuadVerts[4].normal = m_Camera.GetFrustumCorner(Camera::FRUSTUM_TOP_LEFT);
		m_QuadVerts[5].normal = m_Camera.GetFrustumCorner(Camera::FRUSTUM_TOP_RIGHT);

		Vertex* pVerts; 
		m_pQuadVertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&pVerts);
		memcpy(pVerts, m_QuadVerts, Vertex::size*6);
		m_pQuadVertexBuffer->Unmap();

		// Update the scene
		ProcessMessages();
	}


	//--------------------------------------------------------------------------------------
	// Draws the scene
	//--------------------------------------------------------------------------------------
	void Renderer::OnFrameRender()
	{
		// Set the input layout and topology
		Device::SetInputLayout( Vertex::pInputLayout );
		Device::SetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		// Render the environment maps
		//RenderProbes();

		// Update the terrain clipmaps
		UINT offset=0;
		if(m_pTerrain)
		{
			SetRenderToQuad();
			m_pTerrain->UpdateClipmaps(*Device::Effect);
		}

		// Set the hdr buffer
		m_HDRSystem.Buffer().Clear();
		m_HDRSystem.Buffer().ClearDSV();
		m_HDRSystem.Buffer().BindRenderTarget();

		// Setup the proper viewport
		Device::Effect->OrthoProjectionVariable->SetMatrix((float*)m_HDRSystem.Buffer().GetOrthoMatrix());
		g_pd3dDevice->RSSetViewports(1, &m_HDRSystem.Buffer().GetViewport());

		// Render the meshes
#ifdef PHASE_DEBUG
 		if(m_pFocusMesh)
 			RenderFocusMesh();
 		else
		{
 			RenderSceneDeferred();
			ShadeSceneDeferred();
		}
#else
		RenderSceneDeferred();
		ShadeSceneDeferred();
#endif

		// Draw the sky
		//RenderSky(m_Camera.GetPos());

		RenderWater();


		// Go through all particle emitters, then drawe verything that is particle like that is transparent.
		SetRenderToQuad();
		for(int i=0; i<m_Emitters.Size(); i++)
			m_Emitters[i]->Update(Device::FrameStats.ElapsedTime);

		// Draw the transparent meshes
		RenderTransparents();

		// Perform the HDR image processing
		SetRenderToQuad();
		m_HDRSystem.ProcessHDRI(m_PostFXBuffer.GetRTV()[0]);
		//m_HDRSystem.ProcessHDRI(m_pBackBuffer);

		// Build a velocity map
		if(m_CurVelocity==0){ m_CurVelocity=1; m_OldVelocity=0; }
		else { m_CurVelocity=0; m_OldVelocity=1; }
		m_VelocityMap[m_CurVelocity].Clear();
		m_VelocityMap[m_CurVelocity].BindRenderTarget();
		g_pd3dDevice->RSSetViewports(1, &m_VelocityMap[m_CurVelocity].GetViewport());
		Device::Effect->OrthoProjectionVariable->SetMatrix( (float*)&m_VelocityMap[m_CurVelocity].GetOrthoMatrix() );	
		Device::Effect->Pass[PASS_POST_VELOCITY_MAP]->Apply(0);
		g_pd3dDevice->Draw(6, 0);

		// Perform the blur	
		g_pd3dDevice->OMSetRenderTargets(1, &m_pBackBuffer, m_DefaultDepth.GetDSV());
		g_pd3dDevice->RSSetViewports(1, &m_FrameBuffer.GetViewport());
		Device::Effect->OrthoProjectionVariable->SetMatrix( (float*)&m_mOrtho );
		m_PostFXBuffer.BindTextures();
		Device::Effect->PostFXVariable->SetResource(m_VelocityMap[m_CurVelocity].GetSRV()[0]);
		Device::Effect->PostFX2Variable->SetResource(m_VelocityMap[m_OldVelocity].GetSRV()[0]);
		Device::Effect->Pass[PASS_POST_MOTION_BLUR]->Apply(0);
		g_pd3dDevice->Draw(6, 0);


#ifdef PHASE_DEBUG
		// Render the debug
		if(!m_bDemoMode)
			DebugRender();
#endif

		// Render the text
		RenderText();

	}


	//--------------------------------------------------------------------------------------
	// Renders the sky
	//--------------------------------------------------------------------------------------
	void Renderer::RenderSky(const D3DXVECTOR3& pos)
	{
		// Compute the scattering
		if(m_UpdateSky)
		{
			UINT offset=0;
			SetRenderToQuad();
			m_SkySystem.ComputeScattering(m_Camera.GetPos());
			m_UpdateSky = false;
		}
		
		// Render the sky
		m_HDRSystem.Buffer().BindRenderTarget();
		g_pd3dDevice->RSSetViewports(1, &m_HDRSystem.Buffer().GetViewport());
		Device::Effect->OrthoProjectionVariable->SetMatrix( (float*)&m_HDRSystem.Buffer().GetOrthoMatrix() );		
		m_SkySystem.Render(m_Camera.GetPos());
	}


	//--------------------------------------------------------------------------------------
	// Renders Water
	// Water
	//--------------------------------------------------------------------------------------
	void Renderer::RenderWater()
	{
		if(!m_pWater)
			return;

		// Restore the camera view and projection matrices
		D3DXMATRIX mViewProj = m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();
		Device::Effect->ViewProjectionVariable->SetMatrix((float*)&mViewProj);

		m_pWater->draw();

	}
	//--------------------------------------------------------------------------------------
	// Renders terrains
	//--------------------------------------------------------------------------------------
	void Renderer::RenderTerrain(Frustum& frustum, ID3D10EffectPass* pPass)
	{
		// Render the terrain
		if(!m_pTerrain || !m_pTerrain->IsLoaded())
			return;

		// Render the terrain
		Device::SetVertexBuffer(m_pTerrain->GetVertexBuffer(), PosVertex::size);
		Device::SetIndexBuffer(m_pTerrain->GetIndexBuffer(), DXGI_FORMAT_R16_UINT);
		Device::FrameStats.PolysDrawn += m_pTerrain->Render(*Device::Effect);		
	}




	////////////////////////////////////////////////////////////////////////////////////////
	// Renders all the probes
	////////////////////////////////////////////////////////////////////////////////////////
	void Renderer::RenderProbes()
	{
		// Only update probes every 2 frames
		static UINT frameCount = 0;
		frameCount++;
					
		// Make sure there are probes
		if(frameCount<2 || !(m_Probes.Size()>0)) return;
		frameCount = 0;
		
		// Save the old viewport
		D3D10_VIEWPORT OldVP;
		UINT cRT = 1;
		g_pd3dDevice->RSGetViewports( &cRT, &OldVP );

		// Set the viewport for rendering to the probes
		g_pd3dDevice->RSSetViewports( 1, &m_Probes[0]->GetSurface().GetViewport() );

		// Now update all the probe render targets
		Device::CacheRenderTarget();
		for(int i=0; i<m_Probes.Size(); i++)
			if(m_Probes[i]->IsDynamic())
				UpdateProbe(*m_Probes[i]);
		Device::RestoreCachedTargets();

		// Restore the camera view and projection matrices
		D3DXMATRIX mViewProj = m_Camera.GetViewMatrix() * m_Camera.GetProjMatrix();
		Device::Effect->ViewProjectionVariable->SetMatrix((float*)&mViewProj);

		// Restore the old viewport
		g_pd3dDevice->RSSetViewports( 1, &OldVP );
	}


	////////////////////////////////////////////////////////////////////////////////////////
	// Updates an environment probe
	////////////////////////////////////////////////////////////////////////////////////////
	void Renderer::UpdateProbe(EnvironmentProbe& probe)
	{
		// Make sure the probe is visible
		MeshObject& probeMesh = *probe.GetMesh();
		if(!m_Camera.GetFrustum().CheckSphere(probeMesh.GetPos(), probeMesh.GetRadius()))
			return;

		// Render to each face of the cube map
		D3DXVECTOR3 vLookDir;
		D3DXVECTOR3 vUpDir;
		D3DXMATRIX mView;
		Frustum frustum;
		for(int j=0; j<6; j++)
		{
			// Clear and set the render target/depth buffer
			probe.GetSurface().Clear(j);
			probe.GetSurface().ClearDSV();
			probe.GetSurface().BindRenderTarget(j);

			// Set the view matrix for this face
			switch( j )
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
			D3DXMatrixLookAtLH( &mView, &probeMesh.GetPos(), &(probeMesh.GetPos()+vLookDir), &vUpDir );
			mView *= m_mProbeProj;
			Device::Effect->ViewProjectionVariable->SetMatrix((float*)&mView);
			frustum.Build(&mView, &m_mProbeProj);

			// Now render each submesh
			Material* pCurrentMaterial = NULL;
			for(int i=0; i<m_MeshObjects.Size(); i++)
			{
				if(m_MeshObjects[i] == probe.GetMesh())
					continue;				
				MeshObject& mesh = *m_MeshObjects[i];
				if(!frustum.CheckSphere(mesh.GetPos(), mesh.GetRadius()))
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
					// Set the effect variables only when the material changes
					if( pCurrentMaterial != mesh.GetSubMesh(k)->pMaterial)
					{
						Device::SetMaterial( *mesh.GetSubMesh(k)->pMaterial );
						pCurrentMaterial = mesh.GetSubMesh(k)->pMaterial;
					}

					if( mesh.IsSkinned() )	
						Device::Effect->Pass[PASS_FORWARD_ANIM]->Apply(0);
					else
						Device::Effect->Pass[PASS_FORWARD]->Apply(0);

					// Render the mesh				
					Device::DrawSubmesh(*mesh.GetSubMesh(k));
				}	
			}

			// Render the terrain
			RenderTerrain(frustum, Device::Effect->Pass[PASS_FORWARD_TERRAIN]);

			// Render the sky
			RenderSky(probeMesh.GetPos());

			// Reset the material
			Device::SetMaterial(*g_Materials.GetDefault());
		}

		// Generate the mip maps
		if(ProbeSettings::miplevels>1)
			probe.GetSurface().GenerateMips();		
	}




	//--------------------------------------------------------------------------------------
	// Render the help and statistics text
	//--------------------------------------------------------------------------------------
	void Renderer::RenderText()
	{
		// Make sure proper render states are set
		Device::Effect->Pass[PASS_FORWARD]->Apply(0);

		// Now draw the text
		m_pTxtHelper->Begin();
		m_pTxtHelper->SetInsertionPos( 2, 0 );
		m_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
		//m_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );  
		//m_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );

		//if(m_bDebugMode)
		{
			String msg = "FPS: ";
			msg += Device::FrameStats.FPS;
			m_pTxtHelper->DrawTextLine(msg);
			
			msg = "Visible Lights: ";
			msg += m_VisibleLights.Size();
			msg += " / ";
			msg += m_Lights.Size();
			m_pTxtHelper->DrawTextLine(msg);

			msg = "Polygons Rendered: ";
			msg += Device::FrameStats.PolysDrawn;
			m_pTxtHelper->DrawTextLine(msg);
			
			msg = "Draw Calls: ";
			msg += Device::FrameStats.DrawCalls;
			m_pTxtHelper->DrawTextLine(msg);

			// Frame stats
			msg = "Material Changes: ";
			msg += Device::FrameStats.MaterialChanges;
			m_pTxtHelper->DrawTextLine(msg);

			msg = "Light Changes: ";
			msg += Device::FrameStats.LightChanges;
			m_pTxtHelper->DrawTextLine(msg);

			msg = "VB Changes: ";
			msg += Device::FrameStats.VBChanges;
			m_pTxtHelper->DrawTextLine(msg);

			msg = "IB Changes: ";
			msg += Device::FrameStats.IBChanges;
			m_pTxtHelper->DrawTextLine(msg);

			msg = "Messages Processed: ";
			msg += Device::FrameStats.ProcessedMessages;
			m_pTxtHelper->DrawTextLine(msg);

#ifdef PHASE_DEBUG
			if(m_pTerrain)
			{
				msg = "Terrain Sculpter Memory Usage: ";
				msg += m_pTerrain->GetMemoryUsage();
				msg += "MB";
				m_pTxtHelper->DrawTextLine(msg);
			}
#endif
		}

		m_pTxtHelper->End();

		g_pd3dDevice->IASetInputLayout( Vertex::pInputLayout );
		g_pd3dDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	}


	//--------------------------------------------------------------------------------------
	// Saves a screenshot (Thanks to Jack Hoxley)
	//--------------------------------------------------------------------------------------
	void Renderer::TakeScreenShot()
	{
		/*static int s_FrameNumber = 0;
		++s_FrameNumber;
		String outDir = g_szDirectory; 
		outDir += "Screen ";
		outDir += s_FrameNumber;
		outDir += ".jpg";

		// Extract the actual back buffer
		ID3D10Texture2D* pBackBuffer;
		DXUTGetDXGISwapChain()->GetBuffer( 0, __uuidof( ID3D10Texture2D ), reinterpret_cast< void** >( &pBackBuffer ) );

		// Save the backbuffer directly
		if( FAILED( D3DX10SaveTextureToFileA( pBackBuffer, D3DX10_IFF_JPG , outDir.c_str() ) ) )
		{
			Log::Print("Unable to save back-buffer texture to file!");
			return;
		}*/
	}

	//--------------------------------------------------------------------------------------
	// Encodes the material data needed for shading into a texture
	//--------------------------------------------------------------------------------------
	void Renderer::EncodeMaterialTexture()
	{
		// Destroy the material texture
		SAFE_RELEASE(m_pMaterialTex);
		SAFE_RELEASE(m_pMaterialSRV);

		// The number of bytes needed per material
		// (specular-float3, shading model-float)
		// (emissive-float4)
		// (surface props-float4)
		const UINT numFloats = 12;
		const UINT materialSize = numFloats*sizeof(float);

		// Number of materials
		const UINT numMaterials = g_Materials.GetList().Size();

		// Describe the texture
		D3D10_TEXTURE2D_DESC texDesc;
		texDesc.ArraySize           = 1;
		texDesc.BindFlags           = D3D10_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags      = 0;
		texDesc.Format              = DXGI_FORMAT_R32G32B32A32_FLOAT;
		texDesc.Height              = numMaterials;
		texDesc.Width               = materialSize / 16; // Texel is 16-bytes
		texDesc.MipLevels           = 1;
		texDesc.MiscFlags           = 0;
		texDesc.SampleDesc.Count    = 1;
		texDesc.SampleDesc.Quality  = 0;
		texDesc.Usage               = D3D10_USAGE_IMMUTABLE;

		// Generate the initial data
		float* pData = new float[ materialSize*numMaterials ];
		UINT index=0;
		for(UINT i=0; i<numMaterials; i++, index+=numFloats )
		{
			Material& mat = *g_Materials.GetList()[i];
			mat.ID = i;
			
			// Specular
			pData[index] = mat.GetSpecular().x;
			pData[index+1] = mat.GetSpecular().y;
			pData[index+2] = mat.GetSpecular().z;
			
			// Shading model
			pData[index+3] = mat.GetShadingModel();

			// Emissive
			pData[index+4] = mat.GetEmissive().x;
			pData[index+5] = mat.GetEmissive().y;
			pData[index+6] = mat.GetEmissive().z;
			pData[index+7] = mat.GetEmissive().w;

			// Surface properties
			pData[index+8 ] = mat.GetSurfaceParams().x;
			pData[index+9 ] = mat.GetSurfaceParams().y;
			pData[index+10] = mat.GetSurfaceParams().z;
			pData[index+11] = mat.GetSurfaceParams().w;
		}

		D3D10_SUBRESOURCE_DATA initialData;
		initialData.pSysMem          = pData;
		initialData.SysMemPitch      = materialSize;
		initialData.SysMemSlicePitch = 0;

		// Create the actual texture
		HRESULT hr = g_pd3dDevice->CreateTexture2D( &texDesc, &initialData, &m_pMaterialTex );
		if( FAILED( hr ) )
		{
			Log::Print( "Failed to create material texture" );

			delete[] pData;

			return;
		}

		// Create a view onto the texture
		hr = g_pd3dDevice->CreateShaderResourceView( m_pMaterialTex, NULL, &m_pMaterialSRV );
		if( FAILED( hr ) )
		{
			Log::Print( "Failed to create material srv" );

			delete[] pData;

			return;
		}

		// Bind it to the effect variable
		Device::Effect->EncodedMaterial->SetResource( m_pMaterialSRV );

		// Clear up any intermediary resources
		delete[] pData;
	}


	//--------------------------------------------------------------------------------------
	// Generates a random sampling texture for SSAO
	//--------------------------------------------------------------------------------------
	void Renderer::EncodeSSAOTexture(UINT width, UINT height)
	{
		
		width = 256;
		height = 256;
		
		// Describe the texture
		D3D10_TEXTURE2D_DESC texDesc;
		texDesc.ArraySize           = 1;
		texDesc.BindFlags           = D3D10_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags      = 0;
		texDesc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.Height              = height;
		texDesc.Width               = width;
		texDesc.MipLevels           = 1;
		texDesc.MiscFlags           = 0;
		texDesc.SampleDesc.Count    = 1;
		texDesc.SampleDesc.Quality  = 0;
		texDesc.Usage               = D3D10_USAGE_IMMUTABLE;

		// Generate the initial data
		BYTE* pData = new BYTE[ width*height*4 ];
		UINT index=0;
		for(UINT j=0; j<height; j++)
		for(UINT i=0; i<width; i++, index+=4 )
		{
			// Random value from 0-1
			pData[index] = ((float)(rand()%5000) / 5000.0f) * 255;
			pData[index+1] = ((float)(rand()%5000) / 5000.0f) * 255;
			pData[index+2] = ((float)(rand()%5000) / 5000.0f) * 255;
			pData[index+3] = ((float)(rand()%5000) / 5000.0f) * 255;
		}

		D3D10_SUBRESOURCE_DATA initialData;
		initialData.pSysMem          = pData;
		initialData.SysMemPitch      = width*4;
		initialData.SysMemSlicePitch = 0;

		// Create the actual texture
		HRESULT hr = g_pd3dDevice->CreateTexture2D( &texDesc, &initialData, &m_pRandomTex );
		if( FAILED( hr ) )
		{
			Log::Print( "Failed to create random SSAO texture" );

			delete[] pData;

			return;
		}

		// Create a view onto the texture
		hr = g_pd3dDevice->CreateShaderResourceView( m_pRandomTex, NULL, &m_pRandomSRV );
		if( FAILED( hr ) )
		{
			Log::Print( "Failed to create random SSAO srv" );

			delete[] pData;

			return;
		}

		// Bind it to the effect variable
		Device::Effect->RandomVariable->SetResource( m_pRandomSRV );

		// Clear up any intermediary resources
		delete[] pData;
	}

}