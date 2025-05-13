/*
Постройте B-дерево минимального порядка t и выведите его по слоям.
В качестве ключа используются числа, лежащие в диапазоне [0..232-1]
Требования:
B-дерево должно быть реализовано в виде шаблонного класса.
Решение должно поддерживать передачу функции сравнения снаружи.
*/

#include <iostream>
#include <vector>
#include <queue>

template <typename T, typename Compare = std::less<T>>
class BTree
{
public:
    struct Node
    {
        Node(bool leaf)
        : leaf(leaf)
        {
        }
        
        ~Node()
        {
            for (Node* child: children)
            {
                delete child;
            }
        }
        
        bool leaf;
        std::vector<T> keys;
        std::vector<Node*> children;
    };
    
    BTree(size_t minDegree, Compare comp = Compare())
    : t(minDegree), root(nullptr), comp(comp)
    {
    }
    
    ~BTree()
    {
        if (root)
            delete root;
    }
    
    void Insert(const T &key)
    {
        if (!root)
            root = new Node(true);
        
        if (isNodeFull(root))
        {
            Node *newRoot = new Node(false);
            newRoot->children.push_back(root);
            root = newRoot;
            splitChild(root, 0);
        }
        
        insertNonFull(root, key);
    }
    
    template <typename KeyVisitor, typename LevelEndVisitor>
    void Traverse(KeyVisitor visitKey, LevelEndVisitor visitLevelEnd) {
        if (!root)
            return;
            
        std::queue<Node*> queue;
        std::queue<Node*> nextLevel;
        
        queue.push(root);
        
        while (!queue.empty()) {
            Node* node = queue.front();
            queue.pop();
            
            for (const T& key : node->keys) {
                visitKey(key);
            }
            
            for (Node* child : node->children) {
                nextLevel.push(child);
            }
            
            if (queue.empty() && !nextLevel.empty()) {
                visitLevelEnd();
                std::swap(queue, nextLevel);
            }
        }
    }
    
private:
    bool isNodeFull(Node *node)
    {
        return node->keys.size() == 2*t - 1;
    }
    
    void splitChild(Node *node, size_t index)
    {
        Node* y = node->children[index];
        Node* z = new Node(y->leaf);
        
        for (size_t j = 0; j < t - 1; j++)
            z->keys.push_back(y->keys[j + t]);
        
        if (!y->leaf)
        {
            for (size_t j = 0; j < t; j++)
                z->children.push_back(y->children[j + t]);
            
            y->children.erase(y->children.begin() + t, y->children.end());
        }
        
        node->children.insert(node->children.begin() + index + 1, z);
        
        node->keys.insert(node->keys.begin() + index, y->keys[t - 1]);
        
        y->keys.resize(t - 1);
    }
    
    void insertNonFull(Node *node, const T &key)
    {
        int pos = node->keys.size() - 1;
        
        if (node->leaf)
        {
            node->keys.resize(node->keys.size() + 1);
            while (pos >= 0 && comp(key, node->keys[pos]))
            {
                node->keys[pos + 1] = node->keys[pos];
                pos--;
            }
            node->keys[pos + 1] = key;
        }
        else
        {
            while (pos >= 0 && comp(key, node->keys[pos]))
            {
                pos--;
            }
            
            if (isNodeFull(node->children[pos + 1]))
            {
                splitChild(node, pos + 1);
                if (comp(node->keys[pos + 1], key))
                    pos++;
            }
            insertNonFull(node->children[pos + 1], key);
        }
    }
    
    size_t t;
    Node *root;
    Compare comp;
};

int main() {
    size_t t;
    std::cin >> t;
    
    BTree<int> tree(t);
    
    int key;
    while (std::cin >> key) {
        tree.Insert(key);
    }
    
    tree.Traverse(
        [](int key) { std::cout << key << " "; },
        []() { std::cout << std::endl; }
    );
    
    return 0;
}