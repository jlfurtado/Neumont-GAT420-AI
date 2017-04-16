#ifndef COLLISIONTESTER_H
#define COLLISIONTESTER_H

#include "SpatialGrid.h"
#include "Vec3.h"
#include "ExportHeader.h"

// Justin Furtado
// 8/21/2016
// CollisionTester.h
// Does ray casting

namespace Engine
{
	class GraphicalObject;
	class Entity;

	struct ENGINE_SHARED RayCastingOutput
	{
		bool m_didIntersect{ false };
		float m_distance{ 999999.0f };
		Vec3 m_triangleNormal{ 0.0f, 0.0f, 0.0f };
		Vec3 m_alphaBetaGamma{ 0.0f, 0.0f, 0.0f };
		int m_vertexIndex{ -1 };
		GraphicalObject *m_belongsTo{ nullptr };
		Vec3 m_intersectionPoint{ 0.0f, 0.0f, 0.0f };
	};

	enum class ENGINE_SHARED CollisionLayer
	{
		STATIC_GEOMETRY = 0,
		LAYER_1,
		LAYER_2,
		LAYER_3,
		LAYER_4,
		
		NUM_LAYERS // LAST ON PURPOSE
	};

	class ENGINE_SHARED CollisionTester
	{
	public:
		static const char *LayerString(CollisionLayer layer);
		static bool InitializeGridDebugShapes(CollisionLayer gridLayer, Vec3 color, void *pCamMat, void *pPerspMat, int tintIntensityLoc, int tintColorLoc, int modelToWorldMatLoc, int worldToViewMatLoc, int perspectiveMatLoc);
		static void DrawGrid(CollisionLayer gridLayer);
		static void ConsoleLogOutput();
		static RayCastingOutput FindWall(const Vec3& rayPosition, const Vec3& rayDirection, float checkDist, CollisionLayer layer = CollisionLayer::NUM_LAYERS);
		static RayCastingOutput FindWall(Entity *pEntity, float checkDist, CollisionLayer layer = CollisionLayer::NUM_LAYERS);
		static RayCastingOutput FindFloor(Entity *pEntity, float checkDist, CollisionLayer layer = CollisionLayer::NUM_LAYERS);
		static RayCastingOutput FindCeiling(Entity *pEntity, float checkDist, CollisionLayer layer = CollisionLayer::NUM_LAYERS);
		static RayCastingOutput RayTriangleIntersect(const Vec3& rayPosition, const Vec3& rayDirection, const Vec3& p0, const Vec3& p1, const Vec3& p2, float currentClosest);
		static bool AddGraphicalObjectToLayer(GraphicalObject *pGraphicalObjectToAdd, CollisionLayer layer);
		static void RemoveGraphicalObjectFromLayer(GraphicalObject *pGobToRemove, CollisionLayer layer);
		static int GetTriangleCountForSpace(float xPos, float zPos, CollisionLayer layer = CollisionLayer::NUM_LAYERS);
		static int GetGridIndexFromPosX(float xPos, CollisionLayer layer);
		static int GetGridIndexFromPosZ(float zPos, CollisionLayer layer);
		static RayCastingOutput FindFromMousePos(int pixelX, int pixelY, float checkDist, CollisionLayer layer = CollisionLayer::NUM_LAYERS);
		static bool CalculateGrid(CollisionLayer layer = CollisionLayer::NUM_LAYERS);
		static void OnlyShowLayer(CollisionLayer layer = CollisionLayer::NUM_LAYERS);

	private:
		static SpatialGrid s_spatialGrids[(unsigned)CollisionLayer::NUM_LAYERS];

	};
}

#endif // ifndef COLLISIONTESTER_H