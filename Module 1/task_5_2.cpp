/*
5_2. Современники.
Группа людей называется современниками если был
такой момент, когда они могли собраться вместе.
Для этого в этот момент каждому из них должно было
уже исполниться 18 лет, но ещё не исполниться 80 лет.
Дан список Жизни Великих Людей.
Необходимо получить максимальное количество современников.
В день 18летия человек уже может принимать участие в собраниях,
а в день 80летия и в день смерти уже не может.
Замечание. Человек мог не дожить до 18-летия, либо умереть в день 18-летия.
В этих случаях принимать участие в собраниях он не мог.
*/

#include <algorithm>
#include <iostream>
#include <cassert>
#include <sstream>

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

struct Date
{
    int day;
    int month;
    int year;
    int is_start;
};

bool is_leap_year(int year)
{
    if (year % 4 != 0) return false;
    if (year % 100 != 0) return true;
    return (year % 400 == 0);
}

bool is_valid_date(const Date& date)
{
    if (date.month < 1 || date.month > 12) return false;
    if (date.day < 1) return false;

    int max_day;
    switch (date.month)
    {
        case 2:
            max_day = is_leap_year(date.year) ? 29 : 28;
            break;
        case 4: case 6: case 9: case 11:
            max_day = 30;
            break;
        default:
            max_day = 31;
            break;
    }

    return (date.day <= max_day);
}

Date get_adult_birthday(const Date& birth)
{
    Date adult = birth;
    adult.year += 18;

    if (!is_valid_date(adult))
    {
        adult.month = 3;
        adult.day = 1;
    }

    return adult;
}

bool is_date_greater_or_equal(const Date& death, const Date& date)
{
    if (death.year != date.year)
        return death.year > date.year;
    if (death.month != date.month)
        return death.month > date.month;
    return death.day >= date.day;
}

bool is_date_less_or_equal(const Date& date1, const Date& date2)
{
    if (date1.year != date2.year)
        return date1.year < date2.year;
    if (date1.month != date2.month)
        return date1.month < date2.month;
    return date1.day <= date2.day;
}

bool did_live_to_adult(const Date& birth, const Date& death)
{
    Date adult = get_adult_birthday(birth);
    return is_date_greater_or_equal(death, adult);
}

Date get_super_adult_birthday(const Date& birth)
{
    Date eighty = birth;
    eighty.year += 80;

    if (!is_valid_date(eighty))
    {
        eighty.month = 3;
        eighty.day = 1;
    }

    return eighty;
}

Date get_previous_day(const Date& date)
{
    Date prev = date;
    prev.day--;

    if (prev.day >= 1) return prev;

    prev.month--;
    if (prev.month < 1)
    {
        prev.month = 12;
        prev.year--;
    }

    switch (prev.month)
    {
        case 2: prev.day = is_leap_year(prev.year) ? 29 : 28; break;
        case 4: case 6: case 9: case 11: prev.day = 30; break;
        default: prev.day =31;
    }

    return prev;
}

Date get_day_before_super_adult_or_death(const Date& birth, const Date& death)
{
    Date eighty = get_super_adult_birthday(birth);
    Date day_before_80 = get_previous_day(eighty);
    Date day_before_death = get_previous_day(death);

    return is_date_less_or_equal(day_before_death, day_before_80) ? day_before_death : day_before_80;
}

struct IsLessDate
{
    bool operator()(const Date& a, const Date& b)
    {
        if (a.year != b.year) return a.year < b.year;
        if (a.month != b.month) return a.month < b.month;
        if (a.day != b.day) return a.day < b.day;
        return a.is_start > b.is_start;
    }
};

int count_contemporary(Array<Date>& dates)
{
    if (dates.size() > 0)
        merge_sort(dates, 0, dates.size() - 1, IsLessDate());

    int current = 0, max_count = 0;
    for (int i = 0; i < dates.size(); i++)
    {
        if (dates[i].is_start)
        {
            ++current;
            max_count = std::max(max_count, current);
        }
        else
        {
            --current;
        }
    }

    return max_count;
}

void run(std::istream& input, std::ostream& output)
{
    int n;
    input >> n;
    int count = 0;

    Array<Date> dates;

    for (int i = 0; i < n; i++)
    {
        Date birth, death;
        input >> birth.day >> birth.month >> birth.year;
        input >> death.day >> death.month >> death.year;

        if (did_live_to_adult(birth, death))
        {
            Date start = get_adult_birthday(birth);
            start.is_start = 1;
            Date finish = get_day_before_super_adult_or_death(birth, death);
            finish.is_start = 0;

            dates.push_back(start);
            dates.push_back(finish);
        }
    }

    output << count_contemporary(dates);
}

void test()
{
    {
        std::stringstream in;
        std::stringstream out;
        in << "3 2 5 1980 13 11 2055 1 1 1982 1 1 2030 2 1 1920 2 1 2000";
        run(in, out);
        assert(out.str() == "3");
    }
    {
        std::stringstream in;
        std::stringstream out;
        in << "3 2 5 1988 13 11 2005 1 1 1 1 1 30 1 1 1910 1 1 1990";
        run(in, out);
        assert(out.str() == "1");
    }
    {
        std::stringstream in;
        std::stringstream out;
        in << "3 2 5 1968 13 11 2005 1 1 1 1 1 30 1 1 1910 1 1 1990";
        run(in, out);
        assert(out.str() == "2");
    }
    {
        std::stringstream in;
        std::stringstream out;
        in << "3 2 5 1988 13 11 2005 1 1 1 1 1 10 2 1 1910 1 1 1928";
        run(in, out);
        assert(out.str() == "0");
    }
}

int main()
{
    return run(std::cin, std::cout), 0;
}
