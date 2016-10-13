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

void expandCapacity()
{
	type *new_data = (type*) malloc(capacity * 2 * sizeof(type));
	if (new_data == nullptr)
	{
		error("Failed to allocate memory to expand Queue container");
	}
	for (s64 i = back; i < capacity; ++i)
	{
		new(new_data + i - back) type(std::move(data[i]));
	}
	for (s64 i = 0; i < front; ++i)
	{
		new(new_data + i + (capacity - back)) type(std::move(data[i]));
	}
	// just in case objects being moved only have copy constructor, so we have to explicitly destruct them afterwards
	destructInternalData();
	free(data);
	data = new_data;
}


public:

	Queue()
	{
		capacity = INITIAL_CAPACITY;
		data = (type*) malloc(capacity * sizeof(type));
		if (data == nullptr)
		{
			error("Failed to allocate memory to construct Queue container");
		}
		count = 0;
	}

	Queue(s64 capacity)
	{
		if (capacity <= 0)
		{
			error("Queue constructor failed, given non positive as capacity, has to be >= 1");
		}
		this->capacity = capacity;
		data = (type*) malloc(capacity * sizeof(type));
		if (data == nullptr)
		{
			error("Failed to allocate memory to construct Queue container");
		}
		count = 0;
	}

	// uniform initialization -  Queue<float> arr = { 2.3, 2.4 ... }
	Queue(const std::initializer_list<type> & il)
	{
		s64 size = il.size();
		capacity = INITIAL_CAPACITY;
		if (size > capacity)
		{
			capacity = size * 2;
		}
		data = (type*)malloc(capacity * sizeof(type));

		count = 0;
		for (auto & el : il)
		{
			new(data + count++) type(el);
		}
	}

	// add type element to the end
	void enqueue(type el)
	{
		if (count == capacity)
		{
			expandCapacity();
		}
		new(data + count++) type(std::move(el));
	}

	inline bool isEmpty() const
	{
		return count == 0;
	}

	inline s64 size() const
	{
		return count;
	}
}



#endif
