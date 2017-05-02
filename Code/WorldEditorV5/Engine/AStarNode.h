#ifndef ASTARNODE_H
#define ASTARNODE_H

// Justin Furtado
// 5/1/2017
// AStarNode.h
// Defines an A-Star Pathing Node

#include "ExportHeader.h"
#include "Vec3.h"
#include "LinkedList.h"

namespace Engine
{
	class GraphicalObject; // pre-declare to avoid include because we only need a pointer here
	class ENGINE_SHARED AStarNode
	{
	public:
		AStarNode();
		~AStarNode();

		bool CanGoToNode(AStarNode *pOtherNode);
		void ConnectToNode(AStarNode *pNodeToConnectTo);
		void StopConnectingTo(AStarNode *pNodeToNotConnectTo);
		void WalkConnectedNodes(LinkedList<AStarNode>::LinkedListIterationCallback callback, void *pClassInstance);
		void ClearConnectedNodes();
		unsigned int GetConnectedNodeCount();
		Vec3 GetPosition();
		void SetPosition(const Vec3& newPosition);
		float GetRadius();
		void SetRadius(float newRadius);
		GraphicalObject *GetGobPointer();
		void SetGobPointer(GraphicalObject *pGob);
	
	private:
		Vec3 m_posiiton;
		float m_radius;
		GraphicalObject *m_pGob;
		LinkedList<AStarNode> m_connectedToNodes;
	};

}


#endif // ifndef ASTARNODE_H