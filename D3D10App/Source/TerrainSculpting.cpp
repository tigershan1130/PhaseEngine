//--------------------------------------------------------------------------------------
// File: TerrainSculpting.cpp
//
// Terrain sculpting functions
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "Log.h"
#include "Terrain.h"

namespace Core
{

#ifdef PHASE_DEBUG

	//--------------------------------------------------------------------------------------
	// Terrain sculpting
	//--------------------------------------------------------------------------------------
	void Terrain::Sculpt(D3DXVECTOR3 center, float radius, float hardness, float strength, float delta, int detail, SCULPT_TYPE mode)
	{
		// Determine the region we need to use
		D3DXVECTOR2 pos = D3DXVECTOR2(floorf(center.x), floorf(center.z)) + D3DXVECTOR2(0.5f, 0.5f)*m_Size;
		D3DXVECTOR2 minBounds = pos - D3DXVECTOR2(radius, radius);
		D3DXVECTOR2 maxBounds = pos + D3DXVECTOR2(radius, radius);
		Math::Clamp(minBounds, 0, m_Size-1);
		Math::Clamp(maxBounds, 0, m_Size-1);

		// Map the heightmap
		if(mode != SCULPT_PAINT)
		{
			D3D10_MAPPED_TEXTURE2D mappedTexture;
			D3DXFLOAT16* pTexels;
			m_StagingHeightmap->Map(0, D3D10_MAP_READ_WRITE, 0, &mappedTexture);
			pTexels = (D3DXFLOAT16*)mappedTexture.pData;

			// Process the sculpting
			for( int row = (int)minBounds.y; row < (int)maxBounds.y; row++ )
			{
				UINT rowStart = row * mappedTexture.RowPitch/2;
				for( int col = (int)minBounds.x; col < (int)maxBounds.x; col++)
				{
					// Convert this index to world space coords
					D3DXVECTOR2 worldPos;
					worldPos.x = col;
					worldPos.y = row;

					// Only operate within the radius
					float dist = D3DXVec2Length( &(worldPos-pos) );
					if(dist>=radius)
						continue;

					// Smooth strength falloff
					float s = strength*2;
					if(dist > hardness)
						s *= (radius-dist)/(radius-hardness);

					// Modify the heightmap
					if(mode==SCULPT_SMOOTH)
					{	
						// Get the nearby heights and average them
						int count=0;
						float avgHeight = 0;
						for(int y=-2; y<2; y++)
							for(int x=-2; x<2; x++, count++)
							{
								int rx = static_cast<int>(col)+x;
								int ry = static_cast<int>(row)+y;
								Math::Clamp(rx, 0, m_Size-1);
								Math::Clamp(ry, 0, m_Size-1);
								avgHeight += (float)m_Height[ry*m_Size + rx];
							}
							avgHeight /= count;
							pTexels[rowStart + col] = (D3DXFLOAT16)(Math::Lerp((float)m_Height[row*m_Size + col], avgHeight, s*0.3f) /  m_HeightScale);
					}
					else
						pTexels[rowStart + col] = pTexels[rowStart + col]+(D3DXFLOAT16)(delta*s);

					//Math::Clamp(pTexels[rowStart + col], (D3DXFLOAT16)0, (D3DXFLOAT16)1);
				}
			}

			// Update the height values
			for( UINT row = minBounds.y; row < maxBounds.y; row++ )
			{
				UINT rowStart = row * mappedTexture.RowPitch/2;
				for( UINT col = minBounds.x; col < maxBounds.x; col++ )
				{
					// Update the height
					m_Height[row*m_Size+col] = pTexels[rowStart + col] * m_HeightScale;
					
					// Track the min/max
					m_HeightExtents.x = Math::Min(m_HeightExtents.x, (float)m_Height[row*m_Size+col]);
					m_HeightExtents.y = Math::Max(m_HeightExtents.y, (float)m_Height[row*m_Size+col]);
				}
			}
			m_StagingHeightmap->Unmap(0);

			// Copy the subregion back to the main heightmap
			D3D10_BOX sourceRegion;
			sourceRegion.left = minBounds.x;
			sourceRegion.top = minBounds.y;
			sourceRegion.right = maxBounds.x;
			sourceRegion.bottom = maxBounds.y;
			sourceRegion.front = 0;
			sourceRegion.back = 1;
			g_pd3dDevice->CopySubresourceRegion(m_Heightmap.GetTex()[0], 0, minBounds.x, minBounds.y, 0, 
				m_StagingHeightmap, 0, &sourceRegion);

			// Flag all clipmaps to be updated
			m_FlagForUpdate = true;
		}
		else
		{
			D3D10_MAPPED_TEXTURE2D mappedTexture;
			BlendTexel* pTexels;
			m_LayerMaps.GetBaseTexture()->Map(0, D3D10_MAP_READ_WRITE, 0, &mappedTexture);
			pTexels = (BlendTexel*)mappedTexture.pData;

			// Process the sculpting
			D3DXVECTOR4 texel;
			for( int row = (int)minBounds.y; row < (int)maxBounds.y; row++ )
			{
				UINT rowStart = row * mappedTexture.RowPitch/4;
				for( int col = (int)minBounds.x; col < (int)maxBounds.x; col++)
				{
					// Convert this index to world space coords
					D3DXVECTOR2 worldPos;
					worldPos.x = col;
					worldPos.y = row;

					// Only operate within the radius
					float dist = D3DXVec2Length( &(worldPos-pos) );
					if(dist>=radius)
						continue;

					// Smooth strength falloff
					float s = strength * Math::Max(1.0f-(dist/hardness), 0.1f);

					// Put into float space for better accuracy
					D3DXVECTOR2 blend;
					blend.x = (float)pTexels[rowStart + col].data[2] / 255.0f;
					blend.y = (float)pTexels[rowStart + col].data[3] / 255.0f;
					
					// Increment the selected blend component
					const float amt = 0.15f*s;
					if(detail==pTexels[rowStart + col].data[0])
					{
						blend.x += amt;
						blend.y -= amt;
					}
					else if(detail==pTexels[rowStart + col].data[1])
					{
						blend.y += amt;
						blend.x -= amt;
					}
					else
					{
						if(pTexels[rowStart + col].data[2] > pTexels[rowStart + col].data[3])
						{
							pTexels[rowStart + col].data[1] = detail;
							blend.y = amt;
						}
						else
						{
							pTexels[rowStart + col].data[0] = detail;
							blend.x = amt;
						}
					}
					
					// Clamping
					Math::Clamp(blend.x, 0.0f, 1.0f);
					Math::Clamp(blend.y, 0.0f, 1.0f);

					// Scale the components back into 8 bit range
					pTexels[rowStart + col].data[2] = (UCHAR)(blend.x*255.0f);
					pTexels[rowStart + col].data[3] = (UCHAR)(blend.y*255.0f);

					// Move the id down the line if needed
					/*if( pTexels[rowStart + col].data[2] < pTexels[rowStart + col].data[3] )
					{
						Swap(pTexels[rowStart + col].data[0], pTexels[rowStart + col].data[1]);
						Swap(pTexels[rowStart + col].data[2], pTexels[rowStart + col].data[3]);
					}*/
				}
			}

			// Unmap the mip-maps
			float size = 2.0f;
			for(int i=1; i<CLIPMAP_LEVELS-1; i++, size*=2.0f)
			{
				// Map this mip map
				D3D10_MAPPED_TEXTURE2D mappedMip;
				BlendTexel* pMipTexels;
				m_LayerMaps.GetBaseTexture()->Map(i, D3D10_MAP_READ_WRITE, 0, &mappedMip);
				pMipTexels = (BlendTexel*)mappedMip.pData;
				
				// Update the height values for this level
				D3DXVECTOR2 minB = minBounds / size; 
				D3DXVECTOR2 maxB = maxBounds / size; 
				UINT mRow = minBounds.y;
				for( UINT row = minB.y; row < maxB.y; row++, mRow+=size )
				{
					UINT rowStart = row * mappedMip.RowPitch/4;
					UINT mRowStart = mRow * mappedTexture.RowPitch/4;
					UINT mCol = minBounds.x;
					for( UINT col = minB.x; col < maxB.x; col++, mCol+=size )
					{
						// Update the values
						for(int count=0; count<4; count++)
						{
							float sum = static_cast<float>(pTexels[mRowStart + mCol+1].data[count]) / 255.0f;
							sum += static_cast<float>(pTexels[mRowStart + mCol-1].data[count]) / 255.0f;
							sum += static_cast<float>(pTexels[mRowStart+(mappedTexture.RowPitch/4) + mCol].data[count]) / 255.0f;
							sum += static_cast<float>(pTexels[mRowStart-(mappedTexture.RowPitch/4) + mCol].data[count]) / 255.0f;
							sum *= 0.25f;
							sum *= 255.0f;
							pMipTexels[rowStart + col].data[count] = static_cast<UCHAR>(sum);
						}
						
					}
				}
				m_LayerMaps.GetBaseTexture()->Unmap(i);
			}
			m_LayerMaps.GetBaseTexture()->Unmap(0);

			m_LayerMaps.ForceUpdate();
		}
	}


	
	//--------------------------------------------------------------------------------------
	// Reverses the last sculpt action
	//--------------------------------------------------------------------------------------
	void Terrain::UndoSculpt()
	{
		if(m_UndoStack.IsEmpty())
			return;
		
		// Perform the undo
		SculptRegion r = m_UndoStack.Pop();
		ApplyUndo(r);
		
		// Force a clipmap update
		if(r.paint)
			m_LayerMaps.ForceUpdate();
		else
			m_FlagForUpdate = true;

		// Push to the redo stack
		m_RedoStack.Push(r);
	}


	//--------------------------------------------------------------------------------------
	// Re-applies the last undone action
	//--------------------------------------------------------------------------------------
	void Terrain::RedoSculpt()
	{
		if(m_RedoStack.IsEmpty())
			return;
		
		// Perform the redo
		SculptRegion r = m_RedoStack.Pop();
		ApplyUndo(r);

		// Force a clipmap update
		if(r.paint)
			m_LayerMaps.ForceUpdate();
		else
			m_FlagForUpdate = true;
		
		// Push to the undo stack
		m_UndoStack.Push(r);
	}

	
	//--------------------------------------------------------------------------------------
	// Actually applies the sculpt undo to the maps
	//--------------------------------------------------------------------------------------
	void Terrain::ApplyUndo( SculptRegion &r )
	{
		D3D10_BOX localRegion;
		localRegion.left = 0;
		localRegion.right = r.region.right-r.region.left;
		localRegion.front = 0;
		localRegion.back = 1;
		localRegion.top = 0;
		localRegion.bottom = r.region.bottom-r.region.top;

		D3DXVECTOR2 minBounds(r.region.left, r.region.top);
		D3DXVECTOR2 maxBounds(r.region.right, r.region.bottom);
		
		if(r.paint)
		{
			g_pd3dDevice->CopySubresourceRegion(m_LayerMaps.GetBaseTexture(), 0, r.region.left, r.region.top, 0, 
				r.tex, 0, &localRegion);

			// Unmap the mip-maps
			float size = 2.0f;
			D3D10_MAPPED_TEXTURE2D mappedTex;
			m_LayerMaps.GetBaseTexture()->Map(0, D3D10_MAP_READ_WRITE, 0, &mappedTex);
			BlendTexel* pTexels = (BlendTexel*)mappedTex.pData;

			for(int i=1; i<CLIPMAP_LEVELS-1; i++, size*=2.0f)
			{
				// Map this mip map
				D3D10_MAPPED_TEXTURE2D mappedMip;
				BlendTexel* pMipTexels;
				m_LayerMaps.GetBaseTexture()->Map(i, D3D10_MAP_READ_WRITE, 0, &mappedMip);
				pMipTexels = (BlendTexel*)mappedMip.pData;

				// Update the height values for this level
				D3DXVECTOR2 minB = minBounds / size; 
				D3DXVECTOR2 maxB = maxBounds / size; 
				UINT mRow = minBounds.y;
				for( UINT row = minB.y; row < maxB.y; row++, mRow+=size )
				{
					UINT rowStart = row * mappedMip.RowPitch/4;
					UINT mRowStart = mRow * mappedTex.RowPitch/4;
					UINT mCol = minBounds.x;
					for( UINT col = minB.x; col < maxB.x; col++, mCol+=size )
					{
						// Update the values
						for(int count=0; count<4; count++)
						{
							float sum = static_cast<float>(pTexels[mRowStart + mCol+1].data[count]) / 255.0f;
							sum += static_cast<float>(pTexels[mRowStart + mCol-1].data[count]) / 255.0f;
							sum += static_cast<float>(pTexels[mRowStart+(mappedTex.RowPitch/4) + mCol].data[count]) / 255.0f;
							sum += static_cast<float>(pTexels[mRowStart-(mappedTex.RowPitch/4) + mCol].data[count]) / 255.0f;
							sum *= 0.25f;
							sum *= 255.0f;
							pMipTexels[rowStart + col].data[count] = static_cast<UCHAR>(sum);
						}

					}
				}
				m_LayerMaps.GetBaseTexture()->Unmap(i);
			}
			m_LayerMaps.GetBaseTexture()->Unmap(0);
		}
		else
		{
			g_pd3dDevice->CopySubresourceRegion(m_StagingHeightmap, 0, r.region.left, r.region.top, 0, 
				r.tex, 0, &localRegion);

			g_pd3dDevice->CopySubresourceRegion(m_Heightmap.GetTex()[0], 0, r.region.left, r.region.top, 0, 
				r.tex, 0, &localRegion);

			// Update the height values
			D3D10_MAPPED_TEXTURE2D mappedTex;
			m_StagingHeightmap->Map(0, D3D10_MAP_READ_WRITE, 0, &mappedTex);
			D3DXFLOAT16* pTexels = (D3DXFLOAT16*)mappedTex.pData;
			for( UINT row = minBounds.y; row < maxBounds.y; row++ )
			{
				UINT rowStart = row * mappedTex.RowPitch/2;
				for( UINT col = minBounds.x; col < maxBounds.x; col++ )
				{
					// Update the height
					m_Height[row*m_Size+col] = pTexels[rowStart + col] * m_HeightScale;

					// Track the min/max
					m_HeightExtents.x = Math::Min(m_HeightExtents.x, (float)m_Height[row*m_Size+col]);
					m_HeightExtents.y = Math::Max(m_HeightExtents.y, (float)m_Height[row*m_Size+col]);
				}
			}
			m_StagingHeightmap->Unmap(0);
		}
	}


	
	//--------------------------------------------------------------------------------------
	// Used to cache a region onto the undo stack
	//--------------------------------------------------------------------------------------
	void Terrain::CacheSculptRegion(D3DXVECTOR3 pickPoint, float radius, bool paint)
	{
		// Determine the region we need to use
		D3DXVECTOR2 pos = D3DXVECTOR2(floorf(pickPoint.x), floorf(pickPoint.z)) + D3DXVECTOR2(0.5f, 0.5f)*m_Size;
		D3DXVECTOR2 minBounds = pos - D3DXVECTOR2(radius, radius);
		D3DXVECTOR2 maxBounds = pos + D3DXVECTOR2(radius, radius);
		Math::Clamp(minBounds, 0, m_Size-1);
		Math::Clamp(maxBounds, 0, m_Size-1);

		// Copy the subregion back to the main heightmap
		D3D10_BOX sourceRegion;
		sourceRegion.left = minBounds.x;
		sourceRegion.top = minBounds.y;
		sourceRegion.right = maxBounds.x;
		sourceRegion.bottom = maxBounds.y;
		sourceRegion.front = 0;
		sourceRegion.back = 1;

		// Cache this texture onto the undo-stack
		ID3D10Texture2D* pTexToCache;
		D3D10_TEXTURE2D_DESC td;
		if(paint)
			pTexToCache = m_LayerMaps.GetBaseTexture();
		else
			pTexToCache = m_StagingHeightmap;
		
		SculptRegion region;
		region.region = sourceRegion;
		region.paint = paint;

		pTexToCache->GetDesc(&td);
		td.MipLevels = 1;
		td.Width = maxBounds.x-minBounds.x;
		td.Height = maxBounds.y-minBounds.y;
		g_pd3dDevice->CreateTexture2D(&td, NULL, &region.tex);
		g_pd3dDevice->CopySubresourceRegion(region.tex, 0, 0, 0, 0, 
			pTexToCache, 0, &sourceRegion);

		// Clear the redo stack
		while(!m_RedoStack.IsEmpty())
		{
			SculptRegion r = m_RedoStack.Pop();
			m_UndoMemUsage -= r.mem;
			r.tex->Release();
		}

		// In order to cache this, make sure the memory usage is within limits
		if(paint)
			region.mem = (td.Width*td.Height*4) / 1000000.0;
		else
			region.mem = (td.Width*td.Height*2) / 1000000.0;
		/*while(m_UndoMemUsage+region.mem > m_UndoMaxMem && !m_UndoStack.IsEmpty())
		{
			// We need to free some memory from the back of the stack
			SculptRegion r = m_UndoStack.PopBack();
			m_UndoMemUsage -= r.mem;
			r.tex->Release();
		}*/
		
		// Add to the stack
		m_UndoMemUsage += region.mem;
		m_UndoStack.Push(region);		
	}

#endif
}