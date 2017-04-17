#include "SpatialGrid.h"
#include "GameLogger.h"
#include "Mesh.h"
#include "GraphicalObject.h"
#include "Vec3.h"
#include "Vec4.h"
#include "ShapeGenerator.h"
#include "RenderEngine.h"
#include "MathUtility.h"

// Justin Furtado
// SpatialGrid.h
// 8/24/2016
// Holds an array of linked lists of SpatialTriangleData used for spatial partitioning

// TODO: Refactor out duplicate code	

namespace Engine
{
	const float SPATIAL_GRID_SIZE = 50.0f;
	SpatialGrid::SpatialGrid()
		: m_gridScale(SPATIAL_GRID_SIZE)
	{
	}

	SpatialGrid::~SpatialGrid()
	{
		CleanUp();
	}

	bool SpatialGrid::InitializeDisplayGrid(Vec3 color, void *pCamMat, void *pPerspMat, int tintIntensityLoc, int tintColorLoc, int modelToWorldMatLoc, int worldToViewMatLoc, int perspectiveMatLoc)
	{
		Engine::ShapeGenerator::MakeDebugCube(&m_gridDisplayObject, color);

		m_gridDisplayObject.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_gridDisplayObject.GetFullTransformPtr(), modelToWorldMatLoc));
		m_gridDisplayObject.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pCamMat, worldToViewMatLoc));
		m_gridDisplayObject.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pPerspMat, perspectiveMatLoc));
		m_gridDisplayObject.AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &m_gridDisplayObject.GetMatPtr()->m_materialColor, tintColorLoc));
		m_gridDisplayObject.AddUniformData(Engine::UniformData(GL_FLOAT, &m_gridDisplayObject.GetMatPtr()->m_specularIntensity, tintIntensityLoc));
		m_gridDisplayObject.GetMatPtr()->m_materialColor = color;

		GLuint numSections = GetGridWidth() * GetGridHeight();

		Mat4* modelMatrices;
		modelMatrices = new Engine::Mat4[numSections]{ Mat4() };

		for (GLuint i = 0; i < numSections; ++i)
		{
			modelMatrices[i] = Mat4::Translation(MathUtility::GetQuadification(i, GetGridWidth(), GetGridHeight(), GetGridScale())) * Mat4::Scale(m_gridScale, 0.0001f, m_gridScale);
		}

		m_gridInstanceBuffer.Initialize(modelMatrices, 16 * sizeof(float), numSections, 16 * numSections);

		delete[] modelMatrices;

		m_gridDisplayObject.CalcFullTransform();

		Engine::RenderEngine::AddGraphicalObject(&m_gridDisplayObject);
		m_gridDisplayObject.SetEnabled(false);
		return true;
	}

	void SpatialGrid::DrawDebugShapes()
	{
		m_gridDisplayObject.SetEnabled(true);
		Engine::RenderEngine::DrawInstanced(&m_gridDisplayObject, &m_gridInstanceBuffer);
		m_gridDisplayObject.SetEnabled(false);
	}

	SpatialTriangleData * SpatialGrid::GetTriangleDataByGrid(int gridX, int gridZ)
	{
		if (gridX < 0 || gridX >= m_gridSectionsWidth || gridZ < 0 || gridZ >= m_gridSectionsHeight) { return nullptr; }
		int i = gridZ*m_gridSectionsWidth + gridX;
		//GameLogger::Log(MessageType::ConsoleOnly, "i = %d\n", i);
		return &m_pData[m_pGridStartIndices[i]];
	}

	SpatialTriangleData * SpatialGrid::GetTriangleDataByGridAtPosition(float worldX, float worldZ)
	{
		return GetTriangleDataByGrid(int(worldX / m_gridScale - 0.5f*m_gridSectionsWidth), int(worldZ / m_gridScale - 0.5f*m_gridSectionsHeight));
	}

	bool SpatialGrid::AddGraphicalObject(GraphicalObject * pGraphicalObjectToAdd)
	{
		m_totalTriangleCount += pGraphicalObjectToAdd->GetMeshPointer()->GetVertexCount() / 3;
		m_objectList.AddToList(pGraphicalObjectToAdd);
		return true;
	}

	int SpatialGrid::GetGridIndexFromXPos(float xPos)
	{
		return (int)floorf((xPos / m_gridScale) + (m_gridSectionsWidth*0.5f));
	}

	int SpatialGrid::GetGridIndexFromZPos(float zPos)
	{
		return (int)floorf((zPos / m_gridScale) + (m_gridSectionsHeight*0.5f));
	}

	int SpatialGrid::GetGridTriangleCount(int gridX, int gridZ)
	{
		if (gridX < 0 || gridX >= m_gridSectionsWidth || gridZ < 0 || gridZ >= m_gridSectionsHeight) { return -1; }
		int i = gridZ*m_gridSectionsWidth + gridX;

		return m_pGridTriangleCounts[i];
	}

	float SpatialGrid::GetGridScale()
	{
		return m_gridScale;
	}

	void SpatialGrid::CalculateStatisticsFromCounts()
	{
		m_totalTriangleCount = 0;
		m_minGridTriangleCount = -1;
		m_maxGridTriangleCount = -1;

		for (int i = 0; i < m_totalGridSections; ++i)
		{
			if (m_pGridTriangleCounts[i] < m_minGridTriangleCount || m_minGridTriangleCount < 0)
			{
				m_minGridTriangleCount = m_pGridTriangleCounts[i];
			}

			if (m_pGridTriangleCounts[i] > m_maxGridTriangleCount || m_maxGridTriangleCount < 0)
			{
				m_maxGridTriangleCount = m_pGridTriangleCounts[i];
			}

			m_totalTriangleCount += m_pGridTriangleCounts[i];
		}

		m_avgGridTriangleCount = (float)m_totalTriangleCount / (float)m_totalGridSections;
	}

	void SpatialGrid::RemoveGraphicalObject(GraphicalObject * pGobToRemove)
	{
		m_objectList.RemoveFromList(pGobToRemove);
	}

	int SpatialGrid::GetGridWidth()
	{
		return m_gridSectionsWidth;
	}

	int SpatialGrid::GetGridHeight()
	{
		return m_gridSectionsHeight;
	}

	void SpatialGrid::ConsoleLogStats()
	{
		GameLogger::Log(MessageType::cDebug, "Total triangle count for grid is [%d]\n", m_totalTriangleCount);
		GameLogger::Log(MessageType::cDebug, "Average triangle count for grid cells is [%.3f]\n", m_avgGridTriangleCount);
		GameLogger::Log(MessageType::cDebug, "Min triangle count for grid is [%d]\n", m_minGridTriangleCount);
		GameLogger::Log(MessageType::cDebug, "Max triangle count for grid is [%d]\n", m_maxGridTriangleCount);
	}

	bool SpatialGrid::AddTrianglesToPartitions()
	{
		// allow method to be called repeatedly
		if (!m_firstCalculation) { CleanUp(); }
		else { m_firstCalculation = false; }

		m_pGridStartIndices = new int[m_totalGridSections] {0};
		m_pGridTriangleCounts = new int[m_totalGridSections] {0};
		if (!m_pGridStartIndices) { GameLogger::Log(MessageType::cFatal_Error, "Failed to allocate memory for [%d] ints!\n", m_totalGridSections); return false; }
		if (!m_pGridTriangleCounts) { GameLogger::Log(MessageType::cFatal_Error, "Failed to allocate memory for [%d] ints!\n", m_totalGridSections); return false; }

		if (!SetGridStartIndices()) { GameLogger::Log(MessageType::cFatal_Error, "Failed to set grid start indices!\n"); return false; }

		CalculateStatisticsFromCounts();
		m_pData = new SpatialTriangleData[m_totalTriangleCount];
		if (!m_pData) { GameLogger::Log(MessageType::cFatal_Error, "Failed to allocate memory for [%d] triangles!\n", m_totalTriangleCount); return false; }

		for (int i = 0; i < m_totalGridSections; ++i)
		{
			m_pGridTriangleCounts[i] = 0;
		}

		if (!m_objectList.WalkList(SpatialGrid::AddGraphicalObjectToGridPassThrough, this)) { return false; }

		GameLogger::Log(MessageType::Process, "Successfully re-calculated spatial grid!\n");
		return true;
	}

	void SpatialGrid::EnableObjects()
	{
		m_objectList.WalkList(SpatialGrid::EnableObject, nullptr);
	}

	void SpatialGrid::DisableObjects()
	{
		m_objectList.WalkList(SpatialGrid::DisableObject, nullptr);
	}

	bool SpatialGrid::EnableObject(GraphicalObject * pGob, void * /*pInstance*/)
	{
		pGob->SetEnabled(true);
		return true;
	}

	bool SpatialGrid::DisableObject(GraphicalObject * pGob, void * /*pInstance*/)
	{
		pGob->SetEnabled(false);
		return true;
	}

	bool SpatialGrid::SetGridStartIndices()
	{
		if (!m_objectList.WalkList(SpatialGrid::SetGridStartIndicesForObjectPassThrough, this)) {
			return false;
		}

		int c = 0;
		for (int x = 0; x < m_gridSectionsWidth; ++x)
		{
			for (int z = 0; z < m_gridSectionsHeight; ++z)
			{
				m_pGridStartIndices[z*m_gridSectionsWidth + x] = c;
				c += m_pGridTriangleCounts[z*m_gridSectionsWidth + x];
			}
		}

		// indicate success
		GameLogger::Log(MessageType::Process, "Successfully added GraphicalObject to SpatialGrid!\n");
		return true;
	}

	bool SpatialGrid::SetGridStartIndicesForObjectPassThrough(GraphicalObject * pGraphicalObjectToAdd, void * pClassInstance)
	{
		SpatialGrid *pSpatialGrid = reinterpret_cast<SpatialGrid*>(pClassInstance);
		return pSpatialGrid->SetGridStartIndicesForObject(pGraphicalObjectToAdd);
	}

	bool SpatialGrid::SetGridStartIndicesForObject(GraphicalObject * pCurrent)
	{
		// model to world matrix
		Mat4 modelToWorld = *pCurrent->GetFullTransformPtr();

		SpatialCallbackPassData data;
		data.modelToWorld = modelToWorld;
		data.callback = SpatialGrid::SetTriangleIndexPassThrough;
		data.pObj = pCurrent;

		pCurrent->GetMeshPointer()->WalkTriangles(SpatialGrid::ProcessTrianglesPassThrough, this, &data);

		return data.m_success;
	}
	
	bool SpatialGrid::AddGraphicalObjectToGridPassThrough(GraphicalObject * pGraphicalObjectToAdd, void * pClassInstance)
	{
		SpatialGrid	*pInstance = reinterpret_cast<SpatialGrid*>(pClassInstance);
		return pInstance->AddGraphicalObjectToGrid(pGraphicalObjectToAdd);
	}

	bool SpatialGrid::AddGraphicalObjectToGrid(GraphicalObject * pGraphicalObjectToAdd)
	{
		// model to world matrix
		Mat4 modelToWorld = *pGraphicalObjectToAdd->GetFullTransformPtr();

		SpatialCallbackPassData data;
		data.modelToWorld = modelToWorld;
		data.callback = SpatialGrid::AddSpatialTrianglePassThrough;
		data.pObj = pGraphicalObjectToAdd;

		pGraphicalObjectToAdd->GetMeshPointer()->WalkTriangles(SpatialGrid::ProcessTrianglesPassThrough, this, &data);

		// indicate success
		GameLogger::Log(MessageType::Process, "Successfully added GraphicalObject to SpatialGrid!\n");
		return data.m_success;
	}

	bool SpatialGrid::ProcessTrianglesPassThrough(int index, const void * pVert1, const void * pVert2, const void * pVert3, void * pClassInstance, void * pPassThroughData)
	{
		SpatialGrid *pInstance = reinterpret_cast<SpatialGrid *>(pClassInstance);
		return pInstance->ProcessTriangles(index, pVert1, pVert2, pVert3, pPassThroughData);
	}

	bool SpatialGrid::ProcessTriangles(int index, const void * pVert1, const void * pVert2, const void * pVert3, void * pPassThroughData)
	{
		SpatialCallbackPassData *pData = reinterpret_cast<SpatialCallbackPassData*>(pPassThroughData);

		// grab the vertex positions regardless of format
		Vec3 p0 = pData->modelToWorld * (*(reinterpret_cast<const Vec3 *>(pVert1)));
		Vec3 p1 = pData->modelToWorld * (*(reinterpret_cast<const Vec3 *>(pVert2)));
		Vec3 p2 = pData->modelToWorld * (*(reinterpret_cast<const Vec3 *>(pVert3)));

		// extract the leftmost, upmost, downmost, rightmost grid indices for which the bounding box of the triangle enters
		// TODO: investigate the case where a triangle is added to a grid in which the bounding quad intersects but the triangle does not
		// NOTE: if the grid size is larger than the triangle size, the triangle can be in at most four grid spaces, and the above would only have a small effect
		float offsetX = 0.5f*m_gridSectionsWidth;
		float offsetZ = 0.5f*m_gridSectionsHeight;
		int minGridX = (int)fminf(fminf(p0.GetX() / m_gridScale + offsetX, p1.GetX() / m_gridScale + offsetX), p2.GetX() / m_gridScale + offsetX);
		int minGridZ = (int)fminf(fminf(p0.GetZ() / m_gridScale + offsetZ, p1.GetZ() / m_gridScale + offsetZ), p2.GetZ() / m_gridScale + offsetZ);
		int maxGridX = (int)fmaxf(fmaxf(p0.GetX() / m_gridScale + offsetX, p1.GetX() / m_gridScale + offsetX), p2.GetX() / m_gridScale + offsetX);
		int maxGridZ = (int)fmaxf(fmaxf(p0.GetZ() / m_gridScale + offsetZ, p1.GetZ() / m_gridScale + offsetZ), p2.GetZ() / m_gridScale + offsetZ);

		// error checking
		if (minGridX < 0 || minGridX >= m_gridSectionsWidth || minGridZ < 0 || minGridZ >= m_gridSectionsHeight || maxGridX < 0 || maxGridX >= m_gridSectionsWidth || maxGridZ < 0 || maxGridZ >= m_gridSectionsHeight)
		{
			if (pData->callback) { GameLogger::Log(MessageType::cWarning, "Tried to AddGraphicalObject to SpatialGrid but some triangles were out of grid range!\n"); }
			pData->m_success = false;
			return false;
		}

		if (pData->callback)
		{
			// add to all grid cells in range
			for (int x = minGridX; x <= maxGridX; ++x)
			{
				for (int z = minGridZ; z <= maxGridZ; ++z)
				{
					pData->callback(x, z, pData->pObj, index, p0, p1, p2, this);
				}
			}
		}

		pData->m_success = true;
		return true;
	}

	bool SpatialGrid::SetTriangleIndexPassThrough(int x, int z, GraphicalObject * pObj, int index, const Vec3& p0, const Vec3& p1, const Vec3& p2, void *pClassInstance)
	{
		SpatialGrid *pInstance = reinterpret_cast<SpatialGrid *>(pClassInstance);
		return pInstance->SetTriangleIndex(x, z, pObj, index);
	}

	bool SpatialGrid::SetTriangleIndex(int x, int z, GraphicalObject * /*pObj*/, int /*index*/)
	{
		m_pGridTriangleCounts[z*m_gridSectionsWidth + x]++;
		return true;
	}

	bool SpatialGrid::AddSpatialTrianglePassThrough(int x, int z, GraphicalObject * pObj, int index, const Vec3& p0, const Vec3& p1, const Vec3& p2, void *pClassInstance)
	{
		SpatialGrid *pInstance = reinterpret_cast<SpatialGrid *>(pClassInstance);
		return pInstance->AddSpatialTriangle(x, z, pObj, p0, p1, p2, index);
	}

	bool SpatialGrid::AddSpatialTriangle(int x, int z, GraphicalObject * pObj, const Vec3& p0, const Vec3& p1, const Vec3& p2, int index)
	{
		SpatialTriangleData newData;
		newData.p0 = p0;
		newData.p1 = p1;
		newData.p2 = p2;
		newData.m_pTriangleOwner = pObj;
		newData.m_triangleVertexZeroIndex = index; // pIndices[i] is index of p0
		int w = m_pGridStartIndices[z*m_gridSectionsWidth + x] + m_pGridTriangleCounts[z*m_gridSectionsWidth + x];
		m_pData[w] = newData;
		m_pGridTriangleCounts[z*m_gridSectionsWidth + x]++;
		return true;
	}

	void SpatialGrid::CleanUp()
	{
		if (m_pData) { delete[] m_pData; m_pData = nullptr; }
		if (m_pGridStartIndices) { delete[] m_pGridStartIndices; m_pGridStartIndices = nullptr; }
		if (m_pGridTriangleCounts) { delete[] m_pGridTriangleCounts; m_pGridTriangleCounts = nullptr; }
	}

	bool SpatialGrid::DoesFitInGrid(GraphicalObject * pGraphicalObjectToTest)
	{
		// model to world matrix
		Mat4 modelToWorld = *pGraphicalObjectToTest->GetFullTransformPtr();

		SpatialCallbackPassData data;
		data.modelToWorld = modelToWorld;
		data.callback = nullptr;
		data.pObj = pGraphicalObjectToTest;

		pGraphicalObjectToTest->GetMeshPointer()->WalkTriangles(SpatialGrid::ProcessTrianglesPassThrough, this, &data);
		return data.m_success;
	}
}
