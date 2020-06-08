//--------------------------------------------------------------------------------------
// File: Water.fxh
//
// Support for water rendering
//
// Coded by Shan WenQin 2009
// Thanks to Nate Orr
//--------------------------------------------------------------------------------------

Texture2D dudvMap;
Texture2D ReflectionMap;
Texture2D NormalTexture;
float WaterHeight;
Texture2D RefractionMap;

// TWEAKABLE PARAMETERS 
// We can also set those value inside our C++ code.

float3 waterColor = {0.0f, 0.0f, 0.1f};
float kDistortion = .2;
float kRefraction = 0.02;
float WaveRepeat = 1.0f;



//////////// CONNECTOR STRUCTS //////////////////


struct WaterVertOut {
    float4 HPosition  : SV_POSITION;  // in clip space
    float4 WorldPos  : WORLDPOSITION;
    float2 UV  : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float Cycle : TEXCOORD2;
};

///////// SHADER FUNCTIONS ///////////////

WaterVertOut WaterVS(VS_INPUT IN) {
    WaterVertOut OUT = (WaterVertOut)0;
    
    float4 Po = float4(IN.Pos.xyz,1.0);
    float4x4 gWvp = mul(g_mWorld, g_mViewProjection);
    OUT.HPosition = mul(Po,gWvp);
    OUT.UV = IN.Tex.xy;
    OUT.Cycle = fmod(g_ElapsedTime, 100.0);
    OUT.Normal = IN.Norm;
	OUT.WorldPos = OUT.HPosition;
	
	return OUT;
}



float Fresnel(float NdotL, float fresnelBias, float fresnelPow)
{
	float facing = (1.0 - NdotL);
	return max(fresnelBias + (1.0 - fresnelBias) * pow(facing, fresnelPow), 0.0);
}

// Pixel Shaders

float4 WaterPS(WaterVertOut IN) : SV_TARGET
{
    

	float3 lightVec = -float3(0,-1,0); // hard coded light vec variable until it is fully implemented
	     
	float4 distOffset = dudvMap.Sample(g_samLinear, IN.Normal.xy) * IN.Cycle * kDistortion;
	float4 dudvColor = dudvMap.Sample(g_samLinear, float2(IN.UV*WaveRepeat + distOffset));
	dudvColor = normalize(dudvColor * 2.0 - 1.0) * kRefraction;

	float4 normalVector = NormalTexture.Sample(g_samLinear, float2(IN.Normal.xy*WaveRepeat + distOffset))*2.0-1.0;
	normalVector.a = 0.0;
	

    float2 ViewTexC = 0.5 * IN.WorldPos.xy / IN.WorldPos.w + float2( 0.5, 0.5 ) + dudvColor;
	ViewTexC.y	=	1.0f - ViewTexC.y;
	
	
	


	/*float3 reflectionColor =  ReflectionMap.Sample(g_samLinear,ViewTexC);
	float3 refractionColor = RefractionMap.Sample(g_samLinear, ViewTexC);

    float fresnel = max(dot(lightVec, normalVector), 0.6);
	

	float3 final;
    if(g_CameraPos.y > WaterHeight)
    {
	    final = refractionColor*(1-fresnel) + reflectionColor*(fresnel);
	}
	else
	{
		final = refractionColor;
	}

   
    return float4(final, 0.2f); */
    return float4(1.0f, 0.0f, 0.0f, 1.0f);
}

// Techqniues and State setting in DX10

technique10 Water
{
    
    pass RenderingWater
    {
        SetVertexShader( CompileShader( vs_4_0, WaterVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, WaterPS() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }

}