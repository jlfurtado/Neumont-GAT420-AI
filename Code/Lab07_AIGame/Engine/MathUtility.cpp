#include "MathUtility.h"
#include "MathUtility.h"
#include <random>
#include "Vertex.h"
#include "GraphicalObject.h"
#include "Mat4.h"
#include "Mesh.h"

// Justin Furtado
// 10/13/2016
// MathUtility.h
// Degrees to radians, random, etc

namespace Engine
{
	const float MathUtility::PI = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148f;

	float MathUtility::ToRadians(float degrees)
	{
		return degrees / 180.0f * PI;
	}

	float MathUtility::ToDegrees(float radians)
	{
		return radians / PI * 180.0f;
	}

	float MathUtility::Clamp(float value, float min, float max)
	{
		float result = value;
		if (result < min) { result = min; }
		else if (result > max) { result = max; }
		return result;
	}

	 Vec3 MathUtility::Clamp(Vec3 value, Vec3 min, Vec3 max)
	{
		return Vec3(Clamp(value.GetX(), min.GetX(), max.GetX()), 
					Clamp(value.GetY(), min.GetY(), max.GetY()),
					Clamp(value.GetZ(), min.GetZ(), max.GetZ()));
	}

	int MathUtility::Clamp(int value, int min, int max)
	{
		int result = value;
		if (result < min) { result = min; }
		else if (result > max) { result = max; }
		return result;
	}

	int MathUtility::Rand(int min, int max)
	{
		return (rand() % (max - min)) + min;
	}

	float MathUtility::Rand(float minValue, float maxValue)
	{
		return (rand() * (maxValue - minValue) / RAND_MAX)  + minValue;
	}

	float MathUtility::Min(float v1, float v2)
	{
		return v1 < v2 ? v1 : v2;
	}

	float MathUtility::Max(float v1, float v2)
	{
		return v1 > v2 ? v1 : v2;
	}

	Vec3 MathUtility::Rand(const Vec3 & minVec, const Vec3 & maxVec)
	{
		// validate inputs
		Vec3 miv = Min(minVec, maxVec); 
		Vec3 mav = Max(minVec, maxVec);
		
		// do thething
		return Vec3(Rand(miv.GetX(), mav.GetX()), Rand(miv.GetY(), mav.GetY()), Rand(miv.GetZ(), mav.GetZ()));
	}

	Vec3 MathUtility::Min(const Vec3 & v1, const Vec3 & v2)
	{
		return Vec3(Min(v1.GetX(), v2.GetX()),
					Min(v1.GetY(), v2.GetY()),
					Min(v1.GetZ(), v2.GetZ()));
	}

	Vec3 MathUtility::Max(const Vec3 & v1, const Vec3 & v2)
	{
		return Vec3(Max(v1.GetX(), v2.GetX()),
					Max(v1.GetY(), v2.GetY()),
					Max(v1.GetZ(), v2.GetZ()));
	}

	Vec3 MathUtility::GetNormalFromRayCastingOutput(RayCastingOutput output)
	{
		Vec3 normal;
		if (output.m_belongsTo->GetMeshPointer()->GetVertexFormat() & VertexFormat::HasNormal)
		{
			Mat4 rot = output.m_belongsTo->GetRotMat();
			Vec3 n1 = reinterpret_cast<Vertex *>(output.m_belongsTo->GetMeshPointer()->GetNormalAt(output.m_vertexIndex + 0))->m_position.Normalize();
			Vec3 n2 = reinterpret_cast<Vertex *>(output.m_belongsTo->GetMeshPointer()->GetNormalAt(output.m_vertexIndex + 1))->m_position.Normalize();
			Vec3 n3 = reinterpret_cast<Vertex *>(output.m_belongsTo->GetMeshPointer()->GetNormalAt(output.m_vertexIndex + 2))->m_position.Normalize();

			n1 = rot * n1;
			n2 = rot * n2;
			n3 = rot * n3;

			normal = (output.m_alphaBetaGamma.GetX() * n2 + output.m_alphaBetaGamma.GetY() * n3 + output.m_alphaBetaGamma.GetZ() * n1).Normalize();
		}
		else
		{
			normal = output.m_triangleNormal.Normalize();
		}

		return normal;
	}

	float MathUtility::GetVectorAngleRadians(const Vec3& left, const Vec3& right)
	{
		return acosf(left.Normalize().Dot(right.Normalize()));
	}

	Vec3 MathUtility::GetQuadification(int index, int objectsX, int objectsZ, float spacing)
	{
		return Vec3(((index%objectsX - (objectsX / 2 - 0.5f))*spacing), 0.0f, (index / objectsZ - (objectsZ / 2 - 0.5f))*spacing);
	}

	Vec3 MathUtility::GetCubification(int index, int objectsX, int objectsY, int objectsZ, float spacing)
	{
		return Vec3((((index % objectsX) - (objectsX / 2 - 0.5f))*spacing),
					 ((((index / objectsX) % objectsY) - (objectsY / 2 - 0.5f))*spacing),
					 (((index / (objectsX * objectsY)) - (objectsZ / 2 - 0.5f))*spacing));
	}

	Vec3 MathUtility::GetRandSphereEdgeVec(float radius)
	{
		float phi = MathUtility::Rand(0.0f, 2 * PI);
		float theta = MathUtility::Rand(0.0f, 2 * PI);

		return (radius * Vec3(sinf(theta)*cosf(phi), cosf(theta), sinf(theta)*sinf(phi)).Normalize());
	}
	
	

}