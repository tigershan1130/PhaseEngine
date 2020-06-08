//--------------------------------------------------------------------------------------
// File: ResourceManager.cpp
//
// Template resource manager class
// Handles file-based resource objects
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once


#include "stdafx.h"
#include "ResourceManager.h"

namespace Core
{


	//--------------------------------------------------------------------------------------
	// Retrieves an object pointer from the list, otherwise returns NULL
	//--------------------------------------------------------------------------------------
	template <class T>
	T* ResourceManager<T>::Get(const char* sName)
	{
		if(!sName)
			return NULL;
		
		// Itterate through the list
		T* pObj = NULL;
		m_Resources.Itterate();
		while( (pObj=m_Resources.GetCurrent()) )
		{
			// Check if the filenames match
			if( pObj && strcmp(pObj->GetName(),sName) == 0)
				return pObj;
			m_Resources.StepForward();
		}
		
		// Model is not in the list so return NULL
		return pObj;
	}


	//--------------------------------------------------------------------------------------
	// Adds an object to the manager if it's not already in it
	//--------------------------------------------------------------------------------------
	template <class T>
	void ResourceManager<T>::Add(T* pNew)
	{
		if(!pNew)
			return;

		// Make sure it's not already in the model list
		if(!Get(pNew->GetName()))
		{
			// Add it to the list
			m_Resources.Add(pNew);
		}
	}


	//--------------------------------------------------------------------------------------
	// Dereferences an object and deletes it if it's unused
	//--------------------------------------------------------------------------------------
	template <class T>
	void ResourceManager<T>::Deref(T* pObj)
	{
		// If it exists
		if(pObj)
		{
			if(!Get(pObj->GetName()))
				return;
			
			// Dec the ref count
			pObj->Deref();
			
			// If it is no longer active, delete it
			if(!pObj->IsActive())
			{
				m_Resources.Remove(pObj);
				pObj->Release();
				delete pObj;
			}		
		}
	}


	//--------------------------------------------------------------------------------------
	// Loads an object from a file into the manager if it's not already in it
	//--------------------------------------------------------------------------------------
	template <class T>
	T* ResourceManager<T>::Load(const char* sFile)
	{
		if(!sFile)
			return NULL;
		
		// Make sure it's not already in the manager
		T* pObjPtr = Get(sFile);
		if(!pObjPtr)
		{
			// Make a new object
			T* pNew = new T;
			if(!pNew->Load(sFile))
			{
				delete pNew;
				return NULL;
				Log::Print("Resource %s failed to load!", sFile);
			}
			
			
			pNew->AddRef();
			
			// Add it to the list
			m_Resources.Add(pNew);
			return pNew;

		}
		pObjPtr->AddRef();
		return pObjPtr;
	}

	//--------------------------------------------------------------------------------------
	// Reloads the resource
	//--------------------------------------------------------------------------------------
	template <class T>		
	void ResourceManager<T>::Reload(T* pObj)
	{
		if(!pObj || !Get(pObj->GetName()) )
			return;
		String szName = pObj->GetName();
		pObj->Release();
		pObj->Load( szName );
	}	


	//--------------------------------------------------------------------------------------
	// Releases all resources
	//--------------------------------------------------------------------------------------
	template <class T>
	void ResourceManager<T>::Release()
	{
		// Itterate through the list
		T* pObj;
		m_Resources.Itterate();
		while( (pObj=m_Resources.RemoveCurrent()) )
		{
			// Release the object
			pObj->Release();
					
			// Delete the object
			delete pObj;
		}

		// Release the linked list
		m_Resources.Release();

	}

}