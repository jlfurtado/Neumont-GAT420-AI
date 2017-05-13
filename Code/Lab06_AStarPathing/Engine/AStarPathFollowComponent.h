#ifndef ASTARPATHFOLLOWCOMPONENT_H
#define ASTARPATHFOLLOWCOMPONENT_H

// Justin Furtado
// 5/13/2017
// AStarPathFollowComponent.h
// Lets an entity follow An AStarPath

#include "ExportHeader.h"
#include "Component.h"
#include "AStarPathFinder.h"
#include "CollisionTester.h"

namespace Engine
{
	class ENGINE_SHARED AStarPathFollowComponent : public Component
	{
	public:
		AStarPathFollowComponent();
		~AStarPathFollowComponent();

		bool Initialize() override;
		bool Update(float dt) override;
		void SetNodeMapPtr(AStarNodeMap *pNodeMap);
		void SetCheckLayer(CollisionLayer layer);

	private:
		int *followingPath = nullptr;
		int m_pathSize = 0;
		int m_nextPathIndex = 0;
		AStarNodeMap *m_pNodeMap;
		CollisionLayer m_checkLayer;
	};

}

#endif // ifndef ASTARPATHFOLLOWCOMPONENT_H