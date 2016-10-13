#ifndef _sorting_h
#define _sorting_h

#include <cstdint>
#include <random>
#include "utility.h" // swap function
#include "Array.h"

typedef std::int8_t  s8;
typedef std::int16_t s16;
typedef std::int32_t s32;
typedef std::int64_t s64;


enum class outOfThree { FIRST = 1, SECOND, THIRD };
template <typename type>
// returns -1 if a, 0 if b, +1 if c
outOfThree medianOfThree(type & a, type & b, type & c)
{
	if (a > b)
	{
		if (a < c)
		{
			return outOfThree::FIRST;
		}
		else
		{
			if (b > c) return outOfThree::SECOND;
			else 	   return outOfThree::THIRD;
		}
	}
	else
	{
		if (b < c)
		{
			return outOfThree::SECOND;
		}
		else
		{
			if (a > c) return outOfThree::FIRST;
			else 	   return outOfThree::THIRD;
		}
	}
}

template <typename type>
void insertionSort(Array<type> arr)
{
	s64 size = arr.size();
	if (size <= 1) return;

	for (s64 i = 1; i < size; ++i)
	{
		type el = std::move(arr[i]);
		s64 j = i - 1;
		while (j >= 0 && arr[j] > el)
		{
			arr[j + 1] = std::move(arr[j]);
			--j;
		}
		arr[j + 1] = std::move(el);
	}
}

template <typename type>
void quickSortWrapper(Array<type> & arr, s64 start, s64 end) // end included
{
	// base case
	if (end <= start) return;

	// choose pivot
	s64 mid = start + (end - start) / 2;
	switch (medianOfThree(arr[start], arr[mid], arr[end]))
	{
		case outOfThree::SECOND:
			swap(arr[start], arr[mid]);
			break;
		case outOfThree::THIRD:
			swap(arr[start], arr[end]);
			break;
		case outOfThree::FIRST: break;
	}
	type & pivot = arr[start];

	// partition step
	s64 seperator = start + 1; // arr[start..seperator-1] < pivot and arr[seperator..end-1] >= pivot
	for (s64 pos = start + 1; pos <= end; ++pos)
	{
		if (arr[pos] < pivot) swap(arr[seperator++], arr[pos]);
	}
	swap(arr[start], arr[seperator-1]);

	// recursive case
	quickSortWrapper(arr, start, seperator-2); // pivot position is seperator-1
	quickSortWrapper(arr, seperator, end);
}

template <typename type>
void quickSort(Array<type> & arr)
{
	quickSortWrapper(arr, 0, arr.size() - 1);
}

template <typename type>
void mergeSortWrapper(Array<type> & arr, Array<type> & aux, s64 start, s64 end) // end included
{
	if (start >= end) return;

	s64 mid = start + (end - start) / 2;
	mergeSortWrapper(arr, aux, start, mid);
	mergeSortWrapper(arr, aux, mid + 1, end);

	// merge step
	s64 lhs = start;
	s64 rhs = mid + 1;
	s64 len = end - start + 1;
	for (s64 i = 0; i < len; ++i)
	{
		if (lhs == mid + 1)
		{
			aux[i] = std::move(arr[rhs++]);
		}
		else if (rhs > end)
		{
			aux[i] = std::move(arr[lhs++]);
		}
		else if (arr[lhs] < arr[rhs])
		{
			aux[i] = std::move(arr[lhs++]);
		}
		else
		{
			aux[i] = std::move(arr[rhs++]);
		}
	}
	// transfer merged sides from auxiliary array to actual array
	for (s64 i = 0; i < len; ++i)
	{
		arr[start + i] = std::move(aux[i]);
	}
}

template <typename type>
void mergeSort(Array<type> & arr)
{
	Array<type> aux(arr.size());
	mergeSortWrapper(arr, aux, 0, arr.size() - 1);
}

template <typename type>
void shuffle(Array<type> & arr)
{
	std::random_device rd;
	std::mt19937_64 generator(rd());
	for (s64 pos = arr.size() - 1; pos > 0; --pos)
	{
		std::uniform_int_distribution<s64> distribution(0, pos);
		s64 rand_pos = distribution(generator());
		swap(arr[pos], arr[rand_pos]);
	}
}



#endif