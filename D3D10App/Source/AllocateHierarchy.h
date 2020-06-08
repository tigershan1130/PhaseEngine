//--------------------------------------------------------------------------------------
// File: AllocateHierarchy.h
//
// Implementation of the ID3DXAllocateHierarchy interface for loading skinned meshes
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

namespace Core
{

	// Copies a string taking memory allocation into account
	HRESULT AllocateName( LPCSTR Name, LPSTR *pNewName );


	class AllocateHierarchy : public ID3DXAllocateHierarchy
	{
	public:
		STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);
		STDMETHOD(CreateMeshContainer)(THIS_ 
			LPCSTR Name, 
			CONST D3DXMESHDATA *pMeshData,
			CONST D3DXMATERIAL *pMaterials, 
			CONST D3DXEFFECTINSTANCE *pEffectInstances, 
			DWORD NumMaterials, 
			CONST DWORD *pAdjacency, 
			LPD3DXSKININFO pSkinInfo, 
			LPD3DXMESHCONTAINER *ppNewMeshContainer);
		STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
		STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);
	};

}