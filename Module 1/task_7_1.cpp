/*
Дан массив строк. Количество строк не больше 100000.
Отсортировать массив методом поразрядной сортировки MSD по символам.
Размер алфавита - 256 символов. Последний символ строки = ‘\0’.
*/


#include <cstring>
#include <iostream>
#include <cassert>

template<typename T>
class Array {
public:
    Array() : _buffer(nullptr), _buffer_size(0), _real_size(0) { }
    ~Array() { delete[] _buffer; }

    Array(const Array&) = delete;
    Array& operator=(const Array&) = delete;

    const T& at(int index) const;
    T& at(int index);

    const T& operator[](int index) const { return at(index); }
    T& operator[](int index) { return at(index); }

    void push_back(const T& elem);

    int size() const { return _real_size; }
private:
    T* _buffer;
    int _buffer_size;
    int _real_size;

    void _grow();
};

template<typename T>
const T& Array<T>::at(int index) const
{
    assert(index >= 0 && index < _real_size && _buffer != nullptr);
    return _buffer[index];
}

template<typename T>
T& Array<T>::at(int index)
{
    assert(index >= 0 && index < _real_size && _buffer != nullptr);
    return _buffer[index];
}

template<typename T>
void Array<T>::_grow()
{
    int new_buffer_size = _buffer_size == 0 ? 1 : _buffer_size * 2;

    T* new_buffer = new T[new_buffer_size];
    for (int i = 0; i < _real_size; ++i)
    {
        new_buffer[i] = std::move(_buffer[i]);
    }

    delete[] _buffer;
    _buffer = new_buffer;
    _buffer_size = new_buffer_size;
}

template<typename T>
void Array<T>::push_back(const T& elem)
{
    if (_real_size == _buffer_size)
    {
        _grow();
    }

    assert(_real_size < _buffer_size && _buffer != nullptr);
    _buffer[_real_size++] = elem;
}

template<typename T>
void _msd_sort_internal(T* arr, int l, int r, int d, T* buffer)
{
    int n = r - l;

    int count_short = 0;
    for (int i = l; i < r; ++i)
        if (arr[i].size() <= d)
            count_short++;

    if (count_short == n)
        return;

    const int k_radix = 256;
    int count[k_radix + 1] = {0};
    const int len = r - l + 1;

    for (int i = l; i <= r; ++i) {
        unsigned char c = (d < arr[i].size()) ? arr[i][d] : 0;
        count[c + 1]++;
    }

    for (int i = 1; i <= k_radix; ++i)
        count[i] += count[i - 1];

    int count_starts[k_radix + 1];
    std::copy(count, count + k_radix + 1, count_starts);

    for (int i = l; i <= r; ++i) {
        unsigned char c = (d < arr[i].size()) ? arr[i][d] : 0;
        buffer[count[c]++] = arr[i];
    }

    for (int i = 0; i < len; ++i)
        arr[l + i] = buffer[i];

    for (int i = 0; i < k_radix; ++i) {
        int group_start = count_starts[i];
        int group_end = count_starts[i + 1] - 1;
        if (group_start < group_end)
            _msd_sort_internal(arr, l + group_start, l + group_end, d + 1, buffer);
    }
}

template<typename T>
void msd_sort(T* arr, int size)
{
    if (size <= 1)
        return;
    T* buffer = new T[size];
    _msd_sort_internal(arr, 0, size - 1, 0, buffer);
    delete[] buffer;
}

void run(std::istream& input, std::ostream& output)
{
    std::string str;
    Array<std::string> strs;
    while (input >> str)
    {
        strs.push_back(str);
    }

    msd_sort(&strs[0], strs.size());

    for (int i = 0; i < strs.size(); i++)
        output << strs[i] << "\n";
}

int main()
{
    return run(std::cin, std::cout), 0;
}
