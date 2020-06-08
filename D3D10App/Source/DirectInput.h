//--------------------------------------------------------------------------------------
// File: Input.h
//
// Mouse/Keyboard input using DirectInput
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include <dinput.h>

namespace Core
{
	// Creates the directinput object
	inline LPDIRECTINPUT8 CreateDI(){ 
		LPDIRECTINPUT8 di; 
		DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&di, NULL); 
		return di; 
	}

	// Gets a DirectInput keyboard
	inline LPDIRECTINPUTDEVICE8 CreateKeyboard(LPDIRECTINPUT8 di, HWND hWnd){
		LPDIRECTINPUTDEVICE8 device;
		if (FAILED(di->CreateDevice(GUID_SysKeyboard, &device, NULL)))
			return NULL;
		if (FAILED(device->SetDataFormat(&c_dfDIKeyboard)))
			return NULL;
		if (FAILED(device->SetCooperativeLevel(hWnd, DISCL_BACKGROUND |
			DISCL_NONEXCLUSIVE)))
			return NULL;
		if (FAILED(device->Acquire()))
			return NULL;
		return device;
	}

	// Gets a DirectInput mouse
	inline LPDIRECTINPUTDEVICE8 CreateMouse(LPDIRECTINPUT8 di, HWND hWnd){
		LPDIRECTINPUTDEVICE8 device;
		if (FAILED(di->CreateDevice(GUID_SysMouse, &device, NULL)))
			return NULL;
		if (FAILED(device->SetDataFormat(&c_dfDIMouse)))
			return NULL;
		if (FAILED(device->SetCooperativeLevel(hWnd, DISCL_BACKGROUND |
			DISCL_NONEXCLUSIVE)))
			return NULL;
		if (FAILED(device->Acquire()))
			return NULL;
		return device;
	}
	
	
	// Wraps MeshObject
	class Input
	{
	public:

		// Sets up DirectInput
		static bool Init(HWND hWnd);

		// Updates the DI state objects
		static inline void Update()
		{
			m_Keyboard->GetDeviceState(sizeof(UCHAR[256]), (LPVOID)m_pKeyState);
			m_Mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)m_pMouseState);
		}

		// Frees up DirectInput
		static void Release();


		// Key press
		static inline bool  KeyUp(DWORD n){ return ((m_pKeyState[n] & 0x80) ? false : true); }
		static inline bool KeyDown(DWORD key)
		{
			if(!m_bKeyboardBusy)
				return (m_pKeyState[key]!=0);
			return false;
		}

		// Mouse
		static int MouseDeltaX(){ return m_pMouseState->lX; }
		static int MouseDeltaY(){ return m_pMouseState->lY; }
		static int MouseWheelDelta(){ return m_pMouseState->lZ; }
		static bool MouseRightClick(){ return (m_pMouseState->rgbButtons[1]!=0); }
		static bool MouseLeftClick(){ return (m_pMouseState->rgbButtons[0]!=0); }

	protected:
		static LPDIRECTINPUT8         m_pDI;			// The DI interface
		static LPDIRECTINPUTDEVICE8   m_Keyboard;		// The keyboard
		static LPDIRECTINPUTDEVICE8   m_Mouse;			// The mouse
		static UCHAR*				  m_pKeyState;		// Stores the keyboard data
		static DIMOUSESTATE*		  m_pMouseState;	// Stores the mouse data
		static bool					  m_bKeyboardBusy;	// True if the keyboard cant be accessed
	};
}