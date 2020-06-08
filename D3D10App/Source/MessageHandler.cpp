//--------------------------------------------------------------------------------------
// File: MessageHandler.cpp
//
// For passing global messages
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "MessageHandler.h"
namespace Core
{

	Array<Message> MessageHandler::m_Messages;
	int MessageHandler::m_MessageIndex;

	//--------------------------------------------------------------------------------------
	// Gets the next available message
	//--------------------------------------------------------------------------------------
	Message MessageHandler::GetNextMessage()
	{
		if(m_MessageIndex>=m_Messages.Size())
		{
			if(m_Messages.Size()>0)
				m_Messages.Clear();
			m_MessageIndex = 0;

			Message m;
			m.id = MSG_NONE;
			return m;
		}

		return m_Messages[m_MessageIndex++];
	}
}