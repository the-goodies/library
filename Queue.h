#ifndef _queue_h
#define _queue_h

#include <iostream>
#include <cstdlib> // malloc, free
#include "utility.h"


template <typename type>
class Queue
{
	// circular Queue
	// max size before expanding capacity - 1
	s64 capacity; // allocated size
	s64 front; // front position of queue -> ocuppied (data exists at this index)
	s64 back; // back position of queue -> unoccupied (data doesn't yet exists at this index)
	type *data; // container holding data

	// will double everytime reaches limit
	static const u32 INITIAL_CAPACITY = 20;

	// private functions

void destructInternalData()
{
	s64 count = this->size();
	for (s64 i = 0; i < count; ++i) data[(back + i) % capacity].~type();
}

void expandCapacity()
{
	type *new_data = (type*) malloc(capacity * 2 * sizeof(type));
	if (new_data == nullptr)
		ERROR("Failed to allocate %I64u bytes to expand Queue container", capacity * 2 * sizeof(type));
	s64 count = this->size();
	for (s64 i = 0; i < count; ++i) new(new_data + i) type(std::move(data[(back + i) % capacity]));
	// just in case objects being moved only have copy constructor, so we have to explicitly destruct them afterwards
	destructInternalData();
	free(data);

	this->front = 0;
	this->back = count;
	this->capacity *= 2;
	data = new_data;
}



public:

	Queue()
	{
		capacity = INITIAL_CAPACITY;
		data = (type*) malloc(capacity * sizeof(type));
		if (data == nullptr)
			ERROR("Failed to allocate %I64u bytes to construct Queue container", capacity * sizeof(type));
		back = front = 0;
	}

	~Queue()
	{
		destructInternalData();
		back = front = 0;
		free(data);
	}

	// uniform initialization -  Queue<float> queue = { 2.3, 2.4 ... }
	Queue(std::initializer_list<type> il)
	{
		s64 il_size = il.size();
		capacity = INITIAL_CAPACITY;
		if (il_size > capacity) capacity = il_size * 2;

		data = (type*)malloc(capacity * sizeof(type));

		back = front = 0;
		for (auto & el : il) new(data + back++) type(std::move(el));
	}

	// copy constructor
	Queue(const Queue<type> & queue)
	{
		capacity = queue.capacity;
		data = (type*) malloc(capacity * sizeof(type));
		if (data == nullptr)
			ERROR("Failed to allocate %I64u bytes to copy Queue object's data", capacity * sizeof(type));

		s64 count = queue.size();
		for (s64 i = 0; i < count; ++i) new(data + i) type(queue.data[(queue.back + i) % queue.capacity]);
		front = 0;
		back = count;
	}

	// move constructor
	Queue(Queue<type> && queue)
	{
		capacity = queue.capacity;
		front = queue.front;
		back = queue.back;
		data = queue.data;

		queue.data = nullptr;
		queue.back = queue.front = queue.capacity = 0;
	}

	// copy/move assignment utilizing copy/move constructor by taking argument as value
	Queue & operator=(Queue<type> queue)
	{
		// swap
		s64 capacity_temp = capacity;
		s64 front_temp = front;
		s64 back_temp = back;
		type* data_temp = data;

		capacity = queue.capacity;
		front = queue.front;
		back = queue.back;
		data = queue.data;

		queue.capacity = capacity_temp;
		queue.front = front_temp;
		queue.back = back_temp;
		queue.data = data_temp;
		// queue going out of scope will destruct old Queue's object's data

		return *this;
	}

	// add type element to the back
	void enqueue(type el)
	{
		if (size() == capacity - 1) expandCapacity();
		new(data + back) type(std::move(el));
		back = (back + 1) % capacity;
	}

	// synonym for enqueue
	void insert(type el) { enqueue(std::move(el)); }

	// remove type element from the front
	type dequeue()
	{
		if (isEmpty()) ERROR("Can't remove from empty queue");

		type result = std::move(data[front]);
		data[front].~type();
		front = (front + 1) % capacity;
		return result;
	}

	// synonym for dequeue
	type get() { return dequeue(); }

	type peek()
	{
		if (isEmpty()) ERROR("Can't remove from empty queue");
		return data[front];
	}

	void clear()
	{
		destructInternalData();
		back = front = 0;
	}

	inline bool isEmpty() const { return front == back; }
	inline s64 size() const { return (back - front + capacity) % capacity; }

	Queue<type> & operator+=(const type & el)
	{
		this->insert(el);
		return *this;
	}

	// typename type has to support << operator in order to work
	friend std::ostream & operator<(std::ostream & os, const Queue<type> & queue)
	{
		Queue<type> copy(*queue);
		s64 size = copy.size();
		os << typeid(arr).name() << " (size " << size << ") objects in queue order: ";

		for (s64 i = 0; i < size; ++i) os << copy.get() << " ";
		return os;
	}
};



#endif
