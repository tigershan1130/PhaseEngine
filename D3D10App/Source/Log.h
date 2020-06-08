//--------------------------------------------------------------------------------------
// File: Log.h
//
// Event logging
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include <stdio.h>
#include "d3dx9.h"

// Ignore deprecated warnings
#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

namespace Core 
{
	// Debug logging class
	class Log
	{
	public:

		// Opens the log file
		static void Init();

		// Logs a fatal error and exits the app
		static void FatalError(const char* err);

		// Logs an error
		static void Error(const char* err, ...);
		
		// Prints a string to the log file
		static void Print(const char* str, ...);

		// Prints a vector to the log file
		static void Print(D3DXVECTOR3& vec);
		static void Print(D3DXVECTOR2& vec);

  	    // Outputs the result of a D3D10 error code
		static void D3D10Error(HRESULT hr);

	
	private:
		
		// The log file
		static FILE* m_pErrLog;

		// Reads a line from a file into a string, ignoring empty lines
		static void readstr(FILE* f, char* buf);
	};
}


