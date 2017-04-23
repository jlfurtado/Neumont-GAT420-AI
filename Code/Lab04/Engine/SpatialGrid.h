#ifndef SPATIALGRID_H
#define SPATIALGRID_H

// Justin Furtado
// SpatialGrid.h
// 8/24/2016
// Holds an array of linked lists of SpatialTriangleData used for spatial partitioning

#include "SpatialTriangleData.h"
#include "ExportHeader.h"
#include "LinkedList.h"
#include "InstanceBuffer.h"
#include "GraphicalObject.h"

namespace Engine
{
	class ENGINE_SHARED SpatialGrid
	{
	public:
		SpatialGrid();
		~SpatialGrid();

		bool InitializeDisplayGrid(Vec3 color, void *pCamMat, void *pPerspMat, int tintIntensityLoc, int tintColorLoc, int modelToWorldMatLoc, int worldToViewMatLoc, int perspectiveMatLoc);
		void DrawDebugShapes(const Vec3& centerPos);
		SpatialTriangleData *GetTriangleDataByGrid(int gridX, int gridY, int gridZ);
		SpatialTriangleData *GetTriangleDataByGridAtPosition(float worldX, float worldY, float worldZ);
		bool AddGraphicalObject(GraphicalObject *pGraphicalObjectToAdd);
		int GetGridIndexFromXPos(float xPos);
		int GetGridIndexFromYPos(float yPos);
		int GetGridIndexFromZPos(float zPos);
		int GetGridTriangleCount(int gridX, int gridY, int gridZ);
		float GetGridScale();
		void CalculateStatisticsFromCounts();
		void RemoveGraphicalObject(GraphicalObject *pGobToRemove);
		int GetGridWidth();
		int GetGridHeight();
		int GetGridDepth();
		void ConsoleLogStats();
		bool AddTrianglesToPartitions();
		void EnableObjects();
		void DisableObjects();
		static bool EnableObject(GraphicalObject *pGob, void *pInstance);
		static bool DisableObject(GraphicalObject *pGob, void *pInstance);
		bool DoesFitInGrid(GraphicalObject *pGraphicalObjectToTest);
		void SetGridScale(float newScale);

		// TODO: Move!??!?!?!

	private:
		typedef bool(*TriangleProcessingCallback)(int x, int y, int z, GraphicalObject *pObj, int index, const Vec3& p0, const Vec3& p1, const Vec3& p2, void *pClassInstance);

		struct SpatialCallbackPassData
		{
			Mat4 modelToWorld;
			TriangleProcessingCallback callback;
			GraphicalObject *pObj;
			bool m_success{ true };
		};

		static bool AddGraphicalObjectToGridPassThrough(GraphicalObject *pGraphicalObjectToAdd, void *pClassInstance);
		bool AddGraphicalObjectToGrid(GraphicalObject *pGraphicalObjectToAdd);
		static bool ProcessTrianglesPassThrough(int index, const void *pVert1, const void *pVert2, const void *pVert3, void *pClassInstance, void *pPassThroughData);
		bool ProcessTriangles(int index, const void *pVert1, const void *pVert2, const void *pVert3, void *pPassThroughData);
		static bool SetTriangleIndexPassThrough(int x, int y, int z, GraphicalObject *pObj, int index, const Vec3& p0, const Vec3& p1, const Vec3& p2, void *pClassInstance);
		bool SetTriangleIndex(int x, int y, int z, GraphicalObject *pObj, int index);
		static bool AddSpatialTrianglePassThrough(int x, int y, int z, GraphicalObject *pObj, int index, const Vec3& p0, const Vec3& p1, const Vec3& p2, void *pClassInstance);
		bool AddSpatialTriangle(int x, int y, int z, GraphicalObject *pObj, const Vec3& p0, const Vec3& p1, const Vec3& p2, int index);
		bool SetGridStartIndices();
		static bool SetGridStartIndicesForObjectPassThrough(GraphicalObject *pGraphicalObjectToAdd, void *pClassInstance);
		bool SetGridStartIndicesForObject(GraphicalObject *pCurrent);
		int GetArrayIndexFromXYZIndices(int gridX, int gridY, int gridZ);
		bool AreGridIndicesValid(int gridX, int gridY, int gridZ);
		void CleanUp();

		bool m_firstCalculation{ true };
		float m_gridScale;
		SpatialTriangleData *m_pData{ nullptr };
		LinkedList<GraphicalObject> m_objectList;
		int *m_pGridStartIndices{ nullptr };
		int *m_pGridTriangleCounts{ nullptr };
		int m_gridSectionsWidth{ 85 };
		int m_gridSectionsDepth{ 85 };
		int m_gridSectionsHeight{ 85 };
		int m_totalGridSections{ m_gridSectionsHeight*m_gridSectionsWidth*m_gridSectionsDepth };
		int m_minGridTriangleCount{ -1 };
		int m_maxGridTriangleCount{ -1 };
		int m_totalTriangleCount{ -1 };
		float m_avgGridTriangleCount{ -1.0f };
		InstanceBuffer m_gridInstanceBuffer;
		GraphicalObject m_gridDisplayObject;
	};
}

#endif // ifndef SPATIALGRID_H