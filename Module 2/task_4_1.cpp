/*
4_1. Солдаты. В одной военной части
решили построить в одну шеренгу по росту.
Т.к. часть была далеко не образцовая,
то солдаты часто приходили не вовремя,
а то их и вовсе приходилось выгонять из
шеренги за плохо начищенные сапоги.
Однако солдаты в процессе прихода и ухода должны были
всегда быть выстроены по росту – сначала
самые высокие, а в конце – самые низкие.
За расстановку солдат отвечал прапорщик,
который заметил интересную особенность – все солдаты
в части разного роста. Ваша задача состоит в том,
чтобы помочь прапорщику правильно расставлять солдат,
а именно для каждого приходящего солдата указывать,
перед каким солдатом в строе он должен становится.
Требования: скорость выполнения команды - O(log n).
Формат входных данных.
Первая строка содержит число N –
количество команд (1 ≤ N ≤ 30 000).
В каждой следующей строке содержится
описание команды: число 1 и
X если солдат приходит в строй (X – рост солдата,
натуральное число до 100 000 включительно) и
число 2 и Y если солдата, стоящим в строе на месте
Y надо удалить из строя. Солдаты в строе нумеруются с нуля.
Формат выходных данных.
На каждую команду 1 (добавление в строй) вы должны
выводить число K – номер позиции, на которую должен
встать этот солдат (все стоящие за ним двигаются назад).
*/

#include <stack>
#include <cassert>
#include <sstream>
#include <iostream>

typedef unsigned char byte;

template<typename T, typename C>
class avl_tree {
public:
    void insert(const T& key) {
        _root = _insert(_root, key);
    }

    void erase(const T& key) {
        _root = _erase(_root, key);
    }

    int count_greater(const T& key) const {
        return _count_greater(_root, key);
    }

    T find_kth(int k) const {
        node* res = _find_kth(_root, k);
        assert(res != nullptr && "Out of bounds");
        return res->key;
    }

    avl_tree(const C& c) : _cmp(c), _root(nullptr) { }

    ~avl_tree() {
        std::stack<node*> nodes;
        if (_root) nodes.push(_root);
        while (!nodes.empty()) {
            node* current = nodes.top();
            nodes.pop();
            if (current->left) nodes.push(current->left);
            if (current->right) nodes.push(current->right);
            delete current;
        }
    }

private:
    struct node {
        T key;
        byte height;
        int size;
        node* left;
        node* right;
        node(T k) : key(k), height(1), size(1), left(nullptr), right(nullptr) {}
    };

    node* _root;
    C _cmp;

    byte _height(node* p) const {
        return p ? p->height : 0;
    }

    int _bfactor(node* p) const {
        return _height(p->right) - _height(p->left);
    }

    void _fix_height(node* p) {
        p->height = std::max(_height(p->left), _height(p->right)) + 1;
    }

    void _fix_size(node* p) {
        p->size = 1 + (p->left ? p->left->size : 0) + (p->right ? p->right->size : 0);
    }

    node* _rotate_right(node* p) {
        node* q = p->left;
        p->left = q->right;
        q->right = p;
        _fix_height(p);
        _fix_height(q);
        _fix_size(p);
        _fix_size(q);
        return q;
    }

    node* _rotate_left(node* q) {
        node* p = q->right;
        q->right = p->left;
        p->left = q;
        _fix_height(q);
        _fix_height(p);
        _fix_size(q);
        _fix_size(p);
        return p;
    }

    node* _balance(node* p) {
        _fix_height(p);
        _fix_size(p);
        int bf = _bfactor(p);
        if (bf == 2) {
            if (_bfactor(p->right) < 0)
                p->right = _rotate_right(p->right);
            return _rotate_left(p);
        } else if (bf == -2) {
            if (_bfactor(p->left) > 0)
                p->left = _rotate_left(p->left);
            return _rotate_right(p);
        }
        return p;
    }

    node* _insert(node* p, T k) {
        if (!p) return new node(k);
        if (_cmp(k, p->key))
            p->left = _insert(p->left, k);
        else
            p->right = _insert(p->right, k);
        _fix_size(p);
        return _balance(p);
    }

    std::pair<node*, T> _remove_max(node* p) {
        if (!p->right) {
            T max_key = p->key;
            node* left = p->left;
            delete p;
            return {left, max_key};
        }
        auto [new_right, max_key] = _remove_max(p->right);
        p->right = new_right;
        _fix_size(p);
        return {_balance(p), max_key};
    }

    node* _erase(node* p, const T& key) {
        if (!p) return nullptr;
        if (_cmp(key, p->key)) {
            p->left = _erase(p->left, key);
        } else if (_cmp(p->key, key)) {
            p->right = _erase(p->right, key);
        } else {
            node* left = p->left;
            node* right = p->right;
            delete p;
            if (!left) return right;
            auto [new_left, max_key] = _remove_max(left);
            node* new_root = new node(max_key);
            new_root->left = new_left;
            new_root->right = right;
            return _balance(new_root);
        }
        _fix_size(p);
        return _balance(p);
    }

    int _count_greater(node* p, const T& x) const {
        if (!p) return 0;
        if (x < p->key) {
            return (p->left ? p->left->size : 0) + 1 + _count_greater(p->right, x);
        } else {
            return _count_greater(p->left, x);
        }
    }

    node* _find_kth(node* p, int k) const {
        if (!p) return nullptr;
        int left_size = p->left ? p->left->size : 0;
        if (k < left_size) {
            return _find_kth(p->left, k);
        } else if (k == left_size) {
            return p;
        } else {
            return _find_kth(p->right, k - left_size - 1);
        }
    }
};

struct int_cmp_desc { bool operator()(int l, int r) const { return l > r; }; };

void run(std::istream& in, std::ostream& out)
{
    int_cmp_desc cmp;
    avl_tree<int, int_cmp_desc> tree(cmp);

    int n;
    in >> n;

    for (int i = 0; i < n; ++i)
    {
        int cmd;
        in >> cmd;
        switch (cmd)
        {
        case 1:
            int x;
            in >> x;
            tree.insert(x);
            out << tree.count_greater(x) << std::endl;
            break;
        case  2:
            int y;
            in >> y;
            int key = tree.find_kth(y);
            tree.erase(key);
            break;
        }
    }
}

void test()
{
    int n = 1;
    {
        std::stringstream in;
        std::stringstream out;
        in << "5\n1 100\n1 200\n1 50\n2 1\n1 150";
        run(in, out);
        std::stringstream ans;
        ans << "0\n0\n2\n1";
        if (out.str() != ans.str())
        {
            std::cout << "\t\tTEST #" << n << " FAILED" << std::endl;
            std::cout << "expected: \n" << ans.str() << std::endl;
            std::cout << "actual: \n" << out.str() << std::endl;
        }
        ++n;
    }
}

int main()
{
    // test();
    run(std::cin, std::cout);
    return 0;
}
