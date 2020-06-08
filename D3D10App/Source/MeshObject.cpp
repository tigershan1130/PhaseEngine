//--------------------------------------------------------------------------------------
// File: MeshObject.cpp
//
// A static 3D mesh.
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "MeshObject.h"
#include "Log.h"
#include "AllocateHierarchy.h"
#include "Mesh.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	MeshObject::MeshObject()
	{
		m_pMesh = NULL;
		m_vRot = D3DXVECTOR3(0.0f,0.0f,0.0f);
		m_vPos = D3DXVECTOR3(0.0f,0.0f,0.0f);
		m_vScale = D3DXVECTOR3(1,1,1);
		m_pRootFrame = NULL;
		m_pAnimCtrl = NULL;
		m_FrameTime = 0;
		m_bSkinnedMesh = false;
		m_bCubeMap = false;
		m_bHide = false;
		isUpdating=false;
	}

	//--------------------------------------------------------------------------------------
	// Destructor
	//--------------------------------------------------------------------------------------
	MeshObject::~MeshObject()
	{ 

	}

	//--------------------------------------------------------------------------------------
	// Releases the object
	//--------------------------------------------------------------------------------------
	void MeshObject::Release()
	{
		if(m_pMesh)
		{
			// Release the materials
			m_pMaterial.Release();

			// Release the submeshes
			m_pSubMesh.Release();

			// Skinned mesh data
			if( m_pRootFrame )
			{
				AllocateHierarchy allocMeshHierarchy;
				D3DXFrameDestroy(m_pRootFrame, &allocMeshHierarchy);
				m_pRootFrame = NULL;
			}
			SAFE_RELEASE(m_pAnimCtrl);
			m_FinalTransforms.Release();
			
			// Deref the model pointer
			g_Meshes.Deref(m_pMesh);

			// Reset the mesh
			MeshObject();
		}
	}


	//--------------------------------------------------------------------------------------
	// Loads an object model
	//--------------------------------------------------------------------------------------
	bool MeshObject::Load(const char* file)
	{
		// Reset the object
		if(m_pMesh)
			Release();


		// Load the model
		m_pMesh = g_Meshes.Load(file);

		// Verify that the load was a success
		if(!m_pMesh)
		{
			Log::Print("<!>Invalid Model file!! (%s)",file);
			return false;
		}

		// Position the model
		m_vPos = D3DXVECTOR3(0, 0, 0);

		// Get the animation data
		if(m_pMesh->m_bSkinned)
			CloneAnimationData(const_cast<char*>(file));

		// Setup the initial world matrix
		D3DXMatrixTranslation(&m_matT,m_vPos.x,m_vPos.y,m_vPos.z);
		D3DXMatrixRotationYawPitchRoll(&m_matR,m_vRot.y,m_vRot.x,m_vRot.z);
		D3DXMatrixScaling(&m_matS, m_vScale.x,m_vScale.y,m_vScale.z);
		m_matWorld = m_matS * m_matR * m_matT;
		m_pMaterial.Allocate( m_pMesh->GetNumSubMesh() );
		m_pSubMesh.Allocate( m_pMesh->GetNumSubMesh() );
		m_FrameTime = 0;
		for(int i=0; i<m_pMesh->GetNumSubMesh(); i++)
		{
			// Add the default material
			m_pMaterial[i] = m_pMesh->GetDefaultMaterials()[i];
					
			// Set the submesh pointers
			m_pSubMesh[i] = *m_pMesh->GetSubMesh( i );
			m_pSubMesh[i].pWorldMatrix = &m_matWorld;
			m_pSubMesh[i].pWorldMatrixPrev = &m_matWorldPrev;
			m_pSubMesh[i].pWorldPosition = &m_vPos;
			m_pSubMesh[i].pMaterial = m_pMaterial[i];
			m_pSubMesh[i].isSkinned = IsSkinned();
			m_pSubMesh[i].pAnimMatrices = &m_FinalTransforms;
		}
		m_szName = GetFileFromPath(RemoveFileExtension(String(m_pMesh->GetName())));


		// Compute the mesh bounds
		ComputeBounds();

		return true;
	}


	//--------------------------------------------------------------------------------------
	// Get the animation data
	//--------------------------------------------------------------------------------------
	void MeshObject::CloneAnimationData(char* file)
	{
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
			return;
		}


		// Now setup the bones
		SetupBoneMatrixPointers( m_pRootFrame );

		// Find the mesh node	
		D3DXFRAME* f = FindNodeWithMesh(m_pRootFrame);
		m_pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)f->pMeshContainer;
		m_pMeshContainer->pSkinInfo->GetMaxVertexInfluences(&m_MaxVertInfluences);

		// Setup the matrices
		m_FinalTransforms.Allocate(m_pMeshContainer->pSkinInfo->GetNumBones());
		SAFE_RELEASE(m_pMeshContainer->pOrigMesh);
		SAFE_RELEASE(m_pMeshContainer->MeshData.pMesh);

		m_bSkinnedMesh = true;
		AnimationPause();
	}


	//--------------------------------------------------------------------------------------
	// Update the frame matrices
	//--------------------------------------------------------------------------------------
	void MeshObject::UpdateFrameMatrices( LPD3DXFRAME pFrameBase, LPD3DXMATRIX pParentMatrix )
	{
		D3DXFRAME_DERIVED *pFrame = (D3DXFRAME_DERIVED*)pFrameBase;

		if (pParentMatrix != NULL)
			D3DXMatrixMultiply(&pFrame->CombinedTransformationMatrix, &pFrame->TransformationMatrix, pParentMatrix);
		else
			pFrame->CombinedTransformationMatrix = pFrame->TransformationMatrix;

		if (pFrame->pFrameSibling != NULL)
		{
			UpdateFrameMatrices(pFrame->pFrameSibling, pParentMatrix);
		}

		if (pFrame->pFrameFirstChild != NULL)
		{
			UpdateFrameMatrices(pFrame->pFrameFirstChild, &pFrame->CombinedTransformationMatrix);
		}
	}


	//--------------------------------------------------------------------------------------
	// Finds the node that contains mesh data
	//--------------------------------------------------------------------------------------
	D3DXFRAME* MeshObject::FindNodeWithMesh(D3DXFRAME* frame)
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
	// Called to setup the pointers for a given bone to its transformation matrix
	//--------------------------------------------------------------------------------------
	HRESULT MeshObject::SetupBoneMatrixPointersOnMesh( LPD3DXMESHCONTAINER pMeshContainerBase )
	{
		UINT iBone, cBones;
		D3DXFRAME_DERIVED *pFrame;

		D3DXMESHCONTAINER_DERIVED *pMeshContainer = (D3DXMESHCONTAINER_DERIVED*)pMeshContainerBase;

		// if there is a skinmesh, then setup the bone matrices
		if (pMeshContainer->pSkinInfo != NULL)
		{
			cBones = pMeshContainer->pSkinInfo->GetNumBones();

			pMeshContainer->ppBoneMatrixPtrs = new D3DXMATRIX*[cBones];
			if (pMeshContainer->ppBoneMatrixPtrs == NULL)
				return E_OUTOFMEMORY;

			for (iBone = 0; iBone < cBones; iBone++)
			{
				pFrame = (D3DXFRAME_DERIVED*)D3DXFrameFind( m_pRootFrame, pMeshContainer->pSkinInfo->GetBoneName(iBone) );
				if (pFrame == NULL)
					return E_FAIL;

				pMeshContainer->ppBoneMatrixPtrs[iBone] = &pFrame->CombinedTransformationMatrix;
			}
		}

		return S_OK;
	}


	//--------------------------------------------------------------------------------------
	// Called to setup the pointers for a given bone to its transformation matrix
	//--------------------------------------------------------------------------------------
	HRESULT MeshObject::SetupBoneMatrixPointers( LPD3DXFRAME pFrame )
	{
		HRESULT hr;

		if (pFrame->pMeshContainer != NULL)
		{
			hr = SetupBoneMatrixPointersOnMesh(pFrame->pMeshContainer);
			if (FAILED(hr))
				return hr;
		}

		if (pFrame->pFrameSibling != NULL)
		{
			hr = SetupBoneMatrixPointers(pFrame->pFrameSibling);
			if (FAILED(hr))
				return hr;
		}

		if (pFrame->pFrameFirstChild != NULL)
		{
			hr = SetupBoneMatrixPointers(pFrame->pFrameFirstChild);
			if (FAILED(hr))
				return hr;
		}

		return S_OK;
	}



	//--------------------------------------------------------------------------------------
	// Update the animation cycle and matrices
	//--------------------------------------------------------------------------------------
	void MeshObject::UpdateAnimation()
	{
		// Update the animation
		float time = (timeGetTime()*0.001f);
		m_pAnimCtrl->AdvanceTime(time - m_FrameTime, NULL);
		m_FrameTime = time;

		// Now that the frames are updated to the current pose, recurse
		// down the tree and generate a frame's combined-transform.
		D3DXMATRIX identity;
		D3DXMatrixIdentity(&identity);
		UpdateFrameMatrices(m_pRootFrame, &identity);

		// Add the offset-transform
		for(int i = 0; i<m_FinalTransforms.Size(); i++)
			D3DXMatrixMultiply(&m_FinalTransforms[i], &m_pMeshContainer->pBoneOffsetMatrices[i], m_pMeshContainer->ppBoneMatrixPtrs[i]);
	}


	//--------------------------------------------------------------------------------------
	// Update the submeshes
	//--------------------------------------------------------------------------------------
	void MeshObject::UpdateSubMeshes()
	{
		for(int i=0; i<m_pSubMesh.Size(); i++)
		{
			m_pSubMesh[i].pIndexBuffer = m_pMesh->GetSubMesh(i)->pIndexBuffer;
			m_pSubMesh[i].pVertexBuffer = m_pMesh->GetSubMesh(i)->pVertexBuffer;
			m_pSubMesh[i].pPosVertexBuffer = m_pMesh->GetSubMesh(i)->pPosVertexBuffer;
		}
	}


	//--------------------------------------------------------------------------------------
	// Computes the mesh center and bounding radius
	//--------------------------------------------------------------------------------------
	void MeshObject::ComputeBounds()
	{
		// Get the matrix
		D3DXMATRIX mWorld = m_matS * m_matR;
		D3DXMATRIX* pWorld = &mWorld;
		
		// Calculate the center to offset the verts
		D3DXVECTOR3 vMax(-99999.0f,-99999.0f,-99999.0f),vCenter,vMin(99999.0f,99999.0f,99999.0f),vTemp;
		for(int i=0; i<m_pMesh->GetNumVerts(); i++)
		{
			if(IsSkinned())
				D3DXVec3TransformCoord(&vTemp, &m_pMesh->GetSkinnedVerts()[i].pos, pWorld );
			else
				D3DXVec3TransformCoord(&vTemp, &m_pMesh->GetVerts()[i].pos, pWorld );
			vMin = Math::MinVector( vMin, vTemp );
			vMax = Math::MaxVector( vMax, vTemp );
		}
		vCenter = D3DXVECTOR3( vMax.x - fabs(vMin.x),
						  vMax.y - fabs(vMin.y),
						  vMax.z - fabs(vMin.z) ) / 2.0f;
		
		vMax = D3DXVECTOR3(-99999.0f,-99999.0f,-99999.0f);
		for(int i=0; i<m_pMesh->GetNumVerts(); i++)
		{
			if(IsSkinned())
				D3DXVec3TransformCoord(&vTemp, &m_pMesh->GetSkinnedVerts()[i].pos, pWorld );
			else
				D3DXVec3TransformCoord(&vTemp, &m_pMesh->GetVerts()[i].pos, pWorld );
			vMax = Math::MaxVector( vMax, Math::Absolute(vTemp-vCenter) );
		}
		m_Radius = D3DXVec3Length(&vMax);
		m_BoundSize = vMax;

		// Calculate the bounding spheres for each submesh
		for(int i=0; i<m_pSubMesh.Size(); i++)
		{
			vMax = D3DXVECTOR3(-99999.0f,-99999.0f,-99999.0f);
			vMin = D3DXVECTOR3(99999.0f,99999.0f,99999.0f);
			for(int e=m_pSubMesh[i].startIndex; e<m_pSubMesh[i].numIndices+m_pSubMesh[i].startIndex; e++)
			{
				if(IsSkinned())
					D3DXVec3TransformCoord(&vTemp, &m_pMesh->GetSkinnedVerts()[m_pMesh->GetIndices()[e]].pos, pWorld );
				else
					D3DXVec3TransformCoord(&vTemp, &m_pMesh->GetVerts()[m_pMesh->GetIndices()[e]].pos, pWorld );
				vMin = Math::MinVector( vMin, vTemp );
				vMax = Math::MaxVector( vMax, vTemp );
			}
			vCenter = D3DXVECTOR3( vMax.x - fabs(vMin.x),
							  vMax.y - fabs(vMin.y),
							  vMax.z - fabs(vMin.z) ) / 2.0f;
			m_pSubMesh[i].localPosition = vCenter;
			
			vMax = D3DXVECTOR3(-99999.0f,-99999.0f,-99999.0f);
			for(int e=m_pSubMesh[i].startIndex; e<m_pSubMesh[i].numIndices+m_pSubMesh[i].startIndex; e++)
			{
				if(IsSkinned())
					D3DXVec3TransformCoord(&vTemp, &m_pMesh->GetSkinnedVerts()[m_pMesh->GetIndices()[e]].pos, pWorld );
				else
					D3DXVec3TransformCoord(&vTemp, &m_pMesh->GetVerts()[m_pMesh->GetIndices()[e]].pos, pWorld );
				vMax = Math::MaxVector( vMax, Math::Absolute(vTemp-vCenter) );
			}
			
			m_pSubMesh[i].boundRadius = D3DXVec3Length(&vMax);
			m_pSubMesh[i].boundSize = vMax;
		}
	}

	

	//--------------------------------------------------------------------------------------
	// Checks for an intersection with a an AABB
	//--------------------------------------------------------------------------------------
	bool MeshObject::BoxMeshIntersect(D3DXVECTOR3& min, D3DXVECTOR3& max)
	{
		// There is an intersection if any of the meshes
		// bounding box points are contained in the given box
		return Math::PointInBox(m_vPos+D3DXVECTOR3( m_BoundSize.x,  m_BoundSize.y,  m_BoundSize.z), min, max) ||
			   Math::PointInBox(m_vPos+D3DXVECTOR3( -m_BoundSize.x,  m_BoundSize.y, -m_BoundSize.z), min, max) ||
			   Math::PointInBox(m_vPos+D3DXVECTOR3( -m_BoundSize.x,  m_BoundSize.y,  m_BoundSize.z), min, max) ||
			   Math::PointInBox(m_vPos+D3DXVECTOR3( m_BoundSize.x,  m_BoundSize.y, -m_BoundSize.z), min, max) ||
			   Math::PointInBox(m_vPos+D3DXVECTOR3( m_BoundSize.x, -m_BoundSize.y,  m_BoundSize.z), min, max) ||
			   Math::PointInBox(m_vPos+D3DXVECTOR3( -m_BoundSize.x, -m_BoundSize.y, -m_BoundSize.z), min, max) ||
			   Math::PointInBox(m_vPos+D3DXVECTOR3( -m_BoundSize.x, -m_BoundSize.y,  m_BoundSize.z), min, max) ||
			   Math::PointInBox(m_vPos+D3DXVECTOR3( m_BoundSize.x, -m_BoundSize.y, -m_BoundSize.z), min, max);			   
	}



	//--------------------------------------------------------------------------------------
	// Checks for a ray-triangle intersection with a model,
	//    and gets the distance to the triangle as well as 
	//    the barycentric hit coordinates
	//--------------------------------------------------------------------------------------	
	bool RayIntersectMesh( MeshObject* pMesh, D3DXVECTOR3 &orig, D3DXVECTOR3 &dir,
						   float *t, float *u, float *v, int *mesh, float maxD)
	{
		if(!pMesh)
			return 0;
		if(!pMesh->GetMesh())
			return 0;

		// First perform a bounding box test to see if its worth checking per polygon
		if(!D3DXBoxBoundProbe(&(pMesh->GetPos()-pMesh->GetBoundSize()), &(pMesh->GetPos()+pMesh->GetBoundSize()), &orig, &dir))
			return false;

		D3DXVECTOR3 tri[3],vPos;
		bool ret = false;
		float dist,D=maxD,U,V;
		for(int i=0; i<pMesh->GetNumSubMesh(); i++)
		{
			for(int e=pMesh->GetSubMesh(i)->startIndex; 
				e<pMesh->GetSubMesh(i)->startIndex + pMesh->GetSubMesh(i)->numIndices; e+=3)
			{
				// Get verts from a cloth
				/*if(pMesh->IsCloth())
				{
					NxVec3 npos;
					npos = pMesh->GetClothVerts()[pMesh->GetMesh()->GetIndices()[e]];
					tri[0] = D3DXVECTOR3(npos.x, npos.y, npos.z);
					npos = pMesh->GetClothVerts()[pMesh->GetMesh()->GetIndices()[e+1]];
					tri[1] = D3DXVECTOR3(npos.x, npos.y, npos.z);
					npos = pMesh->GetClothVerts()[pMesh->GetMesh()->GetIndices()[e+2]];
					tri[2] = D3DXVECTOR3(npos.x, npos.y, npos.z);
				}
				else*/
				{
					// Skinned meshes
					if(pMesh->IsSkinned())
					{
						// Must perform skinning in software here for accurate picking
						D3DXVECTOR3 tri2[3];
						SkinnedVertex verts[3];
						verts[0] = pMesh->GetMesh()->GetSkinnedVerts()[ pMesh->GetMesh()->GetIndices()[e] ];
						verts[1] = pMesh->GetMesh()->GetSkinnedVerts()[ pMesh->GetMesh()->GetIndices()[e+1] ];
						verts[2] = pMesh->GetMesh()->GetSkinnedVerts()[ pMesh->GetMesh()->GetIndices()[e+2] ];
						tri[0] = tri[1] = tri[2] = D3DXVECTOR3(0,0,0);
						for(DWORD k=0; k<pMesh->GetNumBonesPerVertex(); k++)
						for(int j=0; j<3; j++)
						{
							D3DXVec3TransformCoord(&tri2[j], &verts[j].pos,&pMesh->GetAnimMatrices()[verts[j].indices[k]]);
							tri[j] += verts[j].weights[k] * tri2[j];
						}

						// Now take the world matrix into account
						tri2[0] = tri[0];
						tri2[1] = tri[1];
						tri2[2] = tri[2];
						D3DXVec3TransformCoord(&tri[0], &tri2[0], &pMesh->GetWorldMatrix());
						D3DXVec3TransformCoord(&tri[1], &tri2[1], &pMesh->GetWorldMatrix());
						D3DXVec3TransformCoord(&tri[2], &tri2[2], &pMesh->GetWorldMatrix());
					}
					// Just grab the verts from a normal mesh
					else
					{
						D3DXVec3TransformCoord(&tri[0], &pMesh->GetMesh()->GetVerts()[ pMesh->GetMesh()->GetIndices()[e] ].pos, &pMesh->GetWorldMatrix());
						D3DXVec3TransformCoord(&tri[1], &pMesh->GetMesh()->GetVerts()[ pMesh->GetMesh()->GetIndices()[e+1] ].pos, &pMesh->GetWorldMatrix());
						D3DXVec3TransformCoord(&tri[2], &pMesh->GetMesh()->GetVerts()[ pMesh->GetMesh()->GetIndices()[e+2] ].pos, &pMesh->GetWorldMatrix());
					}
					
				}
				if(D3DXIntersectTri(&tri[0],&tri[1],&tri[2],&orig,&dir,&U,&V,&dist) && (dist>0&&dist<maxD) && dist<D)
				{
					D=dist;
					*t = D;
					*u = U;
					*v = V;
					*mesh = i;
					ret = true;
				}
			}
		}
		return ret;
	}

}