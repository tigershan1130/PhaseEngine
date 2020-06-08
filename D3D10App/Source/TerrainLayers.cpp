//--------------------------------------------------------------------------------------
// File: TerrainLayers.cpp
//
// Implementation of terrain layering system
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Terrain.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Sets up the texture arrays
	//--------------------------------------------------------------------------------------
	void Terrain::BuildTextureLayers(int res, int mips)
	{
		m_LayerSize = res;
		m_LayerMips = mips;

		// Create the arrays
		D3D10_TEXTURE2D_DESC desc;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Width = res;
		desc.Height = res;
		desc.MipLevels = m_LayerMips;
		desc.Usage = D3D10_USAGE_DEFAULT;
		desc.BindFlags = D3D10_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.ArraySize = NUM_LAYERS;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count=1;
		desc.SampleDesc.Quality=0;
		g_pd3dDevice->CreateTexture2D( &desc, NULL, &m_TextureLayerArray);
		g_pd3dDevice->CreateTexture2D( &desc, NULL, &m_NormalLayerArray);

		// Create the shader resources
		D3D10_SHADER_RESOURCE_VIEW_DESC sd;
		sd.Format = desc.Format;
		sd.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DARRAY;
		sd.Texture2DArray.ArraySize = desc.ArraySize;
		sd.Texture2DArray.FirstArraySlice = 0;
		sd.Texture2DArray.MipLevels = mips;
		sd.Texture2DArray.MostDetailedMip = 0;
		g_pd3dDevice->CreateShaderResourceView(m_TextureLayerArray, &sd, &m_TextureLayers);
		g_pd3dDevice->CreateShaderResourceView(m_NormalLayerArray, &sd, &m_NormalLayers);
	}

	//--------------------------------------------------------------------------------------
	// Load a texture
	//--------------------------------------------------------------------------------------
	void Terrain::LoadTexture(char* file, int layer)
	{
		ID3D10Texture2D* pTex;
		D3DX10_IMAGE_LOAD_INFO info = D3DX10_IMAGE_LOAD_INFO();
		info.MipLevels = m_LayerMips;
		if(SUCCEEDED(D3DX10CreateTextureFromFileA(g_pd3dDevice, file, NULL, NULL, (ID3D10Resource**)&pTex, NULL)))
		{
			m_Layers[layer].texMap = file;
			m_HasTextureLayer[layer] = true;

			// Filter the mips
			
			// Copy this into the array slice
			D3D10_BOX region;
			
			// Copy each mip
			int size = 1;
			for(int i=0; i<m_LayerMips; i++, size*=2)
			{
				region.left=0;
				region.top=0;
				region.right = m_LayerSize / size;
				region.bottom = m_LayerSize / size;
				region.front=0;
				region.back=1;
				g_pd3dDevice->CopySubresourceRegion(m_TextureLayerArray, D3D10CalcSubresource(i, layer, m_LayerMips), 0, 0, 0,
					pTex, i, &region);
			}

			pTex->Release();
		}			
	}

	
	//--------------------------------------------------------------------------------------
	// Load a normalmap
	//--------------------------------------------------------------------------------------
	void Terrain::LoadNormalmap(char* file, int layer)
	{
		ID3D10Texture2D* pTex;
		if(SUCCEEDED(D3DX10CreateTextureFromFileA(g_pd3dDevice, file, NULL, NULL, (ID3D10Resource**)&pTex, NULL)))
		{
			m_Layers[layer].normalMap = file;
			m_HasNormalLayer[layer] = true;

			// Copy this into the array slice
			D3D10_BOX region;
			region.left=0;
			region.top=0;
			region.right=m_LayerSize;
			region.bottom=m_LayerSize;
			region.front=0;
			region.back=1;
			g_pd3dDevice->CopySubresourceRegion(m_NormalLayerArray, D3D10CalcSubresource(0, 1, m_LayerMips), 0, 0, 0,
				pTex, 0, &region);

			pTex->Release();
		}				
	}

	
	//--------------------------------------------------------------------------------------
	// Get a detail texture file name
	//--------------------------------------------------------------------------------------
	String Terrain::GetTextureName(int i)
	{
		return m_Layers[i].texMap;
	}

	// Get a normalmap file name
	String Terrain::GetNormalmapName(int i)
	{
		return m_Layers[i].normalMap;
	}

	
	//--------------------------------------------------------------------------------------
	// Clear a texture
	//--------------------------------------------------------------------------------------
	void Terrain::ClearTexture(int layer)
	{
		m_Layers[layer].texMap = "None";
		m_HasTextureLayer[layer] = false;
	}

	
	//--------------------------------------------------------------------------------------
	// Clear a normalmap
	//--------------------------------------------------------------------------------------
	void Terrain::ClearNormalmap(int layer)
	{
		m_Layers[layer].normalMap = "None";
		m_HasNormalLayer[layer] = false;
	}

	
	//--------------------------------------------------------------------------------------
	// Texture scale factor
	//--------------------------------------------------------------------------------------
	void Terrain::SetTextureScale(float scale, int layer)
	{
		m_LayerScales[layer] = scale;
	}

	
	//--------------------------------------------------------------------------------------
	// Get the detail texture scales
	//--------------------------------------------------------------------------------------
	float Terrain::GetTextureScale(int layer)
	{
		return m_LayerScales[layer];
	}

	
	//--------------------------------------------------------------------------------------
	// Binds the textures and normal maps
	//--------------------------------------------------------------------------------------
	void Terrain::BindTextures(Effect& effect)
	{	
		effect.TerrainTextureVariable->SetResource(m_TextureLayers);
		effect.TerrainNormalmapVariable->SetResource(m_NormalLayers);
		effect.TerrainTextureFlagVariable->SetBoolArray(m_HasTextureLayer, 0, NUM_LAYERS);
		effect.TerrainNormalFlagVariable->SetBoolArray(m_HasNormalLayer, 0, NUM_LAYERS);
		effect.TerrainTextureScaleVariable->SetFloatArray(m_LayerScales, 0, NUM_LAYERS);
	}
}