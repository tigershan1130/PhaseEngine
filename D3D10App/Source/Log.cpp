//--------------------------------------------------------------------------------------
// File: Log.cpp
//
// Event logging
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#include "stdafx.h"
#include "Log.h"


// Ignore deprecated warnings
#pragma warning(disable : 4995)
#pragma warning(disable : 4996)

namespace Core
{

	// The log file
	FILE* Log::m_pErrLog = NULL;

	//--------------------------------------------------------------------------------------
	// Opens the log file
	//--------------------------------------------------------------------------------------
	void Log::Init()
	{
		m_pErrLog = fopen("log.txt","w");
		fprintf(m_pErrLog, "Core Engine 3.0 Debug Log\n\n");
		fclose(m_pErrLog);
	}


	//--------------------------------------------------------------------------------------
	// Logs a fatal error and exits the app
	//--------------------------------------------------------------------------------------
	void Log::FatalError(const char* err)
	{
		m_pErrLog = fopen("log.txt","a");
		fprintf(m_pErrLog,"FATAL ERROR> ");
		fprintf(m_pErrLog,err);
		fprintf(m_pErrLog,"\n");
		fclose(m_pErrLog);	
		PostQuitMessage(-1);
	}

	//--------------------------------------------------------------------------------------
	// Logs an error
	//--------------------------------------------------------------------------------------
	void Log::Error(const char* err, ...)
	{
		va_list arg_list;
		va_start(arg_list, err);
		m_pErrLog = fopen("log.txt","a");
		fprintf(m_pErrLog,"ERROR> ");
		vfprintf(m_pErrLog,err,arg_list);
		fprintf(m_pErrLog,"\n");
		fclose(m_pErrLog);
		va_end(arg_list);
	}

	//--------------------------------------------------------------------------------------
	// Prints a string to the log file
	//--------------------------------------------------------------------------------------
	void Log::Print(const char* str, ...)
	{
		va_list arg_list;
		va_start(arg_list, str);

		m_pErrLog = fopen("log.txt","a");
		
		vfprintf(m_pErrLog,str,arg_list);
		fprintf(m_pErrLog,"\n");
		fclose(m_pErrLog);
		va_end(arg_list);
	}


	//--------------------------------------------------------------------------------------
	// Prints a vector to the log file
	//--------------------------------------------------------------------------------------
	void Log::Print(D3DXVECTOR3& vec)
	{
		m_pErrLog = fopen("log.txt","a");
		fprintf(m_pErrLog,"%f %f %f\n", vec.x, vec.y, vec.z);
		fclose(m_pErrLog);
	}
	void Log::Print(D3DXVECTOR2& vec)
	{
		m_pErrLog = fopen("log.txt","a");
		fprintf(m_pErrLog,"%f %f\n", vec.x, vec.y);
		fclose(m_pErrLog);
	}


	//--------------------------------------------------------------------------------------
	// Reads a line from a file into a string, ignoring empty lines
	//--------------------------------------------------------------------------------------
	void Log::readstr(FILE* f, char* buf)
	{
		fgets(buf,255,f);
		while(buf[0]=='\n')
			fgets(buf,255,f);
	}


	//--------------------------------------------------------------------------------------
	// Outputs the result of a D3D10 error code
	//--------------------------------------------------------------------------------------
	void Log::D3D10Error(HRESULT hr)
	{
		String err = "D3D10 Error> ";
		switch( hr )
		{
		case D3D10_ERROR_FILE_NOT_FOUND:
			err += "D3D10_ERROR_FILE_NOT_FOUND";
			break;

		case D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS:
			err += "D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
			break;

		case D3DERR_INVALIDCALL:
			err += "D3DERR_INVALIDCALL";
			break;

		case D3DERR_WASSTILLDRAWING:
			err += "D3DERR_WASSTILLDRAWING";
			break;

		case E_FAIL:
			err += "E_FAIL";
			break;

		case E_INVALIDARG:
			err += "E_INVALIDARG";
			break;

		case E_OUTOFMEMORY:
			err += "E_OUTOFMEMORY";
			break;

		case S_FALSE:
			err += "S_FALSE";
			break;
		
		default:
			err += "Unkown Error!";
			break;
		};

		Print(err.c_str());
	}

}