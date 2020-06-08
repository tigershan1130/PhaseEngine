//--------------------------------------------------------------------------------------
// File: Scene.h
//
// Scenes are similar to traditional gamestates, they contain everything needed
// to perform certain rendering, and can be pushed/popped on the renderer
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------
#pragma unmanaged

namespace Core
{
	class Scene
	{
	public:

	private:


		///////////////////////////////////
		// Scene Objects
		Camera					m_Camera;				// Default camera
		Light					m_Sun;					// The sun light
		D3DXVECTOR4				m_Ambient;				// Ambient lighting
		Array<ParticleEmitter*> m_Emitters;				// Particle emitters
		Array<Light*>			m_Lights;				// Scene lights
		Stack<Light*>			m_LightPool;			// Recycle bin for lights
		Array<MeshObject*>		m_MeshObjects;			// BaseMesh objects
		Array<SubMesh*>			m_TransparentObjects;   // Objects with transparency enabled
		Array<SubMesh*>			m_RenderList;			// Sorted list of submeshes to be rendered
		MeshObject				m_SphereMesh;			// Sphere mesh
		MeshObject				m_BoxMesh;				// Box mesh
		MeshObject				m_ConeMesh;				// Cone mesh
		Array<Light*>			m_VisibleLights;		// List of visible lights
		Terrain*				m_pTerrain;				// The terrain
		

	};
}