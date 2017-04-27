#ifndef WORLDFILEIO_H
#define WORLDFILEIO_H

// Justin Furtado
// 4/27/2017
// WorldFileIO.h
// Manages file io for world files

#include "ExportHeader.h"

namespace Engine
{
	class GraphicalObject; // only need ptr so pre-declare
	class ENGINE_SHARED WorldFileIO
	{
	public:
		static bool WriteFile(GraphicalObject *pObjToLoad, const char *filePath, const char *meshPath);
		static bool ReadFile(const char *filePath, GraphicalObject *outGob, unsigned shaderProgramID);
	};
}


#endif // ifndef WORLDFILEIO_H