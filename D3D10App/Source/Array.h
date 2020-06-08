//--------------------------------------------------------------------------------------
// File: Array.h
//
// Growable array class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged

#pragma once

namespace Core
{
			
	//--------------------------------------------------------------------
	// Name: Array
	// Desc: Stores an array of objects
	//--------------------------------------------------------------------
	template <class T>
	class Array
	{
	public:
		// Constructors
		Array();
		~Array();

		// Adds an object to the end of the array
		inline void Add( T t )
		{
			if(IsFull())
				Resize();
			m_pData[m_iSize] = t;
			m_iSize++;
		}

		// Inserts an object at [index]
		void Insert(T t, int index);

		// Removes the object at [index]
		T Remove(int index);

		// Removes the given object
		bool Remove(T t);
		
		// Grows the array		
		void Resize();

		// Frees all mem and makes an empty array
		void Release();

		// Resets the array without freeing mem
		inline void Clear(){ m_iSize=0; }

		// Bracket [] style access
		inline T& operator[](SHORT i){ return m_pData[i]; }
		inline T& operator[](UINT i){ return m_pData[i]; }
		inline T& operator[](int i){ return m_pData[i]; }
		inline T& operator[](WORD i){ return m_pData[i]; }
		inline T& operator[](DWORD i){ return m_pData[i]; }

		// Is the array full?
		inline bool IsFull(){ return (m_iSize==m_iCapacity); }

		// Is the array empty?
		inline bool IsEmpty(){ return (m_iSize==0); }

		// Returns the size of the array
		inline int Size(){ return m_iSize; }

		// Convert to c-style array/pointer automatically
		inline operator T*() const{ return m_pData; } 

		// Create using an array
		void FromArray(T* pData, int iLength);

		// Create from a heap array and handle the deletion
		void TakeArray(T* pData, int iLength);

		// Pre-allocates memory
		void Allocate(int iLength);

		// Remove unused memory from the end of the array
		void Finalize();

		// Sort the elements in ascending order
		// (NOTE: uses the < operator so overload if nessecary)
		inline void Sort(){ QuickSort(0, m_iSize); }

		// Sort with a user defined compare function
		inline void Sort(int (*Compare)(T&, T&)){ QuickSort(Compare, 0, m_iSize); }

	private:
		T*	m_pData;		// Array data
		int	m_iSize;		// Size of current array
		int	m_iCapacity;	// Capacity

		// Sorting
		void InsertionSort(int (*Compare)(T&, T&), int first, int last);
		int QSPartition(int (*Compare)(T&, T&), int first, int last);
		void QuickSort(int (*Compare)(T&, T&), int first, int last, int minSize = 25);
		void InsertionSort(int first, int last);
		int QSPartition(int first, int last);
		void QuickSort(int first, int last, int minSize = 25);
	};

}