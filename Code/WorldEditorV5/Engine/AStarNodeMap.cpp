#include "AStarNodeMap.h"
#include "ShapeGenerator.h"
#include "RenderEngine.h"

// Justin Furtado
// 5/2/2017
// AStarNodeMap.h
// Maps a-star nodes to eachother

namespace Engine
{
	const Vec3 BASE_ARROW_DIR(1.0f, 0.0f, 0.0f);

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

	void AStarNodeMap::RemoveConnection(LinkedList<GraphicalObject*>* pObjs, GraphicalObject * pConnectionToRemove, DestroyObjectCallback destroyCallback, void * pDestructionInstance, int *outCountToUpdate)
	{
		// p extra data points to the to index in the array, so we can get its index in the full array and the index of the node 
		int *toIndexPtr = reinterpret_cast<int*>(pConnectionToRemove->m_pExtraData);
		int arrayIndex = toIndexPtr - m_pConnectionsTo; // Clever use of pointer arithmetic, huh?

		// find which nodes it goes between so we can fully remove it
		int toIndex = *toIndexPtr;
		unsigned fromIndex;

		// to get the index of the node it goes from, we iterate through the nodes until we would pass it on the next iteration, leaving us on the correct from index
		for (fromIndex = 0; fromIndex < m_numNodes; fromIndex++)
		{
			if ((m_pNodesWithConnections + fromIndex + 1)->m_connectionIndex > arrayIndex) { break; }
		}

		// destory the gob
		destroyCallback(pConnectionToRemove, pDestructionInstance);
		pObjs->RemoveFirstFromList(pConnectionToRemove);
		*outCountToUpdate--;

		// remove the connection from our connections
		RemoveConnectionAndCondense(fromIndex, toIndex);
	}

	bool AStarNodeMap::CalculateMap(LinkedList<GraphicalObject*>* pObjs, CollisionLayer nodeLayer, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void * pDestructionInstance, int *outCountToUpdate)
	{
		// lets clean up before we start...
		if (!ResetPreCalculation(pObjs, connectionLayer, destroyCallback, pDestructionInstance, outCountToUpdate)) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Could not ResetPreCalculation!\n"); return false; }

		// at this point we should be reset to the appropriate state to calculate the map...
			// we should have gobs indicating where to make nodes, but no nodes
			// we should have no connection objects or connections
		
		// lets make some nodes...
		if (!MakeNodesWithNoConnections(pObjs, nodeLayer)) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Could not MakeNodesWithNoConnections!\n"); return false; }
		
		// at this point, we should have made nodes based on our gobs
			// we have the correct number of them, and they should all be placed at decent positions

		// lets make some connections!
		if (!MakeAutomagicNodeConnections(pObjs, connectionLayer, outCountToUpdate)) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Could not MakeAutomagicNodeConnections!\n"); return false; }

		// HOOORRRAY ITS FINALLY OVER!!!!!!!!!!!
		return true;
	}

	void AStarNodeMap::ClearGobs(LinkedList<GraphicalObject*>* pObjs, CollisionLayer nodeLayer, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void *pDestructionInstance, int *outCountToUpdate)
	{
		// removes objs from both layer, calling the destruction callback for each
		ClearGobsForLayer(pObjs, nodeLayer, destroyCallback, pDestructionInstance, outCountToUpdate);
		ClearGobsForLayer(pObjs, connectionLayer, destroyCallback, pDestructionInstance, outCountToUpdate);
	}

	// removes all 
	void AStarNodeMap::ClearGobsForLayer(LinkedList<GraphicalObject*>* pObjs, CollisionLayer layer, DestroyObjectCallback destroyCallback, void *pDestructionInstance, int *outCountToUpdate)
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
				*outCountToUpdate--;
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

	void AStarNodeMap::AddArrowGobToList(const Vec3 & iRightVec, const Vec3 & iToJRightVec, int* pExtra, LinkedList<GraphicalObject*>* pObjs, CollisionLayer connectionLayer, int * outCountToUpdate)
	{
		// obj should get deleted externally in list we put it in
		GraphicalObject *pArrow = new GraphicalObject();
		ShapeGenerator::MakeDebugArrow(pArrow, Vec3(0.25f, .75f, 0.0f), Vec3(0.0f, .75f, 0.0f));

		// make go from i right to j right
		pArrow->SetRotMat(Mat4::RotationToFace(BASE_ARROW_DIR, iToJRightVec.Normalize()));
		pArrow->SetScaleMat(Mat4::Scale(iToJRightVec.Length() / 2.0f, BASE_ARROW_DIR));
		pArrow->SetTransMat(Mat4::Translation(iRightVec));
		pArrow->CalcFullTransform();

		// ugly make it work thing
		pArrow->m_pExtraData = pExtra;

		// add it where it goes
		RenderEngine::AddGraphicalObject(pArrow);
		CollisionTester::AddGraphicalObjectToLayer(pArrow, connectionLayer);
		pObjs->AddToList(pArrow);
		*outCountToUpdate++;
	}

	bool AStarNodeMap::ResetPreCalculation(LinkedList<GraphicalObject*>* pObjs, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void * pDestructionInstance, int *outCountToUpdate)
	{
		CollisionLayer checkLayer = connectionLayer;

		// remove all existing connection gobs!
		ClearGobsForLayer(pObjs, connectionLayer, destroyCallback, pDestructionInstance, outCountToUpdate);

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
		for (unsigned i = 0; i < nodeCount; ++i)
		{
			if (m_pNodesWithConnections[i].m_pNode == nullptr) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Failed to create nodes from gobs! Node [%d] has no node pointer!\n", i); return false; }
		}

		// set our node count
		m_numNodes = nodeCount;

		// we have our nodes, lets make some connections!
		return true;
	}

	const Vec3 UP(0.0f, 1.0f, 0.0f);
	const int INDEX_NOT_SET = -1;
	bool AStarNodeMap::MakeAutomagicNodeConnections(LinkedList<GraphicalObject*>* pObjs, CollisionLayer connectionLayer, int *outCountToUpdate)
	{
		// absolute max num connections is n*n-n 
		// 2 nodes = 4-2 connections = 2 (check)
		// 3 nodes = 9-3 connections = 6 (check)
		// 4 nodes = 16-4 connections = 12 (check)
		// 5 nodes = 25-5 connections = 20 (check) ... and so on...

		// cannot have more than that many connections, but can have less, should never have to worry about overriding array
		int maxSize = m_numNodes * (m_numNodes - 1);

		// allocate and initialize indices
		m_pConnectionsTo = new int[maxSize];
		for (int i = 0; i < maxSize; ++i) { m_pConnectionsTo[i] = INDEX_NOT_SET; }

		// variable to track the start indices for each node
		int nextStartIndex = 0;

		// for each node
		for (unsigned i = 0; i < m_numNodes; ++i)
		{
			// how many nodes this node can see
			int numICanSee = 0;

			// compare to each other node
			for (unsigned j = 0; j < m_numNodes; ++j)
			{
				// don't ever raycast to self
				if (j == i) { continue; }

				// centers of objects
				Vec3 iCenter = m_pNodesWithConnections[i].m_pNode->GetPosition();
				Vec3 jCenter = m_pNodesWithConnections[j].m_pNode->GetPosition();

				// vector going from i to j
				Vec3 iToJCenter = jCenter - iCenter;

				// vector going from center to right edge for sphere based on its radius
				Vec3 iRightOffset = iToJCenter.Normalize().Cross(UP).Normalize() * m_pNodesWithConnections[i].m_pNode->GetRadius();
				Vec3 jRightOffset = (-iToJCenter).Normalize().Cross(UP).Normalize() * m_pNodesWithConnections[j].m_pNode->GetRadius();

				// edge points to raycast from
				Vec3 iLeft = iCenter - iRightOffset;
				Vec3 iRight = iCenter + iRightOffset;
				Vec3 jLeft = jCenter - jRightOffset;
				Vec3 jRight = jCenter + jRightOffset;

				// vectors going from edges to other edges
				Vec3 iToJRight = jRight - iRight;
				Vec3 iToJLeft = jLeft - iLeft;

				// only check other points if the first path is clear, because raycasting is very expensive (will only do subsequent raycasts if the previous are a clear path
				if (PathClear(CollisionTester::FindWall(iCenter, iToJCenter.Normalize(), iToJCenter.Length(), CollisionLayer::NUM_LAYERS), m_pNodesWithConnections[j].m_pNodeOrigin)
					&& PathClear(CollisionTester::FindWall(iRight, iToJRight.Normalize(), iToJRight.Length(), CollisionLayer::NUM_LAYERS), m_pNodesWithConnections[j].m_pNodeOrigin)
					&& PathClear(CollisionTester::FindWall(iLeft, iToJLeft.Normalize(), iToJLeft.Length(), CollisionLayer::NUM_LAYERS), m_pNodesWithConnections[j].m_pNodeOrigin))
				{
					// the index in the array is the start plus the num seen so far
					int arrayIndex = nextStartIndex + numICanSee;

					// put in that index the index of the node to which it is connected
					m_pConnectionsTo[arrayIndex] = j;

					// make the arrow gob
					AddArrowGobToList(iRight, iToJRight, &m_pConnectionsTo[arrayIndex], pObjs, connectionLayer, outCountToUpdate);

					// update counts
					numICanSee++;
				}
			}

			// set start indices and counts, update variable, based on what seen within inner loop
			m_pNodesWithConnections[i].SetConnectionInfo(nextStartIndex, numICanSee);
			nextStartIndex += numICanSee;
		}

		return true;
	}

	// returns false if a ray cast from one object to another hit anything in between
	bool AStarNodeMap::PathClear(const RayCastingOutput & rco, const GraphicalObject *pDestObj)
	{
		// the path is clear if we hit the destination object or it hit nothing
		return (rco.m_didIntersect && rco.m_belongsTo == pDestObj) || !(rco.m_didIntersect);
	}

	// iterates through array, condensing and updating nodes with connections
	void AStarNodeMap::RemoveConnectionAndCondense(int fromIndex, int toIndex)
	{
		// we are removing one connection from node[fromIndex], so all of the things after start one earlier now as we want no holes in the array
		for (unsigned i = fromIndex + 1; i < m_numNodes; ++i)
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

