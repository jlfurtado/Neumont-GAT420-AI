#ifndef LINKEDLIST_H
#define LINKEDLIST_H

// Justin Furtado
// 4/13/2017
// LinkedList.h
// Defines a linked list for a generic linked list

#include "ExportHeader.h"
#include "GameLogger.h"

namespace Engine
{
	template<class T>
	class ENGINE_SHARED LinkedList
	{
		struct Node
		{
			T m_data;
			Node *m_pNextNode;
			Node(T data, Node *pNext) : m_data(data), m_pNextNode(pNext) {}
		};

	public:
		typedef bool(*LinkedListIterationCallback)(T data, void *pClassInstance);

		LinkedList() : m_pHeadNode(nullptr), m_nodeCount(0) { }
		~LinkedList() {	ClearList(); }

		bool Contains(T data) const
		{
			// start at the beginning of the list
			Node *pCurrent = m_pHeadNode;

			// iterate through all of the nodes
			while (pCurrent != nullptr)
			{
				// if they point to the same thing, its in there
				if (pCurrent->m_data == data) { return true; }
				pCurrent = pCurrent->m_pNextNode;
			}

			// data not contained in linked list
			return false;
		}

		T GetFirstObjectData()
		{
			// returns the data of the first object, nullptr if empty list
			return m_pHeadNode ? m_pHeadNode->m_data : T();
		}

		void AddToListFront(T data)
		{
			// add node to the front of list - its at the front and it points to the node that used to be at the front, nullptr if only node
			m_pHeadNode = new Node(data, m_pHeadNode);

			// there is now one more node
			m_nodeCount++;
		}

		// used to sort the list at time of addition, adds to end if callback never returns true
		void AddToListWhere(T data, LinkedListIterationCallback goesBefore, void *pClassInstance)
		{
			// calls a method to determine when to add it, adds node at locaction where callback firstreturns true;
 			if (!m_pHeadNode) { AddToListFront(data); return; /*count updated in method*/ } // if empty list make it the head

			// if belongs before head node, make new head node
			if (goesBefore(m_pHeadNode->m_data, pClassInstance)) { AddToListFront(data); return; /*count updated in method*/ }

			// handle the rest
			Node *pCurrentNode = m_pHeadNode;
			Node *pNextNode = pCurrentNode->m_pNextNode;

			// while there is a next node
			while (pNextNode)
			{
				// if we need to insert between these two nodes stop
				if (goesBefore(pNextNode->m_data, pClassInstance)) { break; }

				// otherwise keep moving forward
				pCurrentNode = pCurrentNode->m_pNextNode;
				pNextNode = pCurrentNode->m_pNextNode;
			}

			// we have the two nodes to insert between (if end, next is just nullptr, which makes the new one go at the end, exacly like we want
			pCurrentNode->m_pNextNode = new Node(data, pNextNode);
			m_nodeCount++; // don't forget to update the count
		}

		T GetWhere(LinkedListIterationCallback callback, void *pClassInstance)
		{
			// grab the first node in the list
			Node *pCurrentNode = m_pHeadNode;

			// while there are nodes, call the callback, stopping if indicated by return value, otherwise moving to the next node
			while (pCurrentNode)
			{
				if (callback(pCurrentNode->m_data, pClassInstance)) { return pCurrentNode->m_data; } // found a match, return it
				pCurrentNode = pCurrentNode->m_pNextNode;
			}

			// return value indicating not present in list
			return nullptr;
		}

		void RemoveWhere(LinkedListIterationCallback callback, void *pClassInstance)
		{
			// can't remove from empty list
			if (!m_pHeadNode) { return; }

			// if we need to remove the head node(s), handle that case
			while (callback(m_pHeadNode->m_data, pClassInstance)) { RemoveHeadNode(); }

			// grab the first node in the list
			Node *pLastNode = m_pHeadNode;
			Node *pCurrentNode = pLastNode->m_pNextNode;

			// while there are nodes, call the callback, acting if indicated by return value, otherwise moving to the next node
			while (pCurrentNode)
			{
				// see if we should remove the current node
				if (callback(pCurrentNode->m_data, pClassInstance)) { RemoveSingleNode(pLastNode, pCurrentNode); } // found a match, remove it

				// keep track of current and last node for removing purposes
				pLastNode = pLastNode->m_pNextNode;
				pCurrentNode = pLastNode->m_pNextNode;
			} 

		}

		void RemoveFirstFromList(T data)
		{
			// can't remove from empty list
			if (!m_pHeadNode) { return; }

			// in case the one we want to remove is the head
			if (m_pHeadNode->m_data == data) { RemoveHeadNode(); return; }

			// if we don't want to remove the head, we need two nodes, so that we can point the node before it at the correct thing
			// grab first two nodes, second is nullptr if only one node
			Node *pLastNode = m_pHeadNode;
			Node *pCurrentNode = pLastNode->m_pNextNode;

			while (pCurrentNode) // keep checking while there's nodes
			{
				// check if the second node is the node we want to remove
				if (pCurrentNode->m_data == data)
				{
					// if it is, remove it
					RemoveSingleNode(pLastNode, pCurrentNode);
					break;
				}

				// move over to the next node
				pLastNode = pLastNode->m_pNextNode;
				pCurrentNode = pLastNode->m_pNextNode;
			}  
		}

		bool WalkListWhere(LinkedListIterationCallback considerNodeCallback, void *pConsiderData, LinkedListIterationCallback callback, void *pClassInstance)
		{
			// grab the first node in the list
			Node *pCurrentNode = m_pHeadNode;

			// while there are nodes, call the callback, stopping if indicated by return value, otherwise moving to the next node
			while (pCurrentNode)
			{
				Node *pNext = pCurrentNode->m_pNextNode; // support for removing while iterating???
				if (considerNodeCallback(pCurrentNode->m_data, pConsiderData) && !callback(pCurrentNode->m_data, pClassInstance)) { return false; } // Callback can return false to stop iterating list - like if we only wanted to walk part of it
				pCurrentNode = pNext;
			}

			return true;
		}

		bool WalkList(LinkedListIterationCallback callback, void *pClassInstance)
		{
			// grab the first node in the list
			Node *pCurrentNode = m_pHeadNode;

			// while there are nodes, call the callback, stopping if indicated by return value, otherwise moving to the next node
			while (pCurrentNode)
			{
				Node *pNext = pCurrentNode->m_pNextNode; // support for removing while iterating???
				if (!callback(pCurrentNode->m_data, pClassInstance)) { return false; } // Callback can return false to stop iterating list - like if we only wanted to walk part of it
				pCurrentNode = pNext;
			}
			
			return true;
		}

		void ClearList()
		{
			// to clear the list, lets just remove the head node over and over...
			while (m_pHeadNode) { RemoveHeadNode(); }

			// check to see if we removed the right number of nodes
			if (m_nodeCount != 0) { GameLogger::Log(MessageType::cError, "Failed to clear Linked List! Node count [%d]\n!", m_nodeCount); }
		}

		unsigned int GetCount() const
		{
			return m_nodeCount;
		}

		unsigned int GetCountWhere(LinkedListIterationCallback callback, void *pInstance)
		{
			// handle empty list
			if (!m_pHeadNode) { return 0; }

			unsigned int count = 0;

			// grab the first node in the list
			Node *pLastNode = m_pHeadNode;
			Node *pCurrentNode = pLastNode->m_pNextNode;

			// if the head node is included, increment counter from 0
			if (callback(m_pHeadNode->m_data, pInstance)) { ++count; }

			// while there are nodes, call the callback, acting if indicated by return value, otherwise moving to the next node
			while (pCurrentNode)
			{
				// see if we should remove the current node
				if (callback(pCurrentNode->m_data, pInstance)) { ++count; } // found a match, remove it

				// keep track of current and last node for removing purposes
				pLastNode = pLastNode->m_pNextNode;
				pCurrentNode = pLastNode->m_pNextNode;
			}

			// return the count
			return count;
		}

		void RemoveHeadNode()
		{
			// only remove the head node if it exists
			if (m_pHeadNode)
			{
				// get a pointer to the new head node
				Node *pNextHead = m_pHeadNode->m_pNextNode;

				// release memory allocated for the last head node
				delete m_pHeadNode;

				// set the pointer to the head node to the new head node
				m_pHeadNode = pNextHead;

				// there is now one less node in the list
				m_nodeCount--;
			}
		}

	private:
		void RemoveSingleNode(Node *pBefore, Node *pNodeToRemove)
		{
			// To remove a node (that is not the head node), we need to point the node before it to the node after it
			pBefore->m_pNextNode = pNodeToRemove->m_pNextNode;

			// then we need to release the memory we allocated for the node
			delete pNodeToRemove;

			// and finally we need to update the node count
			m_nodeCount--;
		}

		Node *m_pHeadNode;
		unsigned int m_nodeCount;
	};
}


#endif // ifndef LINKEDLIST_H