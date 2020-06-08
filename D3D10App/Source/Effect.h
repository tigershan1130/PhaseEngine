//--------------------------------------------------------------------------------------
// File: Renderer.h
//
// Implementation of the effect system
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "Light.h"
#include "Material.h"

namespace Core
{
	// Effect passes
	enum PASS_TYPE
	{
		PASS_GBUFFER = 0,			// GBuffer fill
		PASS_GBUFFER_ANIM,			// GBuffer fill with animation
		PASS_GBUFFER_CUBEMAP,		// GBuffer fill
		PASS_GBUFFER_TERRAIN,		// GBuffer fill for terrain
		PASS_SHADE,					// Screen aligned light quad
		PASS_SHADE_FULL,			// Full screen light quad
		PASS_SHADE_OCCLUSION,		// Tests occlusion queries on lights
		PASS_SHADOWMAP,				// Fill shadow map
		PASS_SHADOWMAP_ANIM,		// Fill shadow map, skinning supported

		// Water
		//PASS_COMPUTE_REFLECTION,	// Compute reflection texture
		PASS_DRAW_WATER,			// Draws out Water

		PASS_Z_FILL,				// Disable color writes and fill the zbuffer
		PASS_FORWARD,				// Very basic forward render pass
		PASS_FORWARD_BLEND,			// Very basic forward render pass with alpha blend
		PASS_FORWARD_NODEPTH,		// Very basic forward render pass without depth
		PASS_FORWARD_ANIM,			// Very basic forward render pass with skinning
		PASS_FORWARD_TERRAIN,		// Very basic forward render pass for terrain
		PASS_FORWARD_AMBIENT,		// Ambient pass that fills zbuffer
		PASS_FORWARD_SHADE,			// Forward render with full lighting
		PASS_FORWARD_AMBIENT_MSAA,	// Ambient pass that fills zbuffer with msaa
		PASS_FORWARD_SHADE_MSAA,	// Forward render with full lighting with msaa

		PASS_PARTICLE,				// Particle rendering into the gbuffer

		PASS_INSTANCED,				// Basic instancing
		
		PASS_FORWARD_TRANSPARENT_SHADE_BACK,		// Transparent mesh lighting accumulation pass for backfaces
		PASS_FORWARD_TRANSPARENT_SHADE_FRONT,		// Transparent mesh lighting accumulation pass for frontfaces
		PASS_FORWARD_TRANSPARENT_FINAL_BACK,		// Transparent mesh final lighting pass
		PASS_FORWARD_TRANSPARENT_FINAL_FRONT,		// Transparent mesh final lighting pass
		PASS_FORWARD_REFRACT_FRONT,					// Refraction using frame-buffer distortion
		PASS_FORWARD_REFRACT_BACK,					// Refraction using frame-buffer distortion

		PASS_WIREFRAME,			// Wireframe forward render
		PASS_WIREFRAME_NODEPTH,	// Wireframe forward render with no depth
		PASS_WIREFRAME_ANIM,	// Wireframe forward render with skinning
		
		PASS_COMPUTE_SCATTERING,// Fills the rayleigh and mie scattering textures
		PASS_SKY,				// Draws the sky with atmospheric scattering
		PASS_CLOUD_PLANE,		// Draws the high atmosphere cloud plane

		

		PASS_CLIPMAP_UPDATE,	// Terrain clipmap updating

		PASS_DOWNSCALE,				// Computes the luminance texture to a texture 1/4 the buffer size
		PASS_DOWNSCALE_LUMINANCE,	// Downscales the luminance texture, computing the min, max, and avg
		PASS_ADAPT,					// Adapts the luminance over time
		PASS_BRIGHTPASS,			// Sends bright pixels to the bloom buffer
		PASS_TONEMAP,				// Tonemaps the HDR render surface to the 0.0-1.0 range in the backbuffer

		PASS_AMBIENT,				// Ambient lighting
		PASS_AMBIENT_OCCLUSION,		// Ambient occlusion
		PASS_POST_VELOCITY_MAP,		// Builds a velocity map
		PASS_POST_MOTION_BLUR,		// Performs motion blur
		PASS_GAUSSIAN_BLUR_H,		// Horizontal gaussian blur
		PASS_GAUSSIAN_BLUR_V,		// Vertical gaussian blur
		PASS_BOX_BLUR_H,			// Seperable box blur
		PASS_BOX_BLUR_V,			// Seperable box blur

#ifdef PHASE_DEBUG
		
		PASS_EDITOR_TERRAIN_WIDGET,				// Draws the widget overlay for terrain sculpting and painting
		PASS_EDITOR_SELECTIONBOX,				// 2D transparent and bordered selection box
		PASS_EDITOR_GRID,
#endif

		PASS_SIZE,				// only used for the number of passes
	};

	// Techniques
	enum TECH_TYPE
	{
		TECH_GBUFFER=0,
		TECH_AMBIENT,
		TECH_TERRAIN,
		TECH_SHADE,		
		TECH_SHADOWMAP,
		TECH_FORWARD,
		TECH_TRANSPARENT,
		TECH_MEGAPARTICLE,
		TECH_SKY,
		TECH_HDR,
		TECH_POSTFX,
		TECH_PARTICLE,

		//Water
		TECH_WATER,

#ifdef PHASE_DEBUG
		TECH_HONUA,
#endif

		TECH_SIZE,
	};
	
	// Wraps the ID3D10Effect interface
	class Effect
	{
	public:

		Effect();

		// Load from a file
		HRESULT Create(char* fileName);

		// Free the effect
		void Release();

		// Sets all the proper light variables
		void SetLight(Light& light);

		// Set material variables
		void SetMaterial(Material& material);

		

		ID3D10EffectTechnique*              Technique[TECH_SIZE];
		ID3D10EffectPass*					Pass[PASS_SIZE];
		ID3D10EffectShaderResourceVariable* GBufferVariable;
		ID3D10EffectShaderResourceVariable* LightBufferVariable;	
		ID3D10EffectShaderResourceVariable* ShadowMapVariable;
		ID3D10EffectShaderResourceVariable* ShadowMapCubeVariable;
		ID3D10EffectShaderResourceVariable* ProbeCubeVariable;
		ID3D10EffectScalarVariable*			ShadowMapSizeVariable;
		ID3D10EffectShaderResourceVariable* FrameBufferVariable;
		ID3D10EffectVectorVariable*			AmbientVariable;
		ID3D10EffectShaderResourceVariable* RandomVariable;

		// Instancing
		ID3D10EffectConstantBuffer*			InstanceConstantBuffer;
		
		// HDR
		ID3D10EffectShaderResourceVariable* HDRVariable;
		ID3D10EffectShaderResourceVariable* HDRBloomVariable;
		ID3D10EffectShaderResourceVariable* LuminanceVariable;
		ID3D10EffectShaderResourceVariable* AdaptedLuminanceVariable;
		ID3D10EffectScalarVariable*			ElapsedTimeVariable;
		ID3D10EffectScalarVariable*			GaussianWeightsH;
		ID3D10EffectScalarVariable*			GaussianWeightsV;
		ID3D10EffectScalarVariable*			GaussianOffsetsH;
		ID3D10EffectScalarVariable*			GaussianOffsetsV;

		// Post processing
		ID3D10EffectShaderResourceVariable* PostFXVariable;
		ID3D10EffectShaderResourceVariable* PostFX2Variable;
		ID3D10EffectScalarVariable*			FilterKernelVariable;


		//Water
		ID3D10EffectVariable* fxWaterHeightVar;
		ID3D10EffectShaderResourceVariable* DuDvMapVariable;
		ID3D10EffectShaderResourceVariable* ReflectionMapVariable;
		ID3D10EffectShaderResourceVariable* NormalMapVariable;
		ID3D10EffectShaderResourceVariable* RefractionMapVariable;



		// Sky
		ID3D10EffectShaderResourceVariable* ScatteringTexturesVariable;
		ID3D10EffectShaderResourceVariable* CloudTextureVariable;
		ID3D10EffectShaderResourceVariable* PerlinVariable;
		ID3D10EffectConstantBuffer*			SkyConstantBuffer;
		ID3D10EffectVectorVariable*			SunDirectionVariable;
		ID3D10EffectScalarVariable*			PerlinLacunarityVariable;
		ID3D10EffectScalarVariable*			PerlinGainVariable;
		ID3D10EffectScalarVariable*			PerlinScaleVariable;
		ID3D10EffectScalarVariable*			CloudCoverVariable;
		ID3D10EffectScalarVariable*			CloudSharpnessVariable;

		// Terrain
		ID3D10EffectVectorVariable*			ClipmapOffsetVariable;
		ID3D10EffectScalarVariable*			ClipmapScaleVariable;
		ID3D10EffectVectorVariable*			ClipmapFixVariable;
		ID3D10EffectScalarVariable*			ClipmapSizeVariable;
		ID3D10EffectScalarVariable*			HeightmapSizeVariable;
		ID3D10EffectScalarVariable*			HeightmapScaleVariable;
		ID3D10EffectShaderResourceVariable* HeightmapVariable;
		ID3D10EffectShaderResourceVariable* ClipmapVariable;
		ID3D10EffectShaderResourceVariable* BlendClipmapVariable;
		ID3D10EffectShaderResourceVariable* TerrainTextureVariable;
		ID3D10EffectShaderResourceVariable* TerrainNormalmapVariable;
		ID3D10EffectScalarVariable*			TerrainTextureFlagVariable;
		ID3D10EffectScalarVariable*			TerrainNormalFlagVariable;
		ID3D10EffectScalarVariable*			TerrainTextureScaleVariable;

		// Material Properties
		ID3D10EffectScalarVariable*			MaterialID;
		ID3D10EffectShaderResourceVariable* EncodedMaterial;
		ID3D10EffectVectorVariable*			MaterialDiffuseVariable;
		ID3D10EffectVectorVariable*			MaterialSpecularVariable;
		ID3D10EffectVectorVariable*			MaterialEmissiveVariable;
		ID3D10EffectScalarVariable*			MaterialReflectivityVariable;	
		ID3D10EffectScalarVariable*			MaterialRefractivityVariable;	
		ID3D10EffectScalarVariable*			MaterialRefractionIndexVariable;
		ID3D10EffectScalarVariable*			MaterialTransparencyVariable;
		ID3D10EffectScalarVariable*			MaterialHeightScaleVariable;
		ID3D10EffectScalarVariable*			MaterialHeightSamplesVariable;
		ID3D10EffectScalarVariable*			MaterialBumpTypeVariable;
		ID3D10EffectScalarVariable*			MaterialShadingModelVariable;
		ID3D10EffectVectorVariable*			MaterialSurfaceParamsVariable;

		ID3D10EffectShaderResourceVariable* MaterialTextureVariable;
		ID3D10EffectScalarVariable*			MaterialTextureFlagVariable;
		

		// Light Properties
		ID3D10EffectVectorVariable*			LightColorVariable;
		ID3D10EffectVectorVariable*			LightPosVariable;
		ID3D10EffectVectorVariable*			LightDirVariable;
		ID3D10EffectScalarVariable*			LightRangeVariable;
		ID3D10EffectScalarVariable*			LightInnerRadiusVariable;
		ID3D10EffectScalarVariable*			LightOuterRadiusVariable;
		ID3D10EffectScalarVariable*			LightTypeVariable;
		ID3D10EffectScalarVariable*			ShadowMapFlagVariable;
		ID3D10EffectShaderResourceVariable* LightTextureVariable;
		ID3D10EffectScalarVariable*			LightTextureFlagVariable;
		ID3D10EffectMatrixVariable*			LightMatrixVariable;
		ID3D10EffectMatrixVariable*			ShadowMatrixVariable;

		// Transforms and camera variables
		ID3D10EffectMatrixVariable*         WorldMatrixVariable;
		ID3D10EffectMatrixVariable*         OldWorldMatrixVariable;
		ID3D10EffectMatrixVariable*         ViewProjectionVariable;
		ID3D10EffectMatrixVariable*         OldViewProjectionVariable;
		ID3D10EffectMatrixVariable*         ShadowOrthoProjectionVariable;
		ID3D10EffectMatrixVariable*         OrthoProjectionVariable;
		ID3D10EffectVectorVariable*			CameraPosVariable;
		ID3D10EffectVectorVariable*			CameraDirVariable;
		ID3D10EffectVectorVariable*			ScreenSizeVariable;
		ID3D10EffectScalarVariable*			FarZVariable;
		ID3D10EffectScalarVariable*			NearZVariable;
		ID3D10EffectMatrixVariable*         AnimMatricesVariable;

		// Editor and debug
#ifdef PHASE_DEBUG
		ID3D10EffectVectorVariable*			TerrainWidgetTexCoordVariable;
		ID3D10EffectVectorVariable*			TerrainWidgetRayVariable;
		ID3D10EffectScalarVariable*			TerrainWidgetRadiusVariable;
		ID3D10EffectScalarVariable*			TerrainWidgetHardnessVariable;

		// Effect validation
		HRESULT Validate();
#endif


	protected:
		ID3D10Effect*		m_pEffect;		// Effect
	};
	
}