#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <fstream>

// Justin Furtado
// 4/4/2017
// HashTable.h
// Defines a hash table for a dictionary of words

class WordList;
class HashTable
{
public:
	bool Initialize(unsigned int tableSize);
	bool Shutdown();
	bool ProcessFile(const char *fileName);
	bool InsertWord(const char *str);
	bool ContainsWord(const char *str);
	int NumberOfWords();
	int NumberOfDuplicates();
	int NumberOfNonWords();
	bool DisplayStats();

private:
	bool OpenFile(const char *fileName);
	bool ReadFile();
	bool CloseFile();
	bool ProcessLine(const char *line);
	bool ProcessPossibleWord(const char *word);
	bool IsWord(const char *str);
	bool MakeLists(unsigned int tableSize);
	bool DeleteLists();
	bool AddWord(const char *str);
	bool IsWhitespace(char c);
	bool IsWordChar(char c, bool first);
	bool IsEndOfLine(char c);
	bool IsWordStartBoundary(char before, char current);
	int m_insertedWords{ 0 };
	int m_rejectedWords{ 0 };
	int m_nonWordsFound{ 0 };
	int m_tableSize{ 0 };
	WordList *m_pWordLists{ nullptr };
	std::ifstream m_inputFileStream;
};

#endif // ifndef HASHTABLE_H