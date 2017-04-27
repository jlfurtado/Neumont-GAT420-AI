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

	bool WorldFileIO::WriteFile(LinkedList<GraphicalObject>* pObjsToWrite, const char * filePath)
	{
		// open the file, start at the beginning, error check
		outFile.open(filePath, std::ios::binary | std::ios::out);
		if (!outFile) { GameLogger::Log(MessageType::cError, "Failed to write file [%s]! Could not open file!\n", filePath); return false; }
		outFile.seekp(0);

		// write the obj count
		int objCount = pObjsToWrite->GetCount();
		outFile.write(reinterpret_cast<char*>(&objCount), sizeof(int));

		// write each object
		pObjsToWrite->WalkList(WorldFileIO::WriteSingleObject, nullptr);

		// close file, indicate success
		outFile.close();
		return true;
	}

	bool WorldFileIO::ReadFile(const char * filePath, LinkedList<GraphicalObject>* outGobs, unsigned shaderProgramID, ObjectInitializerCallback objInit, void *pClass)
	{
		// open the file, start at the beginning, error check
		inFile.open(filePath, std::ios::binary | std::ios::in);
		if (!inFile) { GameLogger::Log(MessageType::cError, "Failed to read file [%s]! Could not open file!\n", filePath); return false; }

		// read the obj count
		int objCount = 0; 
		inFile.read(reinterpret_cast<char*>(&objCount), sizeof(int));

		// read each obj in
		for (int i = 0; i < objCount; ++i)
		{
			char buffer[256]{ '\0' };

			// TODO CHECK LEAK!?!?! MAYBE FINE!?!?
			GraphicalObject *pGob = new GraphicalObject();

			// read in the transform
			inFile.read(reinterpret_cast<char*>(pGob->GetFullTransformPtr()), sizeof(*pGob->GetFullTransformPtr()));

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
			outGobs->AddToList(pGob);
		}
		
		// close file and indicate success
		inFile.close();
		return true;
	}

	bool WorldFileIO::WriteSingleObject(Engine::GraphicalObject * pObj, void *)
	{
		const char *meshPath = Engine::ShapeGenerator::GetPathForMesh(pObj->GetMeshPointer());

		// write out the transform, in binary
		outFile.write(reinterpret_cast<char*>(pObj->GetFullTransformPtr()), sizeof(*pObj->GetFullTransformPtr()));

		// find length of path
		int len = StringFuncs::StringLen(meshPath);

		// write out mesh path length in binary
		outFile.write(reinterpret_cast<char*>(&len), sizeof(int));

		// write out the mesh path in binary
		outFile.write(meshPath, len);

		return true;
	}

}