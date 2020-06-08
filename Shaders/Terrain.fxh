//--------------------------------------------------------------------------------------
// File: Terrain.fxh
//
// GPU Geometry Clipmaps
//
// Coded by Nate Orr 2008
//--------------------------------------------------------------------------------------

#define NUM_LAYERS 16

// Terrain clipmap info
cbuffer cbTerrain
{
	float g_ClipmapScale;			// Scale of the currect clipmap level
	float g_ClipmapSize;			// Size of a clipmap
	float g_HeightmapSize;			// Size of the heightmap
	float g_HeightScale;			// Heightmap scale
	float2 g_ClipmapOffset;			// Offset into the main heightmap
	float2 g_ClipmapFix;			// Trim fix offset
	
	bool g_bTextureLayer[NUM_LAYERS];
	bool g_bNormalLayer[NUM_LAYERS];
};
Texture2D g_txClipmap;			// Clipmap for the current level
Texture2D g_txBlendClipmap;		// Clipmap for the current level

// Texture layers
Texture2DArray g_txTerrainTex;
Texture2DArray g_txTerrainNormal;

//--------------------------------------------------------------------------------------
// Pixel shader for clipmap updating
// The coarse level height is stored in the G channel
//--------------------------------------------------------------------------------------
void PS_ClipmapUpdate( PS_INPUT_TEX input, out float oHeight : SV_Target0 )
{
	// Get the scaled texture coords for this vertex
	float2 texScaled;
	texScaled = (input.Tex-0.5f)*g_ClipmapSize*g_ClipmapScale + g_ClipmapOffset - g_ClipmapScale;
	texScaled /= g_HeightmapSize;
	texScaled += 0.5f;
	oHeight = g_txMaterial[TEX_HEIGHT].SampleLevel(g_samBilinear, texScaled, 0).r;
}


struct PS_INPUT_TERRAIN
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float2 CTex: TEXCOORD1;
    float3 WPos: TEXCOORD2;
    float4 Blend : TEXCOORD3;
};


//--------------------------------------------------------------------------------------
// Vertex Shader for GBuffer pass for terrain using vertex texturing
//--------------------------------------------------------------------------------------
PS_INPUT_TERRAIN VS_GBufferTerrain( VS_INPUT_POS input )
{
    PS_INPUT_TERRAIN output;
    
    // Position based on the patch
    output.WPos.xz = input.Pos.xz*g_ClipmapScale + g_ClipmapOffset;
    
    // Compute the texture coords based on the position
    float isize = 1.0f / (g_ClipmapScale*g_ClipmapSize);
    output.CTex = (output.WPos.xz + g_ClipmapScale) * isize + 0.5f;
    
    // Adjust the position to fit inside the next coarser level
    output.WPos.xz += g_ClipmapFix;
    
    // Sample the height and normal at this location
    output.WPos.y = g_txClipmap.SampleLevel(g_samPoint, output.CTex, 0).r;
    output.Blend = g_txBlendClipmap.SampleLevel(g_samLinear, output.CTex, 0);

	// Compute the height for the next coarser level, and fix the T-Gaps
	float zc;
	float4 bc;
	int2 pos = int2(output.CTex*g_ClipmapSize);
    uint2 mod2 = pos%2;
    const float texel = 1.0f / g_ClipmapSize;
	if(mod2.x==0&&mod2.y==1)
	{
		float2 coordOffset = float2(0, texel);
		zc = (g_txClipmap.SampleLevel(g_samPoint, output.CTex + coordOffset, 0).r+ 
			  g_txClipmap.SampleLevel(g_samPoint, output.CTex - coordOffset, 0).r) * 0.5f;
		bc = (g_txBlendClipmap.SampleLevel(g_samPoint, output.CTex + coordOffset, 0)+ 
			  g_txBlendClipmap.SampleLevel(g_samPoint, output.CTex - coordOffset, 0)) * 0.5f;
		if(pos.x==0||pos.x==g_ClipmapSize-1)
			output.WPos.y = zc;
	}
	else if(mod2.x==1&&mod2.y==0)
	{
		float2 coordOffset = float2(texel, 0);
		zc = (g_txClipmap.SampleLevel(g_samPoint, output.CTex + coordOffset, 0).r+ 
			  g_txClipmap.SampleLevel(g_samPoint, output.CTex - coordOffset, 0).r) * 0.5f;
		bc = (g_txBlendClipmap.SampleLevel(g_samPoint, output.CTex + coordOffset, 0)+ 
			  g_txBlendClipmap.SampleLevel(g_samPoint, output.CTex - coordOffset, 0)) * 0.5f;
		if(pos.y==0||pos.y==g_ClipmapSize-1)
			output.WPos.y = zc;
	}
	else if(mod2.x==1&&mod2.y==1)
	{
		float2 coordOffsetX = float2(texel, 0);
		float2 coordOffsetY = float2(0, texel);
		zc = (g_txClipmap.SampleLevel(g_samPoint, output.CTex + coordOffsetX + coordOffsetY, 0).r+ 
			  g_txClipmap.SampleLevel(g_samPoint, output.CTex - coordOffsetX + coordOffsetY, 0).r+
			  g_txClipmap.SampleLevel(g_samPoint, output.CTex + coordOffsetX - coordOffsetY, 0).r+ 
			  g_txClipmap.SampleLevel(g_samPoint, output.CTex - coordOffsetX - coordOffsetY, 0).r) * 0.25f;
		bc = (g_txBlendClipmap.SampleLevel(g_samPoint, output.CTex + coordOffsetX + coordOffsetY, 0)+ 
			  g_txBlendClipmap.SampleLevel(g_samPoint, output.CTex - coordOffsetX + coordOffsetY, 0)+
			  g_txBlendClipmap.SampleLevel(g_samPoint, output.CTex + coordOffsetX - coordOffsetY, 0)+ 
			  g_txBlendClipmap.SampleLevel(g_samPoint, output.CTex - coordOffsetX - coordOffsetY, 0)) * 0.25f;
	}
	else
	{
		zc = output.WPos.y;
		bc = output.Blend;
	}
		
	// Compute the transition blending factors
	float wFactor = (g_ClipmapSize*g_ClipmapScale) / 10.0f;
	float ax = clamp( (abs(output.WPos.x-g_CameraPos.x) - ((g_ClipmapSize*g_ClipmapScale-1)/2 - wFactor - 1)) / wFactor, 0, 1);
	float ay = clamp( (abs(output.WPos.z-g_CameraPos.z) - ((g_ClipmapSize*g_ClipmapScale-1)/2 - wFactor - 1)) / wFactor, 0, 1);
		
	// Blend the transition heights
	output.WPos.y = lerp(output.WPos.y, zc, max(ax, ay));
	output.Blend = bc;
	
	// Scale the height
	output.WPos.y *= g_HeightScale;
	
	// Store the full tex coords in the normal
	float iheightsize = 1.0f / g_HeightmapSize;
	output.Tex = output.WPos.xz*iheightsize + 0.5;	
	   
    // Transform
    output.Pos = mul( float4(output.WPos,1), g_mViewProjection );
    return output;
}

//--------------------------------------------------------------------------------------
// Pixel shader for GBuffer pass for terrain rendering
//--------------------------------------------------------------------------------------
DL_Output PS_GBufferTerrain( in PS_INPUT_TERRAIN input )
{
 	// Skip if the tex coords are outside the bounds
 	if(input.Tex.x<0.0||input.Tex.y<0.0||input.Tex.x>1.0||input.Tex.y>1.0)
 		discard;
 	
 	// Init the output struct
	DL_Output oBuf = (DL_Output)0;
    
    // Specular factor
	DL_SetSpecularI(oBuf, 1.0);     
    
	// Compute the normal   
    float3 hPos[4];
    float htexel = 1.0 / g_HeightmapSize;
    float hh = g_txMaterial[TEX_HEIGHT].SampleLevel( g_samLinear, input.Tex,  0).r*g_HeightScale;
    hPos[0] = float3( 1.0f, g_txMaterial[TEX_HEIGHT].SampleLevel( g_samLinear, input.Tex + float2(htexel, 0),  0).r*g_HeightScale - hh,  0.0f);
	hPos[1] = float3( 0.0f, g_txMaterial[TEX_HEIGHT].SampleLevel( g_samLinear, input.Tex + float2(0, htexel),  0).r*g_HeightScale - hh,  1.0f);
	hPos[2] = float3(-1.0f, g_txMaterial[TEX_HEIGHT].SampleLevel( g_samLinear, input.Tex + float2(-htexel, 0), 0).r*g_HeightScale - hh,  0.0f);
	hPos[3] = float3( 0.0f, g_txMaterial[TEX_HEIGHT].SampleLevel( g_samLinear, input.Tex + float2(0, -htexel), 0).r*g_HeightScale - hh, -1.0f);
	float3 normal = cross(hPos[0], hPos[1]) + cross(hPos[1], hPos[2]) + cross(hPos[2], hPos[3]) + cross(hPos[3], hPos[0]);
	normal = -normalize(normal);        
        
    // Compute the transition blending factors
	float wFactor = (g_ClipmapSize*g_ClipmapScale) / 10.0f;
	float ax = clamp( (abs(input.WPos.x-g_CameraPos.x) - ((g_ClipmapSize*g_ClipmapScale-1)/2 - wFactor - 1)) / wFactor, 0, 1);
	float ay = clamp( (abs(input.WPos.z-g_CameraPos.z) - ((g_ClipmapSize*g_ClipmapScale-1)/2 - wFactor - 1)) / wFactor, 0, 1);
	float4 Factor = lerp(g_txBlendClipmap.SampleLevel( g_samLinear, input.CTex, 0 ), input.Blend, max(ax, ay));
	
	// Build the tangent matrix for normalmapping
	float3x3 mTan = ComputeTangentFrame( normal, input.WPos, input.Tex);
	
	// Get the layer id's
	float4 blend = Factor;//g_txBlendClipmap.SampleLevel( g_samLinear, input.CTex, 0 )*255;
	int4 id = g_txBlendClipmap.SampleLevel( g_samLinear, input.CTex, 0 )*255;
	//blend = id-blend;
	
	// Apply the normal mapping
	/*if( g_bNormalLayer[id.x] )
		normal += normalize( mul(g_txTerrainNormal[0].Sample( g_samAnisotropic, float3(input.Tex*g_MaterialDiffuse.x, id.x) ).xyz*2.0f-1.0f, mTan) );
	if( g_bNormalLayer[id.y] )
		normal += normalize( mul(g_txTerrainNormal[1].Sample( g_samAnisotropic, float3(input.Tex*g_MaterialDiffuse.y, id.y ).xyz*2.0f-1.0f, mTan) );
	if( g_bNormalLayer[id.z] )
		normal += normalize( mul(g_txTerrainNormal[2].Sample( g_samAnisotropic, float3(input.Tex*g_MaterialDiffuse.y, id.z ).xyz*2.0f-1.0f, mTan) );
	if( g_bNormalLayer[id.w] )
		normal += normalize( mul(g_txTerrainNormal[3].Sample( g_samAnisotropic, float3(input.Tex*g_MaterialDiffuse.y, id.w ).xyz*2.0f-1.0f, mTan) );*/
	DL_SetNormal(oBuf, normalize(normal));
    
    // Diffuse blend from 4 detail textures
	float4 oColor = 1.0f;
	float count = 0;
	if( g_bTextureLayer[id.x] )
	{
		oColor =  g_txTerrainTex.Sample( g_samAnisotropic, float3(input.Tex*g_MaterialDiffuse.x, id.x) );
		count++;
	}
	if( g_bTextureLayer[id.y] )
	{
		oColor += g_txTerrainTex.Sample( g_samAnisotropic, float3(input.Tex*g_MaterialDiffuse.y, id.y) );
		count++;
	}
	if( g_bTextureLayer[id.z] )
	{
		oColor += g_txTerrainTex.Sample( g_samAnisotropic, float3(input.Tex*g_MaterialDiffuse.z, id.z) );
		count++;
	}
	if( g_bTextureLayer[id.w] )
	{
		oColor += g_txTerrainTex.Sample( g_samAnisotropic, float3(input.Tex*g_MaterialDiffuse.w, id.w) );
		count++;
	}
	DL_SetDiffuse(oBuf, oColor/count);	
	//DL_SetDiffuse(oBuf, (float4)id);	

	// Depth
	DL_SetDepth(oBuf, distance(g_CameraPos, input.WPos));

	return oBuf;
}



//--------------------------------------------------------------------------------------
// Passes
//--------------------------------------------------------------------------------------
technique10 Terrain
{
    pass ClipmapUpdate
    {
        SetVertexShader( CompileShader( vs_4_0, VS_OrthoTex() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ClipmapUpdate() ) );
        
		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DisableDepthDS, 0 );
    }
    
    pass GBufferTerrain
    {
        SetVertexShader( CompileShader( vs_4_0, VS_GBufferTerrain() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_GBufferTerrain() ) );
        
		SetRasterizerState(DefaultRS);
		SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetDepthStencilState( DefaultDS, 0 );
    }
    
}