//--------------------------------------------------------------------------------------
// File: RendererInit.cpp
//
// The functions that fill the callbacks used in DXUT
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "Renderer.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Sets up a D3D10 device and window
	//--------------------------------------------------------------------------------------
	bool Renderer::Create(HWND hWnd, bool bWindowed)
	{
		HRESULT hr;

		m_hWnd = hWnd;

		// States
		Device::SetEffect(m_Effect);

		// Create the direct3d10 device
		InitDevice();
		return SUCCEEDED(OnCreateDevice()) && SUCCEEDED(OnResizedSwapChain());
	}


	//--------------------------------------------------------------------------------------
	// Closes the renderer
	//--------------------------------------------------------------------------------------
	void Renderer::Destroy()
	{
		OnReleasingSwapChain();
		OnDestroyDevice();
		MessageHandler::Flush();
	}



	//--------------------------------------------------------------------------------------
	// Setup any d3d objects here that are not dependent on screen size changes
	//--------------------------------------------------------------------------------------
	HRESULT Renderer::OnCreateDevice()
	{
		HRESULT hr;
		Log::Print("Creating Device...");

		// Set the background color to black
		Device::SetClearColor(0,0,0);
		
		// Submesh setup
		SubMesh::CamPointer = &m_Camera;

		// Init text drawing support
		V_RETURN( D3DX10CreateFont( g_pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
			OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
			L"Arial", &m_pFont ) );
		V_RETURN( D3DX10CreateSprite( g_pd3dDevice, 512, &m_pSprite ) );
		m_pTxtHelper = new CDXUTTextHelper( m_pFont, m_pSprite, 15 );

		// Load the effect
		 V_RETURN(Device::Effect->Create("Shaders\\Main.fx"));
		//V_RETURN(Device::Effect->Create("Core.dat"));

		// Create the occlusion query object
		D3D10_QUERY_DESC qd;
		qd.Query = D3D10_QUERY_OCCLUSION_PREDICATE;
		qd.MiscFlags = 0;
		hr = g_pd3dDevice->CreatePredicate(&qd, &m_pPredicate);
		if(FAILED(hr))
		{
			Log::D3D10Error(hr);
			Log::FatalError("Failed to create predication query");
			return E_FAIL;
		}

		// Reset the device states
		Device::Reset();
		ShadowSettings::ShadowSettings();
		ProbeSettings::ProbeSettings();

		// Load a sphere mesh
		BaseMesh::bLoadMaterials=false;
		m_SphereMesh.Load(MESH_SPHERE);
		m_BoxMesh.Load("Models\\Old\\box.x");
		m_ConeMesh.Load(MESH_CONE);
		BaseMesh::bLoadMaterials=true;

		// Set the default ambient light level
		SetAmbientLighting(0.3f, 0.3f, 0.3f);

		// Setup the shadow system
		if(FAILED(InitShadowSystem(512, 4, 4, 1)))
		{
			Log::FatalError("Shadow system failed to load.");
			return E_FAIL;
		}

		// Setup the shadow system
		if(FAILED(m_SkySystem.Init(Device::Effect)))
		{
			Log::FatalError("Sky system failed to load.");
			return E_FAIL;
		}


		// Setup the environment probe system
		m_ProbeDepth.Create(ProbeSettings::resolution, ProbeSettings::resolution, NULL, false);
		D3DXMatrixPerspectiveFovLH( &m_mProbeProj, D3DX_PI*0.5f, 1.0f, 0.01f, 1000.0f);

		// Setup the debug renderer
#ifdef PHASE_DEBUG
		V_RETURN(InitDebugRenderer());

		// Setup the focus mesh lights
		for(int i=0; i<4; i++)
		{
			m_FocusLights[i].Type = Light::LIGHT_DIRECTIONAL;
			m_FocusLights[i].IsShadowed = false;
			m_FocusLights[i].Color = D3DXVECTOR4(0.4,0.4,0.4,0.4);
		}
		m_FocusLights[0].SetDir(0.5,0.5,1);
		m_FocusLights[1].SetDir(0.5,-0.5,1);
		m_FocusLights[2].SetDir(-0.5,0.5,1);
		m_FocusLights[3].SetDir(-0.5,-0.5,1);
#endif

		// Setup the sun lighting
		m_Sun.Type = Light::LIGHT_DIRECTIONAL;
		m_Sun.Color = D3DXVECTOR4(1, 1, 1 ,1);
		m_Lights.Add(&m_Sun);

		Log::Print("Device created!");
		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	// Resizes the device swap chains when the window size changes
	//--------------------------------------------------------------------------------------
	HRESULT Renderer::Resize()
	{
		LOG("Resizing viewport");

		// First release the swap chain
		OnReleasingSwapChain();

		// Now resize it
		if(FAILED(ResizeSwapChain()))
		{
			LOG("ResizeSwapChain() Failed");
			Destroy();
			PostQuitMessage(-1);
		}

		// Now reallocate resources
		if(FAILED(OnResizedSwapChain()))
		{
			LOG("OnResizedSwapChain() Failed");
			Destroy();
			PostQuitMessage(-1);
		}

		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	// Setup any d3d objects here that depend on the screen size
	//--------------------------------------------------------------------------------------
	HRESULT Renderer::OnResizedSwapChain()
	{
		Log::Print("Device resizing...");
		
		// Create an orthoganal and scaled projection matrix for
		// screen sized quad rendering
		D3DXMATRIXA16 mScaleScreen,mOrtho2D;
		D3DXMatrixScaling(&mScaleScreen, (float)m_Width/2.0f, (float)m_Height/2.0f, 0);	
		D3DXMatrixOrthoLH(&mOrtho2D, (float)m_Width, (float)m_Height, 0.0f, 1.0f);
		m_mOrtho = mScaleScreen * mOrtho2D;
		Device::Effect->OrthoProjectionVariable->SetMatrix( (float*)&m_mOrtho );

		// Build the camera projection matrix
		m_Camera.BuildProjectionMatrix(m_Width, m_Height);

		// Create the GBuffers
		HRESULT hr;
		DXGI_FORMAT formats[] = {
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_R32_FLOAT,
		};
		V_RETURN( m_GBuffer.CreateArray(m_Width, m_Height, formats, DS_SIZE, 1, NULL) );
		m_GBuffer.AttachDepthStencil(m_DefaultDepth);
		m_GBuffer.AttachEffectSRVVariable(Device::Effect->GBufferVariable);
		Log::Print("GBuffers created");

		// Create the lighting buffer
		DXGI_FORMAT formats2[] = {
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			DXGI_FORMAT_R16G16B16A16_FLOAT
		};
		V_RETURN( m_LightingBuffer.CreateArray(m_Width, m_Height, formats2, 2, 1, NULL) );
		m_LightingBuffer.AttachDepthStencil(m_DefaultDepth);
		m_LightingBuffer.AttachEffectSRVVariable(Device::Effect->LightBufferVariable);

		// Create a texture resource for the framebuffer
		m_FrameBuffer.Create(m_Width, m_Height, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, NULL, false);
		m_FrameBuffer.AttachEffectSRVVariable(Device::Effect->FrameBufferVariable);

		m_PostFXBuffer.Create(m_Width, m_Height, DXGI_FORMAT_R8G8B8A8_UNORM, 1, NULL);
		m_PostFXBuffer.AttachDepthStencil(m_DefaultDepth);
		m_PostFXBuffer.AttachEffectSRVVariable(Device::Effect->FrameBufferVariable);

		Log::Print("PostFX Framework Loaded");

		// Set the width/height and clip plane
		D3DXVECTOR4 screenSize(m_Width, m_Height, 1.0f/(float)m_Width, 1.0f/(float)m_Height);
		Device::Effect->ScreenSizeVariable->SetFloatVector( (float*)&screenSize );
		Device::Effect->FarZVariable->SetFloat(m_Camera.GetFarZ());
		Device::Effect->NearZVariable->SetFloat(m_Camera.GetNearZ());

		// Create a quad for rendering the light passes
		// The normal contains the frustum corner for that vertex
		// which is essentially the view direction to use for
		// reconstructing the position
		Vertex pNewVerts[6] = 
		{
			{ D3DXVECTOR3( 1.0f, 1.0f, 0.5f), 1.0f, 0.0f, m_Camera.GetFrustumCorner(Camera::FRUSTUM_TOP_RIGHT) },
			{ D3DXVECTOR3( 1.0f,-1.0f, 0.5f), 1.0f, 1.0f, m_Camera.GetFrustumCorner(Camera::FRUSTUM_BOTTOM_RIGHT) },
			{ D3DXVECTOR3(-1.0f,-1.0f, 0.5f), 0.0f, 1.0f, m_Camera.GetFrustumCorner(Camera::FRUSTUM_BOTTOM_LEFT) },
			{ D3DXVECTOR3(-1.0f,-1.0f, 0.5f), 0.0f, 1.0f, m_Camera.GetFrustumCorner(Camera::FRUSTUM_BOTTOM_LEFT) },
			{ D3DXVECTOR3(-1.0f, 1.0f, 0.5f), 0.0f, 0.0f, m_Camera.GetFrustumCorner(Camera::FRUSTUM_TOP_LEFT) },
			{ D3DXVECTOR3( 1.0f, 1.0f, 0.5f), 1.0f, 0.0f, m_Camera.GetFrustumCorner(Camera::FRUSTUM_TOP_RIGHT) },
		};
		CopyMemory(m_QuadVerts, pNewVerts, 6*sizeof(Vertex));

		// Fill the vertex buffer
		SAFE_RELEASE(m_pQuadVertexBuffer);
		m_pQuadVertexBuffer = NULL;
		D3D10_BUFFER_DESC bd;
		bd.Usage = D3D10_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof( Vertex ) * 6;
		bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		D3D10_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = m_QuadVerts;
		g_pd3dDevice->CreateBuffer( &bd, &InitData, &m_pQuadVertexBuffer );

		// Build the SSAO sampling texture
		EncodeSSAOTexture(m_Width, m_Height);

		Log::Print("SSAO Texture Created");


		// Velocity buffer
		m_VelocityMap[0].Create(m_Width, m_Height, DXGI_FORMAT_R16G16_FLOAT, 1, NULL);
		m_VelocityMap[0].AttachDepthStencil(m_DefaultDepth);
		m_VelocityMap[1].Create(m_Width, m_Height, DXGI_FORMAT_R16G16_FLOAT, 1, NULL);
		m_VelocityMap[1].AttachDepthStencil(m_DefaultDepth);

		// SSAO buffer
		m_SSAOBuffer[0].Create(m_Width/2, m_Height/2, DXGI_FORMAT_R8_UNORM, 1, NULL);
		m_SSAOBuffer[0].AttachDepthStencil(m_DefaultDepth);
		m_SSAOBuffer[0].AttachEffectSRVVariable(Device::Effect->PostFXVariable);
		m_SSAOBuffer[1].Create(m_Width/2, m_Height/2, DXGI_FORMAT_R8_UNORM, 1, NULL);
		m_SSAOBuffer[1].AttachDepthStencil(m_DefaultDepth);
		m_SSAOBuffer[1].AttachEffectSRVVariable(Device::Effect->PostFXVariable);

		// Create the HDR system
		Log::Print("Creating HDR System");
		return m_HDRSystem.Create(m_Width, m_Height, m_DefaultDepth.GetDSV(), Device::Effect);
	}



	//--------------------------------------------------------------------------------------
	// Release objects created in OnResizeSwapChain
	void Renderer::OnReleasingSwapChain()
	{
		// Random SSAO texture
		SAFE_RELEASE(m_pRandomTex);
		SAFE_RELEASE(m_pRandomSRV);

		// Release the GBuffers
		m_GBuffer.Release();
		m_LightingBuffer.Release();

		// Other render targets
		m_FrameBuffer.Release();
		m_VelocityMap[0].Release();
		m_VelocityMap[1].Release();
		m_PostFXBuffer.Release();
		m_SSAOBuffer[0].Release();
		m_SSAOBuffer[1].Release();

		// HDR
		m_HDRSystem.Release();

		// The fullscreen quad buffer
		SAFE_RELEASE(m_pQuadVertexBuffer);
		Log::Print("Swap Chain released");
	}

	//--------------------------------------------------------------------------------------
	// Release objects created in OnCreateDevice
	//--------------------------------------------------------------------------------------
	void Renderer::OnDestroyDevice()
	{
		Log::Print("Destroying Device");

		// Unset all resources from the device
		D3DX10UnsetAllDeviceObjects(m_pd3dDevice);

		// Destroy the debug renderer
#ifdef PHASE_DEBUG
		ReleaseDebugRenderer();
#endif
		
		// Destroy the generated textures
		SAFE_RELEASE(m_pMaterialTex);
		SAFE_RELEASE(m_pMaterialSRV);

		// Environment probes
		for(int i=0; i<m_Probes.Size(); i++)
			m_Probes[i]->Release();
		m_Probes.Release();
		m_ProbeDepth.Release();
		
		// Release the scene
		DestroyScene();

		// Shadow system
		ReleaseShadowSystem();

		// Sky system
		m_SkySystem.Release();

		// Release font objects
		SAFE_RELEASE( m_pFont );
		SAFE_RELEASE( m_pSprite );
		SAFE_DELETE( m_pTxtHelper );

		// Release the effect
		Device::Effect->Release();

		// The meshes
		m_ConeMesh.Release();
		m_BoxMesh.Release();
		m_SphereMesh.Release();
		m_VisibleLights.Release();

		// Release the resource managers
		g_Meshes.Release();
		g_Textures.Release();
		g_Materials.Release();
		m_Lights.Release();

		CleanupDevice();
	}

	//--------------------------------------------------------------------------------------
	// Clears the current scene
	//--------------------------------------------------------------------------------------
	void Renderer::DestroyScene()
	{
		Log::Print("Destroying Scene");
		
		// Lights
		m_Lights.Release();
		//m_Lights.Add(&m_Sun);

		// Light pool
		while(!m_LightPool.IsEmpty())
			delete m_LightPool.Pop();
		m_LightPool.Release();

		// Mesh Objects
		m_MeshObjects.Release();

		// Particle emitters
		for(int i=0; i<m_Emitters.Size(); i++)
		{
			m_Emitters[i]->Release();
			delete m_Emitters[i];
		}
		m_Emitters.Release();

		//Water
		
		if(m_pWater)
		{
			m_pWater->Release();
			delete m_pWater;
			m_pWater = NULL;

		}
		// Terrain
		if(m_pTerrain)
		{
			m_pTerrain->Release();
			delete m_pTerrain;
			m_pTerrain = NULL;
		}

		// Materials
		m_RenderList.Release();
		
		m_TransparentObjects.Release();
	}


}