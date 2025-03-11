#ifndef BSTSET_INCLUDED
#define BSTSET_INCLUDED

#include<iostream>

// to insert a new item
// to find an item with a specific value
// to iterate through in ascending order
// to find an item not smaller than a specifc value -> find departure time?

template<typename T>
class BSTSet
{
private:
	struct Node
	{
		T m_value;
		Node* left;
		Node* right;
		Node* parent;
		Node(const T& val) :m_value(val), left(nullptr), right(nullptr), parent(nullptr) {}
	};

	Node* m_root;
	void clear(Node* n)
	{
		if (n == nullptr)
			return;
		clear(n->left);
		clear(n->right);
		delete n;
	}

public:
	BSTSet() : m_root(nullptr) {}
	~BSTSet() { clear(m_root); }

	class SetIterator
	{
	public:
		SetIterator(Node* current = nullptr) :m_cur(current) {}
		const T* get_and_advance()
		{
			if (m_cur == nullptr) return nullptr;
			const T* node = &(m_cur->m_value);
			if (m_cur->right != nullptr) // if there is a right subtree
			{
				m_cur = m_cur->right;
				while (m_cur->left != nullptr)
					m_cur = m_cur->left; // find the smallest element in that right subtree
			}
			else // if there's not a right subtree
			{
				Node* par = m_cur->parent; // move up to the parent node
				while (par != nullptr && par->right == m_cur) // move to the first parent where 
				{
					m_cur = par;
					par = par->parent;
				}
				m_cur = par;
			}
			return node;
		}

	private:
		Node* m_cur;
	};

	void insert(const T& value)
	{
		if (m_root == nullptr)
		{
			m_root = new Node(value);
			return;
		}
		Node* cur = m_root;

		while (true)
		{
			if (value == cur->m_value)
			{
				cur->m_value = value;
				return;
			}

			Node* nextNode = nullptr;
			if (value < cur->m_value)
			{
				nextNode = cur->left;
			}
			else
			{
				nextNode = cur->right;
			}

			if (nextNode == nullptr)
			{
				Node* nn = new Node(value);
				nn->parent = cur;
				if (value < cur->m_value)
					cur->left = nn;
				else
					cur->right = nn;
				return;
			}
			cur = nextNode;
		}
	}

	SetIterator find(const T& value) const
	{
		Node* cur = m_root;
		while (cur != nullptr)
		{
			if (value == cur->m_value)
			{
				return SetIterator(cur);
			}
			else if (value < cur->m_value)
			{
				cur = cur->left;
			}
			else if (value > cur->m_value)
			{
				cur = cur->right;
			}
		}
		return SetIterator();
	}

	SetIterator find_first_not_smaller(const T& value) const
	{
		Node* cur = m_root;
		Node* prev = nullptr;
		while (cur != nullptr)
		{
			if (value == cur->m_value)
				return SetIterator(cur);
			else if (value < cur->m_value)
			{
				prev = cur;
				cur = cur->left;
			}
			else if (cur->m_value < value)
			{
				cur = cur->right;
			}
		}
		return SetIterator(prev);
	}

};

#endif