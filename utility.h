#ifndef _utility_h
#define _utility_h

template <typename type>
void swap(type & a, type & b)
{
	type temp = std::move(a);
	a = std::move(b);
	b = std::move(temp);
}

#endif