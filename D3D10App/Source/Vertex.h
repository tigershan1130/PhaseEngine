//--------------------------------------------------------------------------------------
// File: Vertex.h
//
// 3D Vertex formats
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Phase Engine standard vertex format
	//--------------------------------------------------------------------------------------
	struct Vertex
	{
		D3DXVECTOR3 pos;	// Position
		float tu,tv;		// Texture coords
		D3DXVECTOR3 normal;	// Normal
		
		static ID3D10InputLayout* pInputLayout;
		static const D3D10_INPUT_ELEMENT_DESC Desc[3];
		static const UINT size;
	};

	//--------------------------------------------------------------------------------------
	// Phase Engine vertex format for skinned meshes
	//--------------------------------------------------------------------------------------
	struct SkinnedVertex
	{
		D3DXVECTOR3 pos;	// Position
		float tu,tv;		// Texture coords
		D3DXVECTOR3 normal;	// Normal
		D3DXVECTOR4 weights;// Blend weights
		UCHAR indices[4];	// Bone indices

		static ID3D10InputLayout* pInputLayout;
		static const D3D10_INPUT_ELEMENT_DESC Desc[5];
		static const UINT size;
	};

	//--------------------------------------------------------------------------------------
	// Phase Engine vertex with only position
	//--------------------------------------------------------------------------------------
	struct PosVertex
	{
		D3DXVECTOR3 pos;	// Position
		
		float pad;			// Cache pad

		static ID3D10InputLayout* pInputLayout;
		static const D3D10_INPUT_ELEMENT_DESC Desc[1];
		static const UINT size;
	};
}
