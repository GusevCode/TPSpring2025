#include <iostream>
#include <cstddef>

long invert_bit(long x, long k)
{
	return x ^ (1L << k);
}

void run(std::istream& input, std::ostream& output)
{
	long n;
	long k;
	input >> n >> k;
	output << invert_bit(n, k) << std::endl;
}

int main()
{
	return run(std::cin, std::cout), 0;
}
