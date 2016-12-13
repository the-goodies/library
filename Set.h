#ifndef _set_h
#define _set_h

#include <cstdint> // int8_t ... types
#include <cassert> // assert
#include "Array.h"
#include "error.h"

typedef std::int8_t  s8;
typedef std::int16_t s16;
typedef std::int32_t s32;
typedef std::int64_t s64;

typedef std::uint8_t  u8;
typedef std::uint16_t u16;
typedef std::uint32_t u32;
typedef std::uint64_t u64;

#define ERROR(MESSAGE, ...) error(__FILE__, __LINE__, MESSAGE, __VA_ARGS__)

// Set implemented as Binary Search Tree
template <typename type>
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
	int (*compare)(const type&, const type&);
	s64 nodeCount; // size of Set

	inline static int compare_default(const type & lhs, const type & rhs)
	{
		if (lhs > rhs) return 1;
		else if (lhs == rhs) return 0;
		else return -1; // lhs < rhs
	}

	static Node* findNode(Node* tree, const type & value, int(*compare)(const type&, const type&))
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
	Set(int (*cmp)(const type&, const type&) = compare_default): 
		root(nullptr), compare(cmp), nodeCount(0) { /* empty */ }


	// uniform initialization -  Set<float> set = { 2.3, 2.4 ... }
	Set(const std::initializer_list<type> & il):
		root(nullptr), compare(compare_default), nodeCount(0)
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



	// fills output stream with objects info contained in a Set
	// those objects have to implement << operator in order to work
	friend std::ostream & operator<<(std::ostream & os, const Set<type> & set)
	{
		// typeid(var).name() returns type name of var
		os << typeid(set).name() << " (size " << set.size() << ") values: ";
		for (const type & value : set)
		{
			os << value << " ";
		}
		return os;
	}









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
};

#endif