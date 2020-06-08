//--------------------------------------------------------------------------------------
// File: Frustum.cpp
//
// View Frustum
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Frustum.h"


namespace Core
{

	//--------------------------------------------------------------------------------------
	// Builds the frustum from the view matrix
	//--------------------------------------------------------------------------------------
	void Frustum::Build(const D3DXMATRIX* matView, const D3DXMATRIX* matProj)
	{
		D3DXMATRIX Matrix;
		D3DXMatrixMultiply(&Matrix, matView, matProj);

		// Calculate the planes
		m_Planes[0].a = Matrix._14 + Matrix._13; // Near
		m_Planes[0].b = Matrix._24 + Matrix._23;
		m_Planes[0].c = Matrix._34 + Matrix._33;
		m_Planes[0].d = Matrix._44 + Matrix._43;
		D3DXPlaneNormalize(&m_Planes[0], &m_Planes[0]);

		m_Planes[1].a = Matrix._14 - Matrix._13; // Far
		m_Planes[1].b = Matrix._24 - Matrix._23;
		m_Planes[1].c = Matrix._34 - Matrix._33;
		m_Planes[1].d = Matrix._44 - Matrix._43;
		D3DXPlaneNormalize(&m_Planes[1], &m_Planes[1]);

		m_Planes[2].a = Matrix._14 + Matrix._11; // Left
		m_Planes[2].b = Matrix._24 + Matrix._21;
		m_Planes[2].c = Matrix._34 + Matrix._31;
		m_Planes[2].d = Matrix._44 + Matrix._41;
		D3DXPlaneNormalize(&m_Planes[2], &m_Planes[2]);

		m_Planes[3].a = Matrix._14 - Matrix._11; // Right
		m_Planes[3].b = Matrix._24 - Matrix._21;
		m_Planes[3].c = Matrix._34 - Matrix._31;
		m_Planes[3].d = Matrix._44 - Matrix._41;
		D3DXPlaneNormalize(&m_Planes[3], &m_Planes[3]);

		m_Planes[4].a = Matrix._14 - Matrix._12; // Top
		m_Planes[4].b = Matrix._24 - Matrix._22;
		m_Planes[4].c = Matrix._34 - Matrix._32;
		m_Planes[4].d = Matrix._44 - Matrix._42;
		D3DXPlaneNormalize(&m_Planes[4], &m_Planes[4]);

		m_Planes[5].a = Matrix._14 + Matrix._12; // Bottom
		m_Planes[5].b = Matrix._24 + Matrix._22;
		m_Planes[5].c = Matrix._34 + Matrix._32;
		m_Planes[5].d = Matrix._44 + Matrix._42;
		D3DXPlaneNormalize(&m_Planes[5], &m_Planes[5]);
	}

	
	
	//--------------------------------------------------------------------------------------
	// Build from a set of points.  The order must start with top left and go clockwise
	//--------------------------------------------------------------------------------------
	void Frustum::Build(D3DXVECTOR3& camPos, D3DXVECTOR3 corners[4], float minZ, float maxZ)
	{
		// Get the corners
		D3DXVECTOR3 nearCorners[4], farCorners[4];
		nearCorners[0] = camPos + corners[0] * minZ;
		nearCorners[1] = camPos + corners[1] * minZ;
		nearCorners[2] = camPos + corners[2] * minZ;
		nearCorners[3] = camPos + corners[3] * minZ;
		farCorners[0] = camPos + corners[0] * maxZ;
		farCorners[1] = camPos + corners[1] * maxZ;
		farCorners[2] = camPos + corners[2] * maxZ;
		farCorners[3] = camPos + corners[3] * maxZ;
		
		// We must get the planes by performing dot products with the faces
		
		// Near
		D3DXPlaneFromPoints(&m_Planes[0], &nearCorners[2], &nearCorners[1], &nearCorners[0]);
		
		// Far
		D3DXPlaneFromPoints(&m_Planes[1], &farCorners[0], &farCorners[1], &farCorners[2]);
		
		// Left
		D3DXPlaneFromPoints(&m_Planes[2], &nearCorners[3], &nearCorners[0], &farCorners[0]);
		
		// Right
		D3DXPlaneFromPoints(&m_Planes[3], &farCorners[2], &farCorners[1], &nearCorners[1]);
		
		// Top
		D3DXPlaneFromPoints(&m_Planes[4], &nearCorners[1], &farCorners[1], &farCorners[0]);
		
		// Bottom
		D3DXPlaneFromPoints(&m_Planes[5], &nearCorners[3], &farCorners[3], &farCorners[2]);
	}



	//--------------------------------------------------------------------------------------
	// Checks if a point is in the frustum
	//--------------------------------------------------------------------------------------
	bool Frustum::CheckPoint(const D3DXVECTOR3& vec)
	{
	  // Make sure point is in frustum
	  for(short i=0;i<6;i++) {
		if(D3DXPlaneDotCoord(&m_Planes[i], &vec) < 0.0f)
		  return false;
	  }

	  return true;
	}


	//--------------------------------------------------------------------------------------
	// Checks if a cube is in the frustum
	//--------------------------------------------------------------------------------------
	bool Frustum::CheckCube(float XCenter, float YCenter, float ZCenter, float Size)
	{
	  float minus[3],plus[3];
	  minus[0] = XCenter-Size;
	  minus[1] = YCenter-Size;
	  minus[2] = ZCenter-Size;
	  plus[0] = XCenter+Size;
	  plus[1] = YCenter+Size;
	  plus[2] = ZCenter+Size;

	  D3DXVECTOR3 vec[8] = 
	  {
		D3DXVECTOR3(minus[0], minus[1], minus[2]),
		D3DXVECTOR3(plus[0], minus[1], minus[2]),
		D3DXVECTOR3(minus[0], plus[1], minus[2]),
		D3DXVECTOR3(plus[0], plus[1], minus[2]),
		D3DXVECTOR3(minus[0], minus[1], plus[2]),
		D3DXVECTOR3(plus[0], minus[1], plus[2]),
		D3DXVECTOR3(minus[0], plus[1], plus[2]),
		D3DXVECTOR3(plus[0], plus[1], plus[2]),
	  };


	  // Make sure at least one point is in frustum
	  for(short i=0; i<6; i++) 
	  {
		if(D3DXPlaneDotCoord(&m_Planes[i], &vec[0]) >= 0.0f)
		  continue;
		if(D3DXPlaneDotCoord(&m_Planes[i], &vec[1]) >= 0.0f)
		  continue;
		if(D3DXPlaneDotCoord(&m_Planes[i], &vec[2]) >= 0.0f)
		  continue;
		if(D3DXPlaneDotCoord(&m_Planes[i], &vec[3]) >= 0.0f)
		  continue;
		if(D3DXPlaneDotCoord(&m_Planes[i], &vec[4]) >= 0.0f)
		  continue;
		if(D3DXPlaneDotCoord(&m_Planes[i], &vec[5]) >= 0.0f)
		  continue;
		if(D3DXPlaneDotCoord(&m_Planes[i], &vec[6]) >= 0.0f)
		  continue;
		if(D3DXPlaneDotCoord(&m_Planes[i], &vec[7]) >= 0.0f)
		  continue;

		return false;
	  }

	  return true;
	}

	//--------------------------------------------------------------------------------------
	// Checks if a rectangle is in the frustum
	//--------------------------------------------------------------------------------------
	bool Frustum::CheckRectangle(const D3DXVECTOR3& vPos, const D3DXVECTOR3& vBounds)
	{
		D3DXVECTOR3 p;
		D3DXVECTOR3 vMax = (D3DXVECTOR3)(vPos+vBounds);
		D3DXVECTOR3 vMin = (D3DXVECTOR3)(vPos-vBounds);
		for(short i=0; i < 6; i++) 
		{
			p = vMin;
			if (m_Planes[i].a >= 0)
				p.x = vMax.x;
			if (m_Planes[i].b >=0)
				p.y = vMax.y;
			if (m_Planes[i].c >= 0)
				p.z = vMax.z;
			if( D3DXPlaneDotCoord(&m_Planes[i], &p) < 0)
				return false;
		}
		return true;
	}


	//--------------------------------------------------------------------------------------
	// Checks is a sphere is in the frustum
	//--------------------------------------------------------------------------------------
	bool Frustum::CheckSphere(const D3DXVECTOR3& Pos, float Radius)
	{
	  // Make sure radius is in frustum
	  if( D3DXPlaneDotCoord(&m_Planes[0], &Pos) < -Radius ||
		  D3DXPlaneDotCoord(&m_Planes[1], &Pos) < -Radius ||
		  D3DXPlaneDotCoord(&m_Planes[2], &Pos) < -Radius ||
		  D3DXPlaneDotCoord(&m_Planes[3], &Pos) < -Radius ||
		  D3DXPlaneDotCoord(&m_Planes[4], &Pos) < -Radius ||
		  D3DXPlaneDotCoord(&m_Planes[5], &Pos) < -Radius )
		  return false;
	  
	  return true;
	}

}