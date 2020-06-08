//--------------------------------------------------------------------------------------
// File: Honua.fxh
//
// Editor and debug rendering
//
// Coded by Nate Orr 2008
//--------------------------------------------------------------------------------------

// vars
cbuffer cbEditor
{
	float2 g_TWTexCoord;	// Texture coords for the widget depth lookup
	float3 g_TWRay;			// The view ray for reconstructing the world space widget position
	float g_TWRadius;		// Widget radius
	float g_TWHardness;		// Inner radius
}

//--------------------------------------------------------------------------------------
// Pixel shader for a 2d box
//--------------------------------------------------------------------------------------
float4 PS_2D( PS_INPUT input ) : SV_Target
{
	return float4(-0.2,-0.2,-0.2,0.85);
}

//--------------------------------------------------------------------------------------
// Pixel Shader for rendering the terrain widget
//--------------------------------------------------------------------------------------
float4 PS_Grid( in PS_INPUT_VIEW input ) : SV_Target
{
	if(DL_GetDepth(input.Pos.xy * g_ScreenSize.zw) > g_FarZ*0.9)
		discard;

	// Reconstruct the world space position using the depth buffer value
	float3 wPos = g_CameraPos + DL_GetDepth(input.Pos.xy * g_ScreenSize.zw)*normalize(input.vPos);
	
	// Grid distance factor
	int scale = 0;
	float thickness = 0.25f;
	float4 color = float4(1, 0, 0, 0);
	float dist = distance(wPos, g_CameraPos);
	if(dist<100)
	{
		scale = 10;
		color = float4(1, 0, 1, 0);
		thickness = 0.1f;
	}
	else
	{
		scale = 100;
		color = float4(0, 0, 1, 0);
		thickness = 0.4f;
	}
	
	// Grid lines to show scale
	float2 grid = wPos.xz-floor(wPos.xz);
	if( grid.x<=thickness || grid.y<=thickness )
	{
		if( (int)floor(wPos.x)%scale == 0 || (int)floor(wPos.z)%scale == 0)
			return color;
	}
	
	discard;
	return 0;
}


//--------------------------------------------------------------------------------------
// Pixel Shader for rendering the terrain widget
//--------------------------------------------------------------------------------------
float4 PS_TerrainWidget( in PS_INPUT_VIEW input ) : SV_Target
{
	if(DL_GetDepth(input.Pos.xy * g_ScreenSize.zw) > g_FarZ*0.9)
		discard;
	
	float3 sPos = g_CameraPos + DL_GetDepth(g_TWTexCoord.xy)*g_TWRay;
	sPos.y=0;
	
	// Reconstruct the world space position using the depth buffer value
	float3 wPos = g_CameraPos + DL_GetDepth(input.Pos.xy * g_ScreenSize.zw)*normalize(input.vPos);
	wPos.y = 0;
	
	float thickness = (distance(sPos, g_CameraPos)+5.0f) / 400.0f;

	float dist = distance(sPos, wPos);
		
	if(dist>=g_TWHardness && dist<=g_TWHardness+thickness)
		return float4(1,1,0,0);
	else if(dist>=g_TWRadius && dist<=g_TWRadius+thickness)
		return float4(1,0,0,0);
	discard;
	return 0;
}



//--------------------------------------------------------------------------------------
// Editor / debug passes
//--------------------------------------------------------------------------------------
technique10 Honua
{
    // 2D Selection box
    pass SelectionBox
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Ortho() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_2D() ) );
        
        SetRasterizerState(NoCullRS);
        SetBlendState( BlendBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }
    
    pass TerrainWidget
    {
        SetVertexShader( CompileShader( vs_4_0, VS_DeferredOrtho() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_TerrainWidget() ) );
        
		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DefaultDS, 0 );
    }
    
    pass Grid
    {
        SetVertexShader( CompileShader( vs_4_0, VS_DeferredOrtho() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_Grid() ) );
        
		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DefaultDS, 0 );
    }
}