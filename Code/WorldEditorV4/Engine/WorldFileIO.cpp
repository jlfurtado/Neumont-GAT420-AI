#include "WorldFileIO.h"
#include "Mesh.h"
#include "GraphicalObject.h"
#include <fstream>
#include "GameLogger.h"
#include "StringFuncs.h"
#include "ShapeGenerator.h"

// Justin Furtado
// 4/27/2017
// WorldFileIO.h
// Manages file io for world files

namespace Engine
{

	// TO START: Get basic working
	// Write out the full transform and the mesh path and thats it
	bool WorldFileIO::WriteFile(GraphicalObject * pObjToLoad, const char * filePath, const char *meshPath)
	{
		std::ofstream file;

		// open the file, start at the beginning, error check
		file.open(filePath, std::ios::binary | std::ios::out);
		if (!file) { GameLogger::Log(MessageType::cError, "Failed to write file [%s]! Could not open file!\n", filePath); return false; }
		file.seekp(0);

		// write out the transform, in binary
		file.write(reinterpret_cast<char*>(pObjToLoad->GetFullTransformPtr()), sizeof(*pObjToLoad->GetFullTransformPtr()));

		// find length of path
		int len = StringFuncs::StringLen(meshPath);

		// write out mesh path length in binary
		file.write(reinterpret_cast<char*>(&len), sizeof(int));

		// write out the mesh path in binary
		file.write(meshPath, len);

		// close file, indicate success
		file.close();
		return true;
	}

	bool WorldFileIO::ReadFile(const char * filePath, Engine::GraphicalObject * outGob, unsigned shaderProgramID)
	{
		char buffer[256]{ '\0' };

		std::ifstream file;

		// open the file, start at the beginning, error check
		file.open(filePath, std::ios::binary | std::ios::in);
		if (!file) { GameLogger::Log(MessageType::cError, "Failed to read file [%s]! Could not open file!\n", filePath); return false; }

		// read in the transform
		file.read(reinterpret_cast<char*>(outGob->GetFullTransformPtr()), sizeof(*outGob->GetFullTransformPtr()));

		// read in the mesh path length
		int len = 0;
		file.read(reinterpret_cast<char*>(&len), sizeof(int));

		// read in the mesh path
		file.read(&buffer[0], len);
		
		// make a gob out of it
		const char *str = StringFuncs::AddToCharArray(&buffer[0]);
		ShapeGenerator::ReadSceneFile(str, outGob, shaderProgramID);
		
		// close file and indicate success
		file.close();
		return true;
	}

}