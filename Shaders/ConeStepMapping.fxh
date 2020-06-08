//--------------------------------------------------------------------------------------
// File: ConeStepMapping.fxh
//
// Cone step mapping
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------


// CSM vars
#define csm_gain 1.0
#define csm_offset 0.0


//--------------------------------------------------------------------------------------
// Implementation of the GLSL fwidth() function for float2.
// Based on the GLSL reference documentation.
//--------------------------------------------------------------------------------------
float2 fwidth(in float2 p)
{
	return abs(ddx(p)) + abs(ddy(p));
}

//--------------------------------------------------------------------------------------
// Raytrace the heightfield using a cone step mapping variant with fixed steps
//--------------------------------------------------------------------------------------
void CSM_ConeStepRayTraceFixed( in float3 ViewTS, inout float2 Tex, in int2 TexSize )
{
   // Start the ray at the current tex coords
   float3 RayPos = float3(Tex, 0.0f);
   
   // Get the ray direction
   if(ViewTS.z==0)
		ViewTS.z = 0.001f;
   float3 RayDir;
   RayDir.xy = (-g_MaterialCSMScale * ViewTS.xy) * (1.0f/ViewTS.z);
   RayDir.z = 1.0f;
   
   // The "not Z" component of the direction vector (for a square cone)
   float iz = max(abs(RayDir.x),abs(RayDir.y));
   const float w = 1.2;
   
   // Find the initial location and height
   float4 t=g_txMaterial[TEX_CSM].SampleLevel(g_samLinear,RayPos.xy,0);
   RayPos += RayDir * w * (t.r - RayPos.z) / (iz/(t.g*t.g) + 1.0);
   
   // Trace the heightfield a fixed number of steps
   [unroll]
   for(int i=0; i<15; i++)
   {
		t=g_txMaterial[TEX_CSM].SampleLevel(g_samLinear,RayPos.xy, 0);
		RayPos += RayDir * w * (t.r - RayPos.z) / (iz/(t.g*t.g) + 1.0);
   }
      
   // all done
   Tex = RayPos.xy;
}



//--------------------------------------------------------------------------------------
// Raytrace the heightfield using cone step mapping
//--------------------------------------------------------------------------------------
void CSM_ConeStepRayTrace( in float3 ViewTS, inout float2 Tex, in int2 TexSize  )
{
   // Start the ray at the current tex coords
   float3 RayPos = float3(Tex, 0.0f);
   
   // Get the ray direction
   float3 RayDir;
   RayDir.xy = (-g_MaterialCSMScale * ViewTS.xy) * (1.0f/ViewTS.z);
   RayDir.z = 1.0f;
  
   // doing LOD based on the texture deltas
   float dist_factor = 0.05f * sqrt(length (fwidth (Tex))) * csm_gain + csm_offset / TexSize.x;
  
   // The "not Z" component of the direction vector (for a square cone)
   float iz = max(abs(RayDir.x),abs(RayDir.y));

   // Find the starting location and height
   float4 t = g_txMaterial[TEX_CSM].SampleLevel(g_samLinear, RayPos.xy, 0);
   float CR;
   int numSamples = 0;
   while (t.r > RayPos.z && numSamples<g_MaterialCSMSamples)
   {
		// Compute the cone ratio
		CR = t.g * t.g;

		// March the ray forward one step
		RayPos += RayDir * (dist_factor + (t.r - RayPos.z)*CR) / (iz + CR);

		// Clip the UV bounds to produce correct border rendering
		//clip(RayPos.x-0.0001);
		//clip(RayPos.y-0.0001);
		//clip(1.0-RayPos.x-0.0001);
		//clip(1.0-RayPos.y-0.0001);
		
		// Get the new location and height
		t = g_txMaterial[TEX_CSM].SampleLevel(g_samLinear, RayPos.xy, 0);
		numSamples++;
   }
   
   // Back out to where the cone was (remove the w component)
   float ht = (t.r - RayPos.z);
   dist_factor /= (iz + CR);
   RayPos -= RayDir * dist_factor;
   
   // Sample that location
   t = g_txMaterial[TEX_CSM].SampleLevel(g_samLinear, RayPos.xy, 0);
   float old_ht = t.r - RayPos.z;
   
   // Use linear interpolation to get the position
   RayPos += RayDir * dist_factor * (1.0 - clamp (ht / (ht - old_ht), 0.0, 1.0));
      
   // One last cone step to get the final position
   t = g_txMaterial[TEX_CSM].SampleLevel(g_samLinear, RayPos.xy, 0);
   RayPos += RayDir * (t.r - RayPos.z) / (iz/(t.g*t.g) + 1.0);
   
   // Update the tex coords
   Tex = RayPos.xy;
}


//--------------------------------------------------------------------------------------
// Compute the normal from gradients stored in the cone step map.
// This allows for an accurate normal for the actual pixel position
// based on the current height scale.  Using a normal map does not provide
// an accurate normal in this case.
//--------------------------------------------------------------------------------------
float3 CSM_GetNormal(in float3 pos, in float3 normal, inout float2 tex)
{
    // Get the texture dimensions
	int2 texSize;
	g_txMaterial[TEX_CSM].GetDimensions(texSize.x, texSize.y);
	
	// Build the tangent matrix
	float3x3 mTan = ComputeTangentFrame( normal, pos, tex);
	CSM_ConeStepRayTrace( mul(mTan,pos-g_CameraPos), tex, texSize );
   
   float4 t=g_txMaterial[TEX_CSM].SampleLevel(g_samLinear, tex, 0);
   float3 n = float3 ((t.ba-0.5) * (-g_MaterialCSMScale * texSize), 1.0);
   n = mul(n, mTan);
   return normalize(n);
}