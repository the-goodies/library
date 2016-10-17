#ifndef _queue_h
#define _queue_h

#include <cstdint> // int8_t ... types
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
	static const u32 INITIAL_CAPACITY = 64;

	// private functions

void destructInternalData()
{
	s64 count = this->size();
	for (s64 i = 0; i < count; ++i)
	{
		data[(back + i) % capacity].~type();
	}
}

void expandCapacity()
{
	type *new_data = (type*) malloc(capacity * 2 * sizeof(type));
	if (new_data == nullptr)
	{
		ERROR("Failed to allocate memory to expand Queue container");
	}
	s64 count = this->size();
	for (s64 i = 0; i < count; ++i)
	{
		new(new_data + i) type(std::move(data[(back + i) % capacity]));
	}
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
		{
			ERROR("Failed to allocate memory to construct Queue container");
		}
		back = front = 0;
	}

	~Queue()
	{
		destructInternalData();
		back = front = 0;
		free(data);
	}

	// uniform initialization -  Queue<float> queue = { 2.3, 2.4 ... }
	Queue(const std::initializer_list<type> & il)
	{
		s64 il_size = il.size();
		capacity = INITIAL_CAPACITY;
		if (il_size > capacity)
		{
			capacity = il_size * 2;
		}
		data = (type*)malloc(capacity * sizeof(type));

		back = front = 0;
		for (auto & el : il)
		{
			new(data + back++) type(el);
		}
	}

	// copy constructor
	Queue(const Queue<type> & queue)
	{
		capacity = queue.capacity;
		data = (type*) malloc(capacity * sizeof(type));
		if (data == nullptr)
		{
			ERROR("Failed to allocate memory to copy Queue object's data");
		}

		s64 count = queue.size();
		for (s64 i = 0; i < count; ++i)
		{
			new(data + i) type(queue.data[(queue.back + i) % queue.capacity]);
		}
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
		if (size() == capacity - 1)
		{
			expandCapacity();
		}
		new(data + back) type(std::move(el));
		back = (back + 1) % capacity;
	}

	// remove type element from the front
	void dequeue()
	{
		if (isEmpty()) ERROR("Can't remove from empty queue");

		type result = std::move(data[front]);
		data[front].~type();
		front = (front + 1) % capacity;
		return result;
	}

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
}



#endif
