#include "WorldFileIO.h"
#include "StringFuncs.h"

// Justin Furtado
// 4/24/2017
// WorldFileIO.h
// Handles input and output for world files

namespace Engine
{
	int WorldFileIO::s_allocatedGobCount = 0;
	LinkedList<GraphicalObject> WorldFileIO::s_allocatedGobs;

	bool WorldFileIO::Initialize()
	{
		// nothing to initialize... YET!?!?
		return true;
	}

	bool WorldFileIO::Shutdown()
	{
		return CleanUp();
	}

	const int MAX_LINE = 250;
	GraphicalObject * WorldFileIO::ReadGobs(const char * const filePath)
	{
		std::ifstream inputFile;

		inputFile.open(filePath);

		if (!inputFile.is_open())
		{
			GameLogger::Log(MessageType::cError, "Failed to ReadGobs! File stream failed to open!\n");
			return false;
		}

		int objCount;


		// parse file for count - bare minimum, just count begin commands

		// allocate array objects

		// add array elements to linked list

		// update counts and what not
		
		// return address of allocated array
		return nullptr;
	}

	bool WorldFileIO::WriteGobs(GraphicalObject * pGobs, int numGobs)
	{
		// for each gob

		// write begin command
		// dump info vecs
		// file close
		return false;
	}

	const char *const GOB_COUNT_PREFIX = "NUM GOBS: ";
	const int GOB_COUNT_LENGTH = 10; // TODO IF BROKEN CHECK CONST
	bool WorldFileIO::GetGobCount(std::ifstream& inputFile, int *outCount)
	{
		int count = -1;
		for (int line = 0; !inputFile.eof(); ++line)
		{
			// grab the line, up to MAX_LINE chars
			char buffer[MAX_LINE]{ '\0' };
			inputFile.getline(&buffer[0], MAX_LINE);

			// find out prefix in the substring
			int index = StringFuncs::FindSubString(buffer, GOB_COUNT_PREFIX);
	
			// start already in line, we don't need to check before the substring is found
			for (int offset = index + GOB_COUNT_LENGTH;  offset < MAX_LINE && *(buffer + offset) && index >= 0; ++offset)
			{
				if (StringFuncs::IsDigit(*(buffer + offset)))
				{
					// found a bad number
					if (!StringFuncs::GetSingleIntFromString(buffer + offset, count))
					{
						GameLogger::Log(MessageType::cError, "Failed to read file! Could not get gob count! Failed to parse line [%d] [%s]\n", line + 1, buffer);
						return false;
					}
					else
					{
						// only set out value if successful
						*outCount = count;
						return true;
					}
				}
			}
		}

		// eof with no count found
		GameLogger::Log(MessageType::cError, "Failed to GetGobCount! End of file reached with no count found!\n");
		return false;
	}

	bool WorldFileIO::DeleteObjectCallback(GraphicalObject * pGob, void * /*pClass*/)
	{
		// static so ignore class ptr - refactor later
		
		// TODO: remove from render engine/collision tester here!?!?!

		s_allocatedGobCount--;
		delete pGob;

		return true;
	}

	bool WorldFileIO::CleanUp()
	{
		// walk the list and delete the objs
		if (!s_allocatedGobs.WalkList(WorldFileIO::DeleteObjectCallback, nullptr))
		{
			GameLogger::Log(MessageType::cFatal_Error, "Failed to CleanUp WorldFileIO Allocated GOBS! Failed to walk list!\n");
			return false;
		}

		// error check
		if (s_allocatedGobCount != 0)
		{
			GameLogger::Log(MessageType::cError, "Failed to CleanUp WorldFileIO Allocated Objects Count is invalid or objects were not deleted! Count is [%d]!\n", s_allocatedGobCount);
			return false;
		}

		// successfully deleted all objects
		return true;
	}

}