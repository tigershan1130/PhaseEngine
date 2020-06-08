//--------------------------------------------------------------------------------------
// File: Model.cpp
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

#include "stdafx.h"
#include <d3dx9.h>
#include "Log.h"
#include "Mesh.h"
#include "AllocateHierarchy.h"

#include <iostream>
#include <fstream>
using std::ifstream;
using std::ofstream;
using std::endl;

namespace Core
{

	// Special mesh load inputs
	char MESH_SPHERE[] = "SPHERE_OBJECT";
	char MESH_BOX[] = "BOX_OBJECT";
	char MESH_CONE[] = "CONE_OBJECT";

	// Material loading flag
	bool BaseMesh::bLoadMaterials = true;

	// The global model manager
	ResourceManager<BaseMesh> g_Meshes;


	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	BaseMesh::BaseMesh()
	{
		m_bLoaded=false;
		m_pVertexBuffer = NULL;
		m_pIndexBuffer = NULL;
		m_pPosVertexBuffer = NULL;
		m_bSkinned = false;
	}


	//--------------------------------------------------------------------------------------
	// Destructor
	//--------------------------------------------------------------------------------------
	BaseMesh::~BaseMesh()
	{

	}

	//--------------------------------------------------------------------------------------
	// Destructor
	//--------------------------------------------------------------------------------------
	void BaseMesh::Release()
	{
		Log::Print("BaseMesh::Release(%s)",m_szName.c_str());
		
		// Release the submeshes and PVertex data
		m_pSubMesh.Release();
		m_pVerts.Release();
		m_pPosVerts.Release();
		m_pIndices.Release();
		m_pMaterials.Release();

		// Release the buffers
		SAFE_RELEASE(m_pVertexBuffer);
		SAFE_RELEASE(m_pPosVertexBuffer);
		SAFE_RELEASE(m_pIndexBuffer);
		BaseMesh();
	}

	//--------------------------------------------------------------------------------------
	// Updates the geo buffer
	//--------------------------------------------------------------------------------------
	HRESULT BaseMesh::FillBuffers()
	{
		HRESULT hr;

		// Release the buffers
		SAFE_RELEASE(m_pVertexBuffer);
		SAFE_RELEASE(m_pIndexBuffer);
		
		// Fill the vertex buffer
		D3D10_BUFFER_DESC bd;
		D3D10_SUBRESOURCE_DATA InitData;
		if(m_bSkinned)
		{
			bd.Usage = D3D10_USAGE_IMMUTABLE;
			bd.ByteWidth = sizeof( SkinnedVertex ) * m_pSkinnedVerts.Size();
			bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;
			bd.MiscFlags = 0;
			InitData.pSysMem = (SkinnedVertex*)m_pSkinnedVerts;
			hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );
		}
		else
		{
			bd.Usage = D3D10_USAGE_IMMUTABLE;
			bd.ByteWidth = sizeof( Vertex ) * m_pVerts.Size();
			bd.BindFlags = D3D10_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;
			bd.MiscFlags = 0;
			InitData.pSysMem = (Vertex*)m_pVerts;
			hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );
		}
		if( FAILED(hr) )  
			return hr;

		// Fill the position buffers
		m_pPosVerts.Allocate(m_pVerts.Size());
		for(int i=0; i<m_pVerts.Size(); i++)
			m_pPosVerts[i].pos = m_pVerts[i].pos;
		bd.ByteWidth = sizeof( PosVertex ) * m_pPosVerts.Size();
		InitData.pSysMem = (PosVertex*)m_pPosVerts;
		hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &m_pPosVertexBuffer );
		if( FAILED(hr) )  
			return hr;

		// Update the submeshes
		for(int i=0; i<m_pSubMesh.Size(); i++)
			m_pSubMesh[i].pPosVertexBuffer = m_pPosVertexBuffer;

		// Fill the index buffer
		bd.Usage = D3D10_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof( DWORD ) * m_pIndices.Size();;
		bd.BindFlags = D3D10_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		InitData.pSysMem = (DWORD*)m_pIndices;
		hr = g_pd3dDevice->CreateBuffer( &bd, &InitData, &m_pIndexBuffer );
		if( FAILED(hr) )
			return hr;

		// Set the buffers for the submesh array
		for(int i=0; i<m_pSubMesh.Size(); i++)
		{
			m_pSubMesh[i].pVertexBuffer = m_pVertexBuffer;
			m_pSubMesh[i].pIndexBuffer = m_pIndexBuffer;
		}

		return S_OK;
	}




	//--------------------------------------------------------------------------------------
	// Centers the mesh about the origin
	//--------------------------------------------------------------------------------------
	void BaseMesh::CenterMesh()
	{
		// Calculate the center to offset the verts
		D3DXVECTOR3 vMax(-99999.0f,-99999.0f,-99999.0f),vCenter,vMin(99999.0f,99999.0f,99999.0f),vTemp;
		for(int i=0; i<m_pVerts.Size(); i++)
		{
			vTemp = m_pVerts[i].pos;
			vMin = Math::MinVector( vMin, vTemp );
			vMax = Math::MaxVector( vMax, vTemp );
		}
		vCenter = D3DXVECTOR3( vMax.x - fabs(vMin.x),
						  vMax.y - fabs(vMin.y),
						  vMax.z - fabs(vMin.z) ) / 2.0f;
		for(int i=0; i<m_pVerts.Size(); i++)
			m_pVerts[i].pos -= vCenter;
	}



	//--------------------------------------------------------------------------------------
	// Loads a model file
	//--------------------------------------------------------------------------------------
	bool BaseMesh::Load(const char* file)
	{
		if(file==NULL || file[0] == '\0')
			return false;
		
		if(m_bLoaded)
			Release();

		// Load the mesh
		if( (file[strlen(file)-2] == 's' || file[strlen(file)-2] == 'S') &&
			(file[strlen(file)-1] == 'x' || file[strlen(file)-1] == 'X') )
		{
			Log::Print("Loading Skinned X File");

			// Get the full file path
			String szFile;
			if( (file[0]=='c'||file[0]=='C') && file[1]==':' )
				szFile = file;
			else
				szFile = g_szDirectory + file;

			if(!LoadSkinnedX(szFile))
			{
				MessageBoxA(NULL, file, "Failed to load skinned model!", MB_OK);
				Release();
				return false;
			}
		}
		else if(file[strlen(file)-1] == 'x' || file[strlen(file)-1] == 'X')
		{
			Log::Print("Loading X File");
			
			// Get the full file path
			String szFile;
			if( (file[0]=='c'||file[0]=='C') && file[1]==':' )
				szFile = file;
			else
				szFile = g_szDirectory + file;

			if(!LoadX(szFile))
			{
				MessageBoxA(NULL, file, "Failed to load model!", MB_OK);
				Release();
				return false;
			}
		}	
		else if(strcmp(file, "SPHERE_OBJECT")==0)
		{
			// Create a sphere
			LPD3DXMESH pMesh = NULL;
			D3DXCreateSphere(g_pd3dDevice9, 1.0f, 8, 8, &pMesh, NULL);
			FromXMesh(pMesh);
			pMesh->Release();
		}
		else if(strcmp(file, "BOX_OBJECT")==0)
		{
			// Create a sphere
			LPD3DXMESH pMesh = NULL;
			D3DXCreateBox(g_pd3dDevice9, 1.0f, 1.0f, 1.0f, &pMesh, NULL);
			FromXMesh(pMesh);
			pMesh->Release();
		}
		else if(strcmp(file, "CONE_OBJECT")==0)
		{
			// Create a sphere
			LPD3DXMESH pMesh = NULL;
			D3DXCreateCylinder(g_pd3dDevice9, 0.6f, 1.0f, 0.7f, 8, 8, &pMesh, NULL);
			FromXMesh(pMesh);
			pMesh->Release();
		}
		else
			return false;

		// Finalize the vertex arrays
		m_pVerts.Finalize();
		m_pIndices.Finalize();
		
		// Center the mesh
		CenterMesh();
		
		// Fill the mesh buffers
		if(FAILED(FillBuffers()))
		{
			return false;
		}

		// Store the filename
		m_szName = file;

		// Log
		Log::Print("Model Loaded - \"%s\"",m_szName.c_str());
		return (m_bLoaded=true);
		
	}

	//--------------------------------------------------------------------------------------
	// Creates a Core Engine mesh from a DirectX mesh
	//--------------------------------------------------------------------------------------
	bool BaseMesh::FromXMesh(LPD3DXMESH pMesh)
	{
		if(!pMesh)
			return false;
		
		HRESULT hr;
		
		// Get the D3D9 version of the vertex format
		const D3DVERTEXELEMENT9 vertexDecl[4] = 
		{
			{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
			{0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},		
			D3DDECL_END()
		};
		const D3DVERTEXELEMENT9 skinVertexDecl[6] = 
		{
			{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
			{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
			{0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},		
			{0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},		
			{0, 48, D3DDECLTYPE_UBYTE4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},		
			D3DDECL_END()
		};
		
		// Now clone the mesh into a mesh with the CoreEngine format
		LPD3DXMESH pCoreMesh;
		if(m_bSkinned)
		{
			B_RETURN( pMesh->CloneMesh(D3DXMESH_SYSTEMMEM|D3DXMESH_32BIT,skinVertexDecl,g_pd3dDevice9,&pCoreMesh) );
		}
		else
		{
			B_RETURN( pMesh->CloneMesh(D3DXMESH_SYSTEMMEM|D3DXMESH_32BIT,vertexDecl,g_pd3dDevice9,&pCoreMesh) );
		}

		// Generate adjacency
		DWORD* pAdjacency = new DWORD[pCoreMesh->GetNumFaces()*3];
		B_RETURN(pCoreMesh->GenerateAdjacency(1e-6f,pAdjacency));

		// Compute the mesh normals
		/*LPD3DXMESH pTempMesh = 0;
		B_RETURN( D3DXComputeTangentFrameEx(pCoreMesh, D3DDECLUSAGE_TEXCOORD,0, 
												 D3DX_DEFAULT,0,
												 D3DX_DEFAULT,0,
												 D3DDECLUSAGE_NORMAL,0,
												 D3DXTANGENT_WEIGHT_EQUAL | D3DXTANGENT_CALCULATE_NORMALS,
												 pAdjacency, 0.01f, 0.25f, 0.01f, &pTempMesh, NULL) );
		SAFE_RELEASE(pCoreMesh);
		pCoreMesh = pTempMesh;
		pTempMesh = NULL;*/

		
		// Optimize the mesh
		B_RETURN( pCoreMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE|D3DXMESHOPT_ATTRSORT,
			pAdjacency,NULL,NULL,NULL) );

		// Free the objects that are not needed
		SAFE_DELETE_ARRAY(pAdjacency);

		// Finally copy this mesh over into the Core Engine mesh format
		{
			// Copy the vertices
			if(m_bSkinned)
			{
				SkinnedVertex* pVerts = NULL;
				pCoreMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVerts);
				m_pSkinnedVerts.FromArray(pVerts, pCoreMesh->GetNumVertices());
				pCoreMesh->UnlockVertexBuffer();
			}
			else
			{
				Vertex* pVerts = NULL;
				pCoreMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVerts);
				m_pVerts.FromArray(pVerts, pCoreMesh->GetNumVertices());
				pCoreMesh->UnlockVertexBuffer();
			}

			// Copy the indices
			DWORD* pIndices = NULL;
			pCoreMesh->LockIndexBuffer(D3DLOCK_READONLY,(void**)&pIndices);
			m_pIndices.FromArray(pIndices,pCoreMesh->GetNumFaces()*3);
			pCoreMesh->UnlockIndexBuffer();

			// Get the submesh data
			DWORD iNumMesh;
			pCoreMesh->GetAttributeTable(NULL,&iNumMesh);
			D3DXATTRIBUTERANGE* pAttr = new D3DXATTRIBUTERANGE[iNumMesh];
			pCoreMesh->GetAttributeTable(pAttr,0);
			
			// Make sure there is a material list
			if(m_pMaterials.Size()==0)
			{
				m_pMaterials.Allocate(1);
				m_pMaterials[0] = g_Materials.GetDefault();
			}
			
			// Create the submeshes
			m_pSubMesh.Allocate(iNumMesh);
			for(DWORD i=0; i<iNumMesh; i++)
			{
				m_pSubMesh[i].pMaterial = m_pMaterials[pAttr[i].AttribId];
				m_pSubMesh[i].startIndex = pAttr[i].FaceStart*3;
				m_pSubMesh[i].numIndices = pAttr[i].FaceCount*3;
				m_pSubMesh[i].name = String("SubMesh")+((int)i+1);
				m_pSubMesh[i].isSkinned = m_bSkinned;
			}

			// Free mem
			SAFE_DELETE_ARRAY(pAttr);
		}
		
		SAFE_RELEASE(pCoreMesh);

		return true;
	}



	//--------------------------------------------------------------------------------------
	// Loads a DirectX BaseMesh (.X)
	//  Since Direct3D10 does not support .x loading, a nullref d3d9device is used
	//--------------------------------------------------------------------------------------
	bool BaseMesh::LoadX(const char* file)
	{
		// TODO:
		//		Use effects from the mesh
		
		HRESULT hr;
		
		// Load the mesh into a D3DX BaseMesh object
		LPD3DXMESH pMesh = NULL;
		LPD3DXBUFFER pMaterialBuffer = NULL;
		DWORD dwNumMaterials=0;
		hr = D3DXLoadMeshFromXA( file,  D3DXMESH_SYSTEMMEM,  g_pd3dDevice9, NULL, 
			&pMaterialBuffer, NULL, &dwNumMaterials, &pMesh );
		if(hr!=D3D_OK)
		{
			Log::Print("<!> BaseMesh::D3DXLoadMeshFromX(%s) failed!",file);
			SAFE_RELEASE(pMaterialBuffer);
			SAFE_RELEASE(pMesh);
			return false;
		}

		// Create the materials
		D3DXMATERIAL* pMaterials = (D3DXMATERIAL*)pMaterialBuffer->GetBufferPointer();
		int iNumMaterials = dwNumMaterials;
		m_pMaterials.Allocate(iNumMaterials);
		String szDir = String(file).GetDirectory();
		for(int i=0; i<iNumMaterials; i++)
		{ 
			if(bLoadMaterials)
			{
				// Create and setup a new material from the mesh info
				Material* pMat = g_Materials.Add( String("XMat") + i );
				pMat->SetDiffuse( D3DXVECTOR4( pMaterials[i].MatD3D.Diffuse.r,
								  pMaterials[i].MatD3D.Diffuse.g,
								  pMaterials[i].MatD3D.Diffuse.b,
								  pMaterials[i].MatD3D.Diffuse.a  ));
				pMat->SetSurfaceParam1(pMaterials[i].MatD3D.Power);
				if(pMat->GetSurfaceParam1()<2)
					pMat->SetSurfaceParam1(4);
				if(pMat->GetSurfaceParam1()>50)
					pMat->SetSurfaceParam1(50);
				
				// Try loading the texture
				if(pMaterials[i].pTextureFilename)
				{
					if(!pMat->LoadTexture(szDir + pMaterials[i].pTextureFilename, TEX_DIFFUSE1))
					if(!pMat->LoadTexture(szDir.GetPreviousDirectory() + pMaterials[i].pTextureFilename, TEX_DIFFUSE1))
						pMat->LoadTexture(String("..\\") + pMaterials[i].pTextureFilename, TEX_DIFFUSE1);
				}
				
				m_pMaterials[i] = pMat;
			}
			else
				m_pMaterials[i] = g_Materials.GetDefault();
		}

		// Release mem
		SAFE_RELEASE(pMaterialBuffer);
			
		
		// Now convert to Core Engine Format
		if(!FromXMesh(pMesh))
		{
			Log::Print("<!>BaseMesh Conversion Failed!!");
			SAFE_RELEASE(pMesh);
			return false;
		}
		SAFE_RELEASE(pMesh);
		return true;
	}


	//--------------------------------------------------------------------------------------
	// Finds the node that contains mesh data
	//--------------------------------------------------------------------------------------
	D3DXFRAME* BaseMesh::FindNodeWithMesh(D3DXFRAME* frame)
	{
		// In this demo we stipulate that the input .X file contains only one
		// mesh.  So search for that one and only mesh.

		if( frame->pMeshContainer )
			if( frame->pMeshContainer->MeshData.pMesh != 0 )
				return frame;

		D3DXFRAME* f = 0;
		if(frame->pFrameSibling)
			if( f = FindNodeWithMesh(frame->pFrameSibling) )	
				return f;

		if(frame->pFrameFirstChild)
			if( f = FindNodeWithMesh(frame->pFrameFirstChild) )
				return f;

		return 0;
	}

	//--------------------------------------------------------------------------------------
	////--------------------------------------------------------------------------------------
	bool BaseMesh::LoadSkinnedX(const char* file)
	{
		D3DXFRAME*					m_pRootFrame;			// Bone hierarchy
		bool					    m_bSkinnedMesh;			// True if skinning
		DWORD						m_MaxVertInfluences;	// Max number of bones per vertex
		ID3DXAnimationController*	m_pAnimCtrl;			// Animation controller
		Array<D3DXMATRIX>			m_FinalTransforms;		// Matrix transforms
		float						m_FrameTime;			// Timer for animation
		D3DXMESHCONTAINER_DERIVED*	m_pMeshContainer;		// The mesh contatiner
		
		// Load in the mesh from the file
		AllocateHierarchy allocMeshHierarchy;
		HRESULT hr = D3DXLoadMeshHierarchyFromXA(   file, 
			D3DXMESH_SYSTEMMEM,
			g_pd3dDevice9, 
			&allocMeshHierarchy, 
			0,
			&m_pRootFrame,	
			&m_pAnimCtrl );
		if(FAILED(hr))
		{
			MessageBoxA(NULL, "Couldn't load animation data", "Load Fail", MB_OK);
			return false;
		}

		// Find the mesh node	
		D3DXFRAME* f = FindNodeWithMesh(m_pRootFrame);
		m_pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)f->pMeshContainer;
		m_pMeshContainer->pSkinInfo->GetMaxVertexInfluences(&m_MaxVertInfluences);

		// Build the vertex/index buffers
		m_bSkinned = true;
		if(FromXMesh(m_pMeshContainer->MeshData.pMesh))
		{
			for(int i=0; i<m_pSkinnedVerts.Size(); i++)
				Log::Print("%d %d %d %d,,,,%f %f %f %f", m_pSkinnedVerts[i].indices[0],
														 m_pSkinnedVerts[i].indices[1],
														 m_pSkinnedVerts[i].indices[2],
														 m_pSkinnedVerts[i].indices[3],
														 m_pSkinnedVerts[i].weights.x,
														 m_pSkinnedVerts[i].weights.y,
														 m_pSkinnedVerts[i].weights.z,
														 m_pSkinnedVerts[i].weights.w );
			return true;
		}	
		return false;
	}


	//--------------------------------------------------------------------------------------
	// Cleans the mesh by removing duplicate verts
	//--------------------------------------------------------------------------------------
	void BaseMesh::Clean()
	{
		bool flag=true;
		while(flag)
		{
			flag=false;
			for(int i=0; i<m_pVerts.Size(); i++)
			{
				for(int j=0; j<m_pVerts.Size(); j++)
				{
					if(i==j) continue;
					if( (fabs(m_pVerts[i].pos.x-m_pVerts[j].pos.x)<0.001f) &&
						(fabs(m_pVerts[i].pos.y-m_pVerts[j].pos.y)<0.001f) &&
						(fabs(m_pVerts[i].pos.z-m_pVerts[j].pos.z)<0.001f) )
					{
						m_pVerts.Remove(j);
						for(int k=0; k<m_pIndices.Size(); k++)
						{
							if(m_pIndices[k]==(UINT)j)
								m_pIndices[k]=(UINT)i;
							else
								if(m_pIndices[k]>(UINT)j)
									m_pIndices[k]--;
						}
						flag=true;
						break;
					}
				}
				if(flag==true)
					break;
			}
		}
	}


	//--------------------------------------------------------------------------------------
	// Swap tex coords
	//--------------------------------------------------------------------------------------
	void BaseMesh::FlipUV()
	{
		for(int i=0; i<m_pVerts.Size(); i++)
		{
			float t=m_pVerts[i].tu;
			m_pVerts[i].tu=m_pVerts[i].tv;
			m_pVerts[i].tv=t;
		}
		FillBuffers();
	}

}