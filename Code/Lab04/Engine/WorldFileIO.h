#ifndef WORLDFILEIO_H
#define WORLDFILEIO_H

// Justin Furtado
// 4/24/2017
// WorldFileIO.h
// Handles input and output for world files

#include "LinkedList.h"
#include "GraphicalObject.h"
#include <fstream>

namespace Engine
{
	class WorldFileIO
	{
	public:
		static bool Initialize();
		static bool Shutdown();

		static GraphicalObject *ReadGobs(const char *const filePath);
		static bool WriteGobs(GraphicalObject *pGobs, int numGobs);

	private:
		static bool GetGobCount(std::ifstream& inputFile, int *outCount);
		static bool DeleteObjectCallback(GraphicalObject * pGob, void *pClass);
		static bool CleanUp();

		static LinkedList<GraphicalObject> s_allocatedGobs;
		static int s_allocatedGobCount;
	};

}

#endif // ifndef WORLDFILEIO_H