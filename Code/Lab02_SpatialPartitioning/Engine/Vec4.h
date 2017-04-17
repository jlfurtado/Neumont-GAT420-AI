#ifndef VEC4_H
#define VEC4_H

// Justin Furtado
// 10/12/2016
// Vec4.h
// A vector of 4 floats for games

#include "ExportHeader.h"
#include <cmath>
#include "GameLogger.h"

namespace Engine
{
	class ENGINE_SHARED Vec4
	{
	public:
		Vec4()
			: m_values{ 0.0f, 0.0f, 0.0f, 0.0f }
		{
		}

		Vec4(float v)
			: m_values{ v, v, v, v }
		{
		}

		Vec4(float x, float y, float z, float w)
			: m_values{ x, y, z, w }
		{
		}

		Vec4(const Vec4 & other)
			: m_values{ other.GetX(), other.GetY(), other.GetZ(), other.GetW() }
		{
		}

		~Vec4()
		{
		}

		Vec4 Lerp(const Vec4 & other, float percent) const
		{
			return ((*this)*(1.0f - percent) + (other*percent));
		}

		Vec4 Normalize() const
		{
			float len = Length();
			if (len == 0.0f) { GameLogger::Log(MessageType::Error, "Cannot normalize vector4 of length 0!\n"); return *this; }
			else return *this / len;
		}

		float LengthSquared() const
		{
			return ((GetX()*GetX()) + (GetY()*GetY()) + (GetZ()*GetZ()) + (GetW()*GetW()));
		}

		float Length() const
		{
			return sqrtf(LengthSquared());
		}

		float Dot(const Vec4 & other) const
		{
			return (GetX() * other.GetX()) + (GetY() * other.GetY()) + (GetZ() * other.GetZ()) + (GetW() * other.GetW());
		}

		Vec4 operator+(const Vec4 & other) const
		{
			return Vec4{ GetX() + other.GetX(), GetY() + other.GetY(), GetZ() + other.GetZ(), GetW() + other.GetW() };
		}

		Vec4 operator-(const Vec4 & other) const
		{
			return Vec4{ GetX() - other.GetX(), GetY() - other.GetY(), GetZ() - other.GetZ(), GetW() - other.GetW() };
		}

		Vec4 operator-() const
		{
			return Vec4{ -GetX(), -GetY(), -GetZ(), -GetW() };
		}

		Vec4 operator/(float right) const
		{
			if (right == 0.0f) { GameLogger::Log(MessageType::Error, "Cannot divide vector4 by zero!\n"); return Vec4{ 0.0f, 0.0f, 0.0f, 0.0f }; }
			return Vec4{ GetX() / right, GetY() / right, GetZ() / right, GetW() / right };
		}

		Vec4 operator*(float right) const
		{
			return Vec4{ GetX() * right, GetY() * right, GetX() * right, GetW() * right };
		}

		float operator[](int i) const
		{
			if (i < 0 || i > NUM_ELEMENTS) { GameLogger::Log(MessageType::Error, "Index [%d] out of bounds for Vec4!\n", i); return 0.0f; }
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

		float GetW() const
		{
			return m_values[3];
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

		float GetA() const
		{
			return m_values[3];
		}

		float * GetAddress()
		{
			return &m_values[0];
		}

	private:
		static const int NUM_ELEMENTS = 4;
		float m_values[NUM_ELEMENTS];

	};

	inline Vec4 ENGINE_SHARED operator*(float left, const Vec4& right)
	{
		return right * left;
	}
}

#endif // ifndef VEC4_h