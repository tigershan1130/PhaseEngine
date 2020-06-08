//--------------------------------------------------------------------------------------
// File: ShadowMap.fxh
//
// Support for shadow mapping
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------

// Forward declarations of shading functions
float ComputeDiffuse(in float3 n, in float3 l);
float ComputeAttenuation( in float3 Pos, inout float3 vLight);

//--------------------------------------------------------------------------------------
// Linear step function
//--------------------------------------------------------------------------------------
float linstep(float min, float max, float v)
{
    return clamp((v - min) / (max - min), 0.0f, 1.0f);
}

//--------------------------------------------------------------------------------------
// Compute the Moments for VSM
//--------------------------------------------------------------------------------------
float2 ComputeMoments(float Dist)
{
	float dx = ddx(Dist);
	float dy = ddy(Dist);
	return float2(Dist, Dist*Dist + 0.25f*(dx*dx + dy*dy) );
}

//--------------------------------------------------------------------------------------
// Compute the Chebyshev upper bound
//--------------------------------------------------------------------------------------
float ChebyshevUpperBound(in float2 Moments, in float Mean)
{
    // Standard shadow map comparison
    float p = (Mean <= Moments.x);
    
    // Compute variance
    float Variance = Moments.y - (Moments.x * Moments.x);
    Variance = max(Variance, 0.000001f);
    
    // Compute probabilistic upper bound
    float d     = Mean - Moments.x;
    float p_max = Variance / (Variance + d*d);
    
    return max(p, p_max);
}

//--------------------------------------------------------------------------------------
// Light bleeding reduction
//--------------------------------------------------------------------------------------
float LightBleedingReduction(in float p)
{
    // Lots of options here if we don't care about being an upper bound.
    // Use whatever falloff function works well for your scene.
    return smoothstep(0.18f, 1.0f, p);
}



//--------------------------------------------------------------------------------------
// Computes shadow factor using VSM
//--------------------------------------------------------------------------------------
float ShadowVSM( in float2 Tex, in float Depth )
{
	// Compute the Chebyshev upper bound
	return LightBleedingReduction(ChebyshevUpperBound(g_txShadowMap.Sample(g_samAnisotropicClamp, Tex), Depth));
}

float ShadowVSMCube( in float3 Tex, in float Depth )
{
	// Compute the Chebyshev upper bound
	return LightBleedingReduction(ChebyshevUpperBound(g_txShadowMapCube.Sample(g_samBilinear, Tex), Depth));
}


// Shadow map pass input
struct PS_INPUT_SM
{
	float4 Pos   : SV_POSITION;
	float3 PosWS : TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex shader for shadow map rendering
//--------------------------------------------------------------------------------------
PS_INPUT_SM VS_ShadowMap( VS_INPUT input )
{
	PS_INPUT_SM output = (PS_INPUT_SM)0;
	
	// Output position and depth
	output.Pos = mul(input.Pos,g_mWorld);
	output.PosWS = output.Pos.xyz;
	output.Pos = mul(output.Pos,g_mLight);
	output.Pos = mul(output.Pos,g_mShadowProj);
	return output;
}

//--------------------------------------------------------------------------------------
// Vertex shader for shadow map rendering with animation
//--------------------------------------------------------------------------------------
PS_INPUT_SM VS_ShadowMapAnim( VS_INPUT_SKIN input )
{
	PS_INPUT_SM output = (PS_INPUT_SM)0;
	
	// Perform skinning
	SkinVertex(input, output.Pos);
	
	// Output position and depth
	output.Pos = mul(output.Pos,g_mWorld);
	output.PosWS = output.Pos.xyz;
	output.Pos = mul(output.Pos,g_mLight);
	output.Pos = mul(output.Pos,g_mShadowProj);
	return output;
}

//--------------------------------------------------------------------------------------
// Pixel shader for shadow map rendering
//--------------------------------------------------------------------------------------
float2 PS_ShadowMap( PS_INPUT_SM input) : SV_Target
{
	// Compute the moments
	return ComputeMoments(length(mul(float4(input.PosWS.xyz,1.0f),g_mLight)));
}




//--------------------------------------------------------------------------------------
// Shadow map rendering
//--------------------------------------------------------------------------------------
technique10 ShadowMap
{
    pass ShadowMap
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ShadowMap() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ShadowMap() ) );
        
        SetRasterizerState(MultisampleRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }

	pass ShadowMapAnim
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ShadowMapAnim() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ShadowMap() ) );
        
        SetRasterizerState(MultisampleRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
}