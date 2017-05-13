#ifndef ASTARNODEMAP_H
#define ASTARNODEMAP_H

// Justin Furtado
// 5/2/2017
// AStarNodeMap.h
// Maps a-star nodes to eachother

#include "ExportHeader.h"
#include "AStarNode.h"
#include "LinkedList.h"

namespace Engine
{
	class ENGINE_SHARED AStarNodeMap
	{
		struct NodeWithConnections
		{
			NodeWithConnections() : m_pNode(nullptr) {}
			NodeWithConnections(AStarNode *pNode) : m_pNode(pNode) {}
			~NodeWithConnections() { DeleteNode(); }

			void SetConnectionInfo(int index, int count) { m_connectionCount = count; m_connectionIndex = index; }
			void DeleteNode() { if (m_pNode) { delete m_pNode; m_pNode = nullptr; } }

			const GraphicalObject *m_pNodeOrigin;
			AStarNode *m_pNode{ nullptr };
			int m_connectionIndex{ 0 };
			int m_connectionCount{ 0 };

			friend class AStarPathFinder;
		};

		struct ExtraData
		{
			int m_from{ 0 };
			int m_to{ 0 };
		};

	public:
		typedef bool(*DestroyObjectCallback)(GraphicalObject *pObjToDestroy, void *pClassInstance);
		typedef void(*SetUniformCallback)(GraphicalObject *pObj, void *pClasSInstance);

		AStarNodeMap();
		~AStarNodeMap();

		void ClearMap();
		void RemoveConnection(LinkedList<GraphicalObject*> *pObjs, GraphicalObject *pConnectionToRemove, DestroyObjectCallback destroyCallback, void *pDestructionInstance, int *outCountToUpdate);
		bool CalculateMap(LinkedList<GraphicalObject*> *pObjs, CollisionLayer nodeLayer, CollisionLayer connectionLayer, CollisionLayer geometryLayer, DestroyObjectCallback destroyCallback, void * pDestructionInstance, int *outCountToUpdate, SetUniformCallback uniformCallback, void *uniformInstance);
		void ClearGobs(LinkedList<GraphicalObject*> *pObjs, CollisionLayer nodeLayer, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void *pDestructionInstance, int *outCountToUpdate);
		void ClearGobsForLayer(LinkedList<GraphicalObject*> *pObjs, CollisionLayer layer, DestroyObjectCallback destroyCallback, void *pDestructionInstance, int *outCountToUpdate);
		void MakeArrowsForExistingConnections(LinkedList<GraphicalObject*> *pObjs, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void *pDestructionInstance, int *outCountToUpdate, SetUniformCallback uniformCallback, void *uniformInstance);
		void MakeObjsForExistingNodes(LinkedList<GraphicalObject*> *pObjs, CollisionLayer nodeLayer, DestroyObjectCallback destroyCallback, void *pDestructionInstance, int *outCountToUpdate, SetUniformCallback uniformCallback, void *uniformInstance);
		static bool FromFile(const char *const filePath, AStarNodeMap *pMap);
		static bool ToFile(const AStarNodeMap *const mapToWrite, const char *const filePath);
		bool ToFile(const char *const filePath);
		static bool IsObjInLayer(GraphicalObject *pObj, void *pClass);
		const AStarNode *FindNearestNodeTo(const Vec3& location) const;
		const int FindNearestNodeIndex(const Vec3& location) const;
		const int NodeIndex(const AStarNode *const pNode) const;
		const NodeWithConnections *GetConnectedNodes() const;
		const int *GetConnections() const;

	private:
		void AddSphereGobToList(int index, LinkedList<GraphicalObject*>* pObjs, CollisionLayer nodeLayer, int *outCountToUpdate, SetUniformCallback uniformCallback, void *uniformInstance);
		void AddArrowGobToList(const Vec3& iRightVec, const Vec3& iToJRightVec, int from, int to, LinkedList<GraphicalObject*>* pObjs, CollisionLayer connectionLayer, int *outCountToUpdate, SetUniformCallback uniformCallback, void *uniformInstance);
		bool ResetPreCalculation(LinkedList<GraphicalObject*> *pObjs, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void * pDestructionInstance, int *outCountToUpdate);
		bool MakeNodesWithNoConnections(LinkedList<GraphicalObject*> *pObjs, CollisionLayer nodeLayer);
		bool MakeAutomagicNodeConnections(LinkedList<GraphicalObject*> *pObjs, CollisionLayer connectionLayer, CollisionLayer geometryLayer, int *outCountToUpdate, SetUniformCallback uniformCallback, void *uniformInstance);
		static bool PathClear(const RayCastingOutput& rco, const GraphicalObject *pDestObj, float dist);
		void RemoveConnectionAndCondense(int fromIndex, int toIndex);
		static bool DoMakeNodesFromGobs(GraphicalObject *pObj, void *pClass);

		static const int NODE_MAP_FILE_VERSION = 3;
		int *m_pConnectionsTo{ nullptr };
		unsigned int m_numConnections{ 0 };
		NodeWithConnections *m_pNodesWithConnections{ nullptr };
		unsigned int m_numNodes{ 0 };
		unsigned int m_nextWalkIndex{ 0 };
		unsigned int m_numRemoved{ 0 };
	};
}


#endif // ifndef ASTARNODEMAP_H