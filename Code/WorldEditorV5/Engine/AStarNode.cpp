#include "AStarNode.h"
#include "ShapeGenerator.h"
#include "RenderEngine.h"
#include "CollisionTester.h"

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

	Vec3 AStarNode::GetPosition()
	{
		// simple getter
		return m_position;
	}

	void AStarNode::SetPosition(const Vec3 & newPosition)
	{
		// move
		m_position = newPosition;
	}

	float AStarNode::GetRadius()
	{
		// simple getter
		return m_radius;
	}

	void AStarNode::SetRadius(float newRadius)
	{
		// get fatter/thinner
		m_radius = newRadius;
	}

	// sets whether the node can be used
	void AStarNode::SetNodeEnabled(bool enabled)
	{
		m_enabled = enabled;
	}

	// returns whether or not the node can be used
	bool AStarNode::IsEnabled()
	{
		return m_enabled;
	}

	void AStarNode::SetParent(AStarNode * pParent)
	{
		m_pParent = pParent;
	}

	AStarNode *AStarNode::GetParentPointer()
	{
		return m_pParent;
	}
}
