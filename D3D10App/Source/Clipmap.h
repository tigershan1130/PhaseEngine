//--------------------------------------------------------------------------------------
// File: Clipmap.cpp
//
// Clipmap texture manager
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "RenderSurface.h"

namespace Core
{
	class Clipmap
	{
	public:

		// Creates a clipmap system for an existing texture
		bool Create(int resolution, int clipResolution, int levels, DXGI_FORMAT format);

		// Get the textures
		inline RenderSurface& GetClipmaps(){ return m_Clipmaps; }
		inline ID3D10Texture2D* GetBaseTexture(){ return m_BaseTexture; }
		inline int GetSize(){ return m_Size; }
		inline int GetClipmapSize(){ return m_ClipmapSize; }

		// Forces an update
		inline void ForceUpdate(){ m_FlagForUpdate=true; }

		// Update the clipmaps
		void Update(D3DXVECTOR3 center);

		// Frees all mem
		void Release();

	private:

		ID3D10Texture2D*	m_BaseTexture;		// Local CPU copy of the full texture

		int					m_Size;				// Total texture size
		int					m_ClipmapSize;		// Dimension of each clipmap (N)
		int					m_ClipmapLevels;	// Max number of active levels
		RenderSurface		m_Clipmaps;			// The clipmap textures, one for each level

		bool				m_FlagForUpdate;

		// Trim positioning
		BYTE*			m_TrimXSide;
		BYTE*			m_TrimZSide;
		D3DXVECTOR2*	m_ClipmapOffsets;
	};

}