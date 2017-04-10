#ifndef _sorting_h
#define _sorting_h

#include "Array.h"
#include "utility.h" // swap function

// essentially just mergeSort with added counting how many inversions are done
template <typename type, typename compareType = compare_less<type>> // end included
s64 countingInversionsWrapper(Array<type> & arr, Array<type> & aux, s64 start, s64 end, compareType compare = compare_less<type>())
{
	if (start >= end) return 0;
	
	s64 mid = start + (end - start) / 2;
	s64 left  = countingInversionsWrapper(arr, aux, start, mid, compare);
	s64 right = countingInversionsWrapper(arr, aux, mid + 1, end, compare);


	s64 middle = 0;
	if (compare(arr[mid+1], arr[mid]))
	{
		s64 lhs = start;
		s64 rhs = mid + 1;
		s64 len = end - start + 1;
		for (s64 i = 0; i < len; ++i)
		{
			if (lhs == mid + 1)						aux[i] = std::move(arr[rhs++]);
			else if (rhs > end) 					aux[i] = std::move(arr[lhs++]);
			else if (!compare(arr[rhs], arr[lhs]))	aux[i] = std::move(arr[lhs++]);
			else 
			{
				aux[i] = std::move(arr[rhs++]);
				middle += mid - lhs + 1;
			}
		}
		// transfer merged sides from auxiliary array back to actual array
		for (s64 i = 0; i < len; ++i) arr[start + i] = std::move(aux[i]);		
	}
	return left + middle + right;
}


template <typename type, typename compareType = compare_less<type>>
s64 countingInversions(Array<type> & arr, compareType compare = compare_less<type>())
{
	Array<type> aux(arr.size());
	return countingInversionsWrapper(arr, aux, 0, arr.size() - 1, compare);
}


// type has to consist of non-negative integers
// radix is upper bound of those non-negative integers - [0, radix]
template <typename type>
void countingSort(Array<type> & arr, s64 radix)
{
	if (radix <= 0) return;

	s64 size = arr.size();
	if (size == 0) return;

	Array<s64> count(radix + 2, 0);
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

// NOTE: doesnt WORK because you cant assign a value properly to Array<char*> 
// all elements within arr have to be of the same length
void LSDSort(Array<char*> & arr, s64 length)
{
	s64 size = arr.size();
	if (size == 0) return;

	const u16 radix = 256;
	s64 count[radix + 1] = {0};
	char* aux = new char[size * length];

	for (s64 c = length - 1; c >= 0; --c)
	{
		// count the frequencies of chars within c position of each element in arr
		for (s64 i = 0; i < size; ++i) count[arr[i][c] + 1] += 1;
		// transform frequencies to indices, starting position for each number in sorted array
		for (s64 r = 1; r <= radix; ++r) count[r] += count[r-1];
		// put values in sorted order in auxillary array
		for (s64 i = 0; i < size; ++i)
		{
			char* location = (aux + length * count[arr[i][c]]++);
			for (s64 j = 0; j < length; ++j)
			{
				location[j] = arr[i][j];
			}
		}
		// copy back to real array
		for (s64 i = 0; i < size; ++i)
		{
			for (s64 j = 0; j < length; ++j)
			{
				arr[i][j] = *(aux + length * i + j);
			}
		}
		// clear count for next iteration
		for (s64 r = 0; r <= radix; ++r) count[r] = 0;
	}
	delete[] aux;
}


// NOTE: doesnt WORK because you cant assign a value properly to Array<char*> 
// all elements within arr have to be of the same length
void LSDSortSLOW(Array<char*> & arr, s64 length)
{
	s64 size = arr.size();
	if (size == 0) return;

	const u16 radix = 256;
	s64 count[radix + 1] = {0};
	char** aux = new char*[size];

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
	delete[] aux;
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
void quickSort(Array<type> & arr, s64 start, s64 end, Random64 & rng, compareType compare = compare_less<type>())
{
	// base case
	if (end - start <= 20)
	{
		for (s64 i = start + 1; i <= end; ++i)
		{
			type el = std::move(arr[i]);
			s64 j = i - 1;
			while (j >= start && compare(el, arr[j]))
			{
				arr[j + 1] = std::move(arr[j]);
				--j;
			}
			arr[j + 1] = std::move(el);
		}
		return;
	}

	// choose pivot
	s64 pivot_index = start + rng.random() % (end - start + 1);
	swap(arr[start], arr[pivot_index]);
	type pivot = arr[start];

	s64 lt = start;
	s64 gt = end + 1;
	while (true)
	{
		while (compare(arr[++lt], pivot));
		while (compare(pivot, arr[--gt]));
		if (lt >= gt) break;
		swap(arr[lt], arr[gt]);
	}
	swap(arr[start], arr[gt]);

	quickSort(arr, start, gt - 1, rng, compare);
	quickSort(arr, gt + 1, end, rng, compare);
}

template <typename type, typename compareType = compare_less<type>> // end included
void quickSort(Array<type> & arr, compareType compare = compare_less<type>())
{
	// does a single pass through array and if no exchanges are made returns true
	bool exchanged = false;
	const s64 size = arr.size();
	for (s64 i = 0; i < size - 1; ++i)
	{
		if (compare(arr[i+1], arr[i]))
		{
			swap(arr[i+1], arr[i]);
			exchanged = true;
		}
	}
	if (!exchanged) return;

	Random64 rng;
	quickSort(arr, 0, arr.size() - 2, rng, compare);
}


template <typename type, typename compareType = compare_less<type>> // end included
void quickSort3Way(Array<type> & arr, s64 start, s64 end, Random64 & rng, compareType compare = compare_less<type>())
{
	// base case
	if (end - start <= 256)
	{
		for (s64 i = start + 1; i <= end; ++i)
		{
			type el = std::move(arr[i]);
			s64 j = i - 1;
			while (j >= start && compare(el, arr[j]))
			{
				arr[j + 1] = std::move(arr[j]);
				--j;
			}
			arr[j + 1] = std::move(el);
		}
		return;
	}

	// choose pivot
	s64 pivot_index = start + rng.random() % (end - start + 1);
	swap(arr[start], arr[pivot_index]);
	type pivot = arr[start];


	s64 lt = start;
	s64 gt = end;
	s64 pos = start + 1;
	while (pos <= gt)
	{
		if (compare(arr[pos], pivot)) swap(arr[pos++], arr[lt++]);
		else if (compare(pivot, arr[pos])) swap(arr[pos], arr[gt--]);
		else ++pos;
	}
	quickSort3Way(arr, start, lt - 1, rng, compare);
	quickSort3Way(arr, gt + 1, end, rng, compare);
}

template <typename type, typename compareType = compare_less<type>> // end included
void quickSort3Way(Array<type> & arr, compareType compare = compare_less<type>())
{
	Random64 rng;
	quickSort3Way(arr, 0, arr.size() - 1, rng, compare);
}



template <typename type, typename compareType = compare_less<type>> // end included
void mergeSort(Array<type> & arr, type* aux, s64 start, s64 end, compareType compare = compare_less<type>())
{
	if (end - start <= 200) 
	{
		for (s64 i = start + 1; i <= end; ++i)
		{
			type el = std::move(arr[i]);
			s64 j = i - 1;
			while (j >= start && compare(el, arr[j]))
			{
				arr[j + 1] = std::move(arr[j]);
				--j;
			}
			arr[j + 1] = std::move(el);
		}
		return;
	}

	s64 mid = start + (end - start) / 2;
	mergeSort(arr, aux, start, mid, compare);
	mergeSort(arr, aux, mid + 1, end, compare);

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
	type* aux = new type[arr.size()];
	mergeSort(arr, aux, 0, arr.size() - 1, compare);
	delete[] aux;
}

template <typename type, typename compareType = compare_less<type>>
bool isSorted(const Array<type> & arr, compareType compare = compare_less<type>())
{
	s64 size = arr.size();
	for (s64 i = 0; i < size - 1; ++i)
		if (compare(arr[i+1], arr[i])) return false;
	return true;
}

template <typename type, typename compareType = compare_less<type>>
type select(const Array<type> & arr, s64 k, s64 start, s64 end, Random64 & rng, compareType compare = compare_less<type>())
{
	if (end - start == 0) return arr[start];
	// choose pivot
	s64 pivot_index = start + rng.random() % (end - start + 1);
	swap(arr[start], arr[pivot_index]);
	type pivot = arr[start];

	s64 seperator = start + 1;
	for (s64 pos = start + 1; pos <= end; ++pos)
	{
		if (compare(arr[pos], pivot)) swap(arr[pos], arr[seperator++]);
	}
	swap(arr[start], arr[seperator-1]);

	if (seperator-1 == k) return arr[k];
	else if (seperator-1 < k) return select(arr, k, seperator, end, rng, compare);
	else return select(arr, k, start, seperator - 2, rng, compare);
}

template <typename type, typename compareType = compare_less<type>>
type select(const Array<type> & arr, s64 k, compareType compare = compare_less<type>())
{
	Random64 rng;
	return select(arr, k, 0, arr.size() - 1, rng, compare);
}

template <typename type>
void shuffle(Array<type> & arr)
{
	thread_local Random64 rng;
	for (s64 pos = arr.size() - 1; pos > 0; --pos)
	{
		s64 rand_pos = rng.random() % (pos + 1);
		swap(arr[pos], arr[rand_pos]);
	}
}



#endif