#include "WordList.h"
#include "WordNode.h"
#include <iostream>

// Justin Furtado
// 4/4/2017
// WordList.cpp
// Implements a linked list of WordNodes for the assignment

// initialize data
WordList::WordList()
	: m_pHeadNode(nullptr), m_nodeCount(0)
{
}

// clean up allocated data
WordList::~WordList()
{
	// if the list is not empty
	if (m_pHeadNode)
	{
		// make some variables
		WordNode *lastNode;
		WordNode *currentNode = m_pHeadNode->GetNextNode();
		delete m_pHeadNode; // release memory for first node
		int removed = 1;

		// release memory for the following nodes
		while (currentNode != nullptr)
		{
			lastNode = currentNode;
			currentNode = currentNode->GetNextNode();
			delete lastNode;
			removed++;
		}

		// log if something went wrong - don't want any memory leaks
		if (removed != m_nodeCount) { printf("PROGRAMMER ERROR: Deleted [%d] of [%d] nodes!\n", removed, m_nodeCount); }
	}
}

// its really fast to add to the front of the list
bool WordList::AddToFront(const char * str)
{
	m_pHeadNode = new WordNode(m_pHeadNode, str);
	m_nodeCount++;
	return true;
}

int WordList::GetNodeCount() const
{
	return m_nodeCount;
}
