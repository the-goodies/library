#ifndef _set_h
#define _set_h

#include <iostream>
#include <iomanip>
#include "Array.h"
#include "Queue.h"
#include "utility.h"
#include "mat.h"


// Set implemented as Binary Search Tree
template <typename type, typename compareType = compare_to<type>>
class Set
{
	struct Node
	{
		const type value;
		Node* parent;
		Node* left;
		Node* right;

		Node(const type & value, Node* parent = nullptr, Node* left = nullptr, Node* right = nullptr) :
			value(value), parent(parent), left(left), right(right) { /* empty */ }
	};

	Node* root;
	// returns 1 if 1st element is bigger than 2nd, 0 if equal, -1 if 2nd is bigger than 1st
	compareType compare;
	s64 nodeCount; // size of Set

	static Node* findNode(Node* tree, const type & value, const compareType & compare)
	{
		if (tree == nullptr) return nullptr;

		int cmp = compare(value, tree->value);
		if (cmp < 0) 		return findNode(tree->left, value, compare);
		else if (cmp > 0) 	return findNode(tree->right, value, compare);
		else 				return tree;
	}

	static Node* findMin(Node* tree)
	{
		if (tree == nullptr) return nullptr;

		while (tree->left != nullptr) tree = tree->left;
		return tree;
	}

	static Node* findMax(Node* tree)
	{
		if (tree == nullptr) return nullptr;

		while (tree->right != nullptr) tree = tree->right;
		return tree;
	}

	// returns larger node (next) after given tree node
	// returns nullptr if tree node is largest node
	static Node* successor(Node* tree)
	{
		if (tree == nullptr) 		return nullptr;
		if (tree->right != nullptr) return findMin(tree->right);

		while (tree->parent != nullptr && tree->parent->right == tree)
			tree = tree->parent;

		return tree->parent;
	}

	// returns smaller node (previous) before given tree node
	// returns nullptr if tree node is smallest node
	static Node* predecessor(Node* tree)
	{
		if (tree == nullptr) 		return nullptr;
		if (tree->left != nullptr)	return findMax(tree->left);

		while (tree->parent != nullptr && tree->parent->left == tree)
			tree = tree->parent;

		return tree->parent;
	}

	// returns true if value was not present in container, false otherwhise
	bool insertNode(const type & value)
	{
		if (root == nullptr)
		{
			root = new Node(value);
			nodeCount += 1;
			return true;
		}

		int cmp = 0;
		Node* parent = nullptr;
		Node* current = root;
		while(current != nullptr)
		{
			parent = current;

			cmp = compare(value, current->value);
			if (cmp < 0) 		current = current->left;
			else if (cmp > 0) 	current = current->right;
			else 				return false;
		}

		if (cmp < 0) 		parent->left = new Node(value, parent);
		else if (cmp > 0) 	parent->right = new Node(value, parent);
		else return false;
		nodeCount += 1;
		return true;
	}

	// returns true if node is succesfully removed, false otherwhise
	bool removeNode(Node* tree)
	{
		if (tree == nullptr) return false;
		// replaces tree subtree with substitute subtree:
		// tree's parent with approriate child which points to tree starts to point to substitute instead of pointing to tree
		// substitute's new parent becomes tree's parent irrespective of who substitute's previous parent was
		// substitute can be nullptr then tree's parent's approriate child which points to tree starts to point to nullptr
		auto transplantNode = [this](Node* tree, Node* substitute)
		{
			assert(tree != nullptr);
			// first make tree's parent point to substitute
			// if tree has no parent then it is root therefore make root point to substitute
			if (tree->parent == nullptr) this->root = substitute;
			else if (tree->parent->left == tree) tree->parent->left = substitute;
			else tree->parent->right = substitute;
			// make substitute's new parent tree's parent
			if (substitute != nullptr) substitute->parent = tree->parent;
		};

		if (tree->left == nullptr && tree->right == nullptr) transplantNode(tree, nullptr);
		else if (tree->left == nullptr) transplantNode(tree, tree->right);
		else if (tree->right == nullptr) transplantNode(tree, tree->left);
		else
		{
			Node* next_larger = successor(tree);
			if (tree->right != next_larger)
			{
				transplantNode(next_larger, next_larger->right);
				next_larger->right = tree->right;
				tree->right->parent = next_larger;
			}
			transplantNode(tree, next_larger);
			next_larger->left = tree->left;
			tree->left->parent = next_larger;
		}
		delete tree; nodeCount -= 1; return true;
	}

	// side effect if tree was not empty:
	// after method call: root instance variable points to deallocated memory
	// after method call: nodeCount instance variable is not zero
	static void clearTree(Node* tree)
	{
		if (tree == nullptr) return;

		clearTree(tree->left);
		clearTree(tree->right);

		delete tree;
	}

	// creates a copy of the provided tree and returns new tree's root
	static Node* copyTree(Node* tree, Node* parent = nullptr)
	{
		if (tree == nullptr) return nullptr;

		Node* left = copyTree(tree->left, tree);
		Node* right = copyTree(tree->right, tree);

		return new Node(tree->value, parent, left, right);
	}

public:

	class iterator : public std::iterator<std::input_iterator_tag, type>
	{
		const Set* set;
		// position pointing to the node containing value to return
		// starting from min element (findMin)
		Node* pos;
		// position pointing to the next node
		// usefull if you remove a node while iterating - still have pointer to the next one
		Node* next_pos;
	public:
		// default non usable constructor, which returns invalid iterator
		iterator() : set(nullptr), pos(nullptr), next_pos(nullptr) { /* empty */ }

		// end indicates to initialize end iterator
		iterator(const Set* set, bool end = false)
		{
			this->set = set;
			if (end)
			{
				this->pos = nullptr;
				this->next_pos = nullptr;
			} 
			else
			{
				this->pos = findMin(set->root);
				this->next_pos = successor(this->pos);
			}
		}

		// copy constructor
		iterator(const iterator & it)
		{
			this->set = it.set;
			this->pos = it.pos;
			this->next_pos = it.next_pos;
		}

		iterator & operator++() 
		{
			pos = next_pos;
			next_pos = successor(next_pos);
			return *this;
		}
		iterator operator++(int)
		{
			iterator result(*this);
			++(*this);
			return result;
		}

		const type & operator*() { return pos->value; }
		const type* operator->() { return &pos->value; }

		bool operator==(const iterator & it) const { return this->set == it.set && this->pos == it.pos; }
		bool operator!=(const iterator & it) const { return !(*this == it); }
	};

	// constructor
	Set(): root(nullptr), compare(), nodeCount(0) { /* empty */ }


	// uniform initialization -  Set<float> set = { 2.3, 2.4 ... }
	Set(const std::initializer_list<type> & il): Set()
		{ for (auto & el : il) this->insert(el); }

	// copy constructor
	Set(const Set<type> & set)
	{
		this->root = copyTree(set.root);
		this->nodeCount = set.nodeCount;
		this->compare = set.compare;
	}

	// move constructor
	Set(Set<type> && set)
	{
		this->root = set.root;
		this->nodeCount = set.nodeCount;
		this->compare = set.compare;

		set.root = nullptr;
		set.nodeCount = 0;
	}

	// copy/move assignment
	Set & operator=(Set<type> set)
	{
		// swap
		Node* root_temp = this->root;
		s64 nodeCount_temp = this->nodeCount;

		this->root = set.root;
		this->nodeCount = set.nodeCount;
		this->compare = set.compare;

		set.root = root_temp;
		set.nodeCount = nodeCount_temp;
		// set going out of scope will destruct old tree
		return *this;
	}

	// destructor
	~Set() { clear(); }

	type min() { return findMin(root)->value; }
	type max() { return findMax(root)->value; }
	bool insert(const type & value) { return insertNode(value); }
	bool contains(const type & value) const { return findNode(root, value, compare) != nullptr; }
	bool remove(const type & value) { return removeNode(findNode(root, value, compare)); }
	void clear() { clearTree(root); nodeCount = 0; root = nullptr; }
	inline s64 size() const { return nodeCount; }
	inline bool isEmpty() const { return nodeCount == 0; }
	iterator begin() const { return iterator(this); }
	iterator end() const { return iterator(this, /*end*/ true); }



	// SET OPERATIONS

	// set union
	Set<type> operator+(const Set<type> & rhs) const
	{
		Set<type> set(*this);
		for (type value : rhs)
			set.insert(value);
		return set;
	}

	Set<type> operator+(const type & value) const
	{
		Set<type> set(*this);
		set.insert(value);
		return set;
	}

	Set<type> & operator+=(const Set<type> & rhs)
	{
		for (type value : rhs)
			this->insert(value);
		return *this;
	}

	Set<type> operator+=(const type & value)
	{
		this->insert(value);
		return *this;
	}

	// set intersection
	Set<type> operator*(const Set<type> & rhs) const
	{
		Set<type> set;
		for (type value : rhs)
		{
			if (this->contains(value))
				set.insert(value);
		}
		return set;
	}

	Set<type> & operator*=(const Set<type> & rhs)
	{
		Array<type> toRemove;
		for (type value : *this)
		{
			if (!rhs.contains(value))
				toRemove.insert(value);
		}
		for (type value : toRemove)
			this->remove(value);
		return *this;
	}

	// set difference
	Set<type> operator-(const Set<type> & rhs) const
	{
		Set<type> set;
		for (type value : *this)
		{
			if (!rhs.contains(value))
				set.insert(value);
		}
		return set;
	}

	Set<type> operator-(const type & value) const
	{
		Set<type> set(*this);
		set.remove(value);
		return set;
	}

	Set<type> & operator-=(const Set<type> & rhs)
	{
		for (type value : rhs)
			this->remove(value);
		return *this;
	}

	Set<type> & operator-=(const type & value)
	{
		this->remove(value);
		return *this;
	}


	bool isSubsetOf(const Set<type> & rhs)  const
	{
		for (const type & value : *this)
		{
			if (!rhs.contains(value)) return false;
		}
		return true;
	}

	bool isSupersetOf(const Set<type> & rhs) const
	{
		for (const type & value : rhs)
		{
			if (!this->contains(value)) return false;
		}
		return true;
	}

	bool operator==(const Set<type> & rhs) const
	{ return this->isSubsetOf(rhs) && rhs.isSubsetOf(*this); }

	bool operator!=(const Set<type> & rhs) const { return !(*this == rhs); }






	int heightWrapper(Node* tree, bool & is_balanced) const
	{
		if (tree == nullptr) return -1;
		int left_height = heightWrapper(tree->left, is_balanced);
		int right_height = heightWrapper(tree->right, is_balanced);
		int max_height = left_height > right_height ? left_height : right_height;

		int balance = left_height - right_height;
		if (balance < -1 || balance > 1) is_balanced = false;

		return max_height + 1;
	}

	int height() const { bool unused = true; return heightWrapper(root, unused); }

	bool isBalanced() const
	{
		bool is_balanced = true;
		heightWrapper(root, is_balanced);
		return is_balanced;
	}



// print tree visualization
private:

	struct InfoNode
	{
		Node* node;
		// depth and position count starts at 0
		u32 depth;
		u32 pos; // (2^depth)-1 last position

		InfoNode(Node* tree, u32 depth, u32 pos):
			node(tree), depth(depth), pos(pos) { /* empty */ }
	};

	// returns a grid (nodes within the same depth get the same row)
	static Array<Array<InfoNode>> getNodeGrid(Node* tree)
	{
		Array<Array<InfoNode>> grid;
		Array<InfoNode> level; // will contain all nodes at particular depth
		InfoNode root(tree, 0, 0);

		if (tree == nullptr)
		{
			level.insert(root);
			grid.insert(level);
			return grid;
		}

		u32 depth = 0; // current depth of traversing tree
		u32 pos = 0; // current position at current depth
		Queue<InfoNode> queue {root};
		while (!queue.isEmpty())
		{
			InfoNode info = queue.get();
			if (info.depth != depth)
			{
				grid.insert(level);
				level.clear();
				depth += 1;
				pos = 0;
			}
			// fill level with empty infoNodes upto new node from queue 
			while (pos < info.pos) level.insert(InfoNode(nullptr, depth, pos++));
			// make pos point one past the added element
			// so the next iteration will not fill empty node unnecessary
			level.insert(info); ++pos;

			// add next InfoNodes to the queue
			if (info.node->left != nullptr)
				queue.insert(InfoNode(info.node->left, info.depth + 1, info.pos * 2));
			if (info.node->right != nullptr)
				queue.insert(InfoNode(info.node->right, info.depth + 1, info.pos * 2 + 1));
		}
		grid.insert(level);
		return grid;
	}


	static void printTree(Node* tree, std::ostream & os = std::cout)
	{
		// ammount of space node itself takes
		const u32 NODE_SPACE = 7;
		// spaces between nodes at max depth (lowest level)
		const u32 NODE_DELIMITER_SPACE = 3;
		// to distinguish where node starts and ends additional symbol
		// within NODE_SPACE is used to indicate node boundaries
		const char SYMBOL_NODE_BOUNDARY = '|';
		// symbols used for lines and a connector between a line and a node
		const char SYMBOL_LEFT_LINE = '|';
		const char SYMBOL_LEFT_CONNECTOR = '|';
		const char SYMBOL_RIGHT_LINE = '|';
		const char SYMBOL_RIGHT_CONNECTOR = '|';

		Array<Array<InfoNode>> grid = getNodeGrid(tree);
		const u32 MAX_DEPTH = grid.size() - 1;

		auto lineLength = [NODE_SPACE, NODE_DELIMITER_SPACE, MAX_DEPTH](u32 depth, u32 pos) -> u32
		{
			// counting depth from bottom, instead of from root
			u32 level = MAX_DEPTH - depth;
			u32 result = mat::pow(2, level) * (NODE_SPACE + NODE_DELIMITER_SPACE) / 2;
			return result;
		};

		auto spaceBeforeLine = [NODE_SPACE, NODE_DELIMITER_SPACE, MAX_DEPTH](u32 depth, u32 pos) -> u32
		{
			// counting depth from bottom, instead of from root
			u32 level = MAX_DEPTH - depth;
			u32 result = mat::pow(2, level) * (NODE_SPACE + NODE_DELIMITER_SPACE);
			if (pos == 0) return (result - NODE_DELIMITER_SPACE) / 2;
			else		  return result - 1;
		};

		auto spaceBeforeUpConnector = [NODE_SPACE, NODE_DELIMITER_SPACE, MAX_DEPTH](u32 depth, u32 pos) -> u32
		{
			// counting depth from bottom, instead of from root
			u32 level = MAX_DEPTH - depth;
			u32 result = mat::pow(2, level) * (NODE_SPACE + NODE_DELIMITER_SPACE);
			if (pos == 0) return (result - NODE_DELIMITER_SPACE) / 2;
			else		  return result - 1;
		};

		auto spaceBeforeNode = [NODE_SPACE, NODE_DELIMITER_SPACE, MAX_DEPTH](u32 depth, u32 pos) -> u32
		{
			// counting depth from bottom, instead of from root
			u32 level = MAX_DEPTH - depth;
			u32 result = mat::pow(2, level) * (NODE_SPACE + NODE_DELIMITER_SPACE) - NODE_SPACE;
			if (pos == 0) return (result - NODE_DELIMITER_SPACE) / 2;
			else		  return result;
		};

		auto spaceBeforeDownConnector = [NODE_SPACE, NODE_DELIMITER_SPACE, MAX_DEPTH](u32 depth, u32 pos) -> u32
		{
			// counting depth from bottom, instead of from root
			u32 level = MAX_DEPTH - depth;
			u32 result = mat::pow(2, level) * (NODE_SPACE + NODE_DELIMITER_SPACE);
			if (pos == 0) return (result - NODE_DELIMITER_SPACE) / 2 - 1;
			else		  return result - 3;
		};

		auto draw = [&os](char c, u32 length)
			{ for (u32 i = 0; i < length; ++i) os << c; };

		auto drawNode = [&os, NODE_SPACE, SYMBOL_NODE_BOUNDARY](InfoNode & node)
		{
			if (node.node != nullptr)
			{
				os << SYMBOL_NODE_BOUNDARY;

				// -2 for using BOUNDARY before and after
				os << std::setw(NODE_SPACE - 2);
				os << std::setfill('_');
				os << std::left;
				os << node.node->value;

				os << SYMBOL_NODE_BOUNDARY;
			}
			else
			{				 
				for (u32 space = 0; space < NODE_SPACE; ++space)
					os << ' ';
			}
		};

		// how long the connector between node and line is
		const u32 CONNECTOR_HEIGHT = 2;

		// draw root level
		draw(' ', spaceBeforeNode(0,0));
		drawNode(grid[0][0]);
		draw('\n', 1);
		for (u32 time = 0; time < CONNECTOR_HEIGHT; ++time)
		{
			draw(' ', spaceBeforeDownConnector(0, 0));
			draw(grid[0][0].node != nullptr ? SYMBOL_LEFT_CONNECTOR : ' ', 1);
			draw(' ', 1);
			draw(grid[0][0].node != nullptr ? SYMBOL_RIGHT_CONNECTOR : ' ', 1);
			draw('\n', 1);
		}

		// draw all the rest levels after root
		for (u32 depth = 1; depth <= MAX_DEPTH; ++depth)
		{
			u32 depth_size = grid[depth].size();
			// draw lines between depth-1 and depth
			for (u32 pos = 0; pos < depth_size; ++pos)
			{
				bool isNull = grid[depth][pos].node == nullptr;
				if (pos % 2 == 0)
				{
					draw(' ', spaceBeforeLine(depth, pos));
					draw(isNull ? ' ' : SYMBOL_LEFT_LINE, lineLength(depth, pos));
				}
				else
				{
					draw(' ', 1);
					draw(isNull ? ' ' : SYMBOL_RIGHT_LINE, lineLength(depth, pos));
				}
			}
			draw('\n', 1);

			// draw up connectors between upper lines and below nodes
			for (u32 time = 0; time < CONNECTOR_HEIGHT; ++time)
			{
				for (u32 pos = 0; pos < depth_size; ++pos)
				{
					draw(' ', spaceBeforeUpConnector(depth, pos));
					bool isNull = grid[depth][pos].node == nullptr;
					if (pos % 2 == 0)	draw(isNull ? ' ' : SYMBOL_LEFT_CONNECTOR, 1);
					else				draw(isNull ? ' ' : SYMBOL_RIGHT_CONNECTOR, 1);
				}
				draw('\n', 1);
			}

			// draw nodes
			for (u32 pos = 0; pos < depth_size; ++pos)
			{
				draw(' ', spaceBeforeNode(depth, pos));
				drawNode(grid[depth][pos]);
			}
			draw('\n', 1);

			// draw down connectors between upper nodes and below lines
			for (u32 time = 0; time < CONNECTOR_HEIGHT; ++time)
			{
				for (u32 pos = 0; pos < depth_size; ++pos)
				{
					draw(' ', spaceBeforeDownConnector(depth, pos));
					draw(grid[depth][pos].node != nullptr ? SYMBOL_LEFT_CONNECTOR : ' ', 1);
					draw(' ', 1);
					draw(grid[depth][pos].node != nullptr ? SYMBOL_RIGHT_CONNECTOR : ' ', 1);
				}
				draw('\n', 1);
				if (depth == MAX_DEPTH) break;
			}
		}
	}

public:

	// prints tree visualization
	// those objects have to implement << operator in order to work
	friend std::ostream & operator<<(std::ostream & os, const Set<type> & set)
	{
		printTree(set.root, os);
		return os;
	}

};



#endif