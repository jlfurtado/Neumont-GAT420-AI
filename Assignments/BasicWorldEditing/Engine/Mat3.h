#ifndef MAT3_H
#define MAT3_H

// Justin Furtado
// 10/12/2016
// Mat3.h
// A Matrix3x3 class for transforming vectors

#include "ExportHeader.h"
#include "Vec3.h"
#include "Vec2.h"
#include <cmath>

namespace Engine
{
	class ENGINE_SHARED Mat3
	{
	public:
		// constructors
		Mat3()
			: m_values{ 1.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 1.0f } { }

		Mat3(const Vec2& left, const Vec2& right)
			: m_values{ left.GetX(), right.GetX(), 0.0f,
						left.GetY(), right.GetY(), 0.0f,
						0.0f, 0.0f, 1.0f } { }

		Mat3(const Vec3& left, const Vec3& middle, const Vec3& right) 
			: m_values{ left.GetX(), middle.GetX(), right.GetX(),
						left.GetY(), middle.GetY(), right.GetY(),
						left.GetZ(), middle.GetZ(), right.GetZ() } { }

		Mat3(float topLeft, float topMiddle, float topRight, float middleLeft, float middleMiddle, float middleRight, float bottomLeft, float bottomMiddle, float bottomRight)
			: m_values{ topLeft, topMiddle, topRight,
						middleLeft, middleMiddle, middleRight,
						bottomLeft, bottomMiddle, bottomRight } { }

		//destructor
		~Mat3() {}

		// public static methods that return matrices
		static Mat3 Rotation(float radians)
		{
			return Mat3(cosf(radians), -sinf(radians), 0.0f,
				sinf(radians), cosf(radians), 0.0f,
				0.0f, 0.0f, 1.0f);

		}

		static Mat3 Scale(float scale)
		{
			return Mat3(scale, 0.0f, 0.0f,
				0.0f, scale, 0.0f,
				0.0f, 0.0f, 1.0f);
		}

		static Mat3 Scale(float xScale, float yScale)
		{
			return Mat3(xScale, 0.0f, 0.0f,
				0.0f, yScale, 0.0f,
				0.0f, 0.0f, 1.0f);
		}

		static Mat3 Translation(float x, float y)
		{
			return Mat3(1.0f, 0.0f, x,
				0.0f, 1.0f, y,
				0.0f, 0.0f, 1.0f);
		}

		static Mat3 Translation(const Vec2& translationVector)
		{
			return Mat3(1.0f, 0.0f, translationVector.GetX(),
				0.0f, 1.0f, translationVector.GetY(),
				0.0f, 0.0f, 1.0f);
		}

		// utility
		float * GetAddress()
		{
			return &m_values[0];
		}

		// operator overloads
		Mat3 operator*(const Mat3& right) const
		{
			return Mat3(Vec3(m_values[0], m_values[1], m_values[2]).Dot(Vec3(right.m_values[0], right.m_values[3], right.m_values[6])),
				Vec3(m_values[0], m_values[1], m_values[2]).Dot(Vec3(right.m_values[1], right.m_values[4], right.m_values[7])),
				Vec3(m_values[0], m_values[1], m_values[2]).Dot(Vec3(right.m_values[2], right.m_values[5], right.m_values[8])),
				Vec3(m_values[3], m_values[4], m_values[5]).Dot(Vec3(right.m_values[0], right.m_values[3], right.m_values[6])),
				Vec3(m_values[3], m_values[4], m_values[5]).Dot(Vec3(right.m_values[1], right.m_values[4], right.m_values[7])),
				Vec3(m_values[3], m_values[4], m_values[5]).Dot(Vec3(right.m_values[2], right.m_values[5], right.m_values[8])),
				Vec3(m_values[6], m_values[7], m_values[8]).Dot(Vec3(right.m_values[0], right.m_values[3], right.m_values[6])),
				Vec3(m_values[6], m_values[7], m_values[8]).Dot(Vec3(right.m_values[1], right.m_values[4], right.m_values[7])),
				Vec3(m_values[6], m_values[7], m_values[8]).Dot(Vec3(right.m_values[2], right.m_values[5], right.m_values[8])));
		}

		Vec2 operator*(const Vec2& right) const
		{
			return Vec2(Vec3(m_values[0], m_values[1], m_values[2]).Dot(Vec3(right.GetX(), right.GetY(), 1.0f)),
				Vec3(m_values[3], m_values[4], m_values[5]).Dot(Vec3(right.GetX(), right.GetY(), 1.0f)));
		}

		Vec3 operator*(const Vec3& right) const
		{
			return Vec3(Vec3(m_values[0], m_values[1], m_values[2]).Dot(right),
				Vec3(m_values[3], m_values[4], m_values[5]).Dot(right),
				Vec3(m_values[6], m_values[7], m_values[8]).Dot(right));
		}			

	private:
		static const int ELEMENTS_PER_DIRECTION = 3;
		static const int TOTAL_ELEMENTS = ELEMENTS_PER_DIRECTION * ELEMENTS_PER_DIRECTION;
		float m_values[TOTAL_ELEMENTS];
	};
}

#endif // ifndef MAT3_H