#ifndef _priorityqueue_h
#define _priorityqueue_h

#include <iostream>
#include "Array.h"
#include "utility.h"

// STL library's implementation by default uses less comparison which results in maxHeap
// even though it's convention, to me it's seems unintuitive (what do i know) so i reversed order
// now default less comparison results in minHeap, if you want maxHeap, provide greater comparison function
template <typename compareType>
struct compare_less { bool operator()(const compareType & lhs, const compareType & rhs) { return lhs < rhs; }


// PriorityQueue container which returns elements of type Key with highest priority first - defined by compare function
// compare function compares two elements of typename Key and returns true if first element is of higher priority than second
// compare function is given to constructor or default is used (typename Key has to implement < operator in default case)
template <typename Key, typename compareType = compare_less<Key>>
class PriorityQueue
{
	Array<Key> heap; // holding minHeap in array starting from 1 element, leaving 0 index unused
	compareType compare; // returns true if 1st element is of higher priority than 2nd element


	void exchange(s64 a, s64 b)
	{
		Key temp = std::move(heap[a]);
		heap[a] = std::move(heap[b]);
		heap[b] = std::move(temp);
	}

	// bottom up reheapify (swim from bottom to up exchanging element from given position if it's of higher priority than it's parents)
	void heapUp(s64 el_pos)
	{
		while(el_pos > 1) // keep checking while el node is not the root
		{
			s64 parent_pos = el_pos / 2;
			if (!compare(heap[el_pos], heap[parent_pos])) return; // element doesn't have higher priority than parent so no need to exchange anymore

			exchange(el_pos, parent_pos);
			el_pos /= 2;
		}
	}

	// top down reheapify (sink from top to bottom exchanging element from given position if it's of lower priority than one of it's 2 children)
	void heapDown(s64 el_pos)
	{
		s64 heap_size = this->size();
		while (el_pos * 2 <= heap_size) // keep checking while el node has a child
		{
			s64 child_pos = el_pos * 2; // left child

			// check if right child still exists and if it is of higher priority than left child
			if (child_pos < heap_size && compare(heap[child_pos + 1], heap[child_pos]))
				child_pos += 1;

			// child doesn't have higher priority than element so no need to exchange anymore
			if (!compare(heap[child_pos], heap[el_pos])) return;

			exchange(el_pos, child_pos);
			el_pos = child_pos;
		}
	}

	// transforms heap to ordered one (returns elements with highest priority)
	void heapify()
	{
		// start from first element which is not a leaf and from there go upward
		for (s64 pos = this->size() / 2; pos >= 1; --pos) heapDown(pos);
	}


public:

	// constructor for empty minHeap
	PriorityQueue(): heap(), compare()
	{
		heap.insert(Key()); // fill first element with default value which will not be used
	}

	// constructor from given Array container
	PriorityQueue(const Array<Key> & arr): PriorityQueue()
	{
		heap.reserve(1 + arr.size()); // +1 for 0 index
		for (auto & el : arr) heap.insert(el);
		heapify(); // fix order, so higher priority elements will return first
	}

	PriorityQueue(const std::initializer_list<Key> & il): PriorityQueue()
	{
		heap.reserve(1 + il.size()); // +1 for 0 index
		for (auto & el : il) heap.insert(el);
		heapify(); // fix order, so higher priority elements will return first
	}

	// copy constructor
	PriorityQueue(const PriorityQueue<Key> & PQ): heap(PQ.heap), compare(PQ.compare) { /* empty */ }
	PriorityQueue(PriorityQueue && PQ): heap(std::move(PQ.heap)), compare(PQ.compare) { /* empty */ }

	// copy/move assignment 
	PriorityQueue & operator=(PriorityQueue PQ)
	{
		heap = std::move(PQ.heap);
		compare = PQ.compare;
		return *this;
	}

	~PriorityQueue() { /* empty */ }

	inline s64 size() const { return heap.size() - 1; } // heap[0] doesn't count
	inline bool isEmpty() const { return this->size() == 0; }

	// removes all elements from PriorityQueue
	void clear()
	{
		heap.clear();
		// occupy 0 index, so that container will be in valid state
		heap.insert(Key());
	}

	void insert(Key el)
	{
		heap.insert(std::move(el)); // add to the end
		heapUp(this->size()); // reheapify from just added element to correct position
	}

	// returns highest priority element and removes it from container
	Key get()
	{
		if (this->isEmpty()) ERROR("Can't get element from empty PriorityQueue container");
		Key result = heap[1];

		exchange(1, this->size()); // exchange element of highest priority with last
		heap.remove(this->size()); // remove highest priority element from container
		heapDown(1); // reheapify from top element (which recently was bottom element) to correct position
		return result;
	}

	// returns highest priority element from container, but does not remove it
	Key peek()
	{
		if (this->isEmpty()) ERROR("Can't peek into empty PriorityQueue container");
		return heap[1];
	}

	PriorityQueue<Key> & operator+=(const Key & el)
	{
		this->insert(el);
		return *this;
	}

	// typename Key has to support << operator in order to work
	friend std::ostream & operator<<(std::ostream & os, const PriorityQueue<Key> & PQ)
	{
		PriorityQueue<Key> copy(PQ);
		s64 size = copy.size();
		os << typeid(PQ).name() << " (size " << size << ") objects in priority order: ";

		for (s64 i = 0; i < size; ++i) os << copy.get() << ' ';
		return os;
	}
};



#endif