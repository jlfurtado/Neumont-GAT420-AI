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

	class ENGINE_SHARED StackFSM
	{
		struct FSMPair
		{
			FSMStateUpdate m_update{ StackFSM::LogNotSet };
			void *m_pClass{ nullptr };
		};

	public:

		bool Update(float dt);
		void Push(FSMStateUpdate func, void *pData);
		void Pop();

	private:
		FSMPair GetCurrentState();
		static void LogNotSet(float dt, void *pData);

		LinkedList<FSMPair> m_fsmStack;
	};
}


#endif // ifndef STACKFSMCOMPONENT_H