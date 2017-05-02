#ifndef ASTARNODE_H
#define ASTARNODE_H

// Justin Furtado
// 5/1/2017
// AStarNode.h
// Defines an A-Star Pathing Node

#include "ExportHeader.h"
#include "Vec3.h"
#include "LinkedList.h"
#include "GraphicalObject.h"

namespace Engine
{
	class ENGINE_SHARED AStarNode
	{
		struct Connection
		{
			GraphicalObject m_connectionArrow;
			AStarNode *m_pConnectedTo;
		};
		
		struct PassThroughConnectionToNode
		{
			LinkedList<AStarNode>::LinkedListIterationCallback callback;
			void *pClassInstance;
		};

	public:
		typedef void(*ObjInitCallback)(void *pClass, GraphicalObject *pObj);

		AStarNode();
		~AStarNode();

		static void SetSphereInitCallback(ObjInitCallback callback, void *pClass);
		static void SetArrowInitCallback(ObjInitCallback callback, void *pClass);
		void MakeConnectionGob(GraphicalObject *pGob);
		void InitGob();
		bool CanGoToNode(AStarNode *pOtherNode);
		void ConnectToNode(AStarNode *pNodeToConnectTo);
		void StopConnectingTo(AStarNode *pNodeToNotConnectTo);
		void WalkEnabledConnectedNodes(LinkedList<AStarNode>::LinkedListIterationCallback callback, void *pClassInstance);
		void ClearConnectedNodes();
		unsigned int GetConnectedNodeCount();
		Vec3 GetPosition();
		void SetPosition(const Vec3& newPosition);
		float GetRadius();
		void SetRadius(float newRadius);
		void SetVisualEnabled(bool enabled);
		void SetNodeEnabled(bool enabled);

	private:
		static void LogNotSetError(void *pClass, GraphicalObject *pObj);
		static bool EnableObjForConnection(Connection *pConnectedTo, void *pClassInstance);
		static bool DisableObjForConnection(Connection *pConnectedTo, void *pClassInstance);
		static bool UpdateConnection(Connection *pConnectedTo, void *pClassInstance);
		static bool WalkEnabledConnectedAStarNodes(Connection *pConnectedTo, void *pImDoingSomethingDifferentHere);
		static bool CheckAStarNode(Connection *pConnection, void *pImDoingSomethingDifferentHere);
		static bool DeleteConnection(Connection *pConnection, void *pClassInstance);
		static bool ConnectionContains(Connection *pConnection, void *pImDoingSomethingDifferentHere);

		bool m_enabled{ true };
		Vec3 m_position;
		float m_radius{ 1.0f };
		GraphicalObject m_gob;
		LinkedList<Connection> m_connections;
		int m_connectionCount{ 0 };

		static ObjInitCallback s_sphereInitCallback;
		static void *s_pSphereInitClassInstance;
		static ObjInitCallback s_connectionInitCallback;
		static void *s_pConnectionClassInstance;
	};
	
	template class ENGINE_SHARED LinkedList<AStarNode>;
}


#endif // ifndef ASTARNODE_H