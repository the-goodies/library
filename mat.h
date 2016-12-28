#ifndef _mat_h
#define _mat_h

#include "utility.h"


namespace mat
{

	float abs(float num)
	{
		// removes negative sign
		int n = *reinterpret_cast<int *>(&num);
		n &= 0x7FFFFFFF;
		return *reinterpret_cast<float *>(&n);
	}

	// calculate exp using taylor series
	float exp(float x)
	{
		float term = 1;
		float sum = 1;
		bool negative = false;
		if (x < 0)
		{
			negative = true;
			x = -x;
		}
		for (int n = 1; n < 100; ++n)
		{
			term *= x / n;
			sum += term;
		}

		return negative ? 1 / sum : sum;
	}

	// calculates natural log using newtons method, same as sqrt
	// very slow, since it uses exp function and initial guess is often off
	float ln(float num)
	{
		assert(num > 0);

		float guess = 100;
		float epsilon = 0.00001f;
		while (true)
		{
			float new_guess = guess - 1 + (num / exp(guess));

			if (mat::abs(new_guess - guess) <= epsilon) return new_guess;
			guess = new_guess;
		}
	}



	// returns num^power
	float powf(float num, int p = 2)
	{
		bool negative = false;
		if (p == 0) return 1;
		if (p < 0)
		{
			negative = true;
			p = -p;
		}

		float ans = 1;
		for (int i = 0; i < p; ++i)
		{
			ans *= num;
		}

		return negative ? (1.0f / ans) : (ans);
	}

	// returns num^power
	// only non-negative power
	s64 pow(s64 num, u64 p = 2)
	{
		if (p == 0) return 1;
		if (p == 1) return num;

		if (p % 2) return num * pow(num * num, p/2);
		else return pow(num * num, p/2);
	}

	// taylor series similar to e^x
	float powf(float num, float x)
	{
		float term = 1;
		float sum = 1;
		float ln_times_x = mat::ln(num) * x;

		return exp(ln_times_x);
	}



	// newtons method generalized for any integer power
	float sqrt(float num, int p = 2)
	{
		if (num == 0.0f) return 0;

		float guess = num / 2;
		float epsilon = 0.00001f;
		while (true)
		{
			float new_guess = (1.0f / p) * ((p - 1) * guess + (num / mat::pow(guess, p - 1)));

			if (mat::abs(new_guess - guess) <= epsilon) return new_guess;
			guess = new_guess;
		}
	}

	// returns how many digits number has
	s64 numberOfDigits(s64 num)
	{
		// -2^63 * (-1) = -2^63 because of complement two representation
		const s64 MIN_VALUE = -9223372036854775808LL;
		if (num == MIN_VALUE) return 19;
		if (num < 0) num *= -1;
		if (num <= 9) return 1;

		s64 count = 0;
		while(num != 0)
		{
			++count;
			num /= 10;
		}
		return count;
	}
}

#endif