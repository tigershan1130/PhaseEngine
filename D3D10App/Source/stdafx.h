//--------------------------------------------------------------------------------------
// File: dxstdafx.h
//
// Std header file
//
// Nate Orr 2007
//--------------------------------------------------------------------------------------
#pragma warning( disable : 4995 )
#pragma warning( disable : 4635 )
#pragma unmanaged

#pragma once

// Comment out for release builds!!!
#define PHASE_DEBUG

#define NOMINMAX

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

// MIN / MAX FUNCTIONS
inline const int& MinInt(const int& a, const int&b){	if(a<b)	return a; return b; }
inline const int& MaxInt(const int& a, const int&b){	if(a>b)	return a; return b; }
inline const double& MinDouble(const double& a, const double&b){	if(a<b)	return a; return b; }
inline const double& MaxDouble(const double& a, const double&b){	if(a>b)	return a; return b; }
inline const float& MinFloat(const float& a, const float&b){	if(a<b)	return a; return b; }
inline const float& MaxFloat(const float& a, const float&b){	if(a>b)	return a; return b; }

// Swaps two object
template<class T>
inline void Swap(T& a, T& b)
{
	T temp = a;
	a=b;
	b=temp;;
}

// String conversion macro
#define StringToChar(x) (char*)(void*)Marshal::StringToHGlobalAnsi(x)

// Vector conversion macro
#define ToManagedVector(x) *((Vector3*)&x)
#define ToUnmanagedVector(x) *((D3DXVECTOR3*)&x)

#ifndef B_RETURN
#define B_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return false; } }
#endif

#ifdef PHASE_DEBUG
	// Debug output macros
	#define WIDEN( w ) WIDEN2( w )
	#define WIDEN2( w )	L ##w
	#define INFO_OUT( text ) OutputDebugString( L"(INFO) : " WIDEN( __FUNCTION__ ) L"() - " text L"\n" )
	#define ERR_OUT( text ) OutputDebugString( L"(ERROR) : " WIDEN( __FUNCTION__ ) L"() - " text L"\n" )
	#define WARN_OUT( text ) OutputDebugString( L"(WARNING) : " WIDEN( __FUNCTION__ ) L"() - " text L"\n" )

	// Validation
	#ifndef VALIDATE
	#define VALIDATE(object, text) if(!object->IsValid()){ Log::Print(text); Log::FatalError("Validation Failed!!"); return E_FAIL; }
	#endif
#endif

// Standard Windows includes
#include <windows.h>
#include <initguid.h>
#include <assert.h>
#include <wchar.h>
#include <mmsystem.h>
#include <commctrl.h> // for InitCommonControls() 
#include <shellapi.h> // for ExtractIcon()
#include <new.h>      // for placement new
#include <shlobj.h>
#include <math.h>      
#include <limits.h>      
#include <stdio.h>

// CRT's memory leak detection
#if defined(DEBUG) || defined(_DEBUG)
#include <crtdbg.h>
#endif

// Direct3D9 includes
#include <d3d9.h>
#include <d3dx9.h>

// Direct3D10 includes
#include <dxgi.h>
#include <d3d10_1.h>
#include <d3d10.h>
#include <d3dx10.h>

// XInput includes
#include <xinput.h>

// HRESULT translation for Direct3D10 and other APIs 
#include <dxerr.h>

#include "MString.h"
#include "Util.h"
#include "Log.h"

#define LOG( msg ) Log::Print("LOG: %s [%s(...) @ line %d]\n",msg, __FUNCTION__, __LINE__);

#if defined(DEBUG) || defined(_DEBUG)
#ifndef V
#define V(x)           { hr = (x); if( FAILED(hr) ) { LOG("HR Fail"); } }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { LOG("HR Fail"); return hr; } }
#endif
#else
#ifndef V
#define V(x)           { hr = (x); }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = (x); if( FAILED(hr) ) { return hr; } }
#endif
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

namespace Core
{

	//--------------------------------------------------------------------------------------
	// An object that can be handled by the resource manager
	//--------------------------------------------------------------------------------------
	class EngineResource
	{
	private:
		long m_refCount;
	protected:
		EngineResource(){ m_refCount = 0; m_szName = "EngineResource"; }
	public:
		inline void AddRef(){ m_refCount++; }
		inline void Deref(){ m_refCount--; }
		inline const char* GetName(){ return m_szName.c_str(); }
		inline bool IsActive(){ return (m_refCount>0); }
		virtual void Release() = 0;
		String m_szName;
	};


	// A NullRef D3D9Device, required for d3dx9 functionality such as .x mesh loading
	extern LPDIRECT3DDEVICE9 g_pd3dDevice9;

	// Global D3D10Device
	extern ID3D10Device* g_pd3dDevice;

	// Main working directory
	extern String g_szDirectory;

	// Time scale
	extern float g_TimeScale;

}

