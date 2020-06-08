//--------------------------------------------------------------------------------------
// File: Terrain.h
//
// Terrain using gpu geometry clipmaps
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once
#include "Vertex.h"
#include "Array.cpp"
#include "Material.h"
#include "Camera.h"
#include "MeshObject.h"
#include "ThreadPool.h"
#include "Effect.h"
#include "Clipmap.h"

namespace Core
{


#define CLIPMAP_LEVELS 8

	// Perform sculpting
	enum SCULPT_TYPE
	{
		SCULPT_RAISE=0,
		SCULPT_LOWER,
		SCULPT_GRAB,
		SCULPT_SMOOTH,
		SCULPT_RAMP,
		SCULPT_NOISE,
		SCULPT_EROSION,
		SCULPT_PAINT,
	};



	
	class Terrain : public WorkerThread
	{
		friend class Renderer;
	public:
		Terrain();

		
	//--------------------------------------------------------------------------------------
	// Creation
	//--------------------------------------------------------------------------------------

		// Create a terrain from a heightmap
		HRESULT LoadTerrain(const char* szFile, ID3D10DepthStencilView* pDSV);
		HRESULT CreateFromFile(const char* szFile, Camera* pCam);
		HRESULT Create(UINT size, Camera* pCam);
		HRESULT CreateFromTexture(ID3D10Texture2D* pHeightmap,  ID3D10Texture2D* pBlendmap, Camera* pCam);

		// Internal data loading
	private:
		UINT LoadHeightMap(ID3D10Texture2D* pHeightmap,  ID3D10Texture2D* pBlendmap);
	public:

		// Saving
		void ExportHeightmap(char* file);

		// Destroy the terrain
		void Release();
		
		
		
	//--------------------------------------------------------------------------------------
	// Misc properties
	//--------------------------------------------------------------------------------------
		
		// Vertex/index buffers
		inline ID3D10Buffer* GetVertexBuffer(){ return m_VertexBuffer; }
		inline ID3D10Buffer* GetIndexBuffer(){ return m_IndexBuffer; }
		
		// Gets the world matrix
		inline D3DXMATRIX& GetWorldMatrix(){ return m_WorldMatrix; }
		
		// Returns the name
		inline const String& GetName(){ return m_szName; }
		
		// Is the terrain loaded?
		inline bool IsLoaded(){ return m_bLoaded; }
		
		// Returns the heightfield
		inline Array<D3DXFLOAT16>* GetHeightfield(){ return &m_Height; }

		// Sets the size scale (not working)
		inline void SetScale(float f){ 
			/*float d = 2.0f * ((f-m_Scale) / (m_Scale+f)); 
			m_Material.SetDiffuse(m_Material.GetDiffuse() + m_Material.GetDiffuse()*d);
			m_Scale = f; 
			UpdateWorldMatrix(); */
		}
		
		// Set the factor to scale the heightmap by
		inline void SetHeightScale(float f){ m_HeightExtents/=m_HeightScale;  m_HeightScale = f; m_HeightExtents*=f; UpdateWorldMatrix(); }

		// Get scales
		inline float GetScale(){ return m_Scale; }
		inline float GetHeightScale(){ return m_HeightScale; }

		// Performs a ray intersection test with the terrain
		bool RayIntersection(D3DXVECTOR3& rayPos, D3DXVECTOR3& rayDir, float& dist);

		
	private:

		String				m_szName;			// File name
		bool				m_bLoaded;			// True when a terrain is loaded
		float				m_Scale;			// Scale factor for the terrain
		float				m_HeightScale;		// Height scale factor
		int					m_Size;				// Heightmap dimensions
		D3DXVECTOR3			m_vPos;				// Center of the heightmap
		Array<D3DXFLOAT16>	m_Height;			// Height list
		D3DXMATRIX			m_WorldMatrix;		// World transform
		bool				m_bInThread;		// True if the async thread is running
		bool				m_bQueueThread;		// True if it needs to update again after the thread finishes
		


	//--------------------------------------------------------------------------------------
	// Updates and rendering
	//--------------------------------------------------------------------------------------

		// Draws the full terrain
		int Render(Effect& effect);

#ifdef PHASE_DEBUG
		// Draws the terrain in debug visualization
		void RenderDebug(MeshObject& boxMesh, Effect& effect);
#endif

		// Update a clipmap
		// (must set quad vertex buffer first)
		void UpdateClipmaps(Effect& effect);

		// Updates the world matrix
		void UpdateWorldMatrix();

		// Asyncronous updating
		void UpdateBuffers();

		// Updates the normals and height values in a new thread
		inline void Update(){ 
			if(m_bInThread){ 
				m_bQueueThread=true; 
				return; 
			} 
			g_ThreadPool.SubmitJob(this); 
		}

		// Terrain buffer updating is threaded to help with streaming and deforming
		unsigned virtual ThreadExecute();

		
	//--------------------------------------------------------------------------------------
	// Sculpting
	//--------------------------------------------------------------------------------------
#ifdef PHASE_DEBUG
public:
		// Terrain sculpting
		void Sculpt(D3DXVECTOR3 center, float radius, float hardness, float strength, float delta, int detail, SCULPT_TYPE type);

		// Cache the region about to get sculpted into the undo stack
		void CacheSculptRegion(D3DXVECTOR3 minPoint, float radius, bool paint);
		
		// Reverse a sculpt/paint action
		void UndoSculpt();

		// Redo a sculpt/paint action
		void RedoSculpt();

		// Get the total memory usage of the sculpter
		inline double GetMemoryUsage(){ return m_UndoMemUsage; }

private:
	
		// Maximum memory usage by the undo system
		double m_UndoMaxMem;
		double m_UndoMemUsage;
			
		// Undo and redo is build in here to simplify things
		struct SculptRegion
		{
			ID3D10Texture2D* tex;
			bool paint;
			D3D10_BOX region;
			double mem;
		};
		Stack<SculptRegion> m_UndoStack;
		Stack<SculptRegion> m_RedoStack;

		// Internal helper for undo
		void ApplyUndo( SculptRegion &r );	

#endif



	//--------------------------------------------------------------------------------------
	// Texturing
	//--------------------------------------------------------------------------------------
		#define NUM_LAYERS 16

		//Each layer contains a texture and a normal map
		struct Layer
		{
			String texMap;
			String normalMap;
			Layer(){ texMap=normalMap="None"; }
		};

		// Blendmap texel mapping
		struct BlendTexel
		{
			UCHAR data[4];
		};

		// Sets up the texture arrays
		void BuildTextureLayers(int res, int mips);
		
		// Texture layers
		ID3D10Texture2D*			m_TextureLayerArray;
		ID3D10Texture2D*			m_NormalLayerArray;
		ID3D10ShaderResourceView*   m_TextureLayers;
		ID3D10ShaderResourceView*	m_NormalLayers;
		BOOL						m_HasTextureLayer[NUM_LAYERS];
		BOOL						m_HasNormalLayer[NUM_LAYERS];
		float						m_LayerScales[NUM_LAYERS];
		Layer						m_Layers[NUM_LAYERS];
		int							m_LayerSize;
		int							m_LayerMips;

public:
		// Load a texture
		void LoadTexture(char* file, int layer);

		// Load a normalmap
		void LoadNormalmap(char* file, int layer);

		// Get a detail texture file name
		String GetTextureName(int i);

		// Get a normalmap file name
		String GetNormalmapName(int i);

		// Clear a texture
		void ClearTexture(int layer);

		// Clear a normalmap
		void ClearNormalmap(int layer);

		// Texture scale factor
		void SetTextureScale(float scale, int layer);

		// Get the detail texture scales
		float GetTextureScale(int layer);

		// Binds the textures and normal maps
		void BindTextures(Effect& effect);

		
	//--------------------------------------------------------------------------------------
	// Clipmap system
	//--------------------------------------------------------------------------------------
private:	
		
		// Heightmaps
		RenderSurface		m_Heightmap;
		ID3D10Texture2D*	m_StagingHeightmap;
		
		// Blendmaps
		Clipmap		m_LayerMaps;
		Clipmap		m_BlendMaps;

		// The min and max heights
		D3DXVECTOR2			m_HeightExtents;
		
		// Creates the clipmap building blocks
		void SetupClipmap(int size, int numLevels);

		// Clipmaps
		int				m_ClipmapSize;		// N
		int				m_BlockSize;		// M = (N+1)/4
		int				m_ClipmapLevels;	// Max number of active levels
		RenderSurface   m_Clipmaps;

		// Trim positioning
		BYTE*			m_TrimXSide;
		BYTE*			m_TrimZSide;
		D3DXVECTOR2*	m_ClipmapOffsets;

		// Camera
		Camera*			m_pCamera;

		// Updating
		bool			m_FlagForUpdate;

		// Block positioning
		D3DXVECTOR2 m_BlockOffsets[12];

		// Vertex buffer offsets
		enum VB_OFFSETS
		{
			VB_BLOCK=0,
			VB_RING1,
			VB_RING2,
			VB_TRIM_TL,
			VB_TRIM_TR,
			VB_TRIM_BR,
			VB_TRIM_BL,
			VB_CENTER_TRIM,

			VB_OFFET_LENGTH,
		};

		// Index buffer offsets
		enum IB_OFFSETS
		{
			IB_BLOCK=0,
			IB_RING1,
			IB_RING2,
			IB_TRIM,
			IB_CENTER_TRIM,

			IB_OFFET_LENGTH,
		};
		
		// Vertex/index buffers
		ID3D10Buffer*		m_VertexBuffer;
		int					m_VBOffsets[VB_OFFET_LENGTH];
		ID3D10Buffer*		m_IndexBuffer;
		int					m_IBOffsets[IB_OFFET_LENGTH];
		int					m_IBCount[IB_OFFET_LENGTH];

		
		
	//--------------------------------------------------------------------------------------
	// Spatial optimization
	//--------------------------------------------------------------------------------------
		
		// This quadtree is purely for fast ray-intersection tests at this point
		struct QuadtreeNode
		{	
			D3DXVECTOR3 pos;
			D3DXVECTOR3 size;
			QuadtreeNode* pChildNodes[4];
			bool isLeaf;
			bool gotHit;

			QuadtreeNode()
			{
				pChildNodes[0] = NULL;
				pChildNodes[1] = NULL;
				pChildNodes[2] = NULL;
				pChildNodes[3] = NULL;
				isLeaf = false;
				gotHit=false;
			}
			
			~QuadtreeNode()
			{
				SAFE_DELETE(pChildNodes[0]);
				SAFE_DELETE(pChildNodes[1]);
				SAFE_DELETE(pChildNodes[2]);
				SAFE_DELETE(pChildNodes[3]);
			}
		};
		QuadtreeNode* m_pRootNode;

		// Builds a very basic quadtree down to a minimal node size
		void BuildQuadtree(QuadtreeNode* pNode, float minNodeSize);

		// Returns the leaf node hit by the ray
		float RayNodeIntersect(QuadtreeNode* pNode, D3DXVECTOR3& rayPos, D3DXVECTOR3& rayDir);
		float RayLeafIntersection(QuadtreeNode*, D3DXVECTOR3& rayPos, D3DXVECTOR3& rayDir);

		
#ifdef PHASE_DEBUG
		// Quadtree visualization
		void RenderQuadtreeDebug(MeshObject& boxMesh, Effect& effect);
		void RenderNodeDebug(QuadtreeNode* pNode, MeshObject& boxMesh, Effect& effect);
#endif

		// Update the node heights
		inline void UpdateTreeHeights(){ UpdateNodeHeight(m_pRootNode); }
		void UpdateNodeHeight(QuadtreeNode* pNode);
	};

}