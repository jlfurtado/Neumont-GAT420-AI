#include "AStarPathFinder.h"

// Justin Furtado
// 5/13/2017
// AStarPathFinder.cpp
// Static helper class for AStarPathing

namespace Engine
{
	int * AStarPathFinder::FindPath(const AStarNodeMap * pNodeMap, const Vec3 & fromLocation, const Vec3 & toLocation, int *outNumNodes)
	{
		return FindPath(pNodeMap, pNodeMap->FindNearestNodeIndex(fromLocation), pNodeMap->FindNearestNodeIndex(toLocation), outNumNodes);
	}

	int * AStarPathFinder::FindPath(const AStarNodeMap * pNodeMap, int fromNodeIndex, int toNodeIndex, int *outNumNodes)
	{
		// start and end nodes
		AStarNodeMap::NodeWithConnections *pStartNode = &pNodeMap->m_pNodesWithConnections[fromNodeIndex];
		AStarNodeMap::NodeWithConnections *pEndNode = &pNodeMap->m_pNodesWithConnections[toNodeIndex];

		// if they are trying to pathfind from a node to itself... they needn't move!
		if (pStartNode == pEndNode) { return nullptr; }

		// make the open and closed lists
		LinkedList<AStarNodeMap::NodeWithConnections*> openList;
		LinkedList<AStarNodeMap::NodeWithConnections*> closedList;

		// add the start node to the open list
		openList.AddToListFront(pStartNode); // no need for callback as it should be the only node in the list right now
		pStartNode->m_pNode->m_cost = (pStartNode->m_pNode->m_position - pEndNode->m_pNode->m_position).Length(); // set the total cost of the start node... just the h cost????

		// keep going so long as we have nodes in our open list
		while (openList.GetCount() > 0)
		{
			// THE FIRST NODE IN OUR LIST HAS THE LOWEST COST BECAUSE WE SORT IT AS WE PLACE NODES IN THE LISTS
			AStarNodeMap::NodeWithConnections *pCurrentNode = openList.GetFirstObjectData(); // get the node with the lowest cost in the list
			openList.RemoveFirstFromList(pCurrentNode); // remove the current node from the list
			closedList.AddToListWhere(pCurrentNode, AStarPathFinder::NodeGreaterThan, pCurrentNode); // add to the closed list, in order

			// if the current node is the end node, we are done pathfinding as we have reached out destination
			if (pCurrentNode == pEndNode)
			{
				// allocate memory and return the path to this node
				return GetPathFromAncestors(pNodeMap, pCurrentNode->m_pNode, outNumNodes);
			}

			// for each neighbor node/connected node of the current node
			int end = pCurrentNode->m_connectionIndex + pCurrentNode->m_connectionCount;
			for (int i = pCurrentNode->m_connectionIndex; i < end; ++i)
			{
				// find out neighbor
				AStarNodeMap::NodeWithConnections *pNeighbor = &pNodeMap->m_pNodesWithConnections[i];

				// see if its closed or not traversable
				if (closedList.Contains(pNeighbor) /* || not traversable!!!*/) { continue; }

				// if the new path is shorter or the neighbor is not in the open list
				if (pNeighbor->m_pNode->m_cost < pCurrentNode->m_pNode->m_cost || !openList.Contains(pNeighbor)) // TODO CHECK COST COMPARE...
				{
					// set the cost of the neighbor
					float parentHCost = (pCurrentNode->m_pNode->m_position - pEndNode->m_pNode->m_position).Length();
					float myHCost = (pNeighbor->m_pNode->m_position - pEndNode->m_pNode->m_position).Length();
					float gCostOffset = (pCurrentNode->m_pNode->m_position - pNeighbor->m_pNode->m_position).Length();

					// should give us currect AStarCost!?!?!?!?
					pNeighbor->m_pNode->m_cost = pCurrentNode->m_pNode->m_cost + gCostOffset + (myHCost - parentHCost);

					// set the parent of the neighbor
					pNeighbor->m_pNode->m_pParent = pCurrentNode->m_pNode;

					// add the neighbor to the open list, if it is not already in the list
					if (!openList.Contains(pNeighbor))
					{
						// add it in order so we do not need to search to find the lowest cost node
						openList.AddToListWhere(pNeighbor, AStarPathFinder::NodeGreaterThan, pNeighbor);
					}
				}
			}
		}

		// No valid path exists! Return nullptr
		return nullptr;
	}

	bool AStarPathFinder::NodeGreaterThan(AStarNodeMap::NodeWithConnections * pNode, void * pCheckNode)
	{
		// cast the node we're comparing
		AStarNodeMap::NodeWithConnections *pNodeCompare = reinterpret_cast<AStarNodeMap::NodeWithConnections*>(pCheckNode);

		// return true (meaning stop in the case we're passing this method to) when the cost of the node in the list is greater than the cost of the node we are going to add
		// should place the node in ascending cost order
		return pNode->m_pNode->m_cost > pNodeCompare->m_pNode->m_cost;
	}

	int * AStarPathFinder::GetPathFromAncestors(const AStarNodeMap *pNodeMap, const AStarNode * pNode, int *outNumNodes)
	{
		// start at the node
		const AStarNode *pCurrentNode = pNode;

		// initialize counter
		int numSteps = 0;

		// loop through all the ancestors, counting them
		while (pCurrentNode)
		{
			++numSteps; // add one to counter
			pCurrentNode = pCurrentNode->m_pParent; // move up an ancestor
		}

		// allocate only the amount of memory we need
		int *pPath = new int[numSteps]; // how many nodes we need
		*outNumNodes = numSteps; // set out variable

		// start back at the bottom node
		pCurrentNode = pNode;

		// loop up
		while (pCurrentNode)
		{
			// backwards-fill the array with node indices so we can access them
			pPath[--numSteps] = pNodeMap->NodeIndex(pCurrentNode);
			pCurrentNode = pCurrentNode->m_pParent; // move up a parent
		}

		// return the path
		return pPath;
	}
}