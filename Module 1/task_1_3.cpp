#include <iostream>
#include <cstddef>

size_t count_bits(long x) 
{
	size_t count = 0;
	while (x > 0)
	{
		if (x & 1)
			++count;
		x >>= 1;
	}
	return count;
}

void run(std::istream& input, std::ostream& output)
{
	long n;
	input >> n;
	output << (count_bits(n) == 1 ? "OK" : "FAIL") << std::endl;
}

int main()
{
	return run(std::cin, std::cout), 0;
}
