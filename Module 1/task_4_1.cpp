/*
    Требование для всех вариантов Задачи 4
    Решение всех задач данного раздела предполагает использование кучи,
    реализованной в виде шаблонного класса.
    Решение должно поддерживать передачу функции сравнения снаружи.
    Куча должна быть динамической.
    4.1 Слияние массивов.
    Напишите программу, которая использует кучу для
    слияния K отсортированных массивов суммарной длиной N.
    Требования: время работы O(N * logK). Ограничение на размер кучи O(K)..
    Формат входных данных: Сначала вводится количество массивов K.
    Затем по очереди размер каждого массива и элементы массива.
    Каждый массив упорядочен по возрастанию.
    Формат выходных данных: Итоговый отсортированный массив.
*/

#include <cstddef>
#include <iostream>
#include <sstream>
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
    // explicit Heap(const Array<T>& arr);
    // explicit Heap(T* arr, size_t size, IsLess cmp);
    ~Heap() { };

    void insert(T element);
    T extract();

    const T& peek() const;

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

template<typename T>
struct Element
{
    T value;
    size_t array_index;
    size_t index;
};

template<typename T>
void merge_arrays(Array<Array<T>>& arrays, std::ostream& out)
{
    auto element_cmp = [](const Element<int>& l, const Element<int>& r) { return l.value < r.value; };
    Heap<Element<int>, decltype(element_cmp)> heap(element_cmp);

    for (size_t i = 0; i < arrays.size(); i++)
    {
        if (arrays[i].size() > 0)
        {
            heap.insert(Element<int>{arrays[i][0], i, 0});
        }
    }

    while (heap.size() > 0)
    {
        Element<int> min_elem = heap.extract();
        out << min_elem.value << ' ';

        size_t next_idx = min_elem.index + 1;
        if (next_idx < arrays[min_elem.array_index].size())
        {
            heap.insert(Element<int>{
                arrays[min_elem.array_index][next_idx],
                min_elem.array_index,
                next_idx
            });
        }
    }

    out << std::endl;
}

void run(std::istream& input, std::ostream& output)
{
    int k;
    input >> k;

    Array<Array<int>> arrays;

    for (int i = 0; i < k; i++)
    {
        int size;
        input >> size;

        Array<int> arr;

        for (int j = 0; j < size; j++)
        {
            int value;
            input >> value;
            arr.push_back(value);
        }

        arrays.push_back(std::move(arr));
    }

    std::stringstream merged_arrays;
    merge_arrays(arrays, merged_arrays);

    output << merged_arrays.str() << std::endl;
}

int main()
{
    return run(std::cin, std::cout), 0;
}
