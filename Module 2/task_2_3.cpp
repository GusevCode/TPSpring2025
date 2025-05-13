/*
Дано число N < 10^6 и последовательность
целых чисел из [-2^31..2^31] длиной N.
Требуется построить бинарное дерево,
заданное наивным порядком вставки.
Т.е., при добавлении очередного числа K
в дерево с корнем root, если root→Key ≤ K,
то узел K добавляется в правое поддерево root;
иначе в левое поддерево root.
Требования: Рекурсия запрещена.
Решение должно поддерживать передачу функции сравнения снаружи.

2_3. Выведите элементы в порядке post-order (снизу вверх).
*/

#include <iostream>
#include <stack>

template<typename T, typename C>
class binary_tree
{
public:
    binary_tree(const C& cmp) : _root(nullptr), _cmp(cmp) { }

    void insert(const T& key)
    {
        node** current_node = &_root;
        while (*current_node != nullptr)
            current_node = _cmp((*(*current_node)).key, key) ? &(*(*current_node)).right : &(*(*current_node)).left;
        *current_node = new node(key);
    }

    void traverse(void (*visit)(const T&)) const
    {
        if (!_root) return;
        std::stack<std::pair<node*, bool>> s;
        s.push({_root, false});

        while (!s.empty())
        {
            auto [current, visited] = s.top();
            s.pop();

            if (visited)
            {
                visit(current->key);
            }
            else
            {
                s.push({current, true});
                if (current->right) s.push({current->right, false});
                if (current->left) s.push({current->left, false});
            }
        }
    }

    ~binary_tree()
    {
        std::stack<node*> nodes;
        std::stack<node*> to_delete;
        if (_root) nodes.push(_root);

        while (!nodes.empty())
        {
            node* current = nodes.top();
            nodes.pop();
            to_delete.push(current);
            if (current->left) nodes.push(current->left);
            if (current->right) nodes.push(current->right);
        }

        while (!to_delete.empty())
        {
            delete to_delete.top();
            to_delete.pop();
        }
    };


private:
    struct node
    {
        node(const T& k) : key(k), left(nullptr), right(nullptr) { }
        T key;
        node* left;
        node* right;
    };

    node* _root;
    C _cmp;
};

struct int_cmp { bool operator()(int l, int r) { return l <= r; }; };

int main()
{
    int n, x;
    std::cin >> n;

    int_cmp cmp;
    binary_tree<int, int_cmp> tree(cmp);

    for (int i = 0; i < n; ++i)
    {
        std::cin >> x;
        tree.insert(x);
    }

    tree.traverse([](const int& key) { std::cout << key << ' '; });
    return 0;
}
