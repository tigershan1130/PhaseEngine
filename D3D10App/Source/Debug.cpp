//--------------------------------------------------------------------------------------
// File: Debug.cpp
//
// Support for selecting clicked objects and rendering debug information
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "Renderer.h"

#ifdef PHASE_DEBUG

#include "DirectInput.h"

namespace Core
{


	//--------------------------------------------------------------------------------------
	// Setup the debug renderer
	//--------------------------------------------------------------------------------------
	HRESULT Renderer::InitDebugRenderer()
	{
		// Setup the light outline lines
		float edge = 0.35f;
		Vertex pLightOutline[16];
		pLightOutline[0].pos = D3DXVECTOR3(-1.0f, 1.0f-edge, 0.0f);
		pLightOutline[1].pos = D3DXVECTOR3(-1.0f, 1.0f, 0.0f);
		pLightOutline[2].pos = D3DXVECTOR3(-1.0f, 1.0f, 0.0f);
		pLightOutline[3].pos = D3DXVECTOR3(-1.0f+edge, 1.0f, 0.0f);

		pLightOutline[4].pos = D3DXVECTOR3(1.0f-edge, 1.0f, 0.0f);
		pLightOutline[5].pos = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
		pLightOutline[6].pos = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
		pLightOutline[7].pos = D3DXVECTOR3(1.0f, 1.0f-edge, 0.0f);

		pLightOutline[8].pos = D3DXVECTOR3(1.0f, -1.0f+edge, 0.0f);
		pLightOutline[9].pos = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
		pLightOutline[10].pos = D3DXVECTOR3(1.0f, -1.0f, 0.0f);
		pLightOutline[11].pos = D3DXVECTOR3(1.0f-edge, -1.0f, 0.0f);

		pLightOutline[12].pos = D3DXVECTOR3(-1.0f+edge, -1.0f, 0.0f);
		pLightOutline[13].pos = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
		pLightOutline[14].pos = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
		pLightOutline[15].pos = D3DXVECTOR3(-1.0f, -1.0f+edge, 0.0f);

		// Create the vertex buffer for the line outline
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof( Vertex ) * 16;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = (Vertex*)pLightOutline;
		HRESULT hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &m_pDebugLineOutlineVB );
		if( FAILED(hr) )  
			return hr;

		// Create the single line buffer
		Vertex line[2];
		line[0].pos = D3DXVECTOR3(0,0,0);
		line[1].pos = D3DXVECTOR3(0,0,0.7f);
		bd.Usage = D3D10_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof( Vertex ) * 2;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		InitData.pSysMem = (Vertex*)line;
		hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &m_pDebugLineVB );
		if( FAILED(hr) )  
			return hr;

		// Create the line box buffer
		Vertex lineBox[48];

		// Front face
		lineBox[0].pos = D3DXVECTOR3(-1.0f,  1.0f, -1.0f);
		lineBox[1].pos = D3DXVECTOR3( 1.0f,  1.0f, -1.0f);
		lineBox[2].pos = D3DXVECTOR3( 1.0f,  1.0f, -1.0f);
		lineBox[3].pos = D3DXVECTOR3( 1.0f, -1.0f, -1.0f);
		lineBox[4].pos = D3DXVECTOR3( 1.0f, -1.0f, -1.0f);
		lineBox[5].pos = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		lineBox[6].pos = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		lineBox[7].pos = D3DXVECTOR3(-1.0f,  1.0f, -1.0f);

		// Back face
		lineBox[15].pos = D3DXVECTOR3(-1.0f,  1.0f, 1.0f);
		lineBox[14].pos = D3DXVECTOR3( 1.0f,  1.0f, 1.0f);
		lineBox[13].pos = D3DXVECTOR3( 1.0f,  1.0f, 1.0f);
		lineBox[12].pos = D3DXVECTOR3( 1.0f, -1.0f, 1.0f);
		lineBox[11].pos = D3DXVECTOR3( 1.0f, -1.0f, 1.0f);
		lineBox[10].pos = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
		lineBox[9].pos = D3DXVECTOR3(-1.0f, -1.0f, 1.0f);
		lineBox[8].pos = D3DXVECTOR3(-1.0f,  1.0f, 1.0f);

		// Left face
		lineBox[16].pos = D3DXVECTOR3(-1.0f,  1.0f,  1.0f);
		lineBox[17].pos = D3DXVECTOR3(-1.0f,  1.0f, -1.0f);
		lineBox[18].pos = D3DXVECTOR3(-1.0f,  1.0f, -1.0f);
		lineBox[19].pos = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		lineBox[20].pos = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		lineBox[21].pos = D3DXVECTOR3(-1.0f, -1.0f,  1.0f);
		lineBox[22].pos = D3DXVECTOR3(-1.0f, -1.0f,  1.0f);
		lineBox[23].pos = D3DXVECTOR3(-1.0f,  1.0f,  1.0f);

		// Right face
		lineBox[31].pos = D3DXVECTOR3( 1.0f,  1.0f,  1.0f);
		lineBox[30].pos = D3DXVECTOR3( 1.0f,  1.0f, -1.0f);
		lineBox[29].pos = D3DXVECTOR3( 1.0f,  1.0f, -1.0f);
		lineBox[28].pos = D3DXVECTOR3( 1.0f, -1.0f, -1.0f);
		lineBox[27].pos = D3DXVECTOR3( 1.0f, -1.0f, -1.0f);
		lineBox[26].pos = D3DXVECTOR3( 1.0f, -1.0f,  1.0f);
		lineBox[25].pos = D3DXVECTOR3( 1.0f, -1.0f,  1.0f);
		lineBox[24].pos = D3DXVECTOR3( 1.0f,  1.0f,  1.0f);

		// Top face
		lineBox[32].pos = D3DXVECTOR3(-1.0f,  1.0f,  1.0f);
		lineBox[33].pos = D3DXVECTOR3( 1.0f,  1.0f,  1.0f);
		lineBox[34].pos = D3DXVECTOR3( 1.0f,  1.0f,  1.0f);
		lineBox[35].pos = D3DXVECTOR3( 1.0f,  1.0f, -1.0f);
		lineBox[36].pos = D3DXVECTOR3( 1.0f,  1.0f, -1.0f);
		lineBox[37].pos = D3DXVECTOR3(-1.0f,  1.0f, -1.0f);
		lineBox[38].pos = D3DXVECTOR3(-1.0f,  1.0f, -1.0f);
		lineBox[39].pos = D3DXVECTOR3(-1.0f,  1.0f,  1.0f);

		// Bottom face
		lineBox[47].pos = D3DXVECTOR3(-1.0f, -1.0f,  1.0f);
		lineBox[46].pos = D3DXVECTOR3( 1.0f, -1.0f,  1.0f);
		lineBox[45].pos = D3DXVECTOR3( 1.0f, -1.0f,  1.0f);
		lineBox[44].pos = D3DXVECTOR3( 1.0f, -1.0f, -1.0f);
		lineBox[43].pos = D3DXVECTOR3( 1.0f, -1.0f, -1.0f);
		lineBox[42].pos = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		lineBox[41].pos = D3DXVECTOR3(-1.0f, -1.0f, -1.0f);
		lineBox[40].pos = D3DXVECTOR3(-1.0f, -1.0f,  1.0f);

		// Create the box line buffer
		bd.Usage = D3D10_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof( Vertex ) * 48;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		InitData.pSysMem = (Vertex*)lineBox;
		hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &m_pDebugLineBoxVB );
		if( FAILED(hr) )  
			return hr;

		// Load light texture
		if( !(m_pLightTex = g_Textures.Load("Textures\\Old\\lightbulb.jpg") ))
			return E_FAIL;

		// Load the cone mesh
		BaseMesh::bLoadMaterials=false;
		if(!m_DebugAxisCone.Load("Models\\Editor\\Axis Cone.x"))
			return E_FAIL;
		if(!m_DebugAxisPyre.Load("Models\\Editor\\Axis Pyre.x"))
			return E_FAIL;
		if(!m_DebugAxisBall.Load("Models\\Editor\\Axis Ball.x"))
			return E_FAIL;
		

		m_iPickedSubMesh = 0;
		m_DebugLightSize = 1.0f;
		m_DebugRenderMode = DEBUG_SELECT;
		m_SelectedDebugAxis = AXIS_NONE;

		// Setup for material preview rendering
		m_DebugMaterialPreivewMesh.Load("Models\\Old\\sphere.x");
		BaseMesh::bLoadMaterials=true;
		DXGI_SAMPLE_DESC sd;
		sd.Count = 8;
		sd.Quality = 16;
		m_DebugMaterialPreviewMSAA.Create(106, 103, DXGI_FORMAT_R8G8B8A8_UNORM, 1, &sd);
		m_DebugMaterialPreviewDepthMSAA.Create(106, 103, &sd, false);
		m_DebugMaterialPreviewMSAA.AttachDepthStencil(m_DebugMaterialPreviewDepthMSAA);
		m_DebugMaterialPreview.Create(106, 103, DXGI_FORMAT_R8G8B8A8_UNORM, 1, NULL);

		// Setup for mesh preview rendering
		m_DebugMeshPreviewMSAA.Create(189, 134, DXGI_FORMAT_R8G8B8A8_UNORM, 1, &sd);
		m_DebugMeshPreviewDepthMSAA.Create(189, 134, &sd, false);
		m_DebugMeshPreviewMSAA.AttachDepthStencil(m_DebugMeshPreviewDepthMSAA);
		m_DebugMeshPreview.Create(189, 134, DXGI_FORMAT_R8G8B8A8_UNORM, 1, NULL);
		
		return S_OK;
	}



	//--------------------------------------------------------------------------------------
	// Release debug renderer
	//--------------------------------------------------------------------------------------
	void Renderer::ReleaseDebugRenderer()
	{
		SAFE_RELEASE(m_pDebugLineOutlineVB);
		SAFE_RELEASE(m_pDebugLineVB);
		SAFE_RELEASE(m_pDebugLineBoxVB);
		g_Textures.Deref(m_pLightTex);
		m_pLightTex = NULL;
		m_DebugRenderLightList.Release();
		m_DebugAxisCone.Release();
		m_DebugAxisPyre.Release();
		m_DebugAxisBall.Release();
		m_DebugRenderMeshList.Release();
		m_DebugMaterialPreview.Release();
		m_DebugMaterialPreivewMesh.Release();
		m_DebugMaterialPreviewMSAA.Release();
		m_DebugMaterialPreviewDepthMSAA.Release();

		m_DebugMeshPreviewMSAA.Release();
		m_DebugMeshPreviewDepthMSAA.Release();
		m_DebugMeshPreviewMSAA.Release();
		m_DebugMeshPreview.Release();
	}


	//--------------------------------------------------------------------------------------
	// Adds a mesh to the debug render list
	//--------------------------------------------------------------------------------------
	void Renderer::DebugRenderMesh(MeshObject* pMesh, int submesh, bool wire, bool boundingBox, D3DXVECTOR4 color1, D3DXVECTOR4 color2)
	{
		DebugRenderTarget rt;
		rt.pMesh = pMesh;
		rt.pos = pMesh->GetPos();
		rt.submesh = submesh;
		rt.color[0] = color1;
		rt.color[1] = color2;
		rt.bWireframe = wire;
		rt.boundingBox = boundingBox;
		m_DebugRenderMeshList.Add(rt);
	}

	//--------------------------------------------------------------------------------------
	// Adds a bounding box to the debug render list
	//--------------------------------------------------------------------------------------
	void Renderer::DebugRenderBox(D3DXVECTOR3 pos, D3DXVECTOR3 size, bool wire, D3DXVECTOR4 color1, D3DXVECTOR4 color2)
	{
		DebugRenderTarget rt;
		rt.pos = pos;
		rt.boundSize = size;
		rt.color[0] = color1;
		rt.color[1] = color2;
		rt.bWireframe = wire;
		rt.boundingBox = true;
		m_DebugRenderMeshList.Add(rt);
	}


	//--------------------------------------------------------------------------------------
	// Render lights
	//--------------------------------------------------------------------------------------
	void Renderer::DebugRender()
	{
		// Timing
		static int frames=0;
		frames++;
		if(frames==4){ frames=0; DebugCheckAxis(); }
		
		////////////////////////////////////////
		// Render terrain quadtree
		//if(m_pTerrain)
		//m_pTerrain->RenderQuadtreeDebug(m_BoxMesh, Device::Effect);
			//	m_pTerrain->RenderDebug(m_BoxMesh, Device::Effect);
		
		////////////////////////////////////////
		// Render lights

		// Set textures
		ID3D10ShaderResourceView* pTex[TEX_SIZE];
		BOOL bTex[TEX_SIZE];
		for(int i=0; i<TEX_SIZE; i++)
		{
			bTex[i] = false;
			pTex[i] = NULL;
		}
		bTex[0] = bTex[3] = TRUE;
		pTex[0] = pTex[3] = m_pLightTex->GetResource();
		Device::Effect->MaterialTextureVariable->SetResourceArray(pTex, 0, TEX_SIZE);
		Device::Effect->MaterialTextureFlagVariable->SetBoolArray(bTex, 0, TEX_SIZE);

		// Billboard matrix
		D3DXMATRIX mWorld, mR, mT, mS;
		D3DXMatrixScaling(&mS, m_DebugLightSize*0.35f, m_DebugLightSize*0.5f, 0.0f);
		D3DXMatrixRotationYawPitchRoll(&mR, m_Camera.GetYDeg(), m_Camera.GetXDeg(), 0);

		// Set vertex buffer
		SetRenderToQuad();	
		
		// Render each light as a bilboarded quad
		for(int i=0; i<m_Lights.Size(); i++)
		{
			// Skip directional lights
			if(m_Lights[i]->Type==Light::LIGHT_DIRECTIONAL)
				continue;
			
			// Set proper color
			Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)&m_Lights[i]->Color);

			// Set world matrix
			D3DXMatrixTranslation(&mT, m_Lights[i]->GetPos().x, m_Lights[i]->GetPos().y, m_Lights[i]->GetPos().z);
			mWorld = mS*mR*mT;	
			Device::Effect->WorldMatrixVariable->SetMatrix((float*)&mWorld);

			// Render the quad
			Device::Effect->Pass[PASS_FORWARD]->Apply(0);
			g_pd3dDevice->Draw(6, 0);
		}	

		// Render selected light border
		float dist = 1000;
		for(int i=0; i<m_DebugRenderLightList.Size(); i++)
		{
			if(m_DebugRenderLightList[i]->Type == Light::LIGHT_DIRECTIONAL)
				continue;

			Device::SetVertexBuffer(m_pDebugLineOutlineVB, Vertex::size);
			g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
			D3DXVECTOR3 color(0,0.5f,0.95f);
			Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)&color);
			bTex[0] = bTex[3] = FALSE;
			Device::Effect->MaterialTextureFlagVariable->SetBoolArray(bTex, 0, TEX_SIZE);
			for(float s=1.0f; s>0.95f; s-=0.0004f)
			{
				// Set world matrix
				D3DXMatrixTranslation(&mT, m_DebugRenderLightList[i]->GetPos().x, m_DebugRenderLightList[i]->GetPos().y, m_DebugRenderLightList[i]->GetPos().z);
				D3DXMatrixScaling(&mS, 0.6f*m_DebugLightSize*(s-0.3f),0.6f*m_DebugLightSize*s,0);
				mWorld = mS*mR*mT;	
				Device::Effect->WorldMatrixVariable->SetMatrix((float*)&mWorld);

				// Render the lines
				Device::Effect->Pass[PASS_WIREFRAME]->Apply(0);
				g_pd3dDevice->Draw(16, 0);
			}
			g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Render the meshes it contains
			for(int j=0; j<m_DebugRenderLightList[i]->MeshList.Size(); j++)
			{
				MeshObject& mesh = *m_DebugRenderLightList[i]->MeshList[j];
				RenderMesh(mesh, Device::Effect->Pass[PASS_WIREFRAME]);
			}

			// Set world matrix
			/*if(m_DebugRenderLightList[i]->Type == Light::LIGHT_POINT)
			{
				D3DXMatrixTranslation(&mT, m_DebugRenderLightList[i]->GetPos().x, m_DebugRenderLightList[i]->GetPos().y, m_DebugRenderLightList[i]->GetPos().z);
				float ss = m_DebugRenderLightList[i]->GetRange();
				D3DXMatrixScaling(&mS,ss,ss,ss);
				mWorld = mS*mT;	
				Device::Effect->WorldMatrixVariable->SetMatrix((float*)&mWorld);

				// Render the lines
				Device::Effect->Pass[PASS_WIREFRAME]->Apply(0);
				m_SphereMesh.GetSubMesh(0)->Render();
			}
			else if(m_DebugRenderLightList[i]->Type == Light::LIGHT_SPOT)
			{
				// Build the world matrix for the light geometry
				D3DXMatrixScaling(&mS, m_DebugRenderLightList[i]->GetRange(), m_DebugRenderLightList[i]->GetRange(), m_DebugRenderLightList[i]->GetRange());
				Math::BuildRotationMatrix(m_DebugRenderLightList[i]->GetDir(), mR);
				D3DXVECTOR3 r = m_DebugRenderLightList[i]->GetPos() + m_DebugRenderLightList[i]->GetDir()*0.5f*m_DebugRenderLightList[i]->GetRange();
				D3DXMatrixTranslation(&mT, r.x, r.y, r.z);
				m_DebugRenderLightList[i]->worldMatrix = mS * mR * mT;
				Device::Effect->WorldMatrixVariable->SetMatrix((float*)&m_DebugRenderLightList[i]->worldMatrix);

				// Render the lines
				Device::Effect->Pass[PASS_WIREFRAME]->Apply(0);
				m_ConeMesh.GetSubMesh(0)->Render();
			}*/

			// Render the debug axes
			DebugRenderAxes(m_DebugRenderLightList[i]->GetPos(), m_DebugRenderMode);
		}
		m_DebugRenderLightList.Clear();

		// Render the debug meshes
		bTex[0] = bTex[3] = FALSE;
		Device::Effect->MaterialTextureFlagVariable->SetBoolArray(bTex, 0, TEX_SIZE);
		for(int i=0; i<m_DebugRenderMeshList.Size(); i++)
		{
			// Handle terrains
			if(m_DebugRenderMeshList[i].TerrainWidget)
			{
				DebugRenderTerrainWidget();
				continue;
			}
			
			// Get the mesh
			MeshObject* pMesh = m_DebugRenderMeshList[i].pMesh;
			
			// Pass the matrix pallete to the shader (for skinned meshes)
			if(pMesh)
			{
				if(pMesh->IsSkinned())
				{
					Device::Effect->AnimMatricesVariable->SetMatrixArray((float*)pMesh->GetAnimMatrices(), 0, pMesh->GetNumAnimMatrices());

					// Set the input layout
					Device::SetInputLayout( SkinnedVertex::pInputLayout );
				}

				// Render each submesh
				if(m_DebugRenderMeshList[i].bWireframe && m_DebugRenderMode==DEBUG_SELECT)
					for(int k=0; k<pMesh->GetNumSubMesh(); k++)
					{
						// Set world matrix
						Device::Effect->WorldMatrixVariable->SetMatrix((float*)*pMesh->GetSubMesh(k)->pWorldMatrix);

						// Color
						if(k == m_DebugRenderMeshList[i].submesh)
							Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)m_DebugRenderMeshList[i].color[1]);
						else
							Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)m_DebugRenderMeshList[i].color[0]);

						// Render the wireframe
						if(pMesh->IsSkinned())
							Device::Effect->Pass[PASS_WIREFRAME_ANIM]->Apply(0);
						else
							Device::Effect->Pass[PASS_WIREFRAME]->Apply(0);
						Device::DrawSubmesh(*pMesh->GetSubMesh(k));
					}
			}

			// Render the bounding box
			if(!m_pFocusMesh && m_DebugRenderMeshList[i].boundingBox)
			{
				Device::SetInputLayout( Vertex::pInputLayout );

				// Get the box size and location
				if(pMesh)
				{
					D3DXMatrixScaling(&mS, pMesh->GetBoundSize().x, pMesh->GetBoundSize().y, pMesh->GetBoundSize().z);
					D3DXMatrixTranslation(&mT, pMesh->GetPos().x, pMesh->GetPos().y, pMesh->GetPos().z);
				}
				else
				{
					D3DXMatrixScaling(&mS, m_DebugRenderMeshList[i].boundSize.x, m_DebugRenderMeshList[i].boundSize.y, m_DebugRenderMeshList[i].boundSize.z);
					D3DXMatrixTranslation(&mT, m_DebugRenderMeshList[i].pos.x, m_DebugRenderMeshList[i].pos.y, m_DebugRenderMeshList[i].pos.z);
				}

				// Set world matrix
				mWorld = mS*mT;
				Device::Effect->WorldMatrixVariable->SetMatrix((float*)&mWorld);

				// Color
				Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)D3DXVECTOR4(1.0f, 1.0f, 1.0f,0));	

				// Render the lines
				Device::Effect->Pass[PASS_WIREFRAME]->Apply(0);
				g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
				Device::SetVertexBuffer(m_pDebugLineBoxVB, Vertex::size);
				g_pd3dDevice->Draw(48, 0);
				g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				//D3DXMatrixScaling(&mS, pMesh->GetSubMesh(m_DebugRenderMeshList[i].submesh)->boundSize.x, m_DebugRenderMeshList[i].submesh)->boundSize.y, m_DebugRenderMeshList[i].submesh)->boundSize.z);
				//D3DXMatrixTranslation(&mT, pMesh->GetPos().x, pMesh->GetPos().y, pMesh->GetPos().z);
				//mWorld = mS*mT;
				//Device::Effect->WorldMatrixVariable->SetMatrix((float*)&mWorld);

				//// Color
				//Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)D3DXVECTOR4(1.0f, 1.0f, 1.0f,0));	

				//// Render the lines
				//Device::Effect->Pass[PASS_WIREFRAME]->Apply(0);
				//g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
				//g_pd3dDevice->IASetVertexBuffers( 0, 1, &m_pDebugLineBoxVB, &Vertex::size, &offset );
				//g_pd3dDevice->Draw(48, 0);
				//g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}

			// Render the transform axes
			if(!m_pFocusMesh)
				DebugRenderAxes(m_DebugRenderMeshList[i].pos, m_DebugRenderMode);
		}
		m_DebugRenderMeshList.Clear();


		// Selection box
		if(m_DebugRenderSelectionBox)
			DebugSelectionBox();
		m_DebugRenderSelectionBox=false;
		m_DebugTerrainSculpt=false;

		// Grid lines
		if(m_DebugDrawGrid)
		{
			// Render the widget
			Device::Effect->Pass[PASS_EDITOR_GRID]->Apply(0);
			g_pd3dDevice->Draw(6, 0);
		}
	}

	
	//--------------------------------------------------------------------------------------
	// Checks which debug axis is selected
	//--------------------------------------------------------------------------------------
	void Renderer::DebugCheckAxis()
	{
		if(m_DebugRenderMode==DEBUG_SELECT || Input::MouseLeftClick())
			return;
		
		// Reset the axis selection
		m_SelectedDebugAxis=AXIS_NONE;

		// Get the right mesh
		MeshObject* pAxisMesh;
		if(m_DebugRenderMode==DEBUG_MOVE)
			pAxisMesh = &m_DebugAxisCone;
		else if(m_DebugRenderMode==DEBUG_SCALE)
			pAxisMesh = &m_DebugAxisPyre;
		else if(m_DebugRenderMode==DEBUG_ROTATE)
			pAxisMesh = &m_DebugAxisBall;
		else return;

		// Get the picking ray
		D3DXVECTOR3 rayPos, rayDir;
		float t,u,v;
		int submesh;
		Math::GetPickRay(m_MousePos.x, m_MousePos.y, m_Camera, rayPos, rayDir);

		// Check for each selected mesh
		float dist = 1000;
		for(int i=0; i<m_DebugRenderLightList.Size(); i++)
		{
			// Set the scale
			float scale = (D3DXVec3Length(&(m_Camera.GetPos()-m_DebugRenderLightList[i]->GetPos()))+10) / 15.0f;
			pAxisMesh->SetScale(scale,scale,scale);

			// Z-Axis
			pAxisMesh->SetRot(0,0,0);
			pAxisMesh->SetPos(m_DebugRenderLightList[i]->GetPos().x, m_DebugRenderLightList[i]->GetPos().y, m_DebugRenderLightList[i]->GetPos().z);
			if(RayIntersectMesh(pAxisMesh, rayPos, rayDir, &t, &u, &v, &submesh) && t<dist)
			{
				dist=t;
				m_SelectedDebugAxis = AXIS_Z;
			}

			// X-Axis
			pAxisMesh->SetRot(0, D3DX_PI/2.0f, 0);
			if(RayIntersectMesh(pAxisMesh, rayPos, rayDir, &t, &u, &v, &submesh) && t<dist)
			{
				dist=t;
				m_SelectedDebugAxis = AXIS_X;
			}

			// Y-Axis
			pAxisMesh->SetRot(-D3DX_PI/2.0f, 0, 0);
			if(RayIntersectMesh(pAxisMesh, rayPos, rayDir, &t, &u, &v, &submesh) && t<dist)
			{
				dist=t;
				m_SelectedDebugAxis = AXIS_Y;
			}
		}
				
		// Check for each selected mesh
		for(int i=0; i<m_DebugRenderMeshList.Size(); i++)
		{
			// Set the scale
			float scale = (D3DXVec3Length(&(m_Camera.GetPos()-m_DebugRenderMeshList[i].pos))+10) / 15.0f;
			pAxisMesh->SetScale(scale,scale,scale);
			
			// Z-Axis
			pAxisMesh->SetRot(0,0,0);
			pAxisMesh->SetPos(m_DebugRenderMeshList[i].pos);
			if(RayIntersectMesh(pAxisMesh, rayPos, rayDir, &t, &u, &v, &submesh) && t<dist)
			{
				dist=t;
				m_SelectedDebugAxis = AXIS_Z;
			}

			// X-Axis
			pAxisMesh->SetRot(0, D3DX_PI/2.0f, 0);
			if(RayIntersectMesh(pAxisMesh, rayPos, rayDir, &t, &u, &v, &submesh) && t<dist)
			{
				dist=t;
				m_SelectedDebugAxis = AXIS_X;
			}

			// Y-Axis
			pAxisMesh->SetRot(-D3DX_PI/2.0f, 0, 0);
			if(RayIntersectMesh(pAxisMesh, rayPos, rayDir, &t, &u, &v, &submesh) && t<dist)
			{
				dist=t;
				m_SelectedDebugAxis = AXIS_Y;
			}
		}
	}


	//--------------------------------------------------------------------------------------
	// Renders the transform axes at the given location
	//--------------------------------------------------------------------------------------
	void Renderer::DebugRenderAxes(const D3DXVECTOR3& pos, DEBUG_TYPE type)
	{
		if(type==DEBUG_SELECT)
			return;
		
		// Get the right mesh
		UINT offset=0;		
		MeshObject* pAxisMesh;
		if(m_DebugRenderMode==DEBUG_MOVE)
			pAxisMesh = &m_DebugAxisCone;
		else if(m_DebugRenderMode==DEBUG_SCALE)
			pAxisMesh = &m_DebugAxisPyre;
		else if(m_DebugRenderMode==DEBUG_ROTATE)
			pAxisMesh = &m_DebugAxisBall;
		else return;

		// Set the scale
		float scale = (D3DXVec3Length(&(m_Camera.GetPos()-pos))+10) / 15.0f;
		pAxisMesh->SetScale(scale,scale,scale);
		
		// Z-Axis (green)
		{
			// Position the axis
			pAxisMesh->SetRot(0,0,0);
			pAxisMesh->SetPos(pos.x, pos.y, pos.z);
			Device::Effect->WorldMatrixVariable->SetMatrix((float*)&pAxisMesh->GetWorldMatrix());
			
			// Set the color
			if(m_SelectedDebugAxis == AXIS_Z)
				Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)&D3DXVECTOR4(1,1,0,0));
			else
				Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)&D3DXVECTOR4(0,1,0,0));

			// Render the line
			Device::Effect->Pass[PASS_WIREFRAME_NODEPTH]->Apply(0);
			g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
			Device::SetVertexBuffer(m_pDebugLineVB, Vertex::size);
			g_pd3dDevice->Draw(2, 0);

			// Render the mesh
			g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			RenderMesh(*pAxisMesh, Device::Effect->Pass[PASS_FORWARD_NODEPTH]);
		}

		// X-Axis (red)
		{
			// Position the axis
			pAxisMesh->SetRot(0, D3DX_PI/2.0f, 0);
			Device::Effect->WorldMatrixVariable->SetMatrix((float*)&pAxisMesh->GetWorldMatrix());

			// Set the color
			if(m_SelectedDebugAxis == AXIS_X)
				Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)&D3DXVECTOR4(1,1,0,0));
			else
				Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)&D3DXVECTOR4(1,0,0,0));

			// Render the line
			Device::Effect->Pass[PASS_WIREFRAME_NODEPTH]->Apply(0);
			g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
			Device::SetVertexBuffer(m_pDebugLineVB, Vertex::size);
			g_pd3dDevice->Draw(2, 0);

			// Render the mesh
			g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			RenderMesh(*pAxisMesh, Device::Effect->Pass[PASS_FORWARD_NODEPTH]);
		}

		// Y-Axis (blue)
		{
			// Position the axis
			pAxisMesh->SetRot(-D3DX_PI/2.0f, 0, 0);
			Device::Effect->WorldMatrixVariable->SetMatrix((float*)&pAxisMesh->GetWorldMatrix());

			// Set the color
			if(m_SelectedDebugAxis == AXIS_Y)
				Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)&D3DXVECTOR4(1,1,0,0));
			else
				Device::Effect->MaterialDiffuseVariable->SetFloatVector((float*)&D3DXVECTOR4(0,0,1,0));

			// Render the line
			Device::Effect->Pass[PASS_WIREFRAME_NODEPTH]->Apply(0);
			g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
			Device::SetVertexBuffer(m_pDebugLineVB, Vertex::size);
			g_pd3dDevice->Draw(2, 0);

			// Render the mesh
			g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			RenderMesh(*pAxisMesh, Device::Effect->Pass[PASS_FORWARD_NODEPTH]);
		}
	}


	//--------------------------------------------------------------------------------------
	// Renders a selection box on the screen
	//--------------------------------------------------------------------------------------
	void Renderer::DebugSelectionBox()
	{
		// Dont deal with this when in the middle of a transform
		if(m_SelectedDebugAxis!=AXIS_NONE) return;
		
		// Check for a click
		static POINT lastMouseClickPoint;
		static bool isClicking = false;
		if(Input::MouseLeftClick() && !isClicking)
		{
			// If the click is just happening, set the old position
			lastMouseClickPoint = m_MousePos;
			isClicking = true;
		}

		// Unclick
		if(!Input::MouseLeftClick() && isClicking)
		{
			// Get the mouse position
			isClicking=false;
		}
		
		// If the mouse isnt clicked, dont render the box
		if(!isClicking)
			return;

		// Get the current mouse coords

		// Build the selection frustum
		D3DXVECTOR3 r, d[4];
		int minx = (m_MousePos.x < lastMouseClickPoint.x) ? (int)m_MousePos.x : (int)lastMouseClickPoint.x;
		int miny = (m_MousePos.y > lastMouseClickPoint.y) ? (int)m_MousePos.y : (int)lastMouseClickPoint.y;
		int maxx = (m_MousePos.x > lastMouseClickPoint.x) ? (int)m_MousePos.x : (int)lastMouseClickPoint.x;
		int maxy = (m_MousePos.y < lastMouseClickPoint.y) ? (int)m_MousePos.y : (int)lastMouseClickPoint.y;
		Math::GetPickRay(minx, maxy, m_Camera, r, d[0]);
		Math::GetPickRay(maxx, maxy, m_Camera, r, d[1]);
		Math::GetPickRay(maxx, miny, m_Camera, r, d[2]);
		Math::GetPickRay(minx, miny, m_Camera, r, d[3]);
		m_SelectionBoxFrustum.Build(m_Camera.GetPos(), d, 0.1f, 150.0f);

				
		// Make the verts
		// (p1 and p2 are two opposing corners of the box)
		Vertex verts[6];
		memcpy(verts, m_QuadVerts, 6*sizeof(Vertex));
		D3DXVECTOR2 p1 = D3DXVECTOR2((float)lastMouseClickPoint.x,(float)lastMouseClickPoint.y);
		D3DXVECTOR2 p2= D3DXVECTOR2((float)m_MousePos.x,(float)m_MousePos.y);
		p1.x = 2.0f*p1.x/m_Width - 1.0f;
		p1.y = -(2.0f*p1.y/m_Height - 1.0f);
		p2.x = 2.0f*p2.x/m_Width - 1.0f;
		p2.y = -(2.0f*p2.y/m_Height - 1.0f);
		verts[0].pos = D3DXVECTOR3(p1.x, p1.y, 0.5f);
		verts[1].pos = D3DXVECTOR3(p2.x, p1.y, 0.5f);
		verts[2].pos = D3DXVECTOR3(p2.x, p2.y, 0.5f);
		verts[3].pos = D3DXVECTOR3(p2.x, p2.y, 0.5f);
		verts[4].pos = D3DXVECTOR3(p1.x, p2.y, 0.5f);
		verts[5].pos = D3DXVECTOR3(p1.x, p1.y, 0.5f);

		// Fill the vertex buffer
		Vertex* pVerts; 
		m_pQuadVertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, (void**)&pVerts);
		memcpy(pVerts, verts, sizeof(Vertex)*6);
		m_pQuadVertexBuffer->Unmap();

		// Set the effect
		Device::Effect->Pass[PASS_EDITOR_SELECTIONBOX]->Apply(0);

		// Render the buffer
		SetRenderToQuad();
		g_pd3dDevice->Draw(6, 0);
	}


	//--------------------------------------------------------------------------------------
	// Render the terrain sculpt tool
	//--------------------------------------------------------------------------------------
	void Renderer::DebugRenderTerrainWidget()
	{
		if(!m_pTerrain) return;

		// Render the widget
		static bool mouseDown = false;
		static D3DXVECTOR2 mouseTexCoords;
		bool update = false;
		if(MouseOnScreen())
		{
			// Get the intersection point (only on the first click for grab mode)
			if(m_DebugTerrainSculptMode==SCULPT_GRAB)
			{
				if(Input::MouseLeftClick() && !mouseDown)
				{
					mouseTexCoords = D3DXVECTOR2(m_MousePos.x, m_MousePos.y);
					mouseTexCoords.x /= (float)m_Width;
					mouseTexCoords.y /= (float)m_Height;
					Device::Effect->TerrainWidgetTexCoordVariable->SetFloatVector((float*)&mouseTexCoords);
					mouseDown=true;
					update = true;
				}
				if(!Input::MouseLeftClick())
				{
					mouseTexCoords = D3DXVECTOR2(m_MousePos.x, m_MousePos.y);
					mouseTexCoords.x /= (float)m_Width;
					mouseTexCoords.y /= (float)m_Height;
					Device::Effect->TerrainWidgetTexCoordVariable->SetFloatVector((float*)&mouseTexCoords);
					mouseDown = false;
					update = true;
				}
			}
			else
			{
				mouseTexCoords = D3DXVECTOR2(m_MousePos.x, m_MousePos.y);
				mouseTexCoords.x /= (float)m_Width;
				mouseTexCoords.y /= (float)m_Height;
				Device::Effect->TerrainWidgetTexCoordVariable->SetFloatVector((float*)&mouseTexCoords);
				update = true;
			}

			// Use bilinear interpolation to get the view-ray at this position
			D3DXVECTOR3 ray = m_QuadVerts[4].normal*(1.0f-mouseTexCoords.x)*(1.0f-mouseTexCoords.y) +
				m_QuadVerts[0].normal*mouseTexCoords.x*(1.0f-mouseTexCoords.y) +
				m_QuadVerts[2].normal*(1.0f-mouseTexCoords.x)*mouseTexCoords.y +
				m_QuadVerts[1].normal*mouseTexCoords.x*mouseTexCoords.y;
			D3DXVec3Normalize(&ray, &ray);
			Device::Effect->TerrainWidgetRayVariable->SetFloatVector((float*)&ray);

			// Render the widget
			Device::Effect->Pass[PASS_EDITOR_TERRAIN_WIDGET]->Apply(0);
			g_pd3dDevice->Draw(6, 0);	

			
			static bool isSculpting = false;


			// Get the mouse-terrain pick point
			D3DXVECTOR3 rayPos, rayDir;
			static D3DXVECTOR3 pickPoint;
			Math::GetPickRay(m_MousePos.x, m_MousePos.y, m_Camera, rayPos, rayDir);
			float dist = 0;
			if( !update || (update && m_pTerrain->RayIntersection(rayPos, rayDir, dist)) )
			{
				if(update)
				{
					pickPoint = rayPos + dist*rayDir;
					pickPoint.y = 0;
				}

				// Sculpting
				if(m_DebugTerrainSculpt)
				{
					m_pTerrain->CacheSculptRegion(pickPoint, m_DebugTerrainSculptRadius, m_DebugTerrainSculptMode==SCULPT_PAINT);
					DebugTerrainSculpt(pickPoint);
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------
	// Perform terrain sculpting
	//--------------------------------------------------------------------------------------
	void Renderer::DebugTerrainSculpt(D3DXVECTOR3 pickPoint)
	{
		// Do the sculpting/painting
		m_pTerrain->Sculpt(pickPoint, m_DebugTerrainSculptRadius, m_DebugTerrainSculptHardness, 
			m_DebugTerrainSculptStrength, m_DebugTerrainSculptDelta, m_DebugTerrainSculptDetail, m_DebugTerrainSculptMode);
	}


	//--------------------------------------------------------------------------------------
	// Renders a material preview texture and saves it to a file
	//--------------------------------------------------------------------------------------
	String Renderer::DebugRenderMaterialPreview(Material& mat)
	{
		Device::CacheRenderTarget();
		g_pd3dDevice->RSSetViewports(1, &m_DebugMaterialPreviewMSAA.GetViewport());
		m_DebugMaterialPreviewMSAA.Clear();
		m_DebugMaterialPreviewMSAA.ClearDSV();
		m_DebugMaterialPreviewMSAA.BindRenderTarget();

		g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Device::SetInputLayout( Vertex::pInputLayout );
		
		// Set the material
		Device::SetMaterial(mat);

		// Setup the matrices
		D3DXMATRIX mWorld, mView, mProj, mViewProj;
		D3DXMatrixIdentity(&mWorld);
		D3DXMatrixLookAtLH(&mView, &D3DXVECTOR3(-1,0.5,-1.75), &D3DXVECTOR3(0,0,0), &D3DXVECTOR3(0,1,0));
		D3DXMatrixPerspectiveFovLH( &mProj, (float)(D3DX_PI/3.0), 1.0f, 0.01f, 100.0f );
		D3DXMatrixMultiply(&mViewProj, &mView, &mProj);
		Device::Effect->WorldMatrixVariable->SetMatrix((float*)&mWorld);
		Device::Effect->ViewProjectionVariable->SetMatrix((float*)&mViewProj);

		// Setup a light
		// Render the sphere mesh
		Device::Effect->Pass[PASS_FORWARD_AMBIENT_MSAA]->Apply(0);
		Device::DrawSubmesh(*m_DebugMaterialPreivewMesh.GetSubMesh(0));
		for(int i=0; i<2; i++)
		{
			Device::SetLight(m_FocusLights[i]);
			Device::Effect->Pass[PASS_FORWARD_SHADE_MSAA]->Apply(0);
			Device::DrawSubmesh(*m_DebugMaterialPreivewMesh.GetSubMesh(0));
		}

		Device::RestoreCachedTargets();
		D3DXMatrixMultiply(&mViewProj, &m_Camera.GetViewMatrix(), &m_Camera.GetProjMatrix());
		Device::Effect->ViewProjectionVariable->SetMatrix((float*)&mViewProj);

		// Resolve the msaa target
		m_DebugMaterialPreview.Clear();
		m_DebugMaterialPreviewMSAA.Resolve(m_DebugMaterialPreview);

		// Save the texture to a file
		String file = g_szDirectory + "Textures\\Editor\\Material Previews\\preview" + (int)mat.ID + ".jpg";
		D3DX10SaveTextureToFileA(m_DebugMaterialPreview.GetTex()[0], D3DX10_IFF_JPG, file.c_str());
		return file;
	}

	//--------------------------------------------------------------------------------------
	// Renders a mesh preview texture and saves it to a file
	//--------------------------------------------------------------------------------------
	String Renderer::DebugRenderMeshPreview(MeshObject* pMesh)
	{
		Device::CacheRenderTarget();
		g_pd3dDevice->RSSetViewports(1, &m_DebugMeshPreviewMSAA.GetViewport());
		m_DebugMeshPreviewMSAA.SetClearColor(0.7f, 0.7f, 0.7f, 0.7f);
		m_DebugMeshPreviewMSAA.Clear();
		m_DebugMeshPreviewMSAA.ClearDSV();
		m_DebugMeshPreviewMSAA.BindRenderTarget();

		g_pd3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		Device::SetInputLayout( Vertex::pInputLayout );

		// Setup the matrices
		D3DXMATRIX mWorld, mView, mProj, mViewProj;
		D3DXMatrixIdentity(&mWorld);
		D3DXMatrixLookAtLH(&mView, &D3DXVECTOR3(0,0,-1.5f*pMesh->GetRadius()), &D3DXVECTOR3(0,0,0), &D3DXVECTOR3(0,1,0));
		D3DXMatrixPerspectiveFovLH( &mProj, (float)(D3DX_PI/3.0), 1.0f, 0.01f, 100.0f );
		D3DXMatrixMultiply(&mViewProj, &mView, &mProj);
		Device::Effect->WorldMatrixVariable->SetMatrix((float*)&mWorld);
		Device::Effect->ViewProjectionVariable->SetMatrix((float*)&mViewProj);

		// Build the correct world matrix, using the camera view angles to rotate the mesh
		D3DXVECTOR3 cachedRot = pMesh->GetRot();
		D3DXVECTOR3 cachedPos = pMesh->GetPos();
		pMesh->SetPos(0, 0, 0);
		pMesh->SetRot( 0, 0, 0);

		// First do an ambient pass to fill the zbuffer
		// Render it to the preview texture
		for(int i=0; i<pMesh->GetNumSubMesh(); i++)
		{
			SubMesh& mesh = *pMesh->GetSubMesh(i);

			// World matrix
			Device::Effect->WorldMatrixVariable->SetMatrix( (float*)mesh.pWorldMatrix );

			// Set the material
			Device::SetMaterial(*mesh.pMaterial);

			Device::Effect->Pass[PASS_FORWARD_AMBIENT_MSAA]->Apply(0);
			Device::DrawSubmesh(mesh);
		}
		
		// Render it to the preview texture
		for(int i=0; i<pMesh->GetNumSubMesh(); i++)
		{
			SubMesh& mesh = *pMesh->GetSubMesh(i);

			// World matrix
			Device::Effect->WorldMatrixVariable->SetMatrix( (float*)mesh.pWorldMatrix );

			// Set the material
			Device::SetMaterial(*mesh.pMaterial);

			// Render it for each light
			for(int e=0; e<4; e++)
			{
				Device::SetLight(m_FocusLights[e]);
				Device::Effect->Pass[PASS_FORWARD_SHADE_MSAA]->Apply(0);
				Device::DrawSubmesh(mesh);
			}
		}
		
		// Restore
		pMesh->SetRot(cachedRot);
		pMesh->SetPos(cachedPos);
		Device::RestoreCachedTargets();
		D3DXMatrixMultiply(&mViewProj, &m_Camera.GetViewMatrix(), &m_Camera.GetProjMatrix());
		Device::Effect->ViewProjectionVariable->SetMatrix((float*)&mViewProj);

		// Resolve the msaa target
		m_DebugMeshPreview.Clear();
		m_DebugMeshPreviewMSAA.Resolve(m_DebugMeshPreview);

		// Save the texture to a file
		String file = g_szDirectory + "Textures\\Editor\\Mesh Previews\\preview.jpg";
		D3DX10SaveTextureToFileA(m_DebugMeshPreview.GetTex()[0], D3DX10_IFF_JPG, file.c_str());
		return file;
	}

}

#endif