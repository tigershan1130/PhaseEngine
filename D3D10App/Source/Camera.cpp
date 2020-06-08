//--------------------------------------------------------------------------------------
// File: Camera.cpp
//
// 3D Camera class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Camera.h"
#include "Log.h"
#include "QMath.h"
#include <math.h>

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	Camera::Camera()
	{
		Reset();
		D3DXMatrixIdentity(&m_matWorld);
		D3DXMatrixIdentity(&m_matView);
		D3DXMatrixIdentity(&m_matProj);
	}

	//--------------------------------------------------------------------------------------
	// Restores defaults
	//--------------------------------------------------------------------------------------
	void Camera::Reset()
	{
		m_fXDeg = 0.0f;
		m_fYDeg = 0.0f;
		m_fRadius=2.0f;
		m_fHeight = 6.0f;
		m_fFov = (float)(D3DX_PI/3.0);
		m_iTimer=0;
		m_bView = true;
		m_fFarZ = 5000.0f;
		m_fNearZ = 0.01f;
		m_Type = CAMERA_FREE;
		m_fFocusZoom = 5.0f;
		m_vPos.x=0.0f;
		m_vPos.y=20.0f;
		m_vPos.z=-5.0f;
		m_vVel*=0.0f;
	}

	//--------------------------------------------------------------------------------------
	// Points the camera at the given point
	//--------------------------------------------------------------------------------------
	void Camera::LookAt( D3DXVECTOR3& v )
	{
		// Use 2D projections onto XZ and XY planes to compute
		// rotation angles
		
		// Find the theta components of new of new view direction
		float dThetaY1=0, dThetaY2=100;
		float dThetaX1=100, dThetaX2=100;
		D3DXVECTOR3 newDir = v-m_vPos;
		float mag = sqrtf(newDir.x*newDir.x + newDir.z*newDir.z);
		dThetaY1 = asinf(newDir.z/mag);
		mag = sqrtf(newDir.x*newDir.x + newDir.y*newDir.y);
		dThetaX1 = asinf(newDir.y/mag);
		while( (fabs(dThetaX1-dThetaX2)+fabs(dThetaY1-dThetaY2)) > 0.05f)
		{
			// Find the theta components from the old view direction
			mag = sqrtf(m_vDir.x*m_vDir.x + m_vDir.z*m_vDir.z);
			dThetaY2 = asinf(m_vDir.z/mag);
			mag = sqrtf(m_vDir.x*m_vDir.x + m_vDir.y*m_vDir.y);
			dThetaX2 = asinf(m_vDir.y/mag);

			// Get the final angles
			m_fXDeg -= dThetaX1-dThetaX2;
			m_fYDeg -= dThetaY1-dThetaY2;

			// Get the new direction vector
			D3DXQUATERNION qR;
			D3DXMatrixIdentity(&m_matOrientation);
			D3DXQuaternionRotationYawPitchRoll(&qR, m_fYDeg, m_fXDeg, 0.0f);
			D3DXMatrixAffineTransformation(&m_matOrientation, 1.25f, NULL, &qR, &m_vPos);
			m_vDir = D3DXVECTOR3(0,0,1);
			D3DXVec3TransformNormal ( &m_vDir, &m_vDir, &m_matOrientation );
		}
	}


	//--------------------------------------------------------------------------------------
	// Initializes a camera object
	//--------------------------------------------------------------------------------------
	void Camera::BuildProjectionMatrix(int width, int height)
	{
		// Build the projection matrix
		m_Width = width;
		m_Height = height;
		m_fFovRatio = (float)width/(float)height;
		m_fHNear = 2.0f * tanf(m_fFov / 2.0f) *m_fNearZ;
		m_fWNear = m_fHNear * m_fFovRatio;
		m_fHFar = 2.0f * tan(m_fFov / 2.0f) * m_fFarZ;
		m_fWFar = m_fHFar * m_fFovRatio;
		D3DXMatrixPerspectiveFovLH( &m_matProj, m_fFov, m_fFovRatio, m_fNearZ, m_fFarZ );
	}


	//--------------------------------------------------------------------------------------
	// Updates the motion and input
	//--------------------------------------------------------------------------------------
	void Camera::Update(bool physics)
	{
		// Adjust the view angles
		if(m_bView)
		{
			m_fYDeg+=m_fYDegV;
			m_fXDeg+=m_fXDegV;
			m_fXDegV*=0.5f;
			m_fYDegV*=0.5f;
		}

		// Build the orientation to transform the velocity
		D3DXQUATERNION qR;
		D3DXQuaternionRotationYawPitchRoll(&qR, m_fYDeg, m_fXDeg, 0.0f);
		D3DXMatrixAffineTransformation(&m_matOrientation, 1.25f, NULL, &qR, &m_vPos);
		
		
		// For RTS, we want uniform movement independent of the view angles so ignore rotation
		D3DXVECTOR3 vT = m_vVel;
		if(m_Type == CAMERA_FREE)
			D3DXVec3TransformNormal ( &vT, &vT, &m_matOrientation );	
		m_vPos += vT; 
		
		// Dampen the velocity
		if(m_Type == CAMERA_FREE)
			m_vVel.y*=0.6f;
		m_vVel.x*=0.6f;
		m_vVel.z*=0.6f;
		
		// Build the view matrix and update the view direction
		m_vDir = D3DXVECTOR3(0,0,1);
		m_vNormal = D3DXVECTOR3(1,0,0);
		D3DXMatrixAffineTransformation(&m_matOrientation, 1.25f, NULL, &qR, &m_vPos);
		D3DXVec3TransformNormal ( &m_vNormal, &m_vNormal, &m_matOrientation );
		D3DXVec3TransformNormal ( &m_vDir, &m_vDir, &m_matOrientation );
		
		// Arcball style camera in fixed mode, otherwise normal free moving view matrix
		if(m_Type != CAMERA_FIXED)
			D3DXMatrixInverse(&m_matView, NULL, &m_matOrientation);
		else
		{
			D3DXVECTOR3 vUp = D3DXVECTOR3(0,1,0);
			D3DXVECTOR3 vPos = m_vPos-m_vFocusPoint;
			D3DXVec3Normalize(&vPos, &vPos);
			vPos = m_vFocusPoint + vPos*m_fFocusZoom;
			D3DXMatrixLookAtLH(&m_matView, &vPos, &m_vFocusPoint, &vUp);
			m_vPos = vPos;
		}

		// Build the frustum
		m_Frustum.Build(&m_matView,&m_matProj);

		// Compute the far plane corners
		D3DXVECTOR3 fc = m_vPos + m_fFarZ*m_vDir; 
		D3DXVECTOR3 up = D3DXVECTOR3(0,m_fHFar/2.0f,0);
		D3DXVECTOR3 right = D3DXVECTOR3(m_fWFar/2.0f,0,0);
		D3DXVec3TransformNormal ( &right, &right, &m_matOrientation );
		D3DXVec3TransformNormal ( &up, &up, &m_matOrientation );
		m_vFarCorners[FRUSTUM_TOP_LEFT] = fc + up - right;
		m_vFarCorners[FRUSTUM_TOP_RIGHT] = fc + up + right;
		m_vFarCorners[FRUSTUM_BOTTOM_LEFT] = fc - up - right;
		m_vFarCorners[FRUSTUM_BOTTOM_RIGHT] = fc - up + right;
	}

}