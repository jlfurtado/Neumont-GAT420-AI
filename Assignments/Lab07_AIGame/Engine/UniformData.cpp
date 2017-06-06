#include "UniformData.h"
#include "GL\glew.h"
#include "GameLogger.h"
#include <assert.h>
#include "MyGL.h"

namespace Engine
{
	UniformData::UniformData()
		: m_uniformType(GL_FLOAT_VEC3), m_pUniformData(nullptr), m_uniformDataLoc(0)
	{
	}

	UniformData::UniformData(GLenum type, void * dataAddress, int dataLoc, bool log)
		: m_uniformType(type), m_pUniformData(dataAddress), m_uniformDataLoc(dataLoc), m_logForThis(log)
	{
		InitFromType();
	}

	bool UniformData::PassUniform()
	{
		m_callback(this);
		return true;
	}

	void ** UniformData::GetUniformDataPtrPtr()
	{
		return &m_pUniformData;
	}

	int UniformData::GetUniformDataLoc() const
	{
		return m_uniformDataLoc;
	}
	GLenum UniformData::GetType() const
	{
		return m_uniformType;
	}

	void UniformData::InitFromType()
	{
		if (!m_pUniformData) { GameLogger::Log(MessageType::cError, "Uniform InitFromType called but data address is nullptr!\n"); }
		if (m_uniformDataLoc <= 0) { GameLogger::Log(MessageType::cError, "Uniform InitFromType called but data location was less than or equal to zero!\n"); }

		switch (m_uniformType)
		{
		case GL_FLOAT_MAT4:
			m_callback = PassFloatMat4;
			break;

		case GL_FLOAT_VEC3:
			m_callback = PassFloatVec3;
			break;

		case GL_FLOAT:
			m_callback = PassFloat;
			break;

		case GL_INT:
			m_callback = PassInt;
			break;

		case GL_FLOAT_VEC4:
			m_callback = PassFloatVec4;
			break;

		case GL_FLOAT_VEC2:
			m_callback = PassFloatVec2;
			break;

		case GL_VERTEX_SHADER:
		case GL_GEOMETRY_SHADER:
		case GL_FRAGMENT_SHADER:
			m_callback = PassSubroutineIndex;
			break;

		case GL_TEXTURE0:
		case GL_TEXTURE1:
		case GL_TEXTURE2:
		case GL_TEXTURE3:
		case GL_TEXTURE4:
		case GL_TEXTURE5:
		case GL_TEXTURE6:
		case GL_TEXTURE7:
		case GL_TEXTURE8:
		case GL_TEXTURE9:
		case GL_TEXTURE10:
		case GL_TEXTURE11:
		case GL_TEXTURE12:
		case GL_TEXTURE13:
		case GL_TEXTURE14:
		case GL_TEXTURE15:
		case GL_TEXTURE16:
		case GL_TEXTURE17:
		case GL_TEXTURE18:
		case GL_TEXTURE19:
		case GL_TEXTURE20:
		case GL_TEXTURE21:
		case GL_TEXTURE22:
		case GL_TEXTURE23:
		case GL_TEXTURE24:
		case GL_TEXTURE25:
		case GL_TEXTURE26:
		case GL_TEXTURE27:
		case GL_TEXTURE28:
		case GL_TEXTURE29:
		case GL_TEXTURE30:
		case GL_TEXTURE31:
			m_callback = PassTexture;
			break;

		default:
			m_callback = UnknownType;
		}
	}

	void UniformData::PassFloatMat4(UniformData * pData)
	{
		glUniformMatrix4fv(pData->m_uniformDataLoc, 1, GL_FALSE, reinterpret_cast<float*>(pData->m_pUniformData));
		// log
		//			if (m_logForThis) { GameLogger::Log(MessageType::ConsoleOnly, "Passed (%.3f, %.3f, %.3f, %.3f)\n\t\t\t\t(%.3f, %.3f, %.3f, %.3f)\n\t\t\t\t(%.3f, %.3f, %.3f, %.3f)\n\t\t\t\t(%.3f, %.3f, %.3f, %.3f) to uniform [%d]\n", *(reinterpret_cast<float*>(m_pUniformData) + 0), *(reinterpret_cast<float*>(m_pUniformData) + 4), *(reinterpret_cast<float*>(m_pUniformData) + 8), *(reinterpret_cast<float*>(m_pUniformData) + 12), 				 																																											 *(reinterpret_cast<float*>(m_pUniformData) + 3), *(reinterpret_cast<float*>(m_pUniformData) + 7), *(reinterpret_cast<float*>(m_pUniformData) + 11), *(reinterpret_cast<float*>(m_pUniformData) + 15), m_uniformDataLoc); }

	}

	void UniformData::PassInt(UniformData * pData)
	{
		glUniform1i(pData->m_uniformDataLoc, *reinterpret_cast<int*>(pData->m_pUniformData));
		// log
		//if (m_logForThis) { GameLogger::Log(MessageType::ConsoleOnly, "Passed %d to uniform [%d]\n", *reinterpret_cast<int*>(m_pUniformData), m_uniformDataLoc); }
	}

	void UniformData::PassFloatVec4(UniformData * pData)
	{
		float *pUniformData = reinterpret_cast<float*>(pData->m_pUniformData);
		glUniform4f(pData->m_uniformDataLoc, *pUniformData, *(pUniformData + 1), *(pUniformData + 2), *(pUniformData + 3));
		// LOG
		//if (pData->m_logForThis) { GameLogger::Log(MessageType::ConsoleOnly, "Passed (%.3f, %.3f, %.3f, %.3f) to uniform [%d]\n", *reinterpret_cast<float*>(m_pUniformData), *(pUniformData + 1), *(pUniformData + 2), *(pUniformData + 3), m_uniformDataLoc); }

	}

	void UniformData::PassFloatVec3(UniformData * pData)
	{
		float *pUniformData = reinterpret_cast<float*>(pData->m_pUniformData);
		glUniform3f(pData->m_uniformDataLoc, *pUniformData, *( pUniformData + 1), *(pUniformData + 2));
		// log
		//			//if (m_logForThis) { GameLogger::Log(MessageType::ConsoleOnly, "Passed (%.3f, %.3f, %.3f) to uniform [%d]\n", *reinterpret_cast<float*>(m_pUniformData), *(reinterpret_cast<float*>(m_pUniformData) + 1), *(reinterpret_cast<float*>(m_pUniformData) + 2), m_uniformDataLoc); }

	}

	void UniformData::PassFloatVec2(UniformData * pData)
	{
		float *pUniformData = reinterpret_cast<float*>(pData->m_pUniformData);
		glUniform2f(pData->m_uniformDataLoc, *pUniformData, *(pUniformData) + 1);
		// log
		//if (m_logForThis) { GameLogger::Log(MessageType::ConsoleOnly, "Passed (%.3f, %.3f) to uniform [%d]\n", *reinterpret_cast<float*>(m_pUniformData), *(reinterpret_cast<float*>(m_pUniformData) + 1), m_uniformDataLoc); }

	}

	void UniformData::PassFloat(UniformData * pData)
	{
		glUniform1f(pData->m_uniformDataLoc, *reinterpret_cast<float*>(pData->m_pUniformData));
		// log
	}

	void UniformData::PassSubroutineIndex(UniformData * pData)
	{
		glUniformSubroutinesuiv(pData->m_uniformType, pData->m_uniformDataLoc, reinterpret_cast<GLuint*>(pData->m_pUniformData)); // TODO: IT WORKS BUT ITS NOT READABLE (STUFF BADLY NAMED FOR THIS CASE WHICH IS UGLY ANYWAY)... REFACTOR!!!
		// log
	}

	void UniformData::PassTexture(UniformData * pData)
	{
		glActiveTexture(pData->m_uniformType);
		glBindTexture(GL_TEXTURE_2D, *reinterpret_cast<int*>(pData->m_pUniformData));
		glUniform1i(pData->m_uniformDataLoc, (pData->m_uniformType - GL_TEXTURE0));
		// log
		//if (m_logForThis) { GameLogger::Log(MessageType::ConsoleOnly, "Passed (%d) to uniform [%d]\n", *reinterpret_cast<int*>(m_pUniformData), m_uniformDataLoc); }
	}

	void Engine::UniformData::UnknownType(UniformData * pData)
	{
		GameLogger::Log(MessageType::cError, "Unknown uniform data type [%d]!\n", pData->m_uniformType);
	}
}


