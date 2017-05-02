#include "AStarNode.h"
#include "ShapeGenerator.h"
#include "RenderEngine.h"
#include "CollisionTester.h"

// Justin Furtado
// 5/1/2017
// AStarNode.h
// Defines an A-Star Pathing Node

namespace Engine
{
	// init static data
	AStarNode::ObjInitCallback AStarNode::s_connectionInitCallback = AStarNode::LogNotSetError;
	AStarNode::ObjInitCallback AStarNode::s_sphereInitCallback = AStarNode::LogNotSetError;
	void *AStarNode::s_pConnectionClassInstance = nullptr;
	void *AStarNode::s_pSphereInitClassInstance = nullptr;

	// some useful constants to avoid magic things
	const Vec3 BASE_ARROW_DIR = Engine::Vec3(1.0f, 0.0f, 0.0f);
	const CollisionLayer CONNECTION_LAYER = CollisionLayer::LAYER_3;
	const CollisionLayer SPHERE_LAYER = CollisionLayer::LAYER_4;

	AStarNode::AStarNode()
	{
	}

	AStarNode::~AStarNode()
	{
		ClearConnectedNodes();
	}

	// setter for obj init methods
	void AStarNode::SetSphereInitCallback(ObjInitCallback callback, void * pClass)
	{
		s_sphereInitCallback = callback;
		s_pSphereInitClassInstance = pClass;
	}

	// setter for obj init methods
	void AStarNode::SetArrowInitCallback(ObjInitCallback callback, void * pClass)
	{
		s_connectionInitCallback = callback;
		s_pConnectionClassInstance = pClass;
	}

	// initializez a graphical object for a connection
	void AStarNode::MakeConnectionGob(GraphicalObject * pGob)
	{
		// let the calling code (game/editor) tell how to set up shaders and stuff for the object through a callback
		s_connectionInitCallback(s_pConnectionClassInstance, pGob);
	}

	// initializes the graphical object for this node
	void AStarNode::InitGob()
	{
		// let the calling code (game/editor) tell how to set up shaders and stuff for the object through a callback
		s_sphereInitCallback(s_pSphereInitClassInstance, &m_gob);
	}

	bool AStarNode::CanGoToNode(AStarNode * pOtherNode)
	{
		// walk returns false and stops if a callback returns false, in this case we exploit that to check if a node exists in our list
		// the callback returns false if the pointer passed equals the connections node pointer, effectively stopping the loop and returning false when the nodes are equal
		// if the end of the list is reached, it will return true. To determine if a node is in the list with said callback, we simply negate the return value!
		// Also known as magic
		return !m_connections.WalkList(AStarNode::CheckAStarNode, pOtherNode);
	}

	void AStarNode::ConnectToNode(AStarNode * pNodeToConnectTo)
	{
		// allocate memory for a new connection
		Connection *pNewConnection = new Connection();

		// point to the AStarNode we are trying to add
		pNewConnection->m_pConnectedTo = pNodeToConnectTo;
		
		// Initialize gob
		MakeConnectionGob(&pNewConnection->m_connectionArrow);

		// Set the graphical object's transforms up
		UpdateConnection(pNewConnection, this);

		// add the connections GOB to the rendering and collision systems
		RenderEngine::AddGraphicalObject(&pNewConnection->m_connectionArrow);
		CollisionTester::AddGraphicalObjectToLayer(&pNewConnection->m_connectionArrow, CONNECTION_LAYER);

		// add the connection to the list of connections
		m_connections.AddToList(pNewConnection);

		// increment the connection count
		m_connectionCount++;
	}

	void AStarNode::StopConnectingTo(AStarNode *pNodeToNotConnectTo)
	{
		// find the connection which has this AStarNode inside
		Connection *pConnection = m_connections.GetWhere(AStarNode::ConnectionContains, pNodeToNotConnectTo);

		// delete that connection
		DeleteConnection(pConnection, this);

		// remove the connection from the list (only compares addresses, should be fine that it is deleted)
		m_connections.RemoveFromList(pConnection);
	}

	void AStarNode::WalkEnabledConnectedNodes(LinkedList<AStarNode>::LinkedListIterationCallback callback, void * pClassInstance)
	{
		// make an instance of a struct that contains the callback we want the outside to be able to call, and pass through the instance for each node in the list
		PassThroughConnectionToNode m_uglyThingIDoToMakeItWork;
		m_uglyThingIDoToMakeItWork.callback = callback;
		m_uglyThingIDoToMakeItWork.pClassInstance = pClassInstance;

		m_connections.WalkList(AStarNode::WalkEnabledConnectedAStarNodes, &m_uglyThingIDoToMakeItWork);

		if (m_enabled)
		{
			// in case nodes were changed at all, update the connections
			m_connections.WalkList(AStarNode::UpdateConnection, this);
		}
	}

	void AStarNode::ClearConnectedNodes()
	{
		// delete the connections we allocated
		m_connections.WalkList(AStarNode::DeleteConnection, this);

		// check for memory leak
		if (m_connectionCount != 0) { GameLogger::Log(MessageType::cError, "Failed to ClearConnectedNodes! Connection Count was [%d]! Check for memory leak or counter inaccuracy!\n", m_connectionCount); }
			
		// removes all connections
		m_connections.ClearList();
	}

	unsigned int AStarNode::GetConnectedNodeCount()
	{
		// simple getter
		return m_connections.GetCount();
	}

	Vec3 AStarNode::GetPosition()
	{
		// simple getter
		return m_position;
	}

	void AStarNode::SetPosition(const Vec3 & newPosition)
	{
		// move
		m_position = newPosition;

		if (m_enabled)
		{
			// we moved, we need to move our obj
			m_gob.SetTransMat(Mat4::Translation(m_position));
			m_gob.CalcFullTransform();

			// we moved, we need to move our arrows
			m_connections.WalkList(UpdateConnection, this);
		}
	}

	float AStarNode::GetRadius()
	{
		// simple getter
		return m_radius;
	}

	const float SIZE_MULTIPLIER = 1.0f;
	void AStarNode::SetRadius(float newRadius)
	{
		// get fatter/thinner
		m_radius = newRadius;

		if (m_enabled)
		{
			// update our object
			m_gob.SetScaleMat(Mat4::Scale(m_radius * SIZE_MULTIPLIER));
			m_gob.CalcFullTransform();

			// our connection objects need to be walked
			m_connections.WalkList(UpdateConnection, this);
		}
	}

	// sets whether the geometry is visible
	void AStarNode::SetVisualEnabled(bool enabled)
	{
		m_gob.SetEnabled(enabled);
		m_connections.WalkList((enabled ? AStarNode::EnableObjForConnection : AStarNode::DisableObjForConnection), this);
	}

	// sets whether the node can be used AND whether it is visible
	void AStarNode::SetNodeEnabled(bool enabled)
	{
		m_enabled = enabled;
		SetVisualEnabled(m_enabled);

		// we only set the object properties if it is enabled, so when we turn it on we need to update them
		// (if we're turning nodes off and on, its less redundant to only move the associated gobs when we turn it on, as opposed to every time we move it)
		// we do this because things outside the class can modify the position/scale while it is disabled, so we reduce necessary checks through this
		if (m_enabled)
		{
			m_gob.SetTransMat(Mat4::Translation(m_position));
			m_gob.SetScaleMat(Mat4::Scale(m_radius * SIZE_MULTIPLIER));
			m_gob.CalcFullTransform();

			// our connection objects need to be walked
			m_connections.WalkList(UpdateConnection, this);
		}
	}

	// lets the users of the Engine know they didn't set an object initializer for AStarNodes
	void AStarNode::LogNotSetError(void * /*pClass*/, GraphicalObject * /*pObj*/)
	{
		GameLogger::Log(MessageType::cError, "Tried to call ObjInitCallback but callback was not yet set!\n");
	}

	// enables the connection obj
	bool AStarNode::EnableObjForConnection(Connection *pConnectedTo, void * /*pClassInstance*/)
	{
		pConnectedTo->m_connectionArrow.SetEnabled(true);
		return true;
	}

	// disables the connection obj
	bool AStarNode::DisableObjForConnection(Connection *pConnectedTo, void * /*pClassInstance*/)
	{
		pConnectedTo->m_connectionArrow.SetEnabled(false);
		return true;
	}

	bool AStarNode::UpdateConnection(Connection *pConnectedTo, void * pClassInstance)
	{
		AStarNode *pNode = reinterpret_cast<AStarNode *>(pClassInstance);

		// get vector pointing up
		Engine::Vec3 up(0.0f, 1.0f, 0.0f);

		// get vector going from center of current node to center of node connected to
		Engine::Vec3 connectionVec = pConnectedTo->m_pConnectedTo->GetPosition() - pNode->GetPosition();

		// Get the vector going from the center of the sphere to the side of the sphere in a direction perpendicular to both the up vector and the vector pointing to the node connected to
		Engine::Vec3 side = connectionVec.Cross(up).Normalize() * pNode->GetRadius();

		// find the point on the side of the sphere from the sphere's position and the side vector
		Engine::Vec3 sideStart = pNode->GetPosition() + side;

		// make the arrow in the connection point from sideStart to sideStart plus connectionVec
		pConnectedTo->m_connectionArrow.SetRotMat(Mat4::RotationToFace(BASE_ARROW_DIR, connectionVec.Normalize()));
		pConnectedTo->m_connectionArrow.SetTransMat(Mat4::Translation(sideStart));
		pConnectedTo->m_connectionArrow.SetScaleMat(Mat4::Scale(connectionVec.Length() / 2.0f, BASE_ARROW_DIR));
		pConnectedTo->m_connectionArrow.CalcFullTransform(); // scale then rotate then translate

		// keep going
		return true;
	}

	bool AStarNode::WalkEnabledConnectedAStarNodes(Connection * pConnectedTo, void * pImDoingSomethingDifferentHere)
	{
		// do a pass through with ugly struct technique!
		// call the callback ONLY IF THE NODE IS ENABLED!!!
		PassThroughConnectionToNode *pUglyThingIDoToMakeItWorkInTheOtheMethod = reinterpret_cast<PassThroughConnectionToNode*>(pImDoingSomethingDifferentHere);
		return pConnectedTo->m_pConnectedTo->m_enabled ? true : pUglyThingIDoToMakeItWorkInTheOtheMethod->callback(pConnectedTo->m_pConnectedTo, pUglyThingIDoToMakeItWorkInTheOtheMethod->pClassInstance);
	}

	bool AStarNode::CheckAStarNode(Connection * pConnection, void * pImDoingSomethingDifferentHere)
	{
		// abort by returning false if nodes are equal
		return (pConnection->m_pConnectedTo != pImDoingSomethingDifferentHere);
	}

	// releases allocated memory for connection
	bool AStarNode::DeleteConnection(Connection * pConnection, void * pClassInstance)
	{
		// remove the connections GOB from rendering and collision 
		RenderEngine::RemoveGraphicalObject(&pConnection->m_connectionArrow);
		CollisionTester::RemoveGraphicalObjectFromLayer(&pConnection->m_connectionArrow, CONNECTION_LAYER); 

		// release memory for the connection
		delete pConnection;

		// reduce the connection count
		--reinterpret_cast<AStarNode*>(pClassInstance)->m_connectionCount;

		// indicate success
		return true;
	}

	// checks if a connection contains an A-Star Node... passed in through void * to match callback
	bool AStarNode::ConnectionContains(Connection * pConnection, void * pImDoingSomethingDifferentHere)
	{
		return pConnection->m_pConnectedTo == pImDoingSomethingDifferentHere;
	}
}
