#ifndef FLOCKER_H
#define FLOCKER_H

// Justin Furtado
// 6/2/2017
// Flocker.h
// Helper class for flocking

#include "ExportHeader.h"
#include "SpatialComponent.h"
#include "LinkedList.h"

namespace Engine
{
	class ENGINE_SHARED Flocker
	{
	public:
		static void AddToFlock(SpatialComponent *pSpatial);
		static void Flock(SpatialComponent *pSpatial, float cohesionWeight, float alignmentWeight, float separationWeight, float speed);
		static void RemoveFromFlock(SpatialComponent *pSpatial);

	private:
		static bool CalculateCohesion(SpatialComponent *pSpatial, void *pData);
		static bool CalculateSeparation(SpatialComponent *pSpatial, void *pData);
		static bool CalculateAlignment(SpatialComponent *pSpatial, void *pData);

		static Vec3 CalculateCohesion(SpatialComponent *pSpatial);
		static Vec3 CalculateSeparation(SpatialComponent *pSpatial);
		static Vec3 CalculateAlignment(SpatialComponent *pSpatial);
		static bool IsInRange(SpatialComponent *pSpatial, void *pData);

		// data
		static Engine::LinkedList<Engine::SpatialComponent *> s_flock;
		static Engine::Vec3 s_workVec;
		static int s_workCount;
	};

}

#endif // ifndef FLOCKER_H