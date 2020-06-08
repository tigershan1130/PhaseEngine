//--------------------------------------------------------------------------------------
// File: Util.h
//
// Some useful functions
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "stdafx.h"
#include "Util.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Helper functions for querying information about the processors in the current
	// system.  ( Copied from the doc page for GetLogicalProcessorInformation() )
	//--------------------------------------------------------------------------------------

	//  Helper function to count bits in the processor mask
	DWORD Util::CountBits(ULONG_PTR bitMask)
	{
		DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
		DWORD bitSetCount = 0;
		DWORD bitTest = 1 << LSHIFT;
		DWORD i;

		for( i = 0; i <= LSHIFT; ++i)
		{
			bitSetCount += ((bitMask & bitTest)?1:0);
			bitTest/=2;
		}

		return bitSetCount;
	}

	int Util::GetPhysicalProcessorCount()
	{
		DWORD procCoreCount = 0;    // Return 0 on any failure.  That'll show them.

		LPFN_GLPI Glpi;

		Glpi = (LPFN_GLPI) GetProcAddress(
			GetModuleHandle(TEXT("kernel32")),
			"GetLogicalProcessorInformation");
		if (NULL == Glpi) 
		{
			// GetLogicalProcessorInformation is not supported
			return procCoreCount;
		}

		BOOL done = FALSE;
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
		DWORD returnLength = 0;

		while (!done) 
		{
			DWORD rc = Glpi(buffer, &returnLength);

			if (FALSE == rc) 
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
				{
					if (buffer) 
						free(buffer);

					buffer=(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
						returnLength);

					if (NULL == buffer) 
					{
						// Allocation failure\n
						return procCoreCount;
					}
				} 
				else 
				{
					// Unanticipated error
					return procCoreCount;
				}
			} 
			else done = TRUE;
		}

		DWORD byteOffset = 0;
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
		while (byteOffset < returnLength) 
		{
			if (ptr->Relationship == RelationProcessorCore) 
			{
				if(ptr->ProcessorCore.Flags)
				{
					//  Hyperthreading or SMT is enabled.
					//  Logical processors are on the same core.
					procCoreCount += 1;
				}
				else
				{
					//  Logical processors are on different cores.
					procCoreCount += CountBits(ptr->ProcessorMask);
				}
			}
			byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			ptr++;
		}

		free (buffer);

		return procCoreCount;
	}
	
	
	
	//--------------------------------------------------------------------------------------
	// Gets the number or characters needed to put an int in a char array
	//--------------------------------------------------------------------------------------
	int Util::GetNumIntDigits( int i )
	{
		if(i==0)
			return 1;
		int num=i;
		int digits=0;
		if(i<0)
			digits=1;
		while(num!=0)
		{
			num/=10;
			digits++;
		}
		return digits;
	}

	//--------------------------------------------------------------------------------------
	// Gets the number or characters needed to put an float in a char array
	//--------------------------------------------------------------------------------------
	int Util::GetNumFloatDigits( float f, int p)
	{
		float num = fabs(f);
		int digits=0;
		if(f<0)
			digits=1;
		float iTen = 1.0f/10.0f;
		while(num>1.0f)
		{
			num*=iTen;
			digits++;
		}
		digits++;
		while(num!=0.000000f && p>0)
		{
			num*=iTen;
			digits++;
			p--;
		}
		return digits;
	}

}