//--------------------------------------------------------------------------------------
// File: Stack.cpp
//
// Stack class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "Stack.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Constructor
	//--------------------------------------------------------------------------------------
	template <class T>
	Stack<T>::Stack()
	{ 
		m_pData = NULL; 
		m_iCapacity = 0; 
		m_iSize=-1;
	}

	//--------------------------------------------------------------------------------------
	// Push data onto the stack
	//--------------------------------------------------------------------------------------
	template <class T>
	inline void Stack<T>::Push(T t)
	{ 
		m_iSize++;
		if(m_iSize==m_iCapacity)
			Resize();
		m_pData[m_iSize] = t;
	}


	//--------------------------------------------------------------------------------------
	// Pop data from the stack
	//--------------------------------------------------------------------------------------
	template <class T>
	inline T Stack<T>::Pop()
	{ 
		m_iSize--;
		return m_pData[m_iSize+1]; 
	}

	//--------------------------------------------------------------------------------------
	// Pop data from the back of the stack
	//--------------------------------------------------------------------------------------
	template <class T>
	inline T Stack<T>::PopBack()
	{ 
		int i;
		T ret = m_pData[0];
		for(i=0; i<m_iSize-1; i++)
			m_pData[i] = m_pData[i+1];
		m_iSize--;
		return ret;
	}

	//--------------------------------------------------------------------------------------
	// Is the stack empty?
	//--------------------------------------------------------------------------------------
	template <class T>
	inline bool Stack<T>::IsEmpty() const 
	{ 
		return (m_iSize==-1); 
	}

	//--------------------------------------------------------------------------------------
	// Return the size of the stack
	//--------------------------------------------------------------------------------------
	template <class T>
	inline int Stack<T>::Size() const
	{ 
		return m_iSize; 
	}

	//--------------------------------------------------------------------------------------
	// Free all mem and create an empty stack
	//--------------------------------------------------------------------------------------
	template <class T>
	void Stack<T>::Release()
	{ 
		SAFE_DELETE_ARRAY(m_pData); 
		m_iCapacity = 0; 
		m_iSize=-1; 
	}

	//--------------------------------------------------------------------------------------
	// Free all mem, delete all data and create an empty stack
	//--------------------------------------------------------------------------------------
	template <class T>
	void Stack<T>::ReleaseAndDelete()
	{ 
		for(int i=0; i<m_iSize; i++) 
			delete m_pData[i]; 
		Release();
	}

	//--------------------------------------------------------------------------------------
	// Resize the stack to double its current capacity
	//--------------------------------------------------------------------------------------
	template <class T>
	void Stack<T>::Resize()
	{
		if(m_iCapacity==0)
		{
			m_iCapacity = 10; 
			m_pData = new T[m_iCapacity]; 
			 return;
		}
		T* pTemp = new T[m_iCapacity*2];
		memcpy(pTemp,m_pData,m_iCapacity*sizeof(T));
		delete[] m_pData;
		m_pData = pTemp;
		m_iCapacity*=2;
	}

}