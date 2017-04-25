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
#include "LinkedList.h"
#include "CollisionTester.h"


class WorldEditor
{
public:
	static bool InitializeCallback(void *game, Engine::MyWindow *pWindow);
	static void UpdateCallback(void *game, float dt);
	static void ResizeCallback(void *game);
	static void DrawCallback(void *game);
	static void MouseScrollCallback(void *game, int degrees);
	static void MouseMoveCallback(void *game, int dx, int dy);
	static bool DestroyObjsCallback(Engine::GraphicalObject *pObj, void *pClassInstance);
	
private:
	typedef void(*ActionCallback)(WorldEditor *);

	static void PlaceObject(WorldEditor *pEditor);
	static void RemoveObject(WorldEditor *pEditor);
	static void TranslateObject(WorldEditor *pEditor);
	static void RotateObject(WorldEditor *pEditor);
	static void ScaleObject(WorldEditor *pEditor);

	void SwapToPlace();
	void SwapToRemove();
	void SwapToTranslate();
	void SwapToRotate();
	void SwapToScale();

	void DoMouseOverHighlight();
	void DoSelection();

	void Color(Engine::GraphicalObject *pObj, Engine::Vec3 *pColor);
	void UnColor(Engine::GraphicalObject *pObj);

	void DeSelect();
	void DeMouseOver();

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
	Engine::TextObject m_modeText;
	Engine::Perspective m_perspective;
	int debugColorLoc;
	int matLoc;
	int tintLoc;
	int modelToWorldMatLoc;
	int worldToViewMatLoc;
	int perspectiveMatLoc;
	int tintIntensityLoc;
	int m_objCount;
	Engine::Camera m_camera;
	Engine::Mat4 wtv;
	Engine::LinkedList<Engine::GraphicalObject> m_objs;
	bool drawGrid{ false };
	Engine::Vec3 highlightedColor{ 1.0f, 1.0f, 0.0f };
	Engine::RayCastingOutput m_rco;
	Engine::GraphicalObject *m_pLastHit;
	Engine::GraphicalObject *m_pSelected;
	ActionCallback m_currentMode{ WorldEditor::PlaceObject };
};

#endif // ifndef WORLDEDITOR_H