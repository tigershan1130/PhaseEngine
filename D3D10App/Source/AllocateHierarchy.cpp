//--------------------------------------------------------------------------------------
// File: AllocateHierarchy.h
//
// Implementation of the ID3DXAllocateHierarchy for loading skinned meshes
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "AllocateHierarchy.h"
#include "Mesh.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Copies a string taking memory allocation into account
	//--------------------------------------------------------------------------------------
	HRESULT AllocateName( LPCSTR Name, LPSTR *pNewName )
	{
		UINT cbLength;

		if( Name != NULL )
		{
			cbLength = (UINT)strlen(Name) + 1;
			*pNewName = new CHAR[cbLength];
			if (*pNewName == NULL)
				return E_OUTOFMEMORY;
			memcpy( *pNewName, Name, cbLength*sizeof(CHAR) );
		}
		else
		{
			*pNewName = NULL;
		}

		return S_OK;
	}



	//--------------------------------------------------------------------------------------
	// Creates a frame
	//--------------------------------------------------------------------------------------
	HRESULT AllocateHierarchy::CreateFrame(LPCSTR Name, LPD3DXFRAME *ppNewFrame)
	{
		HRESULT hr = S_OK;
		D3DXFRAME_DERIVED *pFrame;

		*ppNewFrame = NULL;

		pFrame = new D3DXFRAME_DERIVED;
		if (pFrame == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}

		hr = AllocateName(Name, &pFrame->Name);
		if (FAILED(hr))
			goto e_Exit;

		// initialize other data members of the frame
		D3DXMatrixIdentity(&pFrame->TransformationMatrix);
		D3DXMatrixIdentity(&pFrame->CombinedTransformationMatrix);

		pFrame->pMeshContainer = NULL;
		pFrame->pFrameSibling = NULL;
		pFrame->pFrameFirstChild = NULL;

		*ppNewFrame = pFrame;
		pFrame = NULL;

	e_Exit:
		delete pFrame;
		return hr;
	}



	//--------------------------------------------------------------------------------------
	// Creates the needed data for each mesh container
	//--------------------------------------------------------------------------------------
	HRESULT AllocateHierarchy::CreateMeshContainer( LPCSTR Name, 
													const D3DXMESHDATA* pMeshData, const D3DXMATERIAL* pMaterials, 
													const D3DXEFFECTINSTANCE* pEffectInstances, DWORD NumMaterials, 
													const DWORD *pAdjacency, ID3DXSkinInfo* pSkinInfo, 
													D3DXMESHCONTAINER** ppNewMeshContainer)
	{	
		HRESULT hr;
		D3DXMESHCONTAINER_DERIVED *pMeshContainer = NULL;
		UINT NumFaces;
		UINT iBone, cBones;
		LPDIRECT3DDEVICE9 pd3dDevice = NULL;

		LPD3DXMESH pMesh = NULL;

		*ppNewMeshContainer = NULL;

		// this sample does not handle patch meshes, so fail when one is found
		if (pMeshData->Type != D3DXMESHTYPE_MESH)
		{
			hr = E_FAIL;
			goto e_Exit;
		}

		// get the pMesh interface pointer out of the mesh data structure
		pMesh = pMeshData->pMesh;

		// this sample does not FVF compatible meshes, so fail when one is found
		if (pMesh->GetFVF() == 0)
		{
			hr = E_FAIL;
			goto e_Exit;
		}

		// allocate the overloaded structure to return as a D3DXMESHCONTAINER
		pMeshContainer = new D3DXMESHCONTAINER_DERIVED;
		if (pMeshContainer == NULL)
		{
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}
		memset(pMeshContainer, 0, sizeof(D3DXMESHCONTAINER_DERIVED));

		// make sure and copy the name.  All memory as input belongs to caller, interfaces can be addref'd though
		hr = AllocateName(Name, &pMeshContainer->Name);
		if (FAILED(hr))
			goto e_Exit;        

		pMesh->GetDevice(&pd3dDevice);
		NumFaces = pMesh->GetNumFaces();

		// if no normals are in the mesh, add them
		if (!(pMesh->GetFVF() & D3DFVF_NORMAL))
		{
			pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

			// clone the mesh to make room for the normals
			hr = pMesh->CloneMeshFVF( pMesh->GetOptions(), 
				pMesh->GetFVF() | D3DFVF_NORMAL, 
				pd3dDevice, &pMeshContainer->MeshData.pMesh );
			if (FAILED(hr))
				goto e_Exit;

			// get the new pMesh pointer back out of the mesh container to use
			// NOTE: we do not release pMesh because we do not have a reference to it yet
			pMesh = pMeshContainer->MeshData.pMesh;

			// now generate the normals for the pmesh
			D3DXComputeNormals( pMesh, NULL );
		}
		else  // if no normals, just add a reference to the mesh for the mesh container
		{
			pMeshContainer->MeshData.pMesh = pMesh;
			pMeshContainer->MeshData.Type = D3DXMESHTYPE_MESH;

			pMesh->AddRef();
		}

		// allocate memory to contain the material information.
		pMeshContainer->NumMaterials = MaxInt(1, NumMaterials);
		pMeshContainer->pMaterials = new D3DXMATERIAL[pMeshContainer->NumMaterials];
		pMeshContainer->pAdjacency = new DWORD[NumFaces*3];
		if ((pMeshContainer->pAdjacency == NULL) || (pMeshContainer->pMaterials == NULL))
		{
			hr = E_OUTOFMEMORY;
			goto e_Exit;
		}

		memcpy(pMeshContainer->pAdjacency, pAdjacency, sizeof(DWORD) * NumFaces*3);

		// if materials provided, copy them
		if (NumMaterials > 0)            
		{
			memcpy(pMeshContainer->pMaterials, pMaterials, sizeof(D3DXMATERIAL) * NumMaterials);
		}
		else // if no materials provided, use a default one
		{
			pMeshContainer->pMaterials[0].pTextureFilename = NULL;
			memset(&pMeshContainer->pMaterials[0].MatD3D, 0, sizeof(D3DMATERIAL9));
			pMeshContainer->pMaterials[0].MatD3D.Diffuse.r = 0.5f;
			pMeshContainer->pMaterials[0].MatD3D.Diffuse.g = 0.5f;
			pMeshContainer->pMaterials[0].MatD3D.Diffuse.b = 0.5f;
			pMeshContainer->pMaterials[0].MatD3D.Specular = pMeshContainer->pMaterials[0].MatD3D.Diffuse;
		}

		// if there is skinning information, save off the required data and then setup for HW skinning
		if (pSkinInfo != NULL)
		{
			// first save off the SkinInfo and original mesh data
			pMeshContainer->pSkinInfo = pSkinInfo;
			pSkinInfo->AddRef();

			pMeshContainer->pOrigMesh = pMesh;
			pMesh->AddRef();

			// Will need an array of offset matrices to move the vertices from the figure space to the bone's space
			cBones = pSkinInfo->GetNumBones();
			pMeshContainer->pBoneOffsetMatrices = new D3DXMATRIX[cBones];
			if (pMeshContainer->pBoneOffsetMatrices == NULL)
			{
				hr = E_OUTOFMEMORY;
				goto e_Exit;
			}

			// get each of the bone offset matrices so that we don't need to get them later
			for (iBone = 0; iBone < cBones; iBone++)
			{
				pMeshContainer->pBoneOffsetMatrices[iBone] = *(pMeshContainer->pSkinInfo->GetBoneOffsetMatrix(iBone));
			}

			// Take the general skinning information and transform it to a HW friendly version
			SAFE_RELEASE( pMeshContainer->MeshData.pMesh );
			SAFE_RELEASE( pMeshContainer->pBoneCombinationBuf );

			// Get palette size
			pMeshContainer->NumPaletteEntries = pMeshContainer->pSkinInfo->GetNumBones();

			// Convert to a mesh with bone indices and blend weights
			hr = pMeshContainer->pSkinInfo->ConvertToIndexedBlendedMesh
				(
				pMeshContainer->pOrigMesh,
				D3DXMESHOPT_VERTEXCACHE, 
				pMeshContainer->NumPaletteEntries, 
				pMeshContainer->pAdjacency, 
				NULL, NULL, NULL,             
				&pMeshContainer->NumInfl,
				&pMeshContainer->NumAttributeGroups, 
				&pMeshContainer->pBoneCombinationBuf, 
				&pMeshContainer->MeshData.pMesh);
			if (FAILED(hr))
				goto e_Exit;
		}

		*ppNewMeshContainer = pMeshContainer;
		pMeshContainer = NULL;

	e_Exit:
		SAFE_RELEASE(pd3dDevice);

		// call Destroy function to properly clean up the memory allocated 
		if (pMeshContainer != NULL)
		{
			DestroyMeshContainer(pMeshContainer);
		}

		return hr;
	}




	//--------------------------------------------------------------------------------------
	// Frees mem
	//--------------------------------------------------------------------------------------
	HRESULT AllocateHierarchy::DestroyFrame(D3DXFRAME* pFrameToFree) 
	{
		SAFE_DELETE_ARRAY( pFrameToFree->Name );
		SAFE_DELETE( pFrameToFree );
		return S_OK; 
	}


	//--------------------------------------------------------------------------------------
	// Frees mem
	//--------------------------------------------------------------------------------------
	HRESULT AllocateHierarchy::DestroyMeshContainer(D3DXMESHCONTAINER* pMeshContainerBase)
	{
		D3DXMESHCONTAINER_DERIVED *pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;
		SAFE_DELETE_ARRAY( pMeshContainer->Name );
		SAFE_DELETE_ARRAY( pMeshContainer->pAdjacency );
		SAFE_DELETE_ARRAY( pMeshContainer->pMaterials );
		SAFE_DELETE_ARRAY( pMeshContainer->pBoneOffsetMatrices );
		SAFE_DELETE_ARRAY( pMeshContainer->ppBoneMatrixPtrs );
		SAFE_RELEASE( pMeshContainer->pBoneCombinationBuf );
		SAFE_RELEASE( pMeshContainer->MeshData.pMesh );
		SAFE_RELEASE( pMeshContainer->pSkinInfo );
		SAFE_RELEASE( pMeshContainer->pOrigMesh );
		SAFE_DELETE( pMeshContainer );
		return S_OK;
	}

}