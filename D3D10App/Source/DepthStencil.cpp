//--------------------------------------------------------------------------------------
// File: DepthStencil.cpp
//
// Depth stencil texture class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "DepthStencil.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	DepthStencil::DepthStencil()
	{
		m_pTex = NULL;
		m_pSRV = NULL;
		m_pDSV = NULL;
		m_pEffectSRV = NULL;
	}

	//--------------------------------------------------------------------------------------
	// Create the depth targets
	//--------------------------------------------------------------------------------------
	HRESULT DepthStencil::Create(UINT width, UINT height, DXGI_SAMPLE_DESC* pMSAA, bool asTexture)
	{
		HRESULT hr;
	
		// MSAA
		UINT samples = 1;
		UINT quality = 0;
		if(pMSAA)
		{
			samples = pMSAA->Count;
			quality = pMSAA->Quality;
		}

		// Format
		DXGI_FORMAT format[3];
		format[0] = DXGI_FORMAT_R32_TYPELESS;
		format[1] = DXGI_FORMAT_D32_FLOAT;
		format[2] = DXGI_FORMAT_R32_FLOAT;

		// Create depth stencil texture
		D3D10_TEXTURE2D_DESC dstex;
		dstex.ArraySize = 1;
		dstex.Width = width;
		dstex.Height = height;
		dstex.MipLevels = 1;
		dstex.Format = format[0];
		dstex.SampleDesc.Count = samples;
		dstex.SampleDesc.Quality = quality;
		dstex.Usage = D3D10_USAGE_DEFAULT;
		if(!asTexture)
			dstex.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		else
			dstex.BindFlags = D3D10_BIND_DEPTH_STENCIL | D3D10_BIND_SHADER_RESOURCE;
		dstex.CPUAccessFlags = 0;
		dstex.MiscFlags = 0;
		V_RETURN(g_pd3dDevice->CreateTexture2D( &dstex, NULL, &m_pTex ));

		// Create the depth stencil view
		D3D10_DEPTH_STENCIL_VIEW_DESC DescDS;
		DescDS.Format = format[1];
		if(pMSAA)
			DescDS.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DMS;
		else
		{
			DescDS.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
			DescDS.Texture2D.MipSlice=0;
		}
		V_RETURN(g_pd3dDevice->CreateDepthStencilView( m_pTex, &DescDS, &m_pDSV ));

		// Create the shader resource view
		if(asTexture)
		{
			D3D10_SHADER_RESOURCE_VIEW_DESC dssrv;
			dssrv.Format = format[2];
			if(pMSAA)
				dssrv.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DMS;
			else
			{
				dssrv.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
				dssrv.Texture2D.MipLevels = 1;
				dssrv.Texture2D.MostDetailedMip = 0;
			}
			V_RETURN(g_pd3dDevice->CreateShaderResourceView(m_pTex, &dssrv, &m_pSRV));
		}
		
		return S_OK;
	}

	//--------------------------------------------------------------------------------------
	// Create a cube-map depth stencil
	//--------------------------------------------------------------------------------------
	HRESULT DepthStencil::CreateCube(UINT width, UINT height)
	{
		HRESULT hr;

		// Create cubic depth stencil texture.
		D3D10_TEXTURE2D_DESC dstex;
		ZeroMemory( &dstex, sizeof(dstex) );
		dstex.Width = width;
		dstex.Height = height;
		dstex.MipLevels = 1;
		dstex.ArraySize = 6;
		dstex.SampleDesc.Count = 1;
		dstex.SampleDesc.Quality = 0;
		dstex.Format = DXGI_FORMAT_R32_TYPELESS;
		dstex.Usage = D3D10_USAGE_DEFAULT;
		dstex.BindFlags = D3D10_BIND_DEPTH_STENCIL;
		dstex.CPUAccessFlags = 0;
		dstex.MiscFlags = D3D10_RESOURCE_MISC_TEXTURECUBE;
		V_RETURN( g_pd3dDevice->CreateTexture2D( &dstex, NULL, &m_pTex ));

		// Create the depth stencil view for single face rendering
		D3D10_DEPTH_STENCIL_VIEW_DESC DescDS;
		ZeroMemory( &DescDS, sizeof(DescDS) );
		DescDS.Format = DXGI_FORMAT_D32_FLOAT;
		DescDS.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2DARRAY;
		DescDS.Texture2DArray.FirstArraySlice = 0;
		DescDS.Texture2DArray.ArraySize = 1;
		DescDS.Texture2DArray.MipSlice = 0;
		V_RETURN( g_pd3dDevice->CreateDepthStencilView( m_pTex, &DescDS, &m_pDSV ));
		
		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	// Free resources
	//--------------------------------------------------------------------------------------
	void DepthStencil::Release()
	{
		SAFE_RELEASE(m_pTex);
		SAFE_RELEASE(m_pSRV);
		SAFE_RELEASE(m_pDSV);
		m_pEffectSRV = NULL;
	}

}