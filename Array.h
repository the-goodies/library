#ifndef _array_h
#define _array_h

#include <cstdint> // int8_t ... types
#include <cstdlib> // malloc, free
#include "error.h"
#include <iostream>



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
class Array
{
	s64 capacity; // allocated size
	s64 count; // effective size
	type *data; // container holding data

	// will double everytime reaches limit
	static const u32 INITIAL_CAPACITY = 64;
	
	// friend definitions
	friend class BigInt;

	// private functions

	// destructs all objects within container
	void destructInternalData()
	{
		// call destructor manually on all objects, since allocation and construction is done seperately
		for (s64 i = 0; i < count; ++i)
		{
			data[i].~type();
		}
	}

	void expandCapacity()
	{
		capacity *= 2;
		type *new_data = (type*) malloc(capacity * sizeof(type));
		if (new_data == nullptr)
		{
			ERROR("Failed to allocate memory to expand Array object");
		}

		for (s64 i = 0; i < count; ++i)
		{
			new(new_data + i) type(std::move(data[i]));
		}
		// just in case objects being moved only have copy constructor, so we have to explicitly destruct them afterwards
		destructInternalData();
		free(data);

		data = new_data;
	}

	// sort functions
	// insertionSort
	void sortIncreasing()
	{
		if (count <= 1) return;

		for (s64 i = 1; i < count; ++i)
		{
			type el = std::move(data[i]);
			s64 j = i - 1;
			while (j >= 0 && data[j] > el)
			{
				data[j + 1] = std::move(data[j]);
				--j;
			}
			data[j + 1] = std::move(el);
		}
	}

	void sortDecreasing()
	{
		if (count <= 1) return;

		for (s64 i = 1; i < count; ++i)
		{
			type el = std::move(data[i]);
			s64 j = i - 1;
			while (j >= 0 && el > data[j])
			{
				data[j + 1] = std::move(data[j]);
				--j;
			}
			data[j + 1] = std::move(el);
		}
	}

public:

	Array()
	{
		capacity = INITIAL_CAPACITY;
		data = (type*) malloc(capacity * sizeof(type));
		if (data == nullptr)
		{
			ERROR("Failed to allocate memory to construct Array object");
		}
		count = 0;
	}


	// standard constructor just allocates memory, but does no object initialization (construction)
	// this constructor allocates and initializes capacity amount of objects to given value (default if non given)
	// it's more costly, but you can immediately use [] operator to get or set values within [0 : capacity) boundaries
	Array(s64 capacity, type value = type())
	{
		if (capacity <= 0)
		{
			ERROR("Array constructor failed, given non positive as capacity, has to be >= 1");
		}
		this->capacity = capacity;
		data = (type*) malloc(capacity * sizeof(type));
		if (data == nullptr)
		{
			ERROR("Failed to allocate memory to construct Array object");
		}
		count = 0;
		for (s64 i = 0; i < capacity; i++)
		{
			new(data + count++) type(value);	
		}
	}

	// uniform initialization -  Array<float> arr = { 2.3, 2.4 ... }
	Array(const std::initializer_list<type> & il)
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

	// for Array<char> initialization, that could be used like string
	Array(const char *str)
	{
		// find out str size not including NULL
		count = -1;
		while (str[++count] != NULL);

		capacity = INITIAL_CAPACITY;
		if (count > capacity) capacity = count * 2;
		data = (char*)malloc(capacity * sizeof(char));

		for (s64 i = 0; i < count; ++i) data[i] = str[i];
	}

	~Array()
	{
		destructInternalData();
		free(data);
	}

	// copy constructor
	Array(const Array<type> & arr)
	{
		capacity = arr.capacity;
		data = (type*) malloc(capacity * sizeof(type));
		if (data == nullptr)
		{
			ERROR("Failed to allocate memory to copy Array object's data");
		}

		count = arr.count;
		for (s64 i = 0; i < count; ++i)
		{
			new(data + i) type(arr.data[i]);
		}
	}

	// move constructor
	Array(Array<type> && arr)
	{
		capacity = arr.capacity;
		count = arr.count;
		data = arr.data;

		arr.data = nullptr;
		arr.count = 0;
		arr.capacity = 0;
	}

	// copy/move assignment utilizing copy/move constructor by taking argument as value
	Array & operator=(Array<type> arr)
	{
		// swap
		s64 capacity_temp = capacity;
		s64 count_temp = count;
		type* data_temp = data;

		capacity = arr.capacity;
		count = arr.count;
		data = arr.data;

		arr.capacity = capacity_temp;
		arr.count = count_temp;
		arr.data = data_temp;
		// arr going out of scope will destruct old Array's object's data

		return *this;
	}

	// get or set element only through [] operator
	type & operator[](s64 position) const
	{
		if (position < 0 || position >= count)
		{
			ERROR("%s (size %I64u) can't get element from %I64u position - out of range", typeid(*this).name(), this->count, position);
		}
		return data[position];
	}

	// begin of iterator for auto range based loop
	type * begin()
	{
		return data;
	}

	// end of iterator for auto range based loop
	type * end()
	{
		return (data + count);
	}


	// type object has to implement < and > (for reversed sort) operators
	void sort(bool reversed = false)
	{
		if (!reversed)
		{
			sortIncreasing();
		}
		else
		{
			sortDecreasing();
		}
	}

	// add move type element to the end
	void add(type el)
	{
		if (count == capacity)
		{
			expandCapacity();
		}
		new(data + count++) type(std::move(el));
	}

	// add type element to the specified position
	void add(type el, s64 position)
	{
		if (position < 0 || position > count)
		{
			ERROR("%s (size %I64u) can't add element to %I64u position - out of range", typeid(*this).name(), this->count, position);
		}
		if (capacity == count) expandCapacity();

		if (position == count)
		{
			new(data + count++) type(std::move(el));
			return;
		}

		new(data + count) type(std::move(data[count-1]));
		for (s64 i = count - 2; i >= position; --i)
		{
			data[i + 1] = std::move(data[i]);
		}
		data[position] = std::move(el);
		count++;
	}

	// removes type element at specified position
	// ordered preserves order (default), but removal is slower, having to transfer majority of elements: [1, 2, 3, 4] remove from pos 1 -> [1, 3, 4]
	// not ordered, moves last element to the place of removal, making it very fast removal: [1, 2, 3, 4] remove from pos 1 -> [1, 4, 3] 
	void remove(s64 position, bool ordered = true)
	{
		if (position < 0 || position >= count)
		{
			ERROR("%s (size %I64u) can't remove element from %I64u position - out of range", typeid(*this).name(), this->count, position);
		}

		if (ordered)
		{
			for (s64 i = position; i < count - 1; ++i)
			{
				data[i] = std::move(data[i + 1]);
			}
		}
		else
		{
			data[position] = std::move(data[count - 1]);
		}
		data[--count].~type();
	}

	// returns copy of array from start to end (not included)
	Array<type> subArray(s64 start = 0, s64 end = -1) const
	{
		if (end == -1) end = this->count;
		if (start < 0 || start > this->count || end < start)
		{
			ERROR("Can't create subArray from %s (size %I64u), provided indexes %I64s - %I64s are out of range", typeid(*this).name(), this->count, start, end);
		}

		Array<type> result;
		for (s64 i = start; i < end; ++i)
		{
			result.add(this->data[i]);
		}
		return result;
	}

	// removes all elements from this Array
	void clear()
	{
		destructInternalData();
		count = 0;
	}

	inline s64 size() const
	{
		return count;
	}

	// fills output stream with objects info contained in Array
	// those objects have to implement << operator in order to work
	friend std::ostream & operator<<(std::ostream & os, const Array<type> & arr)
	{
		s64 size = arr.count;
		if (typeid(type) == typeid(char))
		{
			for (s64 i = 0; i < size; ++i)
			{
				os << arr.data[i];
			}
			return os;
		}
		// typeid(var).name() returns type name of var
		os << typeid(arr).name() << " (size " << size << ") objects:\n";
		for (s64 i = 0; i < size; ++i)
		{
			os << i << ") " << arr.data[i] << '\n';
		}
		return os;
	}
};


#endif