//--------------------------------------------------------------------------------------
// File: AmbientOcclusion.fx
//
// Screen space ambient occlusion
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Pixel shader for full scene ambient occlusion pass
//--------------------------------------------------------------------------------------
void PS_SSAO( PS_INPUT_VIEW input, out float oColor : SV_Target0 )
{
	// Samples distributed around a sphere
	const float3 g_SphereSamples[48] = 
	{
        float3( 1.00595,    -0.000276625,  0.0012901),
        float3(-0.711506,   -0.711115,     0.0012901),
        float3( 0.503747,   -0.711507,    -0.501922),
        float3( 0.504023,    0.711115,    -0.502199),
        float3( 0.00129056, -0.711115,     0.711506),
        float3( 0.00129038,  0.711506,     0.711115),
        float3( 0.00129014,  0.711115,    -0.711506),
        float3( 0.00110805, -1.00594,     -0.000716925),
        float3(-0.501922,   -0.711506,    -0.503747),
        float3( 0.504024,   -0.711115,     0.502198),
        float3( 0.711506,   -0.711115,    -0.00129092),
        float3( 0.711115,    0.711506,    -0.00129046),
        float3(-0.711506,    0.711115,    -0.00129043),
        float3(-0.503746,   -0.711507,     0.501921),
        float3( 0.501921,    0.711506,    -0.503747),
        float3(-0.00110823,  1.00595,     -0.000716925),
        float3(-0.504023,    0.711115,     0.502198),
        float3(-0.00129068, -0.711115,    -0.711507),
        float3(-0.503747,    0.711506,    -0.501922),
        float3( 0.502198,    0.711115,     0.504023),
        float3( 0.712223,   -0.000276625, -0.710398),
        float3( 0.00129032, -0.000276625, -1.00595),
        float3(-0.710398,   -0.000276625, -0.712223),
        float3(-1.00595,    -0.000276625, -0.00129035),
        float3(-0.712223,   -0.000276625,  0.710398),
        float3(-0.711506,   -0.711115,     0.0012901),
        float3( 0.503747,   -0.711507,    -0.501922),
        float3( 0.504023,    0.711115,    -0.502199),
        float3( 0.00129056, -0.711115,     0.711506),
        float3( 0.00129038,  0.711506,     0.711115),
        float3( 0.00129014,  0.711115,    -0.711506),
        float3( 0.00110805, -1.00594,     -0.000716925),
        float3(-0.501922,   -0.711506,    -0.503747),
        float3( 0.504024,   -0.711115,     0.502198),
        float3( 0.711506,   -0.711115,    -0.00129092),
        float3( 0.711115,    0.711506,    -0.00129046),
        float3(-0.711506,    0.711115,    -0.00129043),
        float3(-0.503746,   -0.711507,     0.501921),
        float3( 0.501921,    0.711506,    -0.503747),
        float3(-0.00110823,  1.00595,     -0.000716925),
        float3(-0.504023,    0.711115,     0.502198),
        float3(-0.00129068, -0.711115,    -0.711507),
        float3(-0.503747,    0.711506,    -0.501922),
        float3( 0.502198,    0.711115,     0.504023),
        float3( 0.712223,   -0.000276625, -0.710398),
        float3( 0.00129032, -0.000276625, -1.00595),
        float3(-0.710398,   -0.000276625, -0.712223),
        float3( 0.710398,   -0.000276625,  0.712223)
    }; 
 	
	// Compute the texture coords
	float2 Tex = (input.Pos.xy * g_ScreenSize.zw*2);
	
	// Linear depth
	float z = DL_GetDepth(Tex);
	
	// World space position
	float3 p = g_CameraPos + z*normalize(input.vPos);

	// World space normal
	//float3 normal = normalize(cross(ddx(p),ddy(p)));
	float3 normal = DL_GetNormal(Tex);

	// TODO: make global and add to editor
	// Params
	const float radius = 0.5;
	const int numSamples = 8;
	const float maxRadius = 0.3;

	// Use the random lookup texture to get a normal of
	// a plane that will be used to reflect the sample point
	float2 randomTex = (p.xz*p.yx*p.zy);
	float4 randomNormal = 2.0f * g_txRandom.Sample(g_samPointWrap, Tex + randomTex) - (float4)1.0f;
	float randomRadius = abs(randomNormal.a * radius);

	// Compute the occlusion term
	float AO = 0;
	for(int i=0; i<numSamples; i++)
	{
		// Reflect the point about the origin using the normal of a random
		// plane to get a random sample direction
		float3 randomDir = reflect(g_SphereSamples[i], randomNormal.rgb);
		randomDir *= sign(dot(randomDir,normal));
		
		// Transform sample point to screen aspace to get the texture coord
		float3 samplePos = p + randomRadius*randomDir;
		float4 screenPos = mul(float4(samplePos,1), g_mViewProjection);
		float2 sampleTex = 0.5f*(screenPos.xy/screenPos.w)+0.5f;
		sampleTex.y = 1.0-sampleTex.y;		

		// Get difference in linear depth at sample point
		float dZ = z - DL_GetDepth(sampleTex);
		
		// If the sample is at an acceptable location and within the maximum allowed radius
		if(dZ<maxRadius && dZ>0)
		{
			// Within this range, an occlusion factor with
			// inverse squared falloff is used.  This is
			// not physically correct but it produces
			// good results.
			AO += 1.0f /(1.0f+dZ*dZ*30);
		}
		else
			AO++;
	}

	// Average out the occlusion factor and give it a smoother range
	AO/=numSamples;
	oColor = saturate(AO);//*0.4+0.45);
}


//--------------------------------------------------------------------------------------    
// Ambient shading techniques
//--------------------------------------------------------------------------------------
technique10 AmbientOcclusion
{   
     // Screen space ambient occlusion is computed
    pass SSAO
    {
        SetVertexShader( CompileShader( vs_4_0, VS_DeferredOrtho() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_SSAO() ) );

        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }
}