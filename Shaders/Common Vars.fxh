//--------------------------------------------------------------------------------------
// File: Common Vars.fxh
//
// Contains all global variables and definitions
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------

/////////////////////////////////
// Global constants
#define g_Epsilon 0.0003f

// Light types
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1
#define LIGHT_SPOT 2

// Animation
#define MAX_BONE_MATRICES 35

// Texture types
#define TEX_DIFFUSE1 0
#define TEX_DIFFUSE2 1
#define TEX_NORMAL 2
#define TEX_ALPHA 3
#define TEX_GLOSS 4
#define TEX_CSM 5
#define TEX_SIZE 6

// Terrain texturing types
#define TEX_D1 0
#define TEX_D2 1
#define TEX_D3 2
#define TEX_D4 3
#define TEX_DETAIL 4
#define TEX_HEIGHT 5

// Bumpmapping modes
#define BUMP_NONE 0
#define BUMP_NORMAL 1
#define BUMP_FAKE_PARALLAX 2
#define BUMP_CSM 3

// Lighting models
#define SHADE_PHONG 0
#define SHADE_COOK_TORRANCE 1 
#define SHADE_OREN_NAYAR 2
#define SHADE_STRAUSS 3
#define SHADE_WARD 4
#define SHADE_ASHIKHMIN_SHIRLEY 5

#define MAX_INSTANCE_CONSTANTS 1024

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

struct PerInstanceData
{
	float4 world1;
	float4 world2;
	float4 world3;
	float4 color;
};

// Instancing
cbuffer cbInstanceData
{
	PerInstanceData	g_Instances[MAX_INSTANCE_CONSTANTS];
}


// Per-object vars
cbuffer cbPerObject
{
	// Transforms
	matrix g_mWorld;
	matrix g_mOldWorld;
};

// Updated once per frame in FrameMove
cbuffer cbPerFrame
{
	// Camera vars
	float3 g_CameraPos;
	float3 g_CameraDir;

	matrix g_mViewProjection;
	matrix g_mOldViewProjection;

	// Elapsed time since last frame
	float g_ElapsedTime;
};


// Material
cbuffer cbGBufferPass
{
	// Material Properties
	float4 g_MaterialDiffuse;
	float4 g_MaterialSpecular;  // (Power is the w component)
	float4 g_MaterialEmissive;
	float4 g_MaterialSurfaceParams;
	float  g_MaterialTransparency;
	float  g_MaterialReflectivity;	
	float  g_MaterialRefractivity;	
	float  g_MaterialRefractionIndex;		
	float  g_MaterialCSMScale;
	int	   g_MaterialCSMSamples;
	int	   g_MaterialBumpType;
	int	   g_MaterialID;
	int	   g_MaterialShadingModel;
	
	// Texture booleans
	bool   g_bMaterialTex[TEX_SIZE];
};

// Light
cbuffer cbShadingPass
{
	// Light Properties
	float3 g_LightPos;
	float g_LightRange;
	float g_LightInnerRadius;
	float g_LightOuterRadius;
	float3 g_LightDir;
	float4 g_LightColor;	
	int g_LightType;
	bool g_bShadowMap;
	bool g_bLightTex;
};

// Shadow map pass
cbuffer cbShadowMap
{
	matrix g_mLight;
	matrix g_mShadowProj;
};


// Variables only modified on resize
cbuffer cbChangesOnResize
{
	// Screen size
	float4 g_ScreenSize;  //(xy is size, zw is 1/size)
	float g_FarZ;
	float g_NearZ;
	
	// Shadowmap projection matrix
	matrix g_mShadowOrtho;
	float g_ShadowMapTexelSize;

	// Scene ambient color
	float4 g_Ambient;
	
	// Orthographic projection for quads
	matrix g_mOrtho;
};

// Constant buffer for bone matrices
cbuffer cbAnimMatrices
{
    matrix g_mBoneWorld[MAX_BONE_MATRICES];
};





//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

// Material textures
Texture2D g_txMaterial[TEX_SIZE];

// Material data needed for shading is encoded into this texture
// (Specular,Power,0,0) (Emissive.rgba)
Texture2D g_txEncodedMaterial;

// Light and shadow textures
Texture2D g_txLight;
Texture2D<float2> g_txShadowMap;
TextureCube<float2> g_txShadowMapCube;

// Random noise texture
Texture2D g_txRandom;

// Environment probe cubemap
TextureCube g_txCubeProbe;

// Light accum. buffer
Texture2D g_txLightBuffer[2];

// Framebuffer texture
Texture2D g_txFrameBuffer;





//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

// Point filter
SamplerState g_samPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};
SamplerState g_samPointWrap
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

// Linear filter
SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

// Basic bilinear sampling
SamplerState g_samBilinear
{
    AddressU = Clamp;
    AddressV = Clamp;
    Filter = MIN_MAG_LINEAR_MIP_POINT;
};

// Anisotropic filter
SamplerState g_samAnisotropic
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState g_samAnisotropicClamp
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;
    AddressU = Clamp;
    AddressV = Clamp;
};


//--------------------------------------------------------------------------------------
// Alpha testing: automatically discards pixels that fail
//--------------------------------------------------------------------------------------
void AlphaTest(in float2 Tex)
{
	if(g_bMaterialTex[TEX_ALPHA] && g_txMaterial[TEX_ALPHA].Sample( g_samAnisotropic, Tex ).r < 0.1f)
		discard;
}

