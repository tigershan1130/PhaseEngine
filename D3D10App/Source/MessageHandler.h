//--------------------------------------------------------------------------------------
// File: MessageHandler.h
//
// For passing global messages
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include "Array.cpp"

namespace Core
{
		
	// Message IDs
	enum MSG_TYPE
	{
		MSG_NONE = 0, 
		MSG_MESH_UPDATE,
		MSG_LIGHT_UPDATE,
		MSG_MATERIAL_UPDATE,
	};

	// Basic message
	struct Message
	{
		int id;
		void* param;
	};


	class MessageHandler
	{
	public:

		MessageHandler()
		{
			m_MessageIndex = 0;
		}

		static inline void Flush()
		{
			m_Messages.Release();
		}

		// Posts a message
		static inline void SendMessage(int id, void* param)
		{
			Message m;
			m.id = id;
			m.param = param;
			m_Messages.Add(m);
		}

		// Gets the next available message
		static Message GetNextMessage();

	private:

		// Message list
		static Array<Message> m_Messages;
		static int m_MessageIndex;
	};

}