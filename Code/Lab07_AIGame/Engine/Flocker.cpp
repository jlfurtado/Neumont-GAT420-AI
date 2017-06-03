#include "Flocker.h"

// Justin Furtado
// 6/2/2017
// Flocker.h
// Helper class for flocking

namespace Engine
{
	LinkedList<Engine::SpatialComponent*> Flocker::s_flock;
	Vec3 Flocker::s_workVec;
	int Flocker::s_workCount;

	void Flocker::AddToFlock(SpatialComponent * pSpatial)
	{
		s_flock.AddToListFront(pSpatial);
	}

	void Flocker::Flock(SpatialComponent * pSpatial, float cohesionWeight, float alignmentWeight, float separationWeight, float speed)
	{
		Vec3 pos = pSpatial->GetPosition();
		
		Vec3 vel = (cohesionWeight * CalculateCohesion(pSpatial))
			+ (alignmentWeight * CalculateAlignment(pSpatial))
			+ (separationWeight * CalculateSeparation(pSpatial));

		pSpatial->SetVelocity((pSpatial->GetVelocity().Normalize() * 0.25f+ vel.Normalize()).Normalize() * speed);
	}

	void Flocker::RemoveFromFlock(SpatialComponent * pSpatial)
	{
		s_flock.RemoveFirstFromList(pSpatial);
	}

	bool Flocker::CalculateCohesion(SpatialComponent * pSpatial, void * pData)
	{
		if (pSpatial != pData) { s_workVec = s_workVec + pSpatial->GetPosition(); s_workCount++; }
		return true;
	}

	bool Flocker::CalculateSeparation(SpatialComponent * pSpatial, void * pData)
	{
		if (pSpatial != pData) { s_workVec = s_workVec + (pSpatial->GetPosition() - reinterpret_cast<SpatialComponent*>(pData)->GetPosition()); s_workCount++; }
		return true;
	}

	bool Flocker::CalculateAlignment(SpatialComponent * pSpatial, void * pData)
	{
		if (pSpatial != pData) { s_workVec = s_workVec + pSpatial->GetVelocity(); s_workCount++; }
		return true;
	}

	Vec3 Flocker::CalculateCohesion(SpatialComponent *pSpatial)
	{
		Engine::Vec3 p = pSpatial->GetPosition();
		s_workVec = Engine::Vec3(0.0f);
		s_workCount = 0;
		s_flock.WalkListWhere(Flocker::IsInRange, &p, Flocker::CalculateCohesion, pSpatial);

		if (s_workCount > 0)
		{
			s_workVec = (s_workVec / (1.0f * s_workCount) - p).Normalize();
		}

		return s_workVec;
	}

	Vec3 Flocker::CalculateSeparation(SpatialComponent *pSpatial)
	{
		Engine::Vec3 p = pSpatial->GetPosition();
		s_workVec = Engine::Vec3(0.0f);
		s_workCount = 0;
		s_flock.WalkListWhere(Flocker::IsInRange, &p, Flocker::CalculateSeparation, pSpatial);

		if (s_workCount > 0)
		{
			s_workVec = (-(s_workVec / (1.0f * s_workCount))).Normalize();
		}

		return s_workVec;
	}

	Vec3 Flocker::CalculateAlignment(SpatialComponent *pSpatial)
	{
		Engine::Vec3 p = pSpatial->GetPosition();
		s_workVec = Engine::Vec3(0.0f);
		s_workCount = 0;
		s_flock.WalkListWhere(Flocker::IsInRange, &p, Flocker::CalculateAlignment, pSpatial);

		if (s_workCount > 0)
		{
			s_workVec = (s_workVec / (1.0f * s_workCount)).Normalize();
		}

		return s_workVec;
	}

	const float range = 500.0f;
	const float rangeSquared = range * range;
	bool Flocker::IsInRange(SpatialComponent * pSpatial, void * pData)
	{
		return (pSpatial->GetPosition() - *reinterpret_cast<Vec3*>(pData)).LengthSquared() < rangeSquared;
	}

}
