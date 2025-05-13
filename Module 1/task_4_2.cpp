#include <cstddef>
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

template<typename T>
void swap(T& a, T& b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

template<typename T>
struct IsLessDefault
{
    bool operator()(const T& l, const T& r) { return l < r; }
};

template<typename T, typename IsLess = IsLessDefault<T>>
class Heap
{
public:
    Heap(const IsLess& cmp = IsLess()) : _cmp(cmp) { };
    ~Heap() { };

    void insert(T element);
    T extract();

    const T& peek() const
    {
        assert(_arr.size() != 0);
        return _arr.at(0);
    }

    size_t size() { return _arr.size(); }
private:
    IsLess _cmp;

    Array<T> _arr;

    void _build_heap();
    void _sift_down(size_t i);
    void _sift_up(size_t i);
};

template<typename T, typename IsLess>
void Heap<T, IsLess>::_sift_down(size_t i)
{
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;

    size_t largest = i;
    if (left < _arr.size() && _cmp(_arr[left], _arr[i]))
    {
        largest = left;
    }
    if (right < _arr.size() && _cmp(_arr[right], _arr[largest]))
    {
        largest = right;
    }

    if (largest != i)
    {
        swap(_arr[i], _arr[largest]);
        _sift_down(largest);
    }
}

template<typename T, typename IsLess>
void Heap<T, IsLess>::_build_heap()
{
    for (size_t i = _arr.size() / 2 - 1; i >= 0; --i)
    {
        _sift_down(i);
    }
}

template<typename T, typename IsLess>
void Heap<T, IsLess>::_sift_up(size_t i)
{
    while (i > 0)
    {
        size_t parent = (i - 1) / 2;
        if (!_cmp(_arr[i], _arr[parent]))
            return;
        swap(_arr[i], _arr[parent]);
        i = parent;
    }
}

template<typename T, typename IsLess>
void Heap<T, IsLess>::insert(T elem)
{
    _arr.push_back(elem);
    _sift_up(_arr.size() - 1);
}

template<typename T, typename IsLess>
T Heap<T, IsLess>::extract()
{
    assert(_arr.size() != 0);
    T result = _arr[0];
    _arr[0] = _arr[_arr.size() - 1];
    _arr.pop_back();

    if (_arr.size() != 0)
    {
        _sift_down(0);
    }

    return result;
}

struct User
{
    User() { }
    User(int _id, int _act) : id(_id), activity(_act) { }
    int id;
    int activity;
};

void run(std::istream& in, std::ostream& out)
{
    int n, k;
    in >> n >> k;

    auto cmp = [](const User& l, const User& r) { return l.activity < r.activity; };

    Heap<User, decltype(cmp)> heap(cmp);

    for (int i = 0; i < n; i++)
    {
        int id, activity;
        in >> id >> activity;

        if (heap.size() < k)
        {
            heap.insert(User(id, activity));
        }
        else
        {
            if (activity > heap.peek().activity)
            {
                heap.extract();
                heap.insert(User(id, activity));
            }
        }
    }

    Array<User> result;
    while (heap.size() > 0)
    {
        result.push_back(heap.extract());
    }

    for (int i = 0; i < result.size(); ++i)
    {
        out << result[i].id << ' ';
    }
}

int main() {
    run(std::cin, std::cout);
    return 0;
}
