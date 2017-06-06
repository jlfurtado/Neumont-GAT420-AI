#include "SteeringBehaviors.h"
#include "MathUtility.h"
#include "Mesh.h"

// Justin Furtado
// 6/2/2017
// SteeringBehaviors.h
// Implements some steering behavoirs

namespace Engine
{
	GraphicalObject *SteeringBehaviors::s_pClosestResource = nullptr;

	void SteeringBehaviors::Seek(SpatialComponent * const pEntitySpatial, const SpatialComponent * const pTargetSpatial, float speed)
	{
		Vec3 moveDir = (pTargetSpatial->GetPosition() - pEntitySpatial->GetPosition()).Normalize();
		pEntitySpatial->SetVelocity(moveDir * speed);
	}

	void SteeringBehaviors::Pursue(SpatialComponent * const pEntitySpatial, const SpatialComponent * const pTargetSpatial, float dt, float speed)
	{
		Vec3 projectedPos = pTargetSpatial->GetPosition() + pTargetSpatial->GetVelocity() * dt;
		Vec3 moveDir = (projectedPos - pEntitySpatial->GetPosition()).Normalize();
		pEntitySpatial->SetVelocity(moveDir * speed);
	}

	void SteeringBehaviors::OffsetPursuit(SpatialComponent * const pEntitySpatial, const SpatialComponent * const pTargetSpatial, float dt, float speed, const Vec3 & offset)
	{
		Vec3 projectedPos = pTargetSpatial->GetPosition() + pTargetSpatial->GetVelocity() * dt;
		Vec3 desiredPos = projectedPos + offset;
		Vec3 moveDir = (desiredPos - pEntitySpatial->GetPosition()).Normalize();
		pEntitySpatial->SetVelocity(moveDir * speed);
	}

	void SteeringBehaviors::OffsetPursuitArrival(SpatialComponent * const pEntitySpatial, const SpatialComponent * const pTargetSpatial, float dt, float speed, float slowRadius, const Vec3 & offset)
	{
		Vec3 projectedPos = pTargetSpatial->GetPosition() + pTargetSpatial->GetVelocity() * dt;
		Vec3 desiredPos = projectedPos + offset;
		Vec3 move = (desiredPos - pEntitySpatial->GetPosition());
		Vec3 modeDir = move.Normalize();
		float dist = move.Length();
		float rampSpeed = speed * (dist / slowRadius);
		float clipSpeed = MathUtility::Min(rampSpeed, speed);
		pEntitySpatial->SetVelocity(clipSpeed / dist * move);
	}

	void SteeringBehaviors::Flee(SpatialComponent * const pEntitySpatial, const SpatialComponent * const pTargetSpatial, float speed)
	{
		Vec3 moveDir = (pEntitySpatial->GetPosition() - pTargetSpatial->GetPosition()).Normalize();
		pEntitySpatial->SetVelocity(moveDir * speed);
	}

	void SteeringBehaviors::Evade(SpatialComponent * const pEntitySpatial, const SpatialComponent * const pTargetSpatial, float dt, float speed)
	{
		Vec3 projectedPos = pTargetSpatial->GetPosition() + pTargetSpatial->GetVelocity() * dt;
		Vec3 moveDir = (pEntitySpatial->GetPosition() - projectedPos).Normalize();
		pEntitySpatial->SetVelocity(moveDir * speed);
	}

	void SteeringBehaviors::Arrival(SpatialComponent * const pEntitySpatial, const SpatialComponent * const pTargetSpatial, float speed, float slowRadius)
	{
		Vec3 move = (pTargetSpatial->GetPosition() - pEntitySpatial->GetPosition());
		Vec3 modeDir = move.Normalize();
		float dist = move.Length();
		float rampSpeed = speed * (dist / slowRadius);
		float clipSpeed = MathUtility::Min(rampSpeed, speed);
		pEntitySpatial->SetVelocity(clipSpeed / dist * move);
	}

	void SteeringBehaviors::Wander(SpatialComponent * const pEntitySpatial, float speed, float radius, float offset)
	{
		Vec3 vel = pEntitySpatial->GetVelocity();
		vel = vel + (vel.Normalize() * offset + MathUtility::GetRandSphereEdgeVec(radius));
		pEntitySpatial->SetVelocity(vel.Normalize() * speed);
	}

	void SteeringBehaviors::Forage(SpatialComponent * const pEntitySpatial, float speed, float wanderRadius, float slowRadius, float seeRadius, float offset, LinkedList<GraphicalObject*>* pResources)
	{
		s_pClosestResource = nullptr;
		Vec3 pos = pEntitySpatial->GetPosition();

		// calculates the closest collectible in the list and stores the ptr
		pResources->WalkListWhere(IsCollectible, nullptr, CalcClosest, &pos);

		Vec3 vel;
		if (s_pClosestResource && (s_pClosestResource->GetPos() - pos).LengthSquared() < seeRadius * seeRadius)
		{
			Vec3 move = (s_pClosestResource->GetPos() - pEntitySpatial->GetPosition());
			Vec3 modeDir = move.Normalize();
			float dist = move.Length();
			float rampSpeed = speed * (dist / slowRadius);
			float clipSpeed = MathUtility::Min(rampSpeed, speed);
			vel = clipSpeed / dist * move;
			if (vel.LengthSquared() < 2.0f*2.0f)
			{
				s_pClosestResource->SetEnabled(false);
			}
		}
		else
		{
			vel = pEntitySpatial->GetVelocity();
			vel = vel + (vel.Normalize() * offset + MathUtility::GetRandSphereEdgeVec(wanderRadius));
			vel = vel.Normalize() * speed;
		}
		pEntitySpatial->SetVelocity(vel);

	}

	bool SteeringBehaviors::IsCollectible(GraphicalObject * pObj, void * /*pData*/)
	{
		return pObj->IsEnabled() && pObj->GetMeshPointer()->GetVertexFormat() == VertexFormat::PositionColor;
	}

	bool SteeringBehaviors::CalcClosest(GraphicalObject * pObj, void * pData)
	{
		Vec3 pos = *reinterpret_cast<Vec3*>(pData);
		if (!s_pClosestResource || (pObj->GetPos() - pos).LengthSquared() < (s_pClosestResource->GetPos() - pos).LengthSquared())
		{
			s_pClosestResource = pObj;
		}

		return true;
	}

}
