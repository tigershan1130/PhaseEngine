//--------------------------------------------------------------------------------------
// File: Texture.cpp
//
// 2D Texture object
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Texture.h"

namespace Core
{

	// The global texture manager
	ResourceManager<Texture>  g_Textures;

	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	Texture::Texture()
	{ 
		m_pTex = NULL; 
	}

	//--------------------------------------------------------------------------------------
	// Loads a texture from a file
	//--------------------------------------------------------------------------------------
	bool Texture::Load(const char* file)
	{
		if(!file)
			return false;

		// First try to load with the raw path
		if(LoadFromFile(file))
			return true;
		
		// Get the file path
		String szFile = file;
		String dir = szFile.GetDirectory();
		if( (file[0]=='c'||file[0]=='C') && file[1]==':' )
			return LoadFromFile(g_szDirectory + szFile.ChopDirectory());
		if(!LoadFromFile(dir + szFile.ChopDirectory()))
			return LoadFromFile(g_szDirectory + file);
		return true;
	}


	//--------------------------------------------------------------------------------------
	// Loads a texture from a file
	//--------------------------------------------------------------------------------------
	bool Texture::LoadFromFile(const char* file)
	{
		if(!file)
			return false;		

		Log::Print(file);
			
		// Load the file into a temp texture
		D3DX10_IMAGE_LOAD_INFO info;
		info.Usage = D3D10_USAGE_IMMUTABLE;
		info.FirstMipLevel = 0;
		info.MipLevels = 0;
		//info.Format = DXGI_FORMAT_BC1_UNORM;
		ID3D10ShaderResourceView* tTex = NULL;	
		if( D3DX10CreateShaderResourceViewFromFileA( g_pd3dDevice, 
													file, 
													&info, 
													NULL, 
													&tTex, NULL ) != S_OK )
				return false;
		if(!tTex)
		{
			Log::Error( "Texture load failed> %s", file);
			return false;
		}

		// If the load was successful, store the new texture
		if(m_pTex)
			Release();
		m_pTex = tTex;

		// Get the width and height
		ID3D10Resource* pRC;
		m_pTex->GetResource(&pRC);
		ID3D10Texture2D* pT = (ID3D10Texture2D*)pRC;
		D3D10_TEXTURE2D_DESC desc;
		pT->GetDesc(&desc);
		m_Width = desc.Width;
		m_Height = desc.Height;
		pRC->Release();
		
		m_szName = String(file).ChopDirectory().c_str();
		return true;
	}


	//--------------------------------------------------------------------------------------
	// Frees the texture
	//--------------------------------------------------------------------------------------
	inline void Texture::Release()
	{
		if(m_pTex)
		{
			ID3D10Resource* pRC;
			m_pTex->GetResource(&pRC);
			pRC->Release();
		}
		SAFE_RELEASE( m_pTex );
	}

}