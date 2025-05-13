/*
    3_2. Реализовать дек с динамическим
    зацикленным буфером (на основе динамического массива).
*/

#include <cassert>
#include <iostream>

template<typename T>
class Array {
public:
    Array() : _buffer(nullptr), _buffer_size(0), _real_size(0) { }
    ~Array() { delete[] _buffer; }

    Array(const Array&) = delete;
    Array& operator=(const Array&) = delete;

    const T& at(int index) const;
    T& at(int index);

    T operator[](int index) const { return at(index); }
    T& operator[](int index) { return at(index); }

    void push_back(T elem);

    size_t size() const { return _real_size; }
    size_t capacity() const { return _buffer_size; }

    void resize(size_t new_size);

    T* get_buffer() { return _buffer; }
    void set_buffer(T* new_buffer) { _buffer = new_buffer; }
    void set_buffer_size(size_t new_size) { _buffer_size = new_size; }

private:
    T* _buffer;
    size_t _buffer_size;
    size_t _real_size;

    void _grow();
};

template<typename T>
const T& Array<T>::at(int index) const
{
    assert(index >= 0 && index < _buffer_size && _buffer != nullptr);
    return _buffer[index];
}

template<typename T>
T& Array<T>::at(int index)
{
    assert(index >= 0 && index < _buffer_size && _buffer != nullptr);
    return _buffer[index];
}

template<typename T>
void Array<T>::_grow()
{
    size_t new_buffer_size = _buffer_size == 0 ? 1 : _buffer_size * 2;
    resize(new_buffer_size);
}

template<typename T>
void Array<T>::resize(size_t new_size)
{
    T* new_buffer = new T[new_size];
    for (size_t i = 0; i < _real_size; ++i)
    {
        new_buffer[i] = _buffer[i];
    }

    delete[] _buffer;
    _buffer = new_buffer;
    _buffer_size = new_size;
}

template<typename T>
void Array<T>::push_back(T elem)
{
    if (_real_size == _buffer_size)
    {
        _grow();
    }

    assert(_real_size < _buffer_size && _buffer != nullptr);
    _buffer[_real_size++] = elem;
}

template<typename T>
class Dequeue {
public:
    Dequeue() : _head_ptr(0), _tail_ptr(0), _size(0), _array(new Array<T>()) { }
    ~Dequeue() { delete _array; }

    Dequeue(const Dequeue&) = delete;
    Dequeue& operator=(const Dequeue&) = delete;

    void push_front(const T&);
    T pop_front();
    void push_back(const T&);
    T pop_back();

    size_t size() const { return _size; }

private:
    size_t _head_ptr;
    size_t _tail_ptr;
    size_t _size;
    Array<T>* _array;

    void _ensure_capacity();
    void _reallocate();
};

template<typename T>
void Dequeue<T>::_ensure_capacity()
{
    if (_size == _array->capacity())
    {
        _reallocate();
    }
}

template<typename T>
void Dequeue<T>::_reallocate()
{
    size_t new_capacity = _array->capacity() == 0 ? 1 : _array->capacity() * 2;
    T* new_buffer = new T[new_capacity];

    size_t current_size = _size;
    for (size_t i = 0; i < current_size; ++i)
    {
        new_buffer[i] = (*_array)[(_head_ptr + i) % _array->capacity()];
    }

    delete[] _array->get_buffer();
    _array->set_buffer(new_buffer);
    _array->set_buffer_size(new_capacity);
    _head_ptr = 0;
    _tail_ptr = current_size;
}

template<typename T>
void Dequeue<T>::push_front(const T& elem)
{
    _ensure_capacity();
    _head_ptr = (_head_ptr == 0) ? _array->capacity() - 1 : _head_ptr - 1;
    (*_array)[_head_ptr] = elem;
    _size++;
}

template<typename T>
T Dequeue<T>::pop_front()
{
    assert(_size != 0);

    T elem = (*_array)[_head_ptr];
    _head_ptr = (_head_ptr + 1) % _array->capacity();
    _size--;
    return elem;
}

template<typename T>
void Dequeue<T>::push_back(const T& elem)
{
    _ensure_capacity();
    (*_array)[_tail_ptr] = elem;
    _tail_ptr = (_tail_ptr + 1) % _array->capacity();
    _size++;
}

template<typename T>
T Dequeue<T>::pop_back()
{
    assert(_size != 0);

    _tail_ptr = (_tail_ptr == 0) ? _array->capacity() - 1 : _tail_ptr - 1;
    T elem = (*_array)[_tail_ptr];
    _size--;
    return elem;
}

void run(std::istream& input, std::ostream& output) {
    int n;
    input >> n;

    Dequeue<int> deq;

    bool check = true;
    int ans = 0;

    for (int i = 0; i < n; i++)
    {
        int a, b;
        input >> a >> b;
        switch (a)
        {
        case 1:
            deq.push_front(b);
            break;
        case 2:
            ans = -1;
            if (deq.size() > 0)
            {
                ans = deq.pop_front();
            }
            if (ans != b) {
                check = false;
            }
            break;
        case 3:
            deq.push_back(b);
            break;
        case 4:
            ans = -1;
            if (deq.size() > 0)
            {
                ans = deq.pop_back();
            }
            if (ans != b) {
                check = false;
            }
            break;
        default:
            assert("unknown command.");
            break;
        }
    }

    output << ((check) ? "YES\n" : "NO\n");
}

int main(int argc, char** argv)
{
    return run(std::cin, std::cout), 0;
}
