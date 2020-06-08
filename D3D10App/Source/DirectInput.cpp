//--------------------------------------------------------------------------------------
// File: Input.h
//
// Mouse/Keyboard input using DirectInput
//
// Coded by Nate Orr 2007
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "DirectInput.h"

namespace Core
{

	LPDIRECTINPUT8         Input::m_pDI;			// The DI interface
	LPDIRECTINPUTDEVICE8   Input::m_Keyboard;		// The keyboard
	LPDIRECTINPUTDEVICE8   Input::m_Mouse;			// The mouse
	UCHAR*				   Input::m_pKeyState;		// Stores the keyboard data
	DIMOUSESTATE*		   Input::m_pMouseState;	// Stores the mouse data
	bool				   Input::m_bKeyboardBusy;	// True if the keyboard cant be accessed
	
	//--------------------------------------------------------------------------------------
	// Sets up DirectInput
	//--------------------------------------------------------------------------------------
	bool Input::Init(HWND hWnd)
	{
		m_pDI = CreateDI();
		if(m_pDI==NULL)
			return false;

		// Initialize the keyboard
		m_Keyboard = CreateKeyboard(m_pDI, hWnd);

		// Initialize the mouse
		m_Mouse = CreateMouse(m_pDI, hWnd);

		if(m_Mouse==NULL || m_Keyboard==NULL)
			return false;

		// Allocate the mouse and keyboard state objects
		m_pKeyState = new UCHAR[256];
		m_pMouseState = new DIMOUSESTATE();

		return true;
	}

	//--------------------------------------------------------------------------------------
	// Frees up DirectInput
	//--------------------------------------------------------------------------------------
	void Input::Release()
	{
		if(m_Mouse)
		{
			m_Mouse->Unacquire();
			m_Mouse->Release();
			m_Mouse = NULL;
		}

		if(m_Keyboard)
		{
			m_Keyboard->Unacquire();
			m_Keyboard->Release();
			m_Keyboard = NULL;
		}

		SAFE_RELEASE(m_pDI);
		SAFE_DELETE(m_pMouseState);
		SAFE_DELETE_ARRAY(m_pKeyState);
	}

}