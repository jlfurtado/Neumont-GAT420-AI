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
	class GraphicalObjectComponent;
	class SpatialComponent;
	class ENGINE_SHARED AStarPathFollowComponent : public Component
	{
	public:
		AStarPathFollowComponent();
		~AStarPathFollowComponent();

		bool Initialize() override;
		bool Update(float dt) override;
		void SetNodeMapPtr(AStarNodeMap *pNodeMap);
		void SetCheckLayer(CollisionLayer layer);
		void SetRandomTargetNode(bool randomTargetNode);
		void SetFollowPos(const Vec3& followPos);
		void ForceRecalc(const Vec3 & followPos);

	private:
		void HandleRecalcAtNext();
		void SetColorFromState();
		
		int *followingPath = nullptr;
		int m_pathSize = 0;
		int m_nextPathIndex = 0;
		AStarNodeMap *m_pNodeMap;
		CollisionLayer m_checkLayer;
		Vec3 m_followPos;
		int m_closestToTarget{ 0 };
		bool m_randomTargetNode{ true };
		bool m_recalcAtNextNode{ false };
		GraphicalObjectComponent *m_pGobComp{ nullptr };
		SpatialComponent *m_pSpatialComp{ nullptr };
	};

}

#endif // ifndef ASTARPATHFOLLOWCOMPONENT_H