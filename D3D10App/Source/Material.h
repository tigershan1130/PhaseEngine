//--------------------------------------------------------------------------------------
// File: Material.h
//
// Renderable surface properties
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "Array.cpp"
#include "Texture.h"
#include "MessageHandler.h"

#include <iostream>
#include <fstream>

namespace Core
{

	// Texture types
	enum TEX_TYPE : int
	{
		TEX_DIFFUSE1=0,
		TEX_DIFFUSE2,
		TEX_NORMAL,
		TEX_ALPHA,
		TEX_GLOSS,
		TEX_CSM,
		TEX_SIZE,	// Only used for number of texture types
	};

	// Normal mapping types
	enum BUMP_TYPE : int
	{
		BUMP_NONE=0,
		BUMP_NORMAL,
		BUMP_FAKE_PARALLAX,
		BUMP_CSM,
	};

	// Shading models
	enum SHADE_TYPE : int
	{
		SHADE_PHONG = 0,
		SHADE_COOK_TORRANCE,
		SHADE_OREN_NAYAR,
		SHADE_STRAUSS,
		SHADE_WARD,
		SHADE_ASHIKHMIN_SHIRLEY,
	};
	
	//--------------------------------------------------------------------------------------
	// Surface properties
	//--------------------------------------------------------------------------------------
	class Material
	{
	friend class Terrain;
	friend class Renderer;
	friend class MaterialManager;
	private:
		D3DXVECTOR4 Diffuse;				// Diffuse lighting component
		float		CSMScale;				// Bump height scale
		int			MaxCSMSamples;			// Max iterations for csm
		BUMP_TYPE   BumpType;				// Bumpmapping type
		SHADE_TYPE  ShadingModel;			// The BRDF
		bool		TwoSidedTransparency;	// If true, both front and backfaces are taken into account for transparency/refraction
		
		
		// These are used in the encoded texture
		//
		D3DXVECTOR4	Specular;			// Specular lighting component
		D3DXVECTOR4 Emissive;			// Emissive lighting component (glow)
		D3DXVECTOR4 SurfaceParams;		// Parameters for the shading models
		float		Transparency;		// Transparency factor (0.0-1.0)	
		float		Refractivity;		// Refractivity amount (0.0-1.0)	
		float		Reflectivity;		// Reflectivity amount (0.0-1.0)	
		float		RefractionIndex;	// Index of refraction
		//---

		// Update the material texture
		inline void Update(){ MessageHandler::SendMessage(MSG_MATERIAL_UPDATE, NULL); }

		Texture*					pTex[TEX_SIZE];		// Texture array	
		ID3D10ShaderResourceView*	pTexSRV[TEX_SIZE];	// SRV pointer array
		BOOL						pTexFlag[TEX_SIZE]; // Texture flag aray	


		String	    szName;				// Material name		
		

	public:	
		
		inline void EnableTwoSidedTransparency(bool enabled){ TwoSidedTransparency=enabled; }
		inline bool IsTwoSidedTransparent(){ return TwoSidedTransparency; }

		// Get/Set
		inline D3DXVECTOR4& GetSurfaceParams(){ return SurfaceParams; }
		inline float GetSurfaceParam1(){ return SurfaceParams.x; }
		inline void SetSurfaceParam1(float f){ SurfaceParams.x = f; Update(); }
		inline float GetSurfaceParam2(){ return SurfaceParams.y; }
		inline void SetSurfaceParam2(float f){ SurfaceParams.y = f; Update(); }
		inline float GetSurfaceParam3(){ return SurfaceParams.z; }
		inline void SetSurfaceParam3(float f){ SurfaceParams.z = f; Update(); }
		inline float GetSurfaceParam4(){ return SurfaceParams.w; }
		inline void SetSurfaceParam4(float f){ SurfaceParams.w = f; Update(); }

		inline SHADE_TYPE GetShadingModel(){ return ShadingModel; }
		inline void SetShadingModel(SHADE_TYPE type){ ShadingModel = type; Update(); }
		
		inline BUMP_TYPE GetBumpType(){ return BumpType; }
		inline void SetBumpType(BUMP_TYPE type){ 
			BumpType = type; 
			if(BumpType!=BUMP_CSM){
				pTexFlag[TEX_CSM] = false;
				pTexSRV[TEX_CSM] = NULL;
			}
			else if(pTex[TEX_CSM]!=NULL){
				pTexFlag[TEX_CSM] = true;
				pTexSRV[TEX_CSM] = pTex[TEX_CSM]->GetResource();
			}
		}

		inline float GetTransparency(){ return Transparency; }
		inline void SetTransparency(float f){ Transparency = f; Update(); }

		inline float GetRefractionIndex(){ return RefractionIndex; }
		inline void SetRefractionIndex(float f){ RefractionIndex = f; Update(); }
		
		inline float GetRefractivity(){ return Refractivity; }
		inline void SetRefractivity(float f){ Refractivity = f; Update(); }

		inline float GetReflectivity(){ return Reflectivity; }
		inline void SetReflectivity(float f){ Reflectivity = f; Update(); }

		inline float GetCSMScale(){ return CSMScale; }
		inline void SetCSMScale(float f){ CSMScale = f; }

		inline int GetMaxCSMSamples(){ return MaxCSMSamples; }
		inline void SetMaxCSMSamples(int i){ MaxCSMSamples = i; }

		inline D3DXVECTOR4& GetDiffuse(){ return Diffuse; }
		inline void SetDiffuse(D3DXVECTOR4& vec){ Diffuse=vec; }

		inline D3DXVECTOR4& GetSpecular(){ return Specular; }
		inline void SetSpecular(D3DXVECTOR4& vec){ Specular.x=vec.x; Specular.y=vec.y; Specular.z=vec.z; Update(); }

		inline D3DXVECTOR4& GetEmissive(){ return Emissive; }		
		inline void SetEmissive(D3DXVECTOR4& vec){ Emissive=vec; Update(); }

		inline String& GetName(){ return szName; }
		inline void SetName(char* str){ szName=str; }

		UINT ID;	// Index into the material list

		// Sets a texture
		inline void SetTexture(Texture* pNewTex, TEX_TYPE type)
		{
			pTex[type] = pNewTex;
			pTexSRV[type] = pNewTex->GetResource();
			pTexFlag[type] = true;
		}
		inline void SetTextureSRV(ID3D10ShaderResourceView* pNewTex, TEX_TYPE type)
		{
			pTexSRV[type] = pNewTex;
			pTexFlag[type] = true;
		}

		// Gets a texture
		inline Texture* GetTexture(TEX_TYPE type)
		{
			return pTex[type];
		}
		
		// Clear the texture
		void ClearTexture(TEX_TYPE type);

		// Full arrays
		inline BOOL* GetTextureFlags(){ return pTexFlag; }
		inline Texture** GetTextures(){ return pTex; }
		inline ID3D10ShaderResourceView** GetTextureResources(){ return pTexSRV; }
		
		
		// Texture loading
		Texture* LoadTexture(const char* file, TEX_TYPE type);
		
		// Load a texture set from the game pack
		bool LoadTextureSet(const char* file);

		// Transparent
		inline bool IsTransparent(){ return (Transparency>0.0f); }
		inline bool IsReflective(){ return (Reflectivity>0.0f); }
		inline bool IsRefractive(){ return (Refractivity>0.0f); }
				
		// Constructor
		Material();

		// Releases the material
		void Release();	

		// File I/O
		void Import(std::ifstream& file);
		void Export(std::ofstream& file);
	};



	//--------------------------------------------------------------------------------------
	// Manages a set of materials
	//--------------------------------------------------------------------------------------
	class MaterialManager
	{
	public:
			Material* Add(const char* str);
			void Remove(Material* pMat);
			inline Array<Material*>& GetList(){ return m_Materials; }
			inline Material* GetDefault(){ return &m_Default; }
			Material* Get( const char* name );
			void Release();

	protected:

			Array<Material*>	m_Materials;
			Material			m_Default;
	};

	// Global material manager
	extern MaterialManager g_Materials;

}
