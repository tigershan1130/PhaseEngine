//--------------------------------------------------------------------------------------
// File: Engine 10.fx
//
// Main engine effect file.  Handles all lighting and shading, as well as deferred
// rendering.  First all visible geometry is rendered into a set of "GBuffer"
// textures which store all the required information to perform lighting in a later
// pass.  This allows for only one geometry processing pass whereas standard
// one-pass-per light forward renderers require geometry processing passes for each
// light.  Lights are rendered as a quad that covered their area of influence with one pass per light.  
// Clipping areas reduce the affected pixels to those affected directly by the light.
//
// Coded by Nate Orr 2008
//--------------------------------------------------------------------------------------



#include "Common States.fxh"
#include "Common Vars.fxh"
#include "Common Shaders.fxh"
#include "Deferred.fxh"
#include "ShadowMap.fxh"
#include "Shading.fxh"
#include "ConeStepMapping.fxh"
#include "MaterialHelper.fxh"
#include "Forward.fxh"
#include "Transparency.fxh"
#include "Perlin.fxh"
#include "Atmosphere.fxh"
#include "EdgeDetection.fxh"
#include "HDR.fxh"
#include "AmbientOcclusion.fxh"
#include "PostFX.fxh"
#include "Terrain.fxh"
#include "Honua.fxh"
#include "Particle.fxh"
#include "Water.fxh"



//--------------------------------------------------------------------------------------
// GBuffer ps input
//--------------------------------------------------------------------------------------
struct PS_INPUT_GBUFFER
{
    float4 Pos  : SV_POSITION;
    float2 Tex  : TEXCOORD0;
    float3 WPos : TEXCOORD1;
    float3 View : TEXCOORD2;    
    float3 Norm : TEXCOORD3;
};


//--------------------------------------------------------------------------------------
// Vertex Shader for GBuffer pass
//--------------------------------------------------------------------------------------
PS_INPUT_GBUFFER VS_GBuffer( VS_INPUT input )
{
    PS_INPUT_GBUFFER output;
    output.Pos = mul( input.Pos, g_mWorld );
    output.WPos = output.Pos;
    output.Pos = mul( output.Pos, g_mViewProjection );
    output.Tex = input.Tex;
    output.Norm = normalize(mul(input.Norm, (float3x3)g_mWorld));
    output.View = g_CameraPos-output.WPos;
    return output;
}


//--------------------------------------------------------------------------------------
// Vertex Shader for GBuffer pass with animation
//--------------------------------------------------------------------------------------
PS_INPUT_GBUFFER VS_GBufferAnim( VS_INPUT_SKIN input )
{
    PS_INPUT_GBUFFER output;
    
	// Perform skinning
	SkinVertexNormal(input, output.Pos, output.Norm);

	output.Pos = mul( output.Pos, g_mWorld );
    output.WPos = output.Pos;
    output.Pos = mul( output.Pos, g_mViewProjection );
    output.Tex = input.Tex;
    output.Norm = normalize( mul(output.Norm, (float3x3)g_mWorld) );
    output.View = g_CameraPos-output.WPos;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel shader for GBuffer pass
//--------------------------------------------------------------------------------------
DL_Output PS_GBuffer( in PS_INPUT_GBUFFER input )
{
	AlphaTest(input.Tex);
	
	// Init the output struct
	DL_Output oBuf = (DL_Output)0;
    
    // Get the normal
    DL_SetNormal(oBuf, GetNormal(input.Tex, input.Norm, input.WPos));
        
    // Set the material ID
	DL_SetMaterialID(oBuf, g_MaterialID);

	// Diffuse
	DL_SetDiffuse(oBuf, GetMaterialTexture(input.Tex));
		
	// Specular factor
	DL_SetSpecularI(oBuf, GetMaterialSpecularFactor(input.Tex)); 

	// Depth
	DL_SetDepth(oBuf, length(input.View));

	return oBuf;
}


//--------------------------------------------------------------------------------------
// Vertex Shader for GBuffer cube map pass
//--------------------------------------------------------------------------------------
PS_INPUT_SHADE_FULL VS_GBufferCube( VS_INPUT input )
{
    PS_INPUT_SHADE_FULL output;
    output.Pos = mul( input.Pos, g_mWorld );
    output.WPos = output.Pos;
    output.Pos = mul( output.Pos, g_mViewProjection );
    output.Tex = input.Tex;
    output.Norm = mul(input.Norm, (float3x3)g_mWorld);
	output.Eye = normalize(g_CameraPos - output.WPos);
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel shader for GBuffer pass with cube mapping
//--------------------------------------------------------------------------------------
DL_Output PS_GBufferCube( in PS_INPUT_SHADE_FULL input )
{
	// Init the output struct
	DL_Output oBuf = (DL_Output)0;

    // Get the normal
	float3 n = GetNormal(input.Tex, input.Norm, input.WPos);
    DL_SetNormal(oBuf, n);
    
    // Set the material ID
	DL_SetMaterialID(oBuf, g_MaterialID);

	// Get the environment mapped color
	float3 v = input.Eye;
	float3 r = reflect(-v, n);
	//float3 i = refract(-v, n, 0.5f);
	float4 reflection = g_txCubeProbe.Sample(g_samLinear, r);
	//float4 refraction = pow(g_txCubeProbe.Sample(g_samLinear, i),2.2);
	//lerp(refraction, reflection, 0.3);
	
	// Get the specular factor
	float specFactor = GetMaterialSpecularFactor(input.Tex);
	float reflectivity = g_MaterialReflectivity * specFactor;
	
	// Diffuse
	float4 diffuse = GetMaterialTexture(input.Tex);
	DL_SetDiffuse(oBuf, lerp(diffuse, reflection, reflectivity) );
	
	// Specular factor
	DL_SetSpecularI(oBuf, specFactor);   

	// Depth
	DL_SetDepth(oBuf, length(g_CameraPos-input.WPos));
		
	return oBuf;
}


//--------------------------------------------------------------------------------------
// Pixel Shader for Shading pass
//--------------------------------------------------------------------------------------
void PS_Shade( PS_INPUT_VIEW input, out float4 oColor : SV_Target0 )
{
	// Get the tex coords from the screen pos
	float2 Tex = input.Pos.xy * g_ScreenSize.zw;

	// Reconstruct the world space position using the depth buffer value
	float3 wPos = g_CameraPos + DL_GetDepth(Tex)*normalize(input.vPos);
	
	
	// Read in the material properties
	matrix material;
	[unroll]
	for(int3 matIndex = int3(0,DL_GetMaterialID(Tex),0); matIndex.x<3; matIndex.x++)
		material[matIndex.x] = g_txEncodedMaterial.Load(matIndex);

	// Setup the shading params
	ShadingParams shadeParams = (ShadingParams)0;
	shadeParams.pos = wPos;
	shadeParams.view = normalize(g_CameraPos-wPos);
	shadeParams.diff = DL_GetDiffuse(Tex).rgb;
	shadeParams.normal = DL_GetNormal(Tex);
	shadeParams.spec = material[0].rgb * DL_GetSpecularI(Tex);
	shadeParams.model = (int)material[0].a;
	shadeParams.params = material[2];
	
	// Compute lighting
	float3 diff, spec;
	ComputeLighting(shadeParams, diff, spec);
	oColor = float4(diff + spec, 1.0f);
	oColor = clamp(oColor, 0, 5.0f);
}


//--------------------------------------------------------------------------------------
// Deferred rendering GBuffer fill passes
//--------------------------------------------------------------------------------------
technique10 GBuffer
{
    pass GBuffer
    {
        SetVertexShader( CompileShader( vs_4_0, VS_GBuffer() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_GBuffer() ) );
        
		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DefaultDS, 0 );
    }

	pass GBufferAnim
    {
        SetVertexShader( CompileShader( vs_4_0, VS_GBufferAnim() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_GBuffer() ) );
        
		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DefaultDS, 0 );
    }

	pass GBufferCubeMap
    {
        SetVertexShader( CompileShader( vs_4_0, VS_GBufferCube() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_GBufferCube() ) );
        
		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DefaultDS, 0 );
    }
}
    
//--------------------------------------------------------------------------------------    
// Deferred shading
//--------------------------------------------------------------------------------------
technique10 Shade
{
    // Occlusion query shaders for lights
    pass Occlusion
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Basic() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_Empty() ) );

        SetRasterizerState(FrontCullRS);
        SetBlendState( NoColorWritesBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthWritesDS, 0 );
    }
    
    // Screen aligned quad the size of the lightsource is rendered
    pass Shading
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Deferred() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_Shade() ) );

        SetRasterizerState(FrontCullRS);
        SetBlendState( AdditiveBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }
    
    // Fullscreen quad is drawn, used for directional lights and inside a light volume
    pass Fullscreen
    {
        SetVertexShader( CompileShader( vs_4_0, VS_DeferredOrtho() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_Shade() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( AdditiveBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }
}



    


    
  
    