//--------------------------------------------------------------------------------------
// File: Renderer.h
//
// Implementation of the effect system
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged


#include "Effect.h"
#include "Vertex.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	Effect::Effect()
	{
		m_pEffect = NULL;
	}

	
	//--------------------------------------------------------------------------------------
	// Load from a file
	//--------------------------------------------------------------------------------------
	HRESULT Effect::Create(char* fileName)
	{
		// Create the effect
		DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
		ID3D10Blob *pErrors = NULL;
		String szEffect = g_szDirectory + fileName;
		Log::Print(szEffect);
		ID3D10Effect* pEffect;
		//HRESULT hr = D3DX10CreateEffectFromFileA( szEffect, NULL, NULL, NULL, NULL, 0, g_pd3dDevice, NULL, NULL, &pEffect, NULL, NULL );
		HRESULT hr = D3DX10CreateEffectFromFileA( szEffect, NULL, NULL, "fx_4_0", dwShaderFlags, 0, g_pd3dDevice, NULL, NULL, &pEffect, &pErrors, NULL );
		if( FAILED( hr ) )
		{
			Log::Print("--Effect file failed to load--");			
			Log::D3D10Error(hr);
			// Output debug errors
			if(pErrors)
			{
				Log::Print((LPCSTR)pErrors->GetBufferPointer());
				Log::Print("--End of effect debug output--");
				MessageBoxA( NULL, (LPCSTR)pErrors->GetBufferPointer(), "Effect Compile Error", MB_OK );
				pErrors->Release();
				pErrors = NULL;
			}
			V_RETURN( hr );
		}
		SAFE_RELEASE(m_pEffect);		
		m_pEffect = pEffect;

		// Get the effect techniques
		Technique[TECH_GBUFFER] = m_pEffect->GetTechniqueByName( "GBuffer" );
		Technique[TECH_AMBIENT] = m_pEffect->GetTechniqueByName( "AmbientOcclusion" );
		Technique[TECH_TERRAIN] = m_pEffect->GetTechniqueByName( "Terrain" );
		Technique[TECH_SHADE] = m_pEffect->GetTechniqueByName( "Shade" );
		Technique[TECH_SHADOWMAP] = m_pEffect->GetTechniqueByName( "ShadowMap" );
		Technique[TECH_FORWARD] = m_pEffect->GetTechniqueByName( "Forward" );
		Technique[TECH_TRANSPARENT] = m_pEffect->GetTechniqueByName( "Transparency" );
		Technique[TECH_SKY] = m_pEffect->GetTechniqueByName( "Sky" );
		Technique[TECH_HDR] = m_pEffect->GetTechniqueByName( "HDR" );
		Technique[TECH_POSTFX] = m_pEffect->GetTechniqueByName( "PostFX" );
		Technique[TECH_PARTICLE] = m_pEffect->GetTechniqueByName( "Particle" );
		//Water
		Technique[TECH_WATER]  = m_pEffect->GetTechniqueByName( "Water" );

		// Get the passes
		Pass[PASS_GBUFFER] = Technique[TECH_GBUFFER]->GetPassByName( "GBuffer" );
		Pass[PASS_GBUFFER_CUBEMAP] = Technique[TECH_GBUFFER]->GetPassByName( "GBufferCubeMap" );
		Pass[PASS_GBUFFER_ANIM] = Technique[TECH_GBUFFER]->GetPassByName( "GBufferAnim" );

		Pass[PASS_AMBIENT] = Technique[TECH_POSTFX]->GetPassByName( "Ambient" );
		Pass[PASS_AMBIENT_OCCLUSION] = Technique[TECH_AMBIENT]->GetPassByName( "SSAO" );

		Pass[PASS_CLIPMAP_UPDATE] = Technique[TECH_TERRAIN]->GetPassByName( "ClipmapUpdate" );
		Pass[PASS_GBUFFER_TERRAIN] = Technique[TECH_TERRAIN]->GetPassByName( "GBufferTerrain" );

		Pass[PASS_SHADE_OCCLUSION] = Technique[TECH_SHADE]->GetPassByName( "Occlusion" );
		Pass[PASS_SHADE] = Technique[TECH_SHADE]->GetPassByName( "Shading" );
		Pass[PASS_SHADE_FULL] = Technique[TECH_SHADE]->GetPassByName( "Fullscreen" );

		Pass[PASS_SHADOWMAP] = Technique[TECH_SHADOWMAP]->GetPassByName( "ShadowMap" );
		Pass[PASS_SHADOWMAP_ANIM] = Technique[TECH_SHADOWMAP]->GetPassByName( "ShadowMapAnim" );

		//Water
		Pass[PASS_DRAW_WATER] = Technique[TECH_WATER]->GetPassByName( "RenderingWater");

		Pass[PASS_WIREFRAME] = Technique[TECH_FORWARD]->GetPassByName( "Wireframe" );
		Pass[PASS_WIREFRAME_NODEPTH] = Technique[TECH_FORWARD]->GetPassByName( "WireframeNoDepth" );
		Pass[PASS_WIREFRAME_ANIM] = Technique[TECH_FORWARD]->GetPassByName( "WireframeAnim" );

		Pass[PASS_PARTICLE] = Technique[TECH_PARTICLE]->GetPassByName( "Particle" );

		Pass[PASS_INSTANCED] = Technique[TECH_FORWARD]->GetPassByName( "Instanced" );
		
		Pass[PASS_Z_FILL] = Technique[TECH_FORWARD]->GetPassByName( "ZFill" );
		Pass[PASS_FORWARD] = Technique[TECH_FORWARD]->GetPassByName( "Unlit" );
		Pass[PASS_FORWARD_BLEND] = Technique[TECH_FORWARD]->GetPassByName( "UnlitBlend" );
		Pass[PASS_FORWARD_NODEPTH] = Technique[TECH_FORWARD]->GetPassByName( "UnlitNoDepth" );
		Pass[PASS_FORWARD_ANIM] = Technique[TECH_FORWARD]->GetPassByName( "UnlitAnim" );
		Pass[PASS_FORWARD_AMBIENT] = Technique[TECH_FORWARD]->GetPassByName( "Ambient" );
		Pass[PASS_FORWARD_SHADE] = Technique[TECH_FORWARD]->GetPassByName( "Shade" );
		Pass[PASS_FORWARD_AMBIENT_MSAA] = Technique[TECH_FORWARD]->GetPassByName( "AmbientMSAA" );
		Pass[PASS_FORWARD_SHADE_MSAA] = Technique[TECH_FORWARD]->GetPassByName( "ShadeMSAA" );
		Pass[PASS_FORWARD_TERRAIN] = Technique[TECH_FORWARD]->GetPassByName( "UnlitTerrain" );

		Pass[PASS_FORWARD_TRANSPARENT_SHADE_BACK] = Technique[TECH_TRANSPARENT]->GetPassByName( "TransparentShadeBack" );
		Pass[PASS_FORWARD_TRANSPARENT_SHADE_FRONT] = Technique[TECH_TRANSPARENT]->GetPassByName( "TransparentShadeFront" );
		Pass[PASS_FORWARD_TRANSPARENT_FINAL_BACK] = Technique[TECH_TRANSPARENT]->GetPassByName( "TransparentFinalBack" );
		Pass[PASS_FORWARD_TRANSPARENT_FINAL_FRONT] = Technique[TECH_TRANSPARENT]->GetPassByName( "TransparentFinalFront" );
		Pass[PASS_FORWARD_REFRACT_FRONT] = Technique[TECH_TRANSPARENT]->GetPassByName( "RefractFront" );
		Pass[PASS_FORWARD_REFRACT_BACK] = Technique[TECH_TRANSPARENT]->GetPassByName( "RefractBack" );

		Pass[PASS_SKY] = Technique[TECH_SKY]->GetPassByName( "Sky" );
		Pass[PASS_CLOUD_PLANE] = Technique[TECH_SKY]->GetPassByName( "CloudPlane" );
		Pass[PASS_COMPUTE_SCATTERING] = Technique[TECH_SKY]->GetPassByName( "ComputeScattering" );

		Pass[PASS_DOWNSCALE] = Technique[TECH_HDR]->GetPassByName( "DownScale" );
		Pass[PASS_DOWNSCALE_LUMINANCE] = Technique[TECH_HDR]->GetPassByName( "DownScaleLuminance" );
		Pass[PASS_ADAPT] = Technique[TECH_HDR]->GetPassByName( "Adapt" );
		Pass[PASS_BRIGHTPASS] = Technique[TECH_HDR]->GetPassByName( "BrightPass" );
		Pass[PASS_TONEMAP] = Technique[TECH_HDR]->GetPassByName( "ToneMap" );

		Pass[PASS_POST_VELOCITY_MAP] = Technique[TECH_POSTFX]->GetPassByName( "VelocityMap" );
		Pass[PASS_POST_MOTION_BLUR] = Technique[TECH_POSTFX]->GetPassByName( "MotionBlur" );
		Pass[PASS_BOX_BLUR_H] = Technique[TECH_POSTFX]->GetPassByName( "BoxBlurH" );
		Pass[PASS_BOX_BLUR_V] = Technique[TECH_POSTFX]->GetPassByName( "BoxBlurV" );
		Pass[PASS_GAUSSIAN_BLUR_H] = Technique[TECH_POSTFX]->GetPassByName( "GaussianBlurH" );
		Pass[PASS_GAUSSIAN_BLUR_V] = Technique[TECH_POSTFX]->GetPassByName( "GaussianBlurV" );

#ifdef PHASE_DEBUG
		Technique[TECH_HONUA] = m_pEffect->GetTechniqueByName( "Honua" );
		Pass[PASS_EDITOR_SELECTIONBOX] = Technique[TECH_HONUA]->GetPassByName( "SelectionBox" );
		Pass[PASS_EDITOR_TERRAIN_WIDGET] = Technique[TECH_HONUA]->GetPassByName( "TerrainWidget" );
		Pass[PASS_EDITOR_GRID] = Technique[TECH_HONUA]->GetPassByName( "Grid" );
#endif

		// Instancing
		InstanceConstantBuffer = m_pEffect->GetConstantBufferByName("cbInstanceData")->AsConstantBuffer();
		
		// Gbuffer vars
		GBufferVariable = m_pEffect->GetVariableByName( "g_txGBuffer" )->AsShaderResource(); 

		// Shadow mapping
		ShadowMapVariable = m_pEffect->GetVariableByName( "g_txShadowMap" )->AsShaderResource(); 
		ShadowMapCubeVariable = m_pEffect->GetVariableByName( "g_txShadowMapCube" )->AsShaderResource(); 
		ShadowMapSizeVariable = m_pEffect->GetVariableByName( "g_ShadowMapTexelSize" )->AsScalar(); 

		// Misc textures
		LightBufferVariable = m_pEffect->GetVariableByName( "g_txLightBuffer" )->AsShaderResource(); 
		ProbeCubeVariable = m_pEffect->GetVariableByName( "g_txCubeProbe" )->AsShaderResource(); 
		FrameBufferVariable = m_pEffect->GetVariableByName( "g_txFrameBuffer" )->AsShaderResource(); 
		RandomVariable = m_pEffect->GetVariableByName( "g_txRandom" )->AsShaderResource(); 

		// Ambient
		AmbientVariable = m_pEffect->GetVariableByName( "g_Ambient" )->AsVector(); 

		//Water
		fxWaterHeightVar = m_pEffect->GetVariableByName( "WaterHeight" )->AsScalar();
		DuDvMapVariable = m_pEffect->GetVariableByName( "dudvMap" )->AsShaderResource();
		ReflectionMapVariable = m_pEffect->GetVariableByName( "ReflectionMap" )->AsShaderResource();
		NormalMapVariable = m_pEffect->GetVariableByName( "NormalTexture" )->AsShaderResource();
		RefractionMapVariable = m_pEffect->GetVariableByName( "RefractionMap" )->AsShaderResource();

		// Sky
		ScatteringTexturesVariable = m_pEffect->GetVariableByName( "g_txScattering" )->AsShaderResource(); 
		CloudTextureVariable = m_pEffect->GetVariableByName( "g_txCloudPlane" )->AsShaderResource(); 
		PerlinVariable = m_pEffect->GetVariableByName( "g_txPerlin" )->AsShaderResource(); 
		SkyConstantBuffer = m_pEffect->GetConstantBufferByName("cbSky")->AsConstantBuffer();
		SunDirectionVariable = m_pEffect->GetVariableByName("g_SunDir")->AsVector();
		PerlinLacunarityVariable = m_pEffect->GetVariableByName("g_PerlinLacunarity")->AsScalar();
		PerlinScaleVariable = m_pEffect->GetVariableByName("g_PerlinScale")->AsScalar();
		PerlinGainVariable = m_pEffect->GetVariableByName("g_PerlinGain")->AsScalar();
		CloudCoverVariable = m_pEffect->GetVariableByName("g_CloudCover")->AsScalar();
		CloudSharpnessVariable = m_pEffect->GetVariableByName("g_CloudSharpness")->AsScalar();


		// Terrain
		ClipmapOffsetVariable = m_pEffect->GetVariableByName("g_ClipmapOffset")->AsVector();
		ClipmapFixVariable = m_pEffect->GetVariableByName("g_ClipmapFix")->AsVector();
		ClipmapScaleVariable = m_pEffect->GetVariableByName( "g_ClipmapScale" )->AsScalar(); 
		ClipmapSizeVariable = m_pEffect->GetVariableByName( "g_ClipmapSize" )->AsScalar(); 
		HeightmapSizeVariable = m_pEffect->GetVariableByName( "g_HeightmapSize" )->AsScalar(); 
		HeightmapScaleVariable = m_pEffect->GetVariableByName( "g_HeightScale" )->AsScalar(); 
		HeightmapVariable = m_pEffect->GetVariableByName( "g_txHeightmap" )->AsShaderResource(); 
		ClipmapVariable = m_pEffect->GetVariableByName( "g_txClipmap" )->AsShaderResource(); 
		BlendClipmapVariable = m_pEffect->GetVariableByName( "g_txBlendClipmap" )->AsShaderResource(); 
		TerrainTextureVariable = m_pEffect->GetVariableByName( "g_txTerrainTex" )->AsShaderResource(); 
		TerrainNormalmapVariable = m_pEffect->GetVariableByName( "g_txTerrainNormal" )->AsShaderResource(); 
		TerrainTextureFlagVariable = m_pEffect->GetVariableByName( "g_bTextureLayer" )->AsScalar(); 
		TerrainNormalFlagVariable = m_pEffect->GetVariableByName( "g_bNormalLayer" )->AsScalar(); 
		TerrainTextureScaleVariable = m_pEffect->GetVariableByName( "g_LayerScales" )->AsScalar(); 


		// HDR
		HDRVariable = m_pEffect->GetVariableByName( "g_txHDR" )->AsShaderResource(); 
		HDRBloomVariable = m_pEffect->GetVariableByName( "g_txHDRBloom" )->AsShaderResource(); 
		LuminanceVariable = m_pEffect->GetVariableByName( "g_txLuminance" )->AsShaderResource(); 
		AdaptedLuminanceVariable = m_pEffect->GetVariableByName( "g_txAdaptedLuminance" )->AsShaderResource(); 
		ElapsedTimeVariable = m_pEffect->GetVariableByName( "g_ElapsedTime" )->AsScalar(); 
		GaussianWeightsH = m_pEffect->GetVariableByName( "g_GWeightsH" )->AsScalar(); 
		GaussianWeightsV = m_pEffect->GetVariableByName( "g_GWeightsV" )->AsScalar(); 
		GaussianOffsetsH = m_pEffect->GetVariableByName( "g_GOffsetsH" )->AsScalar(); 
		GaussianOffsetsV = m_pEffect->GetVariableByName( "g_GOffsetsV" )->AsScalar(); 
		
		// Post processing
		PostFXVariable = m_pEffect->GetVariableByName( "g_txPostFX" )->AsShaderResource(); 
		PostFX2Variable = m_pEffect->GetVariableByName( "g_txPostFX2" )->AsShaderResource(); 
		FilterKernelVariable = m_pEffect->GetVariableByName( "g_FilterKernel" )->AsScalar(); 

		// Material Properties
		MaterialID = m_pEffect->GetVariableByName( "g_MaterialID" )->AsScalar(); 
		EncodedMaterial = m_pEffect->GetVariableByName( "g_txEncodedMaterial" )->AsShaderResource(); 
		MaterialSurfaceParamsVariable = m_pEffect->GetVariableByName( "g_MaterialSurfaceParams" )->AsVector(); 
		MaterialDiffuseVariable = m_pEffect->GetVariableByName( "g_MaterialDiffuse" )->AsVector(); 
		MaterialSpecularVariable = m_pEffect->GetVariableByName( "g_MaterialSpecular" )->AsVector(); 
		MaterialEmissiveVariable = m_pEffect->GetVariableByName( "g_MaterialEmissive" )->AsVector(); 
		MaterialTransparencyVariable = m_pEffect->GetVariableByName( "g_MaterialTransparency" )->AsScalar(); 
		MaterialReflectivityVariable = m_pEffect->GetVariableByName( "g_MaterialReflectivity" )->AsScalar(); 
		MaterialRefractivityVariable = m_pEffect->GetVariableByName( "g_MaterialRefractivity" )->AsScalar(); 
		MaterialRefractionIndexVariable = m_pEffect->GetVariableByName( "g_MaterialRefractionIndex" )->AsScalar(); 
		MaterialHeightScaleVariable = m_pEffect->GetVariableByName( "g_MaterialCSMScale" )->AsScalar(); 
		MaterialHeightSamplesVariable = m_pEffect->GetVariableByName( "g_MaterialCSMSamples" )->AsScalar(); 
		MaterialBumpTypeVariable = m_pEffect->GetVariableByName( "g_MaterialBumpType" )->AsScalar(); 
		MaterialShadingModelVariable = m_pEffect->GetVariableByName( "g_MaterialShadingModel" )->AsScalar(); 
		
		MaterialTextureVariable = m_pEffect->GetVariableByName( "g_txMaterial" )->AsShaderResource(); 
		MaterialTextureFlagVariable = m_pEffect->GetVariableByName( "g_bMaterialTex" )->AsScalar(); 
		
		
		// Light Properties
		LightColorVariable = m_pEffect->GetVariableByName( "g_LightColor" )->AsVector(); 
		LightPosVariable = m_pEffect->GetVariableByName( "g_LightPos" )->AsVector(); 
		LightDirVariable = m_pEffect->GetVariableByName( "g_LightDir" )->AsVector(); 
		LightRangeVariable = m_pEffect->GetVariableByName( "g_LightRange" )->AsScalar(); 
		LightInnerRadiusVariable = m_pEffect->GetVariableByName( "g_LightInnerRadius" )->AsScalar(); 
		LightOuterRadiusVariable = m_pEffect->GetVariableByName( "g_LightOuterRadius" )->AsScalar(); 
		LightTypeVariable = m_pEffect->GetVariableByName( "g_LightType" )->AsScalar(); 
		ShadowMapFlagVariable = m_pEffect->GetVariableByName( "g_bShadowMap" )->AsScalar(); 
		LightTextureVariable = m_pEffect->GetVariableByName( "g_txLight" )->AsShaderResource(); 
		LightTextureFlagVariable = m_pEffect->GetVariableByName( "g_bLightTex" )->AsScalar(); 
		LightMatrixVariable = m_pEffect->GetVariableByName( "g_mLight" )->AsMatrix(); 
		ShadowMatrixVariable = m_pEffect->GetVariableByName( "g_mShadowProj" )->AsMatrix(); 

		// Transforms and camera variables
		WorldMatrixVariable = m_pEffect->GetVariableByName( "g_mWorld" )->AsMatrix(); 
		OldWorldMatrixVariable = m_pEffect->GetVariableByName( "g_mOldWorld" )->AsMatrix(); 
		ViewProjectionVariable = m_pEffect->GetVariableByName( "g_mViewProjection" )->AsMatrix(); 
		OldViewProjectionVariable = m_pEffect->GetVariableByName( "g_mOldViewProjection" )->AsMatrix(); 
		ShadowOrthoProjectionVariable = m_pEffect->GetVariableByName( "g_mShadowOrtho" )->AsMatrix(); 
		OrthoProjectionVariable = m_pEffect->GetVariableByName( "g_mOrtho" )->AsMatrix(); 
		CameraPosVariable = m_pEffect->GetVariableByName( "g_CameraPos" )->AsVector(); 
		CameraDirVariable = m_pEffect->GetVariableByName( "g_CameraDir" )->AsVector(); 
		ScreenSizeVariable = m_pEffect->GetVariableByName( "g_ScreenSize" )->AsVector(); 
		FarZVariable = m_pEffect->GetVariableByName( "g_FarZ" )->AsScalar(); 
		NearZVariable = m_pEffect->GetVariableByName( "g_NearZ" )->AsScalar(); 
		AnimMatricesVariable = m_pEffect->GetVariableByName( "g_mBoneWorld" )->AsMatrix(); 

		// Editor and debug
#ifdef PHASE_DEBUG
		TerrainWidgetTexCoordVariable = m_pEffect->GetVariableByName( "g_TWTexCoord" )->AsVector(); 
		TerrainWidgetRayVariable = m_pEffect->GetVariableByName( "g_TWRay" )->AsVector(); 
		TerrainWidgetRadiusVariable = m_pEffect->GetVariableByName( "g_TWRadius" )->AsScalar(); 
		TerrainWidgetHardnessVariable = m_pEffect->GetVariableByName( "g_TWHardness" )->AsScalar(); 
		
		// Validate all variables
		Validate();
#endif

		m_pEffect->Optimize();



		// Create the input layout for the Vertex format
		SAFE_RELEASE(Vertex::pInputLayout);
		D3D10_PASS_DESC PassDesc;
		Pass[ PASS_SKY ]->GetDesc( &PassDesc );
		hr = g_pd3dDevice->CreateInputLayout( Vertex::Desc, 3, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &Vertex::pInputLayout );
		if( FAILED(hr) )
		{
			Log::FatalError("Failed to create input layout");
			Log::D3D10Error(hr); 		
			return hr;
		}

		//Water  Create the input layout for the vertex format for water here??... Just gonna go with it here.
		// assuming vertex has position, normal and tex.
		
		SAFE_RELEASE(Vertex::pInputLayout);
		Pass[ PASS_DRAW_WATER ]->GetDesc( &PassDesc );
		hr = g_pd3dDevice->CreateInputLayout( Vertex::Desc, 3, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &Vertex::pInputLayout );
		if( FAILED(hr) )
		{
			Log::FatalError("Failed to create input layout for water");
			Log::D3D10Error(hr); 		
			return hr;
		}

		SAFE_RELEASE(SkinnedVertex::pInputLayout);
		Pass[ PASS_GBUFFER_ANIM ]->GetDesc( &PassDesc );
		hr = g_pd3dDevice->CreateInputLayout( SkinnedVertex::Desc, 5, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &SkinnedVertex::pInputLayout );
		if( FAILED(hr) )
		{
			Log::FatalError("Failed to create input layout");
			Log::D3D10Error(hr); 		
			return hr;
		}
		SAFE_RELEASE(PosVertex::pInputLayout);
		Pass[ PASS_GBUFFER_TERRAIN ]->GetDesc( &PassDesc );
		hr = g_pd3dDevice->CreateInputLayout( PosVertex::Desc, 1, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &PosVertex::pInputLayout );
		if( FAILED(hr) )
		{
			Log::FatalError("Failed to create input layout");
			Log::D3D10Error(hr); 		
			return hr;
		}

		return S_OK;
	}

	
	//--------------------------------------------------------------------------------------
	// Free the effect
	//--------------------------------------------------------------------------------------
	void Effect::Release()
	{
		SAFE_RELEASE(m_pEffect);
		SAFE_RELEASE(Vertex::pInputLayout);
		SAFE_RELEASE(SkinnedVertex::pInputLayout);
		SAFE_RELEASE(PosVertex::pInputLayout);
	}


	//--------------------------------------------------------------------------------------
	// Sets all the proper light variables
	//--------------------------------------------------------------------------------------
	void Effect::SetLight(Light& light)
	{
		LightPosVariable->SetFloatVector((float*)&light.GetPos());
		LightDirVariable->SetFloatVector((float*)&light.GetDir());
		LightColorVariable->SetFloatVector((float*)&light.Color);
		LightRangeVariable->SetFloat( light.GetRange()*0.5f );
		LightInnerRadiusVariable->SetFloat( light.InnerRadius );
		LightOuterRadiusVariable->SetFloat( light.OuterRadius );
		LightTypeVariable->SetInt( (int)light.Type );
		if(light.pProj)
		{
			LightTextureVariable->SetResource( light.pProj->GetResource() );
			LightTextureFlagVariable->SetBool( true );
		}
		else
		{
			LightTextureFlagVariable->SetBool( false );
		}
			
		ShadowMapFlagVariable->SetBool( light.IsShadowed );
	}

	//--------------------------------------------------------------------------------------
	// Set the material variables
	//--------------------------------------------------------------------------------------
	void Effect::SetMaterial(Material& material)
	{
		MaterialSurfaceParamsVariable->SetFloatVector((float*)&material.GetSurfaceParams());
		MaterialDiffuseVariable->SetFloatVector((float*)&material.GetDiffuse());
		MaterialSpecularVariable->SetFloatVector( (float*)&material.GetSpecular() );
		MaterialEmissiveVariable->SetFloatVector((float*)&material.GetEmissive());
		MaterialTransparencyVariable->SetFloat(material.GetTransparency());
		MaterialReflectivityVariable->SetFloat( material.GetReflectivity() );
		MaterialRefractivityVariable->SetFloat( material.GetRefractivity() );
		MaterialRefractionIndexVariable->SetFloat( material.GetRefractionIndex() );
		MaterialHeightScaleVariable->SetFloat( material.GetCSMScale() );
		MaterialHeightSamplesVariable->SetInt( material.GetMaxCSMSamples() );
		MaterialBumpTypeVariable->SetInt( material.GetBumpType() );
		MaterialShadingModelVariable->SetInt( material.GetShadingModel() );
		MaterialID->SetInt(material.ID);

		// Set the textures
		MaterialTextureVariable->SetResourceArray( material.GetTextureResources(), 0, TEX_SIZE );
		MaterialTextureFlagVariable->SetBoolArray( material.GetTextureFlags(), 0, TEX_SIZE );
	}


	//--------------------------------------------------------------------------------------
	// Effect validation
	//--------------------------------------------------------------------------------------
#ifdef PHASE_DEBUG
	HRESULT Effect::Validate()
	{

		//Water
		// Maybe validate water variable too? not sure what i should put in the quote. - Shan.
		VALIDATE(ReflectionMapVariable, "ReflectionMapVariable");
		VALIDATE(DuDvMapVariable, "DuDvMapVariable");
		VALIDATE(fxWaterHeightVar, "fxWaterHeightvar");
		VALIDATE(NormalMapVariable, "NormalMapVariable");
		VALIDATE(RefractionMapVariable, "RefractionMapVariable");

		VALIDATE(InstanceConstantBuffer, "InstanceConstantBuffer");	
		
		VALIDATE(OldWorldMatrixVariable, "OldWorldMatrixVariable");		
		
		VALIDATE(TerrainTextureFlagVariable, "TerrainTextureFlagVariable");
		VALIDATE(TerrainNormalFlagVariable, "TerrainNormalFlagVariable");
		VALIDATE(TerrainNormalmapVariable, "TerrainNormalmapVariable");
		VALIDATE(TerrainTextureVariable, "TerrainTextureVariable");
		VALIDATE(ClipmapFixVariable, "ClipmapFixVariable");
		VALIDATE(ClipmapSizeVariable, "ClipmapSizeVariable");
		VALIDATE(HeightmapSizeVariable, "HeightmapSizeVariable");
		VALIDATE(HeightmapScaleVariable, "HeightmapScaleVariable");
		VALIDATE(ClipmapScaleVariable, "ClipmapScaleVariable");
		VALIDATE(ClipmapVariable, "ClipmapVariable");
		VALIDATE(BlendClipmapVariable, "BlendClipmapVariable");
		VALIDATE(ClipmapOffsetVariable, "ClipmapOffsetVariable");
		VALIDATE(PostFX2Variable, "PostFX2Variable");
		
		VALIDATE(PostFXVariable, "PostFXVariable");
		VALIDATE(PostFX2Variable, "PostFX2Variable");

		VALIDATE(PerlinVariable, "PerlinVariable");
		VALIDATE(CloudTextureVariable, "CloudTextureVariable");
		VALIDATE(ScatteringTexturesVariable, "ScatteringTexturesVariable");
		VALIDATE(SkyConstantBuffer, "SkyConstantBuffer");
		VALIDATE(SunDirectionVariable, "SunDirectionVariable");
		VALIDATE(PerlinLacunarityVariable, "PerlinLacunarityVariable");
		VALIDATE(PerlinGainVariable, "PerlinGainVariable");
		VALIDATE(PerlinScaleVariable, "PerlinScaleVariable");
		VALIDATE(CloudCoverVariable, "CloudCoverVariable");
		VALIDATE(CloudSharpnessVariable, "CloudSharpnessVariable");
		
		VALIDATE(ElapsedTimeVariable, "ElapsedTimeVariable");
		VALIDATE(AdaptedLuminanceVariable, "AdaptedLuminanceVariable");
		VALIDATE(LuminanceVariable, "LuminanceVariable");
		VALIDATE(HDRBloomVariable, "HDRBloomVariable");
		VALIDATE(HDRVariable, "HDRVariable");
		VALIDATE(GaussianOffsetsH, "GaussianOffsetsH");
		VALIDATE(GaussianOffsetsV, "GaussianOffsetsV");
		VALIDATE(GaussianWeightsH, "GaussianWeightsH");
		VALIDATE(GaussianWeightsV, "GaussianWeightsV");
		VALIDATE(AmbientVariable, "AmbientVariable");
		VALIDATE(RandomVariable, "RandomVariable");
		VALIDATE(FrameBufferVariable, "FrameBufferVariable");
		VALIDATE(ProbeCubeVariable, "ProbeCubeVariable");
		VALIDATE(LightBufferVariable, "LightBufferVariable");
		VALIDATE(ShadowMapSizeVariable, "ShadowMapSizeVariable");
		VALIDATE(ShadowMapCubeVariable, "ShadowMapCubeVariable");
		VALIDATE(ShadowMapVariable, "ShadowMapVariable");
		VALIDATE(GBufferVariable, "GBufferVariable");

		VALIDATE(MaterialTransparencyVariable, "MaterialTransparencyVariable");
		VALIDATE(MaterialTextureFlagVariable, "MaterialTextureFlagVariable");
		VALIDATE(MaterialTextureVariable, "MaterialTextureVariable");
		VALIDATE(MaterialReflectivityVariable, "MaterialReflectivityVariable");
		VALIDATE(MaterialSpecularVariable, "MaterialSpecularVariable");
		VALIDATE(MaterialEmissiveVariable, "MaterialEmissiveVariable");
		VALIDATE(MaterialDiffuseVariable, "MaterialDiffuseVariable");
		VALIDATE(MaterialID, "MaterialID");
		VALIDATE(EncodedMaterial, "EncodedMaterial");
		VALIDATE(MaterialSurfaceParamsVariable, "MaterialSurfaceParamsVariable");
		VALIDATE(MaterialRefractivityVariable, "MaterialRefractivityVariable");
		VALIDATE(MaterialRefractionIndexVariable, "MaterialRefractionIndexVariable");
		VALIDATE(MaterialHeightSamplesVariable, "MaterialHeightSamplesVariable");
		VALIDATE(MaterialBumpTypeVariable, "MaterialBumpTypeVariable");
		VALIDATE(MaterialShadingModelVariable, "MaterialShadingModelVariable");
		VALIDATE(MaterialHeightScaleVariable, "MaterialHeightScaleVariable");

		VALIDATE(ShadowMatrixVariable, "ShadowMatrixVariable");
		VALIDATE(LightMatrixVariable, "LightMatrixVariable");
		VALIDATE(LightTextureFlagVariable, "LightTextureFlagVariable");
		VALIDATE(LightTextureVariable, "LightTextureVariable");
		VALIDATE(FilterKernelVariable, "FilterKernelVariable");
		VALIDATE(ShadowMapFlagVariable, "ShadowMapFlagVariable");
		VALIDATE(LightTypeVariable, "LightTypeVariable");
		VALIDATE(LightOuterRadiusVariable, "LightOuterRadiusVariable");
		VALIDATE(LightInnerRadiusVariable, "LightInnerRadiusVariable");
		VALIDATE(LightRangeVariable, "LightRangeVariable");
		VALIDATE(LightDirVariable, "LightDirVariable");
		VALIDATE(LightPosVariable, "LightPosVariable");
		VALIDATE(LightColorVariable, "LightColorVariable");
		VALIDATE(AnimMatricesVariable, "AnimMatricesVariable");
		VALIDATE(NearZVariable, "NearZVariable");
		VALIDATE(FarZVariable, "FarZVariable");
		VALIDATE(ScreenSizeVariable, "ScreenSizeVariable");
		VALIDATE(CameraDirVariable, "CameraDirVariable");
		VALIDATE(CameraPosVariable, "CameraPosVariable");
		VALIDATE(OrthoProjectionVariable, "OrthoProjectionVariable");
		VALIDATE(ShadowOrthoProjectionVariable, "ShadowOrthoProjectionVariable");
		VALIDATE(ViewProjectionVariable, "ViewProjectionVariable");
		VALIDATE(OldViewProjectionVariable, "OldViewProjectionVariable");
		VALIDATE(WorldMatrixVariable, "WorldMatrixVariable");
		VALIDATE(TerrainWidgetTexCoordVariable, "TerrainWidgetPositionVariable");
		VALIDATE(TerrainWidgetRayVariable, "TerrainWidgetPositionVariable");
		VALIDATE(TerrainWidgetRadiusVariable, "TerrainWidgetRadiusVariable");
		VALIDATE(TerrainWidgetHardnessVariable, "TerrainWidgetHardnessVariable");
		return S_OK;
	}
#endif
}
