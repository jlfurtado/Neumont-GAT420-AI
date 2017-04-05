#include "HashTable.h"
#include "WordList.h"
#include <iostream>

// Justin Furtado
// 4/4/2017
// HashTable.h
// Defines a hash table for a dictionary of words

const int MAX_CHARS = 1000000;
char words[MAX_CHARS]{ 0 };

// processes the specified file by opening it, reading from it, then closing it
bool HashTable::ProcessFile(const char *fileName)
{
	if (!OpenFile(fileName)) { printf("Could not ProcessFile() because OpenFile() failed!\n"); return false; }
	if (!ReadFile()) { printf("Could not ProcessFile() because ReadFile() failed!\n"); return false; }
	if (!CloseFile()) { printf("Could not ProcessFile() because CloseFile() failed!\n"); return false; }
	return true;
}

// initializes the table by making the lists of the appropriate size
bool HashTable::Initialize(unsigned int tableSize)
{
	if (!MakeLists(tableSize)) { printf("Failed to Initalize()! Could not make lists for table size [%d]!\n", tableSize); return false; }
	return true;
}

// shuts down the table by deleting the lists
bool HashTable::Shutdown()
{
	if (!DeleteLists()) { printf("Failed to delete lists!\n"); return false; }
	return true;
}

// TODO: CHECK THIS
bool HashTable::InsertWord(const char *str)
{
	return ProcessPossibleWord(str);
}

// TODO: IMPLEMENT
bool HashTable::ContainsWord(const char *str)
{
	return false;
}

// returns the total number of words successfully added to the table
int HashTable::NumberOfWords()
{
	return m_insertedWords;
}

// returns the number of words that were rejected because they already exist within the table
int HashTable::NumberOfDuplicates()
{
	return m_rejectedWords;
}

// returns the number of words that were rejected because they did not fit the provided description for words
int HashTable::NumberOfNonWords()
{
	return m_nonWordsFound;
}

// TODO: IMPLEMENT
bool HashTable::DisplayStats()
{
	return false;
}

// opens the file specified
bool HashTable::OpenFile(const char * fileName)
{
	// attempt to open the file 
	m_inputFileStream.open(fileName);

	// check to see if it actially opened
	if (!m_inputFileStream.is_open()) { printf("Failed to open file stream!\n"); return false; }

	// ensure (possibly redundantly) that we begin at the beginning of the file and indicate success
	m_inputFileStream.clear();
	m_inputFileStream.seekg(0, std::ios::beg);

	// indicate success 
	return true;
}

// TODO: IMPLEMENT
bool HashTable::ReadFile()
{
	return false;
}

// Closes the file opened
bool HashTable::CloseFile()
{
	// Attempt to close the file
	m_inputFileStream.close();

	// check if the stream is still open
	if (m_inputFileStream.is_open()) { printf("Failed to close file stream!\n"); return false; }
	return true;
}

// iterates through the line of text, splitting into words and processing the words
bool HashTable::ProcessLine(const char * line)
{
	// iterate through each character
	for (int i = 0; !IsEndOfLine(*(line + i)); ++i)
	{
		char first = i == 0 ? ' ' : *(line + i - 1);
		char second = *(line + i);

		// if it is a word boundary, process the word, if it fails, log an error
		if (IsWordStartBoundary(first, second) && !ProcessPossibleWord(line + i)) { printf("Failed to process word [%s]!\n", line + i); return false; }
	}

	// if we made it through the whole line and processed all the words, the line has been processed successfully
	return true;
}

// processes the strings which may or may not be words
bool HashTable::ProcessPossibleWord(const char * word)
{
	// update counts and add word if necessary, log error if fail
	if (!IsWord(word)) { m_nonWordsFound++; }
	else if (ContainsWord(word)) { m_rejectedWords++; }
	else if (AddWord(word)) { m_insertedWords++; }
	else { printf("Failed to add word [%s]!\n", word); return false; }

	return true;
}

// checks if a string is a word
bool HashTable::IsWord(const char * str)
{
	// iterate through the characters in the string, until whitespace is found
	for (int i = 0; !IsWhitespace(*(str + i)); ++i)
	{
		// if any characters prior to whitespace are non-word characters, its not a word
		if (!IsWordChar(*(str + i), i == 0)) { return false; }
	}
	
	// if we make it to the end of the word with no bad characters found, its a word for the purposes of this assignment
	return true;
}

// initializes the array of linked lists
bool HashTable::MakeLists(unsigned int tableSize)
{
	// attempt to allocate memory and check for errors
	m_pWordLists = new WordList[tableSize];
	if (!m_pWordLists) { printf("Failed to MakeLists()! Could not allocate memory for lists!\n"); return false; }

	// store table size and return success
	m_tableSize = tableSize;
	return true;
}

// releases memory allocated for the words lists, if necessary
bool HashTable::DeleteLists()
{
	bool shouldDelete = (m_pWordLists != nullptr);
	if (shouldDelete) { delete[] m_pWordLists; }
	return shouldDelete;
}

// TODO: IMPLEMENT
bool HashTable::AddWord(const char * str)
{
	return false;
}

// returns whether or not a character is a valid delemiter between words
bool HashTable::IsWhitespace(char c)
{
	// space, tab, end of line and blank char are whitespace for this
	return c == ' ' || c == '\t' || IsEndOfLine(c) || c == ' ';
}

//returns if a character is a valid portion of a word
bool HashTable::IsWordChar(char c, bool first)
{
	// a-z valid, A-Z valid, apostrophe valid if its not the first character
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (!first && c == '\'');
}

// returns if the line is over - maybe a redundant check in here but whatever
bool HashTable::IsEndOfLine(char c)
{
	return c == '\n' || c == '\0';
}

// this is the start of a word if whitespace precedes a word char
bool HashTable::IsWordStartBoundary(char before, char current)
{
	return IsWhitespace(before) && IsWordChar(current, true);
}
