//--------------------------------------------------------------------------------------
// File: Particle.fxh
//
// Instanced particle rendering
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Particle ps input
//--------------------------------------------------------------------------------------
struct PS_INPUT_PARTICLE
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
PS_INPUT_PARTICLE VS_Particle( VS_INPUT_INSTANCED input )
{
    // Get the world matrix
	float4 worldMatrix1 = g_Instances[input.InstanceId].world1;
	float4 worldMatrix2 = g_Instances[input.InstanceId].world2;
	float4 worldMatrix3 = g_Instances[input.InstanceId].world3;
    
    PS_INPUT_PARTICLE output;
    output.Pos = mul( input.Pos, DecodeMatrix(float3x4(worldMatrix1,worldMatrix2,worldMatrix3)) );
    output.WPos = output.Pos;
    output.Pos = mul( output.Pos, g_mViewProjection );
    output.Tex = input.Tex;
    output.Norm = normalize(float3(1, 1, 0)); //normalize(mul(input.Norm, (float3x3)g_mWorld));
    output.View = g_CameraPos-output.WPos;
    return output;
}



//--------------------------------------------------------------------------------------
// Pixel shader for GBuffer pass
//--------------------------------------------------------------------------------------
DL_Output PS_Particle( in PS_INPUT_PARTICLE input )
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
// Deferred rendering GBuffer fill passes
//--------------------------------------------------------------------------------------
technique10 Particle
{
    pass Particle
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Particle() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_Particle() ) );
        
		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DefaultDS, 0 );
    }
    
}