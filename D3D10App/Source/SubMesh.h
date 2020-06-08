//--------------------------------------------------------------------------------------
// File: Mesh.h
//
// Sub mesh class
// A Submesh is defined as a set of renderable geometry with a single material.
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "Vertex.h"
#include "Material.h"
#include "RenderSurface.h"
#include "Camera.h"

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Submesh within a larger mesh
	//--------------------------------------------------------------------------------------
	struct SubMesh
	{
		String			name;			// Name
		ID3D10Buffer*	pIndexBuffer;	// Parent index buffer
		ID3D10Buffer*	pVertexBuffer;	// Parent vertex buffer
		ID3D10Buffer*	pPosVertexBuffer;	// Parent vertex buffer
		INT startIndex, numIndices;		// Number of indices
		D3DXMATRIX*	    pWorldMatrix;	// Ptr to world matrix
		D3DXMATRIX*	    pWorldMatrixPrev;// Ptr to world matrix from the last frame
		D3DXVECTOR3	    localPosition;	// Local position
		D3DXVECTOR3*    pWorldPosition;	// Ptr to world position D3DXVECTOR3
		D3DXVECTOR3     boundSize;		// Bounding info
		float		    boundRadius;	// Bounding info
		Material*		pMaterial;		// Ptr to the material
		bool			isSkinned;		// True for animation
		RenderSurface*  pCubeMap;		// Pointer to the cubemap
		
		Array<D3DXMATRIX>* pAnimMatrices;
			
		// Constructor
		SubMesh();

		// Friend to overloaded operators
		friend int CompareSubMeshDistance(SubMesh*& lhs, SubMesh*& rhs);
		friend class Renderer;

		// Pointer to camera for distance compare
	private:
		static Camera* CamPointer;

	};

	// Comparison operator for a submesh based on distance from camera
	inline int CompareSubMeshDistance(SubMesh*& lhs, SubMesh*& rhs)
	{
		float d1 = D3DXVec3Length(&(SubMesh::CamPointer->GetPos()-*lhs->pWorldPosition));
		float d2 = D3DXVec3Length(&(SubMesh::CamPointer->GetPos()-*rhs->pWorldPosition));
		if(d1<d2)
			return 1;
		else if (d1>d2)
			return -1;
		return 0;
	}
}