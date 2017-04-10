#ifndef _utility_h
#define _utility_h

#include <cstdio>  // freopen, fprintf, stderr
#include <cstdlib> // exit, EXIT_FAILURE
#include <cstdarg> // vargs macros
#include <cstdint> // int8_t ... types
#include <cassert> // assert
#include <chrono> // for getTimeElapsed function


typedef std::int8_t  s8;
typedef std::int16_t s16;
typedef std::int32_t s32;
typedef std::int64_t s64;

typedef std::uint8_t  u8;
typedef std::uint16_t u16;
typedef std::uint32_t u32;
typedef std::uint64_t u64;

#undef ERROR
#define ERROR(MESSAGE, ...) error(__FILE__, __LINE__, MESSAGE, __VA_ARGS__)
// reports msg to error stream and exits program
// utilizes variable arguments (vargs)
static void error(char *file, int line, char *msg, ...)
{
	va_list argp;
	va_start(argp, msg);

	FILE *log;
	fopen_s(&log, "error.txt", "a");
	fprintf(log, "| %s | %s | ", __DATE__, __TIME__);
	fprintf(log, "%s %d | ", file, line);
	vfprintf(log, msg, argp);
	fprintf(log, "\n");

	fclose(log);
	va_end(argp);
	exit(EXIT_FAILURE);
}


template <typename type>
static void swap(type & a, type & b)
{
	type temp = std::move(a);
	a = std::move(b);
	b = std::move(temp);
}

template <typename compareType>
struct compare_less
{
	constexpr bool operator()(const compareType & lhs, const compareType & rhs) const
		{ return lhs < rhs; }
};

template <typename compareType>
struct compare_greater
{
	constexpr bool operator()(const compareType & lhs, const compareType & rhs) const
		{ return lhs > rhs; }
};

template <typename compareType>
struct compare_equal
{
	constexpr bool operator()(const compareType & lhs, const compareType & rhs) const
		{ return lhs == rhs; }
};

template <typename compareType>
struct compare_to
{
	int operator()(const compareType & lhs, const compareType & rhs) const
	{
		if (lhs > rhs) return 1;
		else if (lhs == rhs) return 0;
		else return -1; // lhs < rhs
	}
};

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

// first time called sets the timer and returns 0
// second time returns elapsed time (second call time - first call time) in milliseconds
// third time and so on repeats this cycle
static double getTimeElapsed()
{
	using get_time = std::chrono::steady_clock;
	thread_local get_time::time_point start_time;
	thread_local bool start = true;
	if (start)
	{
		start_time = get_time::now();
		start = !start;
		return 0.0;
	}
	else
	{
		auto end_time = get_time::now();
		start = !start;
		return double(std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count());
	}
}

inline static bool is_overflow_add(u64 a, u64 b)
{
	u64 result = a + b;
	return a > result;
}

inline static bool is_overflow_sub(u64 a, u64 b)
{
	return b > a;
}

inline static bool is_overflow_add(s64 a, s64 b)
{
	s64 result = a + b;
	bool negative_overflow = (a < 0) && (b < 0) && (result >= 0);
	bool positive_overflow = (a >= 0) && (b >= 0) && (result < 0);
	return (negative_overflow || positive_overflow);
}


inline static bool is_overflow_sub(s64 a, s64 b)
{
	if (b != INT64_MIN) return is_overflow_add(a, -b);
	// have to handle special case when b == -2^63
	return a >= 0;
}


/*
 * Code taken and adapted for my needs from
 * PCG Random Number Generation for C.
 * http://www.pcg-random.org
 */
class Random32
{
	u64 state;	// RNG state.  All values are possible.
	u64 inc;	// Controls which RNG sequence (stream) is
				// selected. Must *always* be odd.
public:
	Random32()
	{
		seed(time(NULL), (intptr_t)this);
	}

	Random32(u64 initstate, u64 initseq)
	{
		seed(initstate, initseq);
	}

	void seed(u64 initstate, u64 initseq)
	{
		this->state = 0U;
		this->inc = (initseq << 1u) | 1u;
		random();
		this->state += initstate;
		random();
	}

	u32 random()
	{
		u64 oldstate = this->state;
		this->state = this->state * 6364136223846793005ULL + this->inc;
		u32 xorshifted = (u32)(((oldstate >> 18u) ^ oldstate) >> 27u);
		u32 rot = oldstate >> 59u;
		return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
	}

	// Generate a uniformly distributed number, num, where start <= num <= end
	s32 uniform(s32 start, s32 end)
	{
		if (start > end) ERROR("Random32: wrong values passed to uniform method - start can't be higher than end");
		// special case
		if (start == INT32_MIN && end == INT32_MAX) return (s32)(start + random());

		u32 bound = (u32)(end - start + 1);
		// To avoid bias, we need to make the range of the RNG a multiple of
		// bound, which we do by dropping output less than a threshold.
		// A naive scheme to calculate the threshold would be to do
		//
		//     u32 threshold = 0x100000000ull % bound;
		//
		// but 64-bit div/mod is slower than 32-bit div/mod (especially on
		// 32-bit platforms).  In essence, we do
		//
		//     u32 threshold = (0x100000000ull-bound) % bound;
		//
		// because this version will calculate the same modulus, but the LHS
		// value is less than 2^32.
		u32 threshold = -bound % bound;

		// Uniformity guarantees that this loop will terminate.  In practice, it
		// should usually terminate quickly; on average (assuming all bounds are
		// equally likely), 82.25% of the time, we can expect it to require just
		// one iteration.  In the worst case, someone passes a bound of 2^31 + 1
		// (i.e., 2147483649), which invalidates almost 50% of the range.  In 
		// practice, bounds are typically small and only a tiny amount of the range
		// is eliminated.
		while(true)
		{
			u32 num = random();
			if (num >= threshold)
			{
				num %= bound;
				return (s32)(start + num);
			}
		}
	}
};


class Random64
{
	Random32 rng[2];

public:
	Random64() { /* empty */ }
	Random64(u64 state1, u64 state2, u64 seq1, u64 seq2): rng{ {state1, seq1}, {state2, seq2} } { /* empty */ }

	void seed(u64 state1, u64 state2, u64 seq1, u64 seq2)
	{
		rng[0].seed(state1, seq1);
		rng[1].seed(state2, seq2);
	}

	u64 random() { return (((u64)rng[1].random() << 32) | rng[0].random()); }

	// Generate a uniformly distributed number, num, where start <= num <= end
	s64 uniform(s64 start, s64 end)
	{
		if (start > end) ERROR("Random64: wrong values passed to uniform method - start can't be higher than end");
		// special case
		if (start == INT64_MIN && end == INT64_MAX) return (s64)(start + random());

		u64 bound = (u64)(end - start + 1);
		u64 threshold = -bound % bound;

		while(true)
		{
			u64 num = random();
			if (num >= threshold)
			{
				num %= bound;
				return (s64)(start + num);
			}
		}
	}
};

#endif