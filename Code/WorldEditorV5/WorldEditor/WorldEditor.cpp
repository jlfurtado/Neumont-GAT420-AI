#include "WorldEditor.h"
#include "WorldFileIO.h"
#include "StringFuncs.h"
#include "BinaryWriter.h"
#include "GameLogger.h"
#include <iostream>
#include "Mat4.h"
#include "RenderEngine.h"
#include "ConfigReader.h"
#include "MyGL.h"
#include "ShapeGenerator.h"
#include "MousePicker.h"
#include "MathUtility.h"
#include "MouseManager.h"
#include "AStarNode.h"

// Justin Furtado
// 4/20/2017
// WorldEditor.h
// Enables editing of the world!

const WorldEditor::PlacementData WorldEditor::s_placementData[NUM_PLACEMENT_DATA]{ PlacementData("Placing: Cube\n", WorldEditor::MakeCube),
																				   PlacementData("Placing: Hideout\n", WorldEditor::MakeHideout),
																				   PlacementData("Placing: BetterDargon\n", WorldEditor::MakeBetterDargon),
																				   PlacementData("Placing: Chair\n", WorldEditor::MakeChair),
																				   PlacementData("Placing: Cone\n", WorldEditor::MakeCone),
																				   PlacementData("Placing: Coil\n", WorldEditor::MakeCoil),
																				   PlacementData("Placing: Cup\n", WorldEditor::MakeCup),
																				   PlacementData("Placing: Tree\n", WorldEditor::MakeTree),
																				   PlacementData("Placing: Pipe\n", WorldEditor::MakePipe),
																				   PlacementData("Placing: Wedge\n", WorldEditor::MakeWedge),
																				   PlacementData("Placing: Star\n", WorldEditor::MakeStar),
																				   PlacementData("Placing: House\n", WorldEditor::MakeHouse),
																				   PlacementData("Placing: AStarNodeGob\n", WorldEditor::MakeNodeObj) };


const float MOVE_MORE = 0.5f;
const Engine::Vec3 BASE_ARROW_DIR = Engine::Vec3(1.0f, 0.0f, 0.0f);
const Engine::Vec3 PLUS_X = Engine::Vec3(1.0f, 0.0f, 0.0f);
const Engine::Vec3 PLUS_Y = Engine::Vec3(0.0f, 1.0f, 0.0f);
const Engine::Vec3 PLUS_Z = Engine::Vec3(0.0f, 0.0f, 1.0f);
const float ARROW_SCALE = 10.0f;
const Engine::Vec3 X_ARROW_OFFSET = PLUS_X * (ARROW_SCALE + MOVE_MORE);
const Engine::Vec3 Y_ARROW_OFFSET = PLUS_Y * (ARROW_SCALE + MOVE_MORE);
const Engine::Vec3 Z_ARROW_OFFSET = PLUS_Z * (ARROW_SCALE + MOVE_MORE);

const Engine::Vec3 WorldEditor::RED{ 1.0f, 0.0f, 0.0f };
const Engine::Vec3 WorldEditor::YELLOW{ 1.0f, 1.0f, 0.0f };
const Engine::Vec3 WorldEditor::GREEN{ 0.0f, 1.0f, 0.0f };
const Engine::Vec3 WorldEditor::BLUE{ 0.0f, 0.0f, 1.0f };

const Engine::CollisionLayer EDITOR_ITEMS = Engine::CollisionLayer::LAYER_1;
const Engine::CollisionLayer EDITOR_LIST_OBJS = Engine::CollisionLayer::LAYER_9;
const Engine::CollisionLayer NODE_LAYER = Engine::CollisionLayer::LAYER_2; // TODO: IMPORTANT, place node objs in this layer
const Engine::CollisionLayer CONNECTION_LAYER = Engine::CollisionLayer::LAYER_3; // TODO: IMPORTANT, make sure connections go in this layer
// TODO: IMPORTANT, make sure raycasts check correct layers and correct layers are recalculated at correct times

const float RENDER_DISTANCE = 2500.0f;
bool WorldEditor::InitializeCallback(void * game, Engine::MyWindow * pWindow)
{
	if (!game) { return false; }
	return reinterpret_cast<WorldEditor *>(game)->Initialize(pWindow);
}

void WorldEditor::UpdateCallback(void * game, float dt)
{
	if (!game) { return; }
	reinterpret_cast<WorldEditor *>(game)->Update(dt);
}

void WorldEditor::ResizeCallback(void * game)
{
	if (!game) { return; }
	reinterpret_cast<WorldEditor *>(game)->OnResizeWindow();
}

void WorldEditor::DrawCallback(void * game)
{
	if (!game) { return; }
	reinterpret_cast<WorldEditor *>(game)->Draw();
}

void WorldEditor::MouseScrollCallback(void * game, int degrees)
{
	if (!game) { return; }
	reinterpret_cast<WorldEditor *>(game)->OnMouseScroll(degrees);
}

void WorldEditor::MouseMoveCallback(void * game, int dx, int dy)
{
	if (!game) { return; }
	reinterpret_cast<WorldEditor *>(game)->OnMouseMove(dx, dy);
}

bool WorldEditor::DestroyObjsCallback(Engine::GraphicalObject * pObj, void *pClassInstance)
{
	WorldEditor* pEditor = reinterpret_cast<WorldEditor*>(pClassInstance);
	
	if (pEditor->m_pSelected == pObj) { pEditor->DeSelect(); }
	if (pEditor->m_pLastHit == pObj) { pEditor->DeMouseOver(); }

	Engine::RenderEngine::RemoveGraphicalObject(pObj);
	Engine::CollisionTester::RemoveGraphicalObjectFromLayer(pObj, Engine::CollisionLayer::NUM_LAYERS); // removes object from all layers
	delete pObj;

	pEditor->m_objCount--;
	
	return true;
}

bool WorldEditor::CopyObjList(Engine::GraphicalObject * pObj, void * pDoingSomethingDifferent)
{
	Engine::LinkedList<Engine::GraphicalObject *> *pObjCopy = reinterpret_cast<Engine::LinkedList<Engine::GraphicalObject *>*>(pDoingSomethingDifferent);
	
	pObjCopy->AddToList(pObj);

	return true;
}

void WorldEditor::PlaceObject(WorldEditor *pEditor)
{
	if (Engine::MouseManager::IsLeftMouseClicked() && (pEditor->m_rco.m_didIntersect || pEditor->m_objs.GetCount() == 0))
	{
		// make an obj with the callback
		Engine::CollisionLayer outLayer;
		Engine::GraphicalObject *pNewObj = s_placementData[pEditor->m_currentPlacement].m_callback(pEditor, &outLayer);

		// if scene is empty, place at 0 0 0, else, place at where clicked
		pNewObj->SetTransMat(Engine::Mat4::Translation(pEditor->m_objs.GetCount() == 0 ? Engine::Vec3(0.0f) : pEditor->m_rco.m_intersectionPoint + (pNewObj->GetScaleMatPtr()[0] * pEditor->m_rco.m_triangleNormal.Normalize())));
		if (pEditor->m_objs.GetCount() != 0) { pNewObj->SetRotMat(Engine::Mat4::RotationToFace(PLUS_Y, pEditor->m_rco.m_triangleNormal)); }
		pNewObj->CalcFullTransform();

		// add it to the necessary things, it'll get deleted on shutdown or remove
		Engine::RenderEngine::AddGraphicalObject(pNewObj);
		Engine::CollisionTester::AddGraphicalObjectToLayer(pNewObj, outLayer);
		
		pEditor->m_objs.AddToList(pNewObj);
		pEditor->m_objCount++;

		pEditor->HandleOutsideGrid(pNewObj);
	}
}

void WorldEditor::RemoveObject(WorldEditor *pEditor)
{
	// show which object will be acted upon
	pEditor->DoMouseOverHighlight();

	if (Engine::MouseManager::IsLeftMouseClicked() && pEditor->m_rco.m_didIntersect && pEditor->m_objs.Contains(pEditor->m_rco.m_belongsTo))
	{
		pEditor->DeMouseOver();
		Engine::CollisionLayer cl = CONNECTION_LAYER;

		if (Engine::AStarNodeMap::IsObjInLayer(pEditor->m_rco.m_belongsTo, &cl)) { pEditor->m_nodeMap.RemoveConnection(&pEditor->m_objs, pEditor->m_rco.m_belongsTo, WorldEditor::DestroyObjsCallback, pEditor, &pEditor->m_objCount); }
		else
		{
			DestroyObjsCallback(pEditor->m_rco.m_belongsTo, pEditor);
			pEditor->m_objs.RemoveFirstFromList(pEditor->m_rco.m_belongsTo);
		}

		Engine::CollisionTester::CalculateGrid(Engine::CollisionLayer::NUM_LAYERS); // need to recalc grid so not colliding with non-existant objects	
		// TODO: only recalculate the layer we need
	}
}

// TODO: REFACTOR
const float tolerance = 0.00001f;
void WorldEditor::TranslateObject(WorldEditor *pEditor)
{
	static Engine::Vec3 lastOrigin;
	static Engine::Vec3 v;
	static Engine::Vec3 d;
	static bool arrowClicked = false;

	if (pEditor->m_pSelected)
	{
		Engine::RayCastingOutput arrowCheck = Engine::CollisionTester::FindFromMousePos(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY(), RENDER_DISTANCE, EDITOR_ITEMS);

		if (arrowCheck.m_didIntersect && Engine::MouseManager::IsLeftMouseClicked())
		{			
			arrowClicked = true;
			d = arrowCheck.m_belongsTo->GetRotMat() * BASE_ARROW_DIR;
			lastOrigin = Engine::MousePicker::GetOrigin(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY());
			v = arrowCheck.m_intersectionPoint - lastOrigin;
		}
		else if (Engine::MouseManager::IsLeftMouseDown() && arrowClicked)
		{
			Engine::Vec3 newOrigin = Engine::MousePicker::GetOrigin(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY());
			Engine::Vec3 r = Engine::MousePicker::GetDirection(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY()) + (newOrigin - lastOrigin);
			
			// HOLY MATH BATMAN!!!
			Engine::Vec3 vhat = v.Normalize();
			Engine::Vec3 rhat = r.Normalize();
			Engine::Vec3 innerCross = rhat.Cross(vhat);
			if (innerCross.LengthSquared() > tolerance)
			{
				Engine::Vec3 movementAmount = (vhat.Cross(innerCross) * v.Length() * tanf(acosf(vhat.Dot(rhat)))).ProjectOnto(d);

				pEditor->MoveSelectedObjectTo(pEditor->m_pSelected->GetPos() + pEditor->m_adjustmentSpeedMultiplier*movementAmount);
				pEditor->AttachArrowsTo(pEditor->m_pSelected);

				v = v + lastOrigin - newOrigin + movementAmount;
				lastOrigin = newOrigin;
			}
		}	


		if (Engine::MouseManager::IsLeftMouseReleased())
		{
			arrowClicked = false;
			pEditor->HandleOutsideGrid(pEditor->m_pSelected);
			Engine::CollisionTester::CalculateGrid(EDITOR_ITEMS);
		}
	}
	
	if (!arrowClicked)
	{
		// enable selection and de-selection of objects
		pEditor->DoSelection();
	}

}

void WorldEditor::RotateObject(WorldEditor *pEditor)
{
	static Engine::Mat4 startRot;
	static Engine::Vec3 lastOrigin;
	static Engine::Vec3 v;
	static Engine::Vec3 d;
	static bool arrowClicked = false;

	Engine::CollisionLayer layerCheck = NODE_LAYER;

	if (pEditor->m_pSelected)
	{
		Engine::RayCastingOutput arrowCheck = Engine::CollisionTester::FindFromMousePos(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY(), RENDER_DISTANCE, EDITOR_ITEMS);

		if (arrowCheck.m_didIntersect && Engine::MouseManager::IsLeftMouseClicked())
		{
			arrowClicked = true;
			startRot = pEditor->m_pSelected->GetRotMat();
			d = arrowCheck.m_belongsTo->GetRotMat() * BASE_ARROW_DIR;
			lastOrigin = Engine::MousePicker::GetOrigin(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY());
			v = arrowCheck.m_intersectionPoint - lastOrigin;
		}
		else if (Engine::MouseManager::IsLeftMouseDown() && arrowClicked /* && !Engine::AStarNodeMap::IsObjInLayer(pEditor->m_pSelected, &layerCheck)*/)
		{
			Engine::Vec3 newOrigin = Engine::MousePicker::GetOrigin(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY());
			Engine::Vec3 r = Engine::MousePicker::GetDirection(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY()) + (newOrigin - lastOrigin);

			// HOLY MATH BATMAN!!!
			Engine::Vec3 vhat = v.Normalize();
			Engine::Vec3 rhat = r.Normalize();
			Engine::Vec3 innerCross = rhat.Cross(vhat);
			if (innerCross.LengthSquared() > tolerance)
			{
				Engine::Vec3 s1 = v + lastOrigin - pEditor->m_pSelected->GetPos();
				Engine::Vec3 rminusv = (vhat.Cross(innerCross) * v.Length() * tanf(acosf(vhat.Dot(rhat))));
				Engine::Vec3 d2 = s1 + rminusv;

				pEditor->m_pSelected->SetRotMat(Engine::Mat4::RotationToFace(d, d2) * startRot);
				pEditor->m_pSelected->CalcFullTransform();
				pEditor->AttachArrowsTo(pEditor->m_pSelected);

				lastOrigin = newOrigin;

			}
		}


		if (Engine::MouseManager::IsLeftMouseReleased())
		{
			arrowClicked = false;
			pEditor->HandleOutsideGrid(pEditor->m_pSelected);
			Engine::CollisionTester::CalculateGrid(EDITOR_ITEMS);
		}
	}

	if (!arrowClicked)
	{
		// enable selection and de-selection of objects
		pEditor->DoSelection();
	}
}

void WorldEditor::ScaleObject(WorldEditor *pEditor)
{
	static Engine::Vec3 lastOrigin;
	static Engine::Vec3 v;
	static Engine::Vec3 d;
	static bool arrowClicked = false;

	if (pEditor->m_pSelected)
	{
		Engine::RayCastingOutput arrowCheck = Engine::CollisionTester::FindFromMousePos(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY(), RENDER_DISTANCE, EDITOR_ITEMS);

		if (arrowCheck.m_didIntersect && Engine::MouseManager::IsLeftMouseClicked())
		{
			arrowClicked = true;
			d = pEditor->GetArrowDir(arrowCheck.m_belongsTo);
			lastOrigin = Engine::MousePicker::GetOrigin(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY());
			v = arrowCheck.m_intersectionPoint - lastOrigin;
		}
		else if (Engine::MouseManager::IsLeftMouseDown() && arrowClicked)
		{
			Engine::Vec3 newOrigin = Engine::MousePicker::GetOrigin(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY());
			Engine::Vec3 r = Engine::MousePicker::GetDirection(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY()) + (newOrigin - lastOrigin);

			// HOLY MATH BATMAN!!!
			Engine::Vec3 vhat = v.Normalize();
			Engine::Vec3 rhat = r.Normalize();
			Engine::Vec3 innerCross = rhat.Cross(vhat);
			if (innerCross.LengthSquared() > tolerance)
			{
				Engine::Vec3 movementAmount = (vhat.Cross(innerCross) * v.Length() * tanf(acosf(vhat.Dot(rhat)))).ProjectOnto(d);

				float scaleAmount = d.Dot(movementAmount) > 0.0f ? 1 + pEditor->m_adjustmentSpeedMultiplier*movementAmount.Length() : 1 - pEditor->m_adjustmentSpeedMultiplier*movementAmount.Length();
				Engine::CollisionLayer layerCheck = NODE_LAYER;
				if (Engine::AStarNodeMap::IsObjInLayer(pEditor->m_pSelected, &layerCheck))
				{
					pEditor->m_pSelected->SetScaleMat((pEditor->m_pSelected->GetScaleMat() * Engine::Mat4::Scale(scaleAmount)));
				}
				else
				{
					pEditor->m_pSelected->SetScaleMat((pEditor->m_pSelected->GetScaleMat() * Engine::Mat4::Scale(scaleAmount, movementAmount.Normalize())));
				}

				pEditor->m_pSelected->CalcFullTransform();
				pEditor->AttachArrowsTo(pEditor->m_pSelected);

				v = v + lastOrigin - newOrigin + movementAmount;
				lastOrigin = newOrigin;
			}
		}


		if (Engine::MouseManager::IsLeftMouseReleased())
		{
			arrowClicked = false;
			pEditor->HandleOutsideGrid(pEditor->m_pSelected);
			Engine::CollisionTester::CalculateGrid(EDITOR_ITEMS);
		}
	}

	if (!arrowClicked)
	{
		// enable selection and de-selection of objects
		pEditor->DoSelection();
	}

}

void WorldEditor::SetPCUniforms(Engine::GraphicalObject * pObj, void *editor)
{
	WorldEditor *pEditor = reinterpret_cast<WorldEditor *>(editor);

	pObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pObj->GetFullTransformPtr(), pEditor->modelToWorldMatLoc));
	pObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, &pEditor->wtv, pEditor->worldToViewMatLoc));
	pObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pEditor->m_perspective.GetPerspectivePtr(), pEditor->perspectiveMatLoc));
	pObj->AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &pObj->GetMatPtr()->m_materialColor, pEditor->tintLoc));
	pObj->AddUniformData(Engine::UniformData(GL_FLOAT, &pObj->GetMatPtr()->m_specularIntensity, pEditor->tintIntensityLoc));
}

bool WorldEditor::Initialize(Engine::MyWindow * pWindow)
{
	// set pointer to window so we can talk to it from the class
	m_pWindow = pWindow;

	// do some opengl setup
	if (!InitializeGL())
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to initialize WorldEditor! Could not InitializeGL()!\n");
		return false;
	}

	// initialize the render engine with the shaders we need to tell it about
	if (!Engine::RenderEngine::Initialize(&m_shaderPrograms[0], NUM_SHADER_PROGRAMS))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to initialize WorldEditor! Could not initialize RenderEngine!\n");
		return false;
	}

	// initialize the shape generator
	if (!Engine::ShapeGenerator::Initialize(m_shaderPrograms[1].GetProgramId(), m_shaderPrograms[0].GetProgramId(), m_shaderPrograms[3].GetProgramId()))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to initialize WorldEditor! Failed to initialize ShapeGenerator!\n");
		return false;
	}

	Engine::CollisionTester::SetGridScale(25.0f);

	// place the default objects into the empty world
	if (!UglyDemoCode())
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to initialize WorldEditor! UglyDemoCode failed!\n");
		return false;
	}

	// read in config values
	if (!ReadConfigValues())
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to initialize WorldEditor! Could not ReadConfigValues!\n");
		return false;
	}

	// setup keys for the world editor
	if (!keyboardManager.AddKeys("XWASD1234567890MNK ") || !keyboardManager.AddKey(VK_SHIFT)
		|| !keyboardManager.AddToggle('G', &drawGrid))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to add keys for WorldEditor!\n");
		return false;
	}
	
	// calculate the spatial grid for the currently existing objects
	if (!Engine::CollisionTester::CalculateGrid())
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to calculate spatial grid for WorldEditor!\n");
		return false;
	}

	SetArrowEnabled(false);


	Engine::GameLogger::Log(Engine::MessageType::cProcess, "Successfully initialized WorldEditor!\n");
	return true;
}

bool WorldEditor::Shutdown()
{
	// display info on shutdown
	Engine::CollisionTester::ConsoleLogOutput();
	Engine::RenderEngine::LogStats();
	if (!m_pWindow->Shutdown()) { return false; }

	for (int i = 0; i < NUM_SHADER_PROGRAMS; ++i)
	{
		if (!m_shaderPrograms[i].Shutdown()) { return false; }
	}

	// have to clear objs before shape gen, cuz shape gen deletes mesh before render engine can remove it
	m_objs.WalkList(DestroyObjsCallback, this);
	if (m_objCount != 0) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to DestroyObjs! Check for memory leak or counter inaccuracy [%d] objs left!\n", m_objCount); return false; }

	if (!Engine::TextObject::Shutdown()) { return false; }
	if (!Engine::RenderEngine::Shutdown()) { return false; }
	if (!Engine::ShapeGenerator::Shutdown()) { return false; }

	Engine::GameLogger::Log(Engine::MessageType::cProcess, "Game Shutdown Successfully!!!\n");
	return true;
}

const float CHECK_DIST = 250.0f;
const float DOWN_CHECK = 1.0f;
const float DOWN_OFFSET = 0.75f;
const float FORWARD_CHECK = 0.10f;
const float FORWARD_OFFSET = 0.01f;
void WorldEditor::Update(float dt)
{
	keyboardManager.Update(dt);
	if (!ProcessInput(dt)) { return; }
	ShowFrameRate(dt);
	
	if (m_walkEnabled)
	{
		Engine::RayCastingOutput groundRCO = Engine::CollisionTester::FindWall(m_camera.GetPosition() + (DOWN_CHECK - DOWN_OFFSET) * PLUS_Y, -PLUS_Y, CHECK_DIST, EDITOR_LIST_OBJS);

		if (groundRCO.m_didIntersect)
		{
			m_camera.SetPosition(m_camera.GetPosition() +  (PLUS_Y * (DOWN_CHECK - groundRCO.m_distance)));
		}

	}
	
	wtv = m_camera.GetWorldToViewMatrix();

	/*static int lastX = 0;
	static int lastZ = 0;
	static int lastY = 0;
	static Engine::CollisionLayer lastCollisionLayer;

	float x = m_camera.GetPosition().GetX();
	float y = m_camera.GetPosition().GetY();
	float z = m_camera.GetPosition().GetZ();
	int cX = Engine::CollisionTester::GetGridIndexFromPosX(x, EDITOR_LIST_OBJS);
	int cY = Engine::CollisionTester::GetGridIndexFromPosX(y, EDITOR_LIST_OBJS);
	int cZ = Engine::CollisionTester::GetGridIndexFromPosZ(z, EDITOR_LIST_OBJS);

	if (cX != lastX || cZ != lastZ || cY != lastY)
	{
		char buffer[75], buffer2[75];
		if (Engine::CollisionTester::GetTriangleCountForSpace(x, y, z) < 0) { sprintf_s(buffer, 50, "Outside Spatial Grid!\n"); }
		else { sprintf_s(buffer, 75, "[%d] triangles in [%d] [%d] [%d]\n", Engine::CollisionTester::GetTriangleCountForSpace(x, y, z, EDITOR_LIST_OBJS), cX, cY, cZ); }
		Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "%s\n", buffer);

		lastX = cX;
		lastY = cY;
		lastZ = cZ;
	}*/

	Engine::MousePicker::SetCameraInfo(m_camera.GetPosition(), m_camera.GetViewDir(), m_camera.GetUp());

	m_rco = Engine::CollisionTester::FindFromMousePos(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY(), RENDER_DISTANCE);

	m_currentMode(this);
}

void WorldEditor::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	bool arrowEnabled = m_xArrow.IsEnabled();

	glEnable(GL_DEPTH_TEST);

	SetArrowEnabled(false);
	Engine::RenderEngine::Draw();
	SetArrowEnabled(arrowEnabled);

	if (drawGrid) { Engine::CollisionTester::DrawGrid(EDITOR_ITEMS, m_camera.GetPosition()); }

	glDisable(GL_DEPTH_TEST);

	if (arrowEnabled)
	{
		Engine::RenderEngine::DrawSingleObjectRegularly(&m_xArrow);
		Engine::RenderEngine::DrawSingleObjectRegularly(&m_yArrow);
		Engine::RenderEngine::DrawSingleObjectRegularly(&m_zArrow);
	}


	m_fpsTextObject.RenderText(&m_shaderPrograms[0], debugColorLoc);
	m_modeText.RenderText(&m_shaderPrograms[0], debugColorLoc);
	m_placingText.RenderText(&m_shaderPrograms[0], debugColorLoc);
}

void WorldEditor::OnResizeWindow()
{
	if (m_pWindow == nullptr) { return; }
	float aspect = static_cast<float>(m_pWindow->width()) / m_pWindow->height();
	m_perspective.SetAspectRatio(aspect);
	m_perspective.SetScreenDimmensions(static_cast<float>(m_pWindow->width()), static_cast<float>(m_pWindow->height()));
	Engine::MousePicker::SetPerspectiveInfo(m_perspective.GetFOVY(), m_perspective.GetNearDist(), m_perspective.GetWidth(), m_perspective.GetHeight());
	glViewport(0, 0, m_pWindow->width(), m_pWindow->height());
}

void WorldEditor::OnMouseScroll(int /*degrees*/)
{

}

void WorldEditor::OnMouseMove(int deltaX, int deltaY)
{
	if (Engine::MouseManager::IsRightMouseDown()) { m_camera.MouseRotate(deltaX, deltaY); }
}

bool WorldEditor::ReadConfigValues()
{
	if (!Engine::ConfigReader::pReader->GetClampedFloatForKey("WorldEditor.FpsInterval", m_fpsInterval, 0.5f, 5.0f)) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get float for key FpsInterval!\n"); return false; }
	
	float cameraSpeed = 1.0f;
	if (!Engine::ConfigReader::pReader->GetClampedFloatForKey("WorldEditor.CameraSpeed", cameraSpeed, 0.0f, 9999999999.0f)) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get float for key CameraSpeed!\n"); return false; }
	m_camera.SetSpeed(cameraSpeed);

	float cameraRotationSpeed = 1.0f;
	if (!Engine::ConfigReader::pReader->GetClampedFloatForKey("WorldEditor.CameraRotationSpeed", cameraRotationSpeed, 0.0f, 999999999.0f)) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get float for key CameraRotationSpeed!\n"); return false; }
	m_camera.SetRotateSpeed(cameraRotationSpeed);

	Engine::GameLogger::Log(Engine::MessageType::cProcess, "WorldEditor successfully read in config values!\n");
	return true;
}

bool WorldEditor::InitializeGL()
{
	glViewport(0, 0, m_pWindow->width(), m_pWindow->height());

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);

	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);


	glClearStencil(0);

	if (m_shaderPrograms[0].Initialize())
	{
		m_shaderPrograms[0].AddVertexShader("..\\Data\\Shaders\\Debug.Vert.shader");
		m_shaderPrograms[0].AddFragmentShader("..\\Data\\Shaders\\Debug.Frag.shader");
		m_shaderPrograms[0].LinkProgram();
		m_shaderPrograms[0].UseProgram();
	}

	if (m_shaderPrograms[1].Initialize())
	{
		m_shaderPrograms[1].AddVertexShader("..\\Data\\Shaders\\PC.vert.shader");
		m_shaderPrograms[1].AddFragmentShader("..\\Data\\Shaders\\PC.frag.shader");
		m_shaderPrograms[1].LinkProgram();
		m_shaderPrograms[1].UseProgram();
	}

	if (m_shaderPrograms[2].Initialize())
	{
		m_shaderPrograms[2].AddVertexShader("..\\Data\\Shaders\\SkyBox.vert.shader");
		m_shaderPrograms[2].AddFragmentShader("..\\Data\\Shaders\\SkyBox.frag.shader");
		m_shaderPrograms[2].LinkProgram();
		m_shaderPrograms[2].UseProgram();
	}

	if (m_shaderPrograms[3].Initialize())
	{
		m_shaderPrograms[3].AddVertexShader("..\\Data\\Shaders\\CelPhong.vert.shader");
		m_shaderPrograms[3].AddFragmentShader("..\\Data\\Shaders\\CelPhong.frag.shader");
		m_shaderPrograms[3].LinkProgram();
		m_shaderPrograms[3].UseProgram();
	}

	if (m_shaderPrograms[4].Initialize())
	{
		m_shaderPrograms[4].AddVertexShader("..\\Data\\Shaders\\DebugInstanced.vert.shader");
		m_shaderPrograms[4].AddFragmentShader("..\\Data\\Shaders\\DebugInstanced.frag.shader");
		m_shaderPrograms[4].LinkProgram();
		m_shaderPrograms[4].UseProgram();
	}


	debugColorLoc = m_shaderPrograms[0].GetUniformLocation("tint");
	tintLoc = m_shaderPrograms[3].GetUniformLocation("tint");
	tintIntensityLoc = m_shaderPrograms[1].GetUniformLocation("tintIntensity");
	modelToWorldMatLoc = m_shaderPrograms[0].GetUniformLocation("modelToWorld");
	worldToViewMatLoc = m_shaderPrograms[0].GetUniformLocation("worldToView");
	perspectiveMatLoc = m_shaderPrograms[0].GetUniformLocation("projection");
	matLoc = modelToWorldMatLoc;

	if (Engine::MyGL::TestForError(Engine::MessageType::cFatal_Error, "InitializeGL errors!"))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to InitializeGL()! TestForErrors found gl errors!\n");
		return false;
	}

	Engine::GameLogger::Log(Engine::MessageType::Process, "WorldEditor::InitializeGL() succeeded!\n");
	return true;
}

const float moveCheckOffset = 2.0f;
const float moveCheckDist = 2.0f;
bool WorldEditor::ProcessInput(float dt)
{
	char buffer[256]{ '\0' };

	if (keyboardManager.KeyWasPressed('X')) { Shutdown(); return false; }

	Engine::Vec3 movementVector(0.0f);

	if (keyboardManager.KeyIsDown('W')) { movementVector = movementVector + m_camera.GetViewDir().Normalize() * m_camera.GetSpeed() * dt; }
	if (keyboardManager.KeyIsDown('S')) { movementVector = movementVector - m_camera.GetViewDir().Normalize() * m_camera.GetSpeed() * dt; }
	if (keyboardManager.KeyIsDown('A')) { movementVector = movementVector - m_camera.GetViewDir().Cross(m_camera.GetUp()).Normalize() * m_camera.GetSpeed() * dt; }
	if (keyboardManager.KeyIsDown('D')) { movementVector = movementVector + m_camera.GetViewDir().Cross(m_camera.GetUp()).Normalize() * m_camera.GetSpeed() * dt; }

	if (m_walkEnabled)
	{
		Engine::RayCastingOutput moveRCO = Engine::CollisionTester::FindWall(m_camera.GetPosition() - (moveCheckOffset * movementVector.Normalize()), movementVector.Normalize(), moveCheckOffset + moveCheckDist + 1.0f, EDITOR_LIST_OBJS);
		while (moveRCO.m_didIntersect && moveRCO.m_distance < (moveCheckOffset + moveCheckDist))
		{
			movementVector = movementVector - movementVector.ProjectOnto(moveRCO.m_triangleNormal);

			moveRCO = Engine::CollisionTester::FindWall(m_camera.GetPosition() - (moveCheckOffset * movementVector.Normalize()), movementVector.Normalize(), moveCheckOffset + moveCheckDist + 1.0f, EDITOR_LIST_OBJS);
		}


	}

	if (movementVector.LengthSquared() > 0.0f)
	{
		m_camera.SetPosition(m_camera.GetPosition() + movementVector);
	}

	if (keyboardManager.KeyWasPressed('K') && keyboardManager.KeyIsDown(VK_SHIFT))
	{
		Engine::CollisionLayer nl = NODE_LAYER;
		if (m_objs.GetCountWhere(Engine::AStarNodeMap::IsObjInLayer, &nl) > 0)
		{
			m_nodeMap.CalculateMap(&m_objs, NODE_LAYER, CONNECTION_LAYER, EDITOR_LIST_OBJS, WorldEditor::DestroyObjsCallback, this, &m_objCount, WorldEditor::SetPCUniforms, this);
		}
	}

	if (keyboardManager.KeyWasPressed('1')) { SwapToPlace(); }
	if (keyboardManager.KeyWasPressed('2')) { SwapToRemove(); }
	if (keyboardManager.KeyWasPressed('3')) { SwapToTranslate(); }
	if (keyboardManager.KeyWasPressed('4')) { SwapToRotate(); }
	if (keyboardManager.KeyWasPressed('5')) { SwapToScale(); }
	if (keyboardManager.KeyWasPressed('7')) { SwapMakeForward(); }
	if (keyboardManager.KeyWasPressed('8')) { SwapMakeBackward(); }
	if (keyboardManager.KeyWasPressed('9') && keyboardManager.KeyIsDown(VK_SHIFT))
	{
		if (Engine::ConfigReader::pReader->GetStringForKey("WorldEditor.OutputFile", buffer))
		{
			WriteFile(&buffer[0]);
		}

		if (Engine::ConfigReader::pReader->GetStringForKey("WorldEditor.OutputNodeFile", buffer))
		{
			WriteNodeFile(&buffer[0]);
		}
	}

	if (keyboardManager.KeyWasPressed('0') && keyboardManager.KeyIsDown(VK_SHIFT))
	{
		if (Engine::ConfigReader::pReader->GetStringForKey("WorldEditor.InputFile", buffer))
		{
			ReadFile(&buffer[0]);
		}

		if (Engine::ConfigReader::pReader->GetStringForKey("WorldEditor.InputNodeFile", buffer))
		{
			ReadNodeFile(&buffer[0]);
		}
	}

	if (keyboardManager.KeyWasPressed('M')) { m_adjustmentSpeedMultiplier *= 1.1f; }
	if (keyboardManager.KeyWasPressed('N')) { m_adjustmentSpeedMultiplier *= 0.9f; }
	if (keyboardManager.KeyWasPressed(' ')) {
		m_walkEnabled = !m_walkEnabled;
		m_camera.SetSpeed(m_camera.GetSpeed() * (m_walkEnabled ? 0.25f : 4.0f));
	}
	return true;
}

void WorldEditor::ShowFrameRate(float dt)
{
	static unsigned int numFrames = 0;
	static float    timeAccumulator = 0.0f;

	++numFrames;
	timeAccumulator += dt;

	if (timeAccumulator >= m_fpsInterval)
	{
		//Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "FPS: %6.1f over %3.1f seconds.\n", numFrames / timeAccumulator, m_fpsInterval);
		char fpsText[50];
		sprintf_s(fpsText, 50, "FPS: %6.1f over %3.1f seconds.\n", numFrames / timeAccumulator, m_fpsInterval);
		//Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "FPS: %6.1f over %3.1f seconds.\n", numFrames / timeAccumulator, m_fpsInterval);
		m_fpsTextObject.SetupText(-0.9f, 0.9f, 0.0f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, fpsText);
		timeAccumulator = 0;
		numFrames = 0;
	}
}

bool WorldEditor::UglyDemoCode()
{
	if (!Engine::TextObject::Initialize(matLoc, tintLoc)) { return false; }
	if (!m_fpsTextObject.MakeBuffers()) { return false; }
	if (!m_modeText.MakeBuffers()) { return false; }
	if (!m_placingText.MakeBuffers()) { return false; }

	Engine::CollisionLayer outLayer;

	Engine::GraphicalObject *pHideout = MakeHideout(this, &outLayer);

	Engine::RenderEngine::AddGraphicalObject(pHideout);
	Engine::CollisionTester::AddGraphicalObjectToLayer(pHideout, outLayer);

	m_objs.AddToList(pHideout);
	m_objCount++; // should be one now

	Engine::ShapeGenerator::MakeGrid(&m_grid, 85, 85, Engine::Vec3(0.5f));
	m_grid.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_grid.GetFullTransformPtr(), modelToWorldMatLoc));
	m_grid.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, &wtv, worldToViewMatLoc));
	m_grid.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_perspective.GetPerspectivePtr(), perspectiveMatLoc));
	m_grid.AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &m_grid.GetMatPtr()->m_materialColor, debugColorLoc));
	m_grid.SetScaleMat(Engine::Mat4::Scale(25.0f));
	m_grid.CalcFullTransform();
	Engine::RenderEngine::AddGraphicalObject(&m_grid);

	Engine::ShapeGenerator::MakeSphere(&m_originMarker, Engine::Vec3(1.0f));
	m_originMarker.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_originMarker.GetFullTransformPtr(), modelToWorldMatLoc));
	m_originMarker.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, &wtv, worldToViewMatLoc));
	m_originMarker.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_perspective.GetPerspectivePtr(), perspectiveMatLoc));
	m_originMarker.AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &m_originMarker.GetMatPtr()->m_materialColor, debugColorLoc));
	m_originMarker.SetScaleMat(Engine::Mat4::Scale(25.0f));
	m_originMarker.CalcFullTransform();
	Engine::RenderEngine::AddGraphicalObject(&m_originMarker);

	Engine::ShapeGenerator::MakeDebugArrow(&m_xArrow, YELLOW, GREEN);

	SetPCUniforms(&m_xArrow, this);

	m_xArrow.GetMatPtr()->m_specularIntensity = 0.7f;
	m_xArrow.GetMatPtr()->m_materialColor = RED;

	m_xArrow.SetTransMat(Engine::Mat4::Translation(X_ARROW_OFFSET));
	m_xArrow.SetRotMat(Engine::Mat4::RotationToFace(BASE_ARROW_DIR, PLUS_X));
	m_xArrow.SetScaleMat(Engine::Mat4::Scale(ARROW_SCALE, BASE_ARROW_DIR));
	m_xArrow.CalcFullTransform();

	Engine::RenderEngine::AddGraphicalObject(&m_xArrow);
	Engine::CollisionTester::AddGraphicalObjectToLayer(&m_xArrow, EDITOR_ITEMS);


	Engine::ShapeGenerator::MakeDebugArrow(&m_yArrow, YELLOW, GREEN);

	SetPCUniforms(&m_yArrow, this);

	m_yArrow.GetMatPtr()->m_specularIntensity = 0.7f;
	m_yArrow.GetMatPtr()->m_materialColor = GREEN;

	m_yArrow.SetTransMat(Engine::Mat4::Translation(Y_ARROW_OFFSET));
	m_yArrow.SetRotMat(Engine::Mat4::RotationToFace(BASE_ARROW_DIR, PLUS_Y));
	m_yArrow.SetScaleMat(Engine::Mat4::Scale(ARROW_SCALE, BASE_ARROW_DIR));
	m_yArrow.CalcFullTransform();

	Engine::RenderEngine::AddGraphicalObject(&m_yArrow);
	Engine::CollisionTester::AddGraphicalObjectToLayer(&m_yArrow, EDITOR_ITEMS);


	Engine::ShapeGenerator::MakeDebugArrow(&m_zArrow, YELLOW, BLUE);

	SetPCUniforms(&m_zArrow, this);

	m_zArrow.GetMatPtr()->m_specularIntensity = 0.7f;
	m_zArrow.GetMatPtr()->m_materialColor = BLUE;

	m_zArrow.SetTransMat(Engine::Mat4::Translation(Z_ARROW_OFFSET));
	m_zArrow.SetRotMat(Engine::Mat4::RotationToFace(BASE_ARROW_DIR, PLUS_Z));
	m_zArrow.SetScaleMat(Engine::Mat4::Scale(ARROW_SCALE, BASE_ARROW_DIR));
	m_zArrow.CalcFullTransform();

	Engine::RenderEngine::AddGraphicalObject(&m_zArrow);
	Engine::CollisionTester::AddGraphicalObjectToLayer(&m_zArrow, EDITOR_ITEMS);

	m_perspective.SetPerspective(m_pWindow->width() / static_cast<float>(m_pWindow->height()), Engine::MathUtility::ToRadians(60.0f), 0.1f, RENDER_DISTANCE);
	m_perspective.SetScreenDimmensions(static_cast<float>(m_pWindow->width()), static_cast<float>(m_pWindow->height()));
	Engine::MousePicker::SetPerspectiveInfo(m_perspective.GetFOVY(), m_perspective.GetNearDist(), m_perspective.GetWidth(), m_perspective.GetHeight());

	Engine::CollisionTester::InitializeGridDebugShapes(EDITOR_ITEMS, Engine::Vec3(0.0f, 0.0f, 1.0f), wtv.GetAddress(), m_perspective.GetPerspectivePtr()->GetAddress(), tintIntensityLoc, tintLoc, modelToWorldMatLoc, worldToViewMatLoc, perspectiveMatLoc, m_shaderPrograms[4].GetProgramId());

	m_fpsTextObject.SetupText(-0.9f, 0.9f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, "FPS: 0\n");
	SetupModeText("Mode: Place\n");
	SetupPlacingText("Placing: Cube\n");

	return true;
}

void WorldEditor::SetHighlightColor(Engine::Vec3 color)
{
	highlightedColor = color;
}

void WorldEditor::SetupModeText(char * str)
{
	m_modeText.SetupText(0.3f, 0.9f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, str);
}

void WorldEditor::SetArrowEnabled(bool enabled)
{
	m_xArrow.SetEnabled(enabled);
	m_yArrow.SetEnabled(enabled);
	m_zArrow.SetEnabled(enabled);
}

void WorldEditor::MoveSelectedObjectTo(Engine::Vec3 newPos)
{
	m_pSelected->SetTransMat(Engine::Mat4::Translation(newPos));
	m_pSelected->CalcFullTransform();

}

void WorldEditor::AttachArrowsTo(Engine::GraphicalObject *pObj)
{
	SetArrowEnabled(true);

	m_xArrow.SetRotMat(pObj->GetRotMat() * Engine::Mat4::RotationToFace(BASE_ARROW_DIR, PLUS_X));
	m_xArrow.SetTransMat(Engine::Mat4::Translation(m_xArrow.GetRotMat() * X_ARROW_OFFSET + pObj->GetPos()));
	m_xArrow.CalcFullTransform();


	m_yArrow.SetRotMat(pObj->GetRotMat() * Engine::Mat4::RotationToFace(BASE_ARROW_DIR, PLUS_Y));
	m_yArrow.SetTransMat(Engine::Mat4::Translation(m_yArrow.GetRotMat() * X_ARROW_OFFSET + pObj->GetPos()));
	m_yArrow.CalcFullTransform();


	m_zArrow.SetRotMat(pObj->GetRotMat() * Engine::Mat4::RotationToFace(BASE_ARROW_DIR, PLUS_Z));
	m_zArrow.SetTransMat(Engine::Mat4::Translation(m_zArrow.GetRotMat() * X_ARROW_OFFSET + pObj->GetPos()));
	m_zArrow.CalcFullTransform();
}

void WorldEditor::SelectedObjectChanged()
{
	if (m_currentMode == WorldEditor::TranslateObject || m_currentMode == WorldEditor::RotateObject || m_currentMode == WorldEditor::ScaleObject)
	{
		if (m_pSelected)
		{
			AttachArrowsTo(m_pSelected);
		}
		else
		{
			SetArrowEnabled(false);
		}
	}
}

void WorldEditor::SetupPlacingText(char * str)
{
	m_placingText.SetupText(0.3f, 0.7f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, str);
}

void WorldEditor::InitObj(Engine::GraphicalObject * pObj, void *pClass)
{
	WorldEditor *pEditor = reinterpret_cast<WorldEditor*>(pClass);

	SetPCUniforms(pObj, pEditor);
	pObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));
	pObj->GetMatPtr()->m_specularIntensity = 0.5f;

	// add it to the necessary things, it'll get deleted on shutdown or remove
	Engine::RenderEngine::AddGraphicalObject(pObj);
	Engine::CollisionTester::AddGraphicalObjectToLayer(pObj, EDITOR_LIST_OBJS);
	pEditor->m_objCount++;
}

void WorldEditor::WriteFile(const char * const filePath)
{
	// WRITE THE OBJS TO THE WORLD FILE
	Engine::CollisionLayer toCopy = EDITOR_LIST_OBJS;
	Engine::LinkedList<Engine::GraphicalObject*> objsOnly;

	m_objs.WalkListWhere(Engine::AStarNodeMap::IsObjInLayer, &toCopy, WorldEditor::CopyObjList, &objsOnly);

	Engine::WorldFileIO::WriteGobFile(&objsOnly, filePath);
}

void WorldEditor::ReadFile(const char * const filePath)
{
	// clear the whole scene
	m_objs.WalkList(DestroyObjsCallback, this);
	if (m_objCount != 0) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to DestroyObjs! Check for memory leak or counter inaccuracy [%d] objs left!\n", m_objCount); return; }
	m_objs.ClearList();

	// load from file
	Engine::WorldFileIO::ReadGobFile(filePath, &m_objs, m_shaderPrograms[1].GetProgramId(), WorldEditor::InitObj, this);

	// re-calc grid after entire load
	Engine::CollisionTester::CalculateGrid(Engine::CollisionLayer::NUM_LAYERS);

	m_objCount = m_objs.GetCount();

	// TODO: READ THE NODES FROM THE NODE FILE
}

void WorldEditor::WriteNodeFile(const char * const filePath)
{
	m_nodeMap.ToFile(filePath);
}

void WorldEditor::ReadNodeFile(const char * const filePath)
{
	m_nodeMap.ClearGobs(&m_objs, NODE_LAYER, CONNECTION_LAYER, WorldEditor::DestroyObjsCallback, this, &m_objCount);
	m_nodeMap.ClearMap();
	Engine::AStarNodeMap::FromFile(filePath, &m_nodeMap);
	m_nodeMap.MakeArrowsForExistingConnections(&m_objs, CONNECTION_LAYER, WorldEditor::DestroyObjsCallback, this, &m_objCount, WorldEditor::SetPCUniforms, this);
	m_nodeMap.MakeObjsForExistingNodes(&m_objs, NODE_LAYER, WorldEditor::DestroyObjsCallback, this, &m_objCount, WorldEditor::SetPCUniforms, this);
	Engine::CollisionTester::CalculateGrid(Engine::CollisionLayer::NUM_LAYERS);

}

void WorldEditor::HandleOutsideGrid(Engine::GraphicalObject * pObjToCheck)
{
	if (!Engine::CollisionTester::DoesFitInGrid(pObjToCheck, EDITOR_LIST_OBJS))
	{
		DeSelect();
		DestroyObjsCallback(pObjToCheck, this);
		m_objs.RemoveFirstFromList(pObjToCheck);
		SetArrowEnabled(false);
		AttachArrowsTo(&m_grid);
	}

	Engine::CollisionTester::CalculateGrid(Engine::CollisionLayer::NUM_LAYERS); // TODO: ONLY LAYER OF OBJ
	
}

Engine::Vec3 WorldEditor::GetArrowDir(Engine::GraphicalObject * pArrow)
{
	if (pArrow == &m_xArrow) { return PLUS_X; }
	if (pArrow == &m_yArrow) { return PLUS_Y; }
	if (pArrow == &m_zArrow) { return PLUS_Z; }

	Engine::GameLogger::Log(Engine::MessageType::cWarning, "Not a valid arrow!\n");
	return Engine::Vec3();
}

Engine::GraphicalObject * WorldEditor::MakeCube(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pNewObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::MakeCube(pNewObj);

	SetPCUniforms(pNewObj, pEditor);
	pNewObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));
	pNewObj->GetMatPtr()->m_specularIntensity = 0.5f;

	*outLayer = EDITOR_LIST_OBJS;

	return pNewObj;
}

Engine::GraphicalObject * WorldEditor::MakeHideout(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pHideout = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\AIWorld.PC.scene", pHideout, pEditor->m_shaderPrograms[1].GetProgramId());

	SetPCUniforms(pHideout, pEditor);

	pHideout->GetMatPtr()->m_specularIntensity = 0.75f;
	pHideout->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));

	*outLayer = EDITOR_LIST_OBJS;

	return pHideout;
}

Engine::GraphicalObject * WorldEditor::MakeHouse(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pHouse = new Engine::GraphicalObject();
	Engine::ShapeGenerator::MakeHouse(pHouse);

	SetPCUniforms(pHouse, pEditor);

	pHouse->GetMatPtr()->m_specularIntensity = 0.75f;
	pHouse->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));

	*outLayer = EDITOR_LIST_OBJS;

	return pHouse;
}

Engine::GraphicalObject * WorldEditor::MakeBetterDargon(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pBetterDargon = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\BetterDargon.PC.scene", pBetterDargon, pEditor->m_shaderPrograms[1].GetProgramId());

	SetPCUniforms(pBetterDargon, pEditor);

	pBetterDargon->GetMatPtr()->m_specularIntensity = 0.8f;
	pBetterDargon->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));

	*outLayer = EDITOR_LIST_OBJS;

	return pBetterDargon;
}

Engine::GraphicalObject * WorldEditor::MakeChair(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\Chair.PC.scene", pObj, pEditor->m_shaderPrograms[1].GetProgramId());

	SetPCUniforms(pObj, pEditor);

	pObj->GetMatPtr()->m_specularIntensity = 0.8f;
	pObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));

	*outLayer = EDITOR_LIST_OBJS;

	return pObj;
}

Engine::GraphicalObject * WorldEditor::MakeCoil(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\Coil.PC.scene", pObj, pEditor->m_shaderPrograms[1].GetProgramId());

	SetPCUniforms(pObj, pEditor);

	pObj->GetMatPtr()->m_specularIntensity = 0.8f;
	pObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));

	*outLayer = EDITOR_LIST_OBJS;

	return pObj;
}

Engine::GraphicalObject * WorldEditor::MakeCone(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\Cone.PC.scene", pObj, pEditor->m_shaderPrograms[1].GetProgramId());

	SetPCUniforms(pObj, pEditor);

	pObj->GetMatPtr()->m_specularIntensity = 0.8f;
	pObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));

	*outLayer = EDITOR_LIST_OBJS;

	return pObj;
}

Engine::GraphicalObject * WorldEditor::MakeCup(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\Cup.PC.scene", pObj, pEditor->m_shaderPrograms[1].GetProgramId());

	SetPCUniforms(pObj, pEditor);

	pObj->GetMatPtr()->m_specularIntensity = 0.8f;
	pObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));

	*outLayer = EDITOR_LIST_OBJS;

	return pObj;
}

Engine::GraphicalObject * WorldEditor::MakeStar(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\Star.PC.scene", pObj, pEditor->m_shaderPrograms[1].GetProgramId());

	SetPCUniforms(pObj, pEditor);

	pObj->GetMatPtr()->m_specularIntensity = 0.8f;
	pObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));

	*outLayer = EDITOR_LIST_OBJS;

	return pObj;
}

Engine::GraphicalObject * WorldEditor::MakePipe(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\Pipe.PC.scene", pObj, pEditor->m_shaderPrograms[1].GetProgramId());

	SetPCUniforms(pObj, pEditor);

	pObj->GetMatPtr()->m_specularIntensity = 0.8f;
	pObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));

	*outLayer = EDITOR_LIST_OBJS;

	return pObj;
}

Engine::GraphicalObject * WorldEditor::MakeTree(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\Tree.PC.scene", pObj, pEditor->m_shaderPrograms[1].GetProgramId());

	SetPCUniforms(pObj, pEditor);

	pObj->GetMatPtr()->m_specularIntensity = 0.8f;
	pObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));

	*outLayer = EDITOR_LIST_OBJS;

	return pObj;
}

Engine::GraphicalObject * WorldEditor::MakeWedge(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\Wedge.PC.scene", pObj, pEditor->m_shaderPrograms[1].GetProgramId());

	SetPCUniforms(pObj, pEditor);

	pObj->GetMatPtr()->m_specularIntensity = 0.8f;
	pObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));

	*outLayer = EDITOR_LIST_OBJS;

	return pObj;
}

Engine::GraphicalObject * WorldEditor::MakeNodeObj(WorldEditor * pEditor, Engine::CollisionLayer *outLayer)
{
	Engine::GraphicalObject *pObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\Soccer.PC.scene", pObj, pEditor->m_shaderPrograms[1].GetProgramId());

	SetPCUniforms(pObj, pEditor);

	pObj->GetMatPtr()->m_specularIntensity = 0.8f;
	pObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));
	
	float scale = 1.0f;
	if (Engine::ConfigReader::pReader->GetFloatForKey("WorldEditor.DefaultNodeWidth", scale))
	{
		pObj->SetScaleMat(Engine::Mat4::Scale(scale));
		pObj->CalcFullTransform();
	}

	*outLayer = NODE_LAYER;

	return pObj;
}

void WorldEditor::SwapMakeForward()
{
	++m_currentPlacement %= NUM_PLACEMENT_DATA;
	SetupPlacingText(s_placementData[m_currentPlacement].m_placementStr);
}

void WorldEditor::SwapMakeBackward()
{
	m_currentPlacement = m_currentPlacement == 0 ? NUM_PLACEMENT_DATA - 1 : m_currentPlacement - 1;
	SetupPlacingText(s_placementData[m_currentPlacement].m_placementStr);
}

void WorldEditor::SwapToPlace()
{
	SetupModeText("Mode: Place\n");
	m_currentMode = WorldEditor::PlaceObject;

	DeSelect();
	DeMouseOver();
	SetArrowEnabled(false);
	SetupPlacingText(s_placementData[m_currentPlacement].m_placementStr);
}

void WorldEditor::SwapToRemove()
{
	SetupModeText("Mode: Remove\n");

	m_currentMode = WorldEditor::RemoveObject;

	SetHighlightColor(RED);
	DeSelect();
	SetArrowEnabled(false);
	SetupPlacingText("");

}

void WorldEditor::SwapToTranslate()
{
	SetupModeText("Mode: Translate\n");

	m_currentMode = WorldEditor::TranslateObject;

	SetHighlightColor(YELLOW);
	DeMouseOver();
	m_adjustmentSpeedMultiplier = 1.0f; // good for translate
	SetupPlacingText("");
}

void WorldEditor::SwapToRotate()
{
	SetupModeText("Mode: Rotate\n");

	m_currentMode = WorldEditor::RotateObject;

	SetHighlightColor(YELLOW);
	DeMouseOver();
	m_adjustmentSpeedMultiplier = 0.001f; // to be adjusted
	SetupPlacingText("");
}

void WorldEditor::SwapToScale()
{
	SetupModeText("Mode: Scale\n");

	m_currentMode = WorldEditor::ScaleObject;
	
	SetHighlightColor(YELLOW);
	DeMouseOver();
	m_adjustmentSpeedMultiplier = 0.1f; // scale is too fast normally
	SetupPlacingText("");
}

void WorldEditor::DoMouseOverHighlight()
{
	if (m_rco.m_didIntersect)
	{
		if (m_objs.Contains(m_rco.m_belongsTo))
		{
			// handle mouse move from obj to obj
			if (m_pLastHit && m_pLastHit != m_rco.m_belongsTo)
			{
				UnColor(m_pLastHit);
			}

			// highlight whatever we have hit
			Color(m_rco.m_belongsTo, &highlightedColor);

			// update last hit
			m_pLastHit = m_rco.m_belongsTo;
		}
	}
	else
	{
		if (m_pLastHit)
		{
			// Handle mousing off
			UnColor(m_pLastHit);
		}
	}
}

void WorldEditor::DoSelection()
{
	// if we hit something
	if (Engine::MouseManager::IsLeftMouseClicked())
	{
		if (m_rco.m_didIntersect)
		{
			if (m_objs.Contains(m_rco.m_belongsTo))
			{
				// handle mouse move from obj to obj
				if (m_pSelected && m_pSelected != m_rco.m_belongsTo)
				{
					UnColor(m_pSelected);
				}

				// highlight whatever we have hit
				Color(m_rco.m_belongsTo, &highlightedColor);

				// clicking on something selects it
				m_pSelected = m_rco.m_belongsTo;
			}
		}
		else
		{
			// handle de-selection
			UnColor(m_pSelected);

			// clicking on nothingness de-selects all
			m_pSelected = nullptr;
		}

		SelectedObjectChanged();
	}
}

void WorldEditor::Color(Engine::GraphicalObject * pObj, Engine::Vec3 *pColor)
{
	if (!pObj || !pColor) { return; }
	Engine::Vec3 **pData = reinterpret_cast<Engine::Vec3**>(pObj->GetUniformDataPtrPtrByLoc(tintLoc));
	*pData = pColor;
}

void WorldEditor::UnColor(Engine::GraphicalObject * pObj)
{
	if (!pObj) { return; }
	Engine::Vec3 **pData = reinterpret_cast<Engine::Vec3**>(pObj->GetUniformDataPtrPtrByLoc(tintLoc));
	*pData = &pObj->GetMatPtr()->m_materialColor;
}

void WorldEditor::DeSelect()
{
	UnColor(m_pSelected);
	m_pSelected = nullptr;
}

void WorldEditor::DeMouseOver()
{
	UnColor(m_pLastHit);
	m_pLastHit = nullptr;
}
