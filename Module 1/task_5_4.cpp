#include <iostream>
#include <cassert>

template<typename T>
class Array {
public:
    Array() : _buffer(nullptr), _buffer_size(0), _real_size(0) { }
    ~Array() { delete[] _buffer; }

    Array(const Array&) = delete;
    Array& operator=(const Array&) = delete;

    Array(Array&& arr)
        : _buffer(arr._buffer), _buffer_size(arr._buffer_size), _real_size(arr._real_size)
    {
        arr._buffer = nullptr;
        arr._buffer_size = 0;
        arr._real_size = 0;
    }

    Array& operator=(Array&& arr)
    {
        if (this != &arr)
        {
            delete[] _buffer;
            _buffer = arr._buffer;
            _buffer_size = arr._buffer_size;
            _real_size = arr._real_size;
            arr._buffer = nullptr;
            arr._buffer_size = 0;
            arr._real_size = 0;
        }
        return *this;
    }

    const T& at(int index) const;
    T& at(int index);

    T operator[](int index) const { return at(index); }
    T& operator[](int index) { return at(index); }

    void push_back(const T& elem);
    void push_back(T&& elem);
    void pop_back() { _real_size--; }

    size_t size() const { return _real_size; }
private:
    T* _buffer;
    size_t _buffer_size;
    size_t _real_size;

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
    size_t new_buffer_size = _buffer_size == 0 ? 1 : _buffer_size * 2;

    T* new_buffer = new T[new_buffer_size];
    for (size_t i = 0; i < _real_size; ++i)
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
void Array<T>::push_back(T&& elem)
{
    if (_real_size == _buffer_size)
    {
        _grow();
    }

    assert(_real_size < _buffer_size && _buffer != nullptr);
    _buffer[_real_size++] = std::move(elem);
}

template<typename T, typename IsLess>
void merge(Array<T>& arr, size_t left, size_t mid, size_t right, IsLess cmp)
{
    int size_of_left_partition = mid - left + 1;
    int size_of_right_partition = right - mid;

    T* left_part = new T[size_of_left_partition];
    T* right_part = new T[size_of_right_partition];

    for (size_t i = 0; i < size_of_left_partition; ++i)
        left_part[i] = arr[left + i];
    for (size_t i = 0; i < size_of_right_partition; ++i)
        right_part[i] = arr[mid + 1 + i];

    size_t i = 0, j = 0, ptr = left;

    while (i < size_of_left_partition && j < size_of_right_partition)
    {
        if (cmp(left_part[i], right_part[j]))
            arr[ptr] = left_part[i++];
        else
            arr[ptr] = right_part[j++];
        ptr++;
    }

    while (i < size_of_left_partition)
        arr[ptr++] = left_part[i++];

    while (j < size_of_right_partition)
        arr[ptr++] = right_part[j++];

    delete[] left_part;
    delete[] right_part;
}

template<typename T, typename IsLess>
void merge_sort(Array<T>& arr, size_t left, size_t right, IsLess cmp)
{
    if (left >= right)
        return;
    size_t mid = left + (right - left) / 2;
    merge_sort(arr, left, mid, cmp);
    merge_sort(arr, mid + 1, right, cmp);
    merge(arr, left, mid, right, cmp);
}

struct Point
{
    Point() : x(0), is_start(0) { }
    Point(int a, int start) : x(a), is_start(start) {}
    int x;
    int is_start;
};

struct IsLessPoint
{
    bool operator()(const Point& l, const Point& r)
    {
        if (l.x == r.x)
        {
            return l.is_start < r.is_start;
        }
        return l.x < r.x;
    }
};

void run(std::istream& in, std::ostream& out)
{
    int n;
    in >> n;

    Array<Point> arr;

    for (int i = 0; i < n; i++)
    {
        int x1, x2;
        in >> x1 >> x2;

        if (x1 > x2) std::swap(x1, x2);

        arr.push_back(Point(x1, 1));
        arr.push_back(Point(x2, 0));
    }

    merge_sort(arr, 0, arr.size() - 1, IsLessPoint());

    int balance = 0;
    int prev_x = 0;
    int total_length = 0;

    for (int i = 0; i < arr.size(); i++)
    {
        const Point& current = arr[i];

        if (balance == 1)
        {
            total_length += current.x - prev_x;
        }
        if (current.is_start)
        {
            balance++;
        }
        else
        {
            balance--;
        }
        prev_x = current.x;
    }

    out << total_length;
}

int main() {
    run(std::cin, std::cout);
    return 0;
}
