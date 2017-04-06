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

// Counts characters in a string by iterating through them until '\0' is encountered
int WordNode::StringLength(const char *str) const
{
	if (!str) return 0;

	int pos = 0;
	while (*(str + pos)) { pos++; }
	return pos;
}

// Compares two strings by first comparing their lengths and, if they are of the same length, comparing each character in the string to the character at the same index in the other string.
// If any characters are not equal, the strings are not equal
bool WordNode::WordsAreEqual(const char * str) const
{
	// handle edge cases
	if (str == m_pWord || !str || !m_pWord) return str == m_pWord;

	// if lengths are not the same, they cannot be equal
	if (StringLength(str) != StringLength(m_pWord)) return false;

	// compare chars
	for (int pos = 0; *(m_pWord + pos) && *(m_pWord + pos); ++pos)
	{
		if (*(str + pos) != *(m_pWord + pos)) return false; // not equal
	}

	// equal
	return true;
}
