#include "EngineDemo.h"
#include "MousePicker.h"
#include "MyWindow.h"
#include <iostream>

#include "InstanceBuffer.h"
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

// Justin Furtado
// 6/21/2016
// EngineDemo.cpp
// The game

const float objectSpacing = 500.0f;
const int OBJECTS_PER_ROW = 2;
const int NUM_OBJECTS_DEMO1 = OBJECTS_PER_ROW * OBJECTS_PER_ROW;
const int NUM_OBJECTS_DEMO2 = OBJECTS_PER_ROW * OBJECTS_PER_ROW;
const float OBJECT_PLANE_OFFSET = 1000.0f;
const float LIGHT_HEIGHT = 15.0f;
const int NUM_USED_SHADERS = 10;
const int NUM_OBJECTS_TOTAL = NUM_OBJECTS_DEMO1;
const int NUM_DEMO_OBJECTS = NUM_OBJECTS_TOTAL + 3;
Engine::GraphicalObject m_grid;
Engine::GraphicalObject m_demoObjects[NUM_DEMO_OBJECTS];
Engine::GraphicalObject m_lights[NUM_DEMO_OBJECTS];
Engine::GraphicalObject playerGraphicalObject;
int m_texIDs[NUM_DEMO_OBJECTS]{ 0 };
const float EngineDemo::RENDER_DISTANCE = 2500.0f;
Engine::Entity player;
Engine::ChaseCameraComponent playerCamera(Engine::Vec3(0, 30, 50), Engine::Vec3(0, 5, 0), Engine::Vec3(0), false);
Engine::GraphicalObjectComponent playerGob;
Engine::SpatialComponent playerSpatial;
MouseComponent mouseComponent;
KeyboardComponent playerInput;
int numCelLevels = 4;
Engine::Vec3 backgroundColor(0.0f);

int whiteSquareId = 0;
float halfWidth = 2.0f;
int fractalGradientTextureID{ 0 };
int fractalGradientAlternateTextureID{ 0 };

Engine::Mat4 identity;
Engine::Vec3 eyeLightVal;
int eyeLightPosLoc;
Engine::Vec3 zeroVec(0.0f);

float screenToTexWidth;
float screenToTexHeight;

Engine::Mat4 persp;

SoundObject backgroundMusic;
SoundObject correctSFX;
SoundObject incorrectSFX;

Engine::Entity thing1;
Engine::GraphicalObject thing1Gob;
Engine::GraphicalObjectComponent thing1GobComp;
Engine::SpatialComponent thing1SpatialComp;
StarComp thing1Comp;
bool drawGrid = true;

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

	if (!Engine::ShapeGenerator::Initialize(m_shaderPrograms[1].GetProgramId(), m_shaderPrograms[5].GetProgramId(), m_shaderPrograms[3].GetProgramId()))
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
	if (!keyboardManager.AddKeys("XTWASDRFLGCM QE012345678`9iKNJB")
		|| !keyboardManager.AddKey(VK_OEM_4) || !keyboardManager.AddKey(VK_OEM_6) || !keyboardManager.AddKey(VK_OEM_5)
		|| !keyboardManager.AddKey(VK_PRIOR) || !keyboardManager.AddKey(VK_NEXT)
		|| !keyboardManager.AddKey(VK_OEM_PERIOD) || !keyboardManager.AddKey(VK_SHIFT))
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

void EngineDemo::Update(float dt)
{
	const float maxBorder = 0.1f;
	const float minBorder = -0.1f;

	keyboardManager.Update(dt); // needs to come before to un-pause

	if (!won) { ShowFrameRate(dt); }
	if (!ProcessInput(dt)) { return; }

	playerGraphicalObject.CalcFullTransform();
	player.Update(dt);

	thing1Gob.CalcFullTransform();
	thing1.Update(dt);

	if (paused) { return; }

	for (int i = 0; i < NUM_DEMO_OBJECTS; ++i)
	{
		m_demoObjects[i].SetRotation(m_demoObjects[i].GetRotation() + dt * m_demoObjects[i].GetRotationRate());
		m_demoObjects[i].SetRotMat(Engine::Mat4::RotationAroundAxis(m_demoObjects[i].GetRotationAxis(), m_demoObjects[i].GetRotation()));

		m_lights[i].CalcFullTransform();
		m_demoObjects[i].CalcFullTransform();
	}

	m_grid.CalcFullTransform();

	if (!backgroundMusic.GetIsPlaying())
	{
		backgroundMusic.Play();
	}

	if (Engine::MouseManager::IsLeftMouseClicked())
	{
		Engine::RayCastingOutput rco = Engine::CollisionTester::FindFromMousePos(Engine::MouseManager::GetMouseX(), Engine::MouseManager::GetMouseY(), 1000.0f);
		if (rco.m_didIntersect)
		{
			bool found = false;
			for (int i = 0; i < NUM_DEMO_OBJECTS; ++i)
			{
				if (rco.m_belongsTo == &m_demoObjects[i])
				{
					m_demoObjects[i].GetMatPtr()->m_materialColor = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));
					m_demoObjects[i].GetMatPtr()->m_ambientReflectivity = Engine::MathUtility::Rand(Engine::Vec3(0.0f), Engine::Vec3(1.0f));
					found = true;
				}
			}

		}
	}


	//Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "fractalSeed : (%.3f, %.3f)\n", fractalSeed.GetX(), fractalSeed.GetY()); // Amazingly useful for debugging fractal shader
	shaderOffset += step * dt; if (shaderOffset > maxBorder) { shaderOffset = maxBorder; step *= -1.0f; } if (shaderOffset < minBorder) { shaderOffset = minBorder; step *= -1.0f; }
	fractalSeed.GetAddress()[0] += fsx.GetX() * dt; if (fractalSeed.GetX() < fsx.GetY()) { fractalSeed.GetAddress()[0] = fsx.GetY(); fsx.GetAddress()[0] *= -1.0f; } if (fractalSeed.GetX() > fsx.GetZ()) { fractalSeed.GetAddress()[0] = fsx.GetZ(); fsx.GetAddress()[0] *= -1.0f; }
	fractalSeed.GetAddress()[1] += fsy.GetX() * dt; if (fractalSeed.GetY() < fsy.GetY()) { fractalSeed.GetAddress()[1] = fsy.GetY(); fsy.GetAddress()[0] *= -1.0f; } if (fractalSeed.GetY() > fsy.GetZ()) { fractalSeed.GetAddress()[1] = fsy.GetZ(); fsy.GetAddress()[0] *= -1.0f; }

	m_lights[0].SetTransMat(Engine::Mat4::Translation(playerGraphicalObject.GetPos() + Engine::Vec3(0.0f, 15.0f, 0.0f)));

	static int lastX = 0;
	static int lastZ = 0;

	float x = playerGraphicalObject.GetPos().GetX();
	float z = playerGraphicalObject.GetPos().GetZ();
	int cX = Engine::CollisionTester::GetGridIndexFromPosX(x);
	int cZ = Engine::CollisionTester::GetGridIndexFromPosZ(z);

	if (cX != lastX || cZ != lastZ)
	{
		char buffer[50];
		sprintf_s(buffer, 50, "[%d] triangles in [%d] [%d]\n", Engine::CollisionTester::GetTriangleCountForSpace(x, z), cX, cZ);
		m_objectText.SetupText(0.2f, 0.9f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, buffer);
	}
}

void EngineDemo::Draw()
{
	// Clear window
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	Engine::RenderEngine::Draw();
	if (drawGrid) { Engine::CollisionTester::DrawGrid(0); }

	m_fpsTextObject.RenderText(&m_shaderPrograms[0], debugColorLoc);
	m_objectText.RenderText(&m_shaderPrograms[0], debugColorLoc);
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
		m_shaderPrograms[4].AddVertexShader("..\\Data\\Shaders\\PhongPhong.vert.shader");
		m_shaderPrograms[4].AddFragmentShader("..\\Data\\Shaders\\PhongPhong.frag.shader");
		m_shaderPrograms[4].LinkProgram();
		m_shaderPrograms[4].UseProgram();
	}


	if (m_shaderPrograms[5].Initialize())
	{					 
		m_shaderPrograms[5].AddVertexShader("..\\Data\\Shaders\\DebugInstanced.Vert.shader");
		m_shaderPrograms[5].AddFragmentShader("..\\Data\\Shaders\\DebugInstanced.Frag.shader");
		m_shaderPrograms[5].LinkProgram();
		m_shaderPrograms[5].UseProgram();
	}

	// Text Shader Program

	if (m_shaderProgramText.Initialize())
	{
		m_shaderProgramText.AddVertexShader("..\\Data\\Shaders\\Text.vert.shader");
		m_shaderProgramText.AddFragmentShader("..\\Data\\Shaders\\Text.frag.shader");
		m_shaderProgramText.LinkProgram();
		m_shaderProgramText.UseProgram();
	}

	debugColorLoc = m_shaderPrograms[0].GetUniformLocation("tint");
	tintColorLoc = m_shaderPrograms[3].GetUniformLocation("tint");
	tintIntensityLoc = m_shaderPrograms[0].GetUniformLocation("tintIntensity");
	ambientColorLoc = m_shaderPrograms[3].GetUniformLocation("ambientLightColor");
	ambientIntensityLoc = m_shaderPrograms[3].GetUniformLocation("ambientLightIntensity");
	diffuseColorLoc = m_shaderPrograms[3].GetUniformLocation("diffuseLightColor");
	diffuseIntensityLoc = m_shaderPrograms[3].GetUniformLocation("diffuseLightIntensity");
	specularColorLoc = m_shaderPrograms[3].GetUniformLocation("specularLightColor");
	specularIntensityLoc = m_shaderPrograms[3].GetUniformLocation("specularLightIntensity");
	specularPowerLoc = m_shaderPrograms[3].GetUniformLocation("specularPower");
	modelToWorldMatLoc = m_shaderPrograms[3].GetUniformLocation("modelToWorld");
	worldToViewMatLoc = m_shaderPrograms[3].GetUniformLocation("worldToView");
	perspectiveMatLoc = m_shaderPrograms[3].GetUniformLocation("projection");
	lightLoc = m_shaderPrograms[3].GetUniformLocation("lightPos_WorldSpace");
	cameraPosLoc = m_shaderPrograms[3].GetUniformLocation("cameraPosition_WorldSpace");
	repeatScaleLoc = m_shaderPrograms[2].GetUniformLocation("repeatScale");
	numIterationsLoc = m_shaderPrograms[2].GetUniformLocation("numIterations");
	shaderOffsetLoc = m_shaderPrograms[2].GetUniformLocation("randomValue");

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
bool EngineDemo::ProcessInput(float dt)
{
	static int spotLightIndex = 0;
	static bool specToggle = false;

	//int multiKeyTest[]{ 'J', 'K', VK_OEM_PERIOD };
	//if (keyboardManager.KeysArePressed(&multiKeyTest[0], 3)) { Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "3 keys pressed!\n"); }
	if (keyboardManager.KeyWasPressed('T')) { Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "%f\n", dt); }
	if (keyboardManager.KeyWasReleased('C')) { Engine::CollisionTester::ConsoleLogOutput(); }
	if (keyboardManager.KeyWasPressed('`')) { Engine::ConfigReader::pReader->ProcessConfigFile(); }
	if (keyboardManager.KeyWasPressed('M')) { Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "(%.3f, %.3f, %.3f)\n", playerGraphicalObject.GetPos().GetX(), playerGraphicalObject.GetPos().GetY(), playerGraphicalObject.GetPos().GetZ()); }
	if (keyboardManager.KeyWasPressed('L')) { Engine::RenderEngine::LogStats(); }
	if (keyboardManager.KeyWasPressed('G')) { drawGrid = !drawGrid; }
	if (keyboardManager.KeyWasPressed('7'))
	{
		playerCamera.SetSpeed(Engine::MathUtility::Clamp(playerCamera.GetSpeed() * 0.75f, MIN_SPEED, MAX_SPEED));
		playerCamera.SetRotateSpeed(Engine::MathUtility::Clamp(playerCamera.GetRotateSpeed() * 0.75f, MIN_ROTATION_SPEED, MAX_ROTATION_SPEED));

	}
	if (keyboardManager.KeyWasPressed('6')) 
	{ 
		playerCamera.SetSpeed(Engine::MathUtility::Clamp(playerCamera.GetSpeed() * 1.25f, MIN_SPEED, MAX_SPEED));
		playerCamera.SetRotateSpeed(Engine::MathUtility::Clamp(playerCamera.GetRotateSpeed() * 1.25f, MIN_ROTATION_SPEED, MAX_ROTATION_SPEED));
	}

	//if (keyboardManager.KeyWasPressed('1')) { currentFractalTexID = fractalGradientTextureID; }
	//if (keyboardManager.KeyWasPressed('2')) { currentFractalTexID = fractalGradientAlternateTextureID; }
	if (keyboardManager.KeyWasPressed('X')) { Shutdown(); return false; }

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
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\BetterDargon.PN.scene", &playerGraphicalObject, m_shaderPrograms[3].GetProgramId());
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

	thing1.SetName("Thing1");
	thing1.AddComponent(&thing1SpatialComp, "thing1SpatialComp");
	thing1.AddComponent(&thing1GobComp, "thing1GobComp");
	thing1GobComp.SetGraphicalObject(&thing1Gob);
	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\Star.PN.scene", &thing1Gob, m_shaderPrograms[4].GetProgramId());
	Engine::RenderEngine::AddGraphicalObject(&thing1Gob);
	thing1Gob.AddPhongUniforms(modelToWorldMatLoc, worldToViewMatLoc, playerCamera.GetWorldToViewMatrixPtr()->GetAddress(), perspectiveMatLoc, m_perspective.GetPerspectivePtr()->GetAddress(),
		tintColorLoc, diffuseColorLoc, ambientColorLoc, specularColorLoc, specularPowerLoc, diffuseIntensityLoc, ambientIntensityLoc, specularIntensityLoc,
		&playerGraphicalObject.GetMatPtr()->m_materialColor, cameraPosLoc, playerCamera.GetPosPtr(), lightLoc, m_lights[0].GetLocPtr());
	thing1Gob.GetMatPtr()->m_specularIntensity = 32.0f;
	thing1Gob.GetMatPtr()->m_ambientReflectivity = Engine::Vec3(0.0f, 0.1f, 0.0f);
	thing1Gob.GetMatPtr()->m_diffuseReflectivity = Engine::Vec3(0.0f, 0.7f, 0.0f);
	thing1Gob.GetMatPtr()->m_specularReflectivity = Engine::Vec3(0.0f, 0.1f, 0.0f);
	thing1Gob.SetScaleMat(Engine::Mat4::Scale(10.0f));

	thing1.AddComponent(&thing1Comp, "thing1StarComp");
	thing1.Initialize();

	m_fpsTextObject.SetupText(-0.9f, 0.9f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, "FPS: 0\n");
	m_objectText.SetupText(0.2f, 0.9f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, "TRIANGLEs: 0\n");
	m_perspective.SetPerspective(m_pWindow->width() / static_cast<float>(m_pWindow->height()), Engine::MathUtility::ToRadians(60.0f), 1.0f, RENDER_DISTANCE);
	m_perspective.SetScreenDimmensions(static_cast<float>(m_pWindow->width()), static_cast<float>(m_pWindow->height()));
	Engine::MousePicker::SetPerspectiveInfo(m_perspective.GetFOVY(), m_perspective.GetNearDist(), m_perspective.GetWidth(), m_perspective.GetHeight());

	whiteSquareId = Engine::BitmapLoader::SetupWhitePixel();

	//Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\SkySphere.PT.Scene", &m_demoObjects[NUM_OBJECTS_TOTAL + 2], m_shaderPrograms[2].GetProgramId(), "..\\Data\\Textures\\fractalGradient.bmp", false);
	fractalGradientAlternateTextureID = Engine::BitmapLoader::LoadTexture("..\\Data\\Textures\\fractalGradient.bmp");

	repeatScale = 1.0f;

	for (int i = 0; i < NUM_OBJECTS_TOTAL; ++i)
	{
		// use the shader based on the dargin group
		Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\SkySphere.PT.Scene", &m_demoObjects[i], m_shaderPrograms[2].GetProgramId(), "..\\Data\\Textures\\fractalGradientGray.bmp", false);

		m_demoObjects[i].SetTransMat(Engine::Mat4::Translation(Engine::Vec3((i%OBJECTS_PER_ROW - (OBJECTS_PER_ROW / 2 - 0.5f))*objectSpacing, 15.0f, (i / OBJECTS_PER_ROW - (OBJECTS_PER_ROW / 2 - 0.5f))*objectSpacing)));
		m_demoObjects[i].SetScaleMat(Engine::Mat4::Scale(100.0f));

		m_demoObjects[i].AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_demoObjects[i].GetFullTransformPtr(), modelToWorldMatLoc));
		m_demoObjects[i].AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, playerCamera.GetWorldToViewMatrixPtr()->GetAddress(), worldToViewMatLoc));
		m_demoObjects[i].AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_perspective.GetPerspectivePtr()->GetAddress(), perspectiveMatLoc));
		m_demoObjects[i].AddUniformData(Engine::UniformData(GL_TEXTURE0, &fractalGradientAlternateTextureID, 15));
		//m_texIDs[i] = fractalGradientTextureID;
		m_demoObjects[i].AddUniformData(Engine::UniformData(GL_FLOAT_VEC2, &fractalSeed, shaderOffsetLoc));
		m_demoObjects[i].AddUniformData(Engine::UniformData(GL_FLOAT, &repeatScale, repeatScaleLoc));
		m_demoObjects[i].AddUniformData(Engine::UniformData(GL_INT, &numIterations, numIterationsLoc));
		m_demoObjects[i].AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &m_demoObjects[i].GetMatPtr()->m_materialColor, tintColorLoc));
		m_demoObjects[i].AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &m_demoObjects[i].GetMatPtr()->m_ambientReflectivity, tintIntensityLoc));
		m_demoObjects[i].GetMatPtr()->m_materialColor = Engine::Vec3(0.0f, 0.0f, 0.0f);
		m_demoObjects[i].GetMatPtr()->m_ambientReflectivity = Engine::Vec3(1.0f, 1.0f, 1.0f);

		// random fun stuff
		Engine::RenderEngine::AddGraphicalObject(&m_demoObjects[i]);
		Engine::CollisionTester::AddGraphicalObject(&m_demoObjects[i]);

	}

	Engine::CollisionTester::InitializeGridDebugShapes(0, Engine::Vec3(1.0f, 0.0f, 0.0f), playerCamera.GetWorldToViewMatrixPtr()->GetAddress(),
		m_perspective.GetPerspectivePtr()->GetAddress(), tintIntensityLoc, tintColorLoc, modelToWorldMatLoc, worldToViewMatLoc, perspectiveMatLoc);

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