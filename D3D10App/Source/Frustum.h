//--------------------------------------------------------------------------------------
// File: Frustum.h
//
// View Frustum
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

namespace Core
{

	//--------------------------------------------------------------------------------------
	// View Frustum
	// Defined by 6 planes; supports basic primitive tests
	//--------------------------------------------------------------------------------------
	class Frustum
	{
	public:

		// Contruct the frustum from view and projection matrices	
		void Build(const D3DXMATRIX* matView, const D3DXMATRIX* matProj);

		// Build from a set of points.  The order must start with top left and go clockwise
		void Build(D3DXVECTOR3& camPos, D3DXVECTOR3 corners[4], float minZ, float maxZ);

		// TRUE if the point lies inside the frustum
		bool CheckPoint(const D3DXVECTOR3& vec);

		// TRUE if the cube intersects the frustum
		bool CheckCube(float XCenter, float YCenter, float ZCenter, float Size);
		
		// TRUE if the rect intersects the frustum
		bool CheckRectangle(const D3DXVECTOR3& vPos, const D3DXVECTOR3& vBounds);
		
		// TRUE if the sphere intersects the frustum
		bool CheckSphere(const D3DXVECTOR3& Pos, float Radius);

	protected:

		D3DXPLANE m_Planes[6];	// Bounding planes
	};
}
