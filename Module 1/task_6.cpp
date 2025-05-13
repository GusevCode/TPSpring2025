/*
Дано множество целых чисел из [0..10^9] размера n.
Используя алгоритм поиска k-ой
порядковой статистики, требуется найти
 следующие параметры множества:
10%  перцентиль
медиана
90%  перцентиль
6_2. Реализуйте стратегию выбора опорного элемента “медиана трёх”.
Функцию Partition реализуйте методом прохода двумя
итераторами от конца массива к началу.
*/


#include <algorithm>
#include <iostream>

template<typename T>
struct IsLessDefault {
    bool operator()(const T& l, const T& r) const { return l < r; }
};

template<typename T>
int find_pivot_index(T* arr, int l, int r) {
    int m = l + (r - l) / 2;
    if ((arr[l] <= arr[m] && arr[m] <= arr[r]) || (arr[r] <= arr[m] && arr[m] <= arr[l])) return m;
    if ((arr[m] <= arr[l] && arr[l] <= arr[r]) || (arr[r] <= arr[l] && arr[l] <= arr[m])) return l;
    return r;
}

template<typename T, typename Comparator>
int partition(T* arr, int l, int r, Comparator cmp) {
    int pivot_idx = find_pivot_index(arr, l, r);

    std::swap(arr[pivot_idx], arr[l]);
    const T& pivot = arr[l];

    int i = r, j = r;

    while (j > l)
    {
        if (cmp(pivot, arr[j]))
            std::swap(arr[j], arr[i--]);
        j--;
    }

    std::swap(arr[l], arr[i]);
    return i;
}

template<typename T, typename Comparator = IsLessDefault<T>>
T quick_select(T* data, int n, int k, Comparator cmp = IsLessDefault<T>()) {
    int left = 0, right = n - 1;

    int pivot_pos = partition(data, left, right, cmp);
    while (pivot_pos != k)
    {
        if (pivot_pos < k)
            left = pivot_pos + 1;
        else
            right = pivot_pos - 1;

        pivot_pos = partition(data, left, right, cmp);
    }

    return data[k];
}

void run(std::istream& input, std::ostream& output) {
    int n;
    input >> n;
    if (n <= 0) return;

    int* data = new int[n];
    for (int i = 0; i < n; ++i)
        input >> data[i];

    int* data_copy = new int[n];
    std::copy(data, data + n, data_copy);
    int p10 = quick_select(data_copy, n, n * 1 / 10);

    std::copy(data, data + n, data_copy);
    int median = quick_select(data_copy, n, n / 2);

    std::copy(data, data + n, data_copy);
    int p90 = quick_select(data_copy, n, n * 9 / 10);

    output << p10 << "\n" << median << "\n" << p90 << "\n";

    delete[] data_copy;
    delete[] data;
}

int main() {
    std::cin.tie(0);
    std::cout.tie(0);
    run(std::cin, std::cout);
    return 0;
}
