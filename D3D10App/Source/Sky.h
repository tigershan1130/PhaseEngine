//--------------------------------------------------------------------------------------
// File: Sky.h
//
// Implementation of "Precomputed Atmospheric Scattering"
// 
// Simulates multiple scattering effects from any view point
//
// Coded by Nate Orr 2008
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "Effect.h"
#include "RenderSurface.h"
#include "spa/spa.h"

namespace Core
{


	// Manages the sky
	// Atmospheric scattering, time of day
	class Sky
	{
	public:
		Sky(){
			m_pDomeMesh = NULL;
			m_PerlinSampler = NULL;
			m_PerlinSamplerSRV = NULL;
			m_ConstantBuffer = NULL;
			m_PlaneBuffer = NULL;
		}
		
		// Creates the sky system
		HRESULT Init(Effect* pEffect);

		// Free resources
		void Release();

		// Draw the sky
		void Render(D3DXVECTOR3& pos);

		// Build the scattering textures
		void ComputeScattering(D3DXVECTOR3& pos);

		// Binds the effect variables when the effect is reloaded
		void BindEffectVariables(Effect* pEffect);

		// Set the date
		inline void SetDate(int month, int day, int year){
			m_Month = month;
			m_Day = day;
			m_Year = year;
		}
		
		// Set the time of day
		inline void SetTimeOfDay(int hour, int minute, int second){
			m_Hour = hour;
			m_Minute = minute;
			m_Second = second;
		}
		
		// Set the location on earth
		inline void SetViewerLocation(int latitude, int longitude, int elevation){
			m_TOD.latitude = latitude;
			m_TOD.longitude = longitude;
			m_TOD.elevation = elevation;
		}


		// Get the sun direction
		inline D3DXVECTOR3 GetSunDirection(){
			return -m_SunDirection;
		}



	private:

		// Constants needed for rendering
		struct ScbConstant
		{
			float PI;
			float InnerRadius;
			float OuterRadius;
			float fScale;
			float KrESun;
			float KmESun;
			float Kr4PI;
			float Km4PI;
			int tNumSamples;
			int iNumSamples;
			D3DXVECTOR2 v2dRayleighMieScaleHeight;
			D3DXVECTOR3 WavelengthMie;
			float InvOpticalDepthN;
			D3DXVECTOR3 v3HG;
			float InvOpticalDepthNLessOne;
			D3DXVECTOR2 InvRayleighMieN;
			D3DXVECTOR2 InvRayleighMieNLessOne;
			float HalfTexelOpticalDepthN;
			D3DXVECTOR3 InvWavelength4;
		};

		ID3DX10Mesh*		m_pDomeMesh;				// The skydome mesh
		Effect*				m_pEffect;					// Parent effect
		RenderSurface		m_PrecomputedScattering;	// Precomputed scattering textures
		ID3D10Texture2D*	m_PerlinSampler;
		ID3D10ShaderResourceView*	m_PerlinSamplerSRV;
		Texture				m_CloudTexture;
		RenderSurface		m_Clouds;
		DepthStencil		m_CloudDepth;
		DepthStencil		m_Depth;
		UINT				m_ScatteringTextureSize;	// Dimension of the scattering texture	
		UINT				m_DomeSize;					// Dimension of dome
		UINT				m_OpticalDepthSize;			// Optical depth texture size
		ScbConstant			m_Constants;				// Sky shader constants
		ID3D10Buffer*		m_ConstantBuffer;			// The constant buffer

		// Cloud plane
		Vertex				m_PlaneVerts[6];
		ID3D10Buffer*		m_PlaneBuffer;

		int					m_Month;
		int					m_Day;
		int					m_Year;
		int					m_Hour;
		int					m_Minute;
		int					m_Second;

		spa_data			m_TOD;						// Time of day and sun angle data
		D3DXVECTOR3			m_SunDirection;				// Direction of the sun in the sky

		// Generates the perlin noise sampler
		void GeneratePerlinSampler();
		
		// Builds the dome mesh
		HRESULT BuildDome();

		// Sets up the constants
		void SetConstants();

		// Computes the sun direction from the spa data
		void ComputeSunDirection();

		// Setup the tod system
		void InitTOD();
	};
}
