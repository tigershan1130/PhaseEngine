//--------------------------------------------------------------------------------------
// File: Deferred 10.fx
//
// Deferred rendering functions for encapsulation
//
// Deferred shading gbuffers are organized as follows:
//
//		GBuffer[0].r = Normal.x
//		GBuffer[0].g = Normal.y
//		GBuffer[0].b = Normal.z
//		GBuffer[0].a = Material ID;
//
//		GBuffer[1].r = Diffuse.r
//		GBuffer[1].g = Diffuse.g
//		GBuffer[1].b = Diffuse.b
//		GBuffer[1].a = Material Specular Intensity
//
//		GBuffer[2].r = Depth
//
// Coded by Nate Orr 2008
//-------------------------------------------------------------------------------------


#define DS_NORMAL 0
#define DS_COLOR 1
#define DS_DEPTH 2

//--------------------------------------------------------------------------------------
// GBuffer textures
//--------------------------------------------------------------------------------------
Texture2D g_txGBuffer[3];


//--------------------------------------------------------------------------------------
// GBuffer output
//--------------------------------------------------------------------------------------
struct DL_Output
{
	float4 buf1 : SV_Target0; // Normal
	float4 buf2 : SV_Target1; // Color
	float4 buf3 : SV_Target2; // Depth
};

	
//--------------------------------------------------------------------------------------
// GBuffer access functions for MSAA
//--------------------------------------------------------------------------------------

// Depth
float DL_GetDepth(in float2 Tex)
{
	return g_txGBuffer[DS_DEPTH].Sample(g_samPoint,Tex);
}

// Normal
float3 DL_GetNormal(in float2 Tex)
{
	return g_txGBuffer[DS_NORMAL].Sample(g_samPoint,Tex).xyz;
}

// Material ID
int DL_GetMaterialID(in float2 Tex)
{
	return (int)(g_txGBuffer[DS_NORMAL].Sample(g_samPoint,Tex).a);
}

// Diffuse
float4 DL_GetDiffuse(in float2 Tex)
{
	return float4(g_txGBuffer[DS_COLOR].Sample(g_samPoint,Tex).rgb, 0);
}

// Specular
float DL_GetSpecularI(in float2 Tex)
{
	return g_txGBuffer[DS_COLOR].Sample(g_samPoint,Tex).a;
}


//--------------------------------------------------------------------------------------
// GBuffer set functions
//--------------------------------------------------------------------------------------

// Normal
void DL_SetNormal(inout DL_Output o, in float3 value)
{
	o.buf1.xyz = value;
}

// Material ID
void DL_SetMaterialID(inout DL_Output o, in int ID)
{
	o.buf1.w = ID;
}

// Diffuse
void DL_SetDiffuse(inout DL_Output o, in float4 value)
{
	o.buf2.xyz = value.xyz;
}

// Specular
void DL_SetSpecularI(inout DL_Output o, in float value)
{
	o.buf2.a = value;
}

// Depth
void DL_SetDepth(inout DL_Output o, in float value)
{
	o.buf3 = value;
}

