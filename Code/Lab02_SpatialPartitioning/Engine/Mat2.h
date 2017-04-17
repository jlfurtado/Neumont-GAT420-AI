#ifndef MAT2_H
#define MAT2_H

// Justin Furtado
// Mat2.h
// 10/12/2016
// Matrix2x2 class for transforming vectors

#include "ExportHeader.h"
#include "Vec2.h"
#include <cmath>

namespace Engine
{
	class ENGINE_SHARED Mat2
	{
	public:
		// constructors
		Mat2() : m_values{ 1.0f, 0.0f, 0.0f, 1.0f } {}
		Mat2(const Vec2& leftColumnVector, const Vec2& rightColumnVector) : m_values{ leftColumnVector.GetX(), rightColumnVector.GetX(), leftColumnVector.GetY(), rightColumnVector.GetY() } { }
		Mat2(float topLeft, float topRight, float bottomLeft, float bottomRight) : m_values{ topLeft, topRight, bottomLeft, bottomRight } { }

		// destructor
		~Mat2() {}

		// matrix creation methods
		static Mat2 Rotate(float radians)
		{
			return Mat2(Vec2(cosf(radians), sinf(radians)), Vec2(-sinf(radians), cosf(radians)));
		}
		static Mat2 Scale(float uniformScale)
		{
			return Mat2(Vec2(uniformScale, 0.0f), Vec2(0.0f, uniformScale));
		}
		static Mat2 Scale(float xScale, float yScale)
		{
			return Mat2(Vec2(xScale, 0.0f), Vec2(0.0f, yScale));
		}

		// utility
		float *GetAddress()
		{
			return &m_values[0];
		}

		// operators
		Mat2 operator*(const Mat2& right) const
		{
			/*
			|a00 a10|	|b00 b10|   |a00*b00+a10*b01 a00*b10+a10*b11|
			|a01 a11| * |b01 b11| = |a01*b00+a11*b01 a01*b10+a11*b11|
			*/
			return Mat2(Vec2(Vec2(m_values[0], m_values[1]).Dot(Vec2(right.m_values[0], right.m_values[2])),
				Vec2(m_values[2], m_values[3]).Dot(Vec2(right.m_values[0], right.m_values[2]))),
				Vec2(Vec2(m_values[0], m_values[1]).Dot(Vec2(right.m_values[1], right.m_values[3])),
					Vec2(m_values[2], m_values[3]).Dot(Vec2(right.m_values[1], right.m_values[3]))));
		}

		Vec2 operator*(const Vec2& right) const
		{
			/*
			| a00 a10 |   | b00 |   | a00*b00+a10*b01 |
			| a01 a11 | * | b01 | = | a01*b00+a11*b01 |
			*/
			return Vec2(Vec2(m_values[0], m_values[1]).Dot(right), Vec2(m_values[2], m_values[3]).Dot(right));
		}

	private:
		static const int ELEMENTS_PER_DIRECTION = 2;
		static const int TOTAL_ELEMENTS = ELEMENTS_PER_DIRECTION * ELEMENTS_PER_DIRECTION;
		float m_values[TOTAL_ELEMENTS];
	};
}

#endif // ifndef MAT2_H