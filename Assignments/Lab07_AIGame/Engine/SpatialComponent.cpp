#include "SpatialComponent.h"
#include "GraphicalObjectComponent.h"
#include "GraphicalObject.h"
#include "Mat3.h"

// Justin Furtado
// SpatialComponent.cpp
// 8/15/2016
// Holds transform information for an entity

namespace Engine
{
	const Vec3 BASE_UP = Vec3(0.0f, 1.0f, 0.0f);
	const Vec3 BASE_RIGHT = Vec3(1.0f, 0.0f, 0.0f);
	const Vec3 BASE_FORWARD = Vec3(0.0f, 0.0f, -1.0f);
	SpatialComponent::SpatialComponent()
		: m_position(Vec3(0.0f, 0.0f, 0.0f)), m_yaw(0.0f), m_pitch(0.0f), m_roll(0.0f),
		m_up(BASE_UP), m_right(BASE_RIGHT), m_forward(BASE_FORWARD)
	{
	}

	SpatialComponent::~SpatialComponent()
	{
	}

	bool SpatialComponent::Initialize()
	{
		m_pGobComp = GetSiblingComponent<GraphicalObjectComponent>();
		if (!m_pGobComp) { GameLogger::Log(MessageType::cError, "Failed to initialize SpatialComponent! Entity contains no Gob component!\n"); return true; }

		GameLogger::Log(MessageType::Process, "SpatialComponent [%s] initialized successfully!\n", GetName());
		return true;
	}

	bool SpatialComponent::Update(float dt)
	{
		m_position = m_position + m_velocity * dt;

		// set pos
		m_pGobComp->GetGraphicalObject()->SetTransMat(Mat4::Translation(GetPosition()));

		return true;
	}

	void SpatialComponent::SetPosition(Vec3 newPosition)
	{
		m_position = newPosition;
	}

	void SpatialComponent::Translate(Vec3 deltaPosition)
	{
		m_position = m_position + deltaPosition;
	}

	Vec3 SpatialComponent::GetPosition() const
	{
		return m_position;
	}

	Vec3 SpatialComponent::GetForward() const
	{
		return m_forward;
	}

	Vec3 SpatialComponent::GetUp() const
	{
		return m_up;
	}

	Vec3 SpatialComponent::GetRight() const
	{
		return m_right;
	}

	Vec3 SpatialComponent::GetVelocity() const
	{
		return m_velocity;
	}

	void SpatialComponent::SetVelocity(Vec3 newVelocity)
	{
		m_velocity = newVelocity;
	}

	void SpatialComponent::SetAxes(const Vec3 & forward, const Vec3 & up)
	{
		m_forward = forward.Normalize();
		m_up = up.Normalize();
		m_right = forward.Cross(up).Normalize();
	}

	Mat4 SpatialComponent::CalcRotationMatrix()
	{
		return Engine::Mat4::AxisRotation(m_forward, m_up);
	}
}
