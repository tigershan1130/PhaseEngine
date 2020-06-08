//--------------------------------------------------------------------------------------
// File: PostFX.fxh
//
// Post processing
//
// Coded by Nate Orr 2008
//--------------------------------------------------------------------------------------

// General textures to use for post processing
Texture2D g_txPostFX;
Texture2D g_txPostFX2;

// Contains weights for Gaussian blurring
#define MAX_GAUSS_SAMPLES 9
float g_GWeightsH[MAX_GAUSS_SAMPLES];
float g_GWeightsV[MAX_GAUSS_SAMPLES];
float g_GOffsetsH[MAX_GAUSS_SAMPLES];
float g_GOffsetsV[MAX_GAUSS_SAMPLES];

int g_FilterKernel;

struct PS_INPUT_POSTFX
{
	float4 Pos     : SV_POSITION;
	float2 Tex     : TEXCOORD0;
	float2 TexSize : TEXCOORD1;
};

//--------------------------------------------------------------------------------------
// Generic PostFX vertex shader
//--------------------------------------------------------------------------------------
PS_INPUT_POSTFX VS_PostFX( VS_INPUT input )
{
    PS_INPUT_POSTFX output;
    output.Pos = mul( input.Pos, g_mOrtho );   
    output.Tex = input.Tex; 
    int2 dim;
    g_txPostFX.GetDimensions(dim.x, dim.y);
    output.TexSize = 1.0f / (float2)dim;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel shader for ambient lighting
//--------------------------------------------------------------------------------------
void PS_Ambient( PS_INPUT_TEX input, out float4 oColor : SV_Target0 )
{
	// Ambient lighting
	oColor = DL_GetDiffuse(input.Tex) * g_Ambient;// * g_txPostFX.Sample(g_samPoint, input.Tex).r;
	//oColor = g_txPostFX.Sample(g_samPoint, input.Tex).rrrr;
}

//--------------------------------------------------------------------------------------
// Pixel shader for computing luminance from the HDRI
//--------------------------------------------------------------------------------------
void PS_VelocityMap( PS_INPUT_VIEW input, out float2 oColor : SV_Target )
{
	// Get the tex coords from the screen pos
	float2 Tex = input.Pos.xy * g_ScreenSize.zw;

	// Reconstruct the world space position using the depth buffer value
	float depth = DL_GetDepth(Tex);
	float3 wPos = g_CameraPos + depth*normalize(input.vPos);
	
	// Project using the previous frame's matrix
	float4 screenPos = mul( float4(wPos,1.0f), g_mViewProjection );
	float4 screenPosPrev = mul( float4(wPos,1.0f), g_mOldViewProjection );
	screenPos /= screenPos.w;
	screenPosPrev /= screenPosPrev.w;
	oColor = (screenPos.xy - screenPosPrev.xy) * 0.5f;
}


//--------------------------------------------------------------------------------------
// Pixel shader for computing luminance from the HDRI
//--------------------------------------------------------------------------------------
void PS_MotionBlur( PS_INPUT_TEX input, out float4 oColor : SV_Target )
{
	const int NumberOfPostProcessSamples = 8;
	const float PixelBlurConst = 0.25f;
	float depth = DL_GetDepth(input.Tex);
	if(depth < 0.1f)
	{
		oColor = g_txFrameBuffer.SampleLevel(g_samPoint, input.Tex, 0);
		return;
	}
	
	float2 pixelVelocity;
    
    // Get this pixel's current velocity and this pixel's last frame velocity
    // The velocity is stored in .r & .g channels
    float4 curFramePixelVelocity = g_txPostFX.Sample(g_samPoint, input.Tex);
    float4 lastFramePixelVelocity = g_txPostFX2.Sample(g_samPoint, input.Tex);

    // If this pixel's current velocity is zero, then use its last frame velocity
    // otherwise use its current velocity.  We don't want to add them because then 
    // you would get double the current velocity in the center.  
    // If you just use the current velocity, then it won't blur where the object 
    // was last frame because the current velocity at that point would be 0.  Instead 
    // you could do a filter to find if any neighbors are non-zero, but that requires a lot 
    // of texture lookups which are limited and also may not work if the object moved too 
    // far, but could be done multi-pass.
    float curVelocitySqMag = curFramePixelVelocity.r * curFramePixelVelocity.r +
                             curFramePixelVelocity.g * curFramePixelVelocity.g;
    float lastVelocitySqMag = lastFramePixelVelocity.r * lastFramePixelVelocity.r +
                              lastFramePixelVelocity.g * lastFramePixelVelocity.g;
                                   
    if( lastVelocitySqMag > curVelocitySqMag )
    {
        pixelVelocity.x =  lastFramePixelVelocity.r * PixelBlurConst;   
        pixelVelocity.y = -lastFramePixelVelocity.g * PixelBlurConst;
    }
    else
    {
        pixelVelocity.x =  curFramePixelVelocity.r * PixelBlurConst;   
        pixelVelocity.y = -curFramePixelVelocity.g * PixelBlurConst;    
    }
    
    // For each sample, sum up each sample's color in "Blurred" and then divide
    // to average the color after all the samples are added.
    float3 Blurred = 0;  
    for(float i = 0; i < NumberOfPostProcessSamples; i++)
    {   
        // Sample texture in a new spot based on pixelVelocity vector 
        // and average it with the other samples        
        float2 lookup = pixelVelocity * i / NumberOfPostProcessSamples + input.Tex;
        
		// Lookup the color at this new spot
		float4 Current = g_txFrameBuffer.SampleLevel(g_samPoint, lookup, 0);
    
		// Add it with the other samples
		Blurred += Current.rgb;
    }
    
    // Return the average color of all the samples
    oColor =  float4(Blurred / NumberOfPostProcessSamples, 1.0f); 
}


//--------------------------------------------------------------------------------------
// Horizontal gaussian blur
//--------------------------------------------------------------------------------------
void PS_GaussianBlurH( PS_INPUT_TEX input, out float4 oColor : SV_Target )
{
	oColor = 0;    
	[unroll]
	for(int i=0; i<MAX_GAUSS_SAMPLES; i++)
        oColor += g_txPostFX.Sample(g_samPoint, input.Tex + float2(g_GOffsetsH[i], 0.0f)) * g_GWeightsH[i];
}

//--------------------------------------------------------------------------------------
// Vertical gaussian blur
//--------------------------------------------------------------------------------------
void PS_GaussianBlurV( PS_INPUT_TEX input, out float4 oColor : SV_Target )
{
	oColor = 0;    
	[unroll]
    for(int i=0; i<MAX_GAUSS_SAMPLES; i++)
        oColor += g_txPostFX.Sample(g_samPoint, input.Tex + float2(0.0f, g_GOffsetsV[i])) * g_GWeightsV[i];
}


//--------------------------------------------------------------------------------------
// Box-filtered blur
//--------------------------------------------------------------------------------------
float4 PS_BoxBlur(PS_INPUT_POSTFX input, uniform bool horizontal) : SV_Target
{
    // Preshader
    float2 SampleOffset;
    if(horizontal)
		SampleOffset = input.TexSize * float2(1.0f, 0.0f);
	else
		SampleOffset = input.TexSize * float2(0.0f, 1.0f);
    float2 Offset = 0.5f * float(g_FilterKernel - 1) * SampleOffset;
    
    float2 BaseTexCoord = input.Tex - Offset;
    
    // NOTE: This loop can potentially be optimized to use bilinear filtering to take
    // two samples at a time, rather than handle even/odd filters nicely. However the
    // resulting special-casing required for different filter sizes will probably
    // negate any benefit of fewer samples being taken. Besides, this method is already
    // supidly-fast even for gigantic kernels.
    float2 Sum = float2(0, 0);
    for (int i = 0; i < g_FilterKernel; i++) {
        Sum += g_txPostFX.SampleLevel(g_samBilinear, BaseTexCoord + i * SampleOffset, 0);
    }
    
    return float4(Sum / (float)g_FilterKernel,0,0);
}

//--------------------------------------------------------------------------------------    
// Post Processing effects
//--------------------------------------------------------------------------------------
technique10 PostFX
{

	// Ambient lighting
	pass Ambient
	{
		SetVertexShader( CompileShader( vs_4_0, VS_OrthoTex() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS_Ambient() ) );

		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DisableDepthDS, 0 );
	}

	// Build a velocity map
	pass VelocityMap
	{
		SetVertexShader( CompileShader( vs_4_0, VS_DeferredOrtho() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS_VelocityMap() ) );

		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DisableDepthDS, 0 );
	}

	// Motion blur
	pass MotionBlur
	{
		SetVertexShader( CompileShader( vs_4_0, VS_OrthoTex() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS_MotionBlur() ) );

		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DisableDepthDS, 0 );
	}
	
	// Horizontal gaussian blur
	pass GaussianBlurH
	{
		SetVertexShader( CompileShader( vs_4_0, VS_OrthoTex() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS_GaussianBlurH() ) );

		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DisableDepthDS, 0 );
	}

	// Vertical gaussian blur
	pass GaussianBlurV
	{
		SetVertexShader( CompileShader( vs_4_0, VS_OrthoTex() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_4_0, PS_GaussianBlurV() ) );

		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DisableDepthDS, 0 );
	}
	
	pass BoxBlurH
    {
        SetVertexShader( CompileShader( vs_4_0, VS_PostFX() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_BoxBlur(true) ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }
    
    pass BoxBlurV
    {
        SetVertexShader( CompileShader( vs_4_0, VS_PostFX() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_BoxBlur(false) ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }
	
}