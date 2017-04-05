#include <iostream>
#include "HashTable.h"

// Justin Furtado
// 4/4/2017
// main.cpp
// Starts the application

// Lets the user view the last output before exiting
void HoldOnEnd()
{
	// Display "Press Any Key To Continue..." and wait for a key to be pressed
	system("pause");
}

// Runs the application with valid arguments passed
int Run(const char *const fileName, unsigned int tableSize)
{
	HashTable dictionary;
	if (!dictionary.Initialize(tableSize)) { printf("Failed to Initalize(), cannot run!\n"); return false; }
	if (!dictionary.ProcessFile(fileName)) { printf("Failed to ProcessFile(), cannot run!\n"); return false; }

	// TODO: LOOP AND DO INPUT THING HERE

	return dictionary.Shutdown() ? EXIT_SUCCESS : EXIT_FAILURE;
}

// checks that the command line arguments exist
const int NUM_EXPECTED_ARGS = 3; // zeroth is path
bool ArgsExist(int argc, char *argv)
{
	// check count
	if (argc != NUM_EXPECTED_ARGS) { printf("Invalid Number of Arguments! Expected: [%d] Found: [%d]\n", NUM_EXPECTED_ARGS, argc); return false; }

	// check values
	if (!argv || !*argv) { printf("Invalid arg values!\n"); return false; }

	// indicate success
	return true;
}

// extract table size from args
bool GetTableSize(const char *str, unsigned int *outVal)
{
	// grab value, validate
	int val = atoi(str);
	if (val <= 0) { printf("Table size must be non-negative and non-zero! Found: [%d]\n", val); return false; }

	// set out value if valid, and indicate success
	*outVal = val;
	return true;
}

// walks to second string in arg
char *GetNextArg(char **argv)
{
	int i; for (i = 0; *(*argv + i); ++i) {}
	return *argv + i + 1;
}

// walks to next string in arg
char *GetNextArg(char *argv)
{
	int i; for (i = 0; *(argv + i); ++i) {}
	return argv + i + 1;
}

// checks if the arguments are valid, and if they are, runs the application
int ProcessArgs(int argc, char *argv)
{
	// exit if invalid args
	if (!ArgsExist(argc, argv)) { return EXIT_FAILURE; }

	// grab the fileName
	const char *fileName = argv;

	// grab the table size
	unsigned int tableSize = 0;
	if (!GetTableSize(GetNextArg(argv), &tableSize)) { printf("Unable to extract table size from args!\n"); return EXIT_FAILURE; }

	// run, args are valid and of the correct type
	return Run(fileName, tableSize);
}

// args: fileName tableSize
int main(int argc, char **argv)
{
	// grab the result
	int result = ProcessArgs(argc, GetNextArg(argv));

	// wait for user to press a key
	HoldOnEnd();

	// exit the application
	return result;
}