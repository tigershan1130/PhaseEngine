//--------------------------------------------------------------------------------------
// File: Stack.h
//
// Stack class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once


namespace Core
{

	//--------------------------------------------------------------------------------------
	// Template stack class
	//--------------------------------------------------------------------------------------
	template <class T>
	class Stack
	{
	public:
		Stack();

		// Push data onto the stack
		inline void Push(T data);

		// Pop data from the stack
		inline T Pop();

		// Pop data from the back of the stack
		inline T PopBack();

		// Is the stack empty?
		inline bool IsEmpty() const;

		// Return the stack size
		inline int Size() const;

		// Free all mem and create an empty stack
		void Release();

		// Free all mem, delete all data and create an empty stack
		void ReleaseAndDelete();

	private:
		T*		m_pData;		// Stack data
		int		m_iSize;		// Current stack size
		int		m_iCapacity;	// Stack capacity

		// Resize the stack to double its current capacity
		void Resize();
	};

}