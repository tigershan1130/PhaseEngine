//--------------------------------------------------------------------------------------
// File: Water.h
//
// Implementation of "planar Water reflection/refraction"
//
// Coded By Shan WenQin, 2009.
// Special Thanks to Nate Orr.
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "Effect.h"
#include "RenderSurface.h"

namespace Core
{
	class Water
	{
	public:
		Water();

		// update water, pass such values changing time for animation
		void update(float dt);

		// setup water grid: row, column, post spacing distance, and height.
		HRESULT initWater(Effect* pEffect, int m, int n, float dx, float waterheight);

		// Draw the Water
		void draw();

		// Free resources
		void Release();

		void RenderReflectionMap(Light& light);


		// get water height, this will depend really... who knows, you might want to get water's height to compute
		// for physics.
		float getWaterHeight();
	private:


		float WaterHeight;

		int NUM_ROWS;
		int NUM_COLS;

		int NUM_VERTSX;
		int NUM_VERTSY;

		Effect*				m_pEffect;					// Parent effect

		RenderSurface		WaterReflectionMap;
		DepthStencil		WaterReflectionMapDepth;
		RenderSurface       WaterRefractionMap;
		DepthStencil		WaterRefractionMapDepth;
		Texture				DuDvTexture;	
		Texture             NormalTexture;


		ID3D10Buffer* mVB;
		ID3D10Buffer* mIB;
		DWORD mNumVertices;
		DWORD mNumFaces;
	};


}
