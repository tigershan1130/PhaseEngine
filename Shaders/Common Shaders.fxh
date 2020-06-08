//--------------------------------------------------------------------------------------
// File: Common Shaders.fxh
//
// Contains all global shaders
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Shader I/O
//--------------------------------------------------------------------------------------

// Standard vertex
struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
    float3 Norm: NORMAL;
};

// Standard vertex with instancing
struct VS_INPUT_INSTANCED
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
    float3 Norm: NORMAL;
    uint   InstanceId : SV_InstanceID;
};

// vertex with position
struct VS_INPUT_POS
{
    float4 Pos : POSITION;
};

// vertex with pos and uv coords
struct VS_INPUT_POS_TEX
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
};

// Animated vertex
struct VS_INPUT_SKIN
{
    float4 Pos : POSITION;
    float2 Tex : TEXCOORD;
    float3 Norm: NORMAL;
	float4 Weights: WEIGHTS;
	uint4 Bones : BONES;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
};


struct PS_INPUT_TEX
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT_DEFAULT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 WPos: TEXCOORD1;
};

struct PS_INPUT_VIEW
{
	float4 Pos  : SV_POSITION;
	float3 vPos : TEXCOORD0;
};

struct PS_INPUT_VIEW_TEX
{
	float4 Pos  : SV_POSITION;
	float2 Tex : TEXCOORD0;
	float3 View : TEXCOORD1;
};

struct PS_INPUT_SHADE
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm: TEXCOORD1;
    float3 WPos: TEXCOORD2;
};

struct PS_INPUT_SHADE_FULL
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
    float3 Norm: TEXCOORD1;
    float3 WPos: TEXCOORD2;
	float3 Eye: TEXCOORD3;
};


//--------------------------------------------------------------------------------------
// Basic vertex shaders
//--------------------------------------------------------------------------------------

// Standard projection
PS_INPUT VS_Basic( VS_INPUT input )
{
    PS_INPUT output;
    output.Pos = mul( input.Pos, g_mWorld );
    output.Pos = mul( output.Pos, g_mViewProjection );
    return output;
}

// Ortho2D projection with tex coords
PS_INPUT_TEX VS_BasicTex( VS_INPUT input )
{
    PS_INPUT_TEX output;
    output.Pos = mul( input.Pos, g_mWorld );
    output.Pos = mul( output.Pos, g_mViewProjection ); 
    output.Tex = input.Tex; 
    return output;
}

// Ortho2D projection with tex coords
PS_INPUT_DEFAULT VS_Default( VS_INPUT input )
{
    PS_INPUT_DEFAULT output;
    output.Pos = mul( input.Pos, g_mWorld );
    output.WPos = output.Pos.xyz;
    output.Pos = mul( output.Pos, g_mViewProjection ); 
    output.Tex = input.Tex; 
    return output;
}

// Ortho2D projection
PS_INPUT VS_Ortho( VS_INPUT input )
{
    PS_INPUT output;
    output.Pos = mul( input.Pos, g_mOrtho );    
    return output;
}

// Ortho2D projection with tex coords
PS_INPUT_TEX VS_OrthoTex( VS_INPUT input )
{
    PS_INPUT_TEX output;
    output.Pos = mul( input.Pos, g_mOrtho );   
    output.Tex = input.Tex; 
    return output;
}


// Ortho2D projection
PS_INPUT VS_OrthoShadow( VS_INPUT input )
{
    PS_INPUT output;
    output.Pos = mul( input.Pos, g_mShadowOrtho );    
    return output;
}

// Empty pixel shader (occlusion culling etc)
void PS_Empty(PS_INPUT input, out float4 oColor : SV_Target0)
{
	oColor=0;
}


//--------------------------------------------------------------------------------------
// Vertex Shaders that prep for deferred passed that need reconstructed position
//--------------------------------------------------------------------------------------
// Standard projection
PS_INPUT_VIEW VS_Deferred( VS_INPUT input )
{
    PS_INPUT_VIEW output;
    output.Pos = mul( input.Pos, g_mWorld );
    output.vPos = output.Pos-g_CameraPos;
	output.Pos = mul( output.Pos, g_mViewProjection );
    return output;
}

// Ortho2D projection
PS_INPUT_VIEW VS_DeferredOrtho( VS_INPUT input )
{
    PS_INPUT_VIEW output;
    output.vPos = input.Norm;
    output.Pos = mul( input.Pos, g_mOrtho );    
    return output;
}


//--------------------------------------------------------------------------------------
// Performs skinned animation on the vertex position using the matrix pallete
//--------------------------------------------------------------------------------------
void SkinVertex( in VS_INPUT_SKIN input, inout float4 Pos )
{
	const int numInfluences = 2;
	Pos=0;
	float weights[4] = (float[4])input.Weights;
	int bones[4] = (int[4])input.Bones;
	for(int i=0; i<numInfluences; i++)
		Pos += weights[i] * mul( input.Pos, g_mBoneWorld[bones[i]] );
}

//--------------------------------------------------------------------------------------
// Performs skinned animation on the vertex position and normal using the matrix pallete
//--------------------------------------------------------------------------------------
void SkinVertexNormal( in VS_INPUT_SKIN input, inout float4 Pos, inout float3 Normal )
{
	const int numInfluences = 2;
	Pos=0;
	Normal=0;
	float weights[4] = (float[4])input.Weights;
	int bones[4] = (int[4])input.Bones;
	for(int i=0; i<numInfluences; i++)
	{
		Pos += weights[i] * mul( input.Pos, g_mBoneWorld[bones[i]] );
		Normal += weights[i] * mul( input.Norm, (float3x3)g_mBoneWorld[bones[i]] );
	}
}

//--------------------------------------------------------------------------------------
// Reconstruct a matrix
//--------------------------------------------------------------------------------------
float4x4 DecodeMatrix(float3x4 encodedMatrix)
{
	return float4x4(	float4(encodedMatrix[0].xyz,0),
						float4(encodedMatrix[1].xyz,0),
						float4(encodedMatrix[2].xyz,0),
						float4(encodedMatrix[0].w,encodedMatrix[1].w,encodedMatrix[2].w,1));
}