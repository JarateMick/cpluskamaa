#pragma once

template<class T>
class SListNode
{
public:
	T _data;
	SListNode<T>* _next;

	void InsertAfter(T data)
	{
		SListNode<T>* newNode = new SListNode<T>;

		newNode->_data = data;
		newNode->_next = _next;

		_next = newNode;
	}

};

template<class T>
class SLinkedList;

template<class T>
class SListIterator
{
public:
	SListNode<T>*     node;
	SLinkedList<T>*   list;

	SListIterator(SLinkedList<T>* list = 0, SListNode<T>* node = 0)
	{
		this->list = list;
		this->node = node;
	}

	void Start()
	{
		if (list != 0)
			this->node = list->head;
	}

	void Forth()
	{
		if (node != 0)
			node = node->_next;
	}

	T& Item()
	{
		return node->_data;
	}

	bool valid()
	{
		return (node != 0);
	}
};

template<class T>
class SLinkedList
{
public:
	SListNode<T>* head;
	SListNode<T>* tail;
	int count;

	SLinkedList()
	{
		head = 0;
		tail = 0;
		count = 0;
	}

	~SLinkedList()
	{
		SListNode<T>* itr = head;
		SListNode<T>* next;

		while (itr != 0)
		{
			next = itr->_next;

			delete itr;
			itr = next;
		}
	}

	void Append(T data)
	{
		if (head == 0)
		{
			head = tail = new SListNode<T>;
			head->_data = data;
		}
		else
		{
			tail->InsertAfter(data);
			tail = tail->_next;
		}
		count++;
	}

	void Prepend(T data)
	{
		SListNode<T>* newNode = new SListNode<T>;
		newNode->_data = data;
		newNode->_next = head;

		head = newNode;
		if (tail == 0)
			tail = head;

		count++;
	}

	void RemoveHead()
	{
		SListNode<T>* node = 0;

		if (head != 0)
		{
			node = head->_next;
			delete head;

			head = node;

			if (head == 0)
				tail = 0;

			count--;
		}
	}

	void RemoveTail()
	{
		SListNode<T>* node = head;

		if (head != 0)
		{
			if (head == tail)
			{
				delete head;
				head = tail = 0;
			}
			else
			{
				while (node->_next != tail)
				{
					node = node->_next;
				}

				tail = node;

				delete node->_next;
				node->_next = 0;
			}
			count--;
		}
	}

	SListIterator<T> GetIterator()
	{
		return SListIterator<T>(this, head);
		// return { this, head };
	}
};

