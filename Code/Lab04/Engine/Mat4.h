#ifndef MAT4_H
#define MAT4_H

// Justin Furtado
// 10/13/2016
// Mat4.h
// A Matrix4c4 class for transforming vectors

#include "ExportHeader.h"
#include "Vec2.h"
#include "Vec3.h"
#include "Vec4.h"

namespace Engine
{
	class ENGINE_SHARED Mat4
	{
	public:
		// constructors
		Mat4() 
			: m_values{ 1.0f, 0.0f, 0.0f, 0.0f,
						0.0f, 1.0f, 0.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f } { }

		Mat4(const Vec3& left, const Vec3& middle, const Vec3& right)
			: m_values{ left.GetX(), middle.GetX(), right.GetX(), 0.0f,
						left.GetY(), middle.GetY(), right.GetY(), 0.0f,
						left.GetZ(), middle.GetZ(), right.GetZ(), 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f } { }

		Mat4(const Vec4& left, const Vec4& middleLeft, const Vec4 middleRight, const Vec4& right)
			: m_values{ left.GetX(), middleLeft.GetX(), middleRight.GetX(), right.GetX(),
						left.GetY(), middleLeft.GetY(), middleRight.GetY(), right.GetY(),
						left.GetZ(), middleLeft.GetZ(), middleRight.GetZ(), right.GetZ(),
						left.GetW(), middleLeft.GetW(), middleRight.GetW(), right.GetW() } { }

		Mat4(float f0, float f1, float f2, float f3, float f4, float f5, float f6, float f7, float f8, float f9, float f10, float f11, float f12, float f13, float f14, float f15)
			: m_values{ f0, f4, f8, f12,
						f1, f5, f9, f13,
						f2, f6, f10, f14,
						f3, f7, f11, f15 } { }
		
		//destructor
		~Mat4() {}

		// public static methods that return matrices
		static Mat4 RotationAroundAxis(const Vec3& axisToRotateAbout, float radians)
		{
			Vec3 axis = axisToRotateAbout.Normalize();
			float s = sinf(radians);
			float c = cosf(radians);
			float oc = 1.0f - c;

			return Mat4(oc * axis.GetX() * axis.GetX() + c, oc * axis.GetX() * axis.GetY() - axis.GetZ() * s, oc * axis.GetZ() * axis.GetX() + axis.GetY() * s, 0.0f,
						oc * axis.GetX() * axis.GetY() + axis.GetZ() * s, oc * axis.GetY() * axis.GetY() + c, oc * axis.GetY() * axis.GetZ() - axis.GetX() * s, 0.0f,
						oc * axis.GetZ() * axis.GetX() - axis.GetY() * s, oc * axis.GetY() * axis.GetZ() + axis.GetX() * s, oc * axis.GetZ() * axis.GetZ() + c, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f);

		}

		static Mat4 RotationToFace(const Vec3& baseDir, const Vec3& newDir)
		{
			Vec3 bd = baseDir.Normalize();
			Vec3 nd = newDir.Normalize();
			Vec3 pD = bd.Cross(nd);
			return RotationAroundAxis(pD,acosf(bd.Dot(nd)));
		}

		static Mat4 Scale(float scale)
		{
			return Mat4(scale, 0.0f, 0.0f, 0.0f,
						0.0f, scale, 0.0f, 0.0f,
						0.0f, 0.0f, scale, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f);
		}

		static Mat4 Scale(float xScale, float yScale, float zScale)
		{
			return Mat4(xScale, 0.0f, 0.0f, 0.0f,
						0.0f, yScale, 0.0f, 0.0f,
						0.0f, 0.0f, zScale, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f);
		}

		static Mat4 Scale(float scale, Vec3 axis)
		{
			Vec3 axisNormalized = axis.Normalize();
			float kMinusOne = scale - 1.0f;
			float x = axisNormalized.GetX();
			float y = axisNormalized.GetY();
			float z = axisNormalized.GetZ();
			return Mat4(1 + kMinusOne*x*x, kMinusOne*x*y, kMinusOne*x*z, 0.0f,
						kMinusOne*x*y, 1 + kMinusOne*y*y, kMinusOne*y*z, 0.0f,
						kMinusOne*x*z, kMinusOne*y*z, 1 + kMinusOne*z*z, 0.0f,
						0.0f, 0.0f, 0.0f, 1.0f);
		}

		static Mat4 Translation(float x, float y, float z)
		{
			return Mat4(1.0f, 0.0f, 0.0f, x,
						0.0f, 1.0f, 0.0f, y,
						0.0f, 0.0f, 1.0f, z,
						0.0f, 0.0f, 0.0f, 1.0f);
		}

		static Mat4 Translation(const Vec3& translationVector)
		{
			return Translation(translationVector.GetX(), translationVector.GetY(), translationVector.GetZ());
		}

		static Mat4 Perspective(float fieldOfViewY, float aspectRatio, float nearPlane, float farPlane)
		{
			if (fieldOfViewY <= 0.0f || aspectRatio == 0.0f) { GameLogger::Log(MessageType::Error, "Invalid perspective matrix! fov = [%.3f], aspect = [%.3f]", fieldOfViewY, aspectRatio); }

			float f = 1.0f / tanf(0.5f*fieldOfViewY);
			float depth = farPlane - nearPlane;
			float oneOverDepth = 1.0f / depth;

			return Mat4(-f / aspectRatio, 0.0f, 0.0f, 0.0f,
						0.0f, f, 0.0f, 0.0f,
						0.0f, 0.0f, farPlane*oneOverDepth, -farPlane*nearPlane*oneOverDepth,
						0.0f, 0.0f, 1.0f, 0.0f);
		}

		static Mat4 Orthographic(float left, float right, float top, float bottom)
		{
			return Mat4(2.0f / (right - left), 0.0f, 0.0f, -(right + left) / (right - left),
						0.0f, 2.0f / (top - bottom), 0.0f, -(top + bottom) / (top - bottom),
						0.0f, 0.0f, -1.0f, 0.0f,
						0.0f, 0.0f, 1.0f, 1.0f );
		}

		static Mat4 LookAt(const Vec3& cameraPosition, const Vec3& targetPosition, const Vec3& up)
		{
			Vec3 z = (targetPosition - cameraPosition).Normalize();
			Vec3 x = (up.Cross(z)).Normalize();
			Vec3 y = z.Cross(x);

			return Mat4(x.GetX(), x.GetY(), x.GetZ(), -x.Dot(cameraPosition),
						y.GetX(), y.GetY(), y.GetZ(), -y.Dot(cameraPosition),
						z.GetX(), z.GetY(), z.GetZ(), -z.Dot(cameraPosition),
						0.0f, 0.0f, 0.0f, 1.0f);
		}

		static Mat4 ViewPort(float right, float left, float top, float bottom, float far, float near)
		{
			return Mat4(0.5f * (right - left), 0.0f, 0.0f, 0.5f * (right + left),
						0.0f, 0.5f * (top - bottom), 0.0f, 0.5f * (top + bottom),
						0.0f, 0.0f, 0.5f * (far - near), 0.5f * (far + near),
						0.0f, 0.0f, 0.0f, 1.0f);
		}

		static Mat4 Bias()
		{
			return Mat4(0.5f, 0.0f, 0.0f, 0.5f,
						0.0f, 0.5f, 0.0f, 0.5f,
						0.0f, 0.0f, 0.5f, 0.5f,
						0.0f, 0.0f, 0.0f, 1.0f);
		}

		static Mat4 InfinitePerspective(float fovy, float aspect, float near)
		{
			float range = tanf(fovy * 0.5f) * near;
			float left = -range * aspect;
			float right = range * aspect;
			float bottom = -range;
			float top = range;

			return Mat4((2.0f * near) / (right - left), 0.0f, 0.0f, 0.0f,
						0.0f, (2.0f * near) / (top - bottom), 0.0f, 0.0f,
						0.0f, 0.0f, -1.0f, -1.0f,
						0.0f, 0.0f, -2.0f * near, 0.0f);
		}

		static Mat4 Inverse(Mat4 input)
		{
			Mat4 inverse;
			float determinant;

			inverse[0] = input[5] * input[10] * input[15] - input[5] * input[11] * input[14] - input[9] * input[6] * input[15] + input[9] * input[7] * input[14] + input[13] * input[6] * input[11] - input[13] * input[7] * input[10];
			inverse[4] = -input[4] * input[10] * input[15] + input[4] * input[11] * input[14] + input[8] * input[6] * input[15] - input[8] * input[7] * input[14] - input[12] * input[6] * input[11] + input[12] * input[7] * input[10];
			inverse[8] = input[4] * input[9] * input[15] - input[4] * input[11] * input[13] - input[8] * input[5] * input[15] + input[8] * input[7] * input[13] + input[12] * input[5] * input[11] - input[12] * input[7] * input[9];
			inverse[12] = -input[4] * input[9] * input[14] + input[4] * input[10] * input[13] + input[8] * input[5] * input[14] - input[8] * input[6] * input[13] - input[12] * input[5] * input[10] + input[12] * input[6] * input[9];
			inverse[1] = -input[1] * input[10] * input[15] + input[1] * input[11] * input[14] + input[9] * input[2] * input[15] - input[9] * input[3] * input[14] - input[13] * input[2] * input[11] + input[13] * input[3] * input[10];
			inverse[5] = input[0] * input[10] * input[15] - input[0] * input[11] * input[14] - input[8] * input[2] * input[15] + input[8] * input[3] * input[14] + input[12] * input[2] * input[11] - input[12] * input[3] * input[10];
			inverse[9] = -input[0] * input[9] * input[15] + input[0] * input[11] * input[13] + input[8] * input[1] * input[15] - input[8] * input[3] * input[13] - input[12] * input[1] * input[11] + input[12] * input[3] * input[9];
			inverse[13] = input[0] * input[9] * input[14] - input[0] * input[10] * input[13] - input[8] * input[1] * input[14] + input[8] * input[2] * input[13] + input[12] * input[1] * input[10] - input[12] * input[2] * input[9];
			inverse[2] = input[1] * input[6] * input[15] - input[1] * input[7] * input[14] - input[5] * input[2] * input[15] + input[5] * input[3] * input[14] + input[13] * input[2] * input[7] - input[13] * input[3] * input[6];
			inverse[6] = -input[0] * input[6] * input[15] + input[0] * input[7] * input[14] + input[4] * input[2] * input[15] - input[4] * input[3] * input[14] - input[12] * input[2] * input[7] + input[12] * input[3] * input[6];
			inverse[10] = input[0] * input[5] * input[15] - input[0] * input[7] * input[13] - input[4] * input[1] * input[15] + input[4] * input[3] * input[13] + input[12] * input[1] * input[7] - input[12] * input[3] * input[5];
			inverse[14] = -input[0] * input[5] * input[14] + input[0] * input[6] * input[13] + input[4] * input[1] * input[14] - input[4] * input[2] * input[13] - input[12] * input[1] * input[6] + input[12] * input[2] * input[5];
			inverse[3] = -input[1] * input[6] * input[11] + input[1] * input[7] * input[10] + input[5] * input[2] * input[11] - input[5] * input[3] * input[10] - input[9] * input[2] * input[7] + input[9] * input[3] * input[6];
			inverse[7] = input[0] * input[6] * input[11] - input[0] * input[7] * input[10] - input[4] * input[2] * input[11] + input[4] * input[3] * input[10] + input[8] * input[2] * input[7] - input[8] * input[3] * input[6];
			inverse[11] = -input[0] * input[5] * input[11] + input[0] * input[7] * input[9] + input[4] * input[1] * input[11] - input[4] * input[3] * input[9] - input[8] * input[1] * input[7] + input[8] * input[3] * input[5];
			inverse[15] = input[0] * input[5] * input[10] - input[0] * input[6] * input[9] - input[4] * input[1] * input[10] + input[4] * input[2] * input[9] + input[8] * input[1] * input[6] - input[8] * input[2] * input[5];

			determinant = input[0] * inverse[0] + input[1] * inverse[4] + input[2] * inverse[8] + input[3] * inverse[12];

			if (determinant == 0) return Mat4();

			determinant = 1.0f / determinant;

			for (int i = 0; i < 16; i++)
				inverse[i] = inverse[i] * determinant;

			return inverse;
		}

		// utility
		float * GetAddress()
		{
			return &m_values[0];
		}

		// operator overloads
		Mat4 operator*(const Mat4& right) const
		{
			/*
			|0 4 8  12 |   |0 4 8  12 |
			|1 5 9  13 |   |1 5 9  13 |
			|2 6 10 14 |   |2 6 10 14 |
			|3 7 11 15 | * |3 7 11 15 |*/
			return Mat4(Vec4(m_values[0], m_values[4], m_values[8], m_values[12]).Dot(Vec4(right.m_values[0], right.m_values[1], right.m_values[2], right.m_values[3])),
				Vec4(m_values[0], m_values[4], m_values[8], m_values[12]).Dot(Vec4(right.m_values[4], right.m_values[5], right.m_values[6], right.m_values[7])),
				Vec4(m_values[0], m_values[4], m_values[8], m_values[12]).Dot(Vec4(right.m_values[8], right.m_values[9], right.m_values[10], right.m_values[11])),
				Vec4(m_values[0], m_values[4], m_values[8], m_values[12]).Dot(Vec4(right.m_values[12], right.m_values[13], right.m_values[14], right.m_values[15])),

				Vec4(m_values[1], m_values[5], m_values[9], m_values[13]).Dot(Vec4(right.m_values[0], right.m_values[1], right.m_values[2], right.m_values[3])),
				Vec4(m_values[1], m_values[5], m_values[9], m_values[13]).Dot(Vec4(right.m_values[4], right.m_values[5], right.m_values[6], right.m_values[7])),
				Vec4(m_values[1], m_values[5], m_values[9], m_values[13]).Dot(Vec4(right.m_values[8], right.m_values[9], right.m_values[10], right.m_values[11])),
				Vec4(m_values[1], m_values[5], m_values[9], m_values[13]).Dot(Vec4(right.m_values[12], right.m_values[13], right.m_values[14], right.m_values[15])),

				Vec4(m_values[2], m_values[6], m_values[10], m_values[14]).Dot(Vec4(right.m_values[0], right.m_values[1], right.m_values[2], right.m_values[3])),
				Vec4(m_values[2], m_values[6], m_values[10], m_values[14]).Dot(Vec4(right.m_values[4], right.m_values[5], right.m_values[6], right.m_values[7])),
				Vec4(m_values[2], m_values[6], m_values[10], m_values[14]).Dot(Vec4(right.m_values[8], right.m_values[9], right.m_values[10], right.m_values[11])),
				Vec4(m_values[2], m_values[6], m_values[10], m_values[14]).Dot(Vec4(right.m_values[12], right.m_values[13], right.m_values[14], right.m_values[15])),

				Vec4(m_values[3], m_values[7], m_values[11], m_values[15]).Dot(Vec4(right.m_values[0], right.m_values[1], right.m_values[2], right.m_values[3])),
				Vec4(m_values[3], m_values[7], m_values[11], m_values[15]).Dot(Vec4(right.m_values[4], right.m_values[5], right.m_values[6], right.m_values[7])),
				Vec4(m_values[3], m_values[7], m_values[11], m_values[15]).Dot(Vec4(right.m_values[8], right.m_values[9], right.m_values[10], right.m_values[11])),
				Vec4(m_values[3], m_values[7], m_values[11], m_values[15]).Dot(Vec4(right.m_values[12], right.m_values[13], right.m_values[14], right.m_values[15])));
		}

		Vec3 operator*(const Vec3& right) const
		{
			/*
			|0 4 8  12 |   |0|
			|1 5 9  13 |   |1|
			|2 6 10 14 |   |2|
			|3 7 11 15 | * |3|*/
			return Vec3(Vec3(m_values[0], m_values[4], m_values[8]).Dot(Vec3(right.GetX(), right.GetY(), right.GetZ())) + m_values[12],
				Vec3(m_values[1], m_values[5], m_values[9]).Dot(Vec3(right.GetX(), right.GetY(), right.GetZ())) + m_values[13],
				Vec3(m_values[2], m_values[6], m_values[10]).Dot(Vec3(right.GetX(), right.GetY(), right.GetZ())) + m_values[14]);
		}

		Vec4 operator*(const Vec4& right) const
		{
			return Vec4(Vec4(m_values[0], m_values[4], m_values[8], m_values[12]).Dot(right),
				Vec4(m_values[1], m_values[5], m_values[9], m_values[13]).Dot(right),
				Vec4(m_values[2], m_values[6], m_values[10], m_values[14]).Dot(right),
				Vec4(m_values[3], m_values[7], m_values[11], m_values[15]).Dot(right));
		}

		float& operator[](unsigned index)
		{
			return m_values[index];
		}

	private:

		static const int ELEMENTS_PER_DIRECTION = 4;
		static const int CACHE_SIZE = 1000;
		static const int TOTAL_ELEMENTS = ELEMENTS_PER_DIRECTION * ELEMENTS_PER_DIRECTION;
		float m_values[TOTAL_ELEMENTS];
	};

}

#endif // ifndef MAT4_H