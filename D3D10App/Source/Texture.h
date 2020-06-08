//--------------------------------------------------------------------------------------
// File: Texture.h
//
// 2D Texture object
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once
#include <string>
#include "ResourceManager.cpp"

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Name: Texture
	// Desc: Retro3D texture class
	//--------------------------------------------------------------------------------------
	class Texture : EngineResource
	{
	friend class ResourceManager<Texture>;
	friend class Material;
	friend class RenderSurface;
	friend class Renderer;
	public:
			Texture();
			
			// Loads a texture from file
			bool Load(const char* file);
			
			// Releases the texture
			void Release();

			inline ID3D10ShaderResourceView* GetResource(){ return m_pTex; }
			inline String GetName(){ return m_szName; }

			// Automatic type convertion
			inline operator ID3D10ShaderResourceView*() const
			{
				return m_pTex;
			}

			inline int GetWidth(){ return m_Width; }
			inline int GetHeight(){ return m_Height; }

	protected:
			ID3D10ShaderResourceView*		m_pTex;			// The texture pointer

			// Loads a texture from file
			bool LoadFromFile(const char* file);

			int m_Width,m_Height;
	};

	// The global texture manager
	extern ResourceManager<Texture>  g_Textures;

}
