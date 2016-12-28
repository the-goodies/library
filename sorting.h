#ifndef _sorting_h
#define _sorting_h

#include <random>
#include "Array.h"
#include "utility.h" // swap function


// type has to consist of non-negative integers
// radix is upper bound of those non-negative integers - [0, radix)
template <typename type, typename compareType = compare_less<type>>
void countingSort(Array<type> & arr, u64 radix, compareType compare = compare_less<type>())
{
	s64 size = arr.size();
	if (size == 0) return;

	Array<s64> count(radix + 1, 0);
	Array<type> aux(size, 0);

	// count the frequencies of numbers in arr
	for (s64 i = 0; i < size; ++i) count[arr[i] + 1] += 1;
	// transform frequencies to indices, starting position for each number in sorted array
	for (s64 r = 1; r <= radix; ++r) count[r] += count[r-1];
	// put values in sorted order in auxillary array
	for (s64 i = 0; i < size; ++i) aux[count[arr[i]]++] = arr[i];
	// copy back to real array
	for (s64 i = 0; i < size; ++i) arr[i] = aux[i];
}

// all elements within arr have to be of the same length
template <typename compareType = compare_less<type>>
void LSDSort(Array<char*> & arr, compareType compare = compare_less<type>())
{
	s64 size = arr.size();
	if (size == 0) return;

	char* element = arr[0];
	s64 length = -1;
	while (element[++length] != '\0');

	u16 radix = 256;
	Array<s64> count(radix + 1, 0);
	Array<char> aux(size, 0);

	for (s64 c = length - 1; c >= 0; --c)
	{
		// count the frequencies of chars within c position of each element in arr
		for (s64 i = 0; i < size; ++i) count[arr[i][c] + 1] += 1;
		// transform frequencies to indices, starting position for each number in sorted array
		for (s64 r = 1; r <= radix; ++r) count[r] += count[r-1];
		// put values in sorted order in auxillary array
		for (s64 i = 0; i < size; ++i) aux[count[arr[i][c]]++] = arr[i];
		// copy back to real array
		for (s64 i = 0; i < size; ++i) arr[i] = aux[i];

		// clear count for next iteration
		for (s64 r = 0; r <= radix; ++r) count[r] = 0;
	}
}

enum class outOfThree { FIRST = 1, SECOND = 2, THIRD = 3 };
template <typename type, typename compareType = compare_less<type>>
outOfThree medianOfThree(type & a, type & b, type & c, compareType compare = compare_less<type>())
{
	if (compare(a, b))
	{
		if (compare(b, c)) 		return outOfThree::SECOND;
		else
		{
			if (compare(a, c)) 	return outOfThree::THIRD;
			else				return outOfThree::FIRST;
		}
	}
	else
	{
		if (compare(a, c)) 		return outOfThree::FIRST;
		else
		{
			if (compare(b, c))	return outOfThree::THIRD;
			else				return outOfThree::SECOND;
		}
	}
}


template <typename type, typename compareType = compare_less<type>>
void insertionSort(Array<type> & arr, compareType compare = compare_less<type>())
{
	s64 size = arr.size();
	if (size <= 1) return;

	for (s64 i = 1; i < size; ++i)
	{
		type el = std::move(arr[i]);
		s64 j = i - 1;
		while (j >= 0 && compare(el, arr[j]))
		{
			arr[j + 1] = std::move(arr[j]);
			--j;
		}
		arr[j + 1] = std::move(el);
	}
}

template <typename type, typename compareType = compare_less<type>> // end included
void quickSortWrapper(Array<type> & arr, s64 start, s64 end, compareType compare = compare_less<type>())
{
	// base case
	if (end <= start) return;

	// choose pivot
	s64 mid = start + (end - start) / 2;
	switch (medianOfThree(arr[start], arr[mid], arr[end], compare))
	{
		case outOfThree::SECOND: swap(arr[start], arr[mid]); break;
		case outOfThree::THIRD: swap(arr[start], arr[end]); break;
		case outOfThree::FIRST: break;
	}
	type & pivot = arr[start];

	// partition step
	s64 seperator = start + 1; // arr[start..seperator-1] < pivot and arr[seperator..end-1] >= pivot
	for (s64 pos = start + 1; pos <= end; ++pos)
	{
		if (compare(arr[pos], pivot)) swap(arr[seperator++], arr[pos]);
	}
	swap(arr[start], arr[seperator-1]);

	// recursive case
	quickSortWrapper(arr, start, seperator-2, compare); // pivot position is seperator-1
	quickSortWrapper(arr, seperator, end, compare);
}

template <typename type, typename compareType = compare_less<type>>
void quickSort(Array<type> & arr, compareType compare = compare_less<type>()) 
	{ quickSortWrapper(arr, 0, arr.size() - 1, compare); }

template <typename type, typename compareType = compare_less<type>> // end included
void mergeSortWrapper(Array<type> & arr, Array<type> & aux, s64 start, s64 end, compareType compare = compare_less<type>())
{
	if (start >= end) return;

	s64 mid = start + (end - start) / 2;
	mergeSortWrapper(arr, aux, start, mid, compare);
	mergeSortWrapper(arr, aux, mid + 1, end, compare);

	// optimize for when both sides are already sorted to skip merge
	// if whole array is sorted, skipping merge makes sorting linear
	if (compare(arr[mid+1], arr[mid]))
	{
		// merge step
		s64 lhs = start;
		s64 rhs = mid + 1;
		s64 len = end - start + 1;
		for (s64 i = 0; i < len; ++i)
		{
			if (lhs == mid + 1)						aux[i] = std::move(arr[rhs++]);
			else if (rhs > end) 					aux[i] = std::move(arr[lhs++]);
			else if (compare(arr[rhs], arr[lhs]))	aux[i] = std::move(arr[rhs++]);
			else 									aux[i] = std::move(arr[lhs++]);
		}
		// transfer merged sides from auxiliary array back to actual array
		for (s64 i = 0; i < len; ++i) arr[start + i] = std::move(aux[i]);
	}
}

template <typename type, typename compareType = compare_less<type>>
void mergeSort(Array<type> & arr, compareType compare = compare_less<type>())
{
	Array<type> aux(arr.size());
	mergeSortWrapper(arr, aux, 0, arr.size() - 1, compare);
}

template <typename type, typename compareType = compare_less<type>>
bool isSorted(const Array<type> & arr, compareType compare = compare_less<type>())
{
	s64 size = arr.size();
	for (s64 i = 0; i < size - 1; ++i)
		if (compare(arr[i+1], arr[i])) return false;
	return true;
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