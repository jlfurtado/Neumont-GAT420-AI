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
		void DrawDebugShapes();
		SpatialTriangleData *GetTriangleDataByGrid(int gridX, int gridZ);
		SpatialTriangleData *GetTriangleDataByGridAtPosition(float worldX, float worldZ);
		bool AddGraphicalObject(GraphicalObject *pGraphicalObjectToAdd);
		float GetGridScale();
		int GetGridTriangleCount(int gridX, int gridZ);
		void CalculateStatisticsFromCounts();
		void RemoveGraphicalObject(GraphicalObject *pGobToRemove);
		int GetGridWidth();
		int GetGridHeight();
		void ConsoleLogStats();
		bool AddTrianglesToPartitions();

		// TODO: Move!??!?!?!

	private:
		static bool AddGraphicalObjectToGridPassThrough(GraphicalObject *pGraphicalObjectToAdd, void *pClassInstance);
		bool AddGraphicalObjectToGrid(GraphicalObject *pGraphicalObjectToAdd);
		bool SetGridStartIndices();
		static bool SetGridStartIndicesForObjectPassThrough(GraphicalObject *pGraphicalObjectToAdd, void *pClassInstance);
		bool SetGridStartIndicesForObject(GraphicalObject *pCurrent);
		void CleanUp();

		float m_gridScale;
		SpatialTriangleData *m_pData{ nullptr };
		LinkedList<GraphicalObject> m_objectList;
		int *m_pGridStartIndices{ nullptr };
		int *m_pGridTriangleCounts{ nullptr };
		int m_gridSectionsWidth{ 85 };
		int m_gridSectionsHeight{ 85 };
		int m_totalGridSections{ m_gridSectionsHeight*m_gridSectionsWidth };
		int m_minGridTriangleCount{ -1 };
		int m_maxGridTriangleCount{ -1 };
		int m_totalTriangleCount{ -1 };
		float m_avgGridTriangleCount{ -1.0f };
		InstanceBuffer m_gridInstanceBuffer;
		GraphicalObject m_gridDisplayObject;
	};
}

#endif // ifndef SPATIALGRID_H