#include "AStarNodeMap.h"
#include "ShapeGenerator.h"
#include "RenderEngine.h"
#include <fstream>

// Justin Furtado
// 5/2/2017
// AStarNodeMap.h
// Maps a-star nodes to eachother

namespace Engine
{
	const Vec3 BASE_ARROW_DIR(1.0f, 0.0f, 0.0f);
	const Vec3 UP(0.0f, 1.0f, 0.0f);
	const float RADIUS_MULTIPLIER = 1.9f; // TODO: adjust this to match display object

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

	void AStarNodeMap::RemoveConnection(LinkedList<GraphicalObject*>* pObjs, GraphicalObject * pConnectionToRemove, DestroyObjectCallback destroyCallback, void * pDestructionInstance, int * /*outCountToUpdate*/)
	{
		// remove the connection from our connections
		RemoveConnectionAndCondense(pConnectionToRemove->fromTempDeleteMeLater, pConnectionToRemove->toTempDeleteMeLater);

		// destory the gob
		destroyCallback(pConnectionToRemove, pDestructionInstance);
		pObjs->RemoveFirstFromList(pConnectionToRemove);
		//*outCountToUpdate--;
	}

	bool AStarNodeMap::CalculateMap(LinkedList<GraphicalObject*>* pObjs, CollisionLayer nodeLayer, CollisionLayer connectionLayer, CollisionLayer geometryLayer, DestroyObjectCallback destroyCallback, void * pDestructionInstance, int * outCountToUpdate, SetUniformCallback uniformCallback, void *uniformInstance)
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
		if (!MakeAutomagicNodeConnections(pObjs, connectionLayer, geometryLayer, outCountToUpdate, uniformCallback, uniformInstance)) { GameLogger::Log(MessageType::cError, "Failed to CalculateNodeMap! Could not MakeAutomagicNodeConnections!\n"); return false; }

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
	void AStarNodeMap::ClearGobsForLayer(LinkedList<GraphicalObject*>* pObjs, CollisionLayer layer, DestroyObjectCallback destroyCallback, void *pDestructionInstance, int * /*outCountToUpdate*/)
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
				//*outCountToUpdate--;
			}

			// keep going until we don't find one
		} while (pToRemove);
	}

	void AStarNodeMap::MakeArrowsForExistingConnections(LinkedList<GraphicalObject*>* pObjs, CollisionLayer connectionLayer, DestroyObjectCallback destroyCallback, void * pDestructionInstance, int * outCountToUpdate, SetUniformCallback uniformCallback, void *uniformInstance)
	{
		// clear possibly existing connection gobs
		ClearGobsForLayer(pObjs, connectionLayer, destroyCallback, pDestructionInstance, outCountToUpdate);

		// make new gobs for existing connections
		for (unsigned i = 0; i < m_numNodes; ++i)
		{
			int start = m_pNodesWithConnections[i].m_connectionIndex;
			int end = start + m_pNodesWithConnections[i].m_connectionCount;
			for (int j = start; j < end; ++j)
			{
				int k = m_pConnectionsTo[j];

				// centers of objects
				Vec3 iCenter = m_pNodesWithConnections[i].m_pNode->GetPosition();
				Vec3 kCenter = m_pNodesWithConnections[k].m_pNode->GetPosition();

				// vector going from i to j
				Vec3 iToKCenter = kCenter - iCenter;

				// vector going from center to right edge for sphere based on its radius
				Vec3 iRightOffset = iToKCenter.Normalize().Cross(UP).Normalize() * m_pNodesWithConnections[i].m_pNode->GetRadius();
				Vec3 kRightOffset = (-iToKCenter).Normalize().Cross(UP).Normalize() * m_pNodesWithConnections[k].m_pNode->GetRadius();

				// edge points to raycast from
				Vec3 iRight = iCenter + iRightOffset;
				Vec3 kRight = kCenter - kRightOffset;

				// vectors going from edges to other edges
				Vec3 iToKRight = kRight - iRight;

				//GameLogger::Log(MessageType::ConsoleOnly, "Connecting from [%d] to [%d]\n", i, k);
				AddArrowGobToList(iRight, iToKRight, i, k, pObjs, connectionLayer, outCountToUpdate, uniformCallback, uniformInstance);
			}
		}
	}

	void AStarNodeMap::MakeObjsForExistingNodes(LinkedList<GraphicalObject*>* pObjs, CollisionLayer nodeLayer, DestroyObjectCallback destroyCallback, void * pDestructionInstance, int * outCountToUpdate, SetUniformCallback uniformCallback, void * uniformInstance)
	{
		// clear possibly existing connection gobs
		ClearGobsForLayer(pObjs, nodeLayer, destroyCallback, pDestructionInstance, outCountToUpdate);

		// make new gobs for existing nodes
		for (unsigned i = 0; i < m_numNodes; ++i)
		{
			AddSphereGobToList(i, pObjs, nodeLayer, outCountToUpdate, uniformCallback, uniformInstance);
		}
	}

	// reads a node map from a file and returns it, returns default (empty) node map in case of failure
	bool AStarNodeMap::FromFile(const char * const filePath, AStarNodeMap *pMap)
	{
		std::ifstream inFile;

		// open the file, start at the beginning, error check
		inFile.open(filePath, std::ios::binary | std::ios::in);
		if (!inFile) { GameLogger::Log(MessageType::cError, "Failed to read file [%s]! Could not open file!\n", filePath); return false; }

		// read the format version
		int version = -1;
		inFile.read(reinterpret_cast<char*>(&version), sizeof(NODE_MAP_FILE_VERSION));

		// check that the version matches
		if (version != NODE_MAP_FILE_VERSION)
		{
			// if it is not the latest version, log an error and refuse to load the file
			GameLogger::Log(MessageType::cError, "FAILED TO READ IN WORLD FILE [%s]!!! NODE MAP FILE VERSION FOUND IN HEADER [%d] DOES NOT MATCH THE LATEST FILE VERSION [%d]!! PLEASE ENSURE YOU ARE USING THE LATEST VERSION OF NODE MAP FILE!\n", filePath, version, NODE_MAP_FILE_VERSION);
			return false;
		}

		// read in num nodes
		inFile.read(reinterpret_cast<char *>(&pMap->m_numNodes), sizeof(pMap->m_numNodes));

		// allocate array
		pMap->m_pNodesWithConnections = new NodeWithConnections[pMap->m_numNodes];

		// read in nodes into whole array
		for (unsigned i = 0; i < pMap->m_numNodes; ++i)
		{
			// read in the data for the node
			inFile.read(reinterpret_cast<char *>(&pMap->m_pNodesWithConnections[i].m_connectionCount), sizeof(pMap->m_pNodesWithConnections[i].m_connectionCount));
			inFile.read(reinterpret_cast<char *>(&pMap->m_pNodesWithConnections[i].m_connectionIndex), sizeof(pMap->m_pNodesWithConnections[i].m_connectionIndex));
			inFile.read(reinterpret_cast<char *>(&pMap->m_pNodesWithConnections[i].m_pNodeOrigin), sizeof(pMap->m_pNodesWithConnections[i].m_pNodeOrigin)); // read a nullptr, we delete the gob anyways

			// make a new node that should get cleaned up nicely, set its data below
			pMap->m_pNodesWithConnections[i].m_pNode = new AStarNode();

			// write out data for the AStarNode we'll be pointer-hookup-ing on the other side
			inFile.read(reinterpret_cast<char *>(&pMap->m_pNodesWithConnections[i].m_pNode->m_position), sizeof(pMap->m_pNodesWithConnections[i].m_pNode->m_position));
			inFile.read(reinterpret_cast<char *>(&pMap->m_pNodesWithConnections[i].m_pNode->m_radius), sizeof(pMap->m_pNodesWithConnections[i].m_pNode->m_radius));
			inFile.read(reinterpret_cast<char *>(&pMap->m_pNodesWithConnections[i].m_pNode->m_enabled), sizeof(pMap->m_pNodesWithConnections[i].m_pNode->m_enabled));
		}

		// read in num connections
		inFile.read(reinterpret_cast<char *>(&pMap->m_numConnections), sizeof(pMap->m_numConnections));

		// allocate array
		pMap->m_pConnectionsTo = new int[pMap->m_numConnections];

		// read in connections into whole array
		inFile.read(reinterpret_cast<char *>(&pMap->m_pConnectionsTo[0]), sizeof(pMap->m_pConnectionsTo[0]) * pMap->m_numConnections);

		// load the map from a file (VALIDATE VERSION, log error accordingly)
		return true;
	}

	// writes a node map to a file
	bool AStarNodeMap::ToFile(const AStarNodeMap * const mapToWrite, const char * const filePath)
	{
		std::ofstream outFile;

		// open the file, start at the beginning, error check
		outFile.open(filePath, std::ios::binary | std::ios::out);
		if (!outFile) { GameLogger::Log(MessageType::cError, "Failed to write file [%s]! Could not open file!\n", filePath); return false; }
		outFile.seekp(0);

		// write the format version
		outFile.write(reinterpret_cast<const char*>(&NODE_MAP_FILE_VERSION), sizeof(NODE_MAP_FILE_VERSION));

		// write num nodes
		outFile.write(reinterpret_cast<const char *>(&mapToWrite->m_numNodes), sizeof(mapToWrite->m_numNodes));

		// write out whole nodes array
		for (unsigned i = 0; i < mapToWrite->m_numNodes; ++i)
		{
			int q = 0;
			// write out data for the node
			outFile.write(reinterpret_cast<const char *>(&mapToWrite->m_pNodesWithConnections[i].m_connectionCount), sizeof(mapToWrite->m_pNodesWithConnections[i].m_connectionCount));
			outFile.write(reinterpret_cast<const char *>(&mapToWrite->m_pNodesWithConnections[i].m_connectionIndex), sizeof(mapToWrite->m_pNodesWithConnections[i].m_connectionIndex));
			outFile.write(reinterpret_cast<const char *>(&q), sizeof(q)); // write a nullptr, we delete the gob anyways

			// write out data for the AStarNode we'll be pointer-hookup-ing on the other side
			outFile.write(reinterpret_cast<const char *>(&mapToWrite->m_pNodesWithConnections[i].m_pNode->m_position), sizeof(mapToWrite->m_pNodesWithConnections[i].m_pNode->m_position));
			outFile.write(reinterpret_cast<const char *>(&mapToWrite->m_pNodesWithConnections[i].m_pNode->m_radius), sizeof(mapToWrite->m_pNodesWithConnections[i].m_pNode->m_radius));
			outFile.write(reinterpret_cast<const char *>(&mapToWrite->m_pNodesWithConnections[i].m_pNode->m_enabled), sizeof(mapToWrite->m_pNodesWithConnections[i].m_pNode->m_enabled));
		}

		// write out num connections
		outFile.write(reinterpret_cast<const char *>(&mapToWrite->m_numConnections), sizeof(mapToWrite->m_numConnections));

		// write out whole connections array
		outFile.write(reinterpret_cast<const char *>(&mapToWrite->m_pConnectionsTo[0]), sizeof(mapToWrite->m_pConnectionsTo[0]) * mapToWrite->m_numConnections);

		// indicate success
		return true;
	}

	// writes this node map to a file
	bool AStarNodeMap::ToFile(const char * const filePath)
	{
		return ToFile(this, filePath);
	}

	void AStarNodeMap::AddSphereGobToList(int index, LinkedList<GraphicalObject*>* pObjs, CollisionLayer nodeLayer, int * outCountToUpdate, SetUniformCallback uniformCallback, void * uniformInstance)
	{
		// obj should get deleted externally in list we put it in
		GraphicalObject *pSphere = new GraphicalObject();
		ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\Soccer.PC.scene", pSphere, ShapeGenerator::GetPCShaderID());

		// make go from i right to j right
		pSphere->SetScaleMat(Mat4::Scale(m_pNodesWithConnections[index].m_pNode->GetRadius() / RADIUS_MULTIPLIER));
		pSphere->SetTransMat(Mat4::Translation(m_pNodesWithConnections[index].m_pNode->GetPosition()));
		pSphere->CalcFullTransform();

		pSphere->GetMatPtr()->m_specularIntensity = 0.5f;

		// make it visible
		uniformCallback(pSphere, uniformInstance);

		// add it where it goes
		RenderEngine::AddGraphicalObject(pSphere);
		CollisionTester::AddGraphicalObjectToLayer(pSphere, nodeLayer);
		pObjs->AddToList(pSphere);
		(*outCountToUpdate)++;
	}

	void AStarNodeMap::AddArrowGobToList(const Vec3 & iRightVec, const Vec3 & iToJRightVec, int from, int to, LinkedList<GraphicalObject*>* pObjs, CollisionLayer connectionLayer, int * outCountToUpdate, SetUniformCallback uniformCallback, void *uniformInstance)
	{
		//GameLogger::Log(MessageType::ConsoleOnly, "IRV: (%.3f, %.3f, %.3f) | IJRV (%.3f, %.3f, %.3f) | FROM [%d] | TO [%d]\n", iRightVec.GetX(), iRightVec.GetY(), iRightVec.GetZ(), iToJRightVec.GetX(), iToJRightVec.GetY(), iToJRightVec.GetZ(), from, to);

		// obj should get deleted externally in list we put it in
		GraphicalObject *pArrow = new GraphicalObject();
		ShapeGenerator::MakeDebugArrow(pArrow, Vec3(0.15f, 0.75f, 0.0f), Vec3(0.0f, 0.75f, 0.0f));

		// make go from i right to j right
		pArrow->SetRotMat(Mat4::RotationToFace(BASE_ARROW_DIR, iToJRightVec.Normalize()));
		pArrow->SetScaleMat(Mat4::Scale(iToJRightVec.Length() / 2.0f, BASE_ARROW_DIR));
		pArrow->SetTransMat(Mat4::Translation(iRightVec + iToJRightVec / 2.0f));
		pArrow->CalcFullTransform();

		pArrow->GetMatPtr()->m_specularIntensity = 0.5f;

		// ugly make it work thing
		pArrow->fromTempDeleteMeLater = from;
		pArrow->toTempDeleteMeLater = to;

		// make it visible
		uniformCallback(pArrow, uniformInstance);

		// add it where it goes
		RenderEngine::AddGraphicalObject(pArrow);
		CollisionTester::AddGraphicalObjectToLayer(pArrow, connectionLayer);
		pObjs->AddToList(pArrow);
		(*outCountToUpdate)++;
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

		// Re-calculate the grid because we took stuff out!
		CollisionTester::CalculateGrid(connectionLayer);

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

	const int INDEX_NOT_SET = -1;
	bool AStarNodeMap::MakeAutomagicNodeConnections(LinkedList<GraphicalObject*>* pObjs, CollisionLayer connectionLayer, CollisionLayer geometryLayer, int *outCountToUpdate, SetUniformCallback uniformCallback, void *uniformInstance)
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
				Vec3 jLeft = jCenter + jRightOffset;
				Vec3 jRight = jCenter - jRightOffset;

				// vectors going from edges to other edges
				Vec3 iToJRight = jRight - iRight;
				Vec3 iToJLeft = jLeft - iLeft;

				// only check other points if the first path is clear, because raycasting is very expensive (will only do subsequent raycasts if the previous are a clear path
				if (PathClear(CollisionTester::FindWall(iCenter, iToJCenter.Normalize(), iToJCenter.Length(), geometryLayer), m_pNodesWithConnections[j].m_pNodeOrigin, iToJCenter.Length())
					&& PathClear(CollisionTester::FindWall(iRight, iToJRight.Normalize(), iToJRight.Length(), geometryLayer), m_pNodesWithConnections[j].m_pNodeOrigin, iToJRight.Length())
					&& PathClear(CollisionTester::FindWall(iLeft, iToJLeft.Normalize(), iToJLeft.Length(), geometryLayer), m_pNodesWithConnections[j].m_pNodeOrigin, iToJLeft.Length()))
				{
					// the index in the array is the start plus the num seen so far
					int arrayIndex = nextStartIndex + numICanSee;

					// put in that index the index of the node to which it is connected
					m_pConnectionsTo[arrayIndex] = j;

					// make the arrow gob
					AddArrowGobToList(iRight, iToJRight, i, j, pObjs, connectionLayer, outCountToUpdate, uniformCallback, uniformInstance);

					// update counts
					numICanSee++;
					m_numConnections++;
				}
			}

			// set start indices and counts, update variable, based on what seen within inner loop
			m_pNodesWithConnections[i].SetConnectionInfo(nextStartIndex, numICanSee);
			nextStartIndex += numICanSee;
		}

		// set the number of connections we have so far removed (0 to start)
		m_numRemoved = 0;

		// we added connections, yo!
		CollisionTester::CalculateGrid(connectionLayer);
		return true;
	}

	// returns false if a ray cast from one object to another hit anything in between
	bool AStarNodeMap::PathClear(const RayCastingOutput & rco, const GraphicalObject *pDestObj, float dist)
	{
		// the path is clear if we hit the destination object or it hit nothing withinthe distance
		return (rco.m_didIntersect && rco.m_belongsTo == pDestObj) || !(rco.m_didIntersect) || (rco.m_distance > dist);
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
		m_numRemoved++;
	}

	// returns true if an object is in the collision layer, altered to be match signature for linked list walk callback
	bool AStarNodeMap::IsObjInLayer(GraphicalObject * pObj, void * pDoinSomethingDifferentHere)
	{
		return Engine::CollisionTester::IsInLayer(pObj, *reinterpret_cast<Engine::CollisionLayer*>(pDoinSomethingDifferentHere));
	}

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

