//--------------------------------------------------------------------------------------
// File: TerrainClipmaps.cpp
//
// Implementation of terrain clipmap system
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Terrain.h"
#include "Device.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Creates the clipmap building blocks
	//--------------------------------------------------------------------------------------
	void Terrain::SetupClipmap(int size, int numLevels)
	{
		if(size>1023)
			return;

		// Sizes
		m_ClipmapSize = size;
		m_BlockSize = (size+1)/4;
		m_ClipmapLevels = numLevels;


		// Get the total number of verts needed
		// so we can consolodate into a single
		// vertex buffer to eliminate state changes
		int totalVertLength = m_BlockSize*m_BlockSize +		// Block
			m_BlockSize*3 +				// Ring fix
			m_BlockSize*3 +				// Ring fix
			8*m_BlockSize+4 +				// Trim
			8*m_BlockSize+4 +				// Trim
			8*m_BlockSize+4 +				// Trim
			8*m_BlockSize+4 +				// Trim
			8*m_BlockSize;				// Center trim

		// Allocate the vertex array
		Array<PosVertex> vertexList;
		vertexList.Allocate(totalVertLength);

		// MxM block verts
		m_VBOffsets[VB_BLOCK] = 0;
		int index=0;
		for(int j=0; j<m_BlockSize; j++)
			for(int i=0; i<m_BlockSize; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(i, 0, -j);		

		// Mx3 ring fix verts
		m_VBOffsets[VB_RING1] = index;
		for(int j=0; j<3; j++)
			for(int i=0; i<m_BlockSize; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(i, 0, -j);		

		// 3xM ring fix verts
		m_VBOffsets[VB_RING2] = index;
		for(int i=0; i<m_BlockSize; i++)
			for(int j=0; j<3; j++, index++)
				vertexList[index].pos = D3DXVECTOR3(j, 0, -i);

		// Trim verts (top and left)
		m_VBOffsets[VB_TRIM_TL] = index;
		for(int j=0; j<2; j++)
			for(int i=0; i<2*m_BlockSize+1; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(i, 0, -j);
		for(int j=0; j<2; j++)	
			for(int i=0; i<2*m_BlockSize+1; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(j, 0, -i);

		// Trim verts (top and right)
		m_VBOffsets[VB_TRIM_TR] = index;
		for(int j=0; j<2; j++)
			for(int i=0; i<2*m_BlockSize+1; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(i, 0, -j);
		for(int j=0; j<2; j++)	
			for(int i=0; i<2*m_BlockSize+1; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(j+2*m_BlockSize-1, 0, -i);

		// Trim verts (bottom and right)
		m_VBOffsets[VB_TRIM_BR] = index;
		for(int j=0; j<2; j++)
			for(int i=0; i<2*m_BlockSize+1; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(i, 0, -j-(2*m_BlockSize-1));
		for(int j=0; j<2; j++)	
			for(int i=0; i<2*m_BlockSize+1; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(j+2*m_BlockSize-1, 0, -i+1);

		// Trim verts (bottom and left)
		m_VBOffsets[VB_TRIM_BL] = index;
		for(int j=0; j<2; j++)
			for(int i=0; i<2*m_BlockSize+1; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(i, 0, -j-(2*m_BlockSize-1));
		for(int j=0; j<2; j++)	
			for(int i=0; i<2*m_BlockSize+1; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(j, 0, -i+1);

		// Center Trim verts
		m_VBOffsets[VB_CENTER_TRIM] = index;
		for(int j=0; j<2; j++)
			for(int i=0; i<2*m_BlockSize; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(i, 0, -j);
		for(int j=0; j<2; j++)	
			for(int i=0; i<2*m_BlockSize; i++, index++)
				vertexList[index].pos = D3DXVECTOR3(j, 0, -i);



		// Get the total index length
		int totalIndexLength = (m_BlockSize-1)*(m_BlockSize-1)*6 +	// Block Indices
			(m_BlockSize-1)*12 +					// Ring fix Indices
			(m_BlockSize-1)*12 +					// Ring fix Indices
			(2*m_BlockSize)*12-6 +				// Trim Indices
			(2*m_BlockSize-1)*12;				// CenterTrim Indices

		// Allocate the index array
		Array<USHORT> indexList;
		indexList.Allocate(totalIndexLength);

		// Block Indices
		index=0;
		m_IBOffsets[IB_BLOCK] = index;
		for(int j=0; j<m_BlockSize-1; j++)
			for(int i=0; i<m_BlockSize-1; i++, index+=6)
			{
				// Set the indices for this quad
				int nIndex = i+j*m_BlockSize;
				indexList[index+5]   = nIndex;
				indexList[index+4] = nIndex + m_BlockSize;
				indexList[index+3] = nIndex + m_BlockSize + 1;
				indexList[index+2] = nIndex + m_BlockSize + 1;
				indexList[index+1] = nIndex + 1;
				indexList[index] = nIndex;
			}
			m_IBCount[IB_BLOCK] = index-m_IBOffsets[IB_BLOCK];

			// Ring fix Indices
			m_IBOffsets[IB_RING1] = index;
			for(int j=0; j<2; j++)
				for(int i=0; i<m_BlockSize-1; i++, index+=6)
				{
					// Set the indices for this quad
					int nIndex = i+j*m_BlockSize;
					indexList[index+5]   = nIndex;
					indexList[index+4] = nIndex + m_BlockSize;
					indexList[index+3] = nIndex + m_BlockSize + 1;
					indexList[index+2] = nIndex + m_BlockSize + 1;
					indexList[index+1] = nIndex + 1;
					indexList[index] = nIndex;
				}
				m_IBCount[IB_RING1] = index-m_IBOffsets[IB_RING1];

				// Ring fix Indices
				m_IBOffsets[IB_RING2] = index;
				for(int i=0; i<m_BlockSize-1; i++)
					for(int j=0; j<2; j++, index+=6)
					{
						// Set the indices for this quad
						int nIndex = j+i*3;
						indexList[index+5]   = nIndex;
						indexList[index+4] = nIndex + 3;
						indexList[index+3] = nIndex + 3 + 1;
						indexList[index+2] = nIndex + 3 + 1;
						indexList[index+1] = nIndex + 1;
						indexList[index] = nIndex;
					}
					m_IBCount[IB_RING2] = index-m_IBOffsets[IB_RING2];

					// Trim Indices
					m_IBOffsets[IB_TRIM] = index;
					for(int i=0; i<2*m_BlockSize; i++, index+=6)
					{
						// Set the indices for this quad
						int nIndex = i;
						indexList[index+5]   = nIndex;
						indexList[index+4] = nIndex + 2*m_BlockSize+1;
						indexList[index+3] = nIndex + 2*m_BlockSize+1 + 1;
						indexList[index+2] = nIndex + 2*m_BlockSize+1 + 1;
						indexList[index+1] = nIndex + 1;
						indexList[index] = nIndex;
					}
					for(int i=1; i<2*m_BlockSize; i++, index+=6)
					{
						// Set the indices for this quad
						int nIndex = i + 4*m_BlockSize+2;
						indexList[index]   = nIndex;
						indexList[index+1] = nIndex + 2*m_BlockSize+1;
						indexList[index+2] = nIndex + 2*m_BlockSize+1 + 1;
						indexList[index+3] = nIndex + 2*m_BlockSize+1 + 1;
						indexList[index+4] = nIndex + 1;
						indexList[index+5] = nIndex;
					}
					m_IBCount[IB_TRIM] = index-m_IBOffsets[IB_TRIM];

					// CenterTrim Indices
					m_IBOffsets[IB_CENTER_TRIM] = index;
					for(int i=0; i<2*m_BlockSize-1; i++, index+=6)
					{
						// Set the indices for this quad
						int nIndex = i;
						indexList[index+5]   = nIndex;
						indexList[index+4] = nIndex + 2*m_BlockSize;
						indexList[index+3] = nIndex + 2*m_BlockSize + 1;
						indexList[index+2] = nIndex + 2*m_BlockSize + 1;
						indexList[index+1] = nIndex + 1;
						indexList[index] = nIndex;
					}
					for(int i=0; i<2*m_BlockSize-1; i++, index+=6)
					{
						// Set the indices for this quad
						int nIndex = i + 4*m_BlockSize;
						indexList[index]   = nIndex;
						indexList[index+1] = nIndex + 2*m_BlockSize;
						indexList[index+2] = nIndex + 2*m_BlockSize + 1;
						indexList[index+3] = nIndex + 2*m_BlockSize + 1;
						indexList[index+4] = nIndex + 1;
						indexList[index+5] = nIndex;
					}
					m_IBCount[IB_CENTER_TRIM] = index-m_IBOffsets[IB_CENTER_TRIM];


					// Fill the buffers
					{
						// Master vertex buffer
						D3D10_BUFFER_DESC bd;
						bd.Usage = D3D10_USAGE_IMMUTABLE;
						bd.ByteWidth = sizeof( PosVertex ) * vertexList.Size();
						bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
						bd.CPUAccessFlags = 0;
						bd.MiscFlags = 0;
						D3D10_SUBRESOURCE_DATA InitData;
						InitData.pSysMem = (PosVertex*)vertexList;
						g_pd3dDevice->CreateBuffer( &bd, &InitData, &m_VertexBuffer );

						// Master index buffer
						bd.ByteWidth = sizeof( USHORT ) * indexList.Size();;
						bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
						InitData.pSysMem = (USHORT*)indexList;
						g_pd3dDevice->CreateBuffer( &bd, &InitData, &m_IndexBuffer );
					}

					vertexList.Release();
					indexList.Release();

					//-----------------
					// Block positions
					D3DXVECTOR2 offsetPos = D3DXVECTOR2(-2*(m_BlockSize-1), (m_BlockSize-1));

					// Top row
					m_BlockOffsets[0] = offsetPos+D3DXVECTOR2(0, 0);
					m_BlockOffsets[1] = offsetPos+D3DXVECTOR2((m_BlockSize-1), 0);
					m_BlockOffsets[2] = offsetPos+D3DXVECTOR2(2*(m_BlockSize-1)+2, 0);
					m_BlockOffsets[3] = offsetPos+D3DXVECTOR2(3*(m_BlockSize-1)+2, 0);
					// Bottom row
					m_BlockOffsets[4] = offsetPos+D3DXVECTOR2(0, -3*(m_BlockSize-1)-2);
					m_BlockOffsets[5] = offsetPos+D3DXVECTOR2((m_BlockSize-1), -3*(m_BlockSize-1)-2);
					m_BlockOffsets[6] = offsetPos+D3DXVECTOR2(2*(m_BlockSize-1)+2, -3*(m_BlockSize-1)-2);
					m_BlockOffsets[7] = offsetPos+D3DXVECTOR2(3*(m_BlockSize-1)+2, -3*(m_BlockSize-1)-2);
					// Left column
					m_BlockOffsets[8] = offsetPos+D3DXVECTOR2(0, -(m_BlockSize-1));
					m_BlockOffsets[9] = offsetPos+D3DXVECTOR2(0, -2*(m_BlockSize-1)-2);
					// Right column
					m_BlockOffsets[10] = offsetPos+D3DXVECTOR2(3*(m_BlockSize-1)+2, -(m_BlockSize-1));
					m_BlockOffsets[11] = offsetPos+D3DXVECTOR2(3*(m_BlockSize-1)+2, -2*(m_BlockSize-1)-2);

					// Setup the trim positioning arrays
					m_TrimXSide = new BYTE[m_ClipmapLevels];
					m_TrimZSide = new BYTE[m_ClipmapLevels];
					memset(m_TrimXSide, 0, m_ClipmapLevels);
					memset(m_TrimZSide, 0, m_ClipmapLevels);

					// Cached clipmap positions
					m_ClipmapOffsets = new D3DXVECTOR2[m_ClipmapLevels-1];

					// Create the clipmap height textures
					DXGI_FORMAT* formats = new DXGI_FORMAT[m_ClipmapLevels-1];
					for(int i=0; i<m_ClipmapLevels-1; i++)
						formats[i] = DXGI_FORMAT_R16_FLOAT;
					HRESULT hr = m_Clipmaps.CreateArray(m_ClipmapSize, m_ClipmapSize, formats, m_ClipmapLevels-1, 1, NULL);
					if(FAILED(hr))
						Log::D3D10Error(hr);

					delete[] formats;
	}



	//--------------------------------------------------------------------------------------
	// Update a clipmap
	//--------------------------------------------------------------------------------------
	void Terrain::UpdateClipmaps(Effect& effect)
	{
		// Setup the viewport and ortho matrix
		effect.OrthoProjectionVariable->SetMatrix((float*)m_Clipmaps.GetOrthoMatrix());
		g_pd3dDevice->RSSetViewports(1, &m_Clipmaps.GetViewport());

		// Set the heightmap
		Device::Effect->HeightmapVariable->SetResource(m_Heightmap.GetSRV()[0]);

		// Update each level
		float size = 1.0f;
		D3DXVECTOR3 camPos(floorf(m_pCamera->GetPos().x), 0, floorf(m_pCamera->GetPos().z));
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

			// Update!
			effect.ClipmapOffsetVariable->SetFloatVector((float*)&m_ClipmapOffsets[i]);
			effect.ClipmapScaleVariable->SetFloat(size);
			effect.ClipmapSizeVariable->SetFloat((float)m_ClipmapSize);
			effect.HeightmapSizeVariable->SetFloat((float)m_Size);
			g_pd3dDevice->OMSetRenderTargets(1, &m_Clipmaps.GetRTV()[i], m_Clipmaps.GetDSV());
			effect.Pass[PASS_CLIPMAP_UPDATE]->Apply(0);
			g_pd3dDevice->Draw(6, 0);
		}
		m_FlagForUpdate = false;

		// Update the blendmaps
		m_LayerMaps.Update(m_pCamera->GetPos());
	}



	//--------------------------------------------------------------------------------------
	// Draws a clipmap level
	//--------------------------------------------------------------------------------------
	int Terrain::Render(Effect& effect)
	{
		// Used for offsetting the building blocks
		D3DXVECTOR2 gridOffset;
		D3DXVECTOR3 blockBounds, cullPos, cullBounds;

		// Just for a little optimizing (not worth it i know :)
		const float b2 = m_BlockSize*0.5f;

		// Discrete camera location in the grid
		D3DXVECTOR3 camPos(floorf(m_pCamera->GetPos().x), 0, floorf(m_pCamera->GetPos().z));

		effect.ClipmapSizeVariable->SetFloat((float)m_ClipmapSize);
		effect.HeightmapSizeVariable->SetFloat((float)m_Size);
		effect.HeightmapScaleVariable->SetFloat(m_HeightScale);

		// Compute the height bounds
		float hCenter = (m_HeightExtents.y-m_HeightExtents.x)*0.5f;
		float hMax = m_HeightExtents.y-hCenter;

		// Set the texture layers
		BindTextures(effect);

		// Keep track of poly count
		int polyCount = 0;

		// Patch size
		float size = 1;

		// The finest level is different
		{
			// Bind the clipmaps as textures
			effect.ClipmapVariable->SetResource(m_Clipmaps.GetSRV()[0]);
			effect.BlendClipmapVariable->SetResource(m_LayerMaps.GetClipmaps().GetSRV()[0]);

			// Set the patch size
			effect.ClipmapScaleVariable->SetFloat(size);

			effect.ClipmapFixVariable->SetFloatVector((float*)&m_ClipmapOffsets[0]);

			// Offset
			D3DXVECTOR2 offsetTrim;
			int modX = (int)camPos.x % 2;
			int modY = (int)camPos.z % 2;
			modX += modX < 0 ? 2 : 0;
			modY += modY < 0 ? 2 : 0;
			offsetTrim.x = modX-2;
			offsetTrim.y = modY-2;

			//------------
			// Draw the trim

			// Set the patch offset position
			gridOffset = offsetTrim + D3DXVECTOR2(1-m_BlockSize, m_BlockSize);
			effect.ClipmapOffsetVariable->SetFloatVector((float*)&gridOffset);

			// Draw the trim
			effect.Pass[PASS_GBUFFER_TERRAIN]->Apply(0);
			g_pd3dDevice->DrawIndexed(m_IBCount[IB_CENTER_TRIM], m_IBOffsets[IB_CENTER_TRIM], m_VBOffsets[VB_CENTER_TRIM]);
			polyCount += m_IBCount[IB_CENTER_TRIM]/3;


			//------------
			// Draw the blocks

			// Block positions
			D3DXVECTOR2 offsets[4];
			offsets[0] = D3DXVECTOR2(1, 1);
			offsets[1] = D3DXVECTOR2(2-m_BlockSize, 1);
			offsets[2] = D3DXVECTOR2(1, 2-m_BlockSize);
			offsets[3] = D3DXVECTOR2(2-m_BlockSize, 2-m_BlockSize);

			// 4 blocks
			blockBounds = D3DXVECTOR3(m_BlockSize, hMax, m_BlockSize);
			cullBounds = blockBounds*0.5f;
			for(int i=0; i<4; i++)
			{
				// Set the patch offset position
				gridOffset = offsetTrim + D3DXVECTOR2(offsets[i].x, offsets[i].y + (m_BlockSize-1)-1);
				if(m_pCamera->GetFrustum().CheckRectangle(camPos+D3DXVECTOR3(gridOffset.x + b2, hCenter, gridOffset.y - b2), blockBounds))
				{
					// Draw the patch
					effect.ClipmapOffsetVariable->SetFloatVector((float*)&gridOffset);
					effect.Pass[PASS_GBUFFER_TERRAIN]->Apply(0);
					g_pd3dDevice->DrawIndexed(m_IBCount[IB_BLOCK], m_IBOffsets[IB_BLOCK], m_VBOffsets[VB_BLOCK]);
					polyCount += m_IBCount[IB_BLOCK]/3;
				}
			}
		}

		// Render the coarser levels
		for(int level=0, size=1; level<m_ClipmapLevels-1; level++, size*=2)
		{
			// Set the level size
			effect.ClipmapScaleVariable->SetFloat(size);

			// Bind the clipmaps as textures
			effect.ClipmapVariable->SetResource(m_Clipmaps.GetSRV()[level]);
			effect.BlendClipmapVariable->SetResource(m_LayerMaps.GetClipmaps().GetSRV()[level]);

			// Set the offset
			effect.ClipmapFixVariable->SetFloatVector((float*)&m_ClipmapOffsets[level]);

			// Draw the trim
			{
				// Trim offset
				gridOffset = D3DXVECTOR2(-size*(m_BlockSize+1), size*(m_BlockSize-1));					

				// Draw the trim
				effect.ClipmapOffsetVariable->SetFloatVector((float*)&gridOffset);
				effect.Pass[PASS_GBUFFER_TERRAIN]->Apply(0);
				if(m_TrimZSide[level]==1 && m_TrimXSide[level]==0)
					g_pd3dDevice->DrawIndexed(m_IBCount[IB_TRIM], m_IBOffsets[IB_TRIM], m_VBOffsets[VB_TRIM_TL]);
				else if(m_TrimZSide[level]==1 && m_TrimXSide[level]==1)
					g_pd3dDevice->DrawIndexed(m_IBCount[IB_TRIM], m_IBOffsets[IB_TRIM], m_VBOffsets[VB_TRIM_TR]);
				else if(m_TrimZSide[level]==0 && m_TrimXSide[level]==1)
					g_pd3dDevice->DrawIndexed(m_IBCount[IB_TRIM], m_IBOffsets[IB_TRIM], m_VBOffsets[VB_TRIM_BR]);
				else
					g_pd3dDevice->DrawIndexed(m_IBCount[IB_TRIM], m_IBOffsets[IB_TRIM], m_VBOffsets[VB_TRIM_BL]);
				polyCount += m_IBCount[IB_TRIM]/3;
			}

			// Draw the blocks
			blockBounds = D3DXVECTOR3(m_BlockSize*size, hMax, m_BlockSize*size);
			cullBounds = blockBounds*0.5f;
			float b2s = b2*size;
			for(int i=0; i<12; i++)
			{
				// Set the patch offset position
				gridOffset = D3DXVECTOR2(size*(m_BlockOffsets[i].x-2), size*(m_BlockOffsets[i].y+m_BlockSize-1));

				// Draw the patch if it is visible and contained inside the terrain bounds
				cullPos = camPos+D3DXVECTOR3(gridOffset.x+b2s, hCenter, gridOffset.y-b2s);
				if(m_pCamera->GetFrustum().CheckRectangle(cullPos, blockBounds) &&
					!( cullPos.x-cullBounds.x > m_Size/2 || cullPos.x+cullBounds.x < -m_Size/2 ||
					cullPos.z-cullBounds.z > m_Size/2 || cullPos.z+cullBounds.z < -m_Size/2 ))
				{
					effect.ClipmapOffsetVariable->SetFloatVector((float*)&gridOffset);
					effect.Pass[PASS_GBUFFER_TERRAIN]->Apply(0);
					g_pd3dDevice->DrawIndexed(m_IBCount[IB_BLOCK], m_IBOffsets[IB_BLOCK], m_VBOffsets[VB_BLOCK]);
					polyCount += m_IBCount[IB_BLOCK]/3;
				}
			}


			// Draw the ring fix strips
			{
				// Left
				gridOffset = D3DXVECTOR2(-2*size*m_BlockSize, 0);
				blockBounds = D3DXVECTOR3(m_BlockSize*size, hMax, 6*size);
				cullBounds = blockBounds*0.5f;
				cullPos = camPos+D3DXVECTOR3(gridOffset.x + b2s, hCenter, gridOffset.y);
				if(m_pCamera->GetFrustum().CheckRectangle(cullPos, blockBounds) &&
					!( cullPos.x-cullBounds.x > m_Size/2 || cullPos.x+cullBounds.x < -m_Size/2 ||
					cullPos.z-cullBounds.z > m_Size/2 || cullPos.z+cullBounds.z < -m_Size/2 ))
				{
					effect.ClipmapOffsetVariable->SetFloatVector((float*)&gridOffset);
					effect.Pass[PASS_GBUFFER_TERRAIN]->Apply(0);
					g_pd3dDevice->DrawIndexed(m_IBCount[IB_RING1], m_IBOffsets[IB_RING1], m_VBOffsets[VB_RING1]);
					polyCount += m_IBCount[IB_RING1]/3;
				}

				// Right
				gridOffset = D3DXVECTOR2(size*(m_BlockSize-1), 0);
				cullPos = camPos+D3DXVECTOR3(gridOffset.x +b2s, hCenter, gridOffset.y);
				if(m_pCamera->GetFrustum().CheckRectangle(cullPos, blockBounds) &&
					!( cullPos.x-cullBounds.x > m_Size/2 || cullPos.x+cullBounds.x < -m_Size/2 ||
					cullPos.z-cullBounds.z > m_Size/2 || cullPos.z+cullBounds.z < -m_Size/2 ))
				{
					effect.ClipmapOffsetVariable->SetFloatVector((float*)&gridOffset);
					effect.Pass[PASS_GBUFFER_TERRAIN]->Apply(0);
					g_pd3dDevice->DrawIndexed(m_IBCount[IB_RING1], m_IBOffsets[IB_RING1], m_VBOffsets[VB_RING1]);
					polyCount += m_IBCount[IB_RING1]/3;
				}

				// Top
				gridOffset = D3DXVECTOR2(-2*size, 2*size*(m_BlockSize-1));
				blockBounds = D3DXVECTOR3(6*size, hMax, m_BlockSize*size);
				cullBounds = blockBounds*0.5f;
				cullPos = camPos+D3DXVECTOR3(gridOffset.x+1.5f*size, hCenter, gridOffset.y - b2s);
				if(m_pCamera->GetFrustum().CheckRectangle(cullPos, blockBounds) &&
					!( cullPos.x-cullBounds.x > m_Size/2 || cullPos.x+cullBounds.x < -m_Size/2 ||
					cullPos.z-cullBounds.z > m_Size/2 || cullPos.z+cullBounds.z < -m_Size/2 ))
				{
					effect.ClipmapOffsetVariable->SetFloatVector((float*)&gridOffset);
					effect.Pass[PASS_GBUFFER_TERRAIN]->Apply(0);
					g_pd3dDevice->DrawIndexed(m_IBCount[IB_RING2], m_IBOffsets[IB_RING2], m_VBOffsets[VB_RING2]);
					polyCount += m_IBCount[IB_RING2]/3;
				}

				// Bottom
				gridOffset = D3DXVECTOR2(-2*size, -size*(m_BlockSize+1));
				cullPos = camPos+D3DXVECTOR3(gridOffset.x+1.5f*size, hCenter, gridOffset.y - b2s);
				if(m_pCamera->GetFrustum().CheckRectangle(cullPos, blockBounds) &&
					!( cullPos.x-cullBounds.x > m_Size/2 || cullPos.x+cullBounds.x < -m_Size/2 ||
					cullPos.z-cullBounds.z > m_Size/2 || cullPos.z+cullBounds.z < -m_Size/2 ))
				{
					effect.ClipmapOffsetVariable->SetFloatVector((float*)&gridOffset);
					effect.Pass[PASS_GBUFFER_TERRAIN]->Apply(0);
					g_pd3dDevice->DrawIndexed(m_IBCount[IB_RING2], m_IBOffsets[IB_RING2], m_VBOffsets[VB_RING2]);
					polyCount += m_IBCount[IB_RING2]/3;
				}
			}

		}
		return polyCount;
	}

}