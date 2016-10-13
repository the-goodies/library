#ifndef _stack_h
#define _stack_h

#include <cstdint> // int8_t ... types
#include <cstdlib> // snprintf, malloc, free
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
class Stack
{
	struct Cell
	{
		type data;
		Cell *next;
	};

	s64 count; // how many elements in a linked list
	Cell *first; // pointer to a first element

	// recursively destructs all objects within linked list container
	void destructInternalData(Cell *list)
	{
		if (list->next != nullptr) destructInternalData(list->next);
		list->~Cell(); // manually destruct object, which will call data.~type() destructor
		free(list); // then free memory
	}

	// Creates new Cell with el as data and add's to the top of linked list
	void pushElement(type el)
	{
		Cell *new_cell = (Cell*)malloc(sizeof(Cell));
		new_cell->data = std::move(el);
		new_cell->next = first;
		first = new_cell;
		++count;
	}

	// recursively copy data from list to this
	void copyInternalData(Cell *list) const
	{
		if (list->next != nullptr) copyInternalData(list->next);
		pushElement(list->data);
	}

	// same as copyInternalData, just moves data from list instead of copying
	void moveInternalData(Cell *list)
	{
		if (list->next != nullptr) moveInternalData(list->next);
		pushElement(std::move(list->data));
	}

public:

	Stack()
	{
		first = nullptr;
		count = 0;
	}

	// uniform initialization -  Stack<float> stack = { 2.3, 2.4 ... }
	Stack(const std::initializer_list<type> & il)
	{
		first = nullptr;
		count = 0;
		for (auto & el : il) pushElement(el);
	}

	~Stack()
	{
		if (first != nullptr) destructInternalData(first);
	}

	// copy constructor
	Stack(const Stack<type> & stack)
	{
		first = nullptr;
		count = 0;
		if (stack.first != nullptr) copyInternalData(stack.first);
	}

	// move constructor
	Stack(Stack<type> && stack)
	{
		first = nullptr;
		count = 0;
		if (stack.first != nullptr) moveInternalData(stack.first);
	}

	// copy/move assignment utilizing copy/move constructor by taking argument as value
	Stack & operator=(Stack<type> stack)
	{
		destructInternalData(this->first);
		count = 0;
		if (stack.first != nullptr) moveInternalData(stack.first);

		return *this;
	}

	inline bool isEmpty() const
	{
		return first == nullptr;
	}

	inline s64 size() const
	{
		return count;
	}

	// add element to the top
	void push(type el)
	{
		pushElement(std::move(el));
	}

	// removes last element and returns it
	type pop()
	{
		if (isEmpty()) ERROR("Can't remove element from empty Stack");
		Cell *old_cell = first;
		type result = std::move(first->data);
		first = first->next;
		// even though we moved data, if type object doesnt implement move constructor, then we are
		// copying said data, leaving it untouched, therefore we have to explicitly destruct it
		old_cell->~Cell();
		free(old_cell);
		--count;

		return result;
	}

	// returns first element without removing it
	type peek() const
	{
		if (isEmpty()) ERROR("Can't remove element from empty Stack");
		return first->data;
	}

	// removes all elements from this Stack
	void clear()
	{
		destructInternalData(this->first);
		first = nullptr;
		count = 0;
	}
};

#endif