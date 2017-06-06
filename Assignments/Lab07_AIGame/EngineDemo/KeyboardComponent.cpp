#include "KeyboardComponent.h"
#include "SpatialComponent.h"
#include "ChaseCameraComponent.h"
#include "GraphicalObjectComponent.h"
#include "GraphicalObject.h"

// Justin Furtado
// 8/16/2016
// KeyboardComponent.h
// Detects key presses and calls the proper component methods

KeyboardComponent::KeyboardComponent()
{
}


KeyboardComponent::~KeyboardComponent()
{
}

bool KeyboardComponent::Initialize()
{
	if (!m_keyboardManager.AddKeys("QWEASD F"))
	{
		Engine::GameLogger::Log(Engine::MessageType::cError, "KeyboardComponent failed to initialize! Could not add keys!\n");
		return false;
	}

	Engine::GameLogger::Log(Engine::MessageType::Process, "KeyboardComponent [%s] initialized successfully!\n", GetName());
	return true;
}

bool KeyboardComponent::Update(float dt)
{
	m_keyboardManager.Update(dt);
	return HandleKeyboardInput(dt);
}

bool KeyboardComponent::HandleKeyboardInput(float dt)
{
	// Requires access to transform info to modify stuff
	Engine::SpatialComponent *pSpatialComponent = GetSiblingComponent<Engine::SpatialComponent>();
	if (!pSpatialComponent) { return true; }

	// Requires access to camera to rotate it
	Engine::ChaseCameraComponent *pCameraComponent = GetSiblingComponent<Engine::ChaseCameraComponent>();
	if (!pCameraComponent) { return true; }

	// Requires access to graphical object to rotate it
	Engine::GraphicalObjectComponent *pGraphicalObjectComponent = GetSiblingComponent<Engine::GraphicalObjectComponent>();
	if (!pGraphicalObjectComponent) { return true; }

	// get vectors
	Engine::Vec3 f = pSpatialComponent->GetForward();
	Engine::Vec3 u = pSpatialComponent->GetUp();
	Engine::Vec3 r = pSpatialComponent->GetRight();

	// radians to rotate
	float radians = pCameraComponent->GetRotateSpeed() * dt;

	Engine::Mat4 rot;

	// handle dargon input
	if (m_keyboardManager.KeyIsDown('W')) { rot = Engine::Mat4::RotationAroundAxis(r, -radians) * rot; }
	if (m_keyboardManager.KeyIsDown('S')) { rot = Engine::Mat4::RotationAroundAxis(r, radians) * rot; }
	if (m_keyboardManager.KeyIsDown('A')) { rot = Engine::Mat4::RotationAroundAxis(u, radians) * rot; }
	if (m_keyboardManager.KeyIsDown('D')) { rot = Engine::Mat4::RotationAroundAxis(u, -radians) * rot; }
	if (m_keyboardManager.KeyIsDown('Q')) { rot = Engine::Mat4::RotationAroundAxis(f, -radians) * rot; }
	if (m_keyboardManager.KeyIsDown('E')) { rot = Engine::Mat4::RotationAroundAxis(f, radians) * rot; }

	//Engine::GameLogger::Log(Engine::MessageType::ConsoleOnly, "---(%.3f, %.3f, %.3f)\n", f.GetX(), f.GetY(), f.GetZ());

	// update vectors
	pSpatialComponent->SetAxes(rot * f, rot * u);

	// calculate matrix and sqqqqet it on gob so its drawn
	Engine::Mat4 calcdRot = pSpatialComponent->CalcRotationMatrix();
	pGraphicalObjectComponent->GetGraphicalObject()->SetRotMat(calcdRot);

	// update camera
	pCameraComponent->Move(pSpatialComponent->GetPosition(), Engine::Vec3(0.0f));

	if (m_keyboardManager.KeyIsDown(' ')) { pSpatialComponent->SetVelocity(pCameraComponent->GetSpeed()*(calcdRot*(Engine::Vec3(0.0f, 0.0f, -1.0f)))); }
	else { pSpatialComponent->SetVelocity(Engine::Vec3(0.0f, 0.0f, 0.0f)); }
	return true;
}
