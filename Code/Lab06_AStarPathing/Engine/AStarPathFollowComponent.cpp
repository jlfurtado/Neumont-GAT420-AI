#include "AStarPathFollowComponent.h"
#include "SpatialComponent.h"
#include "MathUtility.h"
#include "GraphicalObjectComponent.h"
#include "CollisionTester.h"


// Justin Furtado
// 5/13/2017
// AStarPathFollowComponent.h
// Lets an entity follow An AStarPath

namespace Engine
{
	const float OK_DISTANCE = 1.0f;
	const float OK_DISTANCE_SQUARED = OK_DISTANCE * OK_DISTANCE;
	const Vec3 PLUS_X(1.0f, 0.0f, 0.0f);
	const Vec3 PLUS_Y(0.0f, 1.0f, 0.0f);
	const Vec3 PLUS_Z(0.0f, 0.0f, 1.0f);

	AStarPathFollowComponent::AStarPathFollowComponent()
	{
	}

	AStarPathFollowComponent::~AStarPathFollowComponent()
	{
		if (followingPath) { delete[] followingPath; }
	}

	bool AStarPathFollowComponent::Initialize()
	{
		return true;
	}

	bool AStarPathFollowComponent::Update(float dt)
	{
		SpatialComponent *pSpatial = GetSiblingComponent<SpatialComponent>();
		if (!pSpatial) { GameLogger::Log(MessageType::cError, "Cannot follow path! Entity has AStarPathFollowComponent but no SpatialComponent!\n"); return true; }
		GraphicalObjectComponent *pGob = GetSiblingComponent<GraphicalObjectComponent>();
		if (!pGob) { GameLogger::Log(MessageType::cError, "Cannot follow path! Entity has AStarPathFollowComponent but no GobComponent!\n"); return true; }

		Vec3 pos = pSpatial->GetPosition();

		if (m_nextPathIndex >= m_pathSize) { delete[] followingPath; followingPath = nullptr; }
		while (!followingPath) { followingPath = AStarPathFinder::FindPath(m_pNodeMap, m_pNodeMap->FindNearestNodeIndex(pos), MathUtility::Rand(0, m_pNodeMap->GetNumNodes()), &m_pathSize); m_nextPathIndex = 0; }

		Vec3 nextNodePos = (m_pNodeMap->GetConnectedNodes() + followingPath[m_nextPathIndex])->m_pNode->GetPosition();
		Vec3 toNextNode = nextNodePos - pos;
		
		//GameLogger::Log(MessageType::ConsoleOnly, "[%s]: [%d]\n", this->m_owner->GetName(), m_nextPathIndex);

		if (toNextNode.LengthSquared() < OK_DISTANCE_SQUARED)
		{
			++m_nextPathIndex;
		}
		else
		{
			pSpatial->SetVelocity(toNextNode.Normalize() * 10.0f);
			RayCastingOutput rco = CollisionTester::FindFloor(m_owner, 5.0f, m_checkLayer);
			Mat4 rtf;
			if (rco.m_didIntersect) 
			{
				Vec3 f = -toNextNode.Normalize();
				Vec3 u = rco.m_triangleNormal;
				Vec3 r = -f.Cross(u);

				rtf = Mat4(r.GetX(), u.GetX(), f.GetX(), 0.0f,
						   r.GetY(), u.GetY(), f.GetY(), 0.0f,
						   r.GetZ(), u.GetZ(), f.GetZ(), 0.0f, 
						   0.0f, 0.0f, 0.0f, 1.0f);
			}
			else { rtf = Mat4::RotationToFace(-PLUS_Z, toNextNode.Normalize()); }
			pGob->GetGraphicalObject()->SetRotMat(rtf);
			pGob->GetGraphicalObject()->CalcFullTransform();
		}

		return true;
	}

	void AStarPathFollowComponent::SetNodeMapPtr(AStarNodeMap * pNodeMap)
	{
		m_pNodeMap = pNodeMap;
	}

	void AStarPathFollowComponent::SetCheckLayer(CollisionLayer layer)
	{
		m_checkLayer = layer;
	}

}
