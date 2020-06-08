// dxstdafx.cpp : source file that includes just the standard includes
// Phase.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information
#pragma unmanaged

#include "stdafx.h"

namespace Core
{

	// A NullRef D3D9Device, required for d3dx9 functionality such as .x mesh loading
	LPDIRECT3DDEVICE9 g_pd3dDevice9 = NULL;

	// Global D3D10Device
	ID3D10Device* g_pd3dDevice = NULL;

	// Main working directory
	String g_szDirectory;

	// Time scale
	float g_TimeScale;

}