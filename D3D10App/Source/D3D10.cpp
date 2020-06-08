//--------------------------------------------------------------------------------------
// File: D3D11.cpp
//
// D3D11 Application Framework
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "D3D10.h"


namespace Core
{


	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	D3D10App::D3D10App()
	{
		m_hWnd = NULL;
		m_pd3dDevice = NULL;
		m_driverType = D3D10_DRIVER_TYPE_NULL;
		m_pSwapChain = NULL;
		m_pBackBuffer = NULL;

		m_pD3D9 = NULL;
		m_pd3dDevice9 = NULL;

		// Create the log system
		// Clear the error log
		Log::Init();
		Log::Print("Phase Engine Log");

		// Determine the number of CPU cores
		m_CPUCores = Util::GetPhysicalProcessorCount();
		Log::Print("CPU Cores: ", m_CPUCores);
	}


	//--------------------------------------------------------------------------------------
	// Create Direct3D device and swap chain
	//--------------------------------------------------------------------------------------
	HRESULT D3D10App::InitDevice()
	{
		HRESULT hr = S_OK;;

		RECT rc;
		GetClientRect( m_hWnd, &rc );
		m_Width = rc.right - rc.left;
		m_Height = rc.bottom - rc.top;

		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

		D3D10_DRIVER_TYPE driverTypes[] =
		{
			D3D10_DRIVER_TYPE_HARDWARE,
			D3D10_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = sizeof( driverTypes ) / sizeof( driverTypes[0] );

		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory( &sd, sizeof( sd ) );
		sd.BufferCount = 1;
		sd.BufferDesc.Width = m_Width;
		sd.BufferDesc.Height = m_Height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = m_hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
		{
			m_driverType = driverTypes[driverTypeIndex];
			hr = D3D10CreateDeviceAndSwapChain( NULL, m_driverType, NULL, createDeviceFlags,
				D3D10_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice );
			if( SUCCEEDED( hr ) )
				break;
		}
		if( FAILED( hr ) )
			return hr;

		// Create a render target view
		ID3D10Texture2D* pBackBuffer;
		hr = m_pSwapChain->GetBuffer( 0, __uuidof( ID3D10Texture2D ), ( LPVOID* )&pBackBuffer );
		if( FAILED( hr ) )
			return hr;

		hr = m_pd3dDevice->CreateRenderTargetView( pBackBuffer, NULL, &m_pBackBuffer );
		pBackBuffer->Release();
		if( FAILED( hr ) )
			return hr;

		g_pd3dDevice = m_pd3dDevice;

		// Create the depth buffer
		m_DefaultDepth.Create(m_Width, m_Height, NULL, false);

		// Set the target
		m_pd3dDevice->OMSetRenderTargets( 1, &m_pBackBuffer, m_DefaultDepth.GetDSV() );

		// Setup the viewport
		D3D10_VIEWPORT vp;
		vp.Width = m_Width;
		vp.Height = m_Height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_pd3dDevice->RSSetViewports( 1, &vp );

		// Setup the d3d9 nullref
		return CreateD3D9NullRef();
	}


	//--------------------------------------------------------------------------------------
	// Resize the swap chain
	//--------------------------------------------------------------------------------------
	HRESULT D3D10App::ResizeSwapChain()
	{
		// Release the old views, as they hold references to the buffers we
		// will be destroying.  Also release the old depth/stencil buffer.
		m_pBackBuffer->Release();
		m_pBackBuffer = NULL;
		m_DefaultDepth.Release();

		// Get the new width and height
		RECT rc;
		GetClientRect( m_hWnd, &rc );
		m_Width = rc.right - rc.left;
		m_Height = rc.bottom - rc.top;

		// Get the swap chain desc
		DXGI_SWAP_CHAIN_DESC sd;
		m_pSwapChain->GetDesc(&sd);
		sd.BufferDesc.Height = m_Height;
		sd.BufferDesc.Width = m_Width;

		// Resize the swap chain and recreate the render target view.
		HRESULT hr;
		if( FAILED(m_pSwapChain->ResizeBuffers(1, m_Width, m_Height, DXGI_FORMAT_R8G8B8A8_UNORM, 0)))
		{
			LOG("Something is very wrong :(");
			return E_FAIL;
		}
		
		// Create the new render target
		ID3D10Texture2D* backBuffer;
		m_pSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), reinterpret_cast<void**>(&backBuffer));
		hr = m_pd3dDevice->CreateRenderTargetView(backBuffer, 0, &m_pBackBuffer);
		if(FAILED(hr))
		{
			LOG("m_pd3dDevice->CreateRenderTargetView() failed");
			Log::D3D10Error(hr);
			return hr;
		}
		backBuffer->Release();

		// Create the depth buffer
		hr = m_DefaultDepth.Create(m_Width, m_Height, NULL, false);
		if(FAILED(hr))
		{
			LOG("m_DefaultDepth.Create() failed");
			Log::D3D10Error(hr);
			return hr;
		}

		// Setup the viewport
		D3D10_VIEWPORT vp;
		vp.Width = m_Width;
		vp.Height = m_Height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_pd3dDevice->RSSetViewports( 1, &vp );
		
		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	// Creates a "NULLREF" D3D9 Device
	//   This way D3DX9 functions can be used in D3DX10
	//--------------------------------------------------------------------------------------
	HRESULT D3D10App::CreateD3D9NullRef()
	{
		m_pD3D9 = Direct3DCreate9( D3D_SDK_VERSION );
		if( NULL == m_pD3D9 )
		{
			LOG( "Failed to create IDirect3D9 interface" );

			return E_FAIL;
		}

		D3DDISPLAYMODE Mode;
		m_pD3D9->GetAdapterDisplayMode(0, &Mode);

		D3DPRESENT_PARAMETERS pp;
		ZeroMemory( &pp, sizeof(D3DPRESENT_PARAMETERS) ); 
		pp.BackBufferWidth  = 1;
		pp.BackBufferHeight = 1;
		pp.BackBufferFormat = Mode.Format;
		pp.BackBufferCount  = 1;
		pp.SwapEffect       = D3DSWAPEFFECT_COPY;
		pp.Windowed         = TRUE;

		if( FAILED( m_pD3D9->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, m_hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &m_pd3dDevice9 ) ) )
		{
			LOG( "Unable to create NULLREF device!" );

			return E_FAIL;
		}

		g_pd3dDevice9 = m_pd3dDevice9;

		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	// Clean up the objects we've created
	//--------------------------------------------------------------------------------------
	void D3D10App::CleanupDevice()
	{
		LOG( "Beginning device and resource clean-up" );

		if( m_pd3dDevice ) m_pd3dDevice->ClearState();

		m_DefaultDepth.Release();

		if( m_pBackBuffer ) m_pBackBuffer->Release();
		if( m_pSwapChain ) m_pSwapChain->Release();
		if( m_pd3dDevice ) m_pd3dDevice->Release();

		// Release D3D9 NullRef devices
		SAFE_RELEASE(m_pd3dDevice9);
		SAFE_RELEASE(m_pD3D9);

		LOG( "... done!" );
	}



	// Clears the current frame
	void D3D10App::ClearFrame()
	{
		// Just clear the backbuffer
		float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red,green,blue,alpha
		m_pd3dDevice->ClearRenderTargetView( m_pBackBuffer, ClearColor );
	}

	// Presents to the back buffer
	void D3D10App::Present()
	{
		m_pSwapChain->Present( 0, 0 );
	}

}
