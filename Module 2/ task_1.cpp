/*
Реализуйте структуру данных типа “множество строк”
на основе динамической хеш-таблицы
с открытой адресацией. Хранимые строки непустые
и состоят из строчных латинских букв.
Хеш-функция строки должна быть реализована
с помощью вычисления значения многочлена методом Горнера.
Начальный размер таблицы должен быть равным 8-ми.
Перехеширование выполняйте при добавлении элементов в случае,
когда коэффициент заполнения таблицы достигает 3/4.
Структура данных должна поддерживать операции добавления
строки в множество, удаления строки из множества
и проверки принадлежности данной строки множеству.
1_2. Для разрешения коллизий используйте двойное хеширование.
*/

#include <iostream>
#include <cassert>
#include <vector>

#define INITIAL_SIZE 8

template<typename T, typename H = std::hash<T>>
class hash_table
{
public:
    hash_table(H hasher1, H hasher2)
        : _table(INITIAL_SIZE), _size(0), _hasher1(hasher1), _hasher2(hasher2)
    { }

    ~hash_table()
    { }

    hash_table(const hash_table& t) = delete;
    hash_table& operator=(const hash_table& t) = delete;

    hash_table(hash_table&& t) = delete;
    hash_table& operator=(hash_table&& t) = delete;

    bool add_key(const T& key)
    {
        if (_size >= _table.size() * 0.75)
        {
            _grow_table();
        }

        size_t table_size = _table.size();
        size_t hash1 = _hasher1(key) % table_size;
        size_t hash2 = (_hasher2(key) * 2 + 1) % table_size;

        if (hash2 == 0) hash2 = 1;

        size_t first_deleted_pos = table_size;
        bool found_deleted = false;
        size_t pos = hash1;

        for (size_t  i = 0; i < table_size; ++i)
        {
            auto& cell = _table[pos];
            if (cell.cell_state == hash_table_cell::empty)
            {
                if (found_deleted)
                {
                    pos = first_deleted_pos;
                }
                _table[pos].key = key;
                _table[pos].hash = hash1;
                _table[pos].cell_state = hash_table_cell::has_key;
                _size++;
                return true;
            }
            else if (cell.cell_state == hash_table_cell::deleted)
            {
                if (!found_deleted)
                {
                    first_deleted_pos = pos;
                    found_deleted = true;
                }
            }
            else if (cell.cell_state == hash_table_cell::has_key)
            {
                if (cell.key == key)
                {
                    return false;
                }
            }
            pos = (hash1 + i * hash2) % table_size;
        }

        if (found_deleted)
        {
            _table[first_deleted_pos].key = key;
            _table[first_deleted_pos].hash = hash1;
            _table[first_deleted_pos].cell_state = hash_table_cell::has_key;
            _size++;
            return true;
        }

        return false;
    }

    bool delete_key(const T& key)
    {
        size_t table_size = _table.size();
        size_t hash1 = _hasher1(key) % table_size;
        size_t hash2 = (_hasher2(key) * 2 + 1) % table_size;

        if (hash2 == 0) hash2 = 1;

        size_t pos = hash1;

        for (size_t i = 0; i < table_size; ++i)
        {
            auto& cell = _table[pos];

            if (cell.cell_state == hash_table_cell::empty)
            {
                return false;
            }

            else if (cell.cell_state == hash_table_cell::has_key && cell.key == key)
            {
                cell.cell_state = hash_table_cell::deleted;
                _size--;
                return true;
            }

            pos = (hash1 + i * hash2) % table_size;
        }

        return false;
    }

    bool has_key(const T& key)
    {
        size_t table_size = _table.size();
        size_t hash1 = _hasher1(key) % table_size;
        size_t hash2 = (_hasher2(key) * 2 + 1) % table_size;

        if (hash2 == 0) hash2 = 1;

        size_t pos = hash1;
        for (size_t i = 0; i < table_size; ++i)
        {
            const auto& cell = _table[pos];

            if (cell.cell_state == hash_table_cell::empty)
            {
                return false;
            }

            if (cell.cell_state == hash_table_cell::has_key && cell.key == key)
            {
                return true;
            }

            pos = (hash1 + i * hash2) % table_size;
        }
        return false;
    }

private:
    H _hasher1;
    H _hasher2;

    struct hash_table_cell
    {
        T key;
        size_t hash = 0;

        enum cell_state {
            empty,
            deleted,
            has_key,
        } cell_state = empty;
    };

    std::vector<hash_table_cell> _table;

    size_t _size;

    void _grow_table()
    {
        std::vector<hash_table_cell> old_table = std::move(_table);
        size_t new_size = old_table.size() * 2;

        _table.resize(new_size);
        _size = 0;

        for (const auto& cell : old_table)
        {
            if (cell.cell_state == hash_table_cell::has_key)
            {
                add_key(cell.key);
            }
        }
    }
};

struct string_hasher
{
    string_hasher(size_t p = 101)
        : _p(p)
    { }

    size_t operator()(const std::string& key) const
    {
        size_t hash = 1;
        for (const char& a : key)
        {
            hash = hash * _p + a;
        }

        return hash;
    }

private:
    size_t _p;
};

void run(std::istream& in, std::ostream& out)
{
    hash_table<std::string, decltype(string_hasher())> set(string_hasher(101), string_hasher(103));

    char op = '\0';
    std::string word;

    while (in >> op >> word)
    {
        switch (op)
        {
        case '+':
            out << (set.add_key(word) ? "OK" : "FAIL") << '\n';
            break;
        case '-':
            out << (set.delete_key(word) ? "OK" : "FAIL") << '\n';
            break;
        case '?':
            out << (set.has_key(word) ? "OK" : "FAIL") << '\n';
            break;
        }
    }
}

int main()
{
    return run(std::cin, std::cout), 0;
}
