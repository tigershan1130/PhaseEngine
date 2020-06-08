//--------------------------------------------------------------------------------------
// File: Array.cpp
//
// Growable array class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "stdafx.h"
#include "Array.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Constructors
	//--------------------------------------------------------------------------------------
	template <class T>
	Array<T>::Array()
	{ 
		m_iSize=0; 
		m_pData = NULL; 
		m_iCapacity=0; 
	} 

	//--------------------------------------------------------------------------------------
	// Destructor
	//--------------------------------------------------------------------------------------
	template <class T>
	Array<T>::~Array()
	{ 

	}

	//--------------------------------------------------------------------------------------
	// Inserts an object at [index]
	//--------------------------------------------------------------------------------------
	template <class T>
	void Array<T>::Insert(T t, int index)
	{
		if(IsFull())
			Resize();
		int i;
		for(i=m_iSize; i>index; i--)
			m_pData[i] = m_pData[i-1];
		m_pData[i] = t;
		m_iSize++;
	}

	//--------------------------------------------------------------------------------------
	// Removes the object at [index]
	//--------------------------------------------------------------------------------------
	template <class T>
	T Array<T>::Remove(int index)
	{
		int i;
		T ret = m_pData[index];
		for(i=index; i<m_iSize-1; i++)
			m_pData[i] = m_pData[i+1];
		m_iSize--;
		return ret;
	}

	//--------------------------------------------------------------------------------------
	// Removes the given object
	//--------------------------------------------------------------------------------------
	template <class T>
	bool Array<T>::Remove(T t)
	{
		int i;
		bool ret = false;
		for(i=0; i<m_iSize; i++)
			if(m_pData[i] == t)
			{
				ret = true;
				m_iSize--;
				break;
			}
		for(; i<m_iSize; i++)
			m_pData[i] = m_pData[i+1];
		return ret;
	}

	//--------------------------------------------------------------------------------------
	// Grows the array
	//--------------------------------------------------------------------------------------
	template <class T>
	void Array<T>::Resize()
	{ 
		if(m_iCapacity==0)
			m_iCapacity = 5;
		else
			m_iCapacity*=2;
		T* pNew = new T[m_iCapacity]; 
		for(int i=0; i<m_iSize; i++)
			pNew[i] = m_pData[i];
		delete[] m_pData;
		m_pData = pNew;
	}

	//--------------------------------------------------------------------------------------
	// Frees all mem and makes an empty array
	//--------------------------------------------------------------------------------------
	template <class T>
	void Array<T>::Release()
	{
		m_iSize=0;
		m_iCapacity=0;
		if(m_pData)
			delete[] m_pData;
		m_pData = NULL;
	}



	//--------------------------------------------------------------------------------------
	// Create using an array
	//--------------------------------------------------------------------------------------
	template <class T>
	void Array<T>::FromArray(T* pData, int iLength)
	{
		Release();
		m_pData = new T[iLength];
		memcpy(m_pData,pData,iLength*sizeof(T));
		m_iSize = m_iCapacity = iLength;
	}

	//--------------------------------------------------------------------------------------
	// Create using an array and manage the deletion
	//--------------------------------------------------------------------------------------
	template <class T>
	void Array<T>::TakeArray(T* pData, int iLength)
	{
		Release();
		m_pData = pData;
		m_iSize = m_iCapacity = iLength;
	}

	//--------------------------------------------------------------------------------------
	// Pre-allocates memory
	//--------------------------------------------------------------------------------------
	template <class T>
	void Array<T>::Allocate(int iLength)
	{
		Release();
		m_pData = new T[iLength];
		m_iSize = m_iCapacity = iLength;
	}	

	//--------------------------------------------------------------------------------------
	// Remove unused memory from the end of the array
	//--------------------------------------------------------------------------------------
	template <class T>
	void Array<T>::Finalize()
	{
		if(!m_pData)
			return;
		T* pNewData = new T[m_iSize];
		memcpy(pNewData, m_pData, m_iSize*sizeof(T));
		delete[] m_pData;
		m_pData = pNewData;
		m_iCapacity = m_iSize;
	}

	
	//--------------------------------------------------------------------------------------
	// Basic insertion sort
	//--------------------------------------------------------------------------------------
	template <class T>
	void Array<T>::InsertionSort(int first, int last)
	{
		// Empty m_pData
		if(last<=first) return;
		int pos = last, index;
		T value;
		while (pos > 0) {
			// Save this element to insert later
			index = pos - 1;
			value = m_pData[index];

			// Move all greater elements to the right
			while ((index < m_iSize-1) && (value > m_pData[index+1])) {
				m_pData[index] = m_pData[index+1];
				index++;
			}

			// Insert the value in the correct spot for this pass
			m_pData[index] = value;
			pos--;
		}
	}

	// Basic insertion sort with comparison function
	template <class T>
	void Array<T>::InsertionSort(int (*Compare)(T&, T&), int first, int last)
	{
		// Empty m_pData
		if(last<=first) return;
		int pos = last, index;
		T value;
		while (pos > 0) {
			// Save this element to insert later
			index = pos - 1;
			value = m_pData[index];

			// Move all greater elements to the right
			while ((index < m_iSize-1) && Compare(value,m_pData[index+1])>0) {
				m_pData[index] = m_pData[index+1];
				index++;
			}

			// Insert the value in the correct spot for this pass
			m_pData[index] = value;
			pos--;
		}
	}

	// Partitioning for quicksort.  Returns the partition
	// index.  The element at this index will be in its
	// final location, with all the elements to the left
	// less than or equal to it, and all greater elements
	// to the right.
	template <class T>
	int Array<T>::QSPartition(int first, int last)
	{
		int lastSmall=first;
		for(int i=first+1; i<=last; i++)
			if(m_pData[i]<=m_pData[first])
			{
				lastSmall++;
				Swap(lastSmall,i);
			}
			Swap(first,lastSmall);
			return lastSmall;
	}
	
	// With comparison function
	template <class T>
	int Array<T>::QSPartition(int (*Compare)(T&, T&), int first, int last)
	{
		int lastSmall=first;
		for(int i=first+1; i<=last; i++)
			if(Compare(m_pData[i],m_pData[first])<=0)
			{
				lastSmall++;
				Swap(lastSmall,i);
			}
			Swap(first,lastSmall);
			return lastSmall;
	}

	// Quicksort algorithm.  If the m_pData is smaller
	// than minSize, it will just use insertion sort
	// to minimize the recursion overhead
	template <class T>
	void Array<T>::QuickSort(int first, int last, int minSize)
	{
		// Empty m_pData
		if(last<=first) return;

		// Check if the m_pData is small enough
		// for insertion sort
		if(last-first <= minSize)
		{
			InsertionSort(first, last);
			return;
		}

		// Find the partition index
		int split = QSPartition(first, last);

		// Now sort the two halves
		QuickSort(first, split-1, minSize);
		QuickSort(split+1, last, minSize);
	}

	// With comparison function
	template <class T>
	void Array<T>::QuickSort(int (*Compare)(T&, T&), int first, int last, int minSize)
	{
		// Empty m_pData
		if(last<=first) return;

		// Check if the m_pData is small enough
		// for insertion sort
		if(last-first <= minSize)
		{
			InsertionSort(Compare, first, last);
			return;
		}

		// Find the partition index
		int split = QSPartition(Compare, first, last);

		// Now sort the two halves
		QuickSort(Compare, first, split-1, minSize);
		QuickSort(Compare, split+1, last, minSize);
	}
}