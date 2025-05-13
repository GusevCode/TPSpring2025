#include <cassert>
#include <iostream>

template<typename T>
int search(T* arr, int size)
{
    assert(size > 0);

    if (size == 1)
        return arr[0];

    int l = 0;
    int r = 1;
    int prev = 0;

    for (r = 1; r < size && arr[r] > arr[r - 1]; r <<= 1);
    l = r >> 1;
    r = (r >= size) ? size - 1 : r;

    while (l < r)
    {
        int m = l + (r - l) / 2;
        if (arr[m] < arr[m + 1])
        {
            l = m + 1;
        }
        else
        {
            r = m;
        }
    }

    return l;
}

void run(std::istream& in, std::ostream& out)
{
    int n;
    in >> n;

    int* arr = new int[n];

    for (int i = 0; i < n; i++)
    {
        in >> arr[i];
    }

    out << search(arr, n) << "\n";

    delete[] arr;
}

int main() {
    run(std::cin, std::cout);
    return 0;
}
