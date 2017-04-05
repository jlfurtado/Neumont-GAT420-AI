#include "HashTable.h"
#include "WordList.h"
#include "StaticCharArray.h"
#include <iostream>

// Justin Furtado
// 4/4/2017
// HashTable.h
// Defines a hash table for a dictionary of words

const int MAX_LINE_LENGTH = 5000; // TODO: CHECK CONSTANT

// processes the specified file by opening it, reading from it, then closing it
bool HashTable::ProcessFile(const char *fileName)
{
	if (!OpenFile(fileName)) { printf("Could not ProcessFile() because OpenFile() failed!\n"); return false; }
	if (!ReadFile()) { printf("Could not ProcessFile() because ReadFile() failed!\n"); return false; }
	if (!CloseFile()) { printf("Could not ProcessFile() because CloseFile() failed!\n"); return false; }
	return true;
}

// initializes the table by making the lists of the appropriate size
bool HashTable::Initialize(unsigned int tableSize, HashCallback hashCallback)
{
	// reset values
	m_insertedWords = 0;
	m_rejectedWords = 0;
	m_nonWordsFound = 0;
	m_tableSize = 0;
	m_pWordLists = nullptr;

	if (!MakeLists(tableSize)) { printf("Failed to Initalize()! Could not make lists for table size [%d]!\n", tableSize); return false; }
	m_hashCallback = hashCallback;
	return true;
}

// shuts down the table by deleting the lists
bool HashTable::Shutdown()
{
	if (!DeleteLists()) { printf("Failed to delete lists!\n"); return false; }
	return true;
}

// Inserts a word, returns true if inserted, false otherwise
bool HashTable::InsertWord(const char *str)
{
	// update counts and add word if necessary, log error if fail
	if (!IsWord(str)) { m_nonWordsFound++; printf("[%s] is not a valid word!\n", str); return false; }
	else if (ContainsWord(str)) { m_rejectedWords++; printf("[%s] is already inside table!\n", str); return false; }
	else if (AddWord(str)) { m_insertedWords++; printf("[%s] inserted successfully!\n", str); return true; }
	else { printf("Failed to add word [%s]!\n", str); return false; }
}

// Checks if the word exists within the database by searching the list the lowercased string hashes to
bool HashTable::ContainsWord(const char *str)
{
	// get lowercase of str
	char wordLowercase[MAX_LINE_LENGTH]{ 0 };
	for (int i = 0; !IsWhitespace(*(str + i)); ++i) { wordLowercase[i] = ToLowerCase(*(str + i)); }

	// get index for list
	int index = GetIndexFromHash(&wordLowercase[0]);
	return m_pWordLists[index].ContainsWord(wordLowercase);
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

//  Do some simple stats 
bool HashTable::DisplayStats()
{
	// make vars
	int min = m_pWordLists[0].GetNodeCount(), minCount = 0;
	int max = m_pWordLists[0].GetNodeCount(), maxCount = 0;
	float idealWordsPerEntry = m_insertedWords * 1.0f / m_tableSize;
	float averageDistance = 0.0f;
	float standardDeviation = 0.0f;

	// iterate through all lists
	for (int i = 0; i < m_tableSize; ++i)
	{
		// grab the number of nodes in the list
		int count = m_pWordLists[i].GetNodeCount();

		// update minimum and minimum count accordingly
		if (count < min) { min = count; minCount = 1; }
		else if (min == count) { minCount++; }

		// update maximum and maximum count accordingly
		if (count > max) { max = count; maxCount = 1; }
		else if (max == count) { maxCount++; }

		// sum absolute difference
		averageDistance += fabsf(count - idealWordsPerEntry);

		// sum squares for deviations
		standardDeviation += (count - idealWordsPerEntry) * (count - idealWordsPerEntry);
	}

	// convert sums to final values
	averageDistance /= m_tableSize;
	standardDeviation = sqrtf(standardDeviation / m_tableSize);

	// dump the info
	printf("Number of table Entries: [%d]\n", m_tableSize);
	printf("Number of total Words: [%d]\n", m_insertedWords);
	printf("Minimum entry size: [%d], number of entries with this size: [%d]\n", min, minCount);
	printf("Maximum entry size: [%d], number of entries with this size: [%d]\n", max, maxCount);
	printf("Ideal words per entry: [%.3f]\n", idealWordsPerEntry);
	printf("Weird Average thing you asked for: [%.3f]\n", averageDistance);
	printf("Standard deviation: [%.3f]\n", standardDeviation);

	return true;
}

// gets an index from the hash
int HashTable::GetIndexFromHash(const char * str)
{
	return m_hashCallback(str) % m_tableSize;
}

// Adds a word to the list of unique words
bool HashTable::AddUniqueWord(const char * word, const char **outLowercasedWordPtr)
{
	// set out variable to beginning of word
	*outLowercasedWordPtr = StaticCharArray::GetNextStringPtr();

	// add the word to the end, copying it character by character and lower-casing it
	for (int i = 0; !IsWhitespace(*(word + i)); ++i)
	{
		char c = *(word + i);
		StaticCharArray::AddChar(ToLowerCase(c));
	}
	
	// null-terminate the string (possibly redundantly)
	StaticCharArray::AddChar('\0');

	// indicate success
	return true;
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

bool HashTable::ReadFile()
{
	// array of characters for the current line
	char currentLine[MAX_LINE_LENGTH]{ 0 };

	// iterate through the file, until it has ended, maintaining a counter for the current line number
	for (int line = 1; !m_inputFileStream.eof(); ++line)
	{
		// grab the current line
		m_inputFileStream.getline(&currentLine[0], MAX_LINE_LENGTH);

		// read the line, if it fails, consider the whole file a failure
		if (!ProcessLine(&currentLine[0])) { printf("Failed to read file! Failed to process line [%d]!\n", line); return false; }
	}

	// no lines failed, end of file reached, all good
	return true;
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
		// grab first two chars, adjust for first word being a boundary regardless manually
		char first = i == 0 ? ' ' : *(line + i - 1);
		char second = *(line + i);

		// if it is a word boundary, process the word, if it fails, log an error
		if (IsWordStartBoundary(first, second) && !ProcessPossibleWord(line + i)) { printf("Failed to process word [%s]!\n", line + i); return false; }
	}

	// if we made it through the whole line and processed all the words, the line has been processed successfully
	return true;
}

// processes the strings which may or may not be words, returns false only in case of error
bool HashTable::ProcessPossibleWord(const char * word)
{
	// update counts and add word if necessary, log error if fail
	if (!IsWord(word)) { m_nonWordsFound++; }
	else if (ContainsWord(word)) { m_rejectedWords++; }
	else if (AddWord(word)) { m_insertedWords++; }
	else { printf("Failed to add word [%s]!\n", word); return false; }

	// success
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

// Adds a word to the appropriate linked list and unique words list
bool HashTable::AddWord(const char * str)
{
	// add the word, grabbing a pointer to the stored lower-cased copy
	const char *strLower;
	if (!AddUniqueWord(str, &strLower)) { printf("Could not add word! Failed to AddUniqueWord()!\n"); return false; }

	// hash for index, and add to that linked list, return success/failure
	int index = GetIndexFromHash(strLower);
	return m_pWordLists[index].AddToFront(strLower);
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

// returns the lowercase equivalent of a character
char HashTable::ToLowerCase(char c)
{
	// if its a letter, move it over 32, otherwise, leave it
	return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : (c);
}
