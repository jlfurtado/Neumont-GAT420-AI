#ifndef VEC3_H
#define VEC3_H

#include "ExportHeader.h"
#include <cmath>
#include "GameLogger.h"

// Justin Furtado
// 10/12/2016
// Vec3.h
// A vector of 3 floats

namespace Engine
{
	class ENGINE_SHARED Vec3
	{
	public:
		Vec3()
			: m_values{ 0.0f, 0.0f, 0.0f }
		{
		}

		Vec3(float v)
			: m_values{ v, v, v }
		{
		}

		Vec3(float x, float y, float z)
			: m_values{ x, y, z }
		{
		}

		Vec3(const Vec3& other)
			: m_values{ other.GetX(), other.GetY(), other.GetZ() }
		{
		}

		~Vec3()
		{
		}

		Vec3 Lerp(const Vec3 & other, float percent) const
		{
			return ((*this)*(1.0f - percent) + (other*percent));
		}

		Vec3 Normalize() const
		{
			float len = Length();
			if (len == 0.0f) {
				return *this;
			}
			else return *this / len;
		}

		float LengthSquared() const
		{
			return ((GetX() * GetX()) + (GetY() * GetY()) + (GetZ() * GetZ()));
		}

		float Length() const
		{
			return sqrtf(LengthSquared());
		}

		float Dot(const Vec3 & other) const
		{
			return ((GetX() * other.GetX()) + (GetY() * other.GetY()) + (GetZ() * other.GetZ()));
		}

		Vec3 Cross(const Vec3 & other) const
		{
			return Vec3((GetY()*other.GetZ()) - (GetZ()*other.GetY()),
				(GetZ()*other.GetX()) - (GetX()*other.GetZ()),
				(GetX()*other.GetY()) - (GetY()*other.GetX()));
		}

		Vec3 operator+(const Vec3 & other) const
		{
			return Vec3(GetX() + other.GetX(), GetY() + other.GetY(), GetZ() + other.GetZ());
		}

		Vec3 operator-(const Vec3 & other) const
		{
			return Vec3(GetX() - other.GetX(), GetY() - other.GetY(), GetZ() - other.GetZ());
		}

		Vec3 operator-() const
		{
			return Vec3(-GetX(), -GetY(), -GetZ());
		}

		Vec3 operator/(float right) const
		{
			if (right == 0.0f) { GameLogger::Log(MessageType::Error, "Cannot divide vector3 by zero!\n"); return Vec3{ 0.0f, 0.0f, 0.0f }; }
			return Vec3(GetX() / right, GetY() / right, GetZ() / right);
		}

		Vec3 operator*(float right) const
		{
			return Vec3(GetX()*right, GetY()*right, GetZ()*right);
		}

		float operator[](int i) const
		{
			if (i < 0 || i > NUM_ELEMENTS) { GameLogger::Log(MessageType::Error, "Index [%d] out of bounds for Vec3!\n", i); return 0.0f; }
			return m_values[i];
		}

		float GetX() const
		{
			return m_values[0];
		}

		float GetY() const
		{
			return m_values[1];
		}

		float GetZ() const
		{
			return m_values[2];
		}

		float GetR() const
		{
			return m_values[0];
		}

		float GetG() const
		{
			return m_values[1];
		}

		float GetB() const
		{
			return m_values[2];
		}

		float * GetAddress()
		{
			return &m_values[0];
		}

	private:
		static const int NUM_ELEMENTS = 3;
		float m_values[NUM_ELEMENTS];
	};

	inline Vec3 ENGINE_SHARED operator*(float left, const Vec3& right)
	{
		return right * left;
	}
}
#endif // ifndef VEC3_h