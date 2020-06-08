//--------------------------------------------------------------------------------------
// File: QString.h
//
// String class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "Util.h"
#include "Stack.h"

namespace Core
{
	//--------------------------------------------------------------------------------------
	// Stores a string
	//--------------------------------------------------------------------------------------
	class String
	{
	public:
		String();
		String(const char* str);	
		String(const String& str);
		~String();

		// Copy constructor
		inline String& operator=( const String& str );
		inline String& operator=( const char* str );

		// Comparison
		inline bool operator==( const char* str );
		inline bool operator==( const String& str );

		// Gets a char at location i
		inline char& operator[](int i){ return m_Str[i]; }

		// Concatinations
		inline String operator+( const String& str );
		inline String operator+( const char* str );
		inline String operator+( int i );
		inline String operator+( float f );
		inline String operator+( double d );
		inline String operator+( char c );		
		inline String& operator+=( const String& str );
		inline String& operator+=( const char* str );
		inline String& operator+=( int i );
		inline String& operator+=( float f );
		inline String& operator+=( double d );
		inline String& operator+=( char c );		

		// Get the char array
		inline const char* c_str() const { return m_Str; }

		// Gets the size of the string
		inline int Length(){ return m_iLength; }

		// Sets the floating point precision
		inline void SetFloatPrecision(BYTE i){ m_iFloatPrecision=i; }

		// Automatic type casting
		inline operator const char* () const { return m_Str; }

		// Returns a substring
		String Substring(int s, int e);

		// Chops off the main directory
		String ChopDirectory();

		// Chops off the working directory
		String ChopWorkingDirectory(String& dir);

		// Returns the containing directory
		String GetDirectory();
		String GetPreviousDirectory();
		String GetFile();

	protected:
		static const int m_MaxLength = 256;		// The maximum string size	
		char			 m_Str[m_MaxLength];	// String data
		int				 m_iLength;				// Number of characters + NULL terminator
		BYTE			 m_iFloatPrecision;		// Number of decimal places floats contain
	};


	//--------------------------------------------------------------------------------------
	// Copy constructor from a char array
	//--------------------------------------------------------------------------------------
	inline String& String::operator=( const char* str )
	{
		m_iLength = (int)strlen(str)+1;
		memcpy(m_Str,str,m_iLength-1);
		m_Str[m_iLength-1] = NULL;
		return *this;
	}

	//--------------------------------------------------------------------------------------
	// Copy constructor from another String
	//--------------------------------------------------------------------------------------
	inline String& String::operator=( const String& str )
	{
		*this = str.c_str();;
		m_iFloatPrecision = str.m_iFloatPrecision;
		return *this;
	}

	//--------------------------------------------------------------------------------------
	// Comparison
	//--------------------------------------------------------------------------------------
	inline bool String::operator==( const char* str )
	{
		return strcmp(str, m_Str)==0;
	}
	inline bool String::operator==( const String& str )
	{
		return strcmp(str.c_str(), m_Str)==0;
	}


	//--------------------------------------------------------------------------------------
	// Concats with a char array
	//--------------------------------------------------------------------------------------
	inline String String::operator+( const char* str )
	{
		String newStr(*this);
		return newStr+=str;
	}

	//--------------------------------------------------------------------------------------
	// Concats with another string
	//--------------------------------------------------------------------------------------
	inline String String::operator+( const String& str )
	{
		String newStr(*this);
		return newStr+=str;
	}

	//--------------------------------------------------------------------------------------
	// Concats with an int
	//--------------------------------------------------------------------------------------
	inline String String::operator+( int i )
	{
		String newStr(*this);
		return newStr+=i;
	}

	//--------------------------------------------------------------------------------------
	// Concats with a float
	//--------------------------------------------------------------------------------------
	inline String String::operator+( float f )
	{
		String newStr(*this);
		return newStr+=f;
	}

	//--------------------------------------------------------------------------------------
	// Concats with a double
	//--------------------------------------------------------------------------------------
	inline String String::operator+( double d )
	{
		String newStr(*this);
		return newStr+=d;
	}

	//--------------------------------------------------------------------------------------
	// Concats with a single character
	//--------------------------------------------------------------------------------------
	inline String String::operator+( char c )
	{
		String newStr(*this);
		return newStr+=c;
	}


	//--------------------------------------------------------------------------------------
	// Concats with a char array
	//--------------------------------------------------------------------------------------
	inline String& String::operator+=( const char* str )
	{
		int len = (int)strlen(str);
		int len2 = m_iLength-1;
		memcpy(m_Str+len2,str,len);
		m_iLength += len;
		m_Str[m_iLength-1]=NULL;
		return *this;
	}

	//--------------------------------------------------------------------------------------
	// Concats with another string
	//--------------------------------------------------------------------------------------
	inline String& String::operator+=( const String& anotherStr )
	{
		return *this+=anotherStr.c_str();
	}

	//--------------------------------------------------------------------------------------
	// Concats with an int
	//--------------------------------------------------------------------------------------
	inline String& String::operator+=( int i )
	{
		int numDigits = Util::GetNumIntDigits(i);
		sprintf(m_Str+m_iLength-1,"%d",i);
		m_iLength += numDigits;
		m_Str[m_iLength-1] = NULL;
		return *this;
	}

	//--------------------------------------------------------------------------------------
	// Concats with a float
	//--------------------------------------------------------------------------------------
	extern char szStringClassTemp[64];
	inline String& String::operator+=( float f )
	{
		int numDigits = Util::GetNumFloatDigits(f,m_iFloatPrecision);
		sprintf(m_Str+m_iLength-1,"%f",f);
		m_iLength += numDigits;
		m_Str[m_iLength-1] = NULL;
		return *this;
	}

	//--------------------------------------------------------------------------------------
	// Concats with a double
	//--------------------------------------------------------------------------------------
	inline String& String::operator+=( double f )
	{
		return *this+=(float)f;
	}


	//--------------------------------------------------------------------------------------
	// Concats with an int
	//--------------------------------------------------------------------------------------
	inline String& String::operator+=( char c )
	{
		m_Str[m_iLength-1] = c;
		m_Str[m_iLength] = NULL;
		m_iLength++;
		return *this;
	}


	//--------------------------------------------------------------------------------------
	// Gets the file portion of a path
	//--------------------------------------------------------------------------------------
	inline String GetFileFromPath(String& buf)
	{
		// Get the first /
		for(int i=buf.Length()-1; i>=0; i--)
			if(buf[i] == '\\')
				return buf.Substring(i+1,buf.Length()-1);
		return buf;
	}

	inline String GetFileDirectory(String& buf)
	{
		// Get the first /
		for(int i=buf.Length()-1; i>=0; i--)
			if(buf[i] == '\\')
				return buf.Substring(0, i+1);
		return buf;
	}

	//--------------------------------------------------------------------------------------
	// Removes the .* extension from a string
	//--------------------------------------------------------------------------------------
	inline String RemoveFileExtension(String& str)
	{
		// Get the first .
		for(int i=str.Length()-1; i>=0; i--)
			if(str[i] == '.')
				return str.Substring(0,i);
		return str;
	}
}