//--------------------------------------------------------------------------------------
// File: Sky.cpp
//
// Implementation of "Precomputed Atmospheric Scattering"
// 
// Simulates multiple scattering effects from any view point
//
// Coded by Nate Orr 2008
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "Sky.h"
#include "Device.h"


namespace Core
{
	//--------------------------------------------------------------------------------------
	// Creates the sky system
	//--------------------------------------------------------------------------------------
	HRESULT Sky::Init(Effect* pEffect)
	{
		Log::Print("Building sky system");

		// Setup the tod system
		void InitTOD();
		
		m_DomeSize = 128;
		m_OpticalDepthSize = 256;
		m_ScatteringTextureSize = 128;

		// Create the scattering textures
		DXGI_FORMAT formats[] = {
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_R16G16B16A16_FLOAT,
		};
		if(FAILED(m_PrecomputedScattering.CreateArray(m_ScatteringTextureSize, m_ScatteringTextureSize/2, formats, 2, 1, NULL, true)))
		{
			Log::Print("Scattering textures failed to create");
			return E_FAIL;
		}
		m_Depth.Create(m_ScatteringTextureSize, m_ScatteringTextureSize/2, NULL, false);
		m_PrecomputedScattering.AttachDepthStencil(m_Depth);

		// Setup the default time of day values
		m_Month = 6;
		m_Day = 16;
		m_Year = 2004;
		m_Hour = 12;
		m_Minute = 10;
		m_Second = 15;

		// Setup the plane verts
		const float planeSize = 5000;
		m_PlaneVerts[0].pos = D3DXVECTOR3(-planeSize, 2000, -planeSize);
		m_PlaneVerts[1].pos = D3DXVECTOR3( planeSize, 2000, -planeSize);
		m_PlaneVerts[2].pos = D3DXVECTOR3( planeSize, 2000,  planeSize);
		m_PlaneVerts[3].pos = D3DXVECTOR3( planeSize, 2000,  planeSize);
		m_PlaneVerts[4].pos = D3DXVECTOR3(-planeSize, 2000,  planeSize);
		m_PlaneVerts[5].pos = D3DXVECTOR3(-planeSize, 2000, -planeSize);
		m_PlaneVerts[0].tu = 0;
		m_PlaneVerts[0].tv = 0;
		m_PlaneVerts[1].tu = 1;
		m_PlaneVerts[1].tv = 0;
		m_PlaneVerts[2].tu = 1;
		m_PlaneVerts[2].tv = 1;
		m_PlaneVerts[3].tu = 1;
		m_PlaneVerts[3].tv = 1;
		m_PlaneVerts[4].tu = 0;
		m_PlaneVerts[4].tu = 1;
		m_PlaneVerts[5].tv = 0;
		m_PlaneVerts[5].tv = 0;
		D3D10_BUFFER_DESC bd;
		D3D10_SUBRESOURCE_DATA InitData;
		bd.Usage = D3D10_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof( Vertex ) * 6;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		InitData.pSysMem = (Vertex*)m_PlaneVerts;
		if(FAILED(g_pd3dDevice->CreateBuffer( &bd, &InitData, &m_PlaneBuffer )))
			Log::Print("Failed to create sky plane buffer");

		// Load a test sky texture
		m_CloudTexture.Load("Textures\\clouds.bmp");

		// Create the cloud render target
		m_Clouds.Create(512, 512, DXGI_FORMAT_R8G8B8A8_UNORM, 1, NULL);
		m_CloudDepth.Create(512, 512, NULL, false);
		m_Clouds.AttachDepthStencil(m_CloudDepth);

		// Generate the perlin texture
		GeneratePerlinSampler();

		// Setup the effect variables
		m_ConstantBuffer = NULL;
		BindEffectVariables(pEffect);

		// Construct the dome mesh
		return BuildDome();
	}


	//--------------------------------------------------------------------------------------
	// Binds the effect variables when the effect is reloaded
	//--------------------------------------------------------------------------------------
	void Sky::BindEffectVariables(Effect* pEffect)
	{
		m_pEffect = pEffect;
		m_PrecomputedScattering.AttachEffectSRVVariable(m_pEffect->ScatteringTexturesVariable);
		m_Clouds.AttachEffectSRVVariable(m_pEffect->CloudTextureVariable);
		m_pEffect->PerlinVariable->SetResource( m_PerlinSamplerSRV );
		SetConstants();
	}


	
	//--------------------------------------------------------------------------------------
	// Setup the constants
	//--------------------------------------------------------------------------------------
	void Sky::SetConstants()
	{
		m_Constants.PI = D3DX_PI;
		double ESun = 20.0;
		double Kr = 0.0025;
		double Km = 0.0010;
		m_Constants.KrESun = Kr * ESun;
		m_Constants.KmESun = Km * ESun;
		m_Constants.Kr4PI = Kr * 4.0 * m_Constants.PI;
		m_Constants.Km4PI = Km * 4.0 * m_Constants.PI;
		m_Constants.InnerRadius = 6356.7523142;
		m_Constants.OuterRadius = m_Constants.InnerRadius * 1.0157313; // karman line
		m_Constants.fScale = 1.0 / (m_Constants.OuterRadius - m_Constants.InnerRadius);
		m_Constants.v2dRayleighMieScaleHeight.x = 0.25; 
		m_Constants.v2dRayleighMieScaleHeight.y = 0.1;

		m_Constants.InvWavelength4.x = 1.0 / pow( 0.650, 4 );
		m_Constants.InvWavelength4.y = 1.0 / pow( 0.570, 4 );
		m_Constants.InvWavelength4.z = 1.0 / pow( 0.475, 4 );
		m_Constants.WavelengthMie.x = pow( 0.650, -0.84 );
		m_Constants.WavelengthMie.y = pow( 0.570, -0.84 );
		m_Constants.WavelengthMie.z = pow( 0.475, -0.84 );

		double g = -0.995;
		double g2 = g * g;
		m_Constants.v3HG.x = 1.5f * ( (1.0f - g2) / (2.0f + g2) );
		m_Constants.v3HG.y = 1.0f + g2;
		m_Constants.v3HG.z = 2.0f * g;

		m_Constants.tNumSamples = 50;
		m_Constants.iNumSamples = 20;

		m_Constants.InvOpticalDepthN = 1.0 / float( m_OpticalDepthSize );
		m_Constants.InvOpticalDepthNLessOne = 1.0 / float( m_OpticalDepthSize - 1 );
		m_Constants.HalfTexelOpticalDepthN = 0.5 / float( m_OpticalDepthSize );
		m_Constants.InvRayleighMieN = D3DXVECTOR2( 1.0f / float( m_ScatteringTextureSize ), 1.0f / float( m_ScatteringTextureSize / 2 ) );
		m_Constants.InvRayleighMieNLessOne = D3DXVECTOR2( 1.0f / float( m_ScatteringTextureSize - 1 ), 1.0f / float( m_ScatteringTextureSize / 2 - 1) );

		D3D10_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = sizeof( ScbConstant );
		cbDesc.Usage = D3D10_USAGE_IMMUTABLE;
		cbDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = 0;
		cbDesc.MiscFlags = 0;

		D3D10_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = &m_Constants;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		SAFE_RELEASE(m_ConstantBuffer);
		g_pd3dDevice->CreateBuffer( &cbDesc, &InitData, &m_ConstantBuffer );
		m_pEffect->SkyConstantBuffer->SetConstantBuffer(m_ConstantBuffer);
	}


	//--------------------------------------------------------------------------------------
	// Setup the tod system
	//--------------------------------------------------------------------------------------
	void Sky::InitTOD()
	{
		m_TOD.year          = m_Year;
		m_TOD.month         = m_Month;
		m_TOD.day           = m_Day;
		m_TOD.hour          = m_Hour;
		m_TOD.minute        = m_Minute;
		m_TOD.second        = m_Second;
		m_TOD.timezone      = -7.0;
		m_TOD.delta_t       = 67;
		m_TOD.longitude     = -105.1786;
		m_TOD.latitude      = 0;
		m_TOD.elevation     = 1830.14;
		m_TOD.pressure      = 820;
		m_TOD.temperature   = 11;
		m_TOD.slope         = 30;
		m_TOD.azm_rotation  = -10;
		m_TOD.atmos_refract = 0.5667;
		m_TOD.function      = SPA_ALL;
	}


	//--------------------------------------------------------------------------------------
	// Free resources
	//--------------------------------------------------------------------------------------
	void Sky::Release()
	{
		SAFE_RELEASE(m_pDomeMesh);
		m_PrecomputedScattering.Release();
		m_Depth.Release();
		SAFE_RELEASE(m_ConstantBuffer);
		m_CloudTexture.Release();
		SAFE_RELEASE(m_PlaneBuffer);
		m_Clouds.Release();
		m_CloudDepth.Release();
		SAFE_RELEASE(m_PerlinSamplerSRV);
		SAFE_RELEASE(m_PerlinSampler);
	}


	//--------------------------------------------------------------------------------------
	// Generates the perlin noise sampler
	// (modified from http://www.sci.utah.edu/~leenak/IndStudy_reportfall/CloudsCode.txt)
	//--------------------------------------------------------------------------------------
	void Sky::GeneratePerlinSampler()
	{
		int perm[256]= {151,160,137,91,90,15,
			131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
			190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
			88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
			77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
			102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
			135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
			5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
			223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
			129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
			251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
			49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
			138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180};

		int grad3[16][3] = {{0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1},
		{1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
		{1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0}, // 12 cube edges
		{1,0,-1},{-1,0,-1},{0,-1,1},{0,1,1}}; // 4 more to make 16

		// Create the texture data

		UINT width = 256;
		UINT height = 256;

		// Describe the texture
		D3D10_TEXTURE2D_DESC texDesc;
		texDesc.ArraySize           = 1;
		texDesc.BindFlags           = D3D10_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags      = 0;
		texDesc.Format              = DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.Height              = height;
		texDesc.Width               = width;
		texDesc.MipLevels           = 1;
		texDesc.MiscFlags           = 0;
		texDesc.SampleDesc.Count    = 1;
		texDesc.SampleDesc.Quality  = 0;
		texDesc.Usage               = D3D10_USAGE_IMMUTABLE;

		// Generate the initial data
		BYTE* pData = new BYTE[ width*height*4 ];
		UINT index=0;
		for(UINT j=0; j<height; j++)
			for(UINT i=0; i<width; i++, index+=4 )
			{
				// Random value from 0-1
				char value = perm[(j+perm[i]) & 0xFF];
				pData[index] = grad3[value & 0x0F][0] * 64 + 64;    // Gradient x
				pData[index+1] = grad3[value & 0x0F][1] * 64 + 64;  // Gradient y
				pData[index+2] = grad3[value & 0x0F][2] * 64 + 64;  // Gradient z
				pData[index+3] = value;								// Permuted index
			}

			D3D10_SUBRESOURCE_DATA initialData;
			initialData.pSysMem          = pData;
			initialData.SysMemPitch      = width*4;
			initialData.SysMemSlicePitch = 0;

			// Create the actual texture
			HRESULT hr = g_pd3dDevice->CreateTexture2D( &texDesc, &initialData, &m_PerlinSampler );
			if( FAILED( hr ) )
			{
				Log::Print( "Failed to create perlin sampler" );

				delete[] pData;

				return;
			}

			// Create a view onto the texture
			hr = g_pd3dDevice->CreateShaderResourceView( m_PerlinSampler, NULL, &m_PerlinSamplerSRV );
			if( FAILED( hr ) )
			{
				Log::Print( "Failed to create perlin srv" );

				delete[] pData;

				return;
			}

			// Clear up any intermediary resources
			delete[] pData;
	}

	
	//--------------------------------------------------------------------------------------
	// Computes the sun direction from the spa data
	//--------------------------------------------------------------------------------------
	void Sky::ComputeSunDirection()
	{
		/*const double d2r = 0.0174532925;
		
		// Get the sun data
		InitTOD();
		Log::Print("spa result %d", spa_calculate(&m_TOD));
		Log::Print("zenith %f", m_TOD.zenith);
		Log::Print("azimuth %f", m_TOD.azimuth);
		Log::Print("incidience %f", m_TOD.incidence);
		Log::Print("hours %d %d", m_TOD.hour, m_Hour);


		// Convert the angles into a direction vector
		D3DXMATRIX mRot, mRot2;
		D3DXMatrixRotationX(&mRot, -((90.0f-m_TOD.zenith)*d2r));
		D3DXMatrixRotationY(&mRot2, m_TOD.azimuth*d2r);
		D3DXMatrixMultiply(&mRot, &mRot2, &mRot);
		D3DXVec3TransformNormal(&m_SunDirection, &D3DXVECTOR3(0,0,1), &mRot);*/

		// Convert the hours to the angle in the sphere
		float deg = ((float)m_Hour / 24.0f) * (2*D3DX_PI);
		D3DXMATRIX mRot;
		D3DXMatrixRotationZ(&mRot, deg);
		D3DXVec3TransformNormal(&m_SunDirection, &D3DXVECTOR3(1,0,0), &mRot);


		// Set it in the shader
		m_pEffect->SunDirectionVariable->SetFloatVector((float*)&m_SunDirection);
	}

	
	//--------------------------------------------------------------------------------------
	// Build the scattering textures
	//--------------------------------------------------------------------------------------
	void Sky::ComputeScattering(D3DXVECTOR3& pos)
	{
		// Set the sun direction
		ComputeSunDirection();
		
		// Compute the scattering integrals into a texture
		m_PrecomputedScattering.BindRenderTarget();
		m_pEffect->OrthoProjectionVariable->SetMatrix( (float*)&m_PrecomputedScattering.GetOrthoMatrix() );
		g_pd3dDevice->RSSetViewports(1, &m_PrecomputedScattering.GetViewport());
		m_pEffect->Pass[PASS_COMPUTE_SCATTERING]->Apply(0);
		g_pd3dDevice->Draw(6, 0);
		Device::FrameStats.DrawCalls++;
		Device::FrameStats.PolysDrawn += 2;

		// Center the dome at the camera position
		/*D3DXMATRIX mWorld;
		D3DXMatrixTranslation(&mWorld, pos.x, pos.y, pos.z);
		m_pEffect->WorldMatrixVariable->SetMatrix( (float*)&mWorld);
		
		// Project the cloud layer onto the skydome
		m_Clouds.Clear();
		m_Clouds.BindRenderTarget();
		m_pEffect->OrthoProjectionVariable->SetMatrix( (float*)&m_Clouds.GetOrthoMatrix() );
		g_pd3dDevice->RSSetViewports(1, &m_Clouds.GetViewport());
		m_pEffect->CloudTextureVariable->SetResource(m_CloudTexture.GetResource());
		m_pEffect->Pass[PASS_CLOUD_PLANE]->Apply(0);	
		g_pd3dDevice->Draw(6, 0);*/
		//m_pDomeMesh->DrawSubset(0);
	}

	//--------------------------------------------------------------------------------------
	// Draw the sky
	//--------------------------------------------------------------------------------------
	void Sky::Render(D3DXVECTOR3& pos)
	{
		// Center the dome at the camera position
		D3DXMATRIX mWorld;
		D3DXMatrixTranslation(&mWorld, pos.x, pos.y, pos.z);
		m_pEffect->WorldMatrixVariable->SetMatrix( (float*)&mWorld);
		
		// Draw the sky dome
		m_PrecomputedScattering.BindTextures();
		//m_Clouds.BindTextures();
		//m_pEffect->CloudTextureVariable->SetResource(m_CloudTexture.GetResource());
		m_pEffect->Pass[PASS_SKY]->Apply(0);
		m_pDomeMesh->DrawSubset(0);
		Device::FrameStats.DrawCalls++;
		Device::FrameStats.PolysDrawn += m_pDomeMesh->GetFaceCount();
		Device::FrameStats.IBChanges++;
		Device::FrameStats.VBChanges++;
		Device::SetVertexBuffer(NULL, 0);
	
		
		// Project the cloud layer onto the skydome
		//UINT offset=0;
		//g_pd3dDevice->IASetVertexBuffers( 0, 1, &m_PlaneBuffer, &Vertex::size, &offset );
		//m_pEffect->CloudTextureVariable->SetResource(m_CloudTexture.GetResource());
		//m_pEffect->Pass[PASS_CLOUD_PLANE]->Apply(0);
		//g_pd3dDevice->Draw(6, 0);
	}


	//--------------------------------------------------------------------------------------
	// Builds the dome mesh
	//--------------------------------------------------------------------------------------
	HRESULT Sky::BuildDome()
	{
		HRESULT hr;
		UINT Latitude = m_DomeSize/2;
		UINT Longitude = m_DomeSize;
		UINT DVSize = Longitude * Latitude;
		UINT DISize = (Longitude - 1) * (Latitude - 1) * 2;
		DVSize *= 2;
		DISize *= 2;

		// Create Mesh
		V_RETURN( D3DX10CreateMesh(
			g_pd3dDevice,
			Vertex::Desc,
			sizeof( Vertex::Desc ) / sizeof( Vertex::Desc[0] ),
			Vertex::Desc[0].SemanticName,
			DVSize,
			DISize,
			0,
			&m_pDomeMesh) );

		// CreateVertexbuffer
		ID3DX10MeshBuffer * Dx10MeshBuffer;
		V_RETURN( m_pDomeMesh->GetVertexBuffer( 0, &Dx10MeshBuffer ) );

		SIZE_T MeshBufferSize;
		Vertex * pVertices;
		V_RETURN( Dx10MeshBuffer->Map( (VOID**)&pVertices, &MeshBufferSize ) );

		UINT DomeIndex = 0;
		for( int i = 0; i < Longitude; i++ )
		{
			//const double dDistribution = (1.0 - exp(-0.5 * (i*10.0) / double(m_DomeSize - 1)));
			//const double MoveXZ = 100.0 * dDistribution * D3DX_PI / 180.0;
			const double MoveXZ = 100.0 * ( i / float(Longitude - 1) ) * D3DX_PI / 180.0;

			for( int j = 0; j < Latitude; j++ )
			{	
				//const double MoveY = (D3DX_PI * 2.0) * j / (m_DomeSize - 1) ;
				const double MoveY = D3DX_PI * j / (Latitude - 1) ;

				pVertices[DomeIndex].pos.x = sin( MoveXZ ) * cos( MoveY  );
				pVertices[DomeIndex].pos.y = cos( MoveXZ );
				pVertices[DomeIndex].pos.z = sin( MoveXZ ) * sin( MoveY  );

				pVertices[DomeIndex].pos *= 10.0f;

				pVertices[DomeIndex].tu = 0.5 / float( Longitude ) + i / float( Longitude );	// [0.5, Texsize-0.5] 
				pVertices[DomeIndex].tv = 0.5 / float( Latitude ) + j / float( Latitude );	// [0.5, Texsize-0.5]

				DomeIndex++;
			}
		}

		for( int i = 0; i < Longitude; i++ )
		{
			//const double dDistribution = (1.0 - exp(-0.5 * (i*10.0) / double(m_DomeSize - 1)));
			//const double MoveXZ = 100.0 * dDistribution * D3DX_PI / 180.0;
			const double MoveXZ = 100.0 * ( i / float(Longitude - 1) ) * D3DX_PI / 180.0;

			for( int j = 0; j < Latitude; j++ )
			{	
				//const double MoveY = (D3DX_PI * 2.0) * j / (m_DomeSize - 1) ;
				const double MoveY = (D3DX_PI * 2.0) - (D3DX_PI * j / (Latitude - 1)) ;

				pVertices[DomeIndex].pos.x = sin( MoveXZ ) * cos( MoveY  );
				pVertices[DomeIndex].pos.y = cos( MoveXZ );
				pVertices[DomeIndex].pos.z = sin( MoveXZ ) * sin( MoveY  );

				pVertices[DomeIndex].pos *= 10.0f;

				pVertices[DomeIndex].tu = 0.5 / float( Longitude ) + i / float( Longitude );	// [0.5, Texsize-0.5] 
				pVertices[DomeIndex].tv = 0.5 / float( Latitude ) + j / float( Latitude );	// [0.5, Texsize-0.5]

				DomeIndex++;
			}
		}

		V_RETURN( Dx10MeshBuffer->Unmap() );
		SAFE_RELEASE( Dx10MeshBuffer );

		// CreateIndexBuffer
		V_RETURN( m_pDomeMesh->GetIndexBuffer( &Dx10MeshBuffer ) );

		unsigned short * pIndices = NULL;
		V_RETURN( Dx10MeshBuffer->Map( (VOID**)&pIndices, &MeshBufferSize ) );

		for( unsigned short i = 0; i < Longitude - 1; i++)
		{
			for( unsigned short j = 0; j < Latitude - 1; j++)
			{
				*(pIndices++) = i * Latitude + j;
				*(pIndices++) = (i + 1) * Latitude + j;
				*(pIndices++) = (i + 1) * Latitude + j + 1;

				*(pIndices++) = (i + 1) * Latitude + j + 1;
				*(pIndices++) = i * Latitude + j + 1;
				*(pIndices++) = i * Latitude + j;
			}
		}

		const UINT Offset = Latitude * Longitude;
		for( unsigned short i = 0; i < Longitude - 1; i++)
		{
			for( unsigned short j = 0; j < Latitude - 1; j++)
			{
				*(pIndices++) = Offset + i * Latitude + j;
				*(pIndices++) = Offset + (i + 1) * Latitude + j + 1;
				*(pIndices++) = Offset + (i + 1) * Latitude + j;

				*(pIndices++) = Offset + i * Latitude + j + 1;
				*(pIndices++) = Offset + (i + 1) * Latitude + j + 1;
				*(pIndices++) = Offset + i * Latitude + j;
			}
		}

		V_RETURN( Dx10MeshBuffer->Unmap() );
		SAFE_RELEASE( Dx10MeshBuffer );

		// Commit to GPU
		V_RETURN( m_pDomeMesh->CommitToDevice() );
	}
}