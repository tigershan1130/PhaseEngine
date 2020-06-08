//--------------------------------------------------------------------------------------
// File: Model.h
//
// 3D BaseMesh
// A Core Engine mesh consists of a submesh for each material.
// This is simply a data container, meshes are instantiated using MeshObject
// or DynamicMesh.  This allow for the same mesh data to be used for multiple
// objects with little increase in required memory.
//
// Loading is currently supported for 3D Studio Max (.3ds) and DirectX (.x) meshes
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "LinkedList.cpp"
#include "Array.cpp"
#include "SubMesh.h"
#include "QMath.h"

namespace Core
{
		
	// Special mesh load inputs
	extern char MESH_SPHERE[];
	extern char MESH_BOX[];
	extern char MESH_CONE[];


	//--------------------------------------------------------------------------------------
	// Modified frame object to hold the combined transform
	//--------------------------------------------------------------------------------------
	struct D3DXFRAME_DERIVED: public D3DXFRAME
	{
		D3DXMATRIX CombinedTransformationMatrix;
	};

	//--------------------------------------------------------------------------------------
	// Structure derived from D3DXMESHCONTAINER so we can add some app-specific
	//--------------------------------------------------------------------------------------
	struct D3DXMESHCONTAINER_DERIVED: public D3DXMESHCONTAINER
	{
		LPD3DXMESH           pOrigMesh;
		LPD3DXATTRIBUTERANGE pAttributeTable;
		DWORD                NumAttributeGroups; 
		DWORD                NumInfl;
		LPD3DXBUFFER         pBoneCombinationBuf;
		D3DXMATRIX**         ppBoneMatrixPtrs;
		D3DXMATRIX*          pBoneOffsetMatrices;
		DWORD                NumPaletteEntries;
	};

		

	//--------------------------------------------------------------------------------------
	// A 3D BaseMesh grouped into submeshes by material, skinned animation is supported
	//--------------------------------------------------------------------------------------
	class BaseMesh : public EngineResource
	{
	friend class ResourceManager<BaseMesh>;
	friend class MeshObject;
	public:
			BaseMesh();
			~BaseMesh();
			
			// Get a mesh
			inline Array<SubMesh>& GetSubMesh(){ return m_pSubMesh; }
			inline SubMesh* GetSubMesh(int i){ return &m_pSubMesh[i]; }
			
			// Get the number of meshes
			inline int GetNumSubMesh(){ return m_pSubMesh.Size(); }

			// Number of polys
			inline int GetNumPoly(){ return m_pIndices.Size()/3; }

			// Updates the geo buffers
			HRESULT FillBuffers();

			// Centers the mesh about the origin
			void CenterMesh();

			// BaseMesh data
			inline Vertex* GetVerts(){ return m_pVerts; }
			inline SkinnedVertex* GetSkinnedVerts(){ return m_pSkinnedVerts; }
			inline int GetNumVerts(){ if(m_bSkinned) return m_pSkinnedVerts.Size(); return m_pVerts.Size(); }
			inline DWORD* GetIndices(){ return m_pIndices; }
			inline int GetNumIndices(){ return m_pIndices.Size(); }
			inline int GetNumFaces(){ return m_pIndices.Size()/3; }

			// Get an array with the default materials
			inline Array<Material*>& GetDefaultMaterials(){ return m_pMaterials; }

			// Cleans the mesh by removing duplicate verts
			void Clean();

			// Swap tex coords
			void FlipUV();

			// Set to TRUE to disable mesh material creation
			// (used for loading scenes)
			static bool bLoadMaterials;

	protected:
			ID3D10Buffer*               m_pVertexBuffer;
			ID3D10Buffer*               m_pPosVertexBuffer;
			ID3D10Buffer*               m_pIndexBuffer;
			Array<SubMesh>				m_pSubMesh;			// The sub meshes
			Array<Material*>			m_pMaterials;		// The materials
			Array<Vertex>				m_pVerts;			// Vertex array
			Array<PosVertex>			m_pPosVerts;		// Vertex array (positions only for faster shadow rendering)
			Array<DWORD>				m_pIndices;			// Index array
			bool						m_bLoaded;			// Is it loaded?

			//---------------------------------------
			// Skinned mesh stuff
			Array<SkinnedVertex>		m_pSkinnedVerts;		// Skinned Vertex array
			bool						m_bSkinned;				// True if a skinned mesh

			// Finds the node that contains mesh data
			D3DXFRAME* FindNodeWithMesh(D3DXFRAME* frame);

			// Update the frame matrices
			void UpdateFrameMatrices( LPD3DXFRAME pFrameBase, LPD3DXMATRIX pParentMatrix );
				
			// 3ds vertex
			struct Vertex3DS
			{
				D3DXVECTOR3 pos;
				float u,v;
			};

			// Loads the model
			bool Load(const char* file);
			bool FromXMesh(LPD3DXMESH pMesh);
			bool LoadX(const char* file);
			bool LoadSkinnedX(const char* file);


			void Release();
	} ;

	// The global model manager
	extern ResourceManager<BaseMesh> g_Meshes;




}