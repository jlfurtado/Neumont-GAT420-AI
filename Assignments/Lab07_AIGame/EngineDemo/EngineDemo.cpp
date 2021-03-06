#include "EngineDemo.h"
#include "AStarPathFinder.h"
#include "MousePicker.h"
#include "MyWindow.h"
#include <iostream>
#include "WorldFileIO.h"

#include "InstanceBuffer.h"
#include "StringFuncs.h"
#include "MouseManager.h"
#include "GameLogger.h"
#include "MyGL.h"
#include "ChaseCameraComponent.h"
#include "CollisionTester.h"
#include "MyFiles.h"
#include "ShaderProgram.h"
#include "BitmapLoader.h"
#include "SoundEngine.h"
#include "SoundObject.h"

#pragma warning(push)
#pragma warning(disable : 4251)
#pragma warning(disable : 4127)
#include <QtGui\qmouseevent>
#pragma warning(pop)

#include "ShapeGenerator.h"
#include "AStarPathFollowComponent.h"
#include "RenderEngine.h"
#include "ConfigReader.h"
#include "MathUtility.h"
#include "GraphicalObjectComponent.h"
#include "SpatialComponent.h"
#include "KeyboardComponent.h"
#include "MouseComponent.h"
#include "UniformData.h"
#include "FrameBuffer.h"
#include <winuser.h>
#include "StarComp.h"
#include "AIDemoDargonComponent.h"

// Justin Furtado
// 6/21/2016
// EngineDemo.cpp
// The game

const float LIGHT_HEIGHT = 15.0f;
Engine::GraphicalObject m_grid;
Engine::GraphicalObject m_lights[1];
Engine::GraphicalObject playerGraphicalObject;
const float EngineDemo::RENDER_DISTANCE = 2000.0f;

Engine::Entity player;
Engine::ChaseCameraComponent playerCamera(Engine::Vec3(0, 30, 50), Engine::Vec3(0, 5, 0), Engine::Vec3(0), true, Engine::CollisionLayer::LAYER_2);
Engine::GraphicalObjectComponent playerGob;
Engine::SpatialComponent playerSpatial;
MouseComponent mouseComponent;
KeyboardComponent playerInput;
int numCelLevels = 4;
Engine::Vec3 backgroundColor(0.0f);
Engine::Vec3 zeroVec(0.0f);

SoundObject backgroundMusic;
SoundObject correctSFX;
SoundObject incorrectSFX;

bool drawGrid = true;
bool allLayersEnabled = true;
Engine::CollisionLayer currentCollisionLayer;
float speedMultiplier = 1.0f;
const Engine::CollisionLayer NODE_LAYER = Engine::CollisionLayer::LAYER_3;
const Engine::CollisionLayer CONNECTION_LAYER = Engine::CollisionLayer::LAYER_4;

const int MAX_NPCS = 250;
Engine::Entity s_NPCS[MAX_NPCS];
Engine::SpatialComponent s_NPCSpatials[MAX_NPCS];
Engine::GraphicalObjectComponent s_NPCGobsComps[MAX_NPCS];
Engine::GraphicalObject s_NPCGobs[MAX_NPCS];
Engine::AStarPathFollowComponent s_NPCFollows[MAX_NPCS];
AIDemoDargonComponent s_NPCBrains[MAX_NPCS];
Engine::Mat4 s_instanceMatrices[MAX_NPCS];
Engine::GraphicalObject s_dargonInstanceObj;
Engine::InstanceBuffer s_instanceBuffer;

int lastDargon = 0;
const float dargontTimer = 0.01f;

bool EngineDemo::Initialize(Engine::MyWindow *window)
{
	m_pWindow = window;

	if (!InitializeGL())
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Unable to initialize game! Failed to InitializeGL()!\n");
		return false;
	}

	if (!Engine::RenderEngine::Initialize(&m_shaderPrograms[0], NUM_SHADER_PROGRAMS))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Could not do anything because RenderEngine failed...\n");
		return false;
	}

	if (!Engine::ShapeGenerator::Initialize(m_shaderPrograms[0].GetProgramId(), m_shaderPrograms[1].GetProgramId(), m_shaderPrograms[2].GetProgramId()))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to initialize game because shape generator failed to initialize!\n");
		return false;
	}

	if (!UglyDemoCode()) { return false; }

	if (!ReadConfigValues())
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Unable to initialize EngineDemo, failed to read config values!\n");
		return false;
	}

	if (!Engine::ConfigReader::pReader->RegisterCallbackForConfigChanges(EngineDemo::OnConfigReload, this))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Unable to register callback for EngineDemo!\n");
		return false;
	}

	// ` for numpad 0
	if (!keyboardManager.AddKeys("XTWASDRFLGCM QEOZ012345678`9iKNJBU")
		|| !keyboardManager.AddKey(VK_OEM_4) || !keyboardManager.AddKey(VK_OEM_6) || !keyboardManager.AddKey(VK_OEM_5)
		|| !keyboardManager.AddKey(VK_PRIOR) || !keyboardManager.AddKey(VK_NEXT)
		|| !keyboardManager.AddKey(VK_OEM_PERIOD) || !keyboardManager.AddKey(VK_SHIFT)
		|| !keyboardManager.AddKey(VK_UP) || !keyboardManager.AddKey(VK_DOWN))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to add keys!\n");
		return false;
	}

	if (!keyboardManager.AddToggle('P', &paused, true))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to add key toggle!\n");
		return false;
	}

	if (!SoundEngine::Initialize())
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to initialize SoundEngine");
		return false;
	}
	backgroundMusic.Initialize("..\\Data\\Sounds\\ElectricalAmbiance.wav");
	correctSFX.Initialize("..\\Data\\Sounds\\correct.wav");
	incorrectSFX.Initialize("..\\Data\\Sounds\\incorrect.wav");

	Engine::CollisionTester::CalculateGrid();

	Engine::GameLogger::Log(Engine::MessageType::cProcess, "Game Initialized Successfully!!!\n");
	return true;
}

bool EngineDemo::Shutdown()
{
	// Display some info on shutdown
	Engine::RenderEngine::LogStats();

	if (!m_pWindow->Shutdown()) { return false; }

	for (int i = 0; i < NUM_SHADER_PROGRAMS; ++i)
	{
		if (!m_shaderPrograms[i].Shutdown()) { return false; }
	}

	// have to clear objs before shape gen, cuz shape gen deletes mesh before render engine can remove it
	m_fromWorldEditorOBJs.WalkList(DestroyObjsCallback, this);
	if (m_objCount != 0) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to DestroyObjs! Check for memory leak or counter inaccuracy [%d] objs left!\n", m_objCount); return false; }

	if (!Engine::TextObject::Shutdown()) { return false; }
	if (!Engine::RenderEngine::Shutdown()) { return false; }
	if (!Engine::ShapeGenerator::Shutdown()) { return false; }
	
	player.Shutdown();

	Engine::GameLogger::Log(Engine::MessageType::cProcess, "Game Shutdown Successfully!!!\n");
	return true;
}

bool EngineDemo::InitializeCallback(void * game, Engine::MyWindow * window)
{
	if (!game) { return false; }
	return reinterpret_cast<EngineDemo *>(game)->Initialize(window);
}

void EngineDemo::UpdateCallback(void * game, float dt)
{
	if (!game) { return; }
	reinterpret_cast<EngineDemo *>(game)->Update(dt);
}

void EngineDemo::ResizeCallback(void * game)
{
	if (!game) { return; }
	reinterpret_cast<EngineDemo *>(game)->OnResizeWindow();
}

void EngineDemo::DrawCallback(void * game)
{
	if (!game) { return; }
	reinterpret_cast<EngineDemo *>(game)->Draw();
}

void EngineDemo::MouseScrollCallback(void * game, int degrees)
{
	if (!game) { return; }
	reinterpret_cast<EngineDemo *>(game)->OnMouseScroll(degrees);
}

void EngineDemo::MouseMoveCallback(void * game, int dx, int dy)
{
	if (!game) { return; }
	reinterpret_cast<EngineDemo *>(game)->OnMouseMove(dx, dy);
}

float tmr = 0.0f;
bool spawnNow = true;
void EngineDemo::Update(float dt)
{
	const float maxBorder = 0.1f;
	const float minBorder = -0.1f;

	keyboardManager.Update(dt); // needs to come before to un-pause

	if (!won) { ShowFrameRate(dt); }
	if (!ProcessInput(dt)) { return; }

	playerGraphicalObject.CalcFullTransform();
	player.Update(dt);

	static int lastX = 0;
	static int lastZ = 0;
	static int lastY = 0;
	static Engine::CollisionLayer lastCollisionLayer;
	static bool objectMoved = false;

	float x = playerGraphicalObject.GetPos().GetX();
	float y = playerGraphicalObject.GetPos().GetY();
	float z = playerGraphicalObject.GetPos().GetZ();
	int cX = Engine::CollisionTester::GetGridIndexFromPosX(x, Engine::CollisionLayer::STATIC_GEOMETRY);
	int cY = Engine::CollisionTester::GetGridIndexFromPosX(y, Engine::CollisionLayer::STATIC_GEOMETRY);
	int cZ = Engine::CollisionTester::GetGridIndexFromPosZ(z, Engine::CollisionLayer::STATIC_GEOMETRY);

	if (cX != lastX || cZ != lastZ || cY != lastY || lastCollisionLayer != currentCollisionLayer || objectMoved)
	{
		char buffer[75], buffer2[75];
		sprintf_s(buffer2, 75, "Layer [%s]:\n", Engine::CollisionTester::LayerString(currentCollisionLayer));
		if (Engine::CollisionTester::GetTriangleCountForSpace(x, y, z) < 0) { sprintf_s(buffer, 50, "Outside Spatial Grid!\n"); }
		else { sprintf_s(buffer, 75, "[%d] triangles in [%d] [%d] [%d]\n", Engine::CollisionTester::GetTriangleCountForSpace(x, y, z, currentCollisionLayer), cX, cY, cZ); }
		m_objectText.SetupText(0.20f, 0.75f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, buffer);
		m_layerText.SetupText(0.20f, 0.9f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, buffer2);
		lastX = cX;
		lastY = cY;
		lastZ = cZ;
		objectMoved = false;
	}

	if (paused) { return; }

	tmr += dt;
	if (tmr > dargontTimer || spawnNow)
	{
		spawnNow = false;
		tmr -= dargontTimer;

		if (lastDargon < MAX_NPCS)
		{
			InitDargon(lastDargon++);
		}
	}

	if (!backgroundMusic.GetIsPlaying())
	{
		backgroundMusic.Play();
	}

	m_lights[0].SetTransMat(Engine::Mat4::Translation(playerGraphicalObject.GetPos() + Engine::Vec3(0.0f, 15.0f, 0.0f)));

	lastCollisionLayer = currentCollisionLayer;

	for (int i = 0; i < lastDargon; ++i)
	{
		s_NPCS[i].Update(dt);
		s_NPCGobs[i].CalcFullTransform();
		s_instanceMatrices[i] = *s_NPCGobs[i].GetFullTransformPtr();
	}

	s_instanceBuffer.UpdateData(&s_instanceMatrices[0], 0, 16*sizeof(float)*lastDargon, lastDargon);

}

void EngineDemo::Draw()
{
	// Clear window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	s_dargonInstanceObj.SetEnabled(false);
	Engine::RenderEngine::Draw();
	s_dargonInstanceObj.SetEnabled(true);

	Engine::RenderEngine::DrawInstanced(&s_dargonInstanceObj, &s_instanceBuffer);
	if (drawGrid) { Engine::CollisionTester::DrawGrid(Engine::CollisionLayer::STATIC_GEOMETRY, playerGraphicalObject.GetPos()); }

	m_fpsTextObject.RenderText(&m_shaderPrograms[1], debugColorLoc);
	m_objectText.RenderText(&m_shaderPrograms[1], debugColorLoc);
	m_layerText.RenderText(&m_shaderPrograms[1], debugColorLoc);

}

void EngineDemo::OnResizeWindow()
{
	if (m_pWindow == nullptr) { return; }
	float aspect = static_cast<float>(m_pWindow->width()) / m_pWindow->height();
	m_perspective.SetAspectRatio(aspect);
	m_perspective.SetScreenDimmensions(static_cast<float>(m_pWindow->width()), static_cast<float>(m_pWindow->height()));
	Engine::MousePicker::SetPerspectiveInfo(m_perspective.GetFOVY(), m_perspective.GetNearDist(), m_perspective.GetWidth(), m_perspective.GetHeight());
	glViewport(0, 0, m_pWindow->width(), m_pWindow->height());

}

void EngineDemo::OnMouseScroll(int degrees)
{
	mouseComponent.MouseScroll(degrees);
}

void EngineDemo::OnMouseMove(int deltaX, int deltaY)
{
	if (Engine::MouseManager::IsRightMouseDown()) { mouseComponent.MouseMove(deltaX, deltaY); }
	mouseComponent.SetMousePos(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY());
}

void EngineDemo::OnConfigReload(void * classInstance)
{
	// error checking
	if (!classInstance) { return; }

	// get pointer to instance
	EngineDemo *pGame = reinterpret_cast<EngineDemo*>(classInstance);

	if (Engine::ConfigReader::pReader->GetFloatForKey("EngineDemo.FpsInterval", pGame->m_fpsInterval))
	{
		if (pGame->m_fpsInterval < 0.5f) { pGame->m_fpsInterval = 0.5f; }
	}

	float value;
	Engine::Vec2 inV2;
	int inInt;
	Engine::Vec3 color;
	Engine::ConfigReader::pReader->GetFloatsForKey("EngineDemo.Spotlight.AttenuationExponent", 4, pGame->spotlightAttenuations.GetAddress());
	if (Engine::ConfigReader::pReader->GetFloatsForKey("EngineDemo.ShaderTest.FractalSeed", 2, inV2.GetAddress())) { pGame->fractalSeed = inV2; }
	if (Engine::ConfigReader::pReader->GetFloatsForKey("EngineDemo.ShaderTest.FSX", 3, color.GetAddress())) { pGame->fsx = color; }
	if (Engine::ConfigReader::pReader->GetFloatsForKey("EngineDemo.ShaderTest.FSY", 3, color.GetAddress())) { pGame->fsy = color; }
	if (Engine::ConfigReader::pReader->GetFloatForKey("EngineDemo.ShaderTest.RepeatScale", value)) { pGame->repeatScale = value; }
	if (Engine::ConfigReader::pReader->GetIntForKey("EngineDemo.ShaderTest.NumIterations", inInt)) { pGame->numIterations = inInt; }
}

bool EngineDemo::InitializeGL()
{
	glViewport(0, 0, m_pWindow->width(), m_pWindow->height());

	glClearColor(backgroundColor.GetX(), backgroundColor.GetY(), backgroundColor.GetZ(), 1.0f);

	glEnable(GL_DEPTH_TEST);

	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);


	glClearStencil(0);

	if (m_shaderPrograms[0].Initialize())
	{
		m_shaderPrograms[0].AddVertexShader("..\\Data\\Shaders\\PC.Vert.shader");
		m_shaderPrograms[0].AddFragmentShader("..\\Data\\Shaders\\PC.Frag.shader");
		m_shaderPrograms[0].LinkProgram();
		m_shaderPrograms[0].UseProgram();
	}

	if (m_shaderPrograms[1].Initialize())
	{
		m_shaderPrograms[1].AddVertexShader("..\\Data\\Shaders\\Debug.vert.shader");
		m_shaderPrograms[1].AddFragmentShader("..\\Data\\Shaders\\Debug.frag.shader");
		m_shaderPrograms[1].LinkProgram();
		m_shaderPrograms[1].UseProgram();
	}

	if (m_shaderPrograms[2].Initialize())
	{					 
		m_shaderPrograms[2].AddVertexShader("..\\Data\\Shaders\\CelPhong.vert.shader");
		m_shaderPrograms[2].AddFragmentShader("..\\Data\\Shaders\\CelPhong.frag.shader");
		m_shaderPrograms[2].LinkProgram();
		m_shaderPrograms[2].UseProgram();
	}

	if (m_shaderPrograms[3].Initialize())
	{					 
		m_shaderPrograms[3].AddVertexShader("..\\Data\\Shaders\\DebugInstanced.Vert.shader");
		m_shaderPrograms[3].AddFragmentShader("..\\Data\\Shaders\\DebugInstanced.Frag.shader");
		m_shaderPrograms[3].LinkProgram();
		m_shaderPrograms[3].UseProgram();
	}

	if (m_shaderPrograms[4].Initialize())
	{
		m_shaderPrograms[4].AddVertexShader("..\\Data\\Shaders\\CelPhongInstanced.vert.shader");
		m_shaderPrograms[4].AddFragmentShader("..\\Data\\Shaders\\CelPhongInstanced.frag.shader");
		m_shaderPrograms[4].LinkProgram();
		m_shaderPrograms[4].UseProgram();
	}

	debugColorLoc = m_shaderPrograms[1].GetUniformLocation("tint");
	tintColorLoc = m_shaderPrograms[2].GetUniformLocation("tint");
	tintIntensityLoc = m_shaderPrograms[1].GetUniformLocation("tintIntensity");
	ambientColorLoc = m_shaderPrograms[2].GetUniformLocation("ambientLightColor");
	ambientIntensityLoc = m_shaderPrograms[2].GetUniformLocation("ambientLightIntensity");
	diffuseColorLoc = m_shaderPrograms[2].GetUniformLocation("diffuseLightColor");
	diffuseIntensityLoc = m_shaderPrograms[2].GetUniformLocation("diffuseLightIntensity");
	specularColorLoc = m_shaderPrograms[2].GetUniformLocation("specularLightColor");
	specularIntensityLoc = m_shaderPrograms[2].GetUniformLocation("specularLightIntensity");
	specularPowerLoc = m_shaderPrograms[2].GetUniformLocation("specularPower");
	modelToWorldMatLoc = m_shaderPrograms[2].GetUniformLocation("modelToWorld");
	worldToViewMatLoc = m_shaderPrograms[2].GetUniformLocation("worldToView");
	perspectiveMatLoc = m_shaderPrograms[2].GetUniformLocation("projection");
	lightLoc = m_shaderPrograms[2].GetUniformLocation("lightPos_WorldSpace");
	cameraPosLoc = m_shaderPrograms[2].GetUniformLocation("cameraPosition_WorldSpace");

	if (Engine::MyGL::TestForError(Engine::MessageType::cFatal_Error, "InitializeGL errors!"))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to InitializeGL()! TestForErrors found gl errors!\n");
		return false;
	}
	
	Engine::GameLogger::Log(Engine::MessageType::Process, "EngineDemo::InitializeGL() succeeded!\n");
	return true;
}

bool EngineDemo::ReadConfigValues()
{
	if (!Engine::ConfigReader::pReader->GetFloatForKey("EngineDemo.FpsInterval", m_fpsInterval)) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get float for key FpsInterval!\n"); return false; }
	if (m_fpsInterval < 0.5f) { m_fpsInterval = 0.5f; }

	float cameraSpeed = 1.0f;
	if (!Engine::ConfigReader::pReader->GetFloatForKey("EngineDemo.CameraSpeed", cameraSpeed)) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get float for key CameraSpeed!\n"); return false; }
	if (cameraSpeed < 0.0f) { cameraSpeed = 1.0f; }
	player.GetComponentByType<Engine::ChaseCameraComponent>()->SetSpeed(cameraSpeed);

	float cameraRotationSpeed = 1.0f;
	if (!Engine::ConfigReader::pReader->GetFloatForKey("EngineDemo.CameraRotationSpeed", cameraRotationSpeed)) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get float for key CameraRotationSpeed!\n"); return false; }
	if (cameraRotationSpeed < 0.0f) { cameraRotationSpeed = 1.0f; }
	player.GetComponentByType<Engine::ChaseCameraComponent>()->SetRotateSpeed(cameraRotationSpeed);

	if (!Engine::ConfigReader::pReader->GetFloatsForKey("EngineDemo.Spotlight.AttenuationExponent", 4, spotlightAttenuations.GetAddress()))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get floats for key AttenuationExponent!\n"); return false;
	}
	
	if(!Engine::ConfigReader::pReader->GetFloatsForKey("EngineDemo.ShaderTest.FractalSeed", 2, fractalSeed.GetAddress())) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get float for key FractalSeed!\n"); return false; }
	if (!Engine::ConfigReader::pReader->GetFloatsForKey("EngineDemo.ShaderTest.FSX", 3, fsx.GetAddress())) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get float for key FSX!\n"); return false; }
	if (!Engine::ConfigReader::pReader->GetFloatsForKey("EngineDemo.ShaderTest.FSY", 3, fsy.GetAddress())) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get float for key FSY!\n"); return false; }
	if (!Engine::ConfigReader::pReader->GetFloatForKey("EngineDemo.ShaderTest.RepeatScale", repeatScale)) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get float for key RepeatScale!\n"); return false; }
	if (!Engine::ConfigReader::pReader->GetIntForKey("EngineDemo.ShaderTest.NumIterations", numIterations)) { Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to get int for key NumIterations!\n"); return false; }

	Engine::GameLogger::Log(Engine::MessageType::Process, "Successfully read in config values!\n");
	return true;
}

const float MULTIPLIER = 250.0f;
const float MIN_SPEED = 250.0f / MULTIPLIER;
const float MAX_SPEED = 250.0f * MULTIPLIER;
const float MIN_ROTATION_SPEED = 0.8f / MULTIPLIER;
const float MAX_ROTATION_SPEED = 0.8f * MULTIPLIER;
bool EngineDemo::ProcessInput(float /*dt*/)
{
	static int spotLightIndex = 0;
	static bool specToggle = false;

	//int multiKeyTest[]{ 'J', 'K', VK_OEM_PERIOD };
	//if (keyboardManager.KeysArePressed(&multiKeyTest[0], 3)) { Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "3 keys pressed!\n"); }
	if (keyboardManager.KeyWasReleased('C')) { Engine::CollisionTester::ConsoleLogOutput(); }
	if (keyboardManager.KeyWasPressed('`')) { Engine::ConfigReader::pReader->ProcessConfigFile(); }
	if (keyboardManager.KeyWasPressed('I')) { Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "(%.3f, %.3f, %.3f)\n", playerGraphicalObject.GetPos().GetX(), playerGraphicalObject.GetPos().GetY(), playerGraphicalObject.GetPos().GetZ()); }
	if (keyboardManager.KeyWasPressed('L')) { Engine::RenderEngine::LogStats(); }
	if (keyboardManager.KeyWasPressed('G')) { drawGrid = !drawGrid; }
	if (keyboardManager.KeyWasPressed('N'))
	{
		playerCamera.SetSpeed(Engine::MathUtility::Clamp(playerCamera.GetSpeed() * 0.75f, MIN_SPEED, MAX_SPEED));
		playerCamera.SetRotateSpeed(Engine::MathUtility::Clamp(playerCamera.GetRotateSpeed() * 0.75f, MIN_ROTATION_SPEED, MAX_ROTATION_SPEED));

	}
	if (keyboardManager.KeyWasPressed('M')) 
	{ 
		playerCamera.SetSpeed(Engine::MathUtility::Clamp(playerCamera.GetSpeed() * 1.25f, MIN_SPEED, MAX_SPEED));
		playerCamera.SetRotateSpeed(Engine::MathUtility::Clamp(playerCamera.GetRotateSpeed() * 1.25f, MIN_ROTATION_SPEED, MAX_ROTATION_SPEED));
	}

	if (keyboardManager.KeyIsDown(VK_SHIFT))
	{
		if (keyboardManager.KeyWasPressed('0')) { currentCollisionLayer = Engine::CollisionLayer::STATIC_GEOMETRY; Engine::CollisionTester::OnlyShowLayer(currentCollisionLayer); }
		if (keyboardManager.KeyWasPressed('1')) { currentCollisionLayer = Engine::CollisionLayer::LAYER_1; Engine::CollisionTester::OnlyShowLayer(currentCollisionLayer); }
		if (keyboardManager.KeyWasPressed('2')) { currentCollisionLayer = Engine::CollisionLayer::LAYER_2; Engine::CollisionTester::OnlyShowLayer(currentCollisionLayer); }
		if (keyboardManager.KeyWasPressed('3')) { currentCollisionLayer = Engine::CollisionLayer::LAYER_3; Engine::CollisionTester::OnlyShowLayer(currentCollisionLayer); }
		if (keyboardManager.KeyWasPressed('4')) { currentCollisionLayer = Engine::CollisionLayer::LAYER_4; Engine::CollisionTester::OnlyShowLayer(currentCollisionLayer); }
		if (keyboardManager.KeyWasPressed('5')) { currentCollisionLayer = Engine::CollisionLayer::LAYER_5; Engine::CollisionTester::OnlyShowLayer(currentCollisionLayer); }
		if (keyboardManager.KeyWasPressed('6')) { currentCollisionLayer = Engine::CollisionLayer::LAYER_6; Engine::CollisionTester::OnlyShowLayer(currentCollisionLayer); }
		if (keyboardManager.KeyWasPressed('7')) { currentCollisionLayer = Engine::CollisionLayer::LAYER_7; Engine::CollisionTester::OnlyShowLayer(currentCollisionLayer); }
		if (keyboardManager.KeyWasPressed('8')) { currentCollisionLayer = Engine::CollisionLayer::LAYER_8; Engine::CollisionTester::OnlyShowLayer(currentCollisionLayer); }
		if (keyboardManager.KeyWasPressed('9')) { currentCollisionLayer = Engine::CollisionLayer::LAYER_9; Engine::CollisionTester::OnlyShowLayer(currentCollisionLayer); }

		if (keyboardManager.KeyWasPressed('U')) { currentCollisionLayer = Engine::CollisionLayer::NUM_LAYERS; Engine::CollisionTester::OnlyShowLayer(Engine::CollisionLayer::NUM_LAYERS); }
	}
	if (keyboardManager.KeyWasPressed('K')) { spawnNow = true; }


	//if (keyboardManager.KeyWasPressed('1')) { currentFractalTexID = fractalGradientTextureID; }
	//if (keyboardManager.KeyWasPressed('2')) { currentFractalTexID = fractalGradientAlternateTextureID; }
	if (keyboardManager.KeyWasPressed('X')) { Shutdown(); return false; }
	if (keyboardManager.KeyWasPressed('O')) { speedMultiplier -= 0.1f; }
	if (keyboardManager.KeyWasPressed('Z')) { speedMultiplier += 0.1f; }
	return true;
}

void EngineDemo::ShowFrameRate(float dt)
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

const int numModels = 9;
const char *modelNames = "..\\Data\\Scenes\\Tree.PN.scene\0..\\Data\\Scenes\\Wedge.PN.scene\0..\\Data\\Scenes\\Soccer.PN.scene\0..\\Data\\Scenes\\Pipe.PN.scene\0..\\Data\\Scenes\\Coil.PN.scene\0..\\Data\\Scenes\\Cup.PN.scene\0..\\Data\\Scenes\\Star.PN.scene\0..\\Data\\Scenes\\Chair.PN.scene\0..\\Data\\Scenes\\Cone.PN.scene\0";
int indicesForModelNames[numModels] = { 0 };
bool EngineDemo::UglyDemoCode()
{
	InitIndicesForMeshNames(modelNames, &indicesForModelNames[0], numModels);

	player.SetName("Player");
	player.AddComponent(&playerSpatial, "PlayerSpatial");
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\BetterDargon.PN.scene", &playerGraphicalObject, m_shaderPrograms[2].GetProgramId());
	playerGraphicalObject.AddPhongUniforms(modelToWorldMatLoc, worldToViewMatLoc, playerCamera.GetWorldToViewMatrixPtr()->GetAddress(), perspectiveMatLoc, m_perspective.GetPerspectivePtr()->GetAddress(),
		tintColorLoc, diffuseColorLoc, ambientColorLoc, specularColorLoc, specularPowerLoc, diffuseIntensityLoc, ambientIntensityLoc, specularIntensityLoc,
		&playerGraphicalObject.GetMatPtr()->m_materialColor, cameraPosLoc, playerCamera.GetPosPtr(), lightLoc, m_lights[0].GetLocPtr());
	playerGraphicalObject.AddUniformData(Engine::UniformData(GL_INT, &numCelLevels, 18));

	playerGraphicalObject.GetMatPtr()->m_specularIntensity = 32.0f;
	playerGraphicalObject.GetMatPtr()->m_ambientReflectivity = Engine::Vec3(0.0f, 0.1f, 0.0f);
	playerGraphicalObject.GetMatPtr()->m_diffuseReflectivity = Engine::Vec3(0.0f, 0.7f, 0.0f);
	playerGraphicalObject.GetMatPtr()->m_specularReflectivity = Engine::Vec3(0.0f, 0.1f, 0.0f);
	playerGraphicalObject.SetScaleMat(Engine::Mat4::Scale(1.0f));
	playerGraphicalObject.SetTransMat(Engine::Mat4::Translation(Engine::Vec3(375.0f, 5.0f, 5.0f)));
	Engine::RenderEngine::AddGraphicalObject(&playerGraphicalObject);
	playerGob.SetGraphicalObject(&playerGraphicalObject);
	player.AddComponent(&playerGob, "PlayerGob");
	player.AddComponent(&playerCamera, "PlayerCamera");
	player.AddComponent(&playerInput, "PlayerInput");
	player.AddComponent(&mouseComponent, "MouseComponent");
	player.Initialize();

	player.GetComponentByType<Engine::SpatialComponent>()->SetPosition(Engine::Vec3(375.0f, 5.0f, 5.0f));
	if (!Engine::TextObject::Initialize(matLoc, tintColorLoc))
	{
		Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Unable to initialize EngineDemo, failed to initialize text renderer!\n");
		return false;
	}

	m_fpsTextObject.MakeBuffers();
	m_objectText.MakeBuffers();
	m_layerText.MakeBuffers();

	m_fpsTextObject.SetupText(-0.9f, 0.9f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, "FPS: 0\n");
	m_objectText.SetupText(0.0f, 0.75f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, "TRIANGLES: 0\n");
	m_layerText.SetupText(0.20f, 0.9f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, "LAYER NOT DISPLAYED YET");
	m_perspective.SetPerspective(m_pWindow->width() / static_cast<float>(m_pWindow->height()), Engine::MathUtility::ToRadians(60.0f), 1.0f, RENDER_DISTANCE);
	m_perspective.SetScreenDimmensions(static_cast<float>(m_pWindow->width()), static_cast<float>(m_pWindow->height()));
	Engine::MousePicker::SetPerspectiveInfo(m_perspective.GetFOVY(), m_perspective.GetNearDist(), m_perspective.GetWidth(), m_perspective.GetHeight());

	Engine::CollisionTester::InitializeGridDebugShapes(Engine::CollisionLayer::STATIC_GEOMETRY, Engine::Vec3(1.0f, 0.0f, 0.0f), playerCamera.GetWorldToViewMatrixPtr()->GetAddress(),
		m_perspective.GetPerspectivePtr()->GetAddress(), tintIntensityLoc, tintColorLoc, modelToWorldMatLoc, worldToViewMatLoc, perspectiveMatLoc, m_shaderPrograms[3].GetProgramId());

	Engine::CollisionTester::OnlyShowLayer(currentCollisionLayer);

	Engine::ShapeGenerator::MakeGrid(&m_grid, 85, 85, Engine::Vec3(0.5f));
	m_grid.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_grid.GetFullTransformPtr(), modelToWorldMatLoc));
	m_grid.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, playerCamera.GetWorldToViewMatrixPtr()->GetAddress(), worldToViewMatLoc));
	m_grid.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_perspective.GetPerspectivePtr(), perspectiveMatLoc));
	m_grid.AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &m_grid.GetMatPtr()->m_materialColor, tintColorLoc));
	m_grid.SetScaleMat(Engine::Mat4::Scale(25.0f));
	m_grid.CalcFullTransform();
	Engine::RenderEngine::AddGraphicalObject(&m_grid);

	Engine::ShapeGenerator::MakeSphere(&m_originMarker, Engine::Vec3(1.0f));
	m_originMarker.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_originMarker.GetFullTransformPtr(), modelToWorldMatLoc));
	m_originMarker.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, playerCamera.GetWorldToViewMatrixPtr()->GetAddress(), worldToViewMatLoc));
	m_originMarker.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_perspective.GetPerspectivePtr(), perspectiveMatLoc));
	m_originMarker.AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &m_originMarker.GetMatPtr()->m_materialColor, tintColorLoc));
	m_originMarker.SetScaleMat(Engine::Mat4::Scale(25.0f));
	m_originMarker.CalcFullTransform();
	Engine::RenderEngine::AddGraphicalObject(&m_originMarker);
	
	LoadWorldFileAndApplyPCUniforms();

	s_instanceBuffer.Initialize(&s_instanceMatrices[0], 16 * sizeof(float), MAX_NPCS, MAX_NPCS * 16, GL_STREAM_DRAW); //todo test dynamic draw and compare!!

	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\BetterDargon.PN.scene", &s_dargonInstanceObj, m_shaderPrograms[4].GetProgramId());
	s_dargonInstanceObj.AddPhongUniforms(modelToWorldMatLoc, worldToViewMatLoc, playerCamera.GetWorldToViewMatrixPtr()->GetAddress(), perspectiveMatLoc, m_perspective.GetPerspectivePtr()->GetAddress(),
		tintColorLoc, diffuseColorLoc, ambientColorLoc, specularColorLoc, specularPowerLoc, diffuseIntensityLoc, ambientIntensityLoc, specularIntensityLoc,
		&s_dargonInstanceObj.GetMatPtr()->m_materialColor, cameraPosLoc, playerCamera.GetPosPtr(), lightLoc, m_lights[0].GetLocPtr());


	s_dargonInstanceObj.AddUniformData(Engine::UniformData(GL_INT, &numCelLevels, 18));
	Engine::RenderEngine::AddGraphicalObject(&s_dargonInstanceObj);

	return true;
}

bool EngineDemo::InitDargon(int index)
{
	char nameBuffer[6] = "NPC";
	nameBuffer[3] = '0' + (char)(index / 10);
	nameBuffer[4] = '0' + (char)(index % 10);
	nameBuffer[5] = '\0';

	//Engine::ShapeGenerator::MakeNormalCube(&s_NPCGobs[index]);

	s_NPCGobs[index].GetMatPtr()->m_specularIntensity = 32.0f;
	s_NPCGobs[index].GetMatPtr()->m_ambientReflectivity = Engine::Vec3(0.1f, 0.0f, 0.0f);
	s_NPCGobs[index].GetMatPtr()->m_diffuseReflectivity = Engine::Vec3(0.7f, 0.0f, 0.0f);
	s_NPCGobs[index].GetMatPtr()->m_specularReflectivity = Engine::Vec3(0.1f, 0.0f, 0.0f);
	s_NPCGobs[index].SetScaleMat(Engine::Mat4::Scale(1.0f));
	Engine::Vec3 pos = Engine::Vec3(100.0f + (index % 10) * 25.0f, 50.0f * (index % 4 + 3), (index / 40 - 2) * -100.0f);
	s_NPCGobs[index].SetTransMat(Engine::Mat4::Translation(pos));
	s_NPCSpatials[index].SetPosition(pos);
	s_NPCGobsComps[index].SetGraphicalObject(&s_NPCGobs[index]);

	s_NPCFollows[index].SetNodeMapPtr(&m_nodeMap);
	s_NPCFollows[index].SetCheckLayer(Engine::CollisionLayer::LAYER_2);

	s_NPCBrains[index].SetPlayerRef(&playerSpatial);
	s_NPCBrains[index].SetPCollectibles(&m_fromWorldEditorOBJs);
	s_NPCBrains[index].SetFormationGobPtr(&s_dargonInstanceObj);

	s_NPCS[index].SetName(&nameBuffer[0]);
	s_NPCS[index].AddComponent(&s_NPCSpatials[index], "NPC Spatial");
	s_NPCS[index].AddComponent(&s_NPCGobsComps[index], "NPC Gob");
	s_NPCS[index].AddComponent(&s_NPCFollows[index], "NPC Follow");
	s_NPCS[index].AddComponent(&s_NPCBrains[index], "NPC Brain");
	s_NPCS[index].Initialize();

	//Engine::RenderEngine::AddGraphicalObject(&s_NPCGobs[index]);
	return true;
}

void EngineDemo::InitIndicesForMeshNames(const char *const meshNames, int *indices, int numMeshes)
{
	indices[0] = 0;

	int meshIndex = 1;
	for (int i = 0; meshIndex < numMeshes; ++i)
	{
		if (*(meshNames + i) == '\0') { indices[meshIndex] = i + 1; meshIndex++; }
	}
}

void EngineDemo::LoadWorldFileAndApplyPCUniforms()
{
	char buffer[256]{ '\0' };
	if (Engine::ConfigReader::pReader->GetStringForKey("EngineDemo.World.InputFileName", buffer))
	{

		// read file
		Engine::WorldFileIO::ReadGobFile(&buffer[0], &m_fromWorldEditorOBJs, m_shaderPrograms[0].GetProgramId(), InitEditorObj, this);

		Engine::CollisionTester::CalculateGrid(Engine::CollisionLayer::NUM_LAYERS);
	}

	if (Engine::ConfigReader::pReader->GetStringForKey("EngineDemo.World.InputNodeFileName", buffer))
	{
		// read file

		m_nodeMap.ClearGobs(&m_fromWorldEditorOBJs, NODE_LAYER, CONNECTION_LAYER, EngineDemo::DestroyObjsCallback, this, &m_objCount);
		m_nodeMap.ClearMap();
		Engine::AStarNodeMap::FromFile(&buffer[0], &m_nodeMap);
		m_nodeMap.MakeArrowsForExistingConnections(&m_fromWorldEditorOBJs, CONNECTION_LAYER, EngineDemo::DestroyObjsCallback, this, &m_objCount, EngineDemo::SetPCUniforms, this);
		m_nodeMap.MakeObjsForExistingNodes(&m_fromWorldEditorOBJs, NODE_LAYER, EngineDemo::DestroyObjsCallback, this, &m_objCount, EngineDemo::SetPCUniforms, this);
		Engine::CollisionTester::CalculateGrid(Engine::CollisionLayer::NUM_LAYERS);
	}
}

bool EngineDemo::DestroyObjsCallback(Engine::GraphicalObject * pObj, void * pClassInstance)
{
	EngineDemo* pDemo = reinterpret_cast<EngineDemo*>(pClassInstance);

	Engine::RenderEngine::RemoveGraphicalObject(pObj);
	Engine::CollisionTester::RemoveGraphicalObjectFromLayer(pObj, Engine::CollisionLayer::LAYER_2); 

	delete pObj;

	pDemo->m_objCount--;

	return true;
}

void EngineDemo::InitEditorObj(Engine::GraphicalObject * pObj, void * pClass)
{
	EngineDemo *pGame = reinterpret_cast<EngineDemo*>(pClass);

	SetPCUniforms(pObj, pGame);
	pObj->GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));
	pObj->GetMatPtr()->m_specularIntensity = 0.5f;

	// add it to the necessary things, it'll get deleted on shutdown or remove
	Engine::RenderEngine::AddGraphicalObject(pObj);
	Engine::CollisionTester::AddGraphicalObjectToLayer(pObj, Engine::CollisionLayer::LAYER_2);
	pGame->m_objCount++;
}

void EngineDemo::SetPCUniforms(Engine::GraphicalObject * pObj, void * pInstance)
{
	EngineDemo *pGame = reinterpret_cast<EngineDemo*>(pInstance);

	pObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pObj->GetFullTransformPtr(), pGame->modelToWorldMatLoc));
	pObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, playerCamera.GetWorldToViewMatrixPtr()->GetAddress(), pGame->worldToViewMatLoc));
	pObj->AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, pGame->m_perspective.GetPerspectivePtr(), pGame->perspectiveMatLoc));
	pObj->AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &pObj->GetMatPtr()->m_materialColor, pGame->tintColorLoc));
	pObj->AddUniformData(Engine::UniformData(GL_FLOAT, &pObj->GetMatPtr()->m_specularIntensity, pGame->tintIntensityLoc));
}
