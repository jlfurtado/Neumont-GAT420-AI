#ifndef WORLDEDITOR_H
#define WORLDEDITOR_H

// Justin Furtado
// 4/20/2017
// WorldEditor.h
// Enables editing of the world!

#include "ShaderProgram.h"
#include "TextObject.h"
#include "Keyboard.h"
#include "Perspective.h"
#include "MyWindow.h"
#include "GraphicalObject.h"
#include "Camera.h"

class WorldEditor
{
public:
	static bool InitializeCallback(void *game, Engine::MyWindow *pWindow);
	static void UpdateCallback(void *game, float dt);
	static void ResizeCallback(void *game);
	static void DrawCallback(void *game);
	static void MouseScrollCallback(void *game, int degrees);
	static void MouseMoveCallback(void *game, int dx, int dy);

private:
	// methods
	bool Initialize(Engine::MyWindow *window);
	bool Shutdown();

	void Update(float dt);
	void Draw();

	void OnResizeWindow();
	void OnMouseScroll(int degrees);
	void OnMouseMove(int deltaX, int deltaY);

	bool ReadConfigValues();
	bool InitializeGL();
	bool ProcessInput(float dt);
	void ShowFrameRate(float dt);
	bool UglyDemoCode();
	
	static const int NUM_SHADER_PROGRAMS = 5;
	Engine::ShaderProgram m_shaderPrograms[NUM_SHADER_PROGRAMS];
	Engine::MyWindow *m_pWindow;
	Engine::Keyboard keyboardManager;
	float m_fpsInterval{ 1.0f };
	Engine::TextObject m_fpsTextObject;
	Engine::Perspective m_perspective;
	int debugColorLoc;
	int matLoc;
	int tintLoc;
	int modelToWorldMatLoc;
	int worldToViewMatLoc;
	int perspectiveMatLoc;
	int tintIntensityLoc;
	Engine::GraphicalObject m_hideout;
	Engine::Camera m_camera;
	Engine::Mat4 wtv;
};

#endif // ifndef WORLDEDITOR_H