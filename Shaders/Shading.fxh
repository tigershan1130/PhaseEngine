//--------------------------------------------------------------------------------------
// File: Shading.fxh
//
// Support for different lighting models
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------

// Parameters needed to perform shading
struct ShadingParams
{
	float3 pos;
	float3 view;
	float3 normal;
	float3 light;
	float3 diff;
	float3 spec;
	float4 params;
	int model;
};

//-----------------------------------
// tangent frame functions
//-----------------------------------
float3x3 ComputeTangentFrame( float3 N, float3 p, float2 uv )
{
    // get edge vectors of the pixel triangle
    float3 dp1 = ddx( p );
    float3 dp2 = ddy( p );
    float2 duv1 = ddx( uv );
    float2 duv2 = ddy( uv );

    // solve the linear system
    float3x3 M = float3x3( dp1, dp2, cross( dp1, dp2 ) );
    float2x3 inversetransposeM = float2x3( cross( M[1], M[2] ), cross( M[2], M[0] ) );
    float3 T = mul( float2( duv1.x, duv2.x ), inversetransposeM );
    float3 B = mul( float2( duv1.y, duv2.y ), inversetransposeM );

    // construct tangent frame 
    return float3x3( normalize(T), normalize(B), N );
}


//--------------------------------------------------------------------------------------
// Compute light attenuation
//--------------------------------------------------------------------------------------
float ComputeAttenuation( in float3 Pos, inout float3 vLight)
{
	// Directional lights are not attenuated
	if( g_LightType == LIGHT_DIRECTIONAL )
	{
		vLight = -g_LightDir;
		return 1.0f;
	}

	// Compute the light vector and the inverse distance
	vLight = g_LightPos - Pos;
	float d =  1.0f/length(vLight);
	vLight = vLight*d;
	
	// Compute light attenuation
	d = (g_LightRange*d) - lerp(0.6f, 0, g_LightRange*d*d);
	
	// Compute spot attenuation
	if(g_LightType == LIGHT_SPOT)
		return d * smoothstep( g_LightOuterRadius, g_LightInnerRadius, dot( g_LightDir, -vLight ) );
	return d;
}

//--------------------------------------------------------------------------------------
// Blinn-Phong shading model
// Specular power is params.x
//--------------------------------------------------------------------------------------
void Blinn_Phong(in ShadingParams input, inout float3 oDiff, inout float3 oSpec)
{    
    // Compute the diffuse term
    float NdotL = dot( input.normal, input.light );
    oDiff = input.diff * NdotL;
    
    // Compute the half vector
    float3 half_vector = normalize(input.light + input.view);
    
    // Compute the angle between the half vector and normal
    float  HdotN = saturate( dot( half_vector, input.normal ) );
 
    // Compute the specular colour
    oSpec = input.spec * pow( HdotN, input.params.x );
}

//--------------------------------------------------------------------------------------
// Cook-Torrance lighting model
// Roughness->SurfaceParams.x
// Reflection at normal incidence->SurfaceParams.y
//--------------------------------------------------------------------------------------
void Cook_Torrance(in ShadingParams input, inout float3 oDiff, inout float3 oSpec)
{    
    // Params from the material texture
    float roughness_value = input.params.x;
    float ref_at_norm_incidence = input.params.y;
    
    // Compute any aliases and intermediary values
    // -------------------------------------------
    float3 half_vector = normalize( input.light + input.view );
    float NdotL        = saturate( dot( input.normal, input.light ) );
    float NdotH        = saturate( dot( input.normal, half_vector ) );
    float NdotV        = saturate( dot( input.normal, input.view ) );
    float VdotH        = saturate( dot( input.view, half_vector ) );
    float r_sq         = roughness_value * roughness_value; 
 
 
    // Evaluate the geometric term
    // --------------------------------
    float geo_numerator   = 2.0f * NdotH;
    float geo_denominator = VdotH;
 
    float geo_b = (geo_numerator * NdotV ) / geo_denominator;
    float geo_c = (geo_numerator * NdotL ) / geo_denominator;
    float geo   = min( 1.0f, min( geo_b, geo_c ) );
 
  
    // Now evaluate the roughness term
    // -------------------------------
    float roughness_a = 1.0f / ( 4.0f * r_sq * pow( NdotH, 4 ) );
    float roughness_b = NdotH * NdotH - 1.0f;
    float roughness_c = r_sq * NdotH * NdotH;

    float roughness = roughness_a * exp( roughness_b / roughness_c ); 
 
 
    // Next evaluate the Fresnel value
    // -------------------------------
    float fresnel = pow( 1.0f - VdotH, 5.0f );
    fresnel *= ( 1.0f - ref_at_norm_incidence );
    fresnel += ref_at_norm_incidence; 
 
 
    // Put all the terms together to compute
    // the specular term in the equation
    // -------------------------------------
    float3 Rs_numerator   = ( fresnel * geo * roughness );
    float Rs_denominator  = NdotV * NdotL;
    float3 Rs             = Rs_numerator/ Rs_denominator; 
 
 
    // Put all the parts together to generate
    // the final colour
    // --------------------------------------
    oDiff = input.diff*NdotL;
    oSpec = input.spec*Rs; 
}


//--------------------------------------------------------------------------------------
// Oren-Nayar lighting model
// Roughness->SurfaceParams.x
//--------------------------------------------------------------------------------------
void Oren_Nayar(in ShadingParams input, inout float3 oDiff, inout float3 oSpec)
{
    // Param from the material texture
    float fRoughness = input.params.x;
    
    // Compute the other aliases
    float gamma   = dot
                    ( 
                        input.view - input.normal * dot( input.view, input.normal ), 
                        input.light - input.normal * dot( input.light, input.normal ) 
                    );
 
    float rough_sq = fRoughness * fRoughness;
 
    float A = 1.0f - 0.5f * (rough_sq / (rough_sq + 0.57f));
 
    float B = 0.45f * (rough_sq / (rough_sq + 0.09));
 
    float alpha = max( acos( dot( input.view, input.normal ) ), acos( dot( input.light, input.normal ) ) );
    float beta  = min( acos( dot( input.view, input.normal ) ), acos( dot( input.light, input.normal ) ) );

    float C = sin(alpha) * tan(beta);
 
    float3 final = (A + B * max( 0.0f, gamma ) * C);
 
    oDiff = input.diff * max( 0.0f, dot( input.normal, input.light ) ) * final;
    oSpec = (float3)0;
}

//--------------------------------------------------------------------------------------
// Oren-Nayar lighting model
// Roughness->SurfaceParams.x
//--------------------------------------------------------------------------------------
void Oren_Nayar_Complex(in ShadingParams input, inout float3 oDiff, inout float3 oSpec)
{
    const float PI = 3.14159f;
    
    // Param from the material texture
    float fRoughness = input.params.x;
 
    // Compute the other aliases
    float NdotL = dot( input.light, input.normal );
    float NdotV = dot( input.view, input.normal );
    float alpha    = max( acos( dot( input.view, input.normal ) ), acos( dot( input.light, input.normal ) ) );
    float beta     = min( acos( dot( input.view, input.normal ) ), acos( dot( input.light, input.normal ) ) );
    float gamma    = dot( input.view - input.normal * NdotV, input.light - input.normal * NdotL );
    float rough_sq = fRoughness * fRoughness;
 
    float C1       = 1.0f - 0.5f * ( rough_sq / ( rough_sq + 0.33f ) );
 
    float C2       = 0.45f * ( rough_sq / ( rough_sq + 0.09 ) );
    if( gamma >= 0 )
    {
        C2 *= sin( alpha );
    }
    else
    {
        C2 *= ( sin( alpha ) - pow( (2 * beta) / PI, 3 ) );
    }
 
    float C3  = (1.0f / 8.0f) ;
    C3       *= ( rough_sq / ( rough_sq + 0.09f ) );
    C3       *= pow( ( 4.0f * alpha * beta ) / (PI * PI), 2 );
 
    float A = gamma * C2 * tan( beta );
    float B = (1 - abs( gamma )) * C3 * tan( (alpha + beta) / 2.0f );
 
    oDiff = input.diff * max(0,NdotL) * ( C1 + A + B );
    oSpec = (float3)0;
}


//--------------------------------------------------------------------------------------
// Strauss lighting model
// Smoothness->SurfaceParams.x
// Metalness->SurfaceParams.y
// Transparency->SurfaceParams.y
// Refraction Index->SurfaceParams.w
//--------------------------------------------------------------------------------------
void Strauss(in ShadingParams input, inout float3 oDiff, inout float3 oSpec)
{
     // Params from the material texture
     float fSmoothness = input.params.x;
     float fMetalness = input.params.y;
     float fTransparency = input.params.z;
     float fRefractionIndex = input.params.w;
     
     float3 h = reflect( input.light, input.normal );
 
    // Declare any aliases:
    float NdotH   = dot( input.normal, h );
    float NdotL   = dot( input.normal, input.light );
    float NdotV   = dot( input.normal, input.view );
    float HdotV   = dot( h, input.view );
    float s_cubed = fSmoothness * fSmoothness * fSmoothness;
    
    // Next evaluate the Fresnel value
    float r_n = (fRefractionIndex-1) / (fRefractionIndex+1);
    r_n *= r_n;
    float fNdotL = pow( 1.0f - NdotL, 5.0f );
    fNdotL *= ( 1.0f - r_n );
    fNdotL += r_n;     
 
    // Evaluate the diffuse term
    float d  = ( 1.0f - fMetalness * fSmoothness );
    float Rd = ( 1.0f - s_cubed ) * ( 1.0f - fTransparency );
    float3 diffuse = NdotL * d * Rd * input.diff;
 
    // Compute the inputs into the specular term
    float r = ( 1.0f - fTransparency ) - Rd;
 
    // Compute the shadow term
    float geo_numerator   = 2.0f * NdotH;
    float geo_denominator = HdotV;
    float geo_b = (geo_numerator * NdotV ) / geo_denominator;
    float geo_c = (geo_numerator * NdotL ) / geo_denominator;
    
    float j = fNdotL * geo_b * geo_c;
 
    // 'k' is used to provide small off-specular
    // peak for very rough surfaces. Can be changed
    // to suit desired results...
    const float k = 0.1f;
    float reflect = min( 1.0f, r + j * ( r + k ) );
 
    float3 C1 = float3( 1.0f, 1.0f, 1.0f );
    float3 Cs = C1 + fMetalness * (1.0f - fNdotL) * (input.spec - C1);
 
    // Evaluate the specular term
    float3 specular = Cs * reflect;
    specular *= pow( -HdotV, 3.0f / (1.0f - fSmoothness) );
 
    // Composite the final result, ensuring
    // the values are >= 0.0f yields better results. Some
    // combinations of inputs generate negative values which
    // looks wrong when rendered...
    oDiff  = max( 0.0f, diffuse );
    oSpec = max( 0.0f, specular );
}


//--------------------------------------------------------------------------------------
// Ward lighting model
// Roughness.x->SurfaceParams.x
// Roughness.y->SurfaceParams.y
//--------------------------------------------------------------------------------------
void Ward(in ShadingParams input, inout float3 oDiff, inout float3 oSpec)
{
	// params
	float2 fAnisotropicRoughness = input.params.xy;
    
    float3 h = normalize( input.light + input.view );
 
    // Apply a small bias to the roughness
    // coefficients to avoid divide-by-zero
    fAnisotropicRoughness += float2( 1e-5f, 1e-5f );
 
    // Define the coordinate frame
    float3 epsilon   = float3( 1.0f, 0.0f, 0.0f );
    float3 tangent   = normalize( cross( input.normal, epsilon ) );
    float3 bitangent = normalize( cross( input.normal, tangent ) );
 
    // Define material properties
    float3 Ps   = float3( 1.0f, 1.0f, 1.0f );
 
    // Generate any useful aliases
    float VdotN = dot( input.view, input.normal );
    float LdotN = dot( input.light, input.normal );
    float HdotN = dot( h, input.normal );
    float HdotT = dot( h, tangent );
    float HdotB = dot( h, bitangent );
 
    // Evaluate the specular exponent
    float beta_a  = HdotT / fAnisotropicRoughness.x;
    beta_a       *= beta_a;
 
    float beta_b  = HdotB / fAnisotropicRoughness.y;
    beta_b       *= beta_b;
 
    float beta = -2.0f * ( ( beta_a + beta_b ) / ( 1.0f + HdotN ) );
 
    // Evaluate the specular denominator
    float s_den  = 4.0f * 3.14159f; 
    s_den       *= fAnisotropicRoughness.x;
    s_den       *= fAnisotropicRoughness.y;
    s_den       *= sqrt( LdotN * VdotN );
 
    // Compute the final specular term
    float3 Specular = Ps * ( exp( beta ) / s_den );
 
    // Composite the final values:
    oDiff = dot( input.normal, input.light ) * input.diff;
    oSpec = input.spec*Specular;
}

//--------------------------------------------------------------------------------------
// Ashikhmin-Shirley lighting model
// Nu->SurfaceParams.x
// Nv->SurfaceParams.y
//--------------------------------------------------------------------------------------
void Ashikhmin_Shirley(in ShadingParams input, inout float3 oDiff, inout float3 oSpec)
{
    float3 h = normalize( input.light + input.view );
 
    // Define the coordinate frame
    float3 epsilon = float3( 1.0f, 0.0f, 0.0f );
    float3 tangent = normalize( cross( input.normal, epsilon ) );
    float3 bitangent = normalize( cross( input.normal, tangent ) );
 
    // Generate any useful aliases
    float VdotN = dot( input.view, input.normal );
    float LdotN = dot( input.light, input.normal );
    float HdotN = dot( h, input.normal );
    float HdotL = dot( h, input.light );
    float HdotT = dot( h, tangent );
    float HdotB = dot( h, bitangent );
 
    float3 Rd = input.diff;
    float3 Rs = 0.3f;
 
    float Nu = input.params.x;
    float Nv = input.params.y;
 
    // Compute the diffuse term
    oDiff = (28.0f * Rd) / ( 23.0f * 3.14159f );
    oDiff *= (1.0f - Rs);
    oDiff *= (1.0f - pow(1.0f - (LdotN / 2.0f), 5.0f));
    oDiff *= (1.0f - pow(1.0f - (VdotN / 2.0f), 5.0f));
    oDiff *= input.diff;
    
 
    // Compute the specular term
    float ps_num_exp = Nu * HdotT * HdotT + Nv * HdotB * HdotB;
    ps_num_exp /= (1.0f - HdotN * HdotN);
 
    float Ps_num = sqrt( (Nu + 1) * (Nv + 1) );
    Ps_num *= pow( HdotN, ps_num_exp );
 
    float Ps_den = 8.0f * 3.14159f * HdotL;
    Ps_den *= max( LdotN, VdotN );
 
    oSpec = Rs * (Ps_num / Ps_den);
    oSpec *= ( Rs + (1.0f - Rs) * pow( 1.0f - HdotL, 5.0f ) );
    oSpec *= input.spec;
 }

//--------------------------------------------------------------------------------------
// Get the shadow factor
//--------------------------------------------------------------------------------------
void ComputeShadowFactor(in float3 p, in float3 l, inout float3 c, inout float att)
{
	// Compute the light distance for shadow mapping
	float4 PosLS = mul( float4(p, 1.0f), g_mLight );
	float Dist = length(PosLS.xyz);	
	
	if(g_LightType == LIGHT_SPOT)
	{
		// Get projective texture coords
		PosLS = mul( PosLS, g_mShadowProj );	
		float2 ProjTex = 0.5f * PosLS.xy / PosLS.w + float2( 0.5f, 0.5f );
		ProjTex.y = 1.0f - ProjTex.y;	
		
		// Shadow mapping
		if(g_bShadowMap)
			att *= ShadowVSM(ProjTex, Dist);
				
		// Light projective texturing
		if( g_bLightTex )
			c *= g_txLight.Sample( g_samAnisotropic, ProjTex );
	}
	else if(g_bShadowMap)
	{	
		att *= ShadowVSMCube(-l, Dist);
	}
}

//--------------------------------------------------------------------------------------
// Compute the full lighting equation
//--------------------------------------------------------------------------------------
void ComputeLighting(in ShadingParams input, inout float3 oDiff, inout float3 oSpec)
{
	// Compute the light vector and attenuation if a point light
	float attenuation = ComputeAttenuation(input.pos, input.light);
	if(attenuation<0.001f)
		discard;

	// Compute shadow mapping factor
	float4 lightColor = g_LightColor;
	ComputeShadowFactor(input.pos, input.light, lightColor.xyz, attenuation);
	clip(attenuation-0.001f);
	lightColor *= attenuation;

	// Compute the final color
	if(input.model == SHADE_PHONG)
		Blinn_Phong(input, oDiff, oSpec);	
	else if(input.model == SHADE_COOK_TORRANCE)
		Cook_Torrance(input, oDiff, oSpec);	
	else if(input.model == SHADE_OREN_NAYAR)
		Oren_Nayar_Complex(input, oDiff, oSpec);
	else if(input.model == SHADE_STRAUSS)
		Strauss(input, oDiff, oSpec);
	else if(input.model == SHADE_WARD)
		Ward(input, oDiff, oSpec);
	else if(input.model == SHADE_ASHIKHMIN_SHIRLEY)
		Ashikhmin_Shirley(input, oDiff, oSpec);
	
	oDiff *= lightColor;
	oSpec *= lightColor;
}
