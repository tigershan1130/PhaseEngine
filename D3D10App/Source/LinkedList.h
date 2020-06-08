//--------------------------------------------------------------------------------------
// File: LinkedList.h
//
// Linked List class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "Stack.cpp"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Template Linked List Class
	//--------------------------------------------------------------------------------------
	template <class T>
	class LinkedList
	{
	public:
			LinkedList();

			// Adds an object to the front of the list
			void Add(T data);

			// Adds an object to the end of the list
			void AddToEnd(T data);

			// Begins an itteration sequence
			inline void Itterate();

			// Moves in itterate
			inline void StepForward();
			inline void StepBack();

			// Returns the object at location i
			T Get(int index);		

			// Gets the next objects (Itterate must be called previously)
			inline T GetCurrent();

			// Returns the last object
			inline T GetLast();

			// Removes the first object
			T RemoveFirst();

			// Removes the object
			T Remove(T data);

			// Removes the last object from the list
			T RemoveLast();

			// Removes the currect object during an itteration
			T RemoveCurrent();

			// Returns the length
			inline int Length();

			// Clears the light
			inline void Clear();

			// Releases dynamic memory
			inline void Release();
			inline void ReleaseAndDelete();

			// Is the list empty?
			inline bool IsEmpty(){ return (m_iLength==0); }
			inline bool HasNext(){ return (m_pTemp!=NULL); }
	protected:
			// Structure for holding T objects
			struct Node
			{
				T data;
				Node* next;
				Node* prev;

				inline void Delete(){ SAFE_DELETE(data); }
				Node();
			};

			Node*	m_pFirst;		// Pointer to first node
			Node*	m_pLast;		// Pointer to last node
			Node*	m_pTemp;		// Pointer to node used in itteration
			int		m_iLength;		// Length of the list
			
			Stack<Node*> m_Mem;		// Mem manager
	};


}