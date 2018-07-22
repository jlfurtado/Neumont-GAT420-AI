#ifndef BUFFERINFO_H
#define BUFFERINFO_H

// Justin Furtado
// 7/31/2016
// BufferInfo.h
// A vertex and index buffer

#include "Mesh.h"
#include "VertexFormat.h"
#include "GraphicalObject.h"
#include "LinkedList.h"

namespace Engine
{
	class BufferInfo
	{
	public:
		BufferInfo();
		~BufferInfo();

		bool Initialize(GLuint bufferSize);
		bool Shutdown();

		bool AddMesh(Mesh *pMeshToAdd);
		bool AddGraphicalObject(GraphicalObject *pGraphicalObjectToAdd);
		void RemoveGraphicalObject(GraphicalObject *pGraphicalObjectToRemove);
		bool ContainedInBuffer(GraphicalObject *pObjToCheck) const;
		bool HasRoomFor(GLuint vertexSizeBytes, GLuint indexSizeBytes);
		bool BelongsInBuffer(GLuint vertexBufferID, GLuint indexBufferID);
		bool BelongsInBuffer(Mesh *pMeshToCheck);
		bool BelongsInBuffer(GraphicalObject *pGraphicalObjectToCheck);
		LinkedList<GraphicalObject*> *GetGraphicalObjectList();
		GLuint GetVertexBufferID();
		GLuint GetIndexBufferID();

	private:
		// default buffer size for all buffer info from config file here...
		LinkedList<GraphicalObject*> m_graphicalObjectList;
		GLuint m_vertexBufferID;
		GLuint m_indexBufferID;
		GLuint m_vertexBufferOffsetBytes;
		GLuint m_indexBufferOffsetBytes;
		GLuint m_bufferSize;
		GLuint m_vertexCount;
		GLuint m_indexCount;
		GLuint m_meshCount;
	};
}

#endif // ifndef BUFFERINFO_H