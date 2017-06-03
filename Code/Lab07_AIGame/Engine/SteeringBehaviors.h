#ifndef STEERINGBEHAVIORS_H
#define STEERINGBEHAVIORS_H

// Justin Furtado
// 6/2/2017
// SteeringBehaviors.h
// Implements some steering behavoirs

#include "ExportHeader.h"
#include "SpatialComponent.h"
#include "LinkedList.h"
#include "GraphicalObject.h"

namespace Engine
{
	class ENGINE_SHARED SteeringBehaviors
	{
	public:
		static void Seek(SpatialComponent *const pEntitySpatial, const SpatialComponent *const pTargetSpatial, float speed);
		static void Pursue(SpatialComponent *const pEntitySpatial, const SpatialComponent *const pTargetSpatial, float dt, float speed);
		static void OffsetPursuit(SpatialComponent *const pEntitySpatial, const SpatialComponent *const pTargetSpatial, float dt, float speed, const Vec3& offset);
		static void OffsetPursuitArrival(SpatialComponent *const pEntitySpatial, const SpatialComponent *const pTargetSpatial, float dt, float speed, float slowRadius, const Vec3& offset);
		static void Flee(SpatialComponent *const pEntitySpatial, const SpatialComponent *const pTargetSpatial, float speed);
		static void Evade(SpatialComponent *const pEntitySpatial, const SpatialComponent *const pTargetSpatial, float dt, float speed);
		static void Arrival(SpatialComponent *const pEntitySpatial, const SpatialComponent *const pTargetSpatial, float speed, float slowRadius);
		static void Wander(SpatialComponent *const pEntitySpatial, float speed, float radius, float offset);
		static void Forage(SpatialComponent *const pEntitySpatial, float speed,  float wanderRadius, float slowRadius, float seeRadius, float offset, LinkedList<GraphicalObject*> *pResources);

	private:
		static bool IsCollectible(GraphicalObject *pObj, void *pData);
		static bool CalcClosest(GraphicalObject *pObj, void *pData);
		static GraphicalObject *s_pClosestResource;
	};
}


#endif // ifndef STEERINGBEHAVIORS_H