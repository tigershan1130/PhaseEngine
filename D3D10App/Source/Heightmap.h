//-----------------------------------------------------------------------------
// File: Heightmap.h
//
// Desc: Support for generating terrain heightmaps
//-----------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "QMath.h"

namespace Core
{


	//-----------------------------------------------------------------------------
	// Name: GenerateLiquidHeightmap()
	// Desc: Genterates a heightmap based on fluid simulation
	// **MODIFIED FROM Francis Woodhouse's ARTICLE ON GAMEDEV.NET**
	// http://www.gamedev.net/reference/articles/article2001.asp
	//-----------------------------------------------------------------------------
	unsigned char* GenerateLiquidHeightmap(int w, int h, int num,float d,float t,float mu,float c)
	{
		// The equation coefficients
		float coefA, coefB, coefC;
		
		// Setup the surface
		float* pSurface[2];
		pSurface[0] = new float[w*h];
		pSurface[1] = new float[w*h];

		// Compute the coef values
		coefA = (4 - (8*c*c*t*t) / (d*d)) / (mu*t + 2);
		coefB = (mu*t - 2) / (mu*t + 2);
		coefC = ((2*c*c*t*t) / (d*d)) / (mu*t + 2);

		// Initialize the heights to random values
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				if (x == 0 || x == (w-1) || y == 0 || y == (h-1))
					pSurface[0][x+y*w] = pSurface[1][x+y*w] = 0;
				else
					pSurface[0][x+y*w] = pSurface[1][x+y*w] = ((float)rand() / RAND_MAX) * (500 - -500) + -500;
			}
		}

		int iCurBuf = 0;
		int iInd;

		float* pOld, * pNew;

		// Iterate over the heightmap, applying the fluid simulation equation.
		// Although it requires knowledge of the two previous timesteps, it only
		// accesses one pixel of the k-1 timestep, so using a simple trick we only
		// need to store the heightmap twice, not three times, and we can avoid
		// a memcpy() every iteration.
		for (int i = 0; i < num; i++) {
			pOld = pSurface[1-iCurBuf];
			pNew = pSurface[iCurBuf];

			for (int y = 1; y < h-1; y++) {
				for (int x = 1; x < w-1; x++) {
					iInd = x+y*w;

					pOld[iInd] = coefA*pNew[iInd] + coefB*pOld[iInd] +
								 coefC*(pNew[iInd+1] + pNew[iInd-1] + pNew[iInd+w] + pNew[iInd-w]);
				}
			}

			iCurBuf = 1-iCurBuf;
		}

		float fMinH=pSurface[iCurBuf][0], fMaxH=pSurface[iCurBuf][0];

		// find the minimum and maximum heights
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++) {
				if (pSurface[iCurBuf][x+y*w] > fMaxH)
					fMaxH = pSurface[iCurBuf][x+y*w];
				else if (pSurface[iCurBuf][x+y*w] < fMinH)
					fMinH = pSurface[iCurBuf][x+y*w];
			}
		}

		// normalize the surface
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++)
				pSurface[iCurBuf][x+y*w] = (pSurface[iCurBuf][x+y*w] - fMinH) / (fMaxH - fMinH);
		}

		// allocate memory for the greyscale image
		unsigned char* pImg = new unsigned char[w*h];

		// put the normalized heightmap into the range [0...255] and into the greyscale image
		for (int y = 0; y < h; y++) {
			for (int x = 0; x < w; x++)
				pImg[x+y*w] = (unsigned char)(pSurface[iCurBuf][x+y*w] * 255);
		}


		// deallocate memory
		delete[] pSurface[0];
		delete[] pSurface[1];

		return pImg;
	}


	//-----------------------------------------------------------------------------
	// Name: GenerateNormalMap()
	// Desc: Genterates a normal map based on a height map
	//-----------------------------------------------------------------------------
	D3DXVECTOR3* GenerateNormalMap(float* pHeight, int w, int h, float hs)
	{
		// Allocate the normal map and vertex weight arrays
		D3DXVECTOR3* pNormal = new D3DXVECTOR3[w*h];
		int* num = new int[w*h];
		for(int i=0; i<w+h; i++)
		{
			num[i] = 0;
			pNormal[i] = D3DXVECTOR3();
		}

		// Vars
		D3DXVECTOR3 tri[3];
		D3DXVECTOR3 normal;

		// For each quad in the heightmap get the average normal for each vertex
		for(int x=0; x<w-1; x++)
			for(int y=0; y<h-1; y++)
			{
				// Get the normal from the first tri in the quad
				tri[0] = D3DXVECTOR3(x*hs,pHeight[y*w+x],y*hs);
				tri[1] = D3DXVECTOR3((x+1)*hs,pHeight[y*w+x+1],y*hs);
				tri[2] = D3DXVECTOR3((x+1)*hs,pHeight[(y+1)*w+x+1],(y+1)*hs);
				Math::GetNormalFromTri(tri,normal);
				pNormal[y*w+x]+=normal;
				num[y*w+x]++;
				pNormal[y*w+x+1]+=normal;
				num[y*w+x+1]++;
				pNormal[(y+1)*w+x+1]+=normal;
				num[(y+1)*w+x+1]++;

				// Get the normal from the second quad
				tri[0] = D3DXVECTOR3((x+1)*hs,pHeight[(y+1)*w+x+1],(y+1)*hs);
				tri[1] = D3DXVECTOR3(x*hs,pHeight[(y+1)*w+x],(y+1)*hs);
				tri[2] = D3DXVECTOR3(x*hs,pHeight[y*w+x],y*hs);
				Math::GetNormalFromTri(tri,normal);
				pNormal[(y+1)*w+x+1]+=normal;
				num[(y+1)*w+x+1]++;
				pNormal[(y+1)*w+x]+=normal;
				num[(y+1)*w+x]++;
				pNormal[y*w+x]+=normal;
				num[y*w+x]++;
			}

		// Divide the normals my the vertex weight
		for(int i=0; i<w+h; i++)
		{
			normal = pNormal[i] / (float)num[i];
			D3DXVec3Normalize(&pNormal[i],&normal);
		}
		SAFE_DELETE_ARRAY(num);

		return pNormal;
	}


	//-----------------------------------------------------------------------------
	// Name: GenerateNormalMap()
	// Desc: Genterates a normal map based on a height map
	//-----------------------------------------------------------------------------
	D3DXVECTOR3* GenerateNormalMap(D3DXFLOAT16* pHeight, int w, int h, float hs)
	{
		// Allocate the normal map and vertex weight arrays
		D3DXVECTOR3* pNormal = new D3DXVECTOR3[w*h];
		int* num = new int[w*h];
		for(int i=0; i<w+h; i++)
		{
			num[i] = 0;
			pNormal[i] = D3DXVECTOR3();
		}

		// Vars
		D3DXVECTOR3 tri[3];
		D3DXVECTOR3 normal;

		// For each quad in the heightmap get the average normal for each vertex
		for(int x=0; x<w-1; x++)
			for(int y=0; y<h-1; y++)
			{
				// Get the normal from the first tri in the quad
				tri[0] = D3DXVECTOR3(x*hs,pHeight[y*w+x],y*hs);
				tri[1] = D3DXVECTOR3((x+1)*hs,pHeight[y*w+x+1],y*hs);
				tri[2] = D3DXVECTOR3((x+1)*hs,pHeight[(y+1)*w+x+1],(y+1)*hs);
				Math::GetNormalFromTri(tri,normal);
				pNormal[y*w+x]+=normal;
				num[y*w+x]++;
				pNormal[y*w+x+1]+=normal;
				num[y*w+x+1]++;
				pNormal[(y+1)*w+x+1]+=normal;
				num[(y+1)*w+x+1]++;

				// Get the normal from the second quad
				tri[0] = D3DXVECTOR3((x+1)*hs,pHeight[(y+1)*w+x+1],(y+1)*hs);
				tri[1] = D3DXVECTOR3(x*hs,pHeight[(y+1)*w+x],(y+1)*hs);
				tri[2] = D3DXVECTOR3(x*hs,pHeight[y*w+x],y*hs);
				Math::GetNormalFromTri(tri,normal);
				pNormal[(y+1)*w+x+1]+=normal;
				num[(y+1)*w+x+1]++;
				pNormal[(y+1)*w+x]+=normal;
				num[(y+1)*w+x]++;
				pNormal[y*w+x]+=normal;
				num[y*w+x]++;
			}

			// Divide the normals my the vertex weight
			for(int i=0; i<w+h; i++)
			{
				normal = pNormal[i] / (float)num[i];
				D3DXVec3Normalize(&pNormal[i],&normal);
			}
			SAFE_DELETE_ARRAY(num);

			return pNormal;
	}

}