#ifndef WORDLIST_H
#define WORDLIST_H

// Justin Furtado
// 4/4/2017
// WordList.h
// Implements a linked list of WordNodes for the assignment

class WordNode;
class WordList
{
public:
	WordList();
	~WordList();
	bool AddToFront(const char *str);
	int GetNodeCount() const;

private:
	WordNode *m_pHeadNode;
	int m_nodeCount;
};

#endif // ifndef WORDLIST_H