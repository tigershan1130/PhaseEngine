//--------------------------------------------------------------------------------------
// File: Forward.fxh
//
// Support for traditional forward rendering
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// Vertex shader for ZFill pass
//--------------------------------------------------------------------------------------
PS_INPUT VS_ZFill( VS_INPUT input )
{
	// Output position and texcoord
	PS_INPUT o;
	o.Pos = mul(input.Pos,g_mWorld);
	o.Pos = mul(o.Pos,g_mViewProjection);
	return o;
}

//--------------------------------------------------------------------------------------
// Pixel shader for ZFill pass
//--------------------------------------------------------------------------------------
float4 PS_ZFill( PS_INPUT input ) : SV_Target
{
	return 0;
}


//--------------------------------------------------------------------------------------
// Vertex shader for forward rendering
//--------------------------------------------------------------------------------------
PS_INPUT_TEX VS_Forward( VS_INPUT input )
{
	// Output position and texcoord
	PS_INPUT_TEX output;
	output.Pos = mul(input.Pos,g_mWorld);
	output.Pos = mul(output.Pos,g_mViewProjection);
	output.Tex = input.Tex;
	return output;
}

//--------------------------------------------------------------------------------------
// Vertex shader for forward rendering with skinning
//--------------------------------------------------------------------------------------
PS_INPUT_TEX VS_ForwardAnim( VS_INPUT_SKIN input )
{
	PS_INPUT_TEX output;

	// Perform skinning
	SkinVertex(input, output.Pos);

	// Output position and texcoord
	output.Pos = mul(output.Pos,g_mWorld);
	output.Pos = mul(output.Pos,g_mViewProjection);
	output.Tex = input.Tex;
	return output;
}


//--------------------------------------------------------------------------------------
// Pixel shader for forward rendering with no lighting
//--------------------------------------------------------------------------------------
float4 PS_ForwardUnlit( PS_INPUT_TEX input ) : SV_Target
{
	// Perform alpha testing
	AlphaTest(input.Tex);
		
	// Simple texturing
	float4 color = g_MaterialDiffuse;
	if(g_bMaterialTex[TEX_DIFFUSE1])
		color *= g_txMaterial[TEX_DIFFUSE1].Sample( g_samAnisotropic, input.Tex );
	color.a = 1-color.r;
	return color;
}

//--------------------------------------------------------------------------------------
// Pixel shader for forward rendering with no lighting
//--------------------------------------------------------------------------------------
float4 PS_ForwardUnlitTerrain( PS_INPUT_TEX input ) : SV_Target
{
	// Perform alpha testing
	AlphaTest(input.Tex);
	
	// Diffuse blend from 4 detail textures
	float4 Factor = g_txMaterial[TEX_DETAIL].Sample( g_samAnisotropic, input.Tex );
	float4 tColor = g_txMaterial[TEX_D1].Sample( g_samAnisotropic, input.Tex*g_MaterialDiffuse.x ) * Factor.x;
	if( g_bMaterialTex[TEX_D2] )
		tColor += g_txMaterial[TEX_D2].Sample( g_samAnisotropic, input.Tex*g_MaterialDiffuse.y ) * Factor.y;
	if( g_bMaterialTex[TEX_D3] )
		tColor += g_txMaterial[TEX_D3].Sample( g_samAnisotropic, input.Tex*g_MaterialDiffuse.z ) * Factor.z;
	if( g_bMaterialTex[TEX_D4] )
		tColor += g_txMaterial[TEX_D4].Sample( g_samAnisotropic, input.Tex*g_MaterialDiffuse.w ) * Factor.w;
	return tColor;
}

//--------------------------------------------------------------------------------------
// Forward rendering pixel shader for ambient pass
//--------------------------------------------------------------------------------------
void PS_ForwardAmbient( PS_INPUT_TEX input, out float4 oColor : SV_Target)
{
	oColor = g_Ambient * GetMaterialTexture(input.Tex);
}




//--------------------------------------------------------------------------------------
// Vertex Shader for lit forward rendering
//--------------------------------------------------------------------------------------
PS_INPUT_SHADE VS_ForwardLit( VS_INPUT input )
{
	PS_INPUT_SHADE output;
	
	// Output position and texcoord
	output.Pos = mul(input.Pos,g_mWorld);
	output.WPos = output.Pos;
	output.Pos = mul(output.Pos,g_mViewProjection);
	output.Tex = input.Tex;
	output.Norm = normalize(mul(input.Norm, (float3x3)g_mWorld));
	return output;
}




//--------------------------------------------------------------------------------------
// Forward rendering pixel shader
//--------------------------------------------------------------------------------------
void PS_ForwardLit( PS_INPUT_SHADE input, out float4 oColor : SV_Target)
{
	// Setup the shading params
	ShadingParams params = (ShadingParams)0;
	params.pos = input.WPos;
	params.view = normalize(g_CameraPos - params.pos);
	params.normal = GetNormal(input.Tex, input.Norm, input.WPos);
    params.diff = GetMaterialTexture(input.Tex).rgb;
	params.spec = g_MaterialSpecular.rgb * GetMaterialSpecularFactor(input.Tex);
	params.params = g_MaterialSurfaceParams;    
	params.model = g_MaterialShadingModel;
		
	// Lighting
	float3 diff, spec;
	ComputeLighting(params, diff, spec);
	oColor = float4(diff+spec, 1.0f);
}



struct PS_INPUT_INSTANCE
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float4 Color : TEXCOORD1;
};


//--------------------------------------------------------------------------------------
// Basic instancing vs
//--------------------------------------------------------------------------------------
PS_INPUT_INSTANCE VS_Instanced( VS_INPUT_INSTANCED input )
{
    // Get the world matrix
	float4 worldMatrix1 = g_Instances[input.InstanceId].world1;
	float4 worldMatrix2 = g_Instances[input.InstanceId].world2;
	float4 worldMatrix3 = g_Instances[input.InstanceId].world3;
    
    PS_INPUT_INSTANCE output;
    output.Pos = mul( input.Pos, DecodeMatrix(float3x4(worldMatrix1,worldMatrix2,worldMatrix3)) );
    output.Pos = mul( output.Pos, g_mViewProjection ); 
    output.Tex = input.Tex; 
    output.Color = g_Instances[input.InstanceId].color;
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel shader for forward rendering with no lighting
//--------------------------------------------------------------------------------------
float4 PS_Instanced( PS_INPUT_INSTANCE input ) : SV_Target
{
	// Perform alpha testing
	AlphaTest(input.Tex);
		
	// Simple texturing
	float4 color = g_MaterialDiffuse*input.Color;
	if(g_bMaterialTex[TEX_DIFFUSE1])
		color *= g_txMaterial[TEX_DIFFUSE1].Sample( g_samAnisotropic, input.Tex );
	color.a = 1-color.r;
	return color;
}



//--------------------------------------------------------------------------------------
// Forward rendering    
//--------------------------------------------------------------------------------------
technique10 Forward
{
    // ZFill pass with color writes disabled
    pass ZFill
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ZFill() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ZFill() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( NoColorWritesBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    // Wireframe rendering
    pass Wireframe
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Forward() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardUnlit() ) );
        
        SetRasterizerState(WireframeRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    // Wireframe rendering (no depth)
    pass WireframeNoDepth
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Forward() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardUnlit() ) );
        
        SetRasterizerState(WireframeRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }

	// Wireframe rendering with skinning
    pass WireframeAnim
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ForwardAnim() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardUnlit() ) );
        
        SetRasterizerState(WireframeRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    // Textured but not lit
    pass Unlit
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Forward() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardUnlit() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    // Textured but not lit
    pass UnlitBlend
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Forward() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardUnlit() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( BlendBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    // Textured but not lit (no depth)
    pass UnlitNoDepth
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Forward() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardUnlit() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }

	// Textured but not lit animated
    pass UnlitAnim
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ForwardAnim() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardUnlit() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }

	// Textured but not lit terrain
    pass UnlitTerrain
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Forward() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardUnlitTerrain() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    // Ambient lighting
    pass Ambient
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Forward() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardAmbient() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    // Shaded with one light per pass, additively blended
    pass Shade
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ForwardLit() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardLit() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( AdditiveBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthWritesDS, 0 );
    }
    
	// Ambient lighting
    pass AmbientMSAA
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Forward() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardAmbient() ) );
        
        SetRasterizerState(MultisampleRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
    
    // Shaded with one light per pass, additively blended
    pass ShadeMSAA
    {
        SetVertexShader( CompileShader( vs_4_0, VS_ForwardLit() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ForwardLit() ) );
        
        SetRasterizerState(MultisampleRS);
        SetBlendState( AdditiveBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthWritesDS, 0 );
    }  
    
    
    // Textured but not lit
    pass Instanced
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Instanced() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_Instanced() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( BlendBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }
  
}
