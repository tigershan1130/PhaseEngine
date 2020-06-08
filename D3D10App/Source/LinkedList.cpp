//--------------------------------------------------------------------------------------
// File: LinkedList.cpp
//
// Linked List class
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once
#include "LinkedList.h"

namespace Core
{

	//--------------------------------------------------------------------------------------
	// Node functions
	//--------------------------------------------------------------------------------------
	template <class T>
	LinkedList<T>::Node::Node()
	{ 
		next=prev=NULL; 
	}


	//--------------------------------------------------------------------------------------
	// Contsuctor
	//--------------------------------------------------------------------------------------
	template <class T>
	LinkedList<T>::LinkedList()
	{
		m_pFirst=m_pLast=m_pTemp=NULL;
		m_iLength=0;
	}


	//--------------------------------------------------------------------------------------
	// Adds an object to the front of the list
	//--------------------------------------------------------------------------------------
	template <class T>
	void LinkedList<T>::Add(T data)
	{
		Node* node;
		if(m_Mem.IsEmpty())
			node = new Node;
		else
			node = m_Mem.Pop();
		node->data = data;
		if(!m_pFirst)
			m_pLast = m_pFirst = node;
		else
		{
			m_pFirst->prev = node;
			node->next = m_pFirst;
			m_pFirst = node;
		}
		m_iLength++;
	}


	//--------------------------------------------------------------------------------------
	// Adds an object to the end of the list
	//--------------------------------------------------------------------------------------
	template <class T>
	void LinkedList<T>::AddToEnd(T data)
	{
		Node* node;
		if(m_Mem.IsEmpty())
			node = new Node;
		else
			node = m_Mem.Pop();
		node->data = data;
		if(!m_pFirst)
			m_pLast = m_pFirst = node;
		else
		{
			node->prev = m_pLast;
			m_pLast->next = node;
			m_pLast = node;
		}
		m_iLength++;
	}


	//--------------------------------------------------------------------------------------
	// Gets the object that is index objects from the front
	//--------------------------------------------------------------------------------------
	template <class T>
	T LinkedList<T>::Get(int index=0)
	{
		if(!m_pFirst || index<0||index>=m_iLength)
			return NULL;
		Node* curr = m_pFirst;
		for(int i=0; i<index; i++)
			curr=curr->next;
		return curr->data;
	}


	//--------------------------------------------------------------------------------------
	// Gets objects at the end of the list
	//--------------------------------------------------------------------------------------
	template <class T>
	inline T LinkedList<T>::GetLast()
	{
		if(m_pLast)
			return m_pLast->data;
		return NULL;
	}


	//--------------------------------------------------------------------------------------
	// Gets the next object during an itteration
	//--------------------------------------------------------------------------------------
	template <class T>
	inline T LinkedList<T>::GetCurrent()
	{
		if(!m_iLength || !m_pTemp)
			return NULL;
		return m_pTemp->data;
	}


	//--------------------------------------------------------------------------------------
	// Moves forward in itterate
	//--------------------------------------------------------------------------------------
	template <class T>
	inline void LinkedList<T>::StepForward()
	{
		if(m_pTemp)
			m_pTemp=m_pTemp->next;
	}

	//--------------------------------------------------------------------------------------
	// Moves forward in itterate
	//--------------------------------------------------------------------------------------
	template <class T>
	inline void LinkedList<T>::StepBack()
	{
		if(m_pTemp)
			m_pTemp=m_pTemp->prev;
	}


	//--------------------------------------------------------------------------------------
	// Begins an itteration
	//--------------------------------------------------------------------------------------
	template <class T>
	inline void LinkedList<T>::Itterate()
	{
		m_pTemp = m_pFirst;
	}


	//--------------------------------------------------------------------------------------
	// Gets the number of objects in the list
	//--------------------------------------------------------------------------------------
	template <class T>
	inline int LinkedList<T>::Length()
	{
		return m_iLength;
	}


	//--------------------------------------------------------------------------------------
	// Removes the first object and returns it
	//--------------------------------------------------------------------------------------
	template <class T>
	T LinkedList<T>::RemoveFirst()
	{
		if(!m_pFirst)
			return NULL;
		T data = m_pFirst->data;
		Node* node = m_pFirst;
		m_iLength--;
		if(m_pFirst == m_pLast)
			m_pFirst = m_pLast = NULL;
		else
		if( (m_pFirst = m_pFirst->next) )
			m_pFirst->prev=NULL;
		node->next=NULL;
		node->prev=NULL;
		m_Mem.Push(node);
		return data;
	}


	//--------------------------------------------------------------------------------------
	// If obj is in the list it is removed and returned
	//--------------------------------------------------------------------------------------
	template <class T>
	T LinkedList<T>::Remove(T obj)
	{
		if(!m_pFirst)
			return NULL;
		
		// Find the object
		Node* node = m_pFirst;
		while(node&&node->data!=obj)
			node=node->next;
			
		// It isnt in the list...
		if(!node)
			return NULL;
		m_iLength--;

		// Clear the nodes prev pointer
		if(node->prev)
			node->prev->next = node->next;
		if(node->next)
			node->next->prev = node->prev;
		if(node == m_pFirst)
			m_pFirst = node->next;
		if(node == m_pLast)
			m_pLast = node->prev;
			
		// Delete it and decrement the length
		T data = node->data;
		node->next=NULL;
		node->prev=NULL;
		m_Mem.Push(node);
		
		return data;
	}


	//--------------------------------------------------------------------------------------
	// Removes the current object during an itteration and returns it
	//--------------------------------------------------------------------------------------
	template <class T>
	T LinkedList<T>::RemoveCurrent()
	{
		T data = NULL;
		if(m_pTemp)
		{
			if(m_pTemp->prev)
				m_pTemp->prev->next = m_pTemp->next;
			if(m_pTemp->next)
				m_pTemp->next->prev = m_pTemp->prev;
			if(m_pTemp == m_pFirst)
				m_pFirst = m_pTemp->next;
			if(m_pTemp == m_pLast)
				m_pLast = m_pTemp->prev;
			Node* node = m_pTemp;
			data = node->data;
			m_pTemp = m_pTemp->next;
			node->next = NULL;
			node->prev=NULL;
			m_Mem.Push(node);		
			m_iLength--;
		}
		return data;
	}


	//--------------------------------------------------------------------------------------
	// Removes the object at the end of the list and returns it
	//--------------------------------------------------------------------------------------
	template <class T>
	T LinkedList<T>::RemoveLast()
	{
		if(!m_pLast)
			return NULL;
		T data = m_pLast->data;
		Node* node = m_pLast;
		m_iLength--;
		if(m_pFirst == m_pLast)
			m_pFirst = m_pLast = NULL;
		else
		if( (m_pLast = m_pLast->prev) )
			m_pLast->next=NULL;
		node->next=NULL;
		node->prev=NULL
		m_Mem.Push(node);
		return data;
	}

	//--------------------------------------------------------------------------------------
	// Makes an empty list
	//--------------------------------------------------------------------------------------
	template <class T>
	inline void LinkedList<T>::Clear()
	{
		Node* temp = m_pFirst;
		while( temp!= NULL )
		{
			m_Mem.Push( temp );
			temp = temp->next;
		}
		m_pFirst = m_pLast = m_pTemp = NULL;
		m_iLength = 0;
	}

	//--------------------------------------------------------------------------------------
	// Frees all mem and creates an empty list
	//--------------------------------------------------------------------------------------
	template <class T>
	inline void LinkedList<T>::Release()
	{
		Node* temp = m_pFirst;
		while(temp)
		{
			Node* n = temp->next;
			delete temp;
			temp = n;
		}		
		m_pTemp = NULL;
		m_pLast = NULL;
		m_pFirst = NULL;
		m_iLength = 0;
		while(!m_Mem.IsEmpty())
			delete m_Mem.Pop();
		m_Mem.Release();
	}

	//--------------------------------------------------------------------------------------
	// Clears the list
	//--------------------------------------------------------------------------------------
	template <class T>
	inline void LinkedList<T>::ReleaseAndDelete()
	{
		Node* temp = m_pFirst;
		while(temp)
		{
			Node* n = temp->next;
			delete temp->data;
			delete temp;
			temp = n;
		}	
		while(!m_Mem.IsEmpty())
		{
			temp = m_Mem.Pop();
			delete temp->data;
			delete temp;
		}
		m_Mem.Release();
		m_pTemp = NULL;
		m_pLast = NULL;
		m_pFirst = NULL;
		m_iLength = 0;
		LinkedList();
	}

}