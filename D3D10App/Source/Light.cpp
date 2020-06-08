//--------------------------------------------------------------------------------------
// File: Light.cpp
//
// 3D Light
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Light.h"
#include "MessageHandler.h"
namespace Core
{
	//--------------------------------------------------------------------------------------
	// Contructor
	//--------------------------------------------------------------------------------------
	Light::Light()
	{
		Type = LIGHT_POINT;
		Color.x=Color.y=Color.z=0.5f;
		Dir = D3DXVECTOR3(0.0f,-0.5f,0.3f);
		Position = D3DXVECTOR3(0,5,0);
		Range = 25.0f;
		InnerRadius = 0.7f;
		OuterRadius = 0.5f;
		pProj = NULL;
		ShadowQuality = 6;
		IsShadowed = false;
		isVolumetric = false;
		pAttachedMesh = NULL;
		isUpdating = false;
	}


	//--------------------------------------------------------------------------------------
	// Free the texture
	//--------------------------------------------------------------------------------------
	void Light::Release()
	{
		MeshList.Release();
		g_Textures.Deref(pProj); 
		pProj = NULL; 
		Light();
	}

	///////////////////////////////////////////////////////////////////////////////////////
	// Send a light to the update list
	//--------------------------------------------------------------------------------------
	void Light::UpdateLight()
	{
		if(Type == Light::LIGHT_DIRECTIONAL)
			return;
		if(isUpdating == true)
			return;
		isUpdating=true;
		MessageHandler::SendMessage(MSG_LIGHT_UPDATE, this);
	}

	//--------------------------------------------------------------------------------------
	// Add a submesh to the visible mesh list
	//--------------------------------------------------------------------------------------
	bool Light::CheckMesh( MeshObject* pMesh )
	{
		// Make sure the mesh is within the light's volume
		D3DXVECTOR3 r = Position - pMesh->GetPos();
		if( D3DXVec3Length(&r) > (Range+pMesh->GetRadius()) )
		{
			RemoveMesh(pMesh);
			return false;
		}
		// Process the mesh
		for(int i=0; i<MeshList.Size(); i++)
		{
			if(MeshList[i] == pMesh)
				return true;
		}
		MeshList.Add(pMesh);
		return true;

	}


	//--------------------------------------------------------------------------------------
	// Remove a mesh
	//--------------------------------------------------------------------------------------
	void Light::RemoveMesh( MeshObject* pMesh )
	{
		MeshList.Remove(pMesh);
	}
}