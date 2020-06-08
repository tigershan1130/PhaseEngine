//--------------------------------------------------------------------------------------
// File: Sky.fxh
//
// Support for skydome
//
// Coded by Nate Orr 2008
//--------------------------------------------------------------------------------------

cbuffer cbSky
{
	float PI = 3.141592653;
	float InnerRadius = 6356752.3142;
	float OuterRadius = 6356752.3142 * 1.015;
	float fScale = 1.0 / (6452.103598913 - 6356.7523142);
	float KrESun = 0.0025 * 30.0;
	float KmESun = 0.0010 * 30.0;
	float Kr4PI = 0.0025 * 4.0 * 3.141592653;
	float Km4PI = 0.0010 * 4.0 * 3.141592653;
	int tNumSamples = 50;
	int iNumSamples = 20;
	float2 v2dRayleighMieScaleHeight= float2( 0.25, 0.1 );
	float3 WavelengthMie = float3( pow( 0.650, -0.84 ), pow( 0.570, -0.84 ), pow( 0.475, -0.84 ) );
	float InvOpticalDepthN = 1.0 / 128.0;
	float3 v3HG = float3( 1.5f * ( (1.0f - (-0.995*-0.995)) / (2.0f + (-0.995*-0.995)) ), 1.0f + (-0.995*-0.995), 2.0f * -0.995 );
	float InvOpticalDepthNLessOne = 1.0 / ( 128.0 - 1.0 );
	float2 InvRayleighMieN = float2( 1.0 / 128.0, 1.0 / 64.0 );
	float2 InvRayleighMieNLessOne = float2( 1.0 / (128.0 - 1.0), 1.0 / (64.0 - 1.0) );
	float HalfTexelOpticalDepthN = float( 0.5 / 128.0 ); 
	float3 InvWavelength4 = float3( 1.0 / pow( 0.650, 4 ), 1.0 / pow( 0.570, 4 ), 1.0 / pow( 0.475, 4 ) );
}

float3 g_SunDir;

// Rayleigh / Mie scattering textures
#define SCATTER_RAYLEIGH 0
#define SCATTER_MIE 1
Texture2D g_txScattering[2];
Texture2D g_txCloudPlane;


//--------------------------------------------------------------------------------------
// Atmospheric Scattering
//--------------------------------------------------------------------------------------
float3 _HDR( float3 LDR)
{
	return 1.0f - exp( -2.0f * LDR );
}

float3 ToneMap( float3 HDR)
{
	return (HDR / (HDR + 1.0f));
}

float getMiePhase(float fCos, float fCos2)
{
	return v3HG.x * (1.0 + fCos2) / pow(v3HG.y - v3HG.z * fCos, 1.5);
}

float getRayleighPhase(float fCos2)
{
	return 0.75 * (1.0 + fCos2);
}

float2 GetDensityRatio( float fHeight )
{
	const float fAltitude = (fHeight - InnerRadius) * fScale;
	return exp( -fAltitude / v2dRayleighMieScaleHeight.xy );
}

float HitOuterSphere( float3 O, float3 Dir ) 
{
	float3 L = -O;

	float B = dot( L, Dir );
	float C = dot( L, L );
	float D = C - B * B; 
	float q = sqrt( OuterRadius * OuterRadius - D );
	return B+q;
}

//#include "Scattering.fxh"


float2 t( float3 P, float3 Px )
{
	// Out Scattering Integral / OpticalDepth
	float2 OpticalDepth = 0;

	float3 v3Vector =  Px - P;
	float fFar = length( v3Vector );
	float3 v3Dir = v3Vector / fFar;
			
	float fSampleLength = fFar / tNumSamples;
	float fScaledLength = fSampleLength * fScale;
	float3 v3SampleRay = v3Dir * fSampleLength;
	P += v3SampleRay * 0.5f;
			
	for(int i = 0; i < tNumSamples; i++)
	{
		float fHeight = length( P );
		OpticalDepth += GetDensityRatio( fHeight );
		P += v3SampleRay;
	}		

	OpticalDepth *= fScaledLength;
	return OpticalDepth;
}

struct PS_INPUT_SKY
{
    float4 Pos : SV_POSITION;
    float2 Tex0 : TEXCOORD0;
	float3 Tex1 : TEXCOORD1;
	float3 View : TEXCOORD2;
};

//--------------------------------------------------------------------------------------
// Skydome shaders
//--------------------------------------------------------------------------------------
PS_INPUT_SKY VS_Sky( VS_INPUT input )
{
    // Output the position
	PS_INPUT_SKY output;
	output.Pos = mul(input.Pos,g_mWorld);
	output.View = output.Pos-g_CameraPos;
    output.Pos = mul(output.Pos,g_mViewProjection);
    output.Pos.z = output.Pos.w;
    output.Pos *= 0.8f;
    output.Tex0 = input.Tex;
    output.Tex1 = -input.Pos.xyz;
    return output;
}

void PS_Sky(PS_INPUT_SKY input, out float4 oColor : SV_Target0)
{
	//oColor = g_txMaterial[TEX_DIFFUSE1].Sample( g_samLinear, input.Tex0 );

	float fCos = dot( g_SunDir, input.Tex1 ) / length( input.Tex1 );
	float fCos2 = fCos * fCos;
	
	float3 v3RayleighSamples = g_txScattering[SCATTER_RAYLEIGH].Sample( g_samBilinear, input.Tex0.xy );
	float3 v3MieSamples = g_txScattering[SCATTER_MIE].Sample( g_samBilinear, input.Tex0.xy );
	//float4 v3RayleighSamples = 0;
	//float4 v3MieSamples = 0;
	
	// Compute scattering (test)
	//ComputeScattering(input.Tex0, v3RayleighSamples, v3MieSamples);

	float3 Color = getRayleighPhase(fCos2) * v3RayleighSamples.rgb + getMiePhase(fCos, fCos2) * v3MieSamples.rgb;
	oColor =  float4( ToneMap(Color.rgb), 1 );
	
	//float f = g_txCloudPlane.Sample(g_samLinear, input.Tex0).r;
	//float f = turbulence(8, g_PerlinScale*(input.Tex1 + 10), g_PerlinLacunarity, g_PerlinGain);
	
	//oColor = lerp(oColor, f.rrrr, 0.25);
}


void PS_ComputeScattering( PS_INPUT_TEX input, out float4 oRayleigh : SV_Target0, out float4 oMie : SV_Target1 )
{
	float2 Tex0 = input.Pos * InvRayleighMieN.xy;
	 
	const float3 v3PointPv = float3( 0, InnerRadius, 0 );
	//const float dDistribution = (1.0 - exp(-0.5 * 10.0 * input.Tex.x ));
	//const float AngleY = 100.0 * dDistribution * PI / 180.0 ;
	const float AngleY = 100.0 * Tex0.x * PI / 180.0;
	const float AngleXZ = PI * Tex0.x;
	
	float3 v3Dir;
	v3Dir.x = sin( AngleY ) * cos( AngleXZ  );
	v3Dir.y = cos( AngleY );
	v3Dir.z = sin( AngleY ) * sin( AngleXZ  );
	v3Dir = normalize( v3Dir );

	// Pv -> Pa 
	float fFarPvPa = HitOuterSphere( v3PointPv , v3Dir );
	float3 v3Ray = v3Dir;

	// In Scattering
	float3 v3PointP = v3PointPv;
	float fSampleLength = fFarPvPa / iNumSamples;
	float fScaledLength = fSampleLength * fScale;
	float3 v3SampleRay = v3Ray * fSampleLength;
	v3PointP += v3SampleRay * 0.5f;
				
	float3 v3RayleighSum = 0;
	float3 v3MieSum = 0;

	for( int k = 0; k < iNumSamples; k++ )
	{
		float PointPHeight = length( v3PointP );

		// Density Ratio at Point P
		float2 DensityRatio = GetDensityRatio( PointPHeight );
		DensityRatio *= fScaledLength;

		// P->Viewer OpticalDepth
		float2 ViewerOpticalDepth = t( v3PointP, v3PointPv );
						
		// P->Sun OpticalDepth
		float dFarPPc = HitOuterSphere( v3PointP, g_SunDir );
		float2 SunOpticalDepth = t( v3PointP, v3PointP + g_SunDir * dFarPPc );

		// Calculate the attenuation factor for the sample ray
		float2 OpticalDepthP = SunOpticalDepth.xy + ViewerOpticalDepth.xy;
		float3 v3Attenuation = exp( - Kr4PI * InvWavelength4 * OpticalDepthP.x - Km4PI * OpticalDepthP.y );

		v3RayleighSum += DensityRatio.x * v3Attenuation;
		v3MieSum += DensityRatio.y * v3Attenuation;

		// Move the position to the center of the next sample ray
		v3PointP += v3SampleRay;
	}

	float3 Rayleigh = v3RayleighSum * KrESun;
	float3 Mie = v3MieSum * KmESun;
	Rayleigh *= InvWavelength4;
	Mie *= WavelengthMie;
	
	oRayleigh = float4( Rayleigh, 1 );
	oMie = float4( Mie, 1 );
}



void PS_CloudPlane(PS_INPUT_TEX input, out float4 oColor : SV_Target0)
{
	float f = g_txCloudPlane.Sample(g_samLinear, input.Tex).r;
	
	// f is the cloud density value from the fractal generator (range 0 to 255)
	/*f = f - 0.3f;//CloudObject->density; 
	if(f<0) f = 0;
	f = pow(2, f);
	//f = pow(CloudObject->sharpness, f);
	f = 255.0 - (f * 255.0);
	// f now is your cloud opacity cloud_alpha*/

	oColor = f;
	oColor.a = 1;
}


PS_INPUT_DEFAULT VS_SkyPlane( VS_INPUT input )
{
    // Output the position
	PS_INPUT_DEFAULT output;
	output.Pos = mul(input.Pos,g_mWorld);
	output.WPos = output.Pos;
    output.Pos = mul(input.Pos,g_mOrtho);
    output.Tex = input.Tex;
    return output;
}


void PS_ProjectCloudPlane(PS_INPUT_DEFAULT input, out float4 oColor : SV_Target0)
{
	float f = turbulence(8,(input.WPos-g_CameraPos)*6,2,0.5);
	
	// f is the cloud density value from the fractal generator (range 0 to 255)
	f = f - 0.1f;//CloudObject->density; 
	if(f<0) f = 0;
	f = pow(0.8, f);
	//f = pow(CloudObject->sharpness, f);
	f = 255.0 - (f * 255.0);
	// f now is your cloud opacity cloud_alpha

	oColor = 1-f;
	oColor.a = 1;
}


//--------------------------------------------------------------------------------------    
// Skydome rendering
//--------------------------------------------------------------------------------------
technique10 Sky
{    
    pass ComputeScattering
    {
        SetVertexShader( CompileShader( vs_4_0, VS_OrthoTex() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ComputeScattering() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }  
    
    pass Sky
    {
        SetVertexShader( CompileShader( vs_4_0, VS_Sky() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_Sky() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( DefaultBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DefaultDS, 0 );
    }  
    
    pass CloudPlane
    {
        SetVertexShader( CompileShader( vs_4_0, VS_SkyPlane() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS_ProjectCloudPlane() ) );
        
        SetRasterizerState(DefaultRS);
        SetBlendState( BlendBS, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepthDS, 0 );
    }  
}
