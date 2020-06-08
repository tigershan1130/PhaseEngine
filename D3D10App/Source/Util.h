//--------------------------------------------------------------------------------------
// File: Util.h
//
// Some useful functions
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include <process.h>

namespace Core 
{
	// Some useful functions
	class Util
	{
	public:

		//--------------------------------------------------------------------------------------
		// Helper functions for querying information about the processors in the current
		// system.  ( Copied from the doc page for GetLogicalProcessorInformation() )
		//--------------------------------------------------------------------------------------
		typedef BOOL (WINAPI *LPFN_GLPI)(
			PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, 
			PDWORD);

		static DWORD CountBits(ULONG_PTR bitMask);
		static int GetPhysicalProcessorCount();
		
		
		// Gets the number or characters needed to put an int in a char array
		static int GetNumIntDigits( int i );

		// Gets the number or characters needed to put an float in a char array
		static int GetNumFloatDigits( float f, int p);

	};

}