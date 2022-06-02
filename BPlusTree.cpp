/*!
 * @brief This file contains the implementation of class methods of our B+-tree.
 */
#include "BPlusTree.h"

/*!
 * @brief Single-arg constructor of node base.
 * @tparam K is type of key objects.
 * @tparam V is type of value objects.
 * @param isLeaf is 1 if derived class is leaves, 0 otherwise.
 */
template<typename K, typename V>
NodeBase<K, V>::NodeBase(bool isLeaf) :leaf_{isLeaf} {}

/*!
 * @brief This method evaluates the index to which the key should present or
 * be inserted.
 * @param k is the key object.
 * @return the desired index.
 */
template<typename K, typename V>
int NodeBase<K, V>::_get_key_index(K k) const {
    auto it = std::lower_bound(key_.begin(), key_.begin() + n_, k);
    return it - key_.begin() <= n_ - 1 ? it - key_.begin() : n_ - 1;
}

/*!
 * @brief The no-arg constructor of internal nodes.
 */
template<typename K, typename V>
InternalNode<K, V>::InternalNode() : NodeBase<K, V>(false) {}

/*!
 * @brief This method recursively inserts a new key into an internal node.
 * @param k is the key object.
 * @param ki is index to which the key should be inserted.
 * @param c is pointer to the child node to which the key will be inserted.
 * @param ci is the index of the child node in the parent node's child list.
 */
template<typename K, typename V>
void InternalNode<K, V>::_insert(K k, int ki, node_ptr c, int ci) {
    for (int j = this->n_ - 1; j >= ki; --j) {
        this->key_[j + 1] = this->key_[j];
    }
    this->key_[ki] = k;
    for (int j = this->n_; j >= ci; --j) {
        c_[j + 1] = c_[j];
    }
    c_[ci] = c;
    this->n_++;
}

/*!
 * @brief This method splits this node.
 * @param p is pointer to the parent of the current node.
 * @param i is the index of the current node in the child list of the parent.
 */
template<typename K, typename V>
void InternalNode<K, V>::_split(node_ptr p, int i) {
    auto r = std::make_shared<InternalNode>();
    this->n_ = r->n_ = d - 1;
    for (int j = 0; j < d - 1; ++j) {
        r->key_[j] = this->key_[j + d];
    }
    for (int j = 0; j < d; ++j) {
        r->c_[j] = this->c_[j + d];
    }
    auto tp = std::dynamic_pointer_cast<InternalNode>(p);
    tp->_insert(this->key_[d - 1], i, r, i + 1);
}

/*!
 * @brief This method attempts to merge the current node and its right neighbor.
 * @param p is pointer to the parent.
 * @param i is index of the current node in the child list of the parent node.
 * @param r is pointer to the right neighbor.
 */
template<typename K, typename V>
void InternalNode<K, V>::_merge(node_ptr p, int i, node_ptr r) {
    auto tp = std::dynamic_pointer_cast<InternalNode>(p);
    auto tr = std::dynamic_pointer_cast<InternalNode>(r);
    _insert(p->key_[i], d - 1, tr->c_[0], d);
    for (int j = 1; j <= tr->n_; ++j) {
        _insert(tr->key_[j - 1], d + j - 1, tr->c_[j], d + j);
    }
    tp->_remove(i, i + 1);
}

/*!
 * @brief This method attempts to remove a key object and a child pointer from the current
 * internal node.
 * @param ki is index of the key to be removed.
 * @param ci is index of the child pointer to be removed.
 */
template<typename K, typename V>
void InternalNode<K, V>::_remove(int ki, int ci) {
    for (int j = ki + 1; j < this->n_; ++j) {
        this->key_[j - 1] = this->key_[j];
    }
    for (int j = ci + 1; j <= this->n_; ++j) {
        this->c_[j - 1] = this->c_[j];
    }
    this->n_--;
}

/*!
 * @brief This method borrows the last key (with the last child pointer) from the left neighbor.
 * @param p is pointer to the parent of the current node.
 * @param i is index of the current node in the child list of the parent.
 * @param l is pointer to the left neighbor.
 */
template<typename K, typename V>
void InternalNode<K, V>::_borrow_from_left(node_ptr p, int i, node_ptr l) {
    auto tl = std::dynamic_pointer_cast<InternalNode>(l);
    if (tl->c_[tl->n_]->leaf_) {
        auto t = std::dynamic_pointer_cast<LeafNode<K, V>>(tl->c_[tl->n_]);
        t->_load_overflow();
    }
    _insert(p->key_[i], 0, tl->c_[tl->n_], 0);
    p->key_[i] = tl->key_[tl->n_ - 1];  // Get the last element of its left sibling.
    tl->_remove(tl->n_ - 1, tl->n_);
}

/*!
 * @brief This method borrows the first key (with the first child pointer) from the right neighbor.
 * @param p is pointer to the parent of the current node.
 * @param i is index of the current node in the child list of the parent.
 * @param l is pointer to the right neighbor.
 */
template<typename K, typename V>
void InternalNode<K, V>::_borrow_from_right(node_ptr p, int i, node_ptr r) {
    auto tr = std::dynamic_pointer_cast<InternalNode>(r);
    if (tr->c_[0]->leaf_) {
        auto t = std::dynamic_pointer_cast<LeafNode<K, V>>(tr->c_[0]);
        t->_load_overflow();
    }
    _insert(p->key_[i], this->n_, tr->c_[0], this->n_ + 1);
    p->key_[i] = tr->key_[0];  // Get the first element of its right sibling.
    tr->_remove(0, 0);
}

/*!
 * @brief This method picks an appropriate child pointer according to the key object.
 * @param k is the key object.
 * @param i is index of the key in the current node.
 * @return index of the appropriate child index.
 */
template<typename K, typename V>
int InternalNode<K, V>::_get_child_index(K k, int i) {
    return k < this->key_[i] ? i : i + 1;
}

/*!
 * @brief No-arg constructor of the leaf nodes.
 */
template<typename K, typename V>
LeafNode<K, V>::LeafNode() : NodeBase<K, V>(true) {}

/*!
 * @brief This method attempts to sort the key-value pair of the overflow block.
 */
template<typename K, typename V>
void LeafNode<K, V>::_sort_overflow() {
    std::vector<std::pair<K, V>> pairs(overflow_n_);
    for (int j = 0; j < overflow_n_; ++j) {
        pairs[j] = std::make_pair(overflow_key_[j], overflow_val_[j]);
    }
    std::sort(pairs.begin(), pairs.end(), [&](const auto& lhs, const auto& rhs) {
        return lhs.first < rhs.first;
    });
    for (int j = 0; j < overflow_n_; ++j) {
        overflow_key_[j] = pairs[j].first;
        overflow_val_[j] = pairs[j].second;
    }
}

/*!
 * @brief This method moves all key-value pairs from the overflow list to the main page.
 */
template<typename K, typename V>
void LeafNode<K, V>::_load_overflow() {
    if (0 == overflow_n_) { return; }
    _sort_overflow();
    for (int j = 0; j < overflow_n_; ++j) {
        _insert_main(overflow_key_[j], overflow_val_[j]);
    }
    overflow_n_ = 0;
}

/*!
 * @brief This method inserts a given key-value pair into the current leaf node.
 * @param k is the key object.
 * @param v is the value object.
 */
template<typename K, typename V>
void LeafNode<K, V>::_insert(K k, V v) {
    if (2 * d - 1 - d / 2 < this->n_) {
        _insert_main(k, v);
        return;
    }
    if (d / 2 == overflow_n_) {
        _load_overflow();
    }
    overflow_key_[overflow_n_] = k;
    overflow_val_[overflow_n_++] = v;
}

/*!
 * @brief This method directly inserts the given key-value pair into the main page.
 * @param k is the key object.
 * @param v is the value object.
 */
template<typename K, typename V>
void LeafNode<K, V>::_insert_main(K k, V v) {
    int j = this->n_ - 1;
    for (; j >= 0 && k < this->key_[j]; --j) {
        this->key_[j + 1] = this->key_[j];
        this->val_[j + 1] = this->val_[j];
    }
    this->key_[j + 1] = k;
    this->val_[j + 1] = v;
    this->n_++;
}

/*!
 * @brief This method removes the `i`th key-value pair from the current leaf node.
 * @param i is index of the node to be removed.
 */
template<typename K, typename V>
void LeafNode<K, V>::_remove(int i) {
    _load_overflow();
    for (int j = i + 1; j < this->n_; ++j) {
        this->key_[j - 1] = this->key_[j];
        this->val_[j - 1] = this->val_[j];
    }
    this->n_--;
}

/*!
 * @brief This method attempts to split the current node.
 * @param p is pointer to the parent of the current node.
 * @param i is index of the current node in the parent's child list.
 */
template<typename K, typename V>
void LeafNode<K, V>::_split(node_ptr p, int i) {
    _load_overflow();
    auto r = std::make_shared<LeafNode>();
    this->n_ = d - 1;
    r->n_ = d;
    if (r_) {
        auto tr = std::dynamic_pointer_cast<LeafNode>(r_);
        tr->l_ = r;
        r->r_ = tr;
    }
    r_ = r;
    r->l_ = this->shared_from_this();
    for (int j = 0; j < d; ++j) {
        r->key_[j] = this->key_[j + d - 1];
        r->val_[j] = this->val_[j + d - 1];
    }
    auto tp = std::dynamic_pointer_cast<InternalNode<K, V>>(p);
    tp->_insert(this->key_[d - 1], i, r, i + 1);
}

/*!
 * @brief This method merges to leaf nodes.
 * @param p is pointer to the parent of the current node.
 * @param i is index of the current node in the parent's child list.
 * @param r is pointer to the right neighbor of the current node.
 */
template<typename K, typename V>
void LeafNode<K, V>::_merge(node_ptr p, int i, node_ptr r) {
    auto tp = std::dynamic_pointer_cast<InternalNode<K, V>>(p);
    auto tr = std::dynamic_pointer_cast<LeafNode>(r);
    _load_overflow();
    tr->_load_overflow();
    for (int j = 0; j < tr->n_; ++j) {
        _insert(tr->key_[j], tr->val_[j]);
    }
    _load_overflow();
    r_ = tr->r_;
    if (tr->r_) {
        auto trr = std::dynamic_pointer_cast<LeafNode>(tr->r_);
        trr->l_ = this->shared_from_this();
    }
    tp->_remove(i, i + 1);
}

/*!
 * @brief This method borrows the first key (with the first child pointer) from the right neighbor.
 * @param p is pointer to the parent of the current node.
 * @param i is index of the current node in the child list of the parent.
 * @param l is pointer to the right neighbor.
 */
template<typename K, typename V>
void LeafNode<K, V>::_borrow_from_left(node_ptr p, int i, node_ptr l) {
    auto tl = std::dynamic_pointer_cast<LeafNode>(l);
    tl->_load_overflow();
    _load_overflow();
    _insert_main(tl->key_[tl->n_ - 1], tl->val_[tl->n_ - 1]);
    tl->_remove(tl->n_ - 1);
    p->key_[i] = this->key_[0];  // Transferred new key.
}

/*!
 * @brief This method borrows the first key (with the first child pointer) from the right neighbor.
 * @param p is pointer to the parent of the current node.
 * @param i is index of the current node in the child list of the parent.
 * @param l is pointer to the right neighbor.
 */
template<typename K, typename V>
void LeafNode<K, V>::_borrow_from_right(node_ptr p, int i, node_ptr r) {
    auto tr = std::dynamic_pointer_cast<LeafNode>(r);
    tr->_load_overflow();
    _load_overflow();
    _insert_main(tr->key_[0], tr->val_[0]);
    tr->_remove(0);
    p->key_[i] = tr->key_[0];
}

/*!
 * @brief This method attempts to insert the given key-value pair into the tree.
 * @param k is the key object.
 * @param v is the value object.
 */
template<typename K, typename V>
void BPlusTree<K, V>::insert(K k, V v) {
    if (nullptr == root_) {
        auto tr = std::make_shared<LeafNode<K, V>>();
        root_ = std::dynamic_pointer_cast<NodeBase<K, V>>(tr);
    }
    if (root_->leaf_) {
        auto tr = std::dynamic_pointer_cast<LeafNode<K, V>>(root_);
        tr->_load_overflow();
    }
    if (root_->n_ == 2 * d - 1) {
        auto r = std::make_shared<InternalNode<K, V>>();
        r->c_[0] = root_;
        root_->_split(r, 0);  // This is a leaf node.
        root_ = std::dynamic_pointer_cast<NodeBase<K, V>>(r);  // Update root pointer.
    }
    _insert(root_, k, v);
}

/*!
 * @brief This method recursively inserts the given key-value pair into the given subtree.
 * @param p is pointer to the root of the subtree to perform insertion.
 * @param k is the key object.
 * @param v is the value object.
 */
template<typename K, typename V>
void BPlusTree<K, V>::_insert(node_ptr p, K k, V v) {
    if (p->leaf_) {
        auto tp = std::dynamic_pointer_cast<LeafNode<K, V>>(p);
        tp->_insert(k, v);
        return;
    }
    // p is an internal node!
    auto tp = std::dynamic_pointer_cast<InternalNode<K, V>>(p);
    int ki = tp->_get_key_index(k);
    int ci = tp->_get_child_index(k, ki);
    auto c = tp->c_[ci];
    if (c->leaf_) {
        auto tc = std::dynamic_pointer_cast<LeafNode<K, V>>(c);
        tc->_load_overflow();
    }
    if (c->n_ >= 2 * d - 1) {
        c->_split(tp, ci);
        if (tp->key_[ci] <= k) {
            c = tp->c_[ci + 1];  // Update child node if necessary.
        }
    }
    _insert(c, k, v);
}

/*!
 * @brief This method checks if the given key exists in the tree.
 * @param k is the key object.
 * @return true if exists, false if not exists.
 */
template<typename K, typename V>
bool BPlusTree<K, V>::contains(K k) {
    return nullptr != search(k);
}

/*!
 * @brief This API attempts to remove the given key from the tree.
 * @param k is the key object.
 * @return a shared pointer to the desired value object.
 */
template<typename K, typename V>
std::shared_ptr<V> BPlusTree<K, V>::search(K k) {
    return _search(root_, k);
}

/*!
 * @brief This method recursively searches for a given key in the given subtree.
 * @param p is pointer to the root of the subtree to perform searching.
 * @param k is the key object.
 * @return a shared pointer to the desired value object.
 */
template<typename K, typename V>
std::shared_ptr<V> BPlusTree<K, V>::_search(node_ptr p, K k) {
    if (nullptr == p) { return nullptr; }
    if (p->leaf_) {  // Directly search and return pointer-to-value.
        auto tp = std::dynamic_pointer_cast<LeafNode<K, V>>(p);
        tp->_load_overflow();
        int i = tp->_get_key_index(k);
        if (i < tp->n_ && k == tp->key_[i]) {  // Found in leaf node!
            return std::make_shared<V>(tp->val_[i]);
        }
        return nullptr;  // Not found.
    }
    auto tp = std::dynamic_pointer_cast<InternalNode<K, V>>(p);
    int i = tp->_get_key_index(k);
    int ci = tp->_get_child_index(k, i);
    return _search(tp->c_[ci], k);
}

/*!
 * @brief This API tries to remove the input key from the tree.
 * @param k is the key object.
 * @return true if succeeded, false if failed.
 */
template<typename K, typename V>
bool BPlusTree<K, V>::remove(K k) {
    if (!search(k)) { return false; }
    if (1 == root_->n_ && !root_->leaf_) {  // `root_` is an internal node with only one key.
        auto tr = std::dynamic_pointer_cast<InternalNode<K, V>>(root_);
        auto l = tr->c_[0];
        auto r = tr->c_[1];
        if (d - 1 == l->n_ && d - 1 == r->n_) {
            l->_merge(tr, 0, r);
            root_ = l;  // Make the left child the root.
        }
    }
    _remove(root_, k);
    return true;
}

/*!
 * @brief This method recursively remove the given key from given subtree.
 * @param p is pointer to the root of the subtree to perform removal.
 * @param k is the key object.
 */
template<typename K, typename V>
void BPlusTree<K, V>::_remove(node_ptr p, K k) {
    if (p->leaf_) {
        auto tp = std::dynamic_pointer_cast<LeafNode<K, V>>(p);
        tp->_load_overflow();
        int i = tp->_get_key_index(k);
        tp->_remove(i);  // Mission accomplished.
        return;
    }
    auto tp = std::dynamic_pointer_cast<InternalNode<K, V>>(p);
    int i = tp->_get_key_index(k);
    int ci = tp->_get_child_index(k, i);
    auto c = tp->c_[ci];
    if (c->leaf_) {
        auto tc = std::dynamic_pointer_cast<LeafNode<K, V>>(c);
        tc->_load_overflow();
    }
    if (d - 1 == c->n_) {
        auto l = ci > 0 ? tp->c_[ci - 1] : nullptr;
        auto r = ci < tp->n_ ? tp->c_[ci + 1] : nullptr;
        if (l && l->leaf_) {
            auto tl = std::dynamic_pointer_cast<LeafNode<K, V>>(l);
            tl->_load_overflow();
        }
        if (r && r->leaf_) {
            auto tr = std::dynamic_pointer_cast<LeafNode<K, V>>(r);
            tr->_load_overflow();
        }
        if (l && l->n_ > d - 1) {
            c->_borrow_from_left(tp, ci - 1, l);
        } else if (r && r->n_ > d - 1) {
            c->_borrow_from_right(tp, ci, r);
        } else if (l) {
            l->_merge(tp, ci - 1, c);  // Perform the merge operation on the left child and then overwrite.
            c = l;
        } else {
            c->_merge(tp, ci, r);
        }
    }
    _remove(c, k);
}


