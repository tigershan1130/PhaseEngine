//--------------------------------------------------------------------------------------
// File: MaterialHelper.fxh
//
// Helper functions
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------

//-----------------------------------
// Get the material texture color
//-----------------------------------
float4 GetMaterialTexture( in float2 Tex )
{
	float4 diff = g_MaterialDiffuse;
    if( g_bMaterialTex[TEX_DIFFUSE2] )
		diff *= 1.5*g_txMaterial[TEX_DIFFUSE1].Sample( g_samAnisotropic, Tex ) * g_txMaterial[TEX_DIFFUSE2].Sample( g_samAnisotropic, Tex*3.0f );
    else if( g_bMaterialTex[TEX_DIFFUSE1] )
		diff *= g_txMaterial[TEX_DIFFUSE1].Sample( g_samAnisotropic, Tex );
	return diff;
}

//-----------------------------------
// Get the specular factor
//-----------------------------------
float GetMaterialSpecularFactor( in float2 Tex )
{
	if(g_bMaterialTex[TEX_GLOSS])
		return 5*g_txMaterial[TEX_GLOSS].Sample( g_samLinear, Tex ).r; 
	return 1.0f;
}

//-----------------------------------
// Get the normal based on the bump mapping mode
//-----------------------------------
float3 GetNormal(inout float2 Tex, in float3 Normal, in float3 Position)
{
	// Cone step mapping
	if(g_MaterialBumpType == BUMP_CSM)
		return CSM_GetNormal(Position, Normal, Tex);

	// Parallax mapping with offset limiting
	else if(g_MaterialBumpType == BUMP_FAKE_PARALLAX)
    {
		const float sfHeightBias = 0.001;
		const float sfHeightScale = 0.05;
		float3x3 mTan = ComputeTangentFrame( Normal, Position, Tex);
		float3 viewTS = mul(mTan,Position-g_CameraPos);
		float fCurrentHeight = g_txMaterial[TEX_NORMAL].Sample( g_samAnisotropic, Tex ).a;	    
	    float fHeight = fCurrentHeight * sfHeightScale + sfHeightBias;
	    fHeight /= viewTS.z;
		float2 texSample = Tex + viewTS.xy * fHeight;

		return normalize( mul(g_txMaterial[TEX_NORMAL].Sample( g_samAnisotropic, texSample ).xyz*2.0f-1.0f, mTan) );
	}
	
    // Standard normal mapping
	else if(g_MaterialBumpType == BUMP_NORMAL)
    {
		float3x3 mTan = ComputeTangentFrame( Normal, Position, Tex);
		return normalize( mul(g_txMaterial[TEX_NORMAL].Sample( g_samLinear, Tex ).xyz*2.0f-1.0f, mTan) );
	}
    
    // Use the interpolated vertex normal	
    else return Normal;
}