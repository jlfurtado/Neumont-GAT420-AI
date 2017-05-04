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

	bool AStarNodeMap::CalculateMap(LinkedList<GraphicalObject*>* pObjs, CollisionLayer nodeLayer, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void * pDestructionInstance)
	{
		// lets clean up before we start...
		if (!ResetPreCalculation(pObjs, connectionLayer, destroyCallback, pDestructionInstance)) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Could not ResetPreCalculation!\n"); return false; }

		// at this point we should be reset to the appropriate state to calculate the map...
			// we should have gobs indicating where to make nodes, but no nodes
			// we should have no connection objects or connections
		
		// lets make some nodes...
		if (!MakeNodesWithNoConnections(pObjs, nodeLayer)) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Could not MakeNodesWithNoConnections!\n"); return false; }
		
		// at this point, we should have made nodes based on our gobs
			// we have the correct number of them, and they should all be placed at decent positions

		// lets make some connections!
		if (!MakeAutomagicNodeConnections(pObjs, connectionLayer)) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Could not MakeAutomagicNodeConnections!\n"); return false; }

		// HOOORRRAY ITS FINALLY OVER!!!!!!!!!!!
		return true;
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

	bool AStarNodeMap::ResetPreCalculation(LinkedList<GraphicalObject*>* pObjs, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void * pDestructionInstance)
	{
		CollisionLayer checkLayer = connectionLayer;

		// remove all existing connection gobs!
		ClearGobsForLayer(pObjs, connectionLayer, destroyCallback, pDestructionInstance);

		// remove all nodes (not gobs) and all connections (not gobs)!
		ClearMap();

		// double check to make sure! (remove checks if performance is an issue???)

		// check that we don't have an allocated array for nodes with connections
		if (m_pNodesWithConnections) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! NodesWithConnections not cleared successfully!\n"); return false; }

		// check that we don't have an allocated array for connections to
		if (m_pConnectionsTo) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! ConnectionsTo not cleared successfully!\n"); return false; }

		// check that our connection count is zero
		if (m_numConnections != 0) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Node connections not cleared successfully! Count is [%d]\n", m_numConnections); return false; }

		// check that our node count is zero
		if (m_numNodes != 0) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Nodes not cleared successfully! Count is [%d]\n", m_numNodes); return false; }

		// check that there are no objects in this list that are in the connection layer
		unsigned int countInConnectionLayer = pObjs->GetCountWhere(AStarNodeMap::IsObjInLayer, &checkLayer);
		if (countInConnectionLayer != 0) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Connection Gobs not cleared successfully! [%d] remain in Layer [%s]", countInConnectionLayer, CollisionTester::LayerString(checkLayer)); return false; }

		// we're ready to go and make some nodes!
		return true;
	}

	bool AStarNodeMap::MakeNodesWithNoConnections(LinkedList<GraphicalObject*>* pObjs, CollisionLayer nodeLayer)
	{
		// count how many GOBS are in the nodeLayer
		CollisionLayer checkLayer = nodeLayer;
		unsigned int nodeCount = pObjs->GetCountWhere(AStarNodeMap::IsObjInLayer, &checkLayer);

		// we don't have any nodes to make a node map out of...
		if (nodeCount == 0) { GameLogger::Log(MessageType::cError, "Did not CalculateNodeMap! No nodes found to make! 0 remain in Layer [%s]", CollisionTester::LayerString(checkLayer)); return false; }

		// allocate space for that many gobs!
		m_pNodesWithConnections = new NodeWithConnections[nodeCount];

		// start at the beginning of the array
		m_nextWalkIndex = 0;

		// call a method that sets up a single NodeWithConnections for each object whish is in the node layer
		if (!pObjs->WalkListWhere(AStarNodeMap::IsObjInLayer, &checkLayer, AStarNodeMap::DoMakeNodesFromGobs, this)) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Failed to DoCopyToArrayWalk!\n");  return false; }

		// we SHOULD HAVE copied each node into the array, leaving us with an index equal to the size of the array
		if (m_nextWalkIndex != nodeCount) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Failed to create nodes from gobs! Needed to make [%d] nodes but made [%d]!\n", nodeCount, m_nextWalkIndex); return false; }

		// to be even more obsessively thorough, lets make sure nodes were allocated for every single slot in the array
		for (int i = 0; i < nodeCount; ++i)
		{
			if (m_pNodesWithConnections[i].m_pNode == nullptr) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Failed to create nodes from gobs! Node [%d] has no node pointer!\n", i); return false; }
		}

		// set our node count
		m_numNodes = nodeCount;

		// we have our nodes, lets make some connections!
		return true;
	}

	// TODO: OPTIMIZE!?!?! if one raycast one direction fails, no need to check the other two!
	bool AStarNodeMap::MakeAutomagicNodeConnections(LinkedList<GraphicalObject*>* pObjs, CollisionLayer connectionLayer)
	{
		// for each node
		for (int i = 0; i < m_numNodes; ++i)
		{
			// compare to each other node
			for (int j = 0; j < m_numNodes; ++j)
			{
				// check both in case of future one-way-walls
				Vec3 up(0.0f, 1.0f, 0.0f);

				// centers of objects
				Vec3 iCenter = m_pNodesWithConnections[i].m_pNode->GetPosition();
				Vec3 jCenter = m_pNodesWithConnections[j].m_pNode->GetPosition();

				// vector going from i to j
				Vec3 iToJCenter = jCenter - iCenter;

				// vector going from center to right edge for sphere based on its radius
				Vec3 iRightOffset = iToJCenter.Normalize().Cross(up).Normalize() * m_pNodesWithConnections[i].m_pNode->GetRadius();
				Vec3 jRightOffset = (-iToJCenter).Normalize().Cross(up).Normalize() * m_pNodesWithConnections[j].m_pNode->GetRadius();

				// edge points to raycast from
				Vec3 iLeft = iCenter - iRightOffset;
				Vec3 iRight = iCenter + iRightOffset;
				Vec3 jLeft = jCenter - jRightOffset;
				Vec3 jRight = jCenter + jRightOffset;

				// vectors going from edges to other edges
				Vec3 iToJRight = jRight - iRight;
				Vec3 iToJLeft = jLeft - iLeft;

				// raycasts from i TO j
				RayCastingOutput iToJCenter = CollisionTester::FindWall(iCenter, iToJCenter.Normalize(), iToJCenter.Length(), CollisionLayer::NUM_LAYERS);
				RayCastingOutput iToJLeft = CollisionTester::FindWall(iLeft, iToJLeft.Normalize(), iToJLeft.Length(), CollisionLayer::NUM_LAYERS);
				RayCastingOutput iToJRight = CollisionTester::FindWall(iRight, iToJRight.Normalize(), iToJRight.Length(), CollisionLayer::NUM_LAYERS);

				// raycasts from j to I
				RayCastingOutput jToICenter = CollisionTester::FindWall(jCenter, -(iToJCenter.Normalize()), iToJCenter.Length(), CollisionLayer::NUM_LAYERS);
				RayCastingOutput jToILeft = CollisionTester::FindWall(jLeft,  -(iToJLeft.Normalize()), iToJLeft.Length(), CollisionLayer::NUM_LAYERS);
				RayCastingOutput jToIRight = CollisionTester::FindWall(jRight, -(iToJRight.Normalize()), iToJRight.Length(), CollisionLayer::NUM_LAYERS);

				// TODO: REACT TO RAYCAST OUTPUT
			}
		}
		/*


			2. Do the raycasting thing that checks for visibility between nodes, creating connections as is applicable
			and adding the gobs for those connections to the list(in the proper layer)

			3. Do all the data structure updating / condensing / setting data for nodesWithConnections, etc.

			4. when make connection, set m_pExtra data to point to the toIndex in the array
		*/

		return false;
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

	const float RADIUS_MULTIPLIER = 1.0f; // TODO: adjust this to match display object
	bool AStarNodeMap::DoMakeNodesFromGobs(GraphicalObject * pObj, void * pClass)
	{
		// get pointer to our map
		AStarNodeMap *pMap = reinterpret_cast<AStarNodeMap*>(pClass);

		// allocate memory for a node
		AStarNode *pNewNode = new AStarNode(); // instantiating, but giving it only to hidden struct only accessible to this class which will destruct it for us...
		
		// set it up based on our gob
		pNewNode->SetPosition(pObj->GetPos());
		pNewNode->SetParent(nullptr);
		pNewNode->SetRadius(pObj->GetScaleMatPtr()->GetAddress()[0] * RADIUS_MULTIPLIER);
		pNewNode->SetNodeEnabled(true);

		// add it to our node with connection
		pMap->m_pNodesWithConnections[pMap->m_nextWalkIndex].m_pNodeOrigin = pObj;
		pMap->m_pNodesWithConnections[pMap->m_nextWalkIndex++].m_pNode = pNewNode;

		return true;
	}

}

