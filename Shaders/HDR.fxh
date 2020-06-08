//--------------------------------------------------------------------------------------
// File: HDR.fxh
//
// HDRI Processing
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------

#define DOWNSAMPLE_SCALE 0.0625		// Squared scale factor for the downsampled image (eg 1/4 size : 0.25*0.25=0.0625)

Texture2D g_txHDR;							// HDR scene texture
Texture2D g_txHDRBloom;						// HDR bloom texture
Texture2D<float2> g_txLuminance;			// Luminance texture
Texture2D<float2> g_txAdaptedLuminance;		// Final luminance texture

//const float3 g_LuminanceVector = float3(0.2125f, 0.7154f, 0.0721f);
const float3 g_LuminanceVector = float3(0.3333f, 0.3333f, 0.3333f);
const float g_MaxLuminance = 1.00f;
const float g_LumMinP=0.01f;
const float g_LumAvgP=0.6f;
const float g_LumMaxP=0.99f;
const float g_MiddleGray = 0.38f;

//--------------------------------------------------------------------------------------
// HDR downscale input
//--------------------------------------------------------------------------------------
struct PS_INPUT_DOWNSCALE
{
	float4	 Pos		: SV_POSITION;
	float4x2 TexSamples : TEXCOORD0;
	float2   Tex		: TEXCOORD2;
};

//--------------------------------------------------------------------------------------
// HDR downscale vertex shader
//--------------------------------------------------------------------------------------
PS_INPUT_DOWNSCALE VS_DownscaleHDR( VS_INPUT input )
{
    PS_INPUT_DOWNSCALE output;
    output.Pos = mul( input.Pos, g_mOrtho );   
    
    // Compute the pixel sizes
	int2 dim;
	g_txHDR.GetDimensions(dim.x, dim.y);
	float2 texSize = 1.0f / (float2)dim;
    
    // Texture coords 
    output.TexSamples[0] = input.Tex + float2( texSize.x, 0);
    output.TexSamples[1] = input.Tex + float2(-texSize.x, 0);
    output.TexSamples[2] = input.Tex + float2(0,  texSize.y);
    output.TexSamples[3] = input.Tex + float2(0, -texSize.y);
    output.Tex = input.Tex;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Pixel shader for computing luminance from the HDRI
//--------------------------------------------------------------------------------------
void PS_DownScaleHDR( PS_INPUT_DOWNSCALE input, out float2 oColor : SV_Target)
{
	// Sample the luminance from surrounding pixels
	float sample1 = dot(g_txHDR.Sample(g_samPoint, input.Tex ).rgb, g_LuminanceVector);
	float sample2 = dot(g_txHDR.Sample(g_samPoint, input.TexSamples[0] ).rgb, g_LuminanceVector);
	float sample3 = dot(g_txHDR.Sample(g_samPoint, input.TexSamples[1] ).rgb, g_LuminanceVector);
	float sample4 = dot(g_txHDR.Sample(g_samPoint, input.TexSamples[2] ).rgb, g_LuminanceVector);
	float sample5 = dot(g_txHDR.Sample(g_samPoint, input.TexSamples[3] ).rgb, g_LuminanceVector);
	
	// Max
	oColor.g = max(sample1, max(sample2, max(sample3, min(sample5, sample4) ) ) );
	
	// Use the log of luminance for better result of averaging
	sample1 = log(sample1 + 0.0001f);
	sample2 = log(sample2 + 0.0001f);
	sample3 = log(sample3 + 0.0001f);
	sample4 = log(sample4 + 0.0001f);
	sample5 = log(sample5 + 0.0001f);
	
	// Avg
	oColor.r = (sample1+sample2+sample3+sample4+sample5) * 0.2f;
}


//--------------------------------------------------------------------------------------
// Luminance downscale vertex shader
//--------------------------------------------------------------------------------------
PS_INPUT_DOWNSCALE VS_DownscaleLum( VS_INPUT input )
{
    PS_INPUT_DOWNSCALE output;
    output.Pos = mul( input.Pos, g_mOrtho );   
    
    // Compute the pixel sizes
	int2 dim;
	g_txLuminance.GetDimensions(dim.x, dim.y);
	float2 texSize = 1.0f / (float2)dim;
    
    // Texture coords 
    output.TexSamples[0] = input.Tex + float2( texSize.x, 0);
    output.TexSamples[1] = input.Tex + float2(-texSize.x, 0);
    output.TexSamples[2] = input.Tex + float2(0,  texSize.y);
    output.TexSamples[3] = input.Tex + float2(0, -texSize.y);
    output.Tex = input.Tex;
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel shader for downscaling the luminance texture of min/max/avg values
//--------------------------------------------------------------------------------------
void PS_DownScaleLuminance( PS_INPUT_DOWNSCALE input, out float2 oColor : SV_Target)
{
	// Sample the luminance from surrounding pixels
	float2 sample1 = g_txLuminance.Sample(g_samPoint, input.Tex );
	float2 sample2 = g_txLuminance.Sample(g_samPoint, input.TexSamples[0] );
	float2 sample3 = g_txLuminance.Sample(g_samPoint, input.TexSamples[1] );
	float2 sample4 = g_txLuminance.Sample(g_samPoint, input.TexSamples[2] );
	float2 sample5 = g_txLuminance.Sample(g_samPoint, input.TexSamples[3] );
	
	// Avg
	oColor.r = sample1.r+sample2.r+sample3.r+sample4.r+sample5.r;
	oColor.r *= 0.2f;
	
	// Max
	oColor.g = max(sample1.g, max(sample2.g, max(sample3.g, min(sample5.g, sample4.g) ) ) );
}


//--------------------------------------------------------------------------------------
// Pixel shader for the adapting luminance over time
//--------------------------------------------------------------------------------------
void PS_AdaptLuminance( PS_INPUT_DOWNSCALE input, out float2 oColor : SV_Target )
{
	// Sample the luminance from surrounding pixels
	float2 sample1 = g_txLuminance.Sample(g_samPoint, input.Tex );
	float2 sample2 = g_txLuminance.Sample(g_samPoint, input.TexSamples[0] );
	float2 sample3 = g_txLuminance.Sample(g_samPoint, input.TexSamples[1] );
	float2 sample4 = g_txLuminance.Sample(g_samPoint, input.TexSamples[2] );
	float2 sample5 = g_txLuminance.Sample(g_samPoint, input.TexSamples[3] );
	
	// Avg
	oColor.r = sample1.r+sample2.r+sample3.r+sample4.r+sample5.r;
	oColor.r *= 0.2f;
	
	// Max
	oColor.g = max(sample1.g, max(sample2.g, max(sample3.g, min(sample5.g, sample4.g) ) ) );
	
	// Reverse the log
	oColor.r = exp(oColor.r);
	
	// Clamp to the correct ranges
	//oColor.r = clamp(oColor.r, g_LumMinP*g_MaxLuminance, g_LumMaxP*g_MaxLuminance);
	//oColor.g = clamp(oColor.g, g_LumMinP*g_MaxLuminance, g_LumMaxP*g_MaxLuminance);
	
	// Adapt from the previous frame's values based on a time factor
	oColor = lerp( g_txAdaptedLuminance.Load(int3(0, 0, 0)), oColor, 1.0 - pow(0.992, 40.0*g_ElapsedTime) );
}

//--------------------------------------------------------------------------------------
// Bright pass sends the brightest pixels to be bloomed
//--------------------------------------------------------------------------------------
void PS_BrightPass( PS_INPUT_TEX input, out float4 oBloom : SV_Target)
{
	// Compute the luminance for this pixel
	float4 sample = g_txHDR.Sample(g_samPoint, input.Tex);
	float lum = dot( sample.rgb, g_LuminanceVector );
	
	// Sample the scene luminance
	float2 lumScene = g_txAdaptedLuminance.Load(int3(0,0,0));
	
	// If the pixel is very bright, send to the bloom target
	if(lum > g_LumMaxP*lumScene.g)
		oBloom = sample;
	else
		oBloom = 0;
		
	// Add emissive light color
	int3 matIndex = int3(1,DL_GetMaterialID(input.Tex),0);
	oBloom += 2.0 * DL_GetDiffuse(input.Tex) * g_txEncodedMaterial.Load(matIndex);
}


//--------------------------------------------------------------------------------------
// Map the pixe; luminance based on the scene's
//--------------------------------------------------------------------------------------
float MapLuminance(float Lw, float2 lumSample)
{
	float Ld = 0.0f;
	if(Lw>0.0f)
	{
		float L = (g_MiddleGray/lumSample.r) * Lw;
		Ld = L * (1.0f + L / (lumSample.g*lumSample.g)) / (1.0f + L);
	}
	return clamp(Ld/Lw, 0.7f, 1.3f);	
}


//--------------------------------------------------------------------------------------
// Tonemapping shader input
//--------------------------------------------------------------------------------------
struct PS_INPUT_TONEMAP
{
	float4	 Pos		: SV_POSITION;
	float4x2 TexSamples : TEXCOORD0;
	float2   Tex		: TEXCOORD2;
	float2   Lum		: TEXCOORD3;
};

//--------------------------------------------------------------------------------------
// Pixel shader for tonemapping
//--------------------------------------------------------------------------------------
PS_INPUT_TONEMAP VS_ToneMap( VS_INPUT input )
{
    PS_INPUT_TONEMAP output;
    output.Pos = mul( input.Pos, g_mOrtho );   
    
    // Texture coords 
    output.TexSamples[0] = input.Tex + float2( g_ScreenSize.z, g_ScreenSize.w);
    output.TexSamples[1] = input.Tex + float2( -g_ScreenSize.z, g_ScreenSize.w);
    output.TexSamples[2] = input.Tex + float2( g_ScreenSize.z, -g_ScreenSize.w);
    output.TexSamples[3] = input.Tex + float2( -g_ScreenSize.z, -g_ScreenSize.w);
    output.Tex = input.Tex;
    
    // Luminance
    output.Lum = g_txAdaptedLuminance.Load(int3(0,0,0));
    
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel shader for tonemapping
//--------------------------------------------------------------------------------------
void PS_ToneMap( PS_INPUT_TONEMAP input, out float4 oColor : SV_Target)
{
	// Makeshift AA using an edge filter
	oColor = g_txHDR.Sample(g_samPoint, input.Tex);
	float edgeFactor = DL_GetEdgeWeight(input.Tex, g_ScreenSize.zw);
	if(edgeFactor>0.0)
	{
		float3 edgeColor = oColor.rgb;
		for(int i=0; i<4; i++)
			edgeColor += g_txHDR.SampleLevel(g_samLinear, input.TexSamples[i], 0).rgb;

		float factor = min(saturate(1 - (DL_GetDepth(input.Tex) / 40)), 0.8f);
		oColor.rgb = lerp(oColor.rgb, edgeColor*0.2f, factor);
	}

	// Tonemapping
	oColor.rgb *= MapLuminance( dot(g_LuminanceVector, oColor.rgb), input.Lum );
	
	// Bloom
	//oColor.rgb += 0.2f * g_txHDRBloom.Sample(g_samLinear, input.Tex).rgb;
}




//--------------------------------------------------------------------------------------    
// HDR Processing
//--------------------------------------------------------------------------------------
technique10 HDR
{
 	// Luminance is downscaled
    pass DownScale
    {
        SetVertexShader( CompileShader( vs_4_0, VS_DownscaleHDR() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_DownScaleHDR() ) );

        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }
    
    // Luminance is downscaled
    pass DownScaleLuminance
    {
        SetVertexShader( CompileShader( vs_4_0, VS_DownscaleLum() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_DownScaleLuminance() ) );

        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }

	// Adapt the luminance
    pass Adapt
    {
        SetVertexShader( CompileShader( vs_4_0, VS_DownscaleLum() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_AdaptLuminance() ) );

        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }

    // Luminance is downscaled
    pass BrightPass
    {
        SetVertexShader( CompileShader( vs_4_0, VS_OrthoTex() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_BrightPass() ) );

        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }

    // HDRI is tone mapped to the range 0.0-1.0
    pass ToneMap
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ToneMap() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ToneMap() ) );

        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }
}