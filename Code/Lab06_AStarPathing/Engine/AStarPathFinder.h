#ifndef ASTARPATHFINDER_H
#define ASTARPATHFINDER_H

// Justin Furtado
// 5/13/2017
// AStarPathFinder.h
// Static helper class for AStarPathing

#include "ExportHeader.h"
#include "AStarNodeMap.h"
#include "LinkedList.h"
#include "AStarNode.h"

namespace Engine
{
	class ENGINE_SHARED AStarPathFinder
	{
	public:
		// WARNING, MEMORY ALLOCATED AND RETURNED, CALLER RESPONSIBILITY TO DELETE
		static int *FindPath(const AStarNodeMap *pNodeMap, const Vec3& fromLocation, const Vec3& toLocation, int *outNumNodes);
		static int *FindPath(const AStarNodeMap *pNodeMap, int fromNodeIndex, int toNodeIndex, int *outNumNodes);
		static bool NodeGreaterThan(AStarNodeMap::NodeWithConnections *pNode, void *pCheckNode);

	private:
		static int *GetPathFromAncestors(const AStarNodeMap *pNodeMap, const AStarNode *pNode, int *outNumNodes);

	};
}


#endif // ifndef ASTARPATHFINDER_H