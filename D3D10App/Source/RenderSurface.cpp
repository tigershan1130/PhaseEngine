//--------------------------------------------------------------------------------------
// File: RenderSurface.cpp
//
// Render target texture class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "RenderSurface.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	RenderSurface::RenderSurface()
	{
		m_pTex = NULL;
		m_pTexRTV = NULL;
		m_pTexSRV = NULL;
		m_pEffectSRV = NULL;
		m_ArraySize = 0;
		m_pDSV = NULL;
		m_bCube = false;
		m_ClearColor[0] = m_ClearColor[1] = m_ClearColor[2] = m_ClearColor[3] = 0.0f;
	}

	//--------------------------------------------------------------------------------------
	// Create a 2D render target from a texture
	//--------------------------------------------------------------------------------------
	HRESULT RenderSurface::CreateFromTexture(ID3D10Texture2D* pTex)
	{
		// Allocate the arrays
		HRESULT hr;
		m_ArraySize = 1;
		m_pTex = new ID3D10Texture2D*[m_ArraySize];
		m_pTexRTV = new ID3D10RenderTargetView*[m_ArraySize];
		m_pTexSRV = new ID3D10ShaderResourceView*[m_ArraySize];
		m_bCube = false;

		// Misc flags
		UINT mipLevels = 1;

		// Get the texture info
		D3D10_TEXTURE2D_DESC td;
		pTex->GetDesc(&td);

		// Setup the RTV desc
		D3D10_RENDER_TARGET_VIEW_DESC dsrtv;
		dsrtv.Format = td.Format;
		dsrtv.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
		dsrtv.Texture2D.MipSlice = 0;

		// Setup the SRV desc
		D3D10_SHADER_RESOURCE_VIEW_DESC dssrv;
		dssrv.Format = td.Format;
		dssrv.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
		dssrv.Texture2D.MipLevels = mipLevels;
		dssrv.Texture2D.MostDetailedMip = 0;

		// Create the resources
		m_pTex[0] = pTex;
		V_RETURN(g_pd3dDevice->CreateRenderTargetView(m_pTex[0], &dsrtv, &m_pTexRTV[0]));
		V_RETURN(g_pd3dDevice->CreateShaderResourceView(m_pTex[0], &dssrv, &m_pTexSRV[0]));

		// Setup the viewport
		m_Viewport.Height = td.Height;
		m_Viewport.Width = td.Width;
		m_Viewport.MinDepth = 0;
		m_Viewport.MaxDepth = 1.0f;
		m_Viewport.TopLeftX = 0;
		m_Viewport.TopLeftY = 0;

		// Create an orthoganal and scaled projection matrix for
		// screen sized quad rendering
		D3DXMATRIXA16 mScaleScreen,mOrtho2D;
		D3DXMatrixScaling(&mScaleScreen, (float)td.Width/2.0f, (float)td.Height/2.0f, 0);	
		D3DXMatrixOrthoLH(&mOrtho2D, (float)td.Width, (float)td.Height, 0.0f, 1.0f);
		m_mOrtho = mScaleScreen * mOrtho2D;

		return S_OK;
	}

	//--------------------------------------------------------------------------------------
	// Create a 2D render target
	//--------------------------------------------------------------------------------------
	HRESULT RenderSurface::Create(UINT width, UINT height, DXGI_FORMAT format, UINT mipLevels, DXGI_SAMPLE_DESC* pMSAA, bool bRTV)
	{
		// Allocate the arrays
		HRESULT hr;
		m_ArraySize = 1;
		m_pTex = new ID3D10Texture2D*[m_ArraySize];
		if(bRTV)
			m_pTexRTV = new ID3D10RenderTargetView*[m_ArraySize];
		m_pTexSRV = new ID3D10ShaderResourceView*[m_ArraySize];
		m_bCube = false;

		// Misc flags
		UINT flags = 0;
		if(mipLevels>1)
			flags = D3D10_RESOURCE_MISC_GENERATE_MIPS;
		if(mipLevels==0) mipLevels++;

		// Setup the texture desc
		D3D10_TEXTURE2D_DESC dstex;
		dstex.ArraySize = 1;
		dstex.MipLevels = mipLevels;
		dstex.Width = width;
		dstex.Height = height;
		dstex.Format = format;
		dstex.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
		dstex.CPUAccessFlags = 0;
		dstex.MiscFlags = flags;	
		dstex.SampleDesc.Count = 1;
		dstex.SampleDesc.Quality = 0;
		if(pMSAA)
		{
			dstex.SampleDesc.Count = pMSAA->Count;
			dstex.SampleDesc.Quality = pMSAA->Quality;
		}
		dstex.Usage = D3D10_USAGE_DEFAULT;


		// Setup the RTV desc
		D3D10_RENDER_TARGET_VIEW_DESC dsrtv;
		dsrtv.Format = dstex.Format;
		if(pMSAA)
			dsrtv.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DMS;
		else
		{
			dsrtv.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
			dsrtv.Texture2D.MipSlice = 0;
		}

		// Setup the SRV desc
		D3D10_SHADER_RESOURCE_VIEW_DESC dssrv;
		dssrv.Format = dstex.Format;
		if(pMSAA)
			dssrv.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DMS;
		else
		{
			dssrv.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
			dssrv.Texture2D.MipLevels = mipLevels;
			dssrv.Texture2D.MostDetailedMip = 0;
		}

		// Create the resources
		for(UINT i=0; i<1; i++)
		{
			V_RETURN(g_pd3dDevice->CreateTexture2D(&dstex, NULL, &m_pTex[i]));
			if(bRTV)
				V_RETURN(g_pd3dDevice->CreateRenderTargetView(m_pTex[i], &dsrtv, &m_pTexRTV[i]));
			V_RETURN(g_pd3dDevice->CreateShaderResourceView(m_pTex[i], &dssrv, &m_pTexSRV[i]));
		}

		// Setup the viewport
		m_Viewport.Height = height;
		m_Viewport.Width = width;
		m_Viewport.MinDepth = 0;
		m_Viewport.MaxDepth = 1.0f;
		m_Viewport.TopLeftX = 0;
		m_Viewport.TopLeftY = 0;

		// Create an orthoganal and scaled projection matrix for
		// screen sized quad rendering
		D3DXMATRIXA16 mScaleScreen,mOrtho2D;
		D3DXMatrixScaling(&mScaleScreen, (float)width/2.0f, (float)height/2.0f, 0);	
		D3DXMatrixOrthoLH(&mOrtho2D, (float)width, (float)height, 0.0f, 1.0f);
		m_mOrtho = mScaleScreen * mOrtho2D;

		return S_OK;
	}

	//--------------------------------------------------------------------------------------
	// Create a 2D render target array
	//--------------------------------------------------------------------------------------
	HRESULT RenderSurface::CreateArray(UINT width, UINT height, DXGI_FORMAT* format, UINT numTargets, UINT mipLevels, DXGI_SAMPLE_DESC* pMSAA, bool bRTV)
	{
		// Allocate the arrays
		HRESULT hr;
		m_ArraySize = numTargets;
		if(m_ArraySize==0)
			m_ArraySize=1;
		m_pTex = new ID3D10Texture2D*[m_ArraySize];
		if(bRTV)
			m_pTexRTV = new ID3D10RenderTargetView*[m_ArraySize];
		m_pTexSRV = new ID3D10ShaderResourceView*[m_ArraySize];
		m_bCube = false;

		// Misc flags
		UINT flags = 0;
		if(mipLevels>1)
			flags = D3D10_RESOURCE_MISC_GENERATE_MIPS;
		if(mipLevels==0) mipLevels++;

		// Setup the texture desc
		D3D10_TEXTURE2D_DESC dstex;
		dstex.ArraySize = 1;
		dstex.MipLevels = mipLevels;
		dstex.Width = width;
		dstex.Height = height;	
		dstex.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
		dstex.CPUAccessFlags = 0;
		dstex.MiscFlags = flags;	
		dstex.SampleDesc.Count = 1;
		dstex.SampleDesc.Quality = 0;
		if(pMSAA)
		{
			dstex.SampleDesc.Count = pMSAA->Count;
			dstex.SampleDesc.Quality = pMSAA->Quality;
		}
		dstex.Usage = D3D10_USAGE_DEFAULT;


		// Setup the RTV desc
		D3D10_RENDER_TARGET_VIEW_DESC dsrtv;
		if(pMSAA)
			dsrtv.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DMS;
		else
		{
			dsrtv.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2D;
			dsrtv.Texture2D.MipSlice = 0;
		}

		// Setup the SRV desc
		D3D10_SHADER_RESOURCE_VIEW_DESC dssrv;
		if(pMSAA)
			dssrv.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2DMS;
		else
		{
			dssrv.ViewDimension = D3D10_SRV_DIMENSION_TEXTURE2D;
			dssrv.Texture2D.MipLevels = mipLevels;
			dssrv.Texture2D.MostDetailedMip = 0;
		}

		// Create the resources
		for(UINT i=0; i<numTargets; i++)
		{
			dstex.Format = dsrtv.Format = dssrv.Format = format[i];
			V_RETURN(g_pd3dDevice->CreateTexture2D(&dstex, NULL, &m_pTex[i]));
			if(bRTV)
				V_RETURN(g_pd3dDevice->CreateRenderTargetView(m_pTex[i], &dsrtv, &m_pTexRTV[i]));
			V_RETURN(g_pd3dDevice->CreateShaderResourceView(m_pTex[i], &dssrv, &m_pTexSRV[i]));
		}

		// Setup the viewport
		m_Viewport.Height = dstex.Height;
		m_Viewport.Width = dstex.Width;
		m_Viewport.MinDepth = 0;
		m_Viewport.MaxDepth = 1.0f;
		m_Viewport.TopLeftX = 0;
		m_Viewport.TopLeftY = 0;

		// Create an orthoganal and scaled projection matrix for
		// screen sized quad rendering
		D3DXMATRIXA16 mScaleScreen,mOrtho2D;
		D3DXMatrixScaling(&mScaleScreen, (float)dstex.Width/2.0f, (float)dstex.Height/2.0f, 0);	
		D3DXMatrixOrthoLH(&mOrtho2D, (float)dstex.Width, (float)dstex.Height, 0.0f, 1.0f);
		m_mOrtho = mScaleScreen * mOrtho2D;

		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	// Create a cube map render target
	//--------------------------------------------------------------------------------------
	HRESULT RenderSurface::CreateCube(UINT width, UINT height, DXGI_FORMAT format, UINT mipLevels)
	{
		HRESULT hr;
		m_bArray = true;
		m_bCube = true;
		m_ArraySize = 6;
		m_pTex = new ID3D10Texture2D*[1];
		m_pTexRTV = new ID3D10RenderTargetView*[m_ArraySize];
		m_pTexSRV = new ID3D10ShaderResourceView*[1];

		// Create cube map texture
		D3D10_TEXTURE2D_DESC dstex;
		ZeroMemory( &dstex, sizeof(dstex) );
		dstex.Width = width;
		dstex.Height = height;
		dstex.MipLevels = mipLevels;
		dstex.ArraySize = 6;
		dstex.SampleDesc.Count = 1;
		dstex.SampleDesc.Quality = 0;
		dstex.Format = format;
		dstex.Usage = D3D10_USAGE_DEFAULT;
		dstex.BindFlags = D3D10_BIND_RENDER_TARGET | D3D10_BIND_SHADER_RESOURCE;
		dstex.CPUAccessFlags = 0;
		dstex.MiscFlags = D3D10_RESOURCE_MISC_GENERATE_MIPS | D3D10_RESOURCE_MISC_TEXTURECUBE;
		V_RETURN( g_pd3dDevice->CreateTexture2D( &dstex, NULL, &m_pTex[0] ));

		// Create the one-face render target views
		D3D10_RENDER_TARGET_VIEW_DESC DescRT;
		ZeroMemory( &DescRT, sizeof(DescRT) );
		DescRT.Format = dstex.Format;
		DescRT.ViewDimension = D3D10_RTV_DIMENSION_TEXTURE2DARRAY;
		DescRT.Texture2DArray.ArraySize = 1;
		DescRT.Texture2DArray.MipSlice = 0;
		for( int i = 0; i < 6; ++i )
		{
			DescRT.Texture2DArray.FirstArraySlice = i;
			V_RETURN( g_pd3dDevice->CreateRenderTargetView( m_pTex[0], &DescRT, &m_pTexRTV[i] ));
		}

		// Create the shader resource view for the cubic env map
		D3D10_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		ZeroMemory( &SRVDesc, sizeof(SRVDesc) );
		SRVDesc.Format = dstex.Format;
		SRVDesc.ViewDimension = D3D10_SRV_DIMENSION_TEXTURECUBE;
		SRVDesc.TextureCube.MipLevels = mipLevels;
		SRVDesc.TextureCube.MostDetailedMip = 0;
		V_RETURN( g_pd3dDevice->CreateShaderResourceView( m_pTex[0], &SRVDesc, &m_pTexSRV[0] ));

		// Setup the viewport
		m_Viewport.Height = dstex.Height;
		m_Viewport.Width = dstex.Width;
		m_Viewport.MinDepth = 0;
		m_Viewport.MaxDepth = 1.0f;
		m_Viewport.TopLeftX = 0;
		m_Viewport.TopLeftY = 0;

		// Create an orthoganal and scaled projection matrix for
		// screen sized quad rendering
		D3DXMATRIXA16 mScaleScreen,mOrtho2D;
		D3DXMatrixScaling(&mScaleScreen, (float)dstex.Width/2.0f, (float)dstex.Height/2.0f, 0);	
		D3DXMatrixOrthoLH(&mOrtho2D, (float)dstex.Width, (float)dstex.Height, 0.0f, 1.0f);
		m_mOrtho = mScaleScreen * mOrtho2D;

		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	// Resolve the MSAA target into a regular target
	//--------------------------------------------------------------------------------------
	void RenderSurface::Resolve(RenderSurface& target)
	{
		ID3D10Resource* pRT;
		target.GetRTV()[0]->GetResource( &pRT );
		D3D10_RENDER_TARGET_VIEW_DESC rtDesc;
		target.GetRTV()[0]->GetDesc( &rtDesc );
		g_pd3dDevice->ResolveSubresource( pRT, D3D10CalcSubresource( 0, 0, 1 ), m_pTex[0], D3D10CalcSubresource( 0, 0, 1 ), rtDesc.Format );
		SAFE_RELEASE( pRT );
	}

	//--------------------------------------------------------------------------------------
	// Bind the SRV to the effect
	//--------------------------------------------------------------------------------------
	void RenderSurface::BindTextures(UINT numResources)
	{ 
		// If an array bind the given number of slices
		if(IsArray() && !IsCube())
		{
			if(numResources==0)
				numResources = m_ArraySize;
			m_pEffectSRV->SetResourceArray(m_pTexSRV, 0, numResources);
		}
		else
			m_pEffectSRV->SetResource(m_pTexSRV[0]);
	}

	//--------------------------------------------------------------------------------------
	// Bind the render target
	//--------------------------------------------------------------------------------------
	void RenderSurface::BindRenderTarget(UINT numResources)
	{
		if(numResources==0)
			g_pd3dDevice->OMSetRenderTargets(m_ArraySize, m_pTexRTV, m_pDSV);
		else
		{
			ID3D10RenderTargetView* pRTV[1];
			pRTV[0] = m_pTexRTV[numResources];
			g_pd3dDevice->OMSetRenderTargets(1, pRTV, m_pDSV);
		}
	}

	//--------------------------------------------------------------------------------------
	// Set the clear color
	//--------------------------------------------------------------------------------------
	void RenderSurface::SetClearColor(float r, float g, float b, float a)
	{ 
		m_ClearColor[0] = r; 
		m_ClearColor[1] = g; 
		m_ClearColor[2] = b; 
		m_ClearColor[3] = a; 
	}


	//--------------------------------------------------------------------------------------
	// Clear the render targets
	//--------------------------------------------------------------------------------------
	void RenderSurface::Clear(UINT index)
	{
		if(index==-1)
		{
			for(UINT i=0; i<m_ArraySize; i++)
				g_pd3dDevice->ClearRenderTargetView(m_pTexRTV[i], m_ClearColor);
		}
		else
			g_pd3dDevice->ClearRenderTargetView(m_pTexRTV[index], m_ClearColor);
	}

	//--------------------------------------------------------------------------------------
	// Clear the Depth target
	//--------------------------------------------------------------------------------------
	void RenderSurface::ClearDSV()
	{
		g_pd3dDevice->ClearDepthStencilView(m_pDSV, D3D10_CLEAR_DEPTH, 1.0, 0 );
	}



	//--------------------------------------------------------------------------------------
	// Free resources
	//--------------------------------------------------------------------------------------
	void RenderSurface::Release()
	{
		if(m_bCube)
		{
			if(m_pTex)
				SAFE_RELEASE(m_pTex[0]);
			if(m_pTexSRV)
				SAFE_RELEASE(m_pTexSRV[0]);
			for(UINT i=0; i<m_ArraySize; i++)
				if(m_pTexRTV)
					SAFE_RELEASE(m_pTexRTV[i]);
		}
		else
			for(UINT i=0; i<m_ArraySize; i++)
			{
				if(m_pTex)
					SAFE_RELEASE(m_pTex[i]);
				if(m_pTexRTV)
					SAFE_RELEASE(m_pTexRTV[i]);
				if(m_pTexSRV)
					SAFE_RELEASE(m_pTexSRV[i]);
			}
			SAFE_DELETE_ARRAY(m_pTex);
			SAFE_DELETE_ARRAY(m_pTexRTV);
			SAFE_DELETE_ARRAY(m_pTexSRV);
			RenderSurface();
	}

}