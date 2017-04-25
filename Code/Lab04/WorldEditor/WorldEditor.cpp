#include "WorldEditor.h"
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

// Justin Furtado
// 4/20/2017
// WorldEditor.h
// Enables editing of the world!

const int PLENTY = 1024 * 8;
char saveBuffer[PLENTY]{ '\0' };
int nextBufferSlot{ 0 };

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
	
	Engine::RenderEngine::RemoveGraphicalObject(pObj);
	Engine::CollisionTester::RemoveGraphicalObjectFromLayer(pObj, EDITOR_LIST_OBJS); // TODO MAKE DYNAMIC;

	delete pObj;

	pEditor->m_objCount--;
	
	return true;
}

bool WorldEditor::TransformVerts(int index, const void * /*pVertex*/, void * /*pClassInstance*/, void * pPassThroughData)
{
	PassThroughData *pPTD = reinterpret_cast<PassThroughData*>(pPassThroughData);
	Engine::Vec3 *pVertexPositionData = reinterpret_cast<Engine::Vec3*>(pPTD->pMesh->GetPointerToVertexAt(index));
	//Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "%.3f, %.3f, %.3f\n", pVertexPositionData->GetX(), pVertexPositionData->GetY(), pVertexPositionData->GetZ());
	*pVertexPositionData = pPTD->fullTransform * (*pVertexPositionData);

	return true;
}

void WorldEditor::PlaceObject(WorldEditor *pEditor)
{
	if (Engine::MouseManager::IsLeftMouseClicked() && (pEditor->m_rco.m_didIntersect || pEditor->m_objs.GetCount() == 0))
	{
		// make an obj with the callback
		Engine::GraphicalObject *pNewObj = pEditor->m_currentPlacement(pEditor);

		// if scene is empty, place at 0 0 0, else, place at where clicked
		pNewObj->SetTransMat(Engine::Mat4::Translation(pEditor->m_objs.GetCount() == 0 ? Engine::Vec3(0.0f) : pEditor->m_rco.m_intersectionPoint));
		pNewObj->CalcFullTransform();

		// add it to the necessary things, it'll get deleted on shutdown or remove
		Engine::RenderEngine::AddGraphicalObject(pNewObj);
		Engine::CollisionTester::AddGraphicalObjectToLayer(pNewObj, EDITOR_LIST_OBJS);
		Engine::CollisionTester::CalculateGrid(EDITOR_LIST_OBJS);
		
		pEditor->m_objs.AddToList(pNewObj);
		pEditor->m_objCount++;
	}
}

void WorldEditor::RemoveObject(WorldEditor *pEditor)
{
	// show which object will be acted upon
	pEditor->DoMouseOverHighlight();

	if (Engine::MouseManager::IsLeftMouseClicked() && pEditor->m_rco.m_didIntersect && pEditor->m_objs.Contains(pEditor->m_rco.m_belongsTo))
	{
		pEditor->m_pLastHit = nullptr; // prevent crash on obj remove
		DestroyObjsCallback(pEditor->m_rco.m_belongsTo, pEditor);
		pEditor->m_objs.RemoveFromList(pEditor->m_rco.m_belongsTo);
		Engine::CollisionTester::CalculateGrid(EDITOR_LIST_OBJS); // need to recalc grid so not colliding with non-existant objects	
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

	// enable selection and de-selection of objects
	pEditor->DoSelection();

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
			Engine::CollisionTester::CalculateGrid(EDITOR_LIST_OBJS);
			Engine::CollisionTester::CalculateGrid(EDITOR_ITEMS);
		}
	}
}

void WorldEditor::RotateObject(WorldEditor *pEditor)
{
	static Engine::Mat4 startRot;
	static Engine::Vec3 lastOrigin;
	static Engine::Vec3 v;
	static Engine::Vec3 d;
	static bool arrowClicked = false;

	// enable selection and de-selection of objects
	pEditor->DoSelection();

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
			Engine::CollisionTester::CalculateGrid(EDITOR_LIST_OBJS);
			Engine::CollisionTester::CalculateGrid(EDITOR_ITEMS);
		}
	}
}

void WorldEditor::ScaleObject(WorldEditor *pEditor)
{
	static Engine::Vec3 lastOrigin;
	static Engine::Vec3 v;
	static Engine::Vec3 d;
	static bool arrowClicked = false;

	// enable selection and de-selection of objects
	pEditor->DoSelection();

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
				pEditor->m_pSelected->SetScaleMat((pEditor->m_pSelected->GetScaleMat() * Engine::Mat4::Scale(1.0f, movementAmount.Normalize())) * Engine::Mat4::Scale(scaleAmount, movementAmount.Normalize()));
				pEditor->m_pSelected->CalcFullTransform();
				pEditor->AttachArrowsTo(pEditor->m_pSelected);

				v = v + lastOrigin - newOrigin + movementAmount;
				lastOrigin = newOrigin;
			}
		}


		if (Engine::MouseManager::IsLeftMouseReleased())
		{
			arrowClicked = false;
			Engine::CollisionTester::CalculateGrid(EDITOR_LIST_OBJS);
			Engine::CollisionTester::CalculateGrid(EDITOR_ITEMS);
		}
	}
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
	if (!keyboardManager.AddKeys("XWASD1234567890MN") || !keyboardManager.AddKey(VK_SHIFT)
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

void WorldEditor::Update(float dt)
{
	keyboardManager.Update(dt);
	if (!ProcessInput(dt)) { return; }
	ShowFrameRate(dt);
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

	Engine::RenderEngine::Draw();

	if (drawGrid) { Engine::CollisionTester::DrawGrid(EDITOR_ITEMS, m_camera.GetPosition()); }

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

	// TODO:
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

bool WorldEditor::ProcessInput(float dt)
{
	static char c1 = '0', c2 = '0';

	if (keyboardManager.KeyWasPressed('X')) { Shutdown(); return false; }

	if (keyboardManager.KeyIsDown('W')) { m_camera.MoveForward(dt); }
	if (keyboardManager.KeyIsDown('S')) { m_camera.MoveBackward(dt); }
	if (keyboardManager.KeyIsDown('A')) { m_camera.StrafeLeft(dt); }
	if (keyboardManager.KeyIsDown('D')) { m_camera.StrafeRight(dt); }
	if (keyboardManager.KeyWasPressed('1')) { SwapToPlace(); }
	if (keyboardManager.KeyWasPressed('2')) { SwapToRemove(); }
	if (keyboardManager.KeyWasPressed('3')) { SwapToTranslate(); }
	if (keyboardManager.KeyWasPressed('4')) { SwapToRotate(); }
	if (keyboardManager.KeyWasPressed('5')) { SwapToScale(); }
	if (keyboardManager.KeyWasPressed('6')) { SwapToMakeCube(); }
	if (keyboardManager.KeyWasPressed('7')) { SwapToMakeHideout(); }
	if (keyboardManager.KeyWasPressed('8')) { SwapToMakeHouse(); }
	if (keyboardManager.KeyWasPressed('9')) 
	{
		char buffer[256]{ '\0' };
		if (Engine::ConfigReader::pReader->GetStringForKey("WorldEditor.OutputFile", buffer))
		{
			int len = Engine::StringFuncs::StringLen(buffer);
			buffer[len] = c1;
			buffer[len+1] = c2;

			c2++; if (c2 > '9') { c2 = '0'; c1++; if (c1 > '9') { Engine::GameLogger::Log(Engine::MessageType::cError, "Your ugly hard coded string thing you probably forgot about ran out of space, ctrl+f for this log!\n"); return false; } }
			Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "[%s]\n", &buffer[0]);
			WriteFile(&buffer[0], m_pSelected);
		}
	}

	if (keyboardManager.KeyWasPressed('0'))
	{
		char buffer[256]{ '\0' };
		if (Engine::ConfigReader::pReader->GetStringForKey("WorldEditor.InputFile", buffer))
		{
			int len = Engine::StringFuncs::StringLen(buffer);
			char lc1, lc2;
			lc1 = c1;
			lc2 = c2 - 1;
			if (lc2 < '0') { lc2 = '9'; lc1--; if (lc1 < '0') { Engine::GameLogger::Log(Engine::MessageType::cError, "Your ugly hard coded string thing you probably forgot about went negative, ctrl+f for this log!\n"); return false; } }

			buffer[len] = lc1;
			buffer[len + 1] = lc2;

			int pos = Engine::StringFuncs::StringConcatIntoBuffer(&saveBuffer[0], &buffer[0], "\0", &saveBuffer[0], PLENTY);

			Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "[%s]\n", &saveBuffer[nextBufferSlot]);
			AddObj(&saveBuffer[nextBufferSlot]);
			nextBufferSlot = pos;

		}
	}

	if (keyboardManager.KeyWasPressed('M')) { m_adjustmentSpeedMultiplier *= 1.1f; }
	if (keyboardManager.KeyWasPressed('N')) { m_adjustmentSpeedMultiplier *= 0.9f; }

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

	Engine::GraphicalObject *pHideout = MakeHideout(this);

	Engine::RenderEngine::AddGraphicalObject(pHideout);
	Engine::CollisionTester::AddGraphicalObjectToLayer(pHideout, EDITOR_LIST_OBJS);

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

	m_xArrow.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_xArrow.GetFullTransformPtr(), modelToWorldMatLoc));
	m_xArrow.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, &wtv, worldToViewMatLoc));
	m_xArrow.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_perspective.GetPerspectivePtr(), perspectiveMatLoc));
	m_xArrow.AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &m_xArrow.GetMatPtr()->m_materialColor, tintLoc));
	m_xArrow.AddUniformData(Engine::UniformData(GL_FLOAT, &m_xArrow.GetMatPtr()->m_specularIntensity, tintIntensityLoc));

	m_xArrow.GetMatPtr()->m_specularIntensity = 0.7f;
	m_xArrow.GetMatPtr()->m_materialColor = RED;

	m_xArrow.SetTransMat(Engine::Mat4::Translation(X_ARROW_OFFSET));
	m_xArrow.SetRotMat(Engine::Mat4::RotationToFace(BASE_ARROW_DIR, PLUS_X));
	m_xArrow.SetScaleMat(Engine::Mat4::Scale(ARROW_SCALE, BASE_ARROW_DIR));
	m_xArrow.CalcFullTransform();

	Engine::RenderEngine::AddGraphicalObject(&m_xArrow);
	Engine::CollisionTester::AddGraphicalObjectToLayer(&m_xArrow, EDITOR_ITEMS);


	Engine::ShapeGenerator::MakeDebugArrow(&m_yArrow, YELLOW, GREEN);

	m_yArrow.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_yArrow.GetFullTransformPtr(), modelToWorldMatLoc));
	m_yArrow.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, &wtv, worldToViewMatLoc));
	m_yArrow.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_perspective.GetPerspectivePtr(), perspectiveMatLoc));
	m_yArrow.AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &m_yArrow.GetMatPtr()->m_materialColor, tintLoc));
	m_yArrow.AddUniformData(Engine::UniformData(GL_FLOAT, &m_yArrow.GetMatPtr()->m_specularIntensity, tintIntensityLoc));

	m_yArrow.GetMatPtr()->m_specularIntensity = 0.7f;
	m_yArrow.GetMatPtr()->m_materialColor = GREEN;

	m_yArrow.SetTransMat(Engine::Mat4::Translation(Y_ARROW_OFFSET));
	m_yArrow.SetRotMat(Engine::Mat4::RotationToFace(BASE_ARROW_DIR, PLUS_Y));
	m_yArrow.SetScaleMat(Engine::Mat4::Scale(ARROW_SCALE, BASE_ARROW_DIR));
	m_yArrow.CalcFullTransform();

	Engine::RenderEngine::AddGraphicalObject(&m_yArrow);
	Engine::CollisionTester::AddGraphicalObjectToLayer(&m_yArrow, EDITOR_ITEMS);


	Engine::ShapeGenerator::MakeDebugArrow(&m_zArrow, YELLOW, BLUE);

	m_zArrow.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_zArrow.GetFullTransformPtr(), modelToWorldMatLoc));
	m_zArrow.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, &wtv, worldToViewMatLoc));
	m_zArrow.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_perspective.GetPerspectivePtr(), perspectiveMatLoc));
	m_zArrow.AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &m_zArrow.GetMatPtr()->m_materialColor, tintLoc));
	m_zArrow.AddUniformData(Engine::UniformData(GL_FLOAT, &m_zArrow.GetMatPtr()->m_specularIntensity, tintIntensityLoc));

	m_zArrow.GetMatPtr()->m_specularIntensity = 0.7f;
	m_zArrow.GetMatPtr()->m_materialColor = BLUE;

	m_zArrow.SetTransMat(Engine::Mat4::Translation(Z_ARROW_OFFSET));
	m_zArrow.SetRotMat(Engine::Mat4::RotationToFace(BASE_ARROW_DIR, PLUS_Z));
	m_zArrow.SetScaleMat(Engine::Mat4::Scale(ARROW_SCALE, BASE_ARROW_DIR));
	m_zArrow.CalcFullTransform();

	Engine::RenderEngine::AddGraphicalObject(&m_zArrow);
	Engine::CollisionTester::AddGraphicalObjectToLayer(&m_zArrow, EDITOR_ITEMS);

	m_perspective.SetPerspective(m_pWindow->width() / static_cast<float>(m_pWindow->height()), Engine::MathUtility::ToRadians(60.0f), 1.0f, RENDER_DISTANCE);
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

void WorldEditor::AddObj(const char * const fp)
{
	// make an obj
	Engine::GraphicalObject *pNewObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile(fp, pNewObj, m_shaderPrograms[1].GetProgramId());

	pNewObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pNewObj->GetFullTransformPtr(), modelToWorldMatLoc));
	pNewObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, &wtv, worldToViewMatLoc));
	pNewObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_perspective.GetPerspectivePtr(), perspectiveMatLoc));
	pNewObj->AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &pNewObj->GetMatPtr()->m_materialColor, tintLoc));
	pNewObj->AddUniformData(Engine::UniformData(GL_FLOAT, &pNewObj->GetMatPtr()->m_specularIntensity, tintIntensityLoc));
	pNewObj->GetMatPtr()->m_materialColor = Engine::Vec3(0.5f, 0.25f, 1.0f);
	pNewObj->GetMatPtr()->m_specularIntensity = 0.5f;

	// add it to the necessary things, it'll get deleted on shutdown or remove
	Engine::RenderEngine::AddGraphicalObject(pNewObj);
	Engine::CollisionTester::AddGraphicalObjectToLayer(pNewObj, EDITOR_LIST_OBJS);
	Engine::CollisionTester::CalculateGrid(EDITOR_LIST_OBJS);

	m_objs.AddToList(pNewObj);
	m_objCount++;
}

void WorldEditor::WriteFile(const char * const filePath, Engine::GraphicalObject * pObj)
{
	// variable to hold length in bytes 
	int dataLen = sizeof(*pObj->GetMeshPointer()) + pObj->GetMeshPointer()->GetIndexSizeInBytes() + pObj->GetMeshPointer()->GetVertexSizeInBytes();

	// allocate space for size specified in file
	char *data = new char[dataLen];

	// zero out newly allocated memory
	memset(data, 0, dataLen);

	// Convert data read to mesh
	Engine::Mesh *pMeshCopy = reinterpret_cast<Engine::Mesh*>(data);

	// get pointer to data vertices
	// vertices stored right after mesh
	char *pVertices = data + sizeof(Engine::Mesh);
	std::memcpy(pVertices, pObj->GetMeshPointer()->GetVertexPointer(), pObj->GetMeshPointer()->GetVertexSizeInBytes());

	// get pointer to indices
	// indices stored right after vertices
	char *pIndices = pVertices + (pObj->GetMeshPointer()->GetSizeOfVertex()*pObj->GetMeshPointer()->GetVertexCount());
	std::memcpy(pIndices, pObj->GetMeshPointer()->GetIndexPointer(), pObj->GetMeshPointer()->GetIndexSizeInBytes());

	// finish meshing it up (puns are great)
	*pMeshCopy = Engine::Mesh(pObj->GetMeshPointer()->GetVertexCount(), pObj->GetMeshPointer()->GetIndexCount(), pVertices, pIndices, pObj->GetMeshPointer()->GetMeshMode(), Engine::IndexSizeInBytes::Uint, 0, pObj->GetMeshPointer()->GetVertexFormat(), pObj->GetMeshPointer()->IsCullingEnabledForObject());
	
	pObj->CalcFullTransform();

	// data for callback
	PassThroughData ptd;
	ptd.pMesh = pMeshCopy;
	ptd.fullTransform = *pObj->GetFullTransformPtr();

	// now we can mesh with our copy (again with the puns!)
	pMeshCopy->WalkVertices(WorldEditor::TransformVerts, this, &ptd, true);

	Engine::BinaryWriter writer;

	if (!writer.WriteCustomBinaryFile(filePath, pMeshCopy))
	{
		Engine::GameLogger::Log(Engine::MessageType::cError, "Failed to write out mesh copy to [%s]... let the crying begin!\n", filePath);
		delete[] data;
		return;
	}

	Engine::GameLogger::Log(Engine::MessageType::Info, "Wrote out mesh copy to [%s]... let the crying cease!\n", filePath);
}

Engine::Vec3 WorldEditor::GetArrowDir(Engine::GraphicalObject * pArrow)
{
	if (pArrow == &m_xArrow) { return PLUS_X; }
	if (pArrow == &m_yArrow) { return PLUS_Y; }
	if (pArrow == &m_zArrow) { return PLUS_Z; }

	Engine::GameLogger::Log(Engine::MessageType::cWarning, "Not a valid arrow!\n");
	return Engine::Vec3();
}

Engine::GraphicalObject * WorldEditor::MakeCube(WorldEditor * pEditor)
{
	Engine::Vec3 color = Engine::Vec3(0.0f, 1.0f, 0.0f);
	Engine::GraphicalObject *pNewObj = new Engine::GraphicalObject();
	Engine::ShapeGenerator::MakeCube(pNewObj);

	pNewObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pNewObj->GetFullTransformPtr(), pEditor->modelToWorldMatLoc));
	pNewObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, &pEditor->wtv, pEditor->worldToViewMatLoc));
	pNewObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pEditor->m_perspective.GetPerspectivePtr(), pEditor->perspectiveMatLoc));
	pNewObj->AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &pNewObj->GetMatPtr()->m_materialColor, pEditor->tintLoc));
	pNewObj->AddUniformData(Engine::UniformData(GL_FLOAT, &pNewObj->GetMatPtr()->m_specularIntensity, pEditor->tintIntensityLoc));
	pNewObj->GetMatPtr()->m_materialColor = color;
	pNewObj->GetMatPtr()->m_specularIntensity = 0.5f;

	return pNewObj;
}

Engine::GraphicalObject * WorldEditor::MakeHideout(WorldEditor * pEditor)
{
	Engine::GraphicalObject *pHideout = new Engine::GraphicalObject();
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\AIWorld.PC.scene", pHideout, pEditor->m_shaderPrograms[1].GetProgramId());

	pHideout->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pHideout->GetFullTransformPtr(), pEditor->modelToWorldMatLoc));
	pHideout->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, &pEditor->wtv, pEditor->worldToViewMatLoc));
	pHideout->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pEditor->m_perspective.GetPerspectivePtr(), pEditor->perspectiveMatLoc));
	pHideout->AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &pHideout->GetMatPtr()->m_materialColor, pEditor->tintLoc));
	pHideout->AddUniformData(Engine::UniformData(GL_FLOAT, &pHideout->GetMatPtr()->m_specularIntensity, pEditor->tintIntensityLoc));

	pHideout->GetMatPtr()->m_specularIntensity = 0.75f;
	pHideout->GetMatPtr()->m_materialColor = Engine::Vec3(1.0f, 0.0f, 0.5f);

	return pHideout;
}

Engine::GraphicalObject * WorldEditor::MakeHouse(WorldEditor * pEditor)
{
	Engine::GraphicalObject *pHouse = new Engine::GraphicalObject();
	Engine::ShapeGenerator::MakeHouse(pHouse);

	pHouse->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pHouse->GetFullTransformPtr(), pEditor->modelToWorldMatLoc));
	pHouse->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, &pEditor->wtv, pEditor->worldToViewMatLoc));
	pHouse->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pEditor->m_perspective.GetPerspectivePtr(), pEditor->perspectiveMatLoc));
	pHouse->AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &pHouse->GetMatPtr()->m_materialColor, pEditor->tintLoc));
	pHouse->AddUniformData(Engine::UniformData(GL_FLOAT, &pHouse->GetMatPtr()->m_specularIntensity, pEditor->tintIntensityLoc));

	pHouse->GetMatPtr()->m_specularIntensity = 0.75f;
	pHouse->GetMatPtr()->m_materialColor = Engine::Vec3(0.0f, 1.0f, 0.5f);

	return pHouse;
}

void WorldEditor::SwapToMakeHideout()
{
	SetupPlacingText("Placing: Hideout\n");
	m_currentPlacement = WorldEditor::MakeHideout;
}

void WorldEditor::SwapToMakeCube()
{
	SetupPlacingText("Placing: Cube\n");
	m_currentPlacement = WorldEditor::MakeCube;
}

void WorldEditor::SwapToMakeHouse()
{
	SetupPlacingText("Placing: House\n");
	m_currentPlacement = WorldEditor::MakeHouse;
}

void WorldEditor::SwapToPlace()
{
	SetupModeText("Mode: Place\n");
	m_currentMode = WorldEditor::PlaceObject;

	DeSelect();
	DeMouseOver();
	SetArrowEnabled(false);

}

void WorldEditor::SwapToRemove()
{
	SetupModeText("Mode: Remove\n");

	m_currentMode = WorldEditor::RemoveObject;

	SetHighlightColor(RED);
	DeSelect();
	SetArrowEnabled(false);

}

void WorldEditor::SwapToTranslate()
{
	SetupModeText("Mode: Translate\n");

	m_currentMode = WorldEditor::TranslateObject;

	SetHighlightColor(YELLOW);
	DeMouseOver();
	m_adjustmentSpeedMultiplier = 1.0f; // good for translate
}

void WorldEditor::SwapToRotate()
{
	SetupModeText("Mode: Rotate\n");

	m_currentMode = WorldEditor::RotateObject;

	SetHighlightColor(YELLOW);
	DeMouseOver();
	m_adjustmentSpeedMultiplier = 0.001f; // to be adjusted
}

void WorldEditor::SwapToScale()
{
	SetupModeText("Mode: Scale\n");

	m_currentMode = WorldEditor::ScaleObject;
	
	SetHighlightColor(YELLOW);
	DeMouseOver();
	m_adjustmentSpeedMultiplier = 0.1f; // scale is too fast normally
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
