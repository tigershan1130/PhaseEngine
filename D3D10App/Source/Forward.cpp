//--------------------------------------------------------------------------------------
// File: Forward.cpp
//
// Traditional forward rendering
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
	// Render a mesh
	//--------------------------------------------------------------------------------------
	void Renderer::RenderMesh(MeshObject& mesh, ID3D10EffectPass* pass)
	{
		// Make sure it's visible
		if(!Device::IsMeshVisible(mesh, m_Camera))
			return;

		// Render the submesh
		Device::Effect->WorldMatrixVariable->SetMatrix( (float*)&mesh.GetWorldMatrix() );
		for(int j=0; j<mesh.GetNumSubMesh(); j++)
		{
			SubMesh& submesh = *mesh.GetSubMesh(j);

			// Set the material
			Device::SetMaterial(*submesh.pMaterial);

			// Setup the effect pass
			pass->Apply(0);

			// Render the mesh
			Device::DrawSubmesh(submesh);
			Device::FrameStats.PolysDrawn += submesh.numIndices/3;
		}
	}

	//--------------------------------------------------------------------------------------
	// Standard forward render of the scene
	//--------------------------------------------------------------------------------------
	void Renderer::RenderScene()
	{
		// ZFill Ambient pass
		// Render the meshes
		for(int i=0; i<m_MeshObjects.Size(); i++)
			RenderMesh(*m_MeshObjects[i], Device::Effect->Pass[PASS_FORWARD_AMBIENT]);

		
		m_VisibleLights.Clear();
		for(int i=0; i<m_Lights.Size(); i++)
		{
			// Make a reference to avoid many pointer derefs on the function calls
			Light& light = *m_Lights[i];

			// Update the light's position
			light.UpdatePosition();

			// Direction lights always visible
			if( light.Type==Light::LIGHT_DIRECTIONAL || IsLightVisible(light))
			{
				// Add to visible lights list
				m_VisibleLights.Add(m_Lights[i]);

				// Set the light effect vars
				Device::SetLight(light);

				// Render the shadow map
				if(light.IsShadowed)
					RenderShadowMap(light);

				// Render the meshes
				for(int i=0; i<m_MeshObjects.Size(); i++)
					RenderMesh(*m_MeshObjects[2], Device::Effect->Pass[PASS_FORWARD_SHADE]);
			}
		}

	}
}