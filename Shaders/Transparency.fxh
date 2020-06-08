//--------------------------------------------------------------------------------------
// File: Transparency.fxh
//
// Support for transparent rendering
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Forward rendering pixel shader
//--------------------------------------------------------------------------------------
void PS_ForwardLitCBuf( PS_INPUT_SHADE input, out float4 oDiff : SV_Target0, out float4 oSpec : SV_Target1 )
{
	// Shading params
	ShadingParams shadeParams = (ShadingParams)0;
	shadeParams.pos = input.WPos;
	shadeParams.view = normalize(g_CameraPos-input.WPos);
	shadeParams.diff = g_MaterialDiffuse.rgb * g_LightColor.rgb;
	shadeParams.spec = g_MaterialSpecular.rgb * g_LightColor.rgb;
	shadeParams.model = SHADE_PHONG;
	shadeParams.params = g_MaterialSurfaceParams;
		
	// Compute the light vector and attenuation if a point light
	float attenuation = ComputeAttenuation(shadeParams.pos, shadeParams.light);
	clip(attenuation-0.001f);
	
	// Get the normal
	shadeParams.normal = GetNormal(input.Tex, input.Norm, shadeParams.pos);
	
	// Diffuse
	float NdotL = dot(shadeParams.normal,shadeParams.light );
	oDiff = float4( attenuation*abs(NdotL)*shadeParams.diff, 1.0f);
	    
	// Specular
	if(NdotL<0)
		oSpec = 0;
	else
	{
		 // Compute the half vector
		float3 half_vector = normalize(shadeParams.light + shadeParams.view);
	    
		// Compute the angle between the half vector and normal
		float  HdotN = abs(dot(half_vector, shadeParams.normal));
	 
		// Compute the specular colour
		oSpec = float4(shadeParams.spec * pow( HdotN, shadeParams.params.x ), 1.0f);	
		oSpec.rgb *= GetMaterialSpecularFactor(input.Tex);
	}
}


//--------------------------------------------------------------------------------------
// Forward rendering ambient pass
//--------------------------------------------------------------------------------------
void PS_ForwardFinal( PS_INPUT_SHADE input, out float4 oColor : SV_Target, uniform bool refract )
{
	// CBuffer tex coords
	float2 CTex = input.Pos.xy * g_ScreenSize.zw;
	
	// Material color
    float4 mColor = GetMaterialTexture(input.Tex);
		
	// Get the refraction color from the framebuffer texture
	if(refract)
	{
		// Get the normal vector
		float3 vNormal = GetNormal(input.Tex, input.Norm, input.WPos);
		
		// Scale the normal based on distance
		float dist = length(g_CameraPos - input.WPos);
		float scale = 0.06f / (dist+1);
			
		// Get the offset texture coords for the framebuffer lookup
		float2 offsetUV = CTex + vNormal.xy*scale;
		
		// Combine the refraction lookup with the material color
		mColor = lerp(mColor, g_txFrameBuffer.Sample(g_samPoint, offsetUV), g_MaterialRefractivity);
	}
	
	// Final color
	float4 Diff = g_txLightBuffer[0].Sample(g_samPoint, CTex);
	float4 Spec = g_txLightBuffer[1].Sample(g_samPoint, CTex);
	oColor = mColor*(g_Ambient+Diff) + Spec;
	oColor.a = g_MaterialTransparency;
}


//--------------------------------------------------------------------------------------
// Transparent rendering    
//--------------------------------------------------------------------------------------
technique10 Transparency
{
    // Shaded with one light per pass, alpha blended (back faces)
    pass TransparentShadeBack
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ForwardLit() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardLitCBuf() ) );
        
        SetRasterizerState(FrontCullRS);
        SetBlendState( Additive2BS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    // Shaded with one light per pass, alpha blended (front faces)
    pass TransparentShadeFront
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ForwardLit() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardLitCBuf() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( Additive2BS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    // Shaded with one light per pass, alpha blended (front faces)
    pass TransparentFinalFront
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ForwardLit() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardFinal(false) ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( BlendBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    // Shaded with one light per pass, alpha blended (back faces)
    pass TransparentFinalBack
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ForwardLit() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardFinal(false) ) );
        
        SetRasterizerState(FrontCullRS);
        SetBlendState( BlendBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    
    // Refraction using framebuffer distortion
    pass RefractFront
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ForwardLit() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardFinal(true) ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    pass RefractBack
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ForwardLit() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardFinal(true) ) );
        
        SetRasterizerState(FrontCullRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
}