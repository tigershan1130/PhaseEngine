//--------------------------------------------------------------------------------------
// File: Terrain.cpp
//
// 3D heightmap based terrain
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "Log.h"
#include "Heightmap.h"
#include "Terrain.h"
#include "QMath.h"

#include <iostream>
#include <fstream>
using std::ifstream;
using std::ofstream;
using std::endl;


namespace Core
{

	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	Terrain::Terrain()
	{
		m_bLoaded=false;
		m_bInThread=false;
		m_bQueueThread=false;
		m_FlagForUpdate = false;
		m_vPos *= 0;
		m_HeightScale = 128.0f;
		m_Scale = 1.0f;

		m_VertexBuffer = NULL;
		m_IndexBuffer = NULL;

		m_TrimXSide = NULL;
		m_TrimZSide = NULL;
		m_ClipmapOffsets = NULL;
		
		m_pRootNode = NULL;

		m_pCamera = NULL;

		// Setup the layers
		m_TextureLayers = NULL;
		m_NormalLayers = NULL;
		for(int i=0; i<NUM_LAYERS; i++)
		{
			m_HasTextureLayer[i] = false;
			m_HasNormalLayer[i] = false;
			m_Layers[i].texMap = "None";
			m_Layers[i].normalMap = "None";
		}

#ifdef PHASE_DEBUG
		m_UndoMaxMem = 50.0;
		m_UndoMemUsage = 0;
#endif


		m_HeightExtents.x = 9999;
		m_HeightExtents.y = -500;

		BuildTextureLayers(512, 4);
	}


	//--------------------------------------------------------------------------------------
	// Build the world matrix
	//--------------------------------------------------------------------------------------
	void Terrain::UpdateWorldMatrix()
	{
		D3DXMATRIX mTrans, mScale;
		m_vPos = D3DXVECTOR3(-(float)m_Size*0.5f,0,-(float)m_Size*0.5f) * m_Scale;
		D3DXMatrixTranslation(&mTrans, m_vPos.x, m_vPos.y, m_vPos.z);
		D3DXMatrixScaling(&mScale, m_Scale, m_HeightScale, m_Scale);
		m_WorldMatrix = mScale*mTrans;
	}

	
	//--------------------------------------------------------------------------------------
	// Saving
	//--------------------------------------------------------------------------------------
	void Terrain::ExportHeightmap(char* fileName)
	{
		// Save the heightmap	
		D3DX10SaveTextureToFileA(m_Heightmap.GetTex()[0], D3DX10_IFF_DDS, fileName);

		// Save the detail layer map
		String szFile = fileName;
		String actualFile = szFile.GetDirectory() + szFile.GetFile().Substring(0, szFile.GetFile().Length()-5);
		D3DX10SaveTextureToFileA(m_LayerMaps.GetBaseTexture(), D3DX10_IFF_DDS, actualFile + "_d.dds");

		// Save the blending map
		D3DX10SaveTextureToFileA(m_BlendMaps.GetBaseTexture(), D3DX10_IFF_DDS, actualFile + "_b.dds");

		// Export the file information
		ofstream file;
		
		file.open(actualFile + ".ptf");

		// Export the texture names
		for(int i=0; i<NUM_LAYERS; i++)
		if(m_HasTextureLayer[i])
			file << GetTextureName(i) << endl;
		else
			file << "NONE" << endl;

		// Export the normal map names
		for(int i=0; i<NUM_LAYERS; i++)
			if(m_HasNormalLayer[i])
				file << GetNormalmapName(i) << endl;
			else
				file << "NONE" << endl;

		// Export the scaling factors
		for(int i=0; i<NUM_LAYERS; i++)
			file << m_LayerScales[i] << endl;

		file.close();
	}


	

	//--------------------------------------------------------------------------------------
	// Loads from an image heightmap
	//--------------------------------------------------------------------------------------
	UINT Terrain::LoadHeightMap(ID3D10Texture2D* pHeightmap,  ID3D10Texture2D* pBlendmap)
	{
		// Create a staging buffer to copy the texture into
		ID3D10Texture2D* pStage;
		D3D10_TEXTURE2D_DESC td;
		pHeightmap->GetDesc(&td);
		td.BindFlags = 0;	
		td.ArraySize = 1;
		td.MiscFlags = 0;
		td.MipLevels = 1;
		td.CPUAccessFlags = D3D10_CPU_ACCESS_READ | D3D10_CPU_ACCESS_WRITE;
		td.Usage = D3D10_USAGE_STAGING;
		g_pd3dDevice->CreateTexture2D(&td, NULL, &pStage);

		// Copy the loaded texture into the staging texture
		D3D10_BOX sourceRegion;
		sourceRegion.left = 0;
		sourceRegion.right = td.Width;
		sourceRegion.top = 0;
		sourceRegion.bottom = td.Height;
		sourceRegion.front = 0;
		sourceRegion.back = 1;
		g_pd3dDevice->CopySubresourceRegion(pStage, 0, 0, 0, 0, pHeightmap, 0, &sourceRegion);	

		// Allocate space in the height array
		m_Height.Allocate(td.Width*td.Height);
		
		// Map the texture and copy the height data over
		D3D10_MAPPED_TEXTURE2D mappedTexture;
		if( SUCCEEDED(pStage->Map(0, D3D10_MAP_READ_WRITE, 0, &mappedTexture)) )
		{
			D3DXFLOAT16* pTexels = (D3DXFLOAT16*)mappedTexture.pData;
			for( UINT row = 0; row < td.Height; row++ )
			{
				UINT rowStart = row * mappedTexture.RowPitch/2;
				for( UINT col = 0; col < td.Width; col++)
				{
					m_Height[row*m_Size+col] = pTexels[rowStart + col] * m_HeightScale;

					// Track the min/max
					m_HeightExtents.x = Math::Min(m_HeightExtents.x, (float)m_Height[row*m_Size+col]);
					m_HeightExtents.y = Math::Max(m_HeightExtents.y, (float)m_Height[row*m_Size+col]);
				}
			}
			pStage->Unmap(0);
		}
		m_StagingHeightmap = pStage;
		pHeightmap->Release();
		
		// Create the blendmaps
		m_LayerMaps.Create(m_Size, m_ClipmapSize, CLIPMAP_LEVELS, DXGI_FORMAT_R8G8B8A8_UNORM);		
		//m_BlendMaps.Create(m_Size, m_ClipmapSize, CLIPMAP_LEVELS, DXGI_FORMAT_R8G8B8A8_UNORM);		

		// Copy from the loaded map
		if( pBlendmap )
		{
			g_pd3dDevice->CopySubresourceRegion(m_LayerMaps.GetBaseTexture(), 0, 0, 0, 0, pBlendmap, 0, &sourceRegion);
			pBlendmap->Release();
		}
		else
		{			
			// Map the texture and update the values
			if( SUCCEEDED(m_LayerMaps.GetBaseTexture()->Map(0, D3D10_MAP_READ_WRITE, 0, &mappedTexture)) )
			{
				UCHAR* pTexels = (UCHAR*)mappedTexture.pData;
				for( UINT index = 0; index<m_LayerMaps.GetSize()*m_LayerMaps.GetSize()*4; index+=4)
				{
					pTexels[index] = 0;
					pTexels[index+1] = 0;
					pTexels[index+2] = 255;
					pTexels[index+3] = 0;
				}
				m_LayerMaps.GetBaseTexture()->Unmap(0);
			}
		}
		
		// Generate the mip-maps
		D3DX10FilterTexture(m_LayerMaps.GetBaseTexture(), 0, D3DX10_FILTER_POINT);

		return td.Height;
	}



	//--------------------------------------------------------------------------------------
	// Create a terrain from a heightmap that is already loaded
	//--------------------------------------------------------------------------------------
	HRESULT Terrain::CreateFromTexture(ID3D10Texture2D* pHeightmap, ID3D10Texture2D* pBlendmap, Camera* pCam)
	{
		// Get the dimensions
		D3D10_TEXTURE2D_DESC hd;
		pHeightmap->GetDesc(&hd);
		m_Size = hd.Width;

		SetScale(1.0f);
		SetHeightScale(128.0f);

		// Setup clipmap
		SetupClipmap(255, CLIPMAP_LEVELS);
		
		// Get the height info
		LoadHeightMap(pHeightmap, pBlendmap);

		// Setup the quadtree
		m_pRootNode = new QuadtreeNode();
		m_pRootNode->pos = D3DXVECTOR3(0, m_HeightScale*0.5f, 0);
		m_pRootNode->size = D3DXVECTOR3((float)m_Size/2, m_HeightScale*0.5f, (float)m_Size/2);
		BuildQuadtree(m_pRootNode, 32);

		// Free resources and return
		Log::Print("Terrain imported!");
		m_bLoaded = true;
		m_pCamera = pCam;
		m_FlagForUpdate = true;
		return S_OK;
	}

	//--------------------------------------------------------------------------------------
	// Create a terrain from a heightmap
	//--------------------------------------------------------------------------------------
	HRESULT Terrain::CreateFromFile(const char* szFile, Camera* pCam)
	{
		// Load a heightmap
		m_szName = szFile;

		// Setup the heightmap load info
		D3DX10_IMAGE_LOAD_INFO info = D3DX10_IMAGE_LOAD_INFO();
		info.FirstMipLevel = 0;
		info.MipLevels = 1;
		info.Usage = D3D10_USAGE_DEFAULT;
		info.Format = DXGI_FORMAT_R16_FLOAT;
		info.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
		ID3D10Texture2D* tTex, *tTex2;

		// Check if we are loading a full terrain file
		int len = strlen(szFile);
		if( (szFile[len-1] == 'f' || szFile[len-1] == 'F') &&
			(szFile[len-2] == 't' || szFile[len-2] == 'T') &&
			(szFile[len-3] == 'p' || szFile[len-3] == 'P') )
		{
			// Load the heightmap
			String fileName = szFile;
			String actualFile = fileName.GetDirectory() + fileName.GetFile().Substring(0, fileName.GetFile().Length()-5);
			if( D3DX10CreateTextureFromFileA( g_pd3dDevice, actualFile + ".dds", &info, 
				NULL, (ID3D10Resource**)&tTex, NULL ) != S_OK )
			{
				Log::Print("Terrain Heightmap failed to load> %s", szFile);
				return E_FAIL;
			}
			
			// Setup the heightmap
			m_Heightmap.CreateFromTexture(tTex);

			// Load the detail map
			if( D3DX10CreateTextureFromFileA( g_pd3dDevice, actualFile + "_d.dds", &info, 
				NULL, (ID3D10Resource**)&tTex2, NULL ) != S_OK )
			{
				Log::Print("Terrain detail map failed to load> %s", szFile);
				return E_FAIL;
			}

			// Load the file
			ifstream file;
			file.open(fileName);
			
			// Detail textures
			char buf[256];
			for(int i=0; i<4; i++)
			{
				file.getline(buf, 255);
				if(strcmp(buf, "NONE") != 0)
					LoadTexture(buf, i);
			}

			// Normal maps
			for(int i=0; i<4; i++)
			{
				file.getline(buf, 255);
				if(strcmp(buf, "NONE") != 0)
					LoadNormalmap(buf, i);
			}

			// Texture scales
			for(int i=0; i<NUM_LAYERS; i++)
			{
				file.getline(buf, 255);
				sscanf(buf, "%f", &m_LayerScales[i]);
			}

			file.close();

			return CreateFromTexture(tTex, tTex2, pCam);
		}

		// Load the heightmap texture
		if( D3DX10CreateTextureFromFileA( g_pd3dDevice, szFile, &info, NULL, (ID3D10Resource**)&tTex, NULL ) != S_OK )
		{
			Log::Print("Terrain Heightmap failed to load> %s", szFile);
			return E_FAIL;
		}

		// Setup the heightmap
		m_Heightmap.CreateFromTexture(tTex);

		// Get the dimensions
		D3D10_TEXTURE2D_DESC hd;
		tTex->GetDesc(&hd);

		// Scale factor
		for(int i=0; i<NUM_LAYERS; i++)
			m_LayerScales[i] = (hd.Width/4);

		// Load a default texture
		LoadTexture("Textures\\grass.png", 0);
				
		return CreateFromTexture(tTex, NULL, pCam);
	}

	//--------------------------------------------------------------------------------------
	// Create a terrain from a heightmap
	//--------------------------------------------------------------------------------------
	HRESULT Terrain::Create(UINT size, Camera* pCam)
	{
		// Load a heightmap
		m_szName = "New Terrain";

		// Create the height texture
		ID3D10Texture2D* tTex = NULL;
		D3D10_TEXTURE2D_DESC desc = D3D10_TEXTURE2D_DESC();
		desc.Width = size;
		desc.Height = size;
		desc.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
		desc.Usage = D3D10_USAGE_DEFAULT;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R16_FLOAT;
		desc.ArraySize = 1;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		HRESULT hr = g_pd3dDevice->CreateTexture2D(&desc, NULL, &tTex);
		if(FAILED(hr)) 
		{
			Log::D3D10Error(hr);
			return hr;
		}

		// Setup the heightmap
		m_Heightmap.CreateFromTexture(tTex);

		// Scale factor
		for(int i=0; i<NUM_LAYERS; i++)
			m_LayerScales[i] = (size/4);

		// Load a default texture
		LoadTexture("Textures\\grass.png", 0);

		return CreateFromTexture(tTex, NULL, pCam);
	}

	

	//--------------------------------------------------------------------------------------
	// Normals are generated by worker threads
	//--------------------------------------------------------------------------------------
	unsigned Terrain::ThreadExecute()
	{
		UpdateBuffers();
		if(m_bQueueThread){
			m_bQueueThread = false;
			Update();
		}
		return (0);
	}



	//--------------------------------------------------------------------------------------
	// Updates the normals and height values in a worker thread
	//--------------------------------------------------------------------------------------
	void Terrain::UpdateBuffers()
	{
		m_bInThread=true;

		UpdateWorldMatrix();
		m_FlagForUpdate = true;
		
		m_bInThread=false;
	}




	//--------------------------------------------------------------------------------------
	// Release the terrain
	//--------------------------------------------------------------------------------------
	void Terrain::Release()
	{
		
#ifdef PHASE_DEBUG
		// Clear the redo stack
		while(!m_RedoStack.IsEmpty())
			m_RedoStack.Pop().tex->Release();

		// Clear the undo stack
		while(!m_UndoStack.IsEmpty())
			m_UndoStack.Pop().tex->Release();		
		m_UndoMemUsage=0;
#endif

		m_HeightExtents.x = 9999;
		m_HeightExtents.y = -500;

		// Release texture layers
		for(int i=0; i<NUM_LAYERS; i++)
		{
			m_Layers[i].texMap = "None";
			m_Layers[i].normalMap = "None";
			m_HasTextureLayer[i] = false;
			m_HasNormalLayer[i] = false;
		}
		m_TextureLayers->Release();
		m_TextureLayerArray->Release();
		m_NormalLayers->Release();
		m_NormalLayerArray->Release();
		
		SAFE_DELETE(m_pRootNode);

		m_Heightmap.Release();
		m_Clipmaps.Release();

		m_StagingHeightmap->Release();

		m_LayerMaps.Release();

		SAFE_RELEASE(m_VertexBuffer);
		SAFE_RELEASE(m_IndexBuffer);

		SAFE_DELETE(m_TrimXSide);
		SAFE_DELETE(m_TrimZSide);
		SAFE_DELETE(m_ClipmapOffsets);

		m_Height.Release();
	}

	
}