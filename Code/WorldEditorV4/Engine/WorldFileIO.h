#ifndef WORLDFILEIO_H
#define WORLDFILEIO_H

// Justin Furtado
// 4/27/2017
// WorldFileIO.h
// Manages file io for world files

#include "ExportHeader.h"
#include "LinkedList.h"
#include "GraphicalObject.h"
#include <fstream>

namespace Engine
{
	class WorldFileIO
	{
	public:
		typedef void(*ObjectInitializerCallback)(Engine::GraphicalObject *pGob, void *pClass);
		static ENGINE_SHARED bool WriteFile(LinkedList<GraphicalObject> *pObjsToWrite, const char *filePath);
		static ENGINE_SHARED bool ReadFile(const char *filePath, LinkedList<GraphicalObject> *outGobs, unsigned shaderProgramID, ObjectInitializerCallback objInit, void *pClass);

	private:
		static std::ofstream outFile;
		static std::ifstream inFile;

		static const int FORMAT_VERSION = 4;
		static bool WriteSingleObject(Engine::GraphicalObject *pObj, void *);
	};
}


#endif // ifndef WORLDFILEIO_H