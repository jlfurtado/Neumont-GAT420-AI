#include "StackFSM.h"
#include "GameLogger.h"

// Justin Furatdo
// 5/29/2017
// StackFSM.cpp
// Implements a simple stack based Finite State Machine

namespace Engine
{
	bool StackFSM::Update(float dt)
	{
		// only update if we have things on our stack
		if (m_fsmStack.GetCount() > 0)
		{
			// updates the top thing on our stack
			FSMPair action = m_fsmStack.GetFirstObjectData();
			action.m_update(dt, action.m_pClass);
		}

		return true;
	}

	void StackFSM::Push(FSMStateUpdate func, void * pData)
	{
		// create an action and add it to the front of our list!
		FSMPair action;
		action.m_update = func;
		action.m_pClass = pData;
		m_fsmStack.AddToListFront(action);
	}

	void StackFSM::Pop()
	{
		// tosses the first thing
		m_fsmStack.RemoveHeadNode();
	}

	StackFSM::FSMPair StackFSM::GetCurrentState()
	{
		// returns current state if present, default FSMPair otherwise
		return m_fsmStack.GetFirstObjectData();
	}

	void StackFSM::LogNotSet(float /*dt*/, void * /*pData*/)
	{
		// useful default for this to warn users if their npcs are doing nothing!?!?
		GameLogger::Log(MessageType::cWarning, "FSMPair was not initialized! Calling default method which does nothing.\n");
	}

}