////////////////////////////////////////////////////////////////////////////////////////
// File: Probe.h
//
// Environment mapping and planar reflections
//
// Coded by Nate Orr
////////////////////////////////////////////////////////////////////////////////////////
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "Log.h"
#include "Probe.h"

namespace Core
{

	////////////////////////////////////////////////////////////////////////////////////////
	// Create a new cubemap probe
	////////////////////////////////////////////////////////////////////////////////////////
	HRESULT EnvironmentProbe::CreateCubemap(UINT size, DXGI_FORMAT format, UINT miplevels, bool dynamic)
	{
		Log::Print("Creating probe");

		// Release previous probe
		Release();

		// Create the render target
		HRESULT hr;
		V_RETURN(m_Surface.CreateCube(size, size, format, miplevels));

		// Setup the properties
		m_bDynamic = dynamic;
		m_Type = PROBE_CUBEMAP;

		Log::Print("Environment cube probe created");

		return S_OK;
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// Free mem
	////////////////////////////////////////////////////////////////////////////////////////
	void EnvironmentProbe::Release()
	{
		m_Surface.Release();
		m_bDynamic = false;
	}
}