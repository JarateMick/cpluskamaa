#include <list>
#include <vector>
#include <queue>

template<class NodeType, class ArcType>
class GraphArc;

template<class NodeType, class ArcType>
class GraphNode
{
public:
	typedef GraphArc<NodeType, ArcType> Arc;
	typedef GraphNode<NodeType, ArcType> Node;

	NodeType data;
	std::list<Arc> archlist;
	bool marked;

	void AddArc(Node* node, ArcType weight)
	{
		Arc a;
		a.node = node;
		a.weight = weight;
		archlist.push_back(a);
	}

	Arc* GetArc(Node* node)
	{
		auto iter = archlist.begin();

		for (iter; iter != archlist.end(); iter++)
		{
			if ((*iter).node == node)
				return &(*iter);
		}
		return 0;

		//auto iter = std::find(archlist.begin(), archlist.end(), node);
		//if (iter != archlist.end())
		//{
		//	return &(*iter);
		//}
		//return nullptr;
	}
};


template<class NodeType, class ArcType>
class GraphArc
{
public:
	GraphNode<NodeType, ArcType>* node;
	ArcType weight;
};

template<class NodeType, class ArcType>
class Graph
{
public:
	typedef GraphArc<NodeType, ArcType> Arc;
	typedef GraphNode<NodeType, ArcType> Node;

	std::vector<Node*> nodes;
	int count;

	Graph(int size) : nodes(size)
	{
		int i;
		for (i = 0; i < size; i++)
		{
			nodes[i] = 0;
		}
		count = 0;
	}

	~Graph()
	{
		int index;
		for (index = 0; index < nodes.size(); index++)
		{
			if (nodes[index] != 0)
			{
				delete nodes[index];
			}
		}
	}

	bool AddNode(NodeType data, int index)
	{
		if (nodes[index] != 0)
		{
			return false;
		}

		nodes[index] = new Node;
		nodes[index]->data = data;
		nodes[index]->marked = false;
		count++;

		return true;
	}

	void RemoveNode(int index)
	{
		if (nodes[index] == 0)
		{
			return;
		}

		int node;
		Arc* arc;

		for (node = 0; node < nodes.size(); node++)
		{
			if (nodes[node] != 0)
			{
				arc = nodes[node]->GetArc(nodes[index]);
				if (arc != 0)
					RemoveArc(node, index);
			}
		}

		delete node[index];
		nodes[index] = 0;
		count--;
	}

	bool AddArc(int from, int to, ArcType weight)
	{
		if (nodes[from] == 0 || nodes[to] == 0)
		{
			return false;
		}
		if (nodes[from]->GetArc(nodes[to]) != 0)
		{
			return false;
		}
		nodes[from]->AddArc(nodes[to], weight);
		return true;
	}

	void RemoveArc(int from, int to)
	{
		if (nodes[from] == 0 || nodes[to] == 0)
		{
			return;
		}

		nodes[from]->RemoveArc(nodes[to]);
	}
	
	Arc* GetArc(int from, int to)
	{
		if (nodes[from] == 0 || nodes[to] == 0)
		{
			return 0;
		}

		return nodes[from]->GetArc(nodes[to]);
	}

	void ClearMarks()
	{
		int index;
		for (index = 0; index < nodes.size(); index++)
		{
			if (nodes[index] != 0)
			{
				nodes[index]->marked = false;
			}
		}
	}

	void DepthFirst(Node* node, void(*process)(Node*))
	{
		if (node == 0)
			return;

		process(node);
		node->marked = true;

		auto iter = node->archlist.begin();
		for (iter; iter != node->archlist.end(); iter++)
		{
			if (iter->node->marked == false)
			{
				DepthFirst((*iter).node, process);
			}
		}
	}

	// breadth-first sivu 511
	void BreadthFirst(Node* node, void(*process)(Node*))
	{
		if (node == 0)
			return;

		std::queue<Node*> queue;

		// list itr

		queue.push(node);
		node->marked = true;

		while (queue.size() != 0)
		{
			process(queue.front());
			auto itr = queue.front()->archlist.begin();
			for (itr; itr != queue.front()->archlist.end(); itr++)
			{
				if (itr->node->marked == false)
				{
					itr->node->marked = true;
					queue.push(itr->node);
				}
			}
			queue.pop();
		}
	}
};

#define EXPORT extern "C" __declspec(dllexport)

EXPORT void* CreateGraph(int size)
{
	Graph<int, int>* map = new Graph<int, int>(size);
	return (void*)map;
}

EXPORT void GetArc(void *graph, int index)
{

}

EXPORT int GetValue(void* graph, int data, int index)
{
	Graph<int, int>* map = (Graph<int, int>*)graph;
	return map->nodes[index]->data;
}

EXPORT void AddNode(void* graph, int data, int index)
{
	Graph<int, int>* map = (Graph<int, int>*)graph;
	map->AddNode(data, index);
}

void process(GraphNode<int, int>* nodes)
{
	printf("%i", nodes->data);
}

EXPORT void PrintAll(void* graph)
{
	Graph<int, int>* map = (Graph<int, int>*)graph;
	map->BreadthFirst(map->nodes[0], process);
}
