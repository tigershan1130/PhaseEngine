//--------------------------------------------------------------------------------------
// File: D3D11.h
//
// D3D11 Application Framework
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include <windows.h>
#include <d3d10.h>
#include <d3dx10.h>
#include "DepthStencil.h"
#include "Util.h"

namespace Core
{


	//--------------------------------------------------------------------------------------
	// D3D11 Application framework
	//--------------------------------------------------------------------------------------
	class D3D10App
	{
	public:

		// Constructor
		D3D10App();

		// Create a window
		HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow, UINT width, UINT height );

		// Setup the d3d device
		HRESULT InitDevice();

		// Resize the swap chain
		HRESULT ResizeSwapChain();

		// Clears the current frame
		void ClearFrame();

		// Presents to the back buffer
		void Present();

		// Create a null-ref d3d9 device so we can use d3dx9
		HRESULT CreateD3D9NullRef();

		// Free anything created in CreateDeviceResources()
		void CleanupDevice();


		// Properties
		inline HWND GetHWND(){ return m_hWnd; }

	protected:
		int							m_Width, m_Height;		// Back buffer dimensions
		HWND                        m_hWnd;					// Handle for the window the device belongs to
		ID3D10Device*				m_pd3dDevice;		    // D3D10 Device
		IDirect3D9*					m_pD3D9;				// D3D9 setup (nullref)
		LPDIRECT3DDEVICE9			m_pd3dDevice9;			// D3D9 Device (nullref)
		D3D10_DRIVER_TYPE			m_driverType;			// Type of driver (hardware or software)
		IDXGISwapChain*             m_pSwapChain;			// Swap chain
		ID3D10RenderTargetView*		m_pBackBuffer;			// Main backbuffer
		DepthStencil				m_DefaultDepth;			// Default depth buffer
		int							m_CPUCores;				// Number of CPU cores
	};
}