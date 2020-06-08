//--------------------------------------------------------------------------------------
// File: Light.h
//
// 3D Light
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "Texture.h"
#include "Frustum.h"
#include "MeshObject.h"
#include "RenderSurface.h"
#include "Array.cpp"
namespace Core
{
	
	//--------------------------------------------------------------------------------------
	// A light object
	//
	// Lights can be either directional, spot, or point.  Directional lights have no
	// attenuation and don't rely on a position, while point and spot lights have a
	// range based falloff.  
	//--------------------------------------------------------------------------------------
	class Light
	{
		friend class MeshObject;
	public:

		// Different types of lights
		enum LIGHT_TYPE
		{
			LIGHT_DIRECTIONAL=0,
			LIGHT_POINT=1,
			LIGHT_SPOT=2,
		};
		
		// Default contructor
		Light();
		
		// Free all memory associated with this object
		void Release();
		
		// Modify the position
		inline void SetPos( D3DXVECTOR3& pos ){ Position = pos; UpdateLight(); }

		// Modify the position
		inline void SetPos( float x, float y, float z ){ Position.x=x; Position.y=y; Position.z=z; UpdateLight(); }
		
		// Modify the direction
		inline void SetDir( D3DXVECTOR3& pos ){ D3DXVec3Normalize(&Dir, &pos); UpdateLight(); }

		// Modify the direction
		inline void SetDir( float x, float y, float z ){ D3DXVec3Normalize(&Dir, &D3DXVECTOR3(x,y,z)); UpdateLight(); }

		// Modify the range
		inline void SetRange( float pos ){ Range = pos; UpdateLight(); }
		
		// Add the value to the position
		inline void AddToPos( D3DXVECTOR3& pos ){ Position += pos; UpdateLight(); }

		// Add the value to the direction
		inline void AddToDir( D3DXVECTOR3& pos ){ D3DXVec3Normalize(&Dir, &(pos+Dir)); UpdateLight(); }
		
		// Add the value to the range
		inline void AddToRange( float pos ){ Range += pos; UpdateLight(); }
		
		// Gets the position
		inline const D3DXVECTOR3& GetPos() const { return Position; }
		
		// Gets the direction
		inline const D3DXVECTOR3& GetDir() const { return Dir; }
		
		// Gets the range
		inline const float GetRange() const { return Range; }

		// Forces the scene to update the light's affected mesh list
		void UpdateLight();

		// If the mesh is contained in the light's volume, it is added to the mesh list
		bool CheckMesh( MeshObject* pMesh );
		
		// Remove the mesh from this light's mesh list
		void RemoveMesh( MeshObject* pMesh );

		// Update the position to match that of the mesh it is attached to
		inline void UpdatePosition()
		{
			if(pAttachedMesh && pAttachedMesh->GetPos()!=Position)
			{
				Position = pAttachedMesh->GetPos();
				UpdateLight();
			}
		}

		// Attach to a mesh object
		inline void AttachToMesh(MeshObject* pMesh){ pAttachedMesh = pMesh; }

		// Get the mesh that this light is attached to
		inline MeshObject* GetAttachedMesh(){ return pAttachedMesh; }


		// Color
		D3DXVECTOR4		Color;				

		// Inner radius for a spotlight
		float			InnerRadius;		

		// Spotlight outer radius
		float			OuterRadius;		

		// Projective texture
		Texture*		pProj;				

		// Light type
		LIGHT_TYPE		Type;				

		// Bounding volume
		D3DXVECTOR3		Bounds;				

		// Controls shadow softness
		int				ShadowQuality;		

		// True for volumetric light (currently unused)
		bool			isVolumetric;		

		// View/projection matrix for shadow mapping
		D3DXMATRIX		matrix;				

		// World matrix
		D3DXMATRIX		worldMatrix;		

		// View frustum, for spotlights only
		Frustum			frustum;			

		// Set to true to use shadowing with this light
		bool			IsShadowed;			

		// The name of this light, used to identify it in the scene
		String			name;

		// A list of submeshes within this light's volume
		Array<MeshObject*> MeshList;
		
		// True if the light is waiting to be processed by the scene
		bool			isUpdating;



	private:
		
		D3DXVECTOR3		Position;			// Position
		D3DXVECTOR3		Dir;				// Direction
		float			Range;				// Range
		MeshObject*		pAttachedMesh;		// A BaseMesh that the light is attached to
	};
		

}