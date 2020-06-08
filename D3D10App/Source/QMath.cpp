//--------------------------------------------------------------------------------------
// File: QMath.h
//
// Various math routines
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "QMath.h"

namespace Core
{

	
	//--------------------------------------------------------------------------------------
	// Computes a gaussian sampling value
	//--------------------------------------------------------------------------------------
	float Math::ComputeGaussianValue( float x, float mean, float std_deviation )
	{
		// The gaussian equation is defined as such:
		/*    
		-(x - mean)^2
		-------------
		1.0               2*std_dev^2
		f(x,mean,std_dev) = -------------------- * e^
		sqrt(2*pi*std_dev^2)

		*/

		return ( 1.0f / sqrt( 2.0f * D3DX_PI * std_deviation * std_deviation ) )
			* expf( ( -( ( x - mean ) * ( x - mean ) ) ) / ( 2.0f * std_deviation * std_deviation ) );
	}
	
	
	//--------------------------------------------------------------------------------------
	// Gets the ray for a given pixel to trace
	//--------------------------------------------------------------------------------------
	void Math::GetPickRay(int x, int y, Camera& cam, D3DXVECTOR3& oPos, D3DXVECTOR3& oDir)
	{
		// Get the ray in screen space
		D3DXMATRIX matProj = cam.GetProjMatrix();
		D3DXVECTOR3 v;
		v.x =  ( ( ( 2.0f * x ) / cam.BackBufferWidth() ) - 1 ) / matProj._11;
		v.y = -( ( ( 2.0f * y ) / cam.BackBufferHeight() ) - 1 ) / matProj._22;
		v.z =  1.0f;
	
		// Get the inverse view matrix
		D3DXMATRIX m;
		D3DXMatrixInverse( &m, NULL, &cam.GetViewMatrix() );
	
		// Transform the screen space ray into 3D space
		oDir.x  = v.x*m._11 + v.y*m._21 + v.z*m._31;
		oDir.y  = v.x*m._12 + v.y*m._22 + v.z*m._32;
		oDir.z  = v.x*m._13 + v.y*m._23 + v.z*m._33;
		D3DXVec3Normalize(&oDir,&oDir);
		oPos.x = m._41;
		oPos.y = m._42;
		oPos.z = m._43;
	}


	//--------------------------------------------------------------------------------------
	// Makes a plane from 3 points
	//--------------------------------------------------------------------------------------
	void Math::PlaneFromPoints( D3DXVECTOR3* tri, D3DXPLANE* oPlane )
	{
		/*D3DXVECTOR3 normal = GetNormal( tri );
		oPlane.a = normal.x;
		oPlane.b = normal.y;
		oPlane.c = normal.z;
		oPlane.d = - D3DXVec3Dot(&tri[0], &normal);*/
	}


	//--------------------------------------------------------------------------------------
	// Ray-Plane intersection test
	//--------------------------------------------------------------------------------------
	bool Math::RayPlaneIntersect(D3DXVECTOR3 &origin, D3DXVECTOR3 &direction, D3DXPLANE &plane, float *t)
	{
		D3DXVECTOR3 normal(plane.a, plane.b, plane.c);
		float rayD =  D3DXVec3Dot(&normal, &direction);
		if (abs(rayD) < EPSILON) return false;
		float originD = -( D3DXVec3Dot(&normal,&origin) + plane.d);
		*t = originD / rayD;
		return true;
	}


	//--------------------------------------------------------------------------------------
	// Splits a triangle with a plane
	//--------------------------------------------------------------------------------------
	bool Math::PlaneSplitTri( Vertex* tri, D3DXPLANE& plane, Vertex* oTri1, Vertex* oTri2, Vertex* oTri3 )
	{
		// Find the intersections of the triangle edges with the plane
		D3DXVECTOR3 edge;
		float edgeLength;
		float t;
		int num=0;
		Vertex points[2];

		// Check if any points are on the plane
		bool onPlane[3];
		onPlane[0] = PointOnPlane(tri[0].pos,plane);
		onPlane[1] = PointOnPlane(tri[1].pos,plane);
		onPlane[2] = PointOnPlane(tri[2].pos,plane);
		int numOnPlane=0;
		if(onPlane[0])
			numOnPlane++;
		if(onPlane[1])
			numOnPlane++;
		if(onPlane[2])
			numOnPlane++;
		
		// With 2 or 3 points on a plane there is no reason to split
		if(numOnPlane>1)
			return false;

		// handle this case of one point on the plane
		if(numOnPlane==1)
		{
			// Compute the other edge vector
			Vertex onPlaneVert;
			if(onPlane[0])
			{
				edge = tri[2].pos-tri[1].pos;
				points[0] = tri[2];
				points[1] = tri[1];
				onPlaneVert = tri[0];
			}
			else if(onPlane[1])
			{
				edge = tri[2].pos-tri[0].pos;
				points[0] = tri[2];
				points[1] = tri[0];
				onPlaneVert = tri[1];
			}
			else if(onPlane[2])
			{
				edge = tri[1].pos-tri[0].pos;
				points[0] = tri[1];
				points[1] = tri[0];
				onPlaneVert = tri[2];
			}
			edgeLength =  D3DXVec3Length(&edge);
			edge /= edgeLength;

			// Get the intersection point
			if(RayPlaneIntersect(points[0].pos, edge, plane, &t) && t<=edgeLength)
			{
				// Make the new vertex
				Vertex newPoint;
				newPoint.pos = points[0].pos + edge*t;
				float factor = t/edgeLength;
				newPoint.tu = (points[1].tu-points[0].tu)*factor + points[0].tu;
				newPoint.tv = (points[1].tu-points[0].tu)*factor + points[0].tu;

				// Now make the new triangles
				oTri1[0] = points[0];
				oTri1[1] = onPlaneVert;
				oTri1[2] = newPoint;
				oTri2[0] = newPoint;
				oTri2[1] = points[1];
				oTri2[2] = onPlaneVert;
				
				// Set the 3rd triangle to a value that will definitely be
				// outside a tree node
				oTri3[0].pos = D3DXVECTOR3(99999999.0f,99999999.0f,99999999.0f);
				oTri3[1].pos = D3DXVECTOR3(99999999.0f,99999999.0f,99999999.0f);
				oTri3[2].pos = D3DXVECTOR3(99999999.0f,99999999.0f,99999999.0f);
				return true;
			}

		}


		// Edge 1
		edge = tri[1].pos-tri[0].pos;
		edgeLength =  D3DXVec3Length(&edge);
		edge /= edgeLength;
		if(RayPlaneIntersect(tri[1].pos, edge, plane, &t) && t<=edgeLength)
		{
			points[num] = tri[1];
			points[num].pos += edge*t;
			float factor = t/edgeLength;
			points[num].tu = (tri[1].tu-tri[0].tu)*factor + tri[0].tu;
			points[num].tv = (tri[1].tu-tri[0].tu)*factor + tri[0].tu;
			num++;
		}

		// Edge 2
		edge = tri[2].pos-tri[0].pos;
		edgeLength =  D3DXVec3Length(&edge);
		edge /= edgeLength;
		if(RayPlaneIntersect(tri[2].pos, edge, plane, &t) && t<=edgeLength)
		{
			points[num] = tri[2];
			points[num].pos += edge*t;
			float factor = t/edgeLength;
			points[num].tu = (tri[2].tu-tri[0].tu)*factor + tri[0].tu;
			points[num].tv = (tri[2].tu-tri[0].tu)*factor + tri[0].tu;
			num++;
		}

		// If no intersections yet, plane does not intersect triangle
		if(num==0)
			return false;

		// If only one intersection, find the next
		if(num==1)
		{
			// Edge 3
			edge = tri[2].pos-tri[1].pos;
			edgeLength =  D3DXVec3Length(&edge);
			edge /= edgeLength;
			if(RayPlaneIntersect(tri[2].pos, edge, plane, &t) && t<=edgeLength)
			{
				points[num] = tri[2];
				points[num].pos += edge*t;
				float factor = t/edgeLength;
				points[num].tu = (tri[2].tu-tri[1].tu)*factor + tri[1].tu;
				points[num].tv = (tri[2].tu-tri[1].tu)*factor + tri[1].tu;
				num++;
			}
		}

		// Make sure there was 2 intersection points
		if(num!=2)
			return false;

		// Now that we have both intersection points, build the new triangles
		{
			// Find out which sides of the planes the points are on
			float D[3];
			D3DXVECTOR3 normal(plane.a, plane.b, plane.c);
			D[0] = D3DXVec3Dot(&tri[0].pos, &normal) - plane.d;
			D[1] = D3DXVec3Dot(&tri[1].pos, &normal) - plane.d;
			D[2] = D3DXVec3Dot(&tri[2].pos, &normal) - plane.d;
			Vertex* pair[2];
			Vertex* lone;
			int neg=0;
			if(D[0]<0) neg++;
			if(D[1]<0) neg++;
			if(D[2]<0) neg++;
			if(neg==0 || neg==3)
				return false;
			if(neg==2)
			{
				int lee=0;
				if(D[0]<0) pair[lee++] = &tri[0]; else lone = &tri[0];
				if(D[1]<0) pair[lee++] = &tri[1]; else lone = &tri[1];
				if(D[2]<0) pair[lee++] = &tri[2]; else lone = &tri[2];
			}
			else
			{
				int lee=0;
				if(D[0]>0) pair[lee++] = &tri[0]; else lone = &tri[0];
				if(D[1]>0) pair[lee++] = &tri[1]; else lone = &tri[1];
				if(D[2]>0) pair[lee++] = &tri[2]; else lone = &tri[2];
			}

			// Now split, the side with the lone point will have one triangle
			// and the side with the 2 points forms a quad that is split into 2 triangles
			oTri1[0] = *lone;
			oTri1[1] = points[0];
			oTri1[2] = points[1];

			oTri2[0] = points[0];
			oTri2[1] = *pair[0];
			oTri2[2] = *pair[1];

			oTri3[0] = *pair[1];
			oTri3[1] = points[1];
			oTri3[2] = points[0];
		}
		return true;
	}


	//--------------------------------------------------------------------------------------
	// Gets a rotation matrix from a view vector
	//--------------------------------------------------------------------------------------
	void Math::BuildRotationMatrix(const D3DXVECTOR3& dir, D3DXMATRIX& m)
	{
		D3DXVECTOR3 x_dir(0.0,0.0,1.0),y_dir;
		float d=dir.z;

		// To avoid problems with normalize in special cases
		if(d>-0.999999999 && d<0.999999999)
		{ 
			x_dir=x_dir-dir*d;
			D3DXVec3Normalize(&x_dir,&x_dir);
			D3DXVec3Cross(&y_dir, &dir, &x_dir);
		}
		else
		{
			x_dir=D3DXVECTOR3(dir.z,0,-dir.x);
			y_dir=D3DXVECTOR3(0,1,0);
		}


		m.m[0][0]=x_dir.x;
		m.m[0][1]=x_dir.y;
		m.m[0][2]=x_dir.z;
		m.m[0][3]=0.0;

		m.m[1][0]=y_dir.x;
		m.m[1][1]=y_dir.y;
		m.m[1][2]=y_dir.z;
		m.m[1][3]=0.0;

		m.m[2][0]=dir.x;
		m.m[2][1]=dir.y;
		m.m[2][2]=dir.z;
		m.m[2][3]=0.0;	

		m.m[3][0]=0;
		m.m[3][1]=0;
		m.m[3][2]=0;
		m.m[3][3]=1.0;
	}


	//--------------------------------------------------------------------------------------
	// Check if a sphere intersects an AABB
	//--------------------------------------------------------------------------------------
	bool Math::SphereIntersectAABB(const D3DXVECTOR3& posSphere, float radius, const D3DXVECTOR3& posBox, const D3DXVECTOR3& bounds )
	{
		// Check the 8 points of the box against the sphere volume
		D3DXVECTOR3 r;
		D3DXVECTOR3 points[4];
		points[0] = posBox + D3DXVECTOR3(bounds.x, bounds.y, bounds.z);
		points[1] = posBox + D3DXVECTOR3(-bounds.x, bounds.y, bounds.z);
		points[2] = posBox + D3DXVECTOR3(-bounds.x, bounds.y, -bounds.z);
		points[3] = posBox + D3DXVECTOR3(bounds.x, bounds.y, -bounds.z);

		r = posSphere + (posBox+points[0]);
		if(D3DXVec3Length(&r) <= radius)
			return true;
		r = posSphere - (posBox+points[0]);
		if(D3DXVec3Length(&r) <= radius)
			return true;
		r = posSphere + (posBox+points[1]);
		if(D3DXVec3Length(&r) <= radius)
			return true;
		r = posSphere - (posBox+points[1]);
		if(D3DXVec3Length(&r) <= radius)
			return true;
		r = posSphere + (posBox+points[2]);
		if(D3DXVec3Length(&r) <= radius)
			return true;
		r = posSphere - (posBox+points[2]);
		if(D3DXVec3Length(&r) <= radius)
			return true;
		r = posSphere + (posBox+points[3]);
		if(D3DXVec3Length(&r) <= radius)
			return true;
		r = posSphere - (posBox+points[3]);
		if(D3DXVec3Length(&r) <= radius)
			return true;
		return false;
	}

}