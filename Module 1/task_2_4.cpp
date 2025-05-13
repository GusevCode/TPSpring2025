#include <iostream>
#include <cassert>

int binary_search(int* arr, int size, int value)
{
    assert(size > 0);
    if (size == 1)
        return 0;

    int left = 0;
    int right = 1;

    for ( ; right < size && arr[right] < value; left = right, right <<= 1);
    right = right >= size ? size - 1 : right;

    while (left <= right)
    {
        int mid = left + (right - left) / 2;
        if (arr[mid] == value)
        {
            return mid;
        }
        else if (arr[mid] < value)
        {
            left = mid + 1;
        }
        else {
            right = mid - 1;
        }
    }

    if (left == 0) return 0;
    if (left == size) return size - 1;
    if (value - arr[left - 1] <= arr[left] - value)
        return left - 1;
    else
        return left;
}

void run(std::istream& in, std::ostream& out) {
    int n;
    in >> n;

    int* A = new int[n];

    for (int i = 0; i < n; i++)
        in >> A[i];


    int m;
    in >> m;

    for (int i = 0; i < m; i++)
    {
        int x;
        in >> x;
        out << binary_search(A, n, x) << ' ';
    }

    delete[] A;
}

int main() {
    run(std::cin, std::cout);
    return 0;
}
