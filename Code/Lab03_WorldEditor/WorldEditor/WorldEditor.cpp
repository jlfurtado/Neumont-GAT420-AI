#include "WorldEditor.h"
#include "GameLogger.h"
#include <iostream>
#include "Mat4.h"
#include "RenderEngine.h"
#include "ConfigReader.h"
#include "CollisionTester.h"
#include "MyGL.h"
#include "ShapeGenerator.h"
#include "MousePicker.h"
#include "MathUtility.h"
#include "MouseManager.h"

// Justin Furtado
// 4/20/2017
// WorldEditor.h
// Enables editing of the world!

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
	if (!keyboardManager.AddKeys("XWASD"))
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

	if (!Engine::TextObject::Shutdown()) { return false; }
	if (!Engine::RenderEngine::Shutdown()) { return false; }
	//if (!Engine::ShapeGenerator::Shutdown()) { return false; }// TODO:???

	//player.Shutdown(); // TODO:???

	Engine::GameLogger::Log(Engine::MessageType::cProcess, "Game Shutdown Successfully!!!\n");
	return true;
}

void WorldEditor::Update(float dt)
{
	keyboardManager.Update(dt);
	if (!ProcessInput(dt)) { return; }
	ShowFrameRate(dt);
	wtv = m_camera.GetWorldToViewMatrix();

}

void WorldEditor::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Engine::RenderEngine::Draw();

	m_fpsTextObject.RenderText(&m_shaderPrograms[0], debugColorLoc);
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

void WorldEditor::OnMouseScroll(int degrees)
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
		m_shaderPrograms[4].AddVertexShader("..\\Data\\Shaders\\PhongPhong.vert.shader");
		m_shaderPrograms[4].AddFragmentShader("..\\Data\\Shaders\\PhongPhong.frag.shader");
		m_shaderPrograms[4].LinkProgram();
		m_shaderPrograms[4].UseProgram();
	}

	// TODO:
	debugColorLoc = m_shaderPrograms[0].GetUniformLocation("tint");
	tintLoc = m_shaderPrograms[3].GetUniformLocation("tint");
	tintIntensityLoc = m_shaderPrograms[1].GetUniformLocation("tintIntensity");
	//ambientColorLoc = m_shaderPrograms[3].GetUniformLocation("ambientLightColor");
	//ambientIntensityLoc = m_shaderPrograms[3].GetUniformLocation("ambientLightIntensity");
	//diffuseColorLoc = m_shaderPrograms[3].GetUniformLocation("diffuseLightColor");
	//diffuseIntensityLoc = m_shaderPrograms[3].GetUniformLocation("diffuseLightIntensity");
	//specularColorLoc = m_shaderPrograms[3].GetUniformLocation("specularLightColor");
	//specularIntensityLoc = m_shaderPrograms[3].GetUniformLocation("specularLightIntensity");
	//specularPowerLoc = m_shaderPrograms[3].GetUniformLocation("specularPower");
	modelToWorldMatLoc = m_shaderPrograms[0].GetUniformLocation("modelToWorld");
	worldToViewMatLoc = m_shaderPrograms[0].GetUniformLocation("worldToView");
	perspectiveMatLoc = m_shaderPrograms[0].GetUniformLocation("projection");
	//lightLoc = m_shaderPrograms[3].GetUniformLocation("lightPos_WorldSpace");
	//cameraPosLoc = m_shaderPrograms[3].GetUniformLocation("cameraPosition_WorldSpace");
	//repeatScaleLoc = m_shaderPrograms[2].GetUniformLocation("repeatScale");
	//numIterationsLoc = m_shaderPrograms[2].GetUniformLocation("numIterations");
	//shaderOffsetLoc = m_shaderPrograms[2].GetUniformLocation("randomValue");
	//texLoc = m_shaderPrograms[6].GetUniformLocation("textureSampler");
	matLoc = modelToWorldMatLoc;

	//if (Engine::MyGL::TestForError(Engine::MessageType::cFatal_Error, "InitializeGL errors!"))
	//{
	//	Engine::GameLogger::Log(Engine::MessageType::cFatal_Error, "Failed to InitializeGL()! TestForErrors found gl errors!\n");
	//	return false;
	//}

	Engine::GameLogger::Log(Engine::MessageType::Process, "WorldEditor::InitializeGL() succeeded!\n");
	return true;
}

bool WorldEditor::ProcessInput(float dt)
{
	if (keyboardManager.KeyWasPressed('X')) { Shutdown(); return false; }

	if (keyboardManager.KeyIsDown('W')) { m_camera.MoveForward(dt); }
	if (keyboardManager.KeyIsDown('S')) { m_camera.MoveBackward(dt); }
	if (keyboardManager.KeyIsDown('A')) { m_camera.StrafeLeft(dt); }
	if (keyboardManager.KeyIsDown('D')) { m_camera.StrafeRight(dt); }


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

	Engine::ShapeGenerator::ReadSceneFile("..\\Data\\Scenes\\AIWorld.PC.scene", &m_hideout, m_shaderPrograms[1].GetProgramId());
	
	m_hideout.SetScaleMat(Engine::Mat4::Scale(1.0f));
	m_hideout.CalcFullTransform();
	
	m_hideout.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_hideout.GetFullTransformPtr(), modelToWorldMatLoc));
	m_hideout.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, &wtv, worldToViewMatLoc));
	m_hideout.AddUniformData(Engine::UniformData(GL_FLOAT_MAT4, m_perspective.GetPerspectivePtr(), perspectiveMatLoc));
	m_hideout.AddUniformData(Engine::UniformData(GL_FLOAT_VEC3, &m_hideout.GetMatPtr()->m_materialColor, tintLoc));
	m_hideout.AddUniformData(Engine::UniformData(GL_FLOAT, &m_hideout.GetMatPtr()->m_specularIntensity, tintIntensityLoc));
	
	m_hideout.GetMatPtr()->m_specularIntensity = 0.75f;
	m_hideout.GetMatPtr()->m_materialColor = Engine::Vec3(1.0f, 0.0f, 0.5f);

	Engine::RenderEngine::AddGraphicalObject(&m_hideout);
	Engine::CollisionTester::AddGraphicalObjectToLayer(&m_hideout, Engine::CollisionLayer::STATIC_GEOMETRY);

	m_perspective.SetPerspective(m_pWindow->width() / static_cast<float>(m_pWindow->height()), Engine::MathUtility::ToRadians(60.0f), 1.0f, RENDER_DISTANCE);
	m_perspective.SetScreenDimmensions(static_cast<float>(m_pWindow->width()), static_cast<float>(m_pWindow->height()));
	Engine::MousePicker::SetPerspectiveInfo(m_perspective.GetFOVY(), m_perspective.GetNearDist(), m_perspective.GetWidth(), m_perspective.GetHeight());


	m_fpsTextObject.SetupText(-0.9f, 0.9f, 0.1f, 1.0f, 0.0f, 1.0f, 0.5f, 1.0f, "FPS: 0\n");
	return true;
}
