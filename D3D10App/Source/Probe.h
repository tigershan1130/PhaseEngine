////////////////////////////////////////////////////////////////////////////////////////
// File: Probe.h
//
// Environment mapping and planar reflections
//
// Coded by Nate Orr
////////////////////////////////////////////////////////////////////////////////////////
#pragma unmanaged
#pragma once

#include "Array.h"
#include "RenderSurface.h"
#include "MeshObject.h"

namespace Core
{

	////////////////////////////////////////////////////////////////////////////////////////
	// Support for environment cube maps and planar reflections.
	////////////////////////////////////////////////////////////////////////////////////////
	class EnvironmentProbe
	{
	public:

		// Probe type
		enum PROBE_TYPE
		{
			PROBE_CUBEMAP=0,
			PROBE_PLANE,
		};

		// Create a new cubemap probe
		HRESULT CreateCubemap(UINT size, DXGI_FORMAT format, UINT miplevels, bool dynamic);

		// Mesh
		inline void AttachMesh(MeshObject* pMesh){ m_pMesh = pMesh; }
		inline MeshObject*GetMesh(){ return m_pMesh; }

		// Dynamic
		inline bool IsDynamic(){ return m_bDynamic; }

		// The render surface
		inline RenderSurface& GetSurface(){ return m_Surface; }

		// Free mem
		void Release();

	private:

		RenderSurface		m_Surface;		// Low res render target
		bool				m_bDynamic;		// If true, updated every frame
		MeshObject*			m_pMesh;		// The mesh that use this probe
		PROBE_TYPE			m_Type;			// Type of probe
	};
}