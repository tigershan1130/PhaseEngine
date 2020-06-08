//--------------------------------------------------------------------------------------
// File: Vertex.cpp
//
// 3D Vertex formats
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Vertex.h"

namespace Core
{

	// Statics
	ID3D10InputLayout* Vertex::pInputLayout = NULL;
	ID3D10InputLayout* SkinnedVertex::pInputLayout = NULL;
	ID3D10InputLayout* PosVertex::pInputLayout = NULL;

	// Vertex format
	const D3D10_INPUT_ELEMENT_DESC Vertex::Desc[3] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },  
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};
	const D3D10_INPUT_ELEMENT_DESC SkinnedVertex::Desc[5] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },  
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D10_INPUT_PER_VERTEX_DATA, 0 },
		{ "BONES", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, 48, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	};
	const D3D10_INPUT_ELEMENT_DESC PosVertex::Desc[1] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },  
	};

	const UINT Vertex::size = sizeof(Vertex);
	const UINT SkinnedVertex::size = sizeof(SkinnedVertex);
	const UINT PosVertex::size = sizeof(PosVertex);

}
