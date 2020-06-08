//--------------------------------------------------------------------------------------
// File: main.cpp
//
// wWinMain
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Renderer.h"
#include "DirectInput.h"

using namespace Core;


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch( message )
	{
	case WM_PAINT:
		hdc = BeginPaint( hWnd, &ps );
		EndPaint( hWnd, &ps );
		break;

	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;

	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
	}

	return 0;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HWND InitWindow( HINSTANCE hInstance, int nCmdShow, UINT width, UINT height )
{
	LOG( "Initializing window" );

	HWND hWnd = NULL;

	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
	wcex.hIcon = NULL;
	wcex.hIconSm = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"D3D10 App";
	if( !RegisterClassEx( &wcex ) )
		return NULL;

	// Create window
	RECT rc = { 0, 0, width, height };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	hWnd = CreateWindow( L"D3D10 App", L"Phase Engine Particle Demo", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance,
		NULL );
	if( !hWnd )
		return NULL;

	ShowWindow( hWnd, nCmdShow );

	LOG( "... done!" );
	return hWnd;
}


//--------------------------------------------------------------------------------------
// Camera updating
//--------------------------------------------------------------------------------------
void UpdateCamera(Camera& camera)
{
	// Update the camera speed
	float speed = 0.04f * g_TimeScale;
	D3DXVECTOR3 deltaSpeed(0,0,0);
	if (Input::KeyDown(DIK_LSHIFT))
		speed *= 8.0f;
	if (Input::KeyDown(DIK_W))
		deltaSpeed.z += speed;
	if (Input::KeyDown(DIK_S))
		deltaSpeed.z -= speed;
	if (Input::KeyDown(DIK_A))
		deltaSpeed.x -= speed;
	if (Input::KeyDown(DIK_D))
		deltaSpeed.x += speed;
	if (Input::KeyDown(DIK_Q))
		deltaSpeed.y -= speed;
	if (Input::KeyDown(DIK_E))
		deltaSpeed.y += speed;
	camera.AddToVelocity(deltaSpeed.x, deltaSpeed.y, deltaSpeed.z);

	// Update the view angles (FPS camera)
	if (Input::MouseRightClick())
		camera.AddToAngularVelocity(Input::MouseDeltaY() / 500.0f, Input::MouseDeltaX() / 500.0f);
}



// Particle callback function
void ParticleProc1(Particle& p, const float& time)
{
	p.Age += time;
	p.Position += p.Velocity;
	p.Velocity.y -= 0.0075f*g_TimeScale;
}

// Particle callback function
void ParticleProc2(Particle& p, const float& time)
{
	p.Age += time;
	p.Position += p.Velocity;
	p.Velocity *= 0.9f;
}

// Particle callback function
void ParticleProcCyclone(Particle& p, const float& time)
{
	p.Age += time;
	static float theta=0;
	const float radius = 0.25;
	theta += time*0.01f;
	p.Position.x = cosf(theta)*radius*(p.Age+1);
	p.Position.y = 2*p.Age;
	p.Position.z = sinf(theta)*radius*(p.Age+1);
	//p.Velocity *= 0.9f;
}


// Process the demo mode
int g_DemoMode = 0;
const int g_NumEmitters = 3;
ParticleEmitter* g_Emitters[g_NumEmitters];
MeshObject* Orc1;

void ActivateEmitter(int index)
{
	for(int i=0; i<g_NumEmitters; i++)
		if(i!=g_DemoMode)
			g_Emitters[i]->Activate(false);
	g_Emitters[g_DemoMode]->Activate(true);
	g_DemoMode=index;
}

void InitDemo(Renderer& app)
{
	

	// one only
	//app.CreateHeightmap(256);
	app.CreateWater(129, 129, 1, 8);

	// multiples
	//Orc1 = app.LoadMesh("Models\\Old\\Dwarf.x");
	//app.AddMesh(Orc1);
	

	g_Emitters[0] = app.CreateParticleEmitter("Textures\\particle4.jpg", 10000, 50, 4.0f);
	g_Emitters[0]->SetCallback(&ParticleProc1);
	g_Emitters[1] = app.CreateParticleEmitter("Textures\\particle2.jpg", 10000, 30, 4.0f);
	g_Emitters[1]->SetCallback(&ParticleProc2);
	g_Emitters[2] = app.CreateParticleEmitter("Textures\\particle.jpg", 10000, 10, 10.0f);
	g_Emitters[2]->SetCallback(&ParticleProcCyclone);
	ActivateEmitter(0);	
}



void DemoProc(Renderer& app)
{
	
	if(Input::KeyDown(DIK_1))
		ActivateEmitter(0);		
	else if(Input::KeyDown(DIK_2))
			ActivateEmitter(1);
	else if(Input::KeyDown(DIK_3))
		ActivateEmitter(2);

	static D3DXVECTOR4 particleColor = D3DXVECTOR4(1, 1, 1, 1);
	static D3DXVECTOR3 emitterPos;
	static float theta=0, radius=25;

	switch(g_DemoMode)
	{
	case 0:
		{
			// Update the particle color
			particleColor.x += app.GetElapsedTime()*0.25f;
			if(particleColor.x>1.0f)
				particleColor.x=0;
			particleColor.y += app.GetElapsedTime()*0.1f;
			if(particleColor.y>1.0f)
				particleColor.y=0;
			particleColor.z += app.GetElapsedTime()*0.02f;
			if(particleColor.z>1.0f)
				particleColor.z=0;
			g_Emitters[g_DemoMode]->SetParticleColor(particleColor);

			break;
		}

	case 1:
		{
			// Move the emitter in a circle
			theta += app.GetElapsedTime();
			g_Emitters[g_DemoMode]->SetRotation(D3DXVECTOR3(theta, theta, 0));

			// Update the particle color
			particleColor.x += app.GetElapsedTime()*0.25f;
			if(particleColor.x>1.0f)
				particleColor.x=0;
			particleColor.y += app.GetElapsedTime()*0.1f;
			if(particleColor.y>1.0f)
				particleColor.y=0;
			particleColor.z += app.GetElapsedTime()*0.02f;
			if(particleColor.z>1.0f)
				particleColor.z=0;
			g_Emitters[g_DemoMode]->SetParticleColor(particleColor);

			break;
		}

	case 2:
		{
			// Move the emitter in a circle
			theta += app.GetElapsedTime();
			g_Emitters[g_DemoMode]->SetRotation(D3DXVECTOR3(theta, theta, 0));

			// Update the particle color
			particleColor.x += app.GetElapsedTime()*0.25f;
			if(particleColor.x>1.0f)
				particleColor.x=0;
			particleColor.y += app.GetElapsedTime()*0.1f;
			if(particleColor.y>1.0f)
				particleColor.y=0;
			particleColor.z += app.GetElapsedTime()*0.02f;
			if(particleColor.z>1.0f)
				particleColor.z=0;
			g_Emitters[g_DemoMode]->SetParticleColor(particleColor);

			break;
		}
	}
}


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	
	LOG( "Application starting..." );
	
	g_szDirectory = "D:\\C++ConsolePractice\\Phase\\Lib\\"; // this need to be changed.

	// Create the renderer
	Renderer app;
	HWND hWnd = InitWindow( hInstance, nCmdShow, 500, 300 );
	if(!hWnd)
		return 0;

	// Setup direct input
	Input::Init(hWnd);

	// Create the renderer and enter the main loop
	MSG msg = {0};
	if( app.Create(hWnd, true) )
	{
		// Setup the emitters
		InitDemo(app);

		// Set the time of day
		app.SetTimeOfDay(3, 0, 0);

		// Position camera
		app.GetCamera().SetPos(D3DXVECTOR3(0, 5, -50));
	
		
		// Main render loop
		while( WM_QUIT != msg.message )
		{
			if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			else
			{
				// Update the input		
				Input::Update();

				// Process camera motion
				UpdateCamera(app.GetCamera());
				
				// Update the particle emitters
				DemoProc(app);

				// Draw the scene
				app.Render();
			}
		}
	}

	// Cleanup
	LOG( "Exiting now..." );
	Input::Release();
	app.Destroy();
	
	return ( int )msg.wParam;
}


