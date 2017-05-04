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
		};

	public:
		typedef void(*DestroyObjectCallback)(GraphicalObject *pObjToDestroy, void *pClassInstance);

		AStarNodeMap();
		~AStarNodeMap();

		void ClearMap();
		void RemoveConnection(LinkedList<GraphicalObject*> *pObjs, GraphicalObject *pConnectionToRemove, DestroyObjectCallback destroyCallback, void *pDestructionInstance);
		bool CalculateMap(LinkedList<GraphicalObject*> *pObjs, CollisionLayer nodeLayer, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void * pDestructionInstance);
		void ClearGobs(LinkedList<GraphicalObject*> *pObjs, CollisionLayer nodeLayer, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void *pDestructionInstance);
		void ClearGobsForLayer(LinkedList<GraphicalObject*> *pObjs, CollisionLayer layer, DestroyObjectCallback destroyCallback, void *pDestructionInstance);

		static AStarNodeMap FromFile(const char *const filePath);
		static bool ToFile(const AStarNodeMap *const mapToWrite, const char *const filePath);
		bool ToFile(const char *const filePath);

	private:
		bool ResetPreCalculation(LinkedList<GraphicalObject*> *pObjs, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void * pDestructionInstance);
		bool MakeNodesWithNoConnections(LinkedList<GraphicalObject*> *pObjs, CollisionLayer nodeLayer);
		bool MakeAutomagicNodeConnections(LinkedList<GraphicalObject*> *pObjs, CollisionLayer connectionLayer);

		void RemoveConnectionAndCondense(int fromIndex, int toIndex);
		static bool IsObjInLayer(GraphicalObject *pObj, void *pClass);
		static bool DoMakeNodesFromGobs(GraphicalObject *pObj, void *pClass);

		static const int NODE_MAP_FILE_VERSION = 1;
		int *m_pConnectionsTo{ nullptr };
		unsigned int m_numConnections{ 0 };
		NodeWithConnections *m_pNodesWithConnections{ nullptr };
		unsigned int m_numNodes{ 0 };
		unsigned int m_nextWalkIndex{ 0 };
	};
}


#endif // ifndef ASTARNODEMAP_H