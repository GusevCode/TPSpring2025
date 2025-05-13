/*
    2.1
    Дан отсортированный массив целых чисел A[0..n-1] и массив целых чисел B[0..m-1].
    Для каждого элемента массива B[i] найдите минимальный индекс k минимального элемента массива A, равного
    или превосходящего B[i]: A[k] >= B[i]. Если такого элемента нет, выведите n. n, m ≤ 10000.
    Требования:  Время работы поиска k для каждого элемента B[i]: O(log(k)).
    Внимание! В этой задаче для каждого B[i] сначала нужно определить диапазон
    для бинарного поиска размером порядка k с помощью экспоненциального поиска,
    а потом уже в нем делать бинарный поиск.
*/

#include <iostream>
#include <cassert>

int binary_search(int* arr, int size, int value)
{
    if (size <= 0)
    {
        return size;
    }

    int l = 0;
    int r = 1;

    for ( ; r < size && arr[r] < value; r <<= 1);
    l = r >> 1;

    r = (r >= size) ? size - 1 : r;

    while (l <= r)
    {
        int m = (l + r) / 2;

        if (arr[m] < value)
        {
            l = m + 1;
        }
        else
        {
            r = m - 1;
        }
    }

    return (l < size) ? l : size;
}

void run(std::istream& input, std::ostream& output)
{
    int n = 0, m = 0, x = 0;

    input >> n >> m;

    int* A = new int[n];

    for (int i = 0; i < n; i++)
    {
        input >> A[i];
    }

    for (int i = 0; i < m; i++)
    {
        input >> x;
        output << binary_search(A, n, x) << ' ';
    }

    delete[] A;
}

int main()
{
    run(std::cin, std::cout);
    return 0;
}
