#include <iostream>
#include <cstddef>

size_t count_unary_bits_on_odd_positions(int x) 
{
	size_t result = 0;
	while (x > 0)
	{
		if (x & 1)
		{
			++result;
		}
		x >>= 2;
	}
	return result;
}

void run(std::istream& input, std::ostream& output)
{
	int n;
	input >> n;
	output << count_unary_bits_on_odd_positions(n) << std::endl;
}

int main()
{
	return run(std::cin, std::cout), 0;
}
