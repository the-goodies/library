#ifndef _utility_h
#define _utility_h

#include <cstdio>  // freopen, fprintf, stderr
#include <cstdlib> // exit, EXIT_FAILURE
#include <cstdarg> // vargs macros
#include <cstdint> // int8_t ... types
#include <cassert> // assert

typedef std::int8_t  s8;
typedef std::int16_t s16;
typedef std::int32_t s32;
typedef std::int64_t s64;

typedef std::uint8_t  u8;
typedef std::uint16_t u16;
typedef std::uint32_t u32;
typedef std::uint64_t u64;


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


#endif