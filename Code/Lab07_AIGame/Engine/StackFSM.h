#ifndef STACKFSMCOMPONENT_H
#define STACKFSMCOMPONENT_H

// Justin Furatdo
// 5/29/2017
// StackFSM.h
// Implements a simple stack based Finite State Machine

#include "ExportHeader.h"
#include "LinkedList.h"

namespace Engine
{
	typedef void(*FSMStateUpdate)(float dt, void *pData);
	typedef void(*FSMStateEnter)(void *pData);
	typedef void(*FSMStateExit)(void *pData);

	struct ENGINE_SHARED FSMPair
	{
		FSMPair() : m_enter(FSMPair::LogNotSet), m_exit(FSMPair::LogNotSet), m_update(FSMPair::LogNotSet), m_pClass(nullptr) {}
		FSMPair(FSMStateEnter enterFunc, FSMStateUpdate updateFunc, FSMStateExit exitFunc, void *pData): m_enter(enterFunc), m_exit(exitFunc), m_update(updateFunc), m_pClass(pData) {}
		FSMStateEnter m_enter{ FSMPair::LogNotSet };
		FSMStateExit m_exit{ FSMPair::LogNotSet };
		FSMStateUpdate m_update{ FSMPair::LogNotSet };
		void *m_pClass{ nullptr };

		static void LogNotSet(float dt, void *pData);
		static void LogNotSet(void *pData);
	};

	class ENGINE_SHARED StackFSM
	{

	public:

		bool Update(float dt);
		void Push(FSMStateEnter enterFunc, FSMStateUpdate updateFunc, FSMStateExit exitFunc, void *pData);
		void Push(FSMPair pair);
		void Pop();
		bool IsEmpty();

	private:
		FSMPair GetCurrentState();

		LinkedList<FSMPair> m_fsmStack;
	};
}


#endif // ifndef STACKFSMCOMPONENT_H