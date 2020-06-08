//--------------------------------------------------------------------------------------
// File: Terrain.cpp
//
// 3D heightmap based terrain
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "Terrain.h"

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Performs a ray intersection test with the terrain
	//--------------------------------------------------------------------------------------
	bool Terrain::RayIntersection(D3DXVECTOR3& rayPos, D3DXVECTOR3& rayDir, float& dist)
	{
		dist = RayNodeIntersect(m_pRootNode, rayPos, rayDir);
		return (dist>0);
	}

	//--------------------------------------------------------------------------------------
	// Performs a ray intersection test with the terrain at the restricted area
	//--------------------------------------------------------------------------------------
	float Terrain::RayLeafIntersection(QuadtreeNode* pNode, D3DXVECTOR3& rayPos, D3DXVECTOR3& rayDir)
	{
		// Convert the node position into heightmap coords
		D3DXVECTOR2 pos = D3DXVECTOR2(floorf(pNode->pos.x), floorf(pNode->pos.z)) + D3DXVECTOR2(0.5f, 0.5f)*m_Size;
		D3DXVECTOR2 minBounds = pos - D3DXVECTOR2(pNode->size.x, pNode->size.z);
		D3DXVECTOR2 maxBounds = pos + D3DXVECTOR2(pNode->size.x, pNode->size.z);
		Math::Clamp(minBounds, 0, m_Size-1);
		Math::Clamp(maxBounds, 0, m_Size-1);

		// Check this region of the heightmap
		D3DXVECTOR3 tri[3];
		float u,v,t;
		D3DXVECTOR3 offset(-m_Size/2, 0, -m_Size/2);
		float dist = 9999999999;
		bool hit=false;
		for(int y=minBounds.y; y<maxBounds.y; y++)
		{
			for(int x=minBounds.x; x<maxBounds.x; x++)
			{
				// Form a quad at this location
				int index = y*m_Size+x;

				// Check the first triangle				
				tri[0] = D3DXVECTOR3(x, (float)m_Height[index], y) + offset;
				tri[1] = D3DXVECTOR3(x+1, (float)m_Height[index+1], y) + offset;
				tri[2] = D3DXVECTOR3(x+1, (float)m_Height[index+m_Size+1], y+1) + offset;
				if(D3DXIntersectTri(&tri[0], &tri[1], &tri[2], &rayPos, &rayDir, &u, &v, &t) && t<dist)
				{
					dist = t;
					hit = true;
				}

				// Check the 2nd triangle
				tri[0] = D3DXVECTOR3(x+1, (float)m_Height[index+m_Size+1], y+1) + offset;
				tri[1] = D3DXVECTOR3(x, (float)m_Height[index+m_Size], y+1) + offset;
				tri[2] = D3DXVECTOR3(x, (float)m_Height[index], y) + offset;
				if(D3DXIntersectTri(&tri[0], &tri[1], &tri[2], &rayPos, &rayDir, &u, &v, &t) && t<dist)
				{
					dist = t;
					hit = true;
				}
			}
		}
		if(hit)
			return dist;
		return 0;
	}


	//--------------------------------------------------------------------------------------
	// Returns the leaf node hit by the ray
	//--------------------------------------------------------------------------------------
	float Terrain::RayNodeIntersect(QuadtreeNode* pNode, D3DXVECTOR3& rayPos, D3DXVECTOR3& rayDir)
	{
		// If the ray hits this node, check all child nodes, otherwise return back null
		if(D3DXBoxBoundProbe( &(pNode->pos-pNode->size), &(pNode->pos+pNode->size), &rayPos, &rayDir))
		{
			pNode->gotHit=true;
			
			// If the node is a leaf, return this final intersection
			if(pNode->isLeaf)
				return RayLeafIntersection(pNode, rayPos, rayDir);
			
			// Get the closest hit child node
			float d1 = RayNodeIntersect(pNode->pChildNodes[0], rayPos, rayDir);
			float d2 = RayNodeIntersect(pNode->pChildNodes[1], rayPos, rayDir);
			float d3 = RayNodeIntersect(pNode->pChildNodes[2], rayPos, rayDir);
			float d4 = RayNodeIntersect(pNode->pChildNodes[3], rayPos, rayDir);
			if(d1+d2+d3+d4 == 0)
				return 0;
			if(d1==0) d1=999999999;
			if(d2==0) d2=999999999;
			if(d3==0) d3=999999999;
			if(d4==0) d4=999999999;
			return Math::Min( Math::Min(d1, d2), Math::Min(d3,d4) );
		}
		else
		{
			pNode->gotHit=false;
			return 0;
		}
	}


	//--------------------------------------------------------------------------------------
	// Builds a very basic quadtree down to a minimal node size
	//--------------------------------------------------------------------------------------
	void Terrain::BuildQuadtree(QuadtreeNode* pNode, float minNodeSize)
	{
		// If the node has reached the minimal size, break out
		if(pNode->size.x<=minNodeSize || pNode->size.z<=minNodeSize)
		{
			pNode->isLeaf=true;
			return;
		}

		// Make four child nodes
		pNode->pChildNodes[0] = new QuadtreeNode();
		pNode->pChildNodes[1] = new QuadtreeNode();
		pNode->pChildNodes[2] = new QuadtreeNode();
		pNode->pChildNodes[3] = new QuadtreeNode();

		// Setup the sizes to be 1/2 the parent size	
		D3DXVECTOR3 newSize(pNode->size.x*0.5f, pNode->size.y, pNode->size.z*0.5f);
		pNode->pChildNodes[0]->size = newSize;
		pNode->pChildNodes[1]->size = newSize;
		pNode->pChildNodes[2]->size = newSize;
		pNode->pChildNodes[3]->size = newSize;
		
		// Split the parent's volume into four equal regions
		pNode->pChildNodes[0]->pos = pNode->pos + D3DXVECTOR3(newSize.x, 0, newSize.z);
		pNode->pChildNodes[1]->pos = pNode->pos + D3DXVECTOR3(-newSize.x, 0, newSize.z);
		pNode->pChildNodes[2]->pos = pNode->pos + D3DXVECTOR3(newSize.x, 0, -newSize.z);
		pNode->pChildNodes[3]->pos = pNode->pos + D3DXVECTOR3(-newSize.x, 0, -newSize.z);

		// Continue the subdivision
		BuildQuadtree(pNode->pChildNodes[0], minNodeSize);
		BuildQuadtree(pNode->pChildNodes[1], minNodeSize);
		BuildQuadtree(pNode->pChildNodes[2], minNodeSize);
		BuildQuadtree(pNode->pChildNodes[3], minNodeSize);
	}

	
	//--------------------------------------------------------------------------------------
	// Process new height extents
	//--------------------------------------------------------------------------------------
	void Terrain::UpdateNodeHeight(QuadtreeNode* pNode)
	{
		if(pNode==NULL) return;

		// Compute the height bounds
		float hCenter = (m_HeightExtents.y-m_HeightExtents.x)*0.5f;
		float hMax = m_HeightExtents.y-hCenter;

		pNode->pos.y = hCenter;
		pNode->size.y = hMax;
		UpdateNodeHeight(pNode->pChildNodes[0]);
		UpdateNodeHeight(pNode->pChildNodes[1]);
		UpdateNodeHeight(pNode->pChildNodes[2]);
		UpdateNodeHeight(pNode->pChildNodes[3]);
	}


	#ifdef PHASE_DEBUG


	//--------------------------------------------------------------------------------------
	// Draws the quadtree nodes in debug
	//--------------------------------------------------------------------------------------
	void Terrain::RenderQuadtreeDebug(MeshObject& boxMesh, Effect& effect)
	{
		RenderNodeDebug(m_pRootNode, boxMesh, effect);
	}


	//--------------------------------------------------------------------------------------
	// Draws the quadtree nodes in debug
	//--------------------------------------------------------------------------------------
	void Terrain::RenderNodeDebug(QuadtreeNode* pNode, MeshObject& boxMesh, Effect& effect)
	{
		/*if(pNode->isLeaf)
		{
			if(pNode->gotHit)
				effect.MaterialDiffuseVariable->SetFloatVector((float*)&D3DXVECTOR4(1, 0, 0, 0));
			else
				effect.MaterialDiffuseVariable->SetFloatVector((float*)&D3DXVECTOR4(1, 1, 1, 1));
			pNode->gotHit = false;
			boxMesh.SetPos(pNode->pos);
			boxMesh.SetScale(pNode->size*2);
			effect.WorldMatrixVariable->SetMatrix((float*)&boxMesh.GetWorldMatrix());
			effect.Pass[PASS_WIREFRAME]->Apply(0);
			boxMesh.GetMesh()->Render();
		}
		else
		{
			RenderNodeDebug(pNode->pChildNodes[0], boxMesh, effect);
			RenderNodeDebug(pNode->pChildNodes[1], boxMesh, effect);
			RenderNodeDebug(pNode->pChildNodes[2], boxMesh, effect);
			RenderNodeDebug(pNode->pChildNodes[3], boxMesh, effect);
		}*/
	}



	//--------------------------------------------------------------------------------------
	// Draws all clipmap levels in visualization mode
	//--------------------------------------------------------------------------------------
	void Terrain::RenderDebug(MeshObject& boxMesh, Effect& effect)
	{
		// Set buffers for the patches
		/*UINT offset=0;
		g_pd3dDevice->IASetVertexBuffers( 0, 1, &m_BlockVertexBuffer, &Vertex::size, &offset );
		g_pd3dDevice->IASetIndexBuffer( m_BlockIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

		// Used for offsetting the building blocks
		D3DXVECTOR2 gridOffset;

		// Discrete camera location in the grid
		D3DXVECTOR3 camPos(floorf(m_pCamera->GetPos().x), 0, floorf(m_pCamera->GetPos().z));


		// Patch size
		float size = 1;

		// The finest level is different
		{
			// Bind the clipmaps as textures
			effect.ClipmapVariable->SetResource(m_Clipmaps.GetSRV()[0]);

			// Set the patch size
			effect.ClipmapScaleVariable->SetFloat(size);

			effect.ClipmapFixVariable->SetFloatVector((float*)&m_ClipmapOffsets[0]);

			// Offset
			D3DXVECTOR2 offsetTrim;
			int modX = (int)floorf(m_pCamera->GetPos().x) % 2;
			int modY = (int)floorf(m_pCamera->GetPos().z) % 2;
			modX += modX < 0 ? 2 : 0;
			modY += modY < 0 ? 2 : 0;
			offsetTrim.x = -(2 - modX);
			offsetTrim.y = -(2 - modY);


			//------------
			// Draw the blocks

			// Set buffers
			g_pd3dDevice->IASetVertexBuffers( 0, 1, &m_BlockVertexBuffer, &Vertex::size, &offset );
			g_pd3dDevice->IASetIndexBuffer( m_BlockIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

			// Block positions
			D3DXVECTOR2 offsets[4];
			offsets[0] = D3DXVECTOR2(1, 1);
			offsets[1] = D3DXVECTOR2(1-(m_BlockSize-1), 1);
			offsets[2] = D3DXVECTOR2(1, -(m_BlockSize-1)+1);
			offsets[3] = D3DXVECTOR2(1-(m_BlockSize-1), -(m_BlockSize-1)+1);

			// 4 blocks
			for(int i=0; i<4; i++)
			{
				// Draw the bounding box
				gridOffset = offsetTrim + D3DXVECTOR2(offsets[i].x, offsets[i].y + (m_BlockSize-1)-1);
				boxMesh.SetPos(camPos+D3DXVECTOR3(gridOffset.x + m_BlockSize*size*0.5f, 1.0f, gridOffset.y - m_BlockSize*size*0.5f));
				boxMesh.SetScale(D3DXVECTOR3(m_BlockSize*size, 1.0f, m_BlockSize*size));
				if(m_pCamera->GetFrustum().CheckRectangle(boxMesh.GetPos(), boxMesh.GetBoundSize()))
					//if( boxMesh.GetPos().x-boxMesh.GetBoundSize().x*0.5f <= m_Size/2 && boxMesh.GetPos().x+boxMesh.GetBoundSize().x*0.5f >= -m_Size/2 &&
//						boxMesh.GetPos().y-boxMesh.GetBoundSize().y*0.5f <= m_Size/2 && boxMesh.GetPos().y+boxMesh.GetBoundSize().y*0.5f >= -m_Size/2)
				{
					effect.WorldMatrixVariable->SetMatrix((float*)&boxMesh.GetWorldMatrix());
					effect.Pass[PASS_WIREFRAME]->Apply(0);
					boxMesh.GetMesh()->Render();
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

			// Set the offset
			effect.ClipmapFixVariable->SetFloatVector((float*)&m_ClipmapOffsets[level]);

			// Set buffers for the blocks
			g_pd3dDevice->IASetVertexBuffers( 0, 1, &m_BlockVertexBuffer, &Vertex::size, &offset );
			g_pd3dDevice->IASetIndexBuffer( m_BlockIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );

			// Draw the blocks
			for(int i=0; i<12; i++)
			{
				// Set the patch offset position
				gridOffset = D3DXVECTOR2(size*m_BlockOffsets[i].x-2*size, size*m_BlockOffsets[i].y+(size-1)*(m_BlockSize-1) + (m_BlockSize-1));

				// Draw the bounding box
				boxMesh.SetPos(camPos+D3DXVECTOR3(gridOffset.x + m_BlockSize*size*0.5f, 1.0f, gridOffset.y - m_BlockSize*size*0.5f));
				boxMesh.SetScale(D3DXVECTOR3(m_BlockSize*size, 1.0f, m_BlockSize*size));
				D3DXVECTOR3 cullPos = camPos+D3DXVECTOR3(gridOffset.x + m_BlockSize*size*0.5f, 1.0f, gridOffset.y - m_BlockSize*size*0.5f);
				D3DXVECTOR3 cullBounds = D3DXVECTOR3(m_BlockSize*size, 1.0f, m_BlockSize*size)*0.5f;
				if(m_pCamera->GetFrustum().CheckRectangle(boxMesh.GetPos(), boxMesh.GetBoundSize()) && 
				  !( cullPos.x-cullBounds.x > m_Size/2 || cullPos.x+cullBounds.x < -m_Size/2 ||
					 cullPos.z-cullBounds.z > m_Size/2 || cullPos.z+cullBounds.z < -m_Size/2 ) )
				{
					effect.WorldMatrixVariable->SetMatrix((float*)&boxMesh.GetWorldMatrix());
					effect.Pass[PASS_WIREFRAME]->Apply(0);
					boxMesh.GetMesh()->Render();
				}
			}


			const float h2 = m_HeightScale*0.5f;
			const float b2 = m_BlockSize*0.5f;
			float b2s = b2*size;
			// Draw the ring fix strips
			{
				// Set buffers for the ring fix Mx3
				g_pd3dDevice->IASetVertexBuffers( 0, 1, &m_RingfixVertexBuffer[0], &Vertex::size, &offset );
				g_pd3dDevice->IASetIndexBuffer( m_RingfixIndexBuffer[0], DXGI_FORMAT_R16_UINT, 0 );

				// Left
				gridOffset = D3DXVECTOR2(-2*size*(m_BlockSize-1)-2*size, 0);
				// Draw the bounding box
				boxMesh.SetPos(camPos+D3DXVECTOR3(gridOffset.x + m_BlockSize*size*0.5f, 1.0f, gridOffset.y));
				boxMesh.SetScale(D3DXVECTOR3(m_BlockSize*size, 1.0f, 6*size));
				effect.MaterialDiffuseVariable->SetFloatVector((float*)&D3DXVECTOR4(0, 1, 1, 1));
				D3DXVECTOR3 blockBounds = D3DXVECTOR3(m_BlockSize*size, h2, 6*size);
				D3DXVECTOR3 cullBounds = blockBounds*0.5f;
				D3DXVECTOR3 cullPos = camPos+D3DXVECTOR3(gridOffset.x + b2s, h2, gridOffset.y);
				if(m_pCamera->GetFrustum().CheckRectangle(cullPos, blockBounds) &&
					!( cullPos.x-cullBounds.x > m_Size/2 || cullPos.x+cullBounds.x < -m_Size/2 ||
					   cullPos.z-cullBounds.z > m_Size/2 || cullPos.z+cullBounds.z < -m_Size/2 ))
				{
					effect.WorldMatrixVariable->SetMatrix((float*)&boxMesh.GetWorldMatrix());
					effect.Pass[PASS_WIREFRAME]->Apply(0);
					boxMesh.GetMesh()->Render();
				}

				// Right
				gridOffset = D3DXVECTOR2(size*(m_BlockSize-1), 0);
				// Draw the bounding box
				boxMesh.SetPos(camPos+D3DXVECTOR3(gridOffset.x + m_BlockSize*size*0.5f, 1.0f, gridOffset.y));
				boxMesh.SetScale(D3DXVECTOR3(m_BlockSize*size, 1.0f, 6*size));
				cullPos = camPos+D3DXVECTOR3(gridOffset.x +b2s, h2, gridOffset.y);
				if(m_pCamera->GetFrustum().CheckRectangle(cullPos, blockBounds) &&
					!( cullPos.x-cullBounds.x > m_Size/2 || cullPos.x+cullBounds.x < -m_Size/2 ||
					   cullPos.z-cullBounds.z > m_Size/2 || cullPos.z+cullBounds.z < -m_Size/2 ))
				{
					effect.WorldMatrixVariable->SetMatrix((float*)&boxMesh.GetWorldMatrix());
					effect.Pass[PASS_WIREFRAME]->Apply(0);
					boxMesh.GetMesh()->Render();
				}

				// Set buffers for the ring fix 3xM
				g_pd3dDevice->IASetVertexBuffers( 0, 1, &m_RingfixVertexBuffer[1], &Vertex::size, &offset );
				g_pd3dDevice->IASetIndexBuffer( m_RingfixIndexBuffer[1], DXGI_FORMAT_R16_UINT, 0 );

				// Top
				gridOffset = D3DXVECTOR2(-2*size, 2*size*(m_BlockSize-1));
				// Draw the bounding box
				boxMesh.SetPos(camPos+D3DXVECTOR3(gridOffset.x+1.5f*size, 1.0f, gridOffset.y - m_BlockSize*size*0.5f));
				boxMesh.SetScale(D3DXVECTOR3(6*size, 1.0f, m_BlockSize*size));
				blockBounds = D3DXVECTOR3(6*size, h2, m_BlockSize*size);
				cullBounds = blockBounds*0.5f;
				cullPos = camPos+D3DXVECTOR3(gridOffset.x+1.5f*size, h2, gridOffset.y - b2s);
				if(m_pCamera->GetFrustum().CheckRectangle(cullPos, blockBounds) &&
					!( cullPos.x-cullBounds.x > m_Size/2 || cullPos.x+cullBounds.x < -m_Size/2 ||
					   cullPos.z-cullBounds.z > m_Size/2 || cullPos.z+cullBounds.z < -m_Size/2 ))
				{
					effect.WorldMatrixVariable->SetMatrix((float*)&boxMesh.GetWorldMatrix());
					effect.Pass[PASS_WIREFRAME]->Apply(0);
					boxMesh.GetMesh()->Render();
				}

				// Bottom
				gridOffset = D3DXVECTOR2(-2*size, -2*size-size*(m_BlockSize-1));
				// Draw the bounding box
				boxMesh.SetPos(camPos+D3DXVECTOR3(gridOffset.x+1.5f*size, 1.0f, gridOffset.y - m_BlockSize*size*0.5f));
				boxMesh.SetScale(D3DXVECTOR3(6*size, 1.0f, m_BlockSize*size));
				cullPos = camPos+D3DXVECTOR3(gridOffset.x+1.5f*size, h2, gridOffset.y - b2s);
				if(m_pCamera->GetFrustum().CheckRectangle(cullPos, blockBounds) &&
					!( cullPos.x-cullBounds.x > m_Size/2 || cullPos.x+cullBounds.x < -m_Size/2 ||
					   cullPos.z-cullBounds.z > m_Size/2 || cullPos.z+cullBounds.z < -m_Size/2 ))
				{
					effect.WorldMatrixVariable->SetMatrix((float*)&boxMesh.GetWorldMatrix());
					effect.Pass[PASS_WIREFRAME]->Apply(0);
					boxMesh.GetMesh()->Render();
				}
			}
			effect.MaterialDiffuseVariable->SetFloatVector((float*)&D3DXVECTOR4(1, 1, 1, 1));

		}*/
	}

	#endif
}

