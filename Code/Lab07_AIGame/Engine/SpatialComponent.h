#ifndef SPATIALCOMPONENT_H
#define SPATIALCOMPONENT_H

// Justin Furtado
// SpatialComponent.h
// 8/15/2016
// Holds transform information for an entity

#include "Component.h"
#include "ExportHeader.h"
#include "Vec3.h"
#include "Mat4.h"

namespace Engine
{
	class GraphicalObjectComponent;
	class ENGINE_SHARED SpatialComponent : public Component
	{
	public:
		SpatialComponent();
		~SpatialComponent();

		bool Initialize() override;
		bool Update(float dt) override;

		void SetPosition(Vec3 newPosition);
		void Translate(Vec3 deltaPosition);
		void SetYaw(float newYaw);
		void Yaw(float amountToYaw);
		void SetPitch(float newPitch);
		void Pitch(float amountToPitch);
		void SetRoll(float newRoll);
		void Roll(float amountToRoll);
		Vec3 GetPosition() const;
		float GetYaw() const;
		float GetPitch() const;
		float GetRoll() const;
		Vec3 GetForward() const;
		Vec3 GetUp() const;
		Vec3 GetRight() const;
		Mat4 CalcRotationMatrix();
		Vec3 GetVelocity() const;
		void SetVelocity(Vec3 newVelocity);

	private:
		Vec3 m_position;
		float m_yaw;
		float m_pitch;
		float m_roll;
		Vec3 m_forward;
		Vec3 m_right;
		Vec3 m_up;
		Vec3 m_velocity;
		GraphicalObjectComponent *m_pGobComp{ nullptr };
	};

}

#endif // ifndef SPATIALCOMPONENT_H