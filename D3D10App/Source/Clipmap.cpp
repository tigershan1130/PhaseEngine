//--------------------------------------------------------------------------------------
// File: Clipmap.cpp
//
// Clipmap texture manager
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Clipmap.h"
#include "QMath.h"


namespace Core
{
	//--------------------------------------------------------------------------------------
	// Creates a clipmap system for an existing texture
	//--------------------------------------------------------------------------------------
	bool Clipmap::Create(int resolution, int clipResolution, int levels, DXGI_FORMAT format)
	{
		m_ClipmapLevels = levels;
		m_ClipmapSize = clipResolution;
		m_Size = resolution;

		// Setup the trim positioning arrays
		m_TrimXSide = new BYTE[m_ClipmapLevels];
		m_TrimZSide = new BYTE[m_ClipmapLevels];
		memset(m_TrimXSide, 0, m_ClipmapLevels);
		memset(m_TrimZSide, 0, m_ClipmapLevels);

		// Cached clipmap positions
		m_ClipmapOffsets = new D3DXVECTOR2[m_ClipmapLevels-1];

		// Create the clipmap textures
		DXGI_FORMAT* formats = new DXGI_FORMAT[m_ClipmapLevels-1];
		for(int i=0; i<m_ClipmapLevels-1; i++)
			formats[i] = format;
		HRESULT hr = m_Clipmaps.CreateArray(m_ClipmapSize, m_ClipmapSize, formats, m_ClipmapLevels-1, 1, NULL);
		if(FAILED(hr))
			Log::D3D10Error(hr);
		delete[] formats;

		// Create the base texture
		D3D10_TEXTURE2D_DESC td;
		td.ArraySize = 1;
		td.BindFlags = 0;	
		td.CPUAccessFlags = D3D10_CPU_ACCESS_READ | D3D10_CPU_ACCESS_WRITE;
		td.Format = format;
		td.Height = m_Size;
		td.MipLevels = m_ClipmapLevels;
		td.MiscFlags = 0;		
		td.SampleDesc.Count=1;
		td.SampleDesc.Quality=0;
		td.Usage = D3D10_USAGE_STAGING;
		td.Width = m_Size;
		if(FAILED(g_pd3dDevice->CreateTexture2D(&td, NULL, &m_BaseTexture)))
			return false;
		
		return true;
	}


	//--------------------------------------------------------------------------------------
	// Frees all mem
	//--------------------------------------------------------------------------------------
	void Clipmap::Release()
	{
		m_BaseTexture->Release();
		m_Clipmaps.Release();
		delete[] m_ClipmapOffsets;
		delete[] m_TrimXSide;
		delete[] m_TrimZSide;
	}



	//--------------------------------------------------------------------------------------
	// Update the clipmaps
	//--------------------------------------------------------------------------------------
	void Clipmap::Update(D3DXVECTOR3 center)
	{
		// Update each level
		float size = 1.0f;
		D3DXVECTOR3 camPos(floorf(center.x), 0, floorf(center.z));
		for(int i=0; i<m_ClipmapLevels-1; i++, size*=2)
		{
			// Calculate the new position
			int G = (int)size;
			int N = m_ClipmapSize;

			// Calculate the modulo to G2 of the new position.
			// This makes sure that the current level always fits in the hole of the
			// coarser level. The gridspacing of the coarser level is G * 2
			int G2 = G*2;
			int modX = (int)(camPos.x) % G2;
			int modY = (int)(camPos.z) % G2;
			modX += modX < 0 ? G2 : 0;
			modY += modY < 0 ? G2 : 0;
			m_ClipmapOffsets[i].x = (camPos.x)+(G2 - modX);
			m_ClipmapOffsets[i].y = (camPos.z)+(G2 - modY);

			// Compute the trim locations
			bool update = false;	
			if(G2 - modX > G)
			{
				if(m_TrimXSide[i]==0)
					update = true;
				m_TrimXSide[i] = 1;
			}
			else
			{
				if(m_TrimXSide[i]==1)
					update = true;
				m_TrimXSide[i] = 0;
			}

			if(G2 - modY > G)
			{
				if(m_TrimZSide[i]==0)
					update = true;
				m_TrimZSide[i] = 1;
			}
			else
			{
				if(m_TrimZSide[i]==1)
					update = true;
				m_TrimZSide[i] = 0;
			}

			if(!update && !m_FlagForUpdate)
				continue;

			// The mip-level 'i' in the base texture corresponds to the clip-level.
			// Copy the required regions

			// Compute the region to copy
			float scaledSize = (float)m_Size / size;
			D3DXVECTOR2 texMin = D3DXVECTOR2(-0.5f*m_ClipmapSize*size, -0.5f*m_ClipmapSize*size) + m_ClipmapOffsets[i];
			if(i>0)
				texMin -= D3DXVECTOR2(0, 1);
			texMin /= (float)m_Size;
			texMin += D3DXVECTOR2(0.5f, 0.5f);
			texMin *= scaledSize;
			D3DXVECTOR2 texMax = texMin + D3DXVECTOR2(m_ClipmapSize, m_ClipmapSize);
			UINT dX=0, dY=0;
			if(texMin.x<0)
				dX = -texMin.x+1;
			if(texMin.y<0)
				dY = -texMin.y+1;

			Math::Clamp(texMin, 0.0f, scaledSize);			
			Math::Clamp(texMax, 0.0f, scaledSize);

			// Clamp
			D3D10_BOX sourceRegion;
			sourceRegion.left = texMin.x;
			sourceRegion.right = texMax.x;
			sourceRegion.top = texMin.y;
			sourceRegion.bottom = texMax.y;
			sourceRegion.front = 0;
			sourceRegion.back = 1;

			g_pd3dDevice->CopySubresourceRegion(m_Clipmaps.GetTex()[i], 0, dX, dY, 0, m_BaseTexture, D3D10CalcSubresource(i, 0, m_ClipmapLevels), &sourceRegion);
		}
		m_FlagForUpdate = false;
	}
}