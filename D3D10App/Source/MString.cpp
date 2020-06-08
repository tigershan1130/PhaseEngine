//--------------------------------------------------------------------------------------
// File: String.cpp
//
// String class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "MString.h"
#include <math.h>

#include "Stack.cpp"

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Default constructor
	//--------------------------------------------------------------------------------------
	String::String()
	{
		m_Str[0] = NULL;
		m_iLength=0;
		m_iFloatPrecision=4;
	}


	//--------------------------------------------------------------------------------------
	// Constructor from a char array
	//--------------------------------------------------------------------------------------
	String::String( const char* str )
	{
		m_Str[0] = NULL;
		m_iLength = 0;
		m_iFloatPrecision = 4;
		*this = str;
	}

	//--------------------------------------------------------------------------------------
	// Constructor from another String
	//--------------------------------------------------------------------------------------
	String::String( const String& str )
	{
		m_Str[0] = NULL;
		m_iLength = 0;
		m_iFloatPrecision = 4;
		*this = str;
	}

	//--------------------------------------------------------------------------------------
	// Destructor
	//--------------------------------------------------------------------------------------
	String::~String()
	{
		m_Str[0] = NULL;
	}




	//--------------------------------------------------------------------------------------
	// Returns a substring
	//--------------------------------------------------------------------------------------
	String String::Substring(int s, int e)
	{ 
		// Make a string object and copy the char array into it
		String sz;
		sz.m_iFloatPrecision = m_iFloatPrecision;
		sz.m_iLength = e-s+1;
		memcpy(sz.m_Str, m_Str+s, e-s);
		sz.m_Str[sz.m_iLength-1] = NULL;
		return sz;    
	}

	//--------------------------------------------------------------------------------------
	// Chops off the working directory
	//--------------------------------------------------------------------------------------
	String String::ChopWorkingDirectory(String& dir)
	{
		int s=0;
		int e=m_iLength;
		while(m_Str[s] == dir[s])
			s++;
		return Substring(s, e);
	}


	//--------------------------------------------------------------------------------------
	// Gets the directory of a file
	//--------------------------------------------------------------------------------------
	String String::GetDirectory()
	{
		int len = strlen(m_Str);
		int i=len-1;
		while( m_Str[i] != '\\' && i>0 )
			i--;
		return Substring(0,i+1);
	}

	String String::GetPreviousDirectory()
	{
		int len = strlen(m_Str);
		int i=len-2;
		while( m_Str[i] != '\\' && i>0 )
			i--;
		return Substring(0,i+1);
	}

	String String::GetFile()
	{
		int len = strlen(m_Str);
		int i=len-1;
		while( m_Str[i] != '\\' && i>0 )
			i--;
		return Substring(i+1,len);
	}

	// Chops off the main directory
	String String::ChopDirectory()
	{
		int i=0;
		while(i<m_iLength && g_szDirectory[i]==m_Str[i])
			i++;
		return Substring(i, m_iLength-1);
	}

}