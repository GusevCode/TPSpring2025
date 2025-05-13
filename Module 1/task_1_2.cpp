/*
1.2 Вернуть значение бита в числе N по его номеру K.
Формат входных данных. Число N, номер бита K
*/

#include <iostream>
#include <cassert>

int getValueOfBit(int n, int k)
{
	return (n >> k) & 1;
}

void run(std::istream& input, std::ostream& output)
{
	int n = 0, k = 0;
	input >> n >> k;
	output << getValueOfBit(n, k);
}

int main()
{
	run(std::cin, std::cout);
	return 0;
}