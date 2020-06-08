//--------------------------------------------------------------------------------------
// File: Water.cpp
//
// Implementation of "planar Water reflection/refraction"
//
// Coded By Shan WenQin, 2009.
// Special Thanks to Nate Orr.
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "Water.h"
#include "Device.h"
#include <vector>
namespace Core
{
	
	Water::Water()
	{

	}

	void Water::RenderReflectionMap(Light& light) // for now we will exclude light.
	{

	}
	//--------------------------------------------------------------------------------------
	// Creates the water grid
	//--------------------------------------------------------------------------------------
	HRESULT Water::initWater(Effect* pEffect, int m, int n, float dx, float waterheight)
	{
		m_pEffect = pEffect;

		Log::Print("Building Water plane");

		WaterHeight = waterheight;

		NUM_ROWS = m-1;
		NUM_COLS = n-1;

		NUM_VERTSX = m;
		NUM_VERTSY = n;

		mNumVertices = m*n;
		mNumFaces    = (m-1)*(n-1)*2;

		DuDvTexture.Load("Textures\\dudvmap.bmp");
		NormalTexture.Load("Textures\\waterNormal.bmp");


		std::vector<Vertex> vertices(m*n);
		float halfWidth = (n-1)*dx*0.5f;
		float halfDepth = (m-1)*dx*0.5f;

		float du = 1.0f / (n-1);
		float dv = 1.0f / (m-1);
		for(int i = 0; i < m; ++i)
		{
			float z = halfDepth - i*dx;
			for(int j = 0; j < n; ++j)
			{
				float x = -halfWidth + j*dx;

				vertices[i*n+j].pos     = D3DXVECTOR3(x, WaterHeight, z);
				vertices[i*n+j].normal  = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

				// Stretch texture over grid.
				vertices[i*n+j].tu = j*du;
				vertices[i*n+j].tv = i*dv;
			}
		}
 
		D3D10_BUFFER_DESC vbd;
		vbd.Usage = D3D10_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex) * mNumVertices;
		vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &vertices[0];
		g_pd3dDevice->CreateBuffer(&vbd, &vinitData, &mVB);


		// Create the index buffer. 

		std::vector<DWORD> indices(mNumFaces*3); // 3 indices per face

		// Iterate over each quad and compute indices.
		int k = 0;
		for(int i = 0; i < m-1; ++i)
		{
			for(int j = 0; j < n-1; ++j)
			{
				indices[k]   = i*n+j;
				indices[k+1] = i*n+j+1;
				indices[k+2] = (i+1)*n+j;

				indices[k+3] = (i+1)*n+j;
				indices[k+4] = i*n+j+1;
				indices[k+5] = (i+1)*n+j+1;

				k += 6; // next quad
			}
		}	

		D3D10_BUFFER_DESC ibd;
		ibd.Usage = D3D10_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(DWORD) * mNumFaces*3;
		ibd.BindFlags = D3D10_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &indices[0];
	    g_pd3dDevice->CreateBuffer(&ibd, &iinitData, &mIB);



		return S_OK;
	}

	void Water::Release()
	{
		WaterReflectionMap.Release();
		WaterReflectionMapDepth.Release();
		WaterRefractionMap.Release();
		WaterRefractionMapDepth.Release();
		DuDvTexture.Release();
		NormalTexture.Release();

	}


	void Water::draw()
	{

	// Render a model object
    //D3D10_TECHNIQUE_DESC techniqueDescription;
    //fxTech->GetDesc(&techniqueDescription);

	// Loop through the technique passes
    //for(UINT p=0; p < techniqueDescription.Passes; ++p)
    //{


		m_pEffect->fxWaterHeightVar->SetRawValue(&WaterHeight, 0, sizeof(float));
		m_pEffect->DuDvMapVariable->SetResource( DuDvTexture.GetResource() );
		m_pEffect->NormalMapVariable->SetResource( NormalTexture.GetResource());
		

		D3DXMATRIX mWorld;
		D3DXMatrixTranslation(&mWorld, 0, WaterHeight, 0);

		m_pEffect->WorldMatrixVariable->SetMatrix((float*)&mWorld);

		m_pEffect->Pass[PASS_DRAW_WATER]->Apply(0);

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		g_pd3dDevice->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);
		g_pd3dDevice->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);
		g_pd3dDevice->DrawIndexed(mNumFaces*3, 0, 0);
   // }


	}


}