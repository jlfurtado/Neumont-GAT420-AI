#include "AStarNodeMap.h"

// Justin Furtado
// 5/2/2017
// AStarNodeMap.h
// Maps a-star nodes to eachother

namespace Engine
{
	const Vec3 BASE_ARROW_DIR = Engine::Vec3(1.0f, 0.0f, 0.0f);


	AStarNodeMap::AStarNodeMap()
	{
	}


	AStarNodeMap::~AStarNodeMap()
	{
		// prevents memory leak when forgetting to call ClearMap() before destruction
		ClearMap();
	}

	// resets our nodes and their connections fully, releasing memory 
	void AStarNodeMap::ClearMap()
	{
		// delete the connections, if they should be deleted
		if (m_pConnectionsTo)
		{
			delete[] m_pConnectionsTo;
			m_numConnections = 0; // update count to reflect full clear
			m_pConnectionsTo = nullptr; // if and nullptr allow calling this method multiple times to be safe
		}

		// delete the nodes, if they should be deleted
		if (m_pNodesWithConnections)
		{
			delete[] m_pNodesWithConnections;
			m_numNodes = 0; // update count to reflect full clear
			m_pNodesWithConnections = nullptr; // if and nullptr allow calling this method multiple times to be safe
		}
	}

	void AStarNodeMap::RemoveConnection(LinkedList<GraphicalObject*>* pObjs, GraphicalObject * pConnectionToRemove, DestroyObjectCallback destroyCallback, void * pDestructionInstance)
	{
		// p extra data points to the to index in the array, so we can get its index in the full array and the index of the node 
		int *toIndexPtr = reinterpret_cast<int*>(pConnectionToRemove->m_pExtraData);
		int arrayIndex = toIndexPtr - m_pConnectionsTo; // Clever use of pointer arithmetic, huh?

		// find which nodes it goes between so we can fully remove it
		int toIndex = *toIndexPtr;
		int fromIndex;

		// to get the index of the node it goes from, we iterate through the nodes until we would pass it on the next iteration, leaving us on the correct from index
		for (fromIndex = 0; fromIndex < m_numNodes; fromIndex++)
		{
			if ((m_pNodesWithConnections + fromIndex + 1)->m_connectionIndex > arrayIndex) { break; }
		}

		// destory the gob
		destroyCallback(pConnectionToRemove, pDestructionInstance);
		pObjs->RemoveFirstFromList(pConnectionToRemove);

		// remove the connection from our connections
		RemoveConnectionAndCondense(fromIndex, toIndex);
	}

	void AStarNodeMap::CalculateMap(LinkedList<GraphicalObject*>* pObjs, CollisionLayer nodeLayer, CollisionLayer connectionLayer)
	{

		// DO ALL THE THINGS!!!

		// TODO: all the things
		/*
		   -1. Clear al previous connections, remove connection objects and nodes (but not node gobs) from previous map calculations

			0. Iterate through the list and remove all objects in the connection layer, freeing memory, recalculating the layer, etc.

			1. Iterate through all gob pointers in list, and find the ones in the node layer
				creating AStarNodes at those locations

			2. Do the raycasting thing that checks for visibility between nodes, creating connections as is applicable
				and adding the gobs for those connections to the list (in the proper layer)

			3. Do all the data structure updating/condensing/setting data for nodesWithConnections, etc.

			4. when make connection, set m_pExtra data to point to the toIndex in the array
		*/
	}

	void AStarNodeMap::ClearGobs(LinkedList<GraphicalObject*>* pObjs, CollisionLayer nodeLayer, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void *pDestructionInstance)
	{
		// removes objs from both layer, calling the destruction callback for each
		ClearGobsForLayer(pObjs, nodeLayer, destroyCallback, pDestructionInstance);
		ClearGobsForLayer(pObjs, connectionLayer, destroyCallback, pDestructionInstance);
	}

	// removes all 
	void AStarNodeMap::ClearGobsForLayer(LinkedList<GraphicalObject*>* pObjs, CollisionLayer layer, DestroyObjectCallback destroyCallback, void *pDestructionInstance)
	{
		// data for iteration
		CollisionLayer layerCpy = layer;
		GraphicalObject *pToRemove = nullptr;

		do
		{
			// grab the first obj that is in the layer
			pToRemove = pObjs->GetWhere(AStarNodeMap::IsObjInLayer, &layerCpy);
			
			if (pToRemove)
			{
				// if we found one, call the destruction callback for it and remove it from the list
				destroyCallback(pToRemove, pDestructionInstance);
				pObjs->RemoveFirstFromList(pToRemove);
			}

			// keep going until we don't find one
		} while (pToRemove);
	}

	// reads a node map from a file and returns it, returns default (empty) node map in case of failure
	AStarNodeMap AStarNodeMap::FromFile(const char * const filePath)
	{
		// load the map from a file (VALIDATE VERSION, log error accordingly)
		return AStarNodeMap();
	}

	// writes a node map to a file
	bool AStarNodeMap::ToFile(const AStarNodeMap * const mapToWrite, const char * const filePath)
	{
		// write the map to a file (INCLUDING VERSION)
		return false;
	}

	// writes this node map to a file
	bool AStarNodeMap::ToFile(const char * const filePath)
	{
		return ToFile(this, filePath);
	}

	// iterates through array, condensing and updating nodes with connections
	void AStarNodeMap::RemoveConnectionAndCondense(int fromIndex, int toIndex)
	{
		// we are removing one connection from node[fromIndex], so all of the things after start one earlier now as we want no holes in the array
		for (int i = fromIndex + 1; i < m_numNodes; ++i)
		{
			(m_pNodesWithConnections + i)->m_connectionIndex--;
		}

		// we want to find the spot in the actual array where toIndex is located, to do this we start at node[fromIndex]'s section of the array, and walk it looking for the value
		int arrayIndex;
		NodeWithConnections *pConnectFrom = (m_pNodesWithConnections + fromIndex);

		// start at the index of the from node in the array
		int start = pConnectFrom->m_connectionIndex;

		// go as many in as counts it has
		int end = start + pConnectFrom->m_connectionCount;
		for (arrayIndex = start; arrayIndex < end; ++arrayIndex)
		{
			// we found the value
			if (*(m_pConnectionsTo + arrayIndex) == toIndex) { break; }
		}

		// we're ditching the one, so we need to update the count
		pConnectFrom->m_connectionCount--;

		// we're going to memcpy the remainder of the array one slot earlier, rather than copying each int manually
		int *copyTo = m_pConnectionsTo + arrayIndex;
		int *copyFrom = copyTo + 1;
		int numInts = m_numConnections - arrayIndex;
		memcpy(copyTo, copyFrom, sizeof(int) * (numInts));

		// update our conter, we removed a connection
		m_numConnections--;
	}

	// returns true if an object is in the collision layer, altered to be match signature for linked list walk callback
	bool AStarNodeMap::IsObjInLayer(GraphicalObject * pObj, void * pDoinSomethingDifferentHere)
	{
		return Engine::CollisionTester::IsInLayer(pObj, *reinterpret_cast<Engine::CollisionLayer*>(pDoinSomethingDifferentHere));
	}

}

