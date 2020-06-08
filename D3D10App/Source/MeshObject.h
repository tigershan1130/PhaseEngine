//--------------------------------------------------------------------------------------
// File: MeshObject.h
//
// An instance of a 3D mesh
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "Array.cpp"
#include "Mesh.h"
#include "RenderSurface.h"
#include "MessageHandler.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Static 3D BaseMesh
	//--------------------------------------------------------------------------------------
	class MeshObject
	{
	public:
		MeshObject();
		~MeshObject();
			
			// Frees mem and makes an empty mesh	
			void Release();

			// Loads the mesh from a file
			bool Load(const char* file);

			// Get the parent mesh
			inline BaseMesh* GetMesh(){ return m_pMesh; }

			// Name
			inline String GetName(){ return m_szName; }
			inline void SetName(char* name){ m_szName = name; }
			
			// Updates the world matrix
			inline void UpdateWorldMatrix(bool bounds=false)
			{ 
				UpdateMesh();
				m_matWorld = m_matS * m_matR * m_matT;
				if(bounds)
					ComputeBounds();
			}

			// Position
			inline void SetPos(D3DXVECTOR3& v)
			{ 
				m_vPos=v; 
				D3DXMatrixTranslation(&m_matT,m_vPos.x,m_vPos.y,m_vPos.z); 
				UpdateWorldMatrix();
			}
			inline void AddToPos(D3DXVECTOR3& v)
			{ 
				m_vPos+=v; 
				D3DXMatrixTranslation(&m_matT,m_vPos.x,m_vPos.y,m_vPos.z); 
				UpdateWorldMatrix(); 
			}
			inline void SetPos(float x, float y, float z)
			{ 
				m_vPos.x=x; m_vPos.y=y; m_vPos.z=z;
				D3DXMatrixTranslation(&m_matT,x,y,z); 
				UpdateWorldMatrix();
			}
			inline void AddToPos(float x, float y, float z)
			{ 
				m_vPos.x+=x; m_vPos.y+=y; m_vPos.z+=z;
				D3DXMatrixTranslation(&m_matT,x,y,z); 
				UpdateWorldMatrix();
			}
			inline D3DXVECTOR3 GetPos(){ return m_vPos; }

			// Rotation
			inline void SetRot(D3DXVECTOR3& v)
			{ 
				m_vRot=v; 
				D3DXMatrixRotationYawPitchRoll(&m_matR,m_vRot.y,m_vRot.x,m_vRot.z); 
				UpdateWorldMatrix(true);
			}
			inline void AddToRot(D3DXVECTOR3& v)
			{ 
				m_vRot+=v; 
				D3DXMatrixRotationYawPitchRoll(&m_matR,m_vRot.y,m_vRot.x,m_vRot.z); 
				UpdateWorldMatrix(true);
			}
			inline void SetRot(float x, float y, float z)
			{ 
				m_vRot.x=x; m_vRot.y=y; m_vRot.z=z;
				D3DXMatrixRotationYawPitchRoll(&m_matR,m_vRot.y,m_vRot.x,m_vRot.z); 
				UpdateWorldMatrix(true);
			}
			inline void AddToRot(float x, float y, float z)
			{ 
				m_vRot.x+=x; m_vRot.y+=y; m_vRot.z+=z;
				D3DXMatrixRotationYawPitchRoll(&m_matR,m_vRot.y,m_vRot.x,m_vRot.z); 
				UpdateWorldMatrix(true);
			}
			inline D3DXVECTOR3 GetRot(){ return m_vRot; }
			

			// Scale
			inline void SetScale(D3DXVECTOR3& v)
			{ 
				m_vScale=v; 
				D3DXMatrixScaling(&m_matS,m_vScale.x,m_vScale.y,m_vScale.z); 
				UpdateWorldMatrix(true); 
			}
			inline void AddToScale(D3DXVECTOR3& v)
			{ 
				m_vScale+=v; 
				D3DXMatrixScaling(&m_matS,m_vScale.x,m_vScale.y,m_vScale.z); 
				UpdateWorldMatrix(true); 
			}
			inline void SetScale(float x, float y, float z)
			{ 
				m_vScale.x=x; m_vScale.y=y; m_vScale.z=z;
				D3DXMatrixScaling(&m_matS,x,y,z); 
				UpdateWorldMatrix(true);
			}
			inline void AddToScale(float x, float y, float z)
			{ 
				m_vScale.x+=x; m_vScale.y+=y; m_vScale.z+=z;
				D3DXMatrixScaling(&m_matS,x,y,z); 
				UpdateWorldMatrix(true);
			}
			inline D3DXVECTOR3 GetScale(){ return m_vScale; }

			inline void SetTransform(D3DXVECTOR3& pos, D3DXMATRIX& mat)
			{
				m_vPos = pos;
				D3DXMATRIX newWorld = m_matS * mat;
				if( m_matWorld!=newWorld )
					UpdateMesh();
				m_matWorld = newWorld;
			}

			
			// Sends an update message to the engine
			bool			isUpdating;
			inline void UpdateMesh(){ 
				if(isUpdating)
					return;
				isUpdating=true;
				MessageHandler::SendMessage(MSG_MESH_UPDATE, this);
			}

			// Gets the world matrix
			inline D3DXMATRIX& GetWorldMatrix(){ return m_matWorld; }
			inline D3DXMATRIX& GetOldWorldMatrix(){ return m_matWorldPrev; }
			inline void SetOldWorldMatrix(D3DXMATRIX& mat){ m_matWorldPrev=mat; }

			// Checks for an intersection with a an AABB
			bool BoxMeshIntersect(D3DXVECTOR3& min, D3DXVECTOR3& max);


			// Materials
			inline int GetNumMaterials(){ return m_pMaterial.Size(); }
			inline Material* GetMaterial(int i){ if(m_pMaterial&&i>=0&&i<=m_pMaterial.Size()) return m_pMaterial[i]; else return 0; }
			inline void SetMaterial(Material* pMat, int i){ m_pSubMesh[i].pMaterial = m_pMaterial[i] = pMat; }

			// Get the center
			inline float GetRadius(){ return m_Radius; }
			inline const D3DXVECTOR3& GetBoundSize() const { return m_BoundSize; }

			// Get a mesh
			inline Array<SubMesh>& GetSubMesh(){ return m_pSubMesh; }
			inline SubMesh* GetSubMesh(int i){ return &m_pSubMesh[i]; }
			
			// Get the number of meshes
			inline int GetNumSubMesh(){ return m_pSubMesh.Size(); }

			// Compute the mesh bounds
			void ComputeBounds();

			// Gets the number of faces
			inline int GetNumFaces(){ return m_pMesh->GetNumFaces(); }

			// Update the submeshes
			void UpdateSubMeshes();

			// Update the animation cycle and matrices
			void UpdateAnimation();

			

			// Skinned?
			inline bool IsSkinned(){ return m_bSkinnedMesh; }

			// Animation matrices
			inline UINT GetNumAnimMatrices(){ return m_FinalTransforms.Size(); }
			inline D3DXMATRIX* GetAnimMatrices(){ return (D3DXMATRIX*)m_FinalTransforms; }

			// The number of bones per vertex
			inline DWORD GetNumBonesPerVertex(){ return m_MaxVertInfluences; }

			// Number of animations
			inline int GetNumAnimations(){ if(m_pAnimCtrl) return m_pAnimCtrl->GetNumAnimationSets(); return 0; }

			// Animation names
			inline char* GetAnimationName(int index){
				ID3DXAnimationSet* pAnimSet;
				m_pAnimCtrl->GetAnimationSet(index, &pAnimSet);
				return (char*)pAnimSet->GetName();
			}
			
			// Enable an animation track
			inline void AnimationPlay(int index){
				ID3DXAnimationSet* pAnimSet;
				m_pAnimCtrl->GetAnimationSet(index, &pAnimSet);
				m_pAnimCtrl->SetTrackAnimationSet(0, pAnimSet);
				m_pAnimCtrl->SetTrackEnable(0, TRUE);
			}

			// Disable all animation tracks
			inline void AnimationPause(){ 
				for(UINT i=0; i<m_pAnimCtrl->GetMaxNumTracks(); i++)
				{
					m_pAnimCtrl->SetTrackEnable(i, FALSE);
					m_pAnimCtrl->SetTrackPosition(i, 0);
				}
			}

			// Is it using cube mapping
			inline bool IsCubeMapped(){ return m_bCubeMap; }
			inline void EnableCubeMapping(bool b){ m_bCubeMap=b; }

	 protected:
			BaseMesh*				m_pMesh;			  // The model
			Array<Material*>		m_pMaterial;		  // The materials
			Array<SubMesh>			m_pSubMesh;			  // The sub meshes
			D3DXVECTOR3				m_vPos;				  // The position
			D3DXVECTOR3				m_vRot;				  // The rotation angles
			D3DXVECTOR3				m_vScale;			  // The object scale
			D3DXMATRIX				m_matT,m_matR,m_matS; // Matrices
			D3DXMATRIX				m_matWorld;			  // The world matrix
			D3DXMATRIX				m_matWorldPrev;		  // The world matrix from the previous frame
			D3DXVECTOR3				m_BoundSize;		  // AABB Size
			float					m_Radius;			  // Bound radius
			String					m_szName;			  // BaseMesh name
			bool					m_bCubeMap;		      // True for cube mapping
			bool					m_bHide;			  // Will not render if true

			// Animation
			D3DXFRAME*					m_pRootFrame;			// Bone hierarchy
			bool					    m_bSkinnedMesh;			// True if skinning
			DWORD						m_MaxVertInfluences;	// Max number of bones per vertex
			ID3DXAnimationController*	m_pAnimCtrl;			// Animation controller
			Array<D3DXMATRIX>			m_FinalTransforms;		// Matrix transforms
			float						m_FrameTime;			// Timer for animation
			D3DXMESHCONTAINER_DERIVED*	m_pMeshContainer;		// The mesh contatiner

			// Get the animation data
			void CloneAnimationData(char* file);

			// Finds the node that contains mesh data
			D3DXFRAME* FindNodeWithMesh(D3DXFRAME* frame);

			// Called to setup the pointers for a given bone to its transformation matrix
			HRESULT SetupBoneMatrixPointers( LPD3DXFRAME pFrame );

			// Called to setup the pointers for a given bone to its transformation matrix
			HRESULT SetupBoneMatrixPointersOnMesh( LPD3DXMESHCONTAINER pMeshContainerBase );

			// Update the frame matrices
			void UpdateFrameMatrices( LPD3DXFRAME pFrameBase, LPD3DXMATRIX pParentMatrix );
	};

	//--------------------------------------------------------------------------------------
	// Checks for a ray-triangle intersection with a model,
	//    and gets the distance to the triangle as well as 
	//    the barycentric hit coordinates
	//--------------------------------------------------------------------------------------	
	bool RayIntersectMesh( MeshObject* pMesh, D3DXVECTOR3 &orig, D3DXVECTOR3 &dir,
						  float *t, float *u, float *v, int *mesh, float maxD = 1000.0f);
}