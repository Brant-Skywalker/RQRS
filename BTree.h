/*!
 * @brief This file contains the class definition of our @em Disk-IO B-tree.
 */
#ifndef CS225_SP22_C2_BETA_B_TREE_BTREE_H_
#define CS225_SP22_C2_BETA_B_TREE_BTREE_H_

#include <vector>
#include <string>
#include <iostream>
#include <array>
#include <utility>
#include <memory>

inline constexpr int t{16};  // The minimum degree of the B-tree.

/*!
 * @brief This class defines a B-tree data structure, which uses the secondary storage! Magic?
 * @tparam K is type of key objects.
 * @tparam V is type of value objects.
 */
template<typename K, typename V>
class BTree {
#ifndef DEBUG
private:
#else
    public:
#endif
    class Node {
        friend BTree;

#ifndef DEBUG
    private:
#else
        public:
#endif
        Node();
        virtual ~Node() = default;
        std::array<K, (size_t) 2 * t - 1> key_{};
        bool leaf_;
        int n_;  // Current amount of keys.
        std::array<V, (size_t) 2 * t - 1> val_{};
        std::array<std::shared_ptr<Node>, (size_t) 2 * t> c_{};  // Positions of child nodes in second storage.
    };

public:
    BTree();
    virtual ~BTree() = default;
    std::shared_ptr<V> search(K k);
    void insert(K k, V v);
    bool remove(K k);
    bool contains(K k);

#ifndef DEBUG
private:
#else
    public:
#endif
    std::shared_ptr<Node> root_;

    std::shared_ptr<V> _search(std::shared_ptr<Node> x, K& k);
    void _split_child(std::shared_ptr<Node> x, int i);
    void _insert_non_full(std::shared_ptr<Node> x, K& k, V& v);
    std::pair<K, V> _get_pred(std::shared_ptr<Node> p, int i);
    std::pair<K, V> _get_succ(std::shared_ptr<Node> x, int i);
    void _borrow_from_prev(std::shared_ptr<Node> p, int i);
    void _borrow_from_next(std::shared_ptr<Node> x, int i);
    void _merge(std::shared_ptr<Node> p, int i);
    void _fill(std::shared_ptr<Node> p, int i);
    int _find_key(std::shared_ptr<Node> x, K& k);
    bool _remove_from_leaf(std::shared_ptr<Node> x, int i);
    bool _remove_from_non_leaf(std::shared_ptr<Node>& x, int i);
    bool _remove_node(std::shared_ptr<Node>& r, K k);
    std::shared_ptr<Node> _allocate_node();
};

#endif //CS225_SP22_C2_BETA_B_TREE_BTREE_H_
