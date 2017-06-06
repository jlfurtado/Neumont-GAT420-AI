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
#include "AStarNode.h"
#include "LinkedList.h"
#include "CollisionTester.h"
#include "AStarNodeMap.h"

class WorldEditor
{
	typedef Engine::GraphicalObject *(*MakeObjectCallback)(WorldEditor *, Engine::CollisionLayer *outLayer);
	typedef void(*ActionCallback)(WorldEditor *);

	struct PlacementData
	{
		PlacementData(char *str, MakeObjectCallback callback) : m_placementStr(str), m_callback(callback) {}
		char *m_placementStr;
		MakeObjectCallback m_callback;
	};

public:
	static bool InitializeCallback(void *game, Engine::MyWindow *pWindow);
	static void UpdateCallback(void *game, float dt);
	static void ResizeCallback(void *game);
	static void DrawCallback(void *game);
	static void MouseScrollCallback(void *game, int degrees);
	static void MouseMoveCallback(void *game, int dx, int dy);
	
private:
	static bool DestroyObjsCallback(Engine::GraphicalObject *pObj, void *pClassInstance);
	static bool CopyObjList(Engine::GraphicalObject *pObj, void *pDoingSomethingDifferent);

	static void PlaceObject(WorldEditor *pEditor);
	static void RemoveObject(WorldEditor *pEditor);
	static void TranslateObject(WorldEditor *pEditor);
	static void RotateObject(WorldEditor *pEditor);
	static void ScaleObject(WorldEditor *pEditor);
	static void SetPCUniforms(Engine::GraphicalObject *pObj, void *pInstance);

	static Engine::GraphicalObject *MakeCube(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakeHideout(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakeHouse(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakeBetterDargon(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakeChair(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakeCoil(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakeCone(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakeCup(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakeStar(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakePipe(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakeTree(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakeWedge(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);
	static Engine::GraphicalObject *MakeNodeObj(WorldEditor *pEditor, Engine::CollisionLayer *outLayer);

	void SwapMakeForward();
	void SwapMakeBackward();
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
	void SetHighlightColor(Engine::Vec3 color);
	void SetupModeText(char *str);
	void SetArrowEnabled(bool enabled);
	void MoveSelectedObjectTo(Engine::Vec3 newPos);
	void AttachArrowsTo(Engine::GraphicalObject *pObj);
	void SelectedObjectChanged();
	void SetupPlacingText(char *str);
	static void InitObj(Engine::GraphicalObject *pObj, void *pClass);
	void WriteFile(const char *const filePath);
	void ReadFile(const char *const filePath);
	void WriteNodeFile(const char *const filePath);
	void ReadNodeFile(const char *const filePath);
	void HandleOutsideGrid(Engine::GraphicalObject *pObjToCheck);

	static const int NUM_PLACEMENT_DATA = 13;
	static const PlacementData s_placementData[NUM_PLACEMENT_DATA];
	static const int NUM_SHADER_PROGRAMS = 5;
	Engine::ShaderProgram m_shaderPrograms[NUM_SHADER_PROGRAMS];
	Engine::MyWindow *m_pWindow;
	Engine::Keyboard keyboardManager;
	float m_fpsInterval{ 1.0f };
	Engine::TextObject m_fpsTextObject;
	Engine::TextObject m_modeText;
	Engine::TextObject m_placingText;
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
	Engine::LinkedList<Engine::GraphicalObject*> m_objs;
	bool drawGrid{ false };
	Engine::Vec3 highlightedColor{ 1.0f, 1.0f, 0.0f };
	Engine::RayCastingOutput m_rco;
	Engine::GraphicalObject *m_pLastHit;
	Engine::GraphicalObject *m_pSelected;
	Engine::GraphicalObject m_xArrow;
	Engine::GraphicalObject m_yArrow;
	Engine::GraphicalObject m_zArrow;
	Engine::GraphicalObject m_grid;
	Engine::GraphicalObject m_originMarker;
	bool m_walkEnabled{ false };
	Engine::Vec3 GetArrowDir(Engine::GraphicalObject *pArrow);
	ActionCallback m_currentMode{ WorldEditor::PlaceObject };
	int m_currentPlacement{ 0 };
	float m_adjustmentSpeedMultiplier{ 1.0f };
	bool m_scaleUniformly = false;
	static const Engine::Vec3 RED;
	static const Engine::Vec3 YELLOW;
	static const Engine::Vec3 GREEN;
	static const Engine::Vec3 BLUE;
	Engine::AStarNodeMap m_nodeMap;
};

#endif // ifndef WORLDEDITOR_H