//--------------------------------------------------------------------------------------
// File: BaseMesh.cpp
//
// Sub mesh class
// A Submesh is defined as a set of geometry with a single material.
// Larger meshes are divided into a group of submeshes, one for each material.
// The submeshes contain pointers to the geometry buffers and are defined by
// start and ending indices. A seperate buffer containing only the PVertex positions 
// to reduce the bandwidth during the Z-Fill pass is used for each mesh.
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "SubMesh.h"

namespace Core
{
	// Camera pointer for distance compare
	Camera* SubMesh::CamPointer = NULL;
	
	//--------------------------------------------------------------------------------------
	// Constructors/Destructors
	//--------------------------------------------------------------------------------------
	SubMesh::SubMesh()
	{
		pVertexBuffer = NULL;
		pIndexBuffer = NULL;
		pPosVertexBuffer = NULL;
		name = "SubMesh";
		startIndex = numIndices = 0;
		pWorldMatrix = NULL;
		pWorldPosition = NULL;
		pCubeMap = NULL;
		isSkinned = false;
		pAnimMatrices = NULL;
	}
}