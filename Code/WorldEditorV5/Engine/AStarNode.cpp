#include "AStarNode.h"

// Justin Furtado
// 5/1/2017
// AStarNode.h
// Defines an A-Star Pathing Node

namespace Engine
{

	AStarNode::AStarNode()
	{
	}

	AStarNode::~AStarNode()
	{
	}

	bool AStarNode::CanGoToNode(AStarNode * pOtherNode)
	{
		return m_connectedToNodes.Contains(pOtherNode);
	}

	void AStarNode::ConnectToNode(AStarNode * pNodeToConnectTo)
	{
		m_connectedToNodes.AddToList(pNodeToConnectTo);
	}

	void AStarNode::StopConnectingTo(AStarNode *pNodeToNotConnectTo)
	{
		m_connectedToNodes.RemoveFromList(pNodeToNotConnectTo);
	}

	void AStarNode::WalkConnectedNodes(LinkedList<AStarNode>::LinkedListIterationCallback callback, void * pClassInstance)
	{
		m_connectedToNodes.WalkList(callback, pClassInstance);
	}

	void AStarNode::ClearConnectedNodes()
	{
		m_connectedToNodes.ClearList();
	}

	unsigned int AStarNode::GetConnectedNodeCount()
	{
		return m_connectedToNodes.GetCount();
	}

	Vec3 AStarNode::GetPosition()
	{
		return m_posiiton;
	}

	void AStarNode::SetPosition(const Vec3 & newPosition)
	{
		m_posiiton = newPosition;
	}

	float AStarNode::GetRadius()
	{
		return m_radius;
	}

	void AStarNode::SetRadius(float newRadius)
	{
		m_radius = newRadius;
	}

	GraphicalObject * AStarNode::GetGobPointer()
	{
		return m_pGob;
	}

	void AStarNode::SetGobPointer(GraphicalObject * pGob)
	{
		m_pGob = pGob;
	}

}
