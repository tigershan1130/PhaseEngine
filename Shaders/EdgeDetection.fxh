//--------------------------------------------------------------------------------------
// File: EdgeDetection.fxh
//
// Edge detection filter post process
// Operates using the deferred shading GBuffers
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------
float edgeDetectScalar(float sx, float sy, float threshold)
{
    float dist = (sx*sx+sy*sy);
    float e = (dist > threshold)? 1: 0;
    return e;
}

//--------------------------------------------------------------------------
float SobelEdgeDetect(in float2 tex, in float2 pixelSize)
{
    // We need eight samples (the centre has zero weight in both kernels).
    float3 offset = {1, -1 , 0};

    float g00 = DL_GetDepth(tex + offset.yy * pixelSize);
    float g01 = DL_GetDepth(tex + offset.zy * pixelSize);
    float g02 = DL_GetDepth(tex + offset.xy * pixelSize);
    float g10 = DL_GetDepth(tex + offset.yz * pixelSize);
    float g11 = DL_GetDepth(tex + offset.zz * pixelSize);
    float g12 = DL_GetDepth(tex + offset.xz * pixelSize);
    float g20 = DL_GetDepth(tex + offset.yx * pixelSize);
    float g21 = DL_GetDepth(tex + offset.zx * pixelSize);
    float g22 = DL_GetDepth(tex + offset.xx * pixelSize);

    // Sobel in horizontal dir.
    float4 sx = 0;
    sx -= g00;
    sx -= g01 * 2;
    sx -= g02;
    sx += g20;
    sx += g21 * 2;
    sx += g22;
    
    // Sobel in vertical dir - weights are just rotated 90 degrees.
    float4 sy = 0;
    sy -= g00;
    sy += g02;
    sy -= g10 * 2;
    sy += g12 * 2;
    sy -= g20;
    sy += g22;
    
    //return g11;

    // In theory, dist should use a sqrt.  This is a common approx.
    //float greySx = 0.333 * (sx.r + sx.g + sx.b);
    //float greySy = 0.333 * (sy.r + sy.g + sy.b);
    //float eR = edgeDetectScalar(greySx, greySy);
    return edgeDetectScalar(sx, sy, 0.1f);
}


////////////////////////////
// Neighbor offset table
////////////////////////////
const static float2 g_EdgeOffsets[9] = {
  float2( 0.0f,  0.0f), // Center		 0
  float2(-1.0f, -1.0f), // Top Left		 1
  float2( 0.0f, -1.0f), // Top			 2
  float2( 1.0f, -1.0f), // Top Right	 3
  float2( 1.0f,  0.0f), // Right		 4
  float2( 1.0f,  1.0f), // Bottom Right  5
  float2( 0.0f,  1.0f), // Bottom		 6
  float2(-1.0f,  1.0f), // Bottom Left	 7
  float2(-1.0f,  0.0f), // Left			 8
};
  
  
//--------------------------------------------------------------------------------------
// Compute an edge weight.  This indicated how much a point is on an edge.  
// For deferred shading, makeshift antialiasing can be done by
// blending the final color for points that lie close to an edge
//--------------------------------------------------------------------------------------
float DL_GetEdgeWeight(in float2 tex, in float2 pixelSize)
{
	float Depth[9];
	float3 Normal[9];
	
	// Retrieve normal and depth data for all neighbors
	[unroll]
	for(int i=0; i<9; i++)
	{
		float2 uv = tex + g_EdgeOffsets[i] * pixelSize;
		Depth[i]  = DL_GetDepth(uv);	// Get depth from GBuffer
		Normal[i] = DL_GetNormal(uv);   // Get normal from GBuffer
	}

	// Compute Deltas in Depth
	float4 Deltas1;
	float4 Deltas2;
	Deltas1.x = Depth[1];		
	Deltas1.y = Depth[2];
	Deltas1.z = Depth[3];
	Deltas1.w = Depth[4];
	
	Deltas2.x = Depth[5];		
	Deltas2.y = Depth[6];
	Deltas2.z = Depth[7];
	Deltas2.w = Depth[8];
	
	// Compute absolute gradients from center
	Deltas1 = abs(Deltas1 - Depth[0]);
	Deltas2 = abs(Depth[0] - Deltas2);
	
	// Find min and max gradient, ensuring min!=0
	float4 maxDeltas = max(Deltas1, Deltas2);
	float4 minDeltas = max(min(Deltas1, Deltas2), 0.00001);
	
	// Compare changes in gradients, flagging ones that change
	// significantly.
	// How severe the change must be to get flagged is a function of the
	// minimum gradient.  It is not resolution dependant.  The constant
	// number here would change based on how the depth values are stored
	// and how sensitive the edge detection should be.
	float4 depthResults = step(minDeltas * 25.0, maxDeltas);
	
	// Compute change in the cosine of the angle between normals
	Deltas1.x = dot(Normal[1], Normal[0]);
	Deltas1.y = dot(Normal[2], Normal[0]);
	Deltas1.z = dot(Normal[3], Normal[0]);
	Deltas1.w = dot(Normal[4], Normal[0]);
	
	Deltas2.x = dot(Normal[5], Normal[0]);
	Deltas2.y = dot(Normal[6], Normal[0]);
	Deltas2.z = dot(Normal[7], Normal[0]);
	Deltas2.w = dot(Normal[8], Normal[0]);
	
	Deltas1 = abs(Deltas1 - Deltas2);
	
	// Compare change in the cosine of the angles, flagging changes
	// above some constant threshold.  The cosine of the angle is not a
	// linear function of the angle, so to have flagging be
	// independant of the angles involved, an arccos function would be 
	// required.
	float4 normalResults = step(0.4, Deltas1);
	
	normalResults = max(normalResults, depthResults);
	return (normalResults.x + normalResults.y + 
			normalResults.z + normalResults.w) * 0.25;
}


