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
		m_pSpatialComp = GetSiblingComponent<SpatialComponent>();
		if (!m_pSpatialComp) { GameLogger::Log(MessageType::cError, "Cannot follow path! Entity has AStarPathFollowComponent but no SpatialComponent!\n"); return false; }

		m_pGobComp = GetSiblingComponent<GraphicalObjectComponent>();
		if (!m_pGobComp) { GameLogger::Log(MessageType::cError, "Cannot follow path! Entity has AStarPathFollowComponent but no GobComponent!\n"); return false; }

		SetColorFromState();

		return true;
	}

	bool AStarPathFollowComponent::Update(float /*dt*/)
	{
		Vec3 pos = m_pSpatialComp->GetPosition();

		if (m_nextPathIndex >= m_pathSize)
		{
			if (m_randomTargetNode)
			{
				delete[] followingPath; followingPath = nullptr;
			}
			else
			{
				m_pSpatialComp->SetVelocity(Vec3(0.0f));
				HandleRecalcAtNext();
			}
		 }
		
		if (!followingPath)
		{ 
			int toPos = m_randomTargetNode ? MathUtility::Rand(0, m_pNodeMap->GetNumNodes()) : m_closestToTarget;
			int fromPos = m_pNodeMap->FindNearestNodeIndex(pos);
			if (fromPos != toPos)
			{
				followingPath = AStarPathFinder::FindPath(m_pNodeMap, fromPos, toPos, &m_pathSize);
				m_nextPathIndex = 0;
			}
		}

		if (followingPath && m_nextPathIndex < m_pathSize)
		{
			Vec3 fromNodePos = m_nextPathIndex == 0 ? pos : (m_pNodeMap->GetConnectedNodes() + followingPath[m_nextPathIndex - 1])->m_pNode->GetPosition();
			Vec3 nextNodePos = (m_pNodeMap->GetConnectedNodes() + followingPath[m_nextPathIndex])->m_pNode->GetPosition();
			Vec3 toNextNode = nextNodePos - pos;
			Vec3 fromNode = nextNodePos - fromNodePos;

			if (toNextNode.Normalize().Dot(fromNode.Normalize()) < 0.01f || toNextNode.LengthSquared() < OK_DISTANCE_SQUARED) 
			{
				++m_nextPathIndex;
				HandleRecalcAtNext();
				m_pSpatialComp->SetVelocity(Vec3(0.0f));
			}
			else
			{
				m_pSpatialComp->SetVelocity(toNextNode.Normalize() * 50.0f);	
				m_pGobComp->GetGraphicalObject()->SetRotMat(Mat4::AxisRotation(toNextNode.Normalize(), toNextNode.Cross(PLUS_Y).Cross(toNextNode).Normalize()));
				m_pGobComp->GetGraphicalObject()->CalcFullTransform();
			}
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

	void AStarPathFollowComponent::SetRandomTargetNode(bool randomTargetNode)
	{
		m_randomTargetNode = randomTargetNode;
		m_recalcAtNextNode = true;

		SetColorFromState();
	}

	void AStarPathFollowComponent::SetFollowPos(const Vec3 & followPos)
	{
		m_followPos = followPos;

		int newTarget = m_pNodeMap->FindNearestNodeIndex(followPos);
		if (newTarget != m_closestToTarget)
		{
			m_closestToTarget = newTarget;
			m_recalcAtNextNode = true;
		}
	}

	void AStarPathFollowComponent::HandleRecalcAtNext()
	{
		if (m_recalcAtNextNode)
		{
			if (followingPath) { delete[] followingPath; followingPath = nullptr; }
			m_recalcAtNextNode = false;
		}
	}

	void AStarPathFollowComponent::SetColorFromState()
	{
		// only need to change color on state change
		m_pGobComp->GetGraphicalObject()->GetMatPtr()->m_ambientReflectivity = m_randomTargetNode ? Vec3(0.1f, 0.1f, 0.0f) : Vec3(0.1f, 0.0f, 0.0f);
		m_pGobComp->GetGraphicalObject()->GetMatPtr()->m_diffuseReflectivity = m_randomTargetNode ? Vec3(0.7f, 0.7f, 0.0f) : Vec3(0.7f, 0.0f, 0.0f);
		m_pGobComp->GetGraphicalObject()->GetMatPtr()->m_specularReflectivity = m_randomTargetNode ? Vec3(0.1f, 0.1f, 0.0f) : Vec3(0.1f, 0.0f, 0.0f);
	}


}
