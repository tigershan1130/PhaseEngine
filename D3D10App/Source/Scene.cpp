//--------------------------------------------------------------------------------------
// File: Scene.cpp
//
// Implementation of the Renderer scene manager
//
// Includes light/mesh loading and culling systems
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "Renderer.h"
#include "Stack.cpp"

#include "MessageHandler.h"

#include <iostream>
#include <fstream>
using std::ifstream;
using std::ofstream;
using std::endl;

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Creates a new light
	//--------------------------------------------------------------------------------------
	Light* Renderer::CreateLight(bool bShadowCasting)
	{
		// Make a new light
		Light* pLight;
		if(m_LightPool.IsEmpty())
			pLight = new Light;
		else
			pLight = m_LightPool.Pop();

		// Setup the parameters		
		pLight->name = String("Light") + m_Lights.Size();
		pLight->IsShadowed = bShadowCasting;
		return pLight;
	}

	
	//--------------------------------------------------------------------------------------
	// Adds a light to the scene
	//--------------------------------------------------------------------------------------
	void Renderer::AddLight(Light* pLight)
	{
		// Add the light ptr to the generic light list
		m_Lights.Add( pLight );
		pLight->UpdateLight();
	}


	//--------------------------------------------------------------------------------------
	// Removes a light from the scene
	//--------------------------------------------------------------------------------------
	void Renderer::RemoveLight( Light* pLight )
	{
		// Check if this light exists within the current scene
		// and if it does, remove it
		for(int i=0; i<m_Lights.Size(); i++)
		{
			if(m_Lights[i] == pLight)
			{
				m_Lights.Remove( pLight );
				return;
			}
		}
	}

	//--------------------------------------------------------------------------------------
	// Deletes a light permanently
	//--------------------------------------------------------------------------------------
	void Renderer::DeleteLight( Light* pLight )
	{
		// First remove the light and then delete it
		RemoveLight(pLight);
		pLight->Release();
		m_LightPool.Push( pLight );
	}


	//--------------------------------------------------------------------------------------
	// Adds a mesh object to the scene.  Animated meshes are added to the front of the
	// list, and static are added to the end.  This keeps things somewhat sorted
	// without having to maintain two seperate lists
	//--------------------------------------------------------------------------------------
	MeshObject* Renderer::LoadMesh( const char* file )
	{
		// Load the mesh
		MeshObject* pMesh = new MeshObject();
		if(!pMesh->Load(file))
		{
			pMesh->Release();
			delete pMesh;
			pMesh = NULL;
			return NULL;
		}

		// Rebuild the material texture
		EncodeMaterialTexture();

		// Make sure there isnt another mesh with the same name
		int num=2;
		String baseName = pMesh->GetName();
		String realName = baseName;
		bool flag = true;
		while(flag)
		{
			flag=false;
			for(int i=0; i<GetNumMesh(); i++)
			{
				if(strcmp(realName.c_str(), GetMesh(i)->GetName().c_str())==0)
				{
					realName = baseName+num;
					num++;
					flag=true;
					break;
				}
			}
		}
		pMesh->SetName(const_cast<char*>(realName.c_str()));

		return pMesh;
	}


	//--------------------------------------------------------------------------------------
	// Adds a mesh object to the scene.  Animated meshes are added to the front of the
	// list, and static are added to the end.  This keeps things somewhat sorted
	// without having to maintain two seperate lists
	//--------------------------------------------------------------------------------------
	void Renderer::AddMesh( MeshObject* pMesh )
	{
		// Update the mesh
		pMesh->UpdateMesh();
		
		// Add it to the scene
		if(pMesh->IsSkinned())
			m_MeshObjects.Insert( pMesh, 0 );
		else
			m_MeshObjects.Add( pMesh ); 

		// Sort into the render list
		bool add;
		for(int i=0; i<pMesh->GetNumSubMesh(); i++)
		{
			add=true;
			for(int j=0; j<m_RenderList.Size(); j++)
				if(m_RenderList[j]->pMaterial == pMesh->GetSubMesh(i)->pMaterial)
				{
					m_RenderList.Insert(pMesh->GetSubMesh(i), j);
					add=false;
					break;
				}
			if(add)
				m_RenderList.Add(pMesh->GetSubMesh(i));
		}
	}


	//--------------------------------------------------------------------------------------
	// Removes a mesh from the scene
	//--------------------------------------------------------------------------------------
	void Renderer::RemoveMesh( MeshObject* pMesh )
	{
		// Get rid of any light attachments
		for(int i=0; i<m_Lights.Size(); i++)
			if(m_Lights[i]->GetAttachedMesh() == pMesh)
			{
				m_Lights[i]->AttachToMesh(NULL);
				RemoveLight(m_Lights[i]);
			}

			// Remove from lights
			for(int e=0; e<m_Lights.Size(); e++)
				m_Lights[e]->RemoveMesh( pMesh );

			// Delete any environment probes associated with it
			for(int i=0; i<m_Probes.Size(); i++)
				if(m_Probes[i]->GetMesh() == pMesh)
				{
					RemoveProbe(m_Probes[i]);
					break;
				}

			// Remove it from the mesh list
			m_MeshObjects.Remove(pMesh);

			// Remove from the render list
			for(int i=0; i<pMesh->GetNumSubMesh(); i++)
				m_RenderList.Remove(pMesh->GetSubMesh(i));
	}

	//--------------------------------------------------------------------------------------
	// Deletes a mesh permanently
	//--------------------------------------------------------------------------------------
	void Renderer::DeleteMesh( MeshObject* pMesh )
	{
		// Remove then delete
		RemoveMesh(pMesh);
		pMesh->Release();
		delete pMesh;
		pMesh = NULL;
 	}


	////////////////////////////////////////////////////////////////////////////////////////
	// Create a new probe
	////////////////////////////////////////////////////////////////////////////////////////
	EnvironmentProbe* Renderer::AddProbe(EnvironmentProbe::PROBE_TYPE type, bool dynamic)
	{
		EnvironmentProbe* probe = new EnvironmentProbe();
		if(FAILED(probe->CreateCubemap(ProbeSettings::resolution, ProbeSettings::format, ProbeSettings::miplevels, dynamic)))
			return NULL;
		probe->GetSurface().AttachEffectSRVVariable(Device::Effect->ProbeCubeVariable);
		probe->GetSurface().AttachDepthStencil(m_ProbeDepth.GetDSV());
		m_Probes.Add(probe);
		return probe;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Attach a mesh to a probe
	////////////////////////////////////////////////////////////////////////////////////////
	void Renderer::AttachMeshToProbe(EnvironmentProbe* probe, MeshObject* mesh)
	{
		// Now attach to this probe
		probe->AttachMesh(mesh);
		mesh->EnableCubeMapping(true);
		for(int i=0; i<mesh->GetNumSubMesh(); i++)
			mesh->GetSubMesh(i)->pCubeMap = &probe->GetSurface();

		// Update it
		UpdateProbe(*probe);
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Remove a probe
	////////////////////////////////////////////////////////////////////////////////////////
	void Renderer::RemoveProbe(EnvironmentProbe* probe)
	{
		if(!probe)
			return;

		// Remove the probe
		m_Probes.Remove(probe);
		probe->GetMesh()->EnableCubeMapping(false);
		for(int i=0; i<probe->GetMesh()->GetNumSubMesh(); i++)
			probe->GetMesh()->GetSubMesh(i)->pCubeMap = NULL;
		probe->Release();
		delete probe;
	}




	//--------------------------------------------------------------------------------------
	//Water
	// Create a new water grid:O
	//--------------------------------------------------------------------------------------
	
	Water* Renderer::CreateWater(int width, int height, int spacing, int waterheight)
	{
		Water* pWater = new Water();

		if(FAILED(pWater->initWater(Device::Effect ,width, height, spacing, waterheight)))
		{
			delete pWater;
			MessageBox(NULL, L"Failed to create water!", L"Core Engine Error", MB_OK);
			return NULL;
		}
		

		return pWater;

	}

	//--------------------------------------------------------------------------------------
	// Create a new emitter
	//--------------------------------------------------------------------------------------
	ParticleEmitter* Renderer::CreateParticleEmitter(char* texFile, int maxParticles, int freq, float lifetime)
	{
		ParticleEmitter* pEmitter = new ParticleEmitter();
		pEmitter->Create(texFile, maxParticles, freq, lifetime, m_Effect, m_Camera);
		m_Emitters.Add(pEmitter);
		return pEmitter;
	}


	//--------------------------------------------------------------------------------------
	// Delete an emitter
	//--------------------------------------------------------------------------------------
	void Renderer::DeleteParticleEmitter(ParticleEmitter* emitter)
	{
		if(emitter && m_Emitters.Remove(emitter))
		{
			emitter->Release();
			delete emitter;
		}
	}




	//--------------------------------------------------------------------------------------
	// Process engine messages
	//--------------------------------------------------------------------------------------
	void Renderer::ProcessMessages()
	{
		// Process the messages
		Message msg;
		int action = 0;
		while(1)
		{
			// Get the next message
			msg = MessageHandler::GetNextMessage();
			if(msg.id==MSG_NONE)
				break;
			Device::FrameStats.ProcessedMessages++;

			// Process it
			switch(msg.id)
			{
			
				// Make sure this mesh are current with the lights	
			case MSG_MESH_UPDATE:
				{
					MeshObject* pMesh = (MeshObject*)msg.param;
					for(int e=0; e<m_Lights.Size(); e++)
						m_Lights[e]->CheckMesh( pMesh );
					pMesh->isUpdating=false;
					break;
				}

			// Make sure these lights are current with the scene
			case MSG_LIGHT_UPDATE:
				{
					Light* pLight = (Light*)msg.param;
					for(int e=0; e<m_MeshObjects.Size(); e++)
						pLight->CheckMesh( m_MeshObjects[e] );
					pLight->isUpdating=false;
					break;
				}

			// Keep the material texture up to date
			case MSG_MATERIAL_UPDATE:
				{
					EncodeMaterialTexture();
					break;
				}
			}
		}
	}


	//--------------------------------------------------------------------------------------
	// Add a terrain
	//--------------------------------------------------------------------------------------
	Terrain* Renderer::ImportHeightmap(char* file)
	{ 
		Terrain* pTerrain = new Terrain();
		if(FAILED(pTerrain->CreateFromFile(file, &m_Camera)))
		{
			delete pTerrain;
			MessageBox(NULL, L"Failed to load the terrain!", L"Core Engine Error", MB_OK);
			return NULL;
		}
		if(m_pTerrain)
		{
			m_pTerrain->Release();
			delete m_pTerrain;
		}
		return m_pTerrain=pTerrain;
	}


	//--------------------------------------------------------------------------------------
	// Save a terrain heightmap to a png
	//--------------------------------------------------------------------------------------
	void Renderer::ExportHeightmap(char* file)
	{ 
		if(m_pTerrain)
			m_pTerrain->ExportHeightmap(file);
	}


	//--------------------------------------------------------------------------------------
	// Create a blank terrain
	//--------------------------------------------------------------------------------------
	Terrain* Renderer::CreateHeightmap(UINT size)
	{
		Terrain* pTerrain = new Terrain();
		if(FAILED(pTerrain->Create(size, &m_Camera)))
		{
			delete pTerrain;
			MessageBox(NULL, L"Failed to create the terrain!", L"Core Engine Error", MB_OK);
			return NULL;
		}
		if(m_pTerrain)
		{
			m_pTerrain->Release();
			delete m_pTerrain;
		}
		return m_pTerrain=pTerrain;
	}



	//--------------------------------------------------------------------------------------
	// Remove all unused materials
	//--------------------------------------------------------------------------------------
	void Renderer::RemoveUnusedMaterials()
	{
		Stack<Material*> materialsToRemove;
		for(int i=0; i<g_Materials.GetList().Size(); i++)
		{
			bool used = false;
			for(int j=0; j<m_MeshObjects.Size(); j++)
			{
				for(int k=0; k<m_MeshObjects[j]->GetNumSubMesh(); k++)
				{
					if(m_MeshObjects[j]->GetSubMesh(k)->pMaterial == g_Materials.GetList()[i])
					{
						used = true;
						break;
					}
				}
				if(used)
					break;
			}
			if(!used)
				materialsToRemove.Push(g_Materials.GetList()[i]);
		}
		while(!materialsToRemove.IsEmpty())
			g_Materials.Remove(materialsToRemove.Pop());
		materialsToRemove.Release();
		
		EncodeMaterialTexture();
	}


	
	//--------------------------------------------------------------------------------------
	// Optimizes the scene structure
	//--------------------------------------------------------------------------------------
	void Renderer::Optimize()
	{
		// Now sort each submesh by material
		for(int i=0; i<m_RenderList.Size(); i++)
			for(int j=i; j<m_RenderList.Size(); j++)
			{
				if(m_RenderList[j]->pMaterial->ID < m_RenderList[i]->pMaterial->ID)
					Swap(m_RenderList[j], m_RenderList[i]);
			}
	}

	
}