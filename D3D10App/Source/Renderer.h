//--------------------------------------------------------------------------------------
// File: Renderer.h
//
// Implementation of the Phase rendering system
//
// Includes deferred renderer, transparent mesh forward renderer and shadow mapping
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "D3D10.h"

#include "Text.h"

#include "Vertex.h"
#include "RenderSurface.h"
#include "Texture.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "Device.h"

#include "Terrain.h"
#include "MeshObject.h"
#include "Probe.h"
#include "Effect.h"
#include "HDR.h"
#include "Sky.h"
#include "Water.h"
#include "Particle.h"


namespace Core
{
	// The main renderer class
	// This controls all of the drawing and manages the details of the scene.
	class Renderer : public D3D10App
	{
	public:
		Renderer();

		///////////////////////////////////////
		// Main engine functionality

		// Sets up a D3D10 device in the given window
		bool Create(HWND hWnd, bool bWindowed);

		// Resizes the device swap chains when the window size changes
		HRESULT Resize();

		// Called after the creation of a device.
		// Sets up any d3d objects that are not dependent on screen size changes
		HRESULT OnCreateDevice();

		// Called after the screen is resized.
		// Sets any d3d objects that depend on the screen size
		HRESULT OnResizedSwapChain();

		// Called before the screen is reset.
		// Releases objects created in OnResizeSwapChain
		void OnReleasingSwapChain();

		// Called before the device is released.
		// Releases objects created in OnCreateDevice
		void OnDestroyDevice();

		// Clears the current scene
		void DestroyScene();

		// Closes the renderer
		void Destroy();



		// Called once per frame, before rendering.
		// Updates the scene and prepares it for rendering
		void OnFrameMove();

		// Called once per frame, during rendering.
		// Draws the scene
		void OnFrameRender();

		// Renders the scene
		void Render();

		// Takes a screenshot and saves it to a file
		void TakeScreenShot();

		// Creates the shadow system
		HRESULT InitShadowSystem(UINT resolution, UINT samples, UINT quality, UINT mips);
		
		// Frees memory used by the shadow system
		void ReleaseShadowSystem();

		
		// True if the mouse is on the render screen
		inline bool MouseOnScreen(){
			return !(m_MousePos.x<0||m_MousePos.y<0||m_MousePos.x>(int)m_Width||m_MousePos.y>(int)m_Height);
		}

		// Gets the mouse position
		inline POINT& GetMousePos(){ return m_MousePos; }



		///////////////////////////////////////
		// Scene management

		// Optimizes the scene structure
		void Optimize();
		
		// Searches the material list, and deletes any materials that are not used by a mesh
		void RemoveUnusedMaterials();
		
		// Process engine messages that are queued
		void ProcessMessages();		

		// Creates a new light.  This does not add it to the scene
		Light* CreateLight(bool bShadowCasting);

		// Adds a light to the scene
		void AddLight(Light* pLight);

		// Removes a light from the scene, but does not delete it
		void RemoveLight( Light* pLight );

		// Permanently deletes a light
		void DeleteLight( Light* pLight );

		// Get the number of lights contained in the scene
		inline int GetNumLights(){ return m_Lights.Size(); }
		
		// Get a light using it's index in the scene list
		inline Light* GetLight(int i){ return m_Lights[i]; }

		// Checks if a light is within the camera view.  This does not take occulsion into account
		inline bool IsLightVisible(Light& light){
			return m_Camera.GetFrustum().CheckSphere( light.GetPos(), light.GetRange() );
		}

		// Loads a mesh from a file.  This does not add it to the scene
		MeshObject* LoadMesh( const char* file );
		
		// Adds a mesh object to the scene
		void AddMesh( MeshObject* pMesh );

		// Get the number of meshes contained in the scene
		inline int GetNumMesh(){ return m_MeshObjects.Size(); }
		
		// Get a mesh using it's index in the scene list
		inline MeshObject* GetMesh(int i){ return m_MeshObjects[i]; }

		// Removes a mesh from the scene, but does not delete it
		void RemoveMesh( MeshObject* pMesh );

		// Permanently deletes a mesh
		void DeleteMesh( MeshObject* pMesh );

		// Set the color of the ambient lighting
		inline void SetAmbientLighting(float r, float g, float b){ m_Ambient = D3DXVECTOR4(r,g,b,0);  Device::Effect->AmbientVariable->SetFloatVector((float*)&m_Ambient); }

		// Get a the active camera
		inline Camera& GetCamera(){ return m_Camera; }

		


		///////////////////////////////
		// Particle systems

		// Create a new emitter
		ParticleEmitter* CreateParticleEmitter(char* texFile, int maxParticles, int freq, float lifetime);

		// Delete an emitter
		void DeleteParticleEmitter(ParticleEmitter* emitter);

				
		///////////////////////////////
		// Environment probes

		// Create a new environment probe
		EnvironmentProbe* AddProbe(EnvironmentProbe::PROBE_TYPE type, bool dynamic);

		// Attaches a mesh to an environment probe.
		// The mesh will use this probe for computing reflections/refractions
		void AttachMeshToProbe(EnvironmentProbe* probe, MeshObject* mesh);

		// Removes a probe from the scene and deletes it
		void RemoveProbe(EnvironmentProbe* probe);

		// Gets the number of environment probes that are active in the scene
		inline int GetNumProbes(){ return m_Probes.Size(); }
		
		// Gets an environment probe from it's index in the scene list
		inline EnvironmentProbe* GetProbe(int i){ return m_Probes[i]; }


		///////////////////////////////
		//Water   use Search "//Water" to search all the water code I implemented. -Shan.	
		// Water systems

		Water* CreateWater(int width, int height, int spacing, int waterheight);

		///////////////////////////////
		// Terrain support

		// Creates a terrain from a heightmap file
		Terrain* ImportHeightmap(char* file);

		// Saves the terrain heightmap to a png
		void ExportHeightmap(char* file);

		// Creates a blank terrain
		Terrain* CreateHeightmap(UINT size);

		// Get the terrain.  Will return NULL if no terrain exists
		inline Terrain* GetTerrain(){ return m_pTerrain; }

		// Gets the intersection point of the mouse cursor and the terrain
		// and the normal at that location
		bool MouseTerrainIntersect(D3DXVECTOR3& oPos, D3DXVECTOR3& oNormal);

	

		///////////////////////////////////////
		// Properties and accessors

		// The screen width
		inline int GetWidth(){ return m_Width; }

		// The screen height
		inline int GetHeight(){ return m_Height; }
		
		// Total number of polygons in the current scene
		inline int GetScenePolyCount(){ return m_TotalPolyThisScene; }
		
		// The time it took to render the previous frame
		inline float GetElapsedTime(){ return Device::FrameStats.ElapsedTime; }

		
		

		


		///////////////////////////////////
		// Time of day system
		
		// Sets the current time, and adjusts the sun position accordingly
		inline void SetTimeOfDay(int hours, int minutes, int seconds){
			m_SkySystem.SetTimeOfDay(hours, minutes, seconds);
			m_UpdateSky = true;
		}

		// Sets the current date, and adjusts the sun position accordingly
		inline void SetDate(int month, int day, int year){
			m_SkySystem.SetDate(month, day, year);
		}


		// Cloud shape
		inline void SetPerlinLacunarity(float f){ Device::Effect->PerlinLacunarityVariable->SetFloat(f); }
		inline void SetPerlinGain(float f){ Device::Effect->PerlinGainVariable->SetFloat(f); }
		inline void SetPerlinScale(float f){ Device::Effect->PerlinScaleVariable->SetFloat(f); }
		inline void SetCloudCover(float f){ Device::Effect->CloudCoverVariable->SetFloat(f); }
		inline void SetCloudSharpness(float f){ Device::Effect->CloudSharpnessVariable->SetFloat(f); }

		

		///////////////////////////////////
		// Engine properties
	private:
		int		m_TotalPolyThisScene;	// Total polygons in this scene
		POINT   m_MousePos;				// Mouse coords
		
		D3DXVECTOR4 m_Ambient;	// Ambient lighting
		
		///////////////////////////////////
		// Text system
		CDXUTTextHelper*	 m_pTxtHelper;	// Text batching helper
		ID3DX10Font*         m_pFont;		// Font for drawing text
		ID3DX10Sprite*       m_pSprite;		// Sprite for batching text drawing


		///////////////////////////////////
		// Scene Objects
		Camera					m_Camera;				// Default camera
		Light					m_Sun;					// The sun light
		Array<ParticleEmitter*> m_Emitters;				// Particle emitters
		Array<Light*>			m_Lights;				// Scene lights
		Stack<Light*>			m_LightPool;			// Recycle bin for lights
		Array<MeshObject*>		m_MeshObjects;			// BaseMesh objects
		Array<SubMesh*>			m_TransparentObjects;   // Objects with transparency enabled
		Array<SubMesh*>			m_RenderList;			// Sorted list of submeshes to be rendered
		MeshObject				m_SphereMesh;			// Sphere mesh
		MeshObject				m_BoxMesh;				// Box mesh
		MeshObject				m_ConeMesh;				// Cone mesh
		Array<Light*>			m_VisibleLights;		// List of visible lights
		Terrain*				m_pTerrain;				// The terrain

		//Water
		Water*                  m_pWater;               // Water~ Later on we set to Array Maybe?

		
		///////////////////////////////////
		// Rendering system objects
		
		// GBuffer types
		enum DS_TYPE
		{
			DS_NORMAL=0,
			DS_COLOR,
			DS_DEPTH,
			DS_SIZE,
		};
		
		RenderSurface				m_GBuffer;				// Deferred shading GBuffer
		RenderSurface				m_LightingBuffer;		// Light accumulation buffer
		Vertex						m_QuadVerts[6];			// Verts for the quad
		ID3D10Buffer*				m_pQuadVertexBuffer;	// Contains a quad to render for light passes
		D3DXMATRIX					m_mOrtho;				// Orthoganal projection matrix for light passes
		ID3D10RenderTargetView*		m_pCachedRTV;			// RTV cached
		ID3D10DepthStencilView*		m_pCachedDSV;			// DSV cached
		D3D10_VIEWPORT				m_CachedVP;				// Viewport cached
		RenderSurface				m_FrameBuffer;			// HDR framebuffer surface
		RenderSurface				m_PostFXBuffer;			// RT for post processing
		ID3D10Predicate*			m_pPredicate;			// Occlusion query object
		ID3D10Texture2D*			m_pMaterialTex;			// Encoded material texture
		ID3D10ShaderResourceView*	m_pMaterialSRV;			// Encoded material texture
		ID3D10Texture2D*			m_pRandomTex;			// Encoded material texture
		ID3D10ShaderResourceView*	m_pRandomSRV;			// Encoded material texture
		D3DXMATRIX					m_mViewProj;			// View/projection matrix
		D3DXMATRIX					m_mOldViewProj;			// View/projection matrix from the previous frame

		// Encodes the material data needed for shading into a texture
		void EncodeMaterialTexture();

		// Generates a random sampling texture for SSAO
		void EncodeSSAOTexture(UINT width, UINT height);

		// Set the quad vertex buffer
		UINT m_qOffset;
		inline void SetRenderToQuad()
		{
			Device::SetVertexBuffer(m_pQuadVertexBuffer, Vertex::size);
		}

		// Renders all the probes
		void RenderProbes();
		void UpdateProbe(EnvironmentProbe& probe);
		
		// Render a mesh
		void RenderMesh(MeshObject& mesh, ID3D10EffectPass* pass);

		// Renders the scene
		void RenderScene();
		
		// Render a mesh to the GBuffers
		void RenderMeshDeferred(SubMesh& mesh);

		// Renders each mesh to the GBuffers
		void RenderSceneDeferred();

		// Performs the shading phase on the GBuffers
		void ShadeSceneDeferred();

		// Render the transparent meshes
		void RenderTransparents();

		// Render a single transparent mesh
		void RenderTransparentMesh(SubMesh& mesh);

		// Renders a shadowmap
		void RenderShadowMap(Light& light);

		// Renders the terrains
		void RenderTerrain(Frustum& frustum, ID3D10EffectPass* pPass);
		
		//Water Rendering
		void RenderWater();

		// Draw the sky
		void RenderSky(const D3DXVECTOR3& pos);

		// Render the output text
		void RenderText();

		// Renders the focus mode mesh
		void RenderFocusMesh();

	
		
		////////////////////////////////////////////////
		// Sky system
		Sky m_SkySystem;
		bool m_UpdateSky;
		

		

		////////////////////////////////////////////////
		// HDR system
		HDRSystem m_HDRSystem;

		



		////////////////////////////////////////////////
		// Shadow system

		// Shadow map quality settings
		struct ShadowSettings
		{
			static UINT Resolution;
			static DXGI_SAMPLE_DESC MSAA;
			static UINT MipLevels;
			static D3D10_VIEWPORT Viewport;

			// Default settings
			ShadowSettings()
			{
				Resolution = 1024;
				MSAA.Count = 4;
				MSAA.Quality = 4;
				MipLevels = 1;
			}
		};

		RenderSurface				m_ShadowMap;			// Shadow map
		RenderSurface				m_ShadowMapMSAA;		// Shadow map with MSAA
		RenderSurface				m_ShadowMapCube;		// Cube shadow map
		DepthStencil				m_ShadowMapDepth;		// Regular shadow map depth target
		DepthStencil				m_ShadowMapDepthMSAA;	// MSAA enabled smap depth
		DepthStencil				m_ShadowMapDepthCube;	// Cube shadow map depth
		RenderSurface				m_ShadowMapBlur;		// Texture for blurring shadow maps
		D3DXMATRIX					m_mShadowOrtho;			// Orthoganal projection matrix for shadow blur
		D3DXMATRIX					m_mShadowProjSpot;		// Shadow map projection matrix for spotlights
		D3DXMATRIX					m_mShadowProjCube;		// Shadow map projection matrix for point lights





		////////////////////////////////////////////////
		// Post FX
		RenderSurface				m_VelocityMap[2];			// Per-pixel velocity for motion blur
		RenderSurface				m_SSAOBuffer[2];			// SSAO 8-bit buffer 1/2 screen size
		UINT						m_CurVelocity;				// Current velocity sampler
		UINT						m_OldVelocity;				// Old velocity sampler
		



		////////////////////////////////////////////////
		// Environment mapping system

		// Quality settings
		static struct ProbeSettings
		{
			static UINT resolution;
			static DXGI_FORMAT format;
			static UINT miplevels;

			ProbeSettings()
			{
				resolution = 512;
				format = DXGI_FORMAT_R8G8B8A8_UNORM;
				miplevels = 4;
			}
		};	
		Array<EnvironmentProbe*>	m_Probes;
		DepthStencil				m_ProbeDepth;
		D3DXMATRIX					m_mProbeProj;


		///////////////////////////////////
		// Effect framework

		// Re-binds effect variables after recompiling a shader
		// All render targets must place their attach code in here as
		// well as in their init area or they will not work when
		// the shader is recompiled
		void RebindEffectVariables();

		// Main effect object
		Effect m_Effect;

		

#ifdef PHASE_DEBUG

		// Material preview texture
		RenderSurface m_DebugMaterialPreviewMSAA;
		DepthStencil m_DebugMaterialPreviewDepthMSAA;
		RenderSurface m_DebugMaterialPreview;
		MeshObject m_DebugMaterialPreivewMesh;

		// Mesh preview rendering
		RenderSurface m_DebugMeshPreviewMSAA;
		DepthStencil m_DebugMeshPreviewDepthMSAA;
		RenderSurface m_DebugMeshPreview;
public:
		// Render a material preview texture to a file
		String DebugRenderMaterialPreview(Material& mat);
		String DebugRenderMeshPreview(MeshObject* pMesh);
				
		////////////////////////////////////
		// Picking and debug
	
		// Init debug renderer
		HRESULT InitDebugRenderer();
		void ReleaseDebugRenderer();

		// Reload the effect
		inline void ReloadEffect()
		{
			if(SUCCEEDED(Device::Effect->Create("Shaders\\Main.fx")))
				RebindEffectVariables();
		}

		// Debug mesh rendering
		struct DebugRenderTarget
		{
			DebugRenderTarget(){
				pMesh = NULL;
				bWireframe = false;
				TerrainWidget = false;
			}
			
			MeshObject* pMesh;
			int submesh;
			bool bWireframe;
			D3DXVECTOR4 color[2];
			bool boundingBox;
			D3DXVECTOR3 pos;
			D3DXVECTOR3 boundSize;
			bool TerrainWidget;
		};
		

		// Adds a mesh to the debug render list
		void DebugRenderMesh(MeshObject* pMesh, int submesh=-1, bool wire=true, bool boundingBox=true, D3DXVECTOR4 color1=D3DXVECTOR4(0.85f,0.25f,0,0), D3DXVECTOR4 color2 = D3DXVECTOR4(0,0.25f,0.85f,0));
		void DebugRenderBox(D3DXVECTOR3 pos, D3DXVECTOR3 size, bool wire=true, D3DXVECTOR4 color1=D3DXVECTOR4(0.85f,0.25f,0,0), D3DXVECTOR4 color2 = D3DXVECTOR4(0,0.25f,0.85f,0));
		inline void DebugAddRenderMesh(DebugRenderTarget& rt){ m_DebugRenderMeshList.Add(rt); }

		// Renders a selection box on the screen
		void DebugSelectionBox();
		inline void DebugDrawSelectionBox(){ m_DebugRenderSelectionBox=true; }
		bool m_DebugRenderSelectionBox;
		
		// Rendering
		void DebugRender();
		inline void SetDemoMode(bool b){m_bDemoMode=b;}

		// Selection test
		inline bool SelectionBoxTest(D3DXVECTOR3& pos, float radius){ return m_SelectionBoxFrustum.CheckSphere(pos, radius); }
		inline bool SelectionBoxTest(D3DXVECTOR3& pos, D3DXVECTOR3& size){ return m_SelectionBoxFrustum.CheckRectangle(pos, size); }

		// Set the selected light to render
		inline void DebugRenderLight(Light* pLight){ m_DebugRenderLightList.Add(pLight); }

		// Transform axis types
		enum DEBUG_TYPE
		{
			DEBUG_SELECT=0,
			DEBUG_MOVE,
			DEBUG_ROTATE,
			DEBUG_SCALE,
		};

		// Different axes
		enum AXIS_TYPE
		{
			AXIS_X=0,
			AXIS_Y=1,
			AXIS_Z=2,
			AXIS_NONE=-1,
		};
		
		// Renders the transform axes at the given location
		void DebugRenderAxes(const D3DXVECTOR3& pos, DEBUG_TYPE type);

		// Checks which debug axis is selected
		void DebugCheckAxis();

		// Sets the axis type
		inline void SetDebugRenderMode(DEBUG_TYPE type){ m_DebugRenderMode=type; }

		// Gets the axis the mouse is over
		inline AXIS_TYPE GetSelectedDebugAxis(){ return m_SelectedDebugAxis; }

		
		SCULPT_TYPE m_DebugTerrainSculptMode;
		float m_DebugTerrainSculptDelta;
		float m_DebugTerrainSculptRadius;
		float m_DebugTerrainSculptHardness;
		float m_DebugTerrainSculptStrength;
		int m_DebugTerrainSculptDetail;
		inline void DebugSetTerrainWidgetRadius(float f){ m_DebugTerrainSculptRadius=f; Device::Effect->TerrainWidgetRadiusVariable->SetFloat(f); }
		inline void DebugSetTerrainWidgetHardness(float f){ m_DebugTerrainSculptHardness=f; Device::Effect->TerrainWidgetHardnessVariable->SetFloat(f); }
		inline void DebugSetTerrainWidgetStrength(float f){ m_DebugTerrainSculptStrength=f; }
		inline void DebugSetTerrainWidgetDetail(int f){ m_DebugTerrainSculptDetail=f; }
		inline void DebugSetTerrainWidgetDelta(float f){ m_DebugTerrainSculptDelta=f; }
		inline void DebugUpdateTerrain(){ if(m_pTerrain) m_pTerrain->Update(); }
		bool m_DebugTerrainSculpt;
		inline void DebugDoTerrainSculpt(){m_DebugTerrainSculpt=true;}
		void DebugTerrainSculpt(D3DXVECTOR3 pickPoint);
		inline void DebugSetTerrainSculptMode(SCULPT_TYPE mode){ m_DebugTerrainSculptMode=mode; }
		
		// Render the terrain sculpt tool
		void DebugRenderTerrainWidget();

		// Enable grid
		inline bool IsGridEnabled(){ return m_DebugDrawGrid; }
		inline void DrawGridLines(bool enable){ m_DebugDrawGrid=enable; }

		///////////////////////////////////
		// Debug scene objects
	private:
		Texture*				m_pLightTex;			// Texture for the light billboard
		int			 		    m_iPickedSubMesh;		// Index of the submesh for the most recent picked mesh
		ID3D10Buffer*			m_pDebugLineOutlineVB;  // Vertex buffer to store the lines for the light borders
		ID3D10Buffer*			m_pDebugLineVB;			// A single line buffer
		ID3D10Buffer*			m_pDebugLineBoxVB;		// A buffer for a box made out of lines
		bool					m_bDemoMode;			// Demo mode
		Array<MeshObject*>		m_TestObjects;			// For demo mode
		Array<Light*>			m_TestLights;			// For demo mode
		Frustum					m_SelectionBoxFrustum;	// View frustum for the object selection box
		float					m_DebugLightSize;		// Size scale for rendering lights in debug mode
		DEBUG_TYPE				m_DebugRenderMode;		// Type of debug mode to render
		MeshObject				m_DebugAxisCone;		// Mesh for the debug axis rendering
		MeshObject				m_DebugAxisPyre;		// Mesh for the debug axis rendering
		MeshObject				m_DebugAxisBall;		// Mesh for the debug axis rendering
		AXIS_TYPE				m_SelectedDebugAxis;	// The transform axis that the mouse is currently over
		bool					m_DebugDrawGrid;
		
		Array<DebugRenderTarget> m_DebugRenderMeshList;
		Array<Light*> m_DebugRenderLightList;

		///////////////////////////////////
		// Render Modes:
		// Focus mode will use a fixed camera about the focus mesh
		// and only render the focus mesh
	private:
		// The camera cache object is used to cache the camera
		// info while the engine is in mesh focus mode so
		// it can be restored after
		struct CameraCache
		{
			static Camera::CAMERA_TYPE Type;
			static D3DXVECTOR3 Position;
			static D3DXVECTOR2 Direction;
			static D3DXVECTOR3 MeshRotation;
		};
		MeshObject*				m_pFocusMesh;		// The mesh currently in focus.  Set to NULL when not in focus mode
		Light					m_FocusLights[4];	// The lights used in focus mode
		


		//////////////////////////////////////////////
		// Focus Mode

		// Set the render mode to have a fixed camera and only render the focus mesh
		// Camera data is cached during this mode for easy return to original mode
	public:
		inline MeshObject* GetFocusMesh(){ return m_pFocusMesh; }
		inline void SetFocusMode(MeshObject* pMesh)
		{
			if( pMesh != NULL )
			{
				CameraCache::Type = m_Camera.GetType();
				CameraCache::Position = m_Camera.GetPos();
				CameraCache::Direction.x = m_Camera.GetXDeg();
				CameraCache::Direction.y = m_Camera.GetYDeg();
				CameraCache::MeshRotation = pMesh->GetRot();
				m_Camera.SetType(Camera::CAMERA_FIXED);
				m_Camera.SetFocusPoint(pMesh->GetPos());
				Device::SetClearColor(0.25f,0.25f,0.8f);
			}
			else
			{
				// Restore camera values
				m_Camera.SetType(CameraCache::Type);
				m_Camera.SetXDeg(CameraCache::Direction.x);
				m_Camera.SetYDeg(CameraCache::Direction.y);
				m_Camera.SetPos(CameraCache::Position);
				m_pFocusMesh->SetRot(CameraCache::MeshRotation);
				Device::SetClearColor(0,0,0);
			}
			m_pFocusMesh=pMesh;
		}


#endif

	};
}

