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
	std::ofstream WorldFileIO::outFile;
	std::ifstream WorldFileIO::inFile;

	bool WorldFileIO::WriteGobFile(LinkedList<GraphicalObject*>* pObjsToWrite, const char * filePath)
	{
		// open the file, start at the beginning, error check
		outFile.open(filePath, std::ios::binary | std::ios::out);
		if (!outFile) { GameLogger::Log(MessageType::cError, "Failed to write file [%s]! Could not open file!\n", filePath); return false; }
		outFile.seekp(0);
		
		// write the format version
		outFile.write(reinterpret_cast<const char*>(&FORMAT_VERSION), sizeof(FORMAT_VERSION));

		// write the obj count
		int objCount = pObjsToWrite->GetCount();
		outFile.write(reinterpret_cast<char*>(&objCount), sizeof(int));

		// write each object
		pObjsToWrite->WalkList(WorldFileIO::WriteSingleObject, nullptr);

		// close file, indicate success
		outFile.close();
		return true;
	}

	bool WorldFileIO::ReadGobFile(const char * filePath, LinkedList<GraphicalObject*>* outGobs, unsigned shaderProgramID, ObjectInitializerCallback objInit, void *pClass)
	{
		// open the file, start at the beginning, error check
		inFile.open(filePath, std::ios::binary | std::ios::in);
		if (!inFile) { GameLogger::Log(MessageType::cError, "Failed to read file [%s]! Could not open file!\n", filePath); return false; }

		// read the format version
		int version = -1;
		inFile.read(reinterpret_cast<char*>(&version), sizeof(FORMAT_VERSION));

		// check that the version matches
		if (version != FORMAT_VERSION)
		{
			// if it is not the latest version, log an error and refuse to load the file
			GameLogger::Log(MessageType::cError, "FAILED TO READ IN WORLD FILE [%s]!!! WORLD FILE VERSION FOUND IN HEADER [%d] DOES NOT MATCH THE LATEST FILE VERSION [%d]!! PLEASE ENSURE YOU ARE USING THE LATEST VERSION OF WORLD FILE!\n", filePath, version, FORMAT_VERSION); 
			return false;
		}

		// read the obj count
		int objCount = 0; 
		inFile.read(reinterpret_cast<char*>(&objCount), sizeof(int));

		// read each obj in
		for (int i = 0; i < objCount; ++i)
		{
			char buffer[256]{ '\0' };

			// EXPECTS TO BE DELETED BY OUTSIDE CODE
			GraphicalObject *pGob = new GraphicalObject();

			// read in the transform
			inFile.read(reinterpret_cast<char*>(pGob->GetScaleMatPtr()), sizeof(Engine::Mat4));
			inFile.read(reinterpret_cast<char*>(pGob->GetRotMatPtr()), sizeof(Engine::Mat4));
			inFile.read(reinterpret_cast<char*>(pGob->GetTransMatPtr()), sizeof(Engine::Mat4));
			pGob->CalcFullTransform();

			// read in the mesh path length
			int len = 0;
			inFile.read(reinterpret_cast<char*>(&len), sizeof(int));

			// read in the mesh path
			inFile.read(&buffer[0], len);

			// make a gob out of it
			const char *str = StringFuncs::AddToCharArray(&buffer[0]);
			ShapeGenerator::ReadSceneFile(str, pGob, shaderProgramID);

			// call the callback to initialize the graphical object
			objInit(pGob, pClass);

			// add it to the list
			outGobs->AddToListFront(pGob);
		}
		
		// close file and indicate success
		inFile.close();
		return true;
	}

	bool WorldFileIO::WriteSingleObject(Engine::GraphicalObject * pObj, void *)
	{
		const char *meshPath = Engine::ShapeGenerator::GetPathForMesh(pObj->GetMeshPointer());

		// write out the transforms, in binary
		outFile.write(reinterpret_cast<char*>(pObj->GetScaleMatPtr()), sizeof(Engine::Mat4));
		outFile.write(reinterpret_cast<char*>(pObj->GetRotMatPtr()), sizeof(Engine::Mat4));
		outFile.write(reinterpret_cast<char*>(pObj->GetTransMatPtr()), sizeof(Engine::Mat4));

		// find length of path
		int len = StringFuncs::StringLen(meshPath);

		// write out mesh path length in binary
		outFile.write(reinterpret_cast<char*>(&len), sizeof(int));

		// write out the mesh path in binary
		outFile.write(meshPath, len);

		return true;
	}

}