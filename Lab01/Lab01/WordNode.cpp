#include "WordNode.h"

// Justin Furtado
// 4/4/2017
// WordNode.h
// Represents a node with a word for the linked lists for this assignment

WordNode::WordNode()
	: m_pWord(nullptr), m_pNextNode(nullptr)
{
}

WordNode::WordNode(WordNode * pNext, const char * pWord)
	: m_pWord(pWord), m_pNextNode(pNext)
{
}

WordNode * WordNode::GetNextNode()
{
	return m_pNextNode;
}

void WordNode::PointTo(WordNode * pNext)
{
	m_pNextNode = pNext;
}

const char * WordNode::GetWord() const
{
	return m_pWord;
}
