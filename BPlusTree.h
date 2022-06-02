#ifndef CS225_SP22_C2_BETA_BPLUSTREE_BPLUSTREE_H_
#define CS225_SP22_C2_BETA_BPLUSTREE_BPLUSTREE_H_

#include <vector>
#include <queue>
#include <array>
#include <iostream>
#include <algorithm>
#include <memory>

inline constexpr int d{32};  // The minimum degree of the B-tree.

template<typename K, typename V>
class NodeBase {
#ifdef DEBUG
    public:
#else
protected:
#endif
    using node_ptr = std::shared_ptr<NodeBase>;

    explicit NodeBase(bool isLeaf);
    virtual ~NodeBase() = default;
    int _get_key_index(K k) const;

    virtual void _split(node_ptr p, int i) = 0;
    virtual void _merge(node_ptr p, int i, node_ptr r) = 0;
    virtual void _borrow_from_left(node_ptr p, int i, node_ptr l) = 0;
    virtual void _borrow_from_right(node_ptr p, int i, node_ptr r) = 0;

    bool leaf_{};
    int n_{};
    std::array<K, 2 * d - 1> key_{};
};

template<typename K, typename V>
class InternalNode : public NodeBase<K, V> {
    using node_ptr = typename NodeBase<K, V>::node_ptr;
public:
    InternalNode();
    virtual ~InternalNode() = default;

    void _remove(int ki, int ci);
    void _insert(K k, int ki, node_ptr c, int ci);
    void _split(node_ptr p, int i) override;
    void _merge(node_ptr p, int i, node_ptr r) override;
    void _borrow_from_left(node_ptr p, int i, node_ptr l) override;
    void _borrow_from_right(node_ptr p, int i, node_ptr r) override;
    int _get_child_index(K k, int i);

#ifdef DEBUG
    public:
#else
private:
#endif
    std::array<node_ptr, 2 * d> c_;
};

template<typename K, typename V>
class LeafNode : public NodeBase<K, V>, public std::enable_shared_from_this<LeafNode<K, V>> {
    using node_ptr = typename NodeBase<K, V>::node_ptr;
public:
    LeafNode();
    virtual ~LeafNode() = default;

    void _insert(K k, V v);
    void _insert_main(K k, V v);
    void _remove(int i);
    void _split(node_ptr p, int i) override;
    void _merge(node_ptr p, int i, node_ptr r) override;
    void _borrow_from_left(node_ptr p, int i, node_ptr l) override;
    void _borrow_from_right(node_ptr p, int i, node_ptr r) override;
    void _load_overflow();
    void _sort_overflow();

#ifdef DEBUG
    public:
#else
private:
#endif
    std::array<V, 2 * d - 1> val_{};  // Main page.
    node_ptr l_{};
    node_ptr r_{};
    int overflow_n_{};
    std::array<K, d / 2> overflow_key_{};
    std::array<V, d / 2> overflow_val_{};
};

template<typename K, typename V>
class BPlusTree {
    using node_ptr = typename NodeBase<K, V>::node_ptr;
public:
    BPlusTree() = default;
    virtual ~BPlusTree() = default;

    void insert(K k, V v);
    bool remove(K k);
    bool contains(K k);
    std::shared_ptr<V> search(K k);

private:
    void _insert(node_ptr p, K k, V v);
    void _remove(node_ptr p, K k);
    std::shared_ptr<V> _search(node_ptr p, K k);

    node_ptr root_{};
};

#endif //CS225_SP22_C2_BETA_BPLUSTREE_BPLUSTREE_H_
