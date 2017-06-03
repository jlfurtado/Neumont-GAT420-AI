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
		Mat4 rotate = CalcRotationMatrix();
		m_forward = rotate * BASE_FORWARD;
		m_right = rotate * BASE_RIGHT;
		m_up = rotate * BASE_UP;

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

	void SpatialComponent::SetYaw(float newYaw)
	{
		m_yaw = newYaw;
	}

	void SpatialComponent::Yaw(float amountToYaw)
	{
		m_yaw += amountToYaw;
	}

	void SpatialComponent::SetPitch(float newPitch)
	{
		m_pitch = newPitch;
	}

	void SpatialComponent::Pitch(float amountToPitch)
	{
		m_pitch += amountToPitch;
	}

	void SpatialComponent::SetRoll(float newRoll)
	{
		m_roll = newRoll;
	}

	void SpatialComponent::Roll(float amountToRoll)
	{
		m_roll += amountToRoll;
	}

	Vec3 SpatialComponent::GetPosition() const
	{
		return m_position;
	}

	float SpatialComponent::GetYaw() const
	{
		return m_yaw;
	}

	float SpatialComponent::GetPitch() const
	{
		return m_pitch;
	}

	float SpatialComponent::GetRoll() const
	{
		return m_roll;
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

	Mat4 SpatialComponent::CalcRotationMatrix()
	{
		Mat4 y = Mat4::RotationAroundAxis(BASE_UP, m_yaw);
		Mat4 p = Mat4::RotationAroundAxis(BASE_RIGHT, m_pitch);
		Mat4 r = Mat4::RotationAroundAxis(BASE_FORWARD, m_roll);

		return y * p * r;
	}

	Vec3 SpatialComponent::GetVelocity() const
	{
		return m_velocity;
	}

	void SpatialComponent::SetVelocity(Vec3 newVelocity)
	{
		m_velocity = newVelocity;
	}
}
