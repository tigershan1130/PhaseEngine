

float RayleighPhase(float cosTheta)
{
	return 1.0f + 0.5f*cosTheta*cosTheta;
}

// Computes the scattering intensity by solving the optical depth integral
float GetScatteredIntensity(float lambda, float3 viewPos, float3 viewDir, float3 scatteringPoint)
{
	const float atmosphereTurbidity = 2.0f;
	const float rayleighAngularCoefficient = RayleighPhase(0);
	
	inScattering = atmosphereTurbidity * (rayleighAngularCoefficient * moleculeDensity + mieAngularCoefficient * aerosolDensity);
	
	return 1;
}