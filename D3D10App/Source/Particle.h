//--------------------------------------------------------------------------------------
// File: Particle.h
//
// Particle systems
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "Array.cpp"
#include "Stack.cpp"
#include "LinkedList.cpp"
#include "Effect.h"
#include "Camera.h"

namespace Core
{

#define INTERNAL_MAX_INSTANCES 1024
#define INTERNAL_MAX_BUFFERS 100

	// Basic particle data
	struct Particle
	{
		D3DXVECTOR3 Position;
		D3DXVECTOR3 Velocity;
		D3DXVECTOR4 Color;
		float Age;
	};


	class ParticleEmitter
	{
	public:
		
		ParticleEmitter(){
			m_Color = D3DXVECTOR4(1,1,1,1); 
			m_Position = D3DXVECTOR3(0,0,0);
			SetRotation(D3DXVECTOR3(0,0,0));
			m_Callback = NULL;
			m_IsActive = false;
		}
		

		// Creates new emitter
		void Create(char* texFile, int maxParticles, int emitFrequency, float particleLife, Effect& effect, Camera& camera);

		// Updates the system
		void Update(float elapsedTime);

		// Spawns a particle
		void SpawnParticle(D3DXVECTOR3& pos, D3DXVECTOR3& vel, D3DXVECTOR4& color);

		// Sets the color
		inline void SetParticleColor(D3DXVECTOR4& color){ m_Color = color; }

		// Set the position
		inline void SetPosition(D3DXVECTOR3& pos){ m_Position = pos; }

		// Set the direction
		inline void SetRotation(D3DXVECTOR3& dir){ D3DXMatrixRotationYawPitchRoll(&m_matRotation, dir.x, dir.y, dir.z); }

		// Set the simulation callback function
		inline void SetCallback(void (*foo)(Particle&, const float&)){ m_Callback=foo; }

		// Change status
		inline void Activate(bool enable){ m_IsActive=enable; }

		// Frees mem
		void Release();

	private:		
		Array<Particle>		  m_Particles;			// Particle memory block
		LinkedList<Particle*> m_ActiveParticles;	// Active particles in the scene
		Stack<Particle*>	  m_Pool;				// Pool of reserve particles
		Effect*				  m_pEffect;			// The effect to use for drawing
		Texture*			  m_Texture;			// Texture for billboards
		Camera*				  m_pCamera;			// Camera for billboarding
		int					  m_EmitFrequency;		// Number of particles emitted per frame
		D3DXVECTOR4			  m_Color;				// Particle color
		D3DXVECTOR3			  m_Position;			// Emitter position
		D3DXMATRIX			  m_matRotation;		// Rotation matrix for velocity direction
		bool				  m_IsActive;			// True for processing

		// Properties
		float		m_Lifetime;		// Time for a particle to live, in seconds

		// Simulation callback function
		void (*m_Callback)(Particle&, const float&);

		////////////////////////////
		// Instancing

		struct InstanceData
		{
			D3DXVECTOR4 world1;			// the world transform for this matrix row 1
			D3DXVECTOR4 world2;			// the world transform for this matrix row 2
			D3DXVECTOR4 world3;			// the world transform for this matrix row 3 (row 4 is implicit)
			D3DXVECTOR4 color;
		};

		// Encode a world matrix into the instance buffer
		void SetWorldMatrix(int instance, D3DXMATRIX &mWorld);

		// Encode a color into the instance buffer
		void SetColor(int instance, D3DXVECTOR4& color);

		InstanceData					m_instanceData[INTERNAL_MAX_BUFFERS*INTERNAL_MAX_INSTANCES];
		D3DXMATRIX						m_initialMatrix[INTERNAL_MAX_BUFFERS*INTERNAL_MAX_INSTANCES];
		ID3D10Buffer*					m_pInstanceDataBuffer[INTERNAL_MAX_BUFFERS];
	};
}