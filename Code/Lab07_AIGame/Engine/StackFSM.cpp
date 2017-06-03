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

	void StackFSM::Push(FSMStateEnter enterFunc, FSMStateUpdate updateFunc, FSMStateExit exitFunc, void * pData)
	{
		// create an action and add it to the front of our list!
		this->Push(FSMPair(enterFunc, updateFunc, exitFunc, pData));
	}

	void StackFSM::Push(FSMPair pair)
	{
		// don't over push
		bool empty = this->IsEmpty();
		FSMPair current = GetCurrentState();
		if (current.m_enter == pair.m_enter && current.m_update == pair.m_update && pair.m_exit == current.m_exit && pair.m_pClass == current.m_pClass) { return; }

		// add it
		m_fsmStack.AddToListFront(pair);

		// if we were empty before the addition, call exit on the thing we covered up
		if (!empty) { current.m_exit(pair.m_pClass); }
		
		// EXIT THE LAST ONE BEFORE ENTERING THE NEW ONE

		// call enter func
		pair.m_enter(pair.m_pClass);
	}

	void StackFSM::Pop()
	{
		// can't pop an empty stack
		if (this->IsEmpty()) { return; }

		// tosses the first thing after calling its exit, then calls the new one's enter
		FSMPair current = GetCurrentState();
		current.m_exit(current.m_pClass);
		m_fsmStack.RemoveHeadNode();

		// only call enter if there is a new state
		if (m_fsmStack.GetCount() > 0)
		{
			current = GetCurrentState();
			current.m_enter(current.m_pClass);
		}
	}

	bool StackFSM::IsEmpty()
	{
		return m_fsmStack.GetCount() <= 0;
	}

	FSMPair StackFSM::GetCurrentState()
	{
		// returns current state if present, default FSMPair otherwise
		return m_fsmStack.GetFirstObjectData();
	}

	void FSMPair::LogNotSet(float /*dt*/, void * pData)
	{
		LogNotSet(pData);
	}

	void FSMPair::LogNotSet(void * /*pData*/)
	{
		// useful default for this to warn users if their npcs are doing nothing!?!?
		GameLogger::Log(MessageType::cWarning, "FSMPair was not initialized! Calling default method which does nothing.\n");
	}

}