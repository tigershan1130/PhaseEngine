//--------------------------------------------------------------------------------------
// File: Particle.h
//
// Particle systems
//
// Coded by Nate Orr 2009
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Particle.h"
#include "Device.h"

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Creates new emitter
	//--------------------------------------------------------------------------------------
	void ParticleEmitter::Create(char* texFile, int maxParticles, int emitFrequency, float particleLife, Effect& effect, Camera& camera)
	{
		// Zero out the instance buffer
		memset(m_instanceData,0,INTERNAL_MAX_BUFFERS*INTERNAL_MAX_INSTANCES*sizeof(InstanceData));
		
		// Create the particle memory block
		m_Particles.Allocate(maxParticles);
		for(int i=0; i<maxParticles; i++)
			m_Pool.Push(&m_Particles[i]);
		
		
		// Setup
		m_pEffect = &effect;
		m_pCamera = &camera;
		m_Texture = g_Textures.Load(texFile);
		m_EmitFrequency = emitFrequency;
		m_Lifetime = particleLife;



		// Create a	resource with the input	matrices
		D3D10_BUFFER_DESC bufferDesc =
		{
			INTERNAL_MAX_INSTANCES * sizeof( InstanceData ),
			D3D10_USAGE_DEFAULT,
			D3D10_BIND_CONSTANT_BUFFER,
			0,
			0
		};

		// Make a series of small buffers to be used for constants based instancing.
		for(int i=0;i<INTERNAL_MAX_BUFFERS;i++)
			g_pd3dDevice->CreateBuffer( &bufferDesc, NULL, &m_pInstanceDataBuffer[i] );

		m_IsActive = true;
	}


	//--------------------------------------------------------------------------------------
	// Encode a world matrix into the instance buffer
	//--------------------------------------------------------------------------------------
	void ParticleEmitter::SetWorldMatrix(int instance, D3DXMATRIX &mWorld)
	{
		m_instanceData[instance].world1 = D3DXVECTOR4(mWorld._11,mWorld._12,mWorld._13,mWorld._41);
		m_instanceData[instance].world2 = D3DXVECTOR4(mWorld._21,mWorld._22,mWorld._23,mWorld._42);
		m_instanceData[instance].world3 = D3DXVECTOR4(mWorld._31,mWorld._32,mWorld._33,mWorld._43);
	}


	//--------------------------------------------------------------------------------------
	// Encode a color into the instance buffer
	//--------------------------------------------------------------------------------------
	void ParticleEmitter::SetColor(int instance, D3DXVECTOR4& color)
	{
		m_instanceData[instance].color = color;
	}



	//--------------------------------------------------------------------------------------
	// Spawns a particle
	//--------------------------------------------------------------------------------------
	void ParticleEmitter::SpawnParticle(D3DXVECTOR3& pos, D3DXVECTOR3& vel, D3DXVECTOR4& color)
	{
		if(!m_Pool.IsEmpty() && m_ActiveParticles.Length()<INTERNAL_MAX_INSTANCES*INTERNAL_MAX_BUFFERS)
		{
			Particle* p = m_Pool.Pop();
			p->Position = pos+m_Position;
			D3DXVECTOR4 newVel;
			D3DXVec3Transform(&newVel, &vel, &m_matRotation);
			p->Velocity.x = newVel.x; p->Velocity.y = newVel.y; p->Velocity.z = newVel.z;
			p->Age = 0;
			p->Color = color;
			m_ActiveParticles.Add(p);
		}
	}

	//--------------------------------------------------------------------------------------
	// Updates the system
	//--------------------------------------------------------------------------------------
	void ParticleEmitter::Update(float elapsedTime)
	{
		if(!m_Callback)
		{
			MessageBoxA(NULL, "You must provide a particle simulation callback function.  Use ParticleEmitter::SetCallback()", "Fatal Error", MB_OK);
			PostQuitMessage(-1);
			return;
		}
		if(!m_IsActive)
			return;
		
		// Create the billboard matrix
		// Billboard matrix
		D3DXMATRIX mWorld, mR, mT;
		D3DXMatrixRotationYawPitchRoll(&mR, m_pCamera->GetYDeg(), m_pCamera->GetXDeg(), 0);

		// Set the texture
		ID3D10ShaderResourceView* pSRV[] = {
			m_Texture->GetResource(),
			NULL,
			NULL,
			m_Texture->GetResource(),
			NULL,
			NULL,
		};
		BOOL pFlag[] = {true, false, false, true, false, false, };
		m_pEffect->MaterialTextureFlagVariable->SetBoolArray(pFlag, 0, 6);
		m_pEffect->MaterialTextureVariable->SetResourceArray(pSRV, 0, 6);
		m_pEffect->MaterialDiffuseVariable->SetFloatVector((float*)&D3DXVECTOR4(1, 1, 1, 1));

		// PURELY FOR TESTING, SHOULD BE SCRIPTED LATER
		for(int i=0; i<m_EmitFrequency*g_TimeScale; i++)
			SpawnParticle(D3DXVECTOR3(0,0,0), D3DXVECTOR3(rand()%50-rand()%50, rand()%350, rand()%50-rand()%50) / 100.0f, m_Color);
		
		// Update each particle
		m_ActiveParticles.Itterate();
		Particle* p;
		int index=0;
		while( (p=m_ActiveParticles.GetCurrent()) != NULL)
		{
			// Run the simulation callback function
			(*m_Callback)(*p, elapsedTime);

			// Process and render
			if(p->Age>m_Lifetime)
			{
				m_ActiveParticles.RemoveCurrent();
				m_Pool.Push(p);
			}
			else
			{	
				// Draw the particle
				D3DXMatrixTranslation(&mT, p->Position.x, p->Position.y, p->Position.z);
				D3DXMatrixMultiply(&mWorld, &mR, &mT);
				SetWorldMatrix(index, mWorld);
				SetColor(index, p->Color);
				m_ActiveParticles.StepForward();
				index++;
			}			
		}

		// Fill the buffers
		unsigned int buffersUsed = (m_ActiveParticles.Length() / INTERNAL_MAX_INSTANCES)+1;
		for(unsigned int buffer = 0;buffer < buffersUsed ; buffer++)
		{

			D3D10_BUFFER_DESC bDesc;
			m_pInstanceDataBuffer[buffer]->GetDesc(&bDesc);
			assert(bDesc.ByteWidth == INTERNAL_MAX_INSTANCES*sizeof(InstanceData));

			g_pd3dDevice->UpdateSubresource(m_pInstanceDataBuffer[buffer],D3D10CalcSubresource(0,0,1),NULL,(void*)((byte*)m_instanceData + buffer*bDesc.ByteWidth),0,0);
		}


		ID3D10Buffer *pEffectConstantBuffer = NULL;
		m_pEffect->InstanceConstantBuffer->GetConstantBuffer(&pEffectConstantBuffer);

		// Draw the instances
		for(unsigned int buffer = 0;buffer < buffersUsed ; buffer++)
		{
			int numInstances = INTERNAL_MAX_INSTANCES;
			if(buffer == buffersUsed - 1)       // restrict drawing all instances in the final buffer to get an exact count
				numInstances = m_ActiveParticles.Length() % INTERNAL_MAX_INSTANCES;

			D3D10_BUFFER_DESC srcDesc;
			D3D10_BUFFER_DESC dstDesc;
			m_pInstanceDataBuffer[buffer]->GetDesc(&srcDesc);
			pEffectConstantBuffer->GetDesc(&dstDesc);
			g_pd3dDevice->CopyResource(pEffectConstantBuffer,m_pInstanceDataBuffer[buffer]);

			// Draw
			m_pEffect->Pass[PASS_INSTANCED]->Apply(0);
			g_pd3dDevice->DrawInstanced(6,numInstances,0,0);
			Device::FrameStats.DrawCalls++;
		}
		Device::FrameStats.PolysDrawn += 2*m_ActiveParticles.Length();
		SAFE_RELEASE(pEffectConstantBuffer);
	}


	//--------------------------------------------------------------------------------------
	// Frees mem
	//--------------------------------------------------------------------------------------
	void ParticleEmitter::Release()
	{
		m_Particles.Release();
		m_Pool.Release();
		m_ActiveParticles.Release();
		g_Textures.Deref(m_Texture);

		for(int i=0;i<INTERNAL_MAX_BUFFERS;i++)
			SAFE_RELEASE(m_pInstanceDataBuffer[i]);
	}
}