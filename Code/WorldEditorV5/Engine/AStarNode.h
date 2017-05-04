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
#include "CollisionTester.h"

namespace Engine
{
	class ENGINE_SHARED AStarNode
	{
	public:
		AStarNode();
		~AStarNode();

		Vec3 GetPosition();
		void SetPosition(const Vec3& newPosition);
		float GetRadius();
		void SetRadius(float newRadius);
		void SetNodeEnabled(bool enabled);
		bool IsEnabled();
		void SetParent(AStarNode *pParent);
		AStarNode *GetParentPointer();

	private:
		AStarNode* m_pParent{ nullptr };
		bool m_enabled{ true };
		Vec3 m_position;
		float m_radius{ 1.0f };
	};
	
}


#endif // ifndef ASTARNODE_H