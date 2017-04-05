#ifndef WORDNODE_H
#define WORDNODE_H

// Justin Furtado
// 4/4/2017
// WordNode.h
// Represents a node with a word for the linked lists for this assignment

class WordNode
{
public:
	WordNode();
	WordNode(WordNode *pNext, const char *pWord);
	WordNode *GetNextNode();
	void PointTo(WordNode *pNext);
	const char *GetWord() const;

private:
	const char *m_pWord;
	WordNode *m_pNextNode;
};

#endif // ifndef WORDNODE_H