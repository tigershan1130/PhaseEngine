//--------------------------------------------------------------------------------------
// File: Camera.h
//
// 3D Camera class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "Frustum.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// 3D First Person Camera Class
	//--------------------------------------------------------------------------------------
	class Camera
	{
	public:	

		// Frustum corders
		enum FRUSTUM_TYPE : int
		{
			FRUSTUM_TOP_RIGHT=0,
			FRUSTUM_TOP_LEFT,
			FRUSTUM_BOTTOM_RIGHT,
			FRUSTUM_BOTTOM_LEFT,
		};

		// Camera type
		enum CAMERA_TYPE : int
		{
			CAMERA_FREE=0,
			CAMERA_FIXED=1,
		};

		Camera();
			
		// Restores defaults
		void Reset();

		// Initializes the camera matrix
		void BuildProjectionMatrix(int width, int height);		

		// Points the camera at the given point
		void LookAt( D3DXVECTOR3& v );
		
		// Updates camera movement
		void Update(bool physics);

		// Set the camera type
		inline void SetType(CAMERA_TYPE type){ m_Type = type; }
		inline CAMERA_TYPE GetType(){ return m_Type; }

		// Set the focal point
		inline void SetFocusPoint(D3DXVECTOR3& pos){ m_vFocusPoint = pos; }
		inline void SetFocusZoom(float f){ m_fFocusZoom=f; }
		inline float GetFocusZoom(){ return m_fFocusZoom; }

		// Gets the position
		inline D3DXVECTOR3& GetPos(){ return m_vPos; }
		inline D3DXVECTOR3& GetVelocity(){ return m_vVel; }
		
		// Sets the position
		inline void SetPos(const D3DXVECTOR3& v){ m_vPos = v; }
		inline void SetVelocity(const D3DXVECTOR3& v){ m_vVel = v; }
		inline void AddToPos(const D3DXVECTOR3& v){ m_vPos += v; }
		inline void AddToPos(float x, float y, float z){ m_vPos.x += x; m_vPos.y += y; m_vPos.z += z;}
		inline void AddToVelocity(const D3DXVECTOR3& v){ m_vVel += v; }
		inline void AddToVelocity(float x, float y, float z){ m_vVel.x += x; m_vVel.y += y; m_vVel.z += z;}
		
		// Gets the view angles
		inline float GetXDeg(){ return m_fXDeg; }
		inline float GetYDeg(){ return m_fYDeg; }

		// Sets the view angles
		inline void SetXDeg(const float d){ m_fXDeg=d; }
		inline void SetYDeg(const float d){ m_fYDeg=d; }
		inline void AddToAngularVelocity(float x, float y){ m_fXDegV+=x; m_fYDegV+=y; }
		
		// Gets the camera radius
		inline float GetRadius(){ return m_fRadius; }
		
		// Sets the radius
		inline void SetRadius(const float r){ m_fRadius=r; }
		
		// Gets the view D3DXVECTOR3
		inline const D3DXVECTOR3& GetView(){ return m_vDir; }
		
		// Gets the field of fiew
		inline float GetFOV(){ return m_fFov; }

		// Get a frustum corner
		inline D3DXVECTOR3& GetFrustumCorner(FRUSTUM_TYPE corner){ return m_vFarCorners[corner]; }

		// Gets the matrices
		inline const D3DXMATRIX& GetViewMatrix(){ return m_matView; }
		inline const D3DXMATRIX& GetProjMatrix(){ return m_matProj; }
		inline const D3DXMATRIX& GetWorldMatrix(){ return m_matWorld; }
		
		// Gets the view frustum
		inline Frustum& GetFrustum(){ return m_Frustum; }

		// View toggle
		inline void EnableView(){ m_bView=true; }
		inline void DisableView(){ m_bView=false; }

		// Clips
		inline float GetFarZ(){ return m_fFarZ; }
		inline float GetNearZ(){ return m_fNearZ; }
		inline void SetFarZ(float f){ m_fFarZ=f; }
		inline void SetNearZ(float f){ m_fNearZ=f; }

		// Dimensions
		inline UINT BackBufferWidth(){ return m_Width; }
		inline UINT BackBufferHeight(){ return m_Height; }

	protected:
			
		D3DXVECTOR3			m_vPos;				// position
		D3DXVECTOR3			m_vVel;				// velocity
		D3DXVECTOR3			m_vForce;			// force
		D3DXVECTOR3			m_vFocusPoint;		// Focal point for fixed cameras	
		float				m_fHeight;			// height
		float				m_fFocusZoom;		// Focal radius
		D3DXVECTOR3			m_vDir;				// direction
		D3DXVECTOR3			m_vNormal;			// normal to the view
		D3DXMATRIX			m_matWorld;			// world transformation matrix
		D3DXMATRIX			m_matView;			// view transformation matrix
		D3DXMATRIX			m_matProj;			// projection transformation matrix
		D3DXMATRIX			m_matOrientation;	// view orientation matrix
		float				m_fXDeg;			// x-view angle
		float				m_fYDeg;			// y-view angle
		float				m_fXDegV;			// x-view angle velocity
		float				m_fYDegV;			// y-view angle velocity
		float				m_fRadius;			// radius
		float				m_fFarZ;			// Far clip Z
		float				m_fNearZ;			// Near clip Z
		float				m_fFov;				// field of view
		float				m_fFovRatio;		// width/height
		float				m_fHNear;			// Near plane height
		float				m_fWNear;			// Near plane width
		float				m_fHFar;			// Far plane height
		float				m_fWFar;			// Far plane width
		D3DXVECTOR3			m_vFarCorners[4];	// Far frustum plane corners
		float				m_fDist;			// distance to the ground
		Frustum				m_Frustum;			// view frustum
		int					m_iTimer;			// Timer
		bool				m_bView;
		UINT				m_Width,m_Height;			// BackBuffer dimensions
		CAMERA_TYPE			m_Type;
	};

}