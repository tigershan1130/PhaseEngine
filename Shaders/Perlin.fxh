//--------------------------------------------------------------------------------------
// File: Perlin.fxh
//
// Improved perlin noise
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------

Texture2D g_txPerlin;


cbuffer cbClouds
{
	float g_PerlinLacunarity;
	float g_PerlinGain;
	float g_PerlinScale;
	float g_CloudCover;
	float g_CloudSharpness;
};

#define ONE 0.00390625
#define ONEHALF 0.001953125

float fade(float t) {
  //return t*t*(3.0-2.0*t); // Old fade
  return t*t*t*(t*(t*6.0-15.0)+10.0); // Improved fade
}
 
float noise(float3 P)
{
  float3 Pi = ONE*floor(P)+ONEHALF; 
                                 
  float3 Pf = P-floor(P);
  
  // Noise contributions from (x=0, y=0), z=0 and z=1
  float perm00 = g_txPerlin.Sample(g_samPoint, Pi.xy).a ;
  float3  grad000 = g_txPerlin.Sample(g_samPoint, float2(perm00, Pi.z)).rgb * 4.0 - 1.0;
  float n000 = dot(grad000, Pf);
  float3  grad001 = g_txPerlin.Sample(g_samPoint, float2(perm00, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n001 = dot(grad001, Pf - float3(0.0, 0.0, 1.0));

  // Noise contributions from (x=0, y=1), z=0 and z=1
  float perm01 = g_txPerlin.Sample(g_samPoint, Pi.xy + float2(0.0, ONE)).a ;
  float3  grad010 = g_txPerlin.Sample(g_samPoint, float2(perm01, Pi.z)).rgb * 4.0 - 1.0;
  float n010 = dot(grad010, Pf - float3(0.0, 1.0, 0.0));
  float3  grad011 = g_txPerlin.Sample(g_samPoint, float2(perm01, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n011 = dot(grad011, Pf - float3(0.0, 1.0, 1.0));

  // Noise contributions from (x=1, y=0), z=0 and z=1
  float perm10 = g_txPerlin.Sample(g_samPoint, Pi.xy + float2(ONE, 0.0)).a ;
  float3  grad100 = g_txPerlin.Sample(g_samPoint, float2(perm10, Pi.z)).rgb * 4.0 - 1.0;
  float n100 = dot(grad100, Pf - float3(1.0, 0.0, 0.0));
  float3  grad101 = g_txPerlin.Sample(g_samPoint, float2(perm10, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n101 = dot(grad101, Pf - float3(1.0, 0.0, 1.0));

  // Noise contributions from (x=1, y=1), z=0 and z=1
  float perm11 = g_txPerlin.Sample(g_samPoint, Pi.xy + float2(ONE, ONE)).a ;
  float3  grad110 = g_txPerlin.Sample(g_samPoint, float2(perm11, Pi.z)).rgb * 4.0 - 1.0;
  float n110 = dot(grad110, Pf - float3(1.0, 1.0, 0.0));
  float3  grad111 = g_txPerlin.Sample(g_samPoint, float2(perm11, Pi.z + ONE)).rgb * 4.0 - 1.0;
  float n111 = dot(grad111, Pf - float3(1.0, 1.0, 1.0));

  // Blend contributions along x
  float4 n_x = lerp(float4(n000, n001, n010, n011), float4(n100, n101, n110, n111), fade(Pf.x));

  // Blend contributions along y
  float2 n_xy = lerp(n_x.xy, n_x.zw, fade(Pf.y));

  // Blend contributions along z
  float n_xyz = lerp(n_xy.x, n_xy.y, fade(Pf.z));
 
  return n_xyz;
}

float turbulence(int octaves, float3 P, float lacunarity, float gain)
{	
  float sum = 0;
  float scale = 0.5;
  float totalgain = 1;
  for(int i=0;i<octaves;i++){
    sum += totalgain*noise(P*scale);
    scale *= lacunarity;
    totalgain *= gain;
  }
  float f = abs(sum);
  
  // Exponential cutoff
  float c = f - g_CloudCover;
  if(c<0)
	c=0;
  f = 255 - (pow(g_CloudSharpness,c) * 255);

  return f / 128.0;

}
