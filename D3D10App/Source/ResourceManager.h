//--------------------------------------------------------------------------------------
// File: ResourceManager.h
//
// Template resource manager class
// Handles file-based resource objects
//
// Any class to be used with a ResourceManager MUST inherit from class EngineResource! 
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "LinkedList.cpp"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Core Engine Resource Manager template class
	//--------------------------------------------------------------------------------------
	template <class T>
	class ResourceManager
	{
	public:
			
			// Adds a new resource to the manager if it's not already in it
			void Add(T* pNew);
			
			// Retrieves an object pointer from the manager by its name
			T* Get(const char* sName); 

			// Loads a new object from a file
			T* Load(const char* sFile);

			// Reloads the resource
			void Reload(T* pObj);

			// Removes an object reference and deletes the object if its unused
			void Deref(T* pObj);

			// Releases all resources
			void Release();

			// Gets the resource list
			inline LinkedList<T*>* GetList(){ return &m_Resources; }
			
			LinkedList<T*>		m_Resources;	// Linked list of object smart ptrs
	};

}