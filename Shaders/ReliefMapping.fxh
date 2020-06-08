//--------------------------------------------------------------------------------------
// File: ReliefMapping.fxh
//
// Relief mapping using relaxed cone steps
//  offeres many advantages over other methods of parallax mapping
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
// ray intersect depth map using binary cone space leaping
// depth value stored in alpha channel (black is at object surface)
// and cone ratio stored in blue channel
//--------------------------------------------------------------------------------------
void RM_RelaxedConeRayTrace( in float3 ViewTS, inout float2 Tex )
{
	// Start the ray at the current texture coords
	float3 RayPos = float3(Tex,0.0f);
	
	// Compute the ray direction and apply a height scale
	float3 RayDir = ViewTS;
	RayDir.z = abs(RayDir.z);
	RayDir.xy *= g_MaterialHeightScale;
	RayDir *= 1.0f/RayDir.z;
	
	// Constant number of steps
	const int cone_steps=15;
	const int binary_steps=8;	
	
	// Trace ray along cone steps
	float dist = length(RayDir.xy);
	for( int i=0;i<cone_steps;i++ )
	{
		// Get the height and cone ratio
		float2 tex = g_txMaterial[TEX_CSM].SampleLevel(g_samLinear, RayPos.xy, 0).zw;
		float height = saturate(tex.y - RayPos.z);
		
		// Cone ratio is tex.x
		RayPos += RayDir * (tex.x * height / (dist + tex.x));
	}

	// Now refine with the binary trace
	RayDir *= RayPos.z*0.5f;
	RayPos = float3(Tex,0.0f) + RayDir;
	for( int i=0;i<binary_steps;i++ )
	{
		RayDir *= 0.5f;
		if(RayPos.z < g_txMaterial[TEX_CSM].SampleLevel(g_samLinear, RayPos.xy, 0).w)
			RayPos+=RayDir;
		else
			RayPos-=RayDir;
	}
	
	// Set the final texture coord
	Tex = RayPos.xy;
}