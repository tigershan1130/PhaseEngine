//--------------------------------------------------------------------------------------
// File: Material.cpp
//
// Renderable surface properties
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "Material.h"

namespace Core
{

	// Global material manager
	MaterialManager g_Materials;


	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	Material::Material()
	{
		for(int i=0; i<TEX_SIZE; i++)
		{
			pTex[i] = NULL;
			pTexSRV[i] = NULL;
			pTexFlag[i] = FALSE;
		}
		Transparency = Reflectivity = Refractivity = 0.0f;
		RefractionIndex = 1.51714f; // (glass)
		Diffuse = D3DXVECTOR4(1,1,1,1);
		Emissive = D3DXVECTOR4(0,0,0,0);
		SurfaceParams  = D3DXVECTOR4(0,0,0,0);
		Specular = Diffuse*0.1f;
		SurfaceParams.x = 16;
		CSMScale = 0.06f;
		MaxCSMSamples = 30;
		BumpType = BUMP_NONE;
		ShadingModel = SHADE_PHONG;
		szName = "Material";
		TwoSidedTransparency = false;
		ID=0;
	}

	//--------------------------------------------------------------------------------------
	// Clear the texture
	//--------------------------------------------------------------------------------------
	void Material::ClearTexture(TEX_TYPE type)
	{
		if(pTexFlag[type])
		{
			g_Textures.Deref(pTex[type]);
			pTex[type] = NULL;
			pTexSRV[type] = NULL;
			pTexFlag[type] = false;
		}
	}

	//--------------------------------------------------------------------------------------
	// Texture loading
	//--------------------------------------------------------------------------------------
	Texture* Material::LoadTexture(const char* file, TEX_TYPE type)
	{
		// Clear any old texture
		ClearTexture(type);

		// Load the texture
		pTex[type] = g_Textures.Load(file);
		if(pTex[type])
		{
			pTexFlag[type] = TRUE;
			pTexSRV[type] = pTex[type]->GetResource();
			return pTex[type];
		}
		pTexFlag[type] = FALSE;
		pTexSRV[type] = NULL;
		return NULL;
	}


	//--------------------------------------------------------------------------------------
	// Load a texture set from the game pack
	//--------------------------------------------------------------------------------------
	bool Material::LoadTextureSet(const char* file)
	{
		// Get the starting directory and grab the texture name from the file
		String dirT = g_szDirectory + "Textures\\Game Pack\\Textures\\";
		String dirN = g_szDirectory + "Textures\\Game Pack\\Normal Maps\\";
		String dirS = g_szDirectory + "Textures\\Game Pack\\Specular Maps\\";
		String dirA = g_szDirectory + "Textures\\Game Pack\\Alpha Maps\\";
		//String dirB = g_szDirectory + "Textures\\Game Pack\\Cone Step Maps\\";
		String name = GetFileFromPath(String(file));
		String root = RemoveFileExtension(name);

		// Load each texture
		LoadTexture(dirT+name, TEX_DIFFUSE1);
		LoadTexture(dirS+root+"s.jpg", TEX_GLOSS);
		LoadTexture(dirN+root+"n.png", TEX_NORMAL);
		LoadTexture(dirA+root+"a.png", TEX_ALPHA);
		//LoadTexture(dirB+root+"b.jpg", TEX_CSM);
		return true;
	}

	//--------------------------------------------------------------------------------------
	// Releases the material
	//--------------------------------------------------------------------------------------
	void Material::Release()
	{
		for(int i=0; i<TEX_SIZE; i++)
			g_Textures.Deref(pTex[i]);
		Material();
	}

	// Load from file
	void Material::Import(std::ifstream& file)
	{
		// Props
		char buf[256];
		file.getline(buf,255);
		szName = buf;
		file.getline(buf,255);
		sscanf(buf,"%f %f %f %f",&Diffuse.x,&Diffuse.y,&Diffuse.z,&Diffuse.w);
		file.getline(buf,255);
		sscanf(buf,"%f %f %f %f",&Specular.x,&Specular.y,&Specular.z,&Specular.w);
		file.getline(buf,255);
		sscanf(buf,"%f %f %f %f",&Emissive.x,&Emissive.y,&Emissive.z,&Emissive.w);
		file.getline(buf,255);
		int dualTrans;
		sscanf(buf,"%f %f %f %f %d",&Reflectivity,&Refractivity, &RefractionIndex, &Transparency, &dualTrans);
		TwoSidedTransparency = (dualTrans==1);
		file.getline(buf,255);
		sscanf(buf,"%f %f %f %f",&SurfaceParams.x,&SurfaceParams.y,&SurfaceParams.z,&SurfaceParams.w);
		file.getline(buf,255);
		sscanf(buf,"%f %d %d %d %d",&CSMScale, &MaxCSMSamples, &BumpType, &ShadingModel, &ID);
		
		// Texture names
		for(int i=0; i<TEX_SIZE; i++)
		{
			file.getline(buf,255);
			if(strcmp(buf,"NONE")!=0)
				LoadTexture(buf, (TEX_TYPE)i);
		}

		Update();
	}

	// Save to file
	void Material::Export(std::ofstream& file)
	{
		// Props
		file << szName << std::endl;
		file << Diffuse.x << " " << Diffuse.y << " " << Diffuse.z << " " << Diffuse.w << std::endl;
		file << Specular.x << " " << Specular.y << " " << Specular.z << " " << Specular.w << std::endl;
		file << Emissive.x << " " << Emissive.y << " " << Emissive.z << " " << Emissive.w << std::endl;
		file << Reflectivity << " " << Refractivity << " " << RefractionIndex << " " << Transparency << " " << (int)TwoSidedTransparency << std::endl;
		file << SurfaceParams.x << " " << SurfaceParams.y << " " << SurfaceParams.z << " " << SurfaceParams.w << std::endl;
		file << CSMScale << " " << MaxCSMSamples << " " << BumpType << " " << ShadingModel << " " << ID << std::endl;
		
		// Texture names
		for(int i=0; i<TEX_SIZE; i++)
		{
			if(pTex[i])
				file << pTex[i]->GetName().ChopWorkingDirectory(g_szDirectory).c_str() << std::endl;
			else
				file << "NONE" << std::endl;
		}
		file << std::endl;
	}



	//-----------------------------------------------------------------------------
	// Add a new material
	//-----------------------------------------------------------------------------
	Material* MaterialManager::Add(const char* str)
	{
		static int additive=2;
		
		Material* pNewMat = new Material;
		
		// Make sure there isnt another mesh with the same name
		String baseName = str;
		String realName = baseName;
		bool flag = true;
		while(flag)
		{
			flag=false;
			for(int i=0; i<GetList().Size(); i++)
			{
				if(strcmp(realName.c_str(), GetList()[i]->GetName().c_str())==0)
				{
					realName = baseName+additive;
					additive++;
					flag=true;
					break;
				}
			}
		}
		pNewMat->SetName(const_cast<char*>(realName.c_str()));

		pNewMat->ID = m_Materials.Size();
		pNewMat->Update();
		m_Materials.Add(pNewMat);
		
		Log::Print("Material Added> %s", str);
		return pNewMat;
	}

	Material* MaterialManager::Get( const char* name )
	{
		if(strcmp(name,"Material")==0)
			return &m_Default;
		
		for(int i=0; i<m_Materials.Size(); i++)
		{
			if( strcmp( name, m_Materials[i]->GetName() ) == 0 )
				return m_Materials[i];
		}
		return &m_Default;
	}


	//-----------------------------------------------------------------------------
	// Remove and delete a material
	//-----------------------------------------------------------------------------
	void MaterialManager::Remove(Material* pMat)
	{
		if(m_Materials.Remove(pMat))
		{
			pMat->Release();
			delete pMat;
		}

		// Update all IDs
		for(int i=0; i<m_Materials.Size(); i++)
			m_Materials[i]->ID = i;
	}

	//-----------------------------------------------------------------------------
	// Delete all materials
	//-----------------------------------------------------------------------------
	void MaterialManager::Release()
	{
		for(int i=0; i<m_Materials.Size(); i++)
		{
			m_Materials[i]->Release();
			delete m_Materials[i];
		}
		m_Materials.Release();
	}

}