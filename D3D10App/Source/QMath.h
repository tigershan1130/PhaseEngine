//--------------------------------------------------------------------------------------
// File: QMath.h
//
// Various math routines
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

const float EPSILON=0.000001f;

#include <math.h>
#include "Vertex.h"
#include "Camera.h"

namespace Core
{
	// Math helper functions
	class Math
	{
	public:

		//--------------------------------------------------------------------------------------
		// Computes a gaussian sampling value
		//--------------------------------------------------------------------------------------
		static float ComputeGaussianValue( float x, float mean, float std_deviation );
		
		//--------------------------------------------------------------------------------------
		// Checks if the number is a power of 2
		//--------------------------------------------------------------------------------------
		static inline bool IsPowerOf2(int i)
		{
			return i > 0 && (i & (i - 1)) == 0;
		} 

	
		//--------------------------------------------------------------------------------------
		// Clamp an int
		//--------------------------------------------------------------------------------------
		template <class T>
		static inline T& Clamp(T& val, T a, T b)
		{
			if(val<a) val=a;
			if(val>b) val=b;
			return val;
		}
		
		//--------------------------------------------------------------------------------------
		// Clamp a vector
		//--------------------------------------------------------------------------------------
		static inline void Clamp(D3DXVECTOR2& vec, float a, float b)
		{
			if(vec.x<a) vec.x=a;
			if(vec.y<a) vec.y=a;
			if(vec.x>b) vec.x=b;
			if(vec.y>b) vec.y=b;
		}

		//--------------------------------------------------------------------------------------
		// Get the Max value
		//--------------------------------------------------------------------------------------
		template <class T>
		static inline const T Max(T a, T b)
		{
			return a<b ? b : a;
		}

		//--------------------------------------------------------------------------------------
		// Get the min value
		//--------------------------------------------------------------------------------------
		template <class T>
		static inline const T Min(T a, T b)
		{
			return a>b ? b : a;
		}

		//--------------------------------------------------------------------------------------
		// Get the Max value
		//--------------------------------------------------------------------------------------
		static inline D3DXVECTOR3 MaxVector(const D3DXVECTOR3& v1, const D3DXVECTOR3& v2)
		{
			return D3DXVECTOR3( MaxFloat(v1.x,v2.x), MaxFloat(v1.y,v2.y), MaxFloat(v1.z,v2.z) );
		}

		//--------------------------------------------------------------------------------------
		// Get the Min value
		//--------------------------------------------------------------------------------------
		static inline D3DXVECTOR3 MinVector(const D3DXVECTOR3& v1, const D3DXVECTOR3& v2)
		{
			return D3DXVECTOR3( MinFloat(v1.x,v2.x), MinFloat(v1.y,v2.y), MinFloat(v1.z,v2.z) );
		}

		//--------------------------------------------------------------------------------------
		// Linear interpolation
		//--------------------------------------------------------------------------------------
		static inline float Lerp(float x, float y, float s)
		{
			return x + s*(y - x);
		}


		//--------------------------------------------------------------------------------------
		// Converts a float to a DWORD
		//--------------------------------------------------------------------------------------
		static inline DWORD F2DW( float f ) 
		{ 
			return *((DWORD*)&f); 
		}


		//--------------------------------------------------------------------------------------
		// Return the absolute value of a vector
		//--------------------------------------------------------------------------------------
		static inline D3DXVECTOR3 Absolute(const D3DXVECTOR3& v)
		{
			return D3DXVECTOR3(fabs(v.x),fabs(v.y),fabs(v.z));
		}


		//--------------------------------------------------------------------------------------
		// Converts a color to a vector
		//--------------------------------------------------------------------------------------
		static inline D3DXVECTOR4 ColorToVector(const D3DCOLORVALUE& color)
		{
			return D3DXVECTOR4(color.r,color.g,color.b,color.a);
		}

		
		//--------------------------------------------------------------------------------------
		// Checks for a box-sphere intersection
		//--------------------------------------------------------------------------------------
		static inline bool BoxSphereIntersect(const D3DXVECTOR3& bmin, const D3DXVECTOR3& bmax, const D3DXVECTOR3& center, const float radius)
		{
			return false;
// 			dmin = 0;
// 			for( i = 0; i < n; i++ ) {
// 				if( C[i] < Bmin[i] ) dmin += SQR(C[i] - Bmin[i] ); else
// 					if( C[i] > Bmax[i] ) dmin += SQR( C[i] - Bmax[i] );     
// 			}
// 			if( dmin <= r2 ) return( TRUE );

		}

		
		// Checks for a ray-triangle intersection and gets the distance
		// to the triangle and the barycentric hit coordinates
		//--------------------------------------------------------------------------------------	
		static int RayIntersectTri(D3DXVECTOR3 &vert0, D3DXVECTOR3 &vert1, D3DXVECTOR3 &vert2,
							D3DXVECTOR3 &orig, D3DXVECTOR3 &dir,
							float *t, float *u, float *v);

		//--------------------------------------------------------------------------------------
		// Gets the ray for a given pixel to trace
		//--------------------------------------------------------------------------------------
		static void GetPickRay(int x, int y, Camera& cam, D3DXVECTOR3& oPos, D3DXVECTOR3& oDir);


		//--------------------------------------------------------------------------------------
		// Makes a plane from 3 points
		//--------------------------------------------------------------------------------------
		static void PlaneFromPoints( D3DXVECTOR3* tri, D3DXPLANE* oPlane );


		//--------------------------------------------------------------------------------------
		// Ray-Plane intersection test
		//--------------------------------------------------------------------------------------
		static bool RayPlaneIntersect(D3DXVECTOR3 &origin, D3DXVECTOR3 &direction, D3DXPLANE &plane, float *t);


		//--------------------------------------------------------------------------------------
		// Splits a triangle with a plane
		//--------------------------------------------------------------------------------------
		static bool PlaneSplitTri( Vertex* tri, D3DXPLANE& plane, Vertex* oTri1, Vertex* oTri2, Vertex* oTri3 );

		//--------------------------------------------------------------------------------------
		// Checks if a point is on a plane
		//--------------------------------------------------------------------------------------
		static inline bool PointOnPlane( D3DXVECTOR3& point, D3DXPLANE& plane )
		{
			D3DXVECTOR3 p(plane.a,plane.b,plane.c);
			return D3DXVec3Dot(&point,&p) == plane.d;
		}

		//--------------------------------------------------------------------------------------
		// Checks if a point is in a box
		//--------------------------------------------------------------------------------------
		static inline bool PointInBox( D3DXVECTOR3& point, D3DXVECTOR3& min, D3DXVECTOR3& max)
		{
			return ( (point.x>=min.x && point.x<=max.x)&&
					 (point.y>=min.y && point.y<=max.y)&&
					 (point.z>=min.z && point.z<=max.z) );
		}


		//--------------------------------------------------------------------------------------
		// Returns the normal of a polygon
		//--------------------------------------------------------------------------------------
		static inline void GetNormalFromTri(const D3DXVECTOR3 vPolygon[], D3DXVECTOR3& normal)					
		{														
			D3DXVECTOR3 vVector1 = vPolygon[2] - vPolygon[0];
			D3DXVECTOR3 vVector2 = vPolygon[1] - vPolygon[0];
			D3DXVec3Cross(&normal,&vVector1,&vVector2);
			D3DXVec3Normalize(&normal,&normal);
		}


		//--------------------------------------------------------------------------------------
		// Gets a rotation matrix from a view vector
		//--------------------------------------------------------------------------------------
		static void BuildRotationMatrix(const D3DXVECTOR3& dir, D3DXMATRIX& mat);


		//--------------------------------------------------------------------------------------
		// Check if a sphere intersects an AABB
		//--------------------------------------------------------------------------------------
		static bool SphereIntersectAABB(const D3DXVECTOR3& posSphere, float radius, const D3DXVECTOR3& posBox, const D3DXVECTOR3& bounds );
	};

}