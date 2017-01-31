#ifndef _array_h
#define _array_h

#include <iostream>
#include <cstdlib> // malloc, free
#include "utility.h"

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
	
	// default -1 means that double capacity will be used
	void expandCapacity(s64 capacity = -1)
	{
		if (capacity > this->capacity) this->capacity = capacity;
		else if (capacity == -1) this->capacity *= 2;
		else return;

		type *new_data = (type*) malloc(this->capacity * sizeof(type));
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

public:

	Array()
	{
		capacity = INITIAL_CAPACITY;
		data = (type*) malloc(capacity * sizeof(type));
		if (data == nullptr) ERROR("Failed to allocate memory to construct Array object");
		count = 0;
	}

	// standard constructor just allocates memory, but does no object initialization (construction)
	// this constructor allocates and initializes capacity amount of objects to given value (default if non given)
	// it's more costly, but you can immediately use [] operator to get or set values within [0 : capacity) boundaries
	Array(s64 capacity, const type & value = type())
	{
		if (capacity <= 0) ERROR("Array constructor failed, given non positive as capacity, has to be >= 1");
		this->capacity = capacity;
		data = (type*) malloc(capacity * sizeof(type));
		if (data == nullptr) ERROR("Failed to allocate memory to construct Array object");

		count = 0;
		for (s64 i = 0; i < capacity; i++) new(data + count++) type(value);
	}

	// uniform initialization -  Array<float> arr = { 2.3, 2.4 ... }
	Array(const std::initializer_list<type> & il)
	{
		s64 size = il.size();
		capacity = INITIAL_CAPACITY;
		if (size > capacity) capacity = size * 2;
		data = (type*)malloc(capacity * sizeof(type));

		count = 0;
		for (auto & el : il) new(data + count++) type(el);
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
		count = 0;
		free(data);
	}

	// copy constructor
	Array(const Array<type> & arr)
	{
		capacity = arr.capacity;
		data = (type*) malloc(capacity * sizeof(type));
		if (data == nullptr) ERROR("Failed to allocate memory to copy Array object's data");

		count = arr.count;
		for (s64 i = 0; i < count; ++i) new(data + i) type(arr.data[i]);
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

	void reserve(s64 capacity)
	{
		if (capacity > this->capacity) expandCapacity(capacity);
	}

	// get or set element only through [] operator
	type & operator[](s64 position) const
	{
		if (position < 0 || position >= count)
			ERROR("%s (size %I64s) can't get element from %I64s position - out of range", typeid(*this).name(), this->count, position);
		return data[position];
	}

	// begin of iterator for auto range based loop
	type * begin() { return data; }

	// end of iterator for auto range based loop
	type * end() { return (data + count); }


	// type has to implement < operator
	// otherwhise have to explicitly give comparator of given type
	template <typename compareType = compare_less<type>>
	void sort(compareType compare = compare_less<type>())
	{
		if (count <= 1) return;

		for (s64 i = 1; i < count; ++i)
		{
			type el = std::move(data[i]);
			s64 j = i - 1;
			while (j >= 0 && compare(el, data[j]))
			{
				data[j + 1] = std::move(data[j]);
				--j;
			}
			data[j + 1] = std::move(el);
		}
	}

	// insert move type element to the end
	void insert(type el)
	{
		if (count == capacity)
		{
			expandCapacity();
		}
		new(data + count++) type(std::move(el));
	}

	// insert type element to the specified position
	void insert(type el, s64 position)
	{
		if (position < 0 || position > count)
		{
			ERROR("%s (size %I64s) can't insert element to %I64s position - out of range", typeid(*this).name(), this->count, position);
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
			ERROR("%s (size %I64s) can't remove element from %I64s position - out of range", typeid(*this).name(), this->count, position);

		if (ordered)
		{
			for (s64 i = position; i < count - 1; ++i)
			{
				data[i] = std::move(data[i + 1]);
			}
		}
		else data[position] = std::move(data[count - 1]);

		data[--count].~type();
	}

	// removes last element and returns it
	type pop()
	{
		if (this->count == 0) ERROR("%s is empty container: can't pop element", typeid(*this).name());
		type result = std::move(this->data[count-1]);
		this->remove(count-1);
		return result;
	}

	// returns the index of the first element which is equal to given value within start to end (not included)
	// returns -1 if not found or provided range is not whithin container boundaries
	// typename type has to implement == operator or provide comparator of given type
	template <typename compareType = compare_equal<type>>
	s64 find(const type & value, s64 start = 0, s64 end = -1, compareType compare = compare_equal<type>())
	{
		if (end == -1) end = this->count;
		if (start < 0 || start >= this->count || end <= start) return -1;

		for (s64 i = start; i < end; ++i)
		{
			if (compare(data[i], value)) return i;
		}
		return -1;
	}

	// returns copy of array from start to end (not included)
	Array<type> subArray(s64 start = 0, s64 end = -1) const
	{
		if (end == -1) end = this->count;
		if (start < 0 || start > this->count || end < start)
		{
			ERROR("Can't create subArray from %s (size %I64s), provided indexes %I64s - %I64s are out of range", typeid(*this).name(), this->count, start, end);
		}

		Array<type> result;
		for (s64 i = start; i < end; ++i)
		{
			result.insert(this->data[i]);
		}
		return result;
	}

	// removes all elements from this Array
	void clear()
	{
		destructInternalData();
		count = 0;
	}

	inline s64 size() const { return count; }
	inline bool isEmpty() const { return count == 0; }

	Array<type> & extend(const Array<type> & rhs)
	{
		for (const type & value : rhs)
			this->insert(value);
		return *this;
	}

	// appends rhs array to this and returns new array
	Array<type> operator+(const Array<type> & rhs)
	{
		Array<type> result(*this);
		return result.extend(rhs);
	}

	Array<type> & operator+=(const Array<type> & rhs) { return this->extend(rhs); }

	Array<type> & operator+=(const type & value)
	{
		this->insert(value);
		return *this;
	}

	bool operator==(const Array<type> & rhs) const
	{
		s64 size = this->size();
		if (size != rhs.size()) return false;

		for (s64 i = 0; i < size; ++i)
		{
			if (this[i] != rhs[i]) return false;
		}
		return true;
	}

	bool operator!=(const Array<type> & rhs) const { return !(*this == rhs); }

	// fills output stream with objects info contained in Array
	// those objects have to implement << operator in order to work
	friend std::ostream & operator<<(std::ostream & os, const Array<type> & arr)
	{
		s64 size = arr.count;
		if (typeid(type) == typeid(char))
		{
			for (s64 i = 0; i < size; ++i) os << arr.data[i];
			return os;
		}
		// typeid(var).name() returns type name of var
		os << typeid(arr).name() << " (size " << size << ") values: ";
		for (s64 i = 0; i < size; ++i) os << arr.data[i] << " ";
		return os;
	}
};


#endif