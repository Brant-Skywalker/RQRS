/*!
 * @brief This file contains the implementation of our @em Disk-IO B-Tree.
 */
#include "BTree.h"

/*!
 * @brief This no-arg constructor initializes some fields of the class.
 */
template<typename K, typename V>
BTree<K, V>::Node::Node() : leaf_(false), n_(0) {}

/*!
 * @brief This overloaded constructor further initialize the other two properties.
 * @param pos is desired file IO position.
 * @param isLeaf is the desired `leaf_` attribute.
 */

/*!
 * @brief The no-arg constructor of the B-tree.
 */
template<typename K, typename V>
BTree<K, V>::BTree() {
    root_ = _allocate_node();
    root_->leaf_ = true;
}

/*!
 * @brief This method attempts to search for a key in the tree.
 * @param k is the key object.
 * @return a shared pointer to the value object.
 */
template<typename K, typename V>
std::shared_ptr<V> BTree<K, V>::search(K k) {
    return _search(root_, k);
}

/*!
 * @brief This method recursively searches for a key in the given subtree.
 * @param x is pointer to the root of the subtree.
 * @param k is the key object.
 * @return a shared pointer to the value object.
 */
template<typename K, typename V>
std::shared_ptr<V> BTree<K, V>::_search(std::shared_ptr<Node> x, K& k) {
    if (nullptr == x) { return nullptr; }
    int i = _find_key(x, k);
    if (i < x->n_ && k == x->key_[i]) {
        return std::make_shared<V>(x->val_[i]);
    }
    if (x->leaf_) { return nullptr; }
    return _search(x->c_[i], k);
}

/*!
 * @brief This method attempts to insert a key-value pair into the tree.
 * @param k is the key object.
 * @param v is the value object.
 */
template<typename K, typename V>
void BTree<K, V>::insert(K k, V v) {
    if (nullptr == root_) {
        root_ = _allocate_node();
        root_->leaf_ = true;
    }
    if (root_->n_ == 2 * t - 1) {
        auto s = _allocate_node();  // New root.
        s->c_[0] = root_;
        s->leaf_ = false;
        _split_child(s, 0);
        root_ = s;
    }
    _insert_non_full(root_, k, v);
}

/*!
 * @brief This method splits the child of the given node at given index.
 * @param x is pointer to the given node.
 * @param i is index of the child in the child list of the current node.
 */
template<typename K, typename V>
void BTree<K, V>::_split_child(std::shared_ptr<Node> x, int i) {
    auto y = x->c_[i];
    auto z = _allocate_node();
    z->leaf_ = y->leaf_;
    for (int j = 0; j < t - 1; ++j) {
        z->key_[j] = y->key_[j + t];
        z->val_[j] = y->val_[j + t];
    }
    if (!y->leaf_) {
        for (int j = 0; j < t; ++j) {
            z->c_[j] = y->c_[j + t];
        }
    }
    y->n_ = z->n_ = t - 1;
    for (int j = x->n_; j >= i + 1; --j) {
        x->c_[j + 1] = x->c_[j];
    }
    x->c_[i + 1] = z;
    for (int j = x->n_ - 1; j >= i; --j) {
        x->key_[j + 1] = x->key_[j];
        x->val_[j + 1] = x->val_[j];
    }
    x->key_[i] = y->key_[t - 1];
    x->val_[i] = y->val_[t - 1];
    ++x->n_;
}

/*!
 * @brief This method recursively inserts a given key-value pair into the given subtree.
 * @param x is root of the subtree to perform insertion.
 * @param k is reference to the key object.
 * @param v is reference to the value object.
 */
template<typename K, typename V>
void BTree<K, V>::_insert_non_full(std::shared_ptr<Node> x, K& k, V& v) {
    int i = x->n_ - 1;
    if (x->leaf_) {
        while (i >= 0 && k < x->key_[i]) {
            x->key_[i + 1] = x->key_[i];
            x->val_[i + 1] = x->val_[i];
            --i;
        }
        x->key_[i + 1] = k;
        x->val_[i + 1] = v;
        ++x->n_;
        return;
    }
    while (i >= 0 && k < x->key_[i]) {
        --i;
    }
    i++;
    auto child = x->c_[i];
    if (child->n_ == 2 * t - 1) {
        _split_child(x, i);
        if (k > x->key_[i]) {
            ++i;
        }
        child = x->c_[i];
    }
    _insert_non_full(child, k, v);
}

/*!
 * @brief This method allocates a new node.
 * @return a shared pointer to the node.
 */
template<typename K, typename V>
std::shared_ptr<typename BTree<K, V>::Node> BTree<K, V>::_allocate_node() {
    return std::make_shared<Node>();
}

/*!
 * @brief This method checks to see if the given key exists in the current tree.
 * @param k is the key object.
 * @return true if exists, false otherwise.
 */
template<typename K, typename V>
bool BTree<K, V>::contains(K k) {
    return nullptr != search(k);
}

/*!
 * @brief This method removes the given key from the tree.
 * @param k is the key object to be removed.
 * @return true if successfully removed, false otherwise.
 */
template<typename K, typename V>
bool BTree<K, V>::remove(K k) {
    return _remove_node(root_, k);
}

/*!
 * @brief This method retrieves the predecessor node's key-value pair.
 * @param p is pointer to the parent node.
 * @param i is the index of the target node.
 * @return a key-value pair of the desired node.
 */
template<typename K, typename V>
std::pair<K, V> BTree<K, V>::_get_pred(std::shared_ptr<Node> p, int i) {
    auto curr = p->c_[i];
    while (!curr->leaf_) {  // Now keep moving to the right most child.
        curr = curr->c_[curr->n_];
    }
    return std::make_pair(curr->key_[curr->n_ - 1], curr->val_[curr->n_ - 1]);
}

/*!
 * @brief This method retrieves the successor node's key-value pair.
 * @param p is pointer to the parent node.
 * @param i is the index of the target node.
 * @return a key-value pair of the desired node.
 */
template<typename K, typename V>
std::pair<K, V> BTree<K, V>::_get_succ(std::shared_ptr<Node> x, int i) {
    auto curr = x->c_[i + 1];
    while (!curr->leaf_) {
        curr = curr->c_[0];
    }
    return std::make_pair(curr->key_[0], curr->val_[0]);
}

/*!
 * @brief This method borrows the last key-value pair and the last child pointer from the immediate left sibling.
 * @param p is pointer to the parent node.
 * @param i is index of the current node in the parent's child list.
 */
template<typename K, typename V>
void BTree<K, V>::_borrow_from_prev(std::shared_ptr<Node> p, int i) {
    auto c = p->c_[i];
    auto l = p->c_[i - 1];
    for (int j = c->n_; j > 0; --j) {  // Move all keys in c to the left by 1.
        c->key_[j] = c->key_[j - 1];
        c->val_[j] = c->val_[j - 1];
    }
    if (!c->leaf_) {
        for (int j = c->n_ + 1; j > 0; --j) {
            c->c_[j] = c->c_[j - 1];
        }
    }
    c->key_[0] = p->key_[i - 1];
    c->val_[0] = p->val_[i - 1];
    if (!c->leaf_) {
        c->c_[0] = l->c_[l->n_];
    }
    p->key_[i - 1] = l->key_[l->n_ - 1];
    p->val_[i - 1] = l->val_[l->n_ - 1];
    c->n_++;
    l->n_--;
}

/*!
 * @brief This method borrows the first key-value pair and the first child pointer from the immediate right sibling.
 * @param p is pointer to the parent node.
 * @param i is index of the current node in the parent's child list.
 */
template<typename K, typename V>
void BTree<K, V>::_borrow_from_next(std::shared_ptr<Node> x, int i) {
    auto c = x->c_[i];
    auto r = x->c_[i + 1];

    c->val_[c->n_] = x->val_[i];  // Insert the keys[i] into c.
    c->key_[c->n_] = x->key_[i];
    x->val_[i] = r->val_[0];  // Promote the first key of the sibling.
    x->key_[i] = r->key_[0];
    if (!c->leaf_) {
        c->c_[c->n_ + 1] = r->c_[0];  // Make the first child of sibling the last child of c.
    }
    for (int j = 0; j < r->n_ - 1; ++j) {  // Move all keys in the sibling to the left by 1.
        r->key_[j] = r->key_[j + 1];
        r->val_[j] = r->val_[j + 1];
    }
    if (!r->leaf_) {  // Move the child pointers to the left by one.
        for (int j = 0; j < r->n_; ++j) {
            r->c_[j] = r->c_[j + 1];
        }
    }
    // Update the key counts of c and s.
    r->n_--;
    c->n_++;
}

/*!
 * @brief This method attempts to merge the current node with its right immediate sibling.
 * @param p is pointer to the parent node.
 * @param i is index of the current node in the parent's child list.
 */
template<typename K, typename V>
void BTree<K, V>::_merge(std::shared_ptr<Node> p, int i) {
    auto l = p->c_[i];
    auto r = p->c_[i + 1];
    l->val_[l->n_] = p->val_[i];  // Demote a key from x.
    l->key_[l->n_] = p->key_[i];
    l->n_++;
    for (int j = 0; j < r->n_; ++j) {  // Copy keys from s to c.
        l->key_[j + l->n_] = r->key_[j];
        l->val_[j + l->n_] = r->val_[j];
    }
    if (!l->leaf_) {  // Copy child pointers from s to c.
        for (int j = 0; j <= r->n_; ++j) {
            l->c_[j + l->n_] = r->c_[j];
        }
    }
    l->n_ += r->n_;
    r->n_ = 0;
    for (int j = i; j < p->n_ - 1; ++j) {  // Move keys in x to the left by 1.
        p->key_[j] = p->key_[j + 1];
        p->val_[j] = p->val_[j + 1];
    }
    if (!p->leaf_) {
        for (int j = i + 1; j < p->n_; ++j) {  // Move child pointers in x to the left by 1.
            p->c_[j] = p->c_[j + 1];
        }
    }
    p->n_--;
}

/*!
 * @brief This method attempts to fill the current node so that
 * it has one more element than the minimum requirement.
 * @param p is pointer to the parent node.
 * @param i is index of the current node in the parent's child list.
 */
template<typename K, typename V>
void BTree<K, V>::_fill(std::shared_ptr<Node> p, int i) {
    if (i > 0 && p->c_[i - 1]->n_ > t - 1) {
        _borrow_from_prev(p, i);
        return;
    }
    if (i < p->n_ && p->c_[i + 1]->n_ > t - 1) {
        _borrow_from_next(p, i);
        return;
    }
    if (i < p->n_) {
        _merge(p, i);
        return;
    }
    _merge(p, i - 1);
}

/*!
 * @brief This method recursively remove the given key from the given subtree.
 * @param r is reference of pointer to the root of the subtree.
 * @param k is the key object.
 * @return true if succeeded, false otherwise.
 */
template<typename K, typename V>
bool BTree<K, V>::_remove_node(std::shared_ptr<Node>& r, K k) {
    if (0 == r->n_) {
        r = r->c_[0];
    }
    int i = _find_key(r, k);
    if (i < r->n_ && r->key_[i] == k) {  // The key k exists in node x.
        return r->leaf_ ? _remove_from_leaf(r, i) : _remove_from_non_leaf(r, i);
    }
    if (r->leaf_) {
        return false;
    }
    bool temp = i == r->n_;  // Save current child status.
    if (r->c_[i]->n_ < t) {
        _fill(r, i);
    }
    if (temp && i > r->n_) {  // Merge happened!
        return _remove_node(r->c_[i - 1], k);
    }
    return _remove_node(r->c_[i], k);
}

/*!
 * @brief This finds the appropriate index according to the given key.
 * @param x is the target node.
 * @param k is reference to the key object.
 * @return the appropriate index.
 */
template<typename K, typename V>
int BTree<K, V>::_find_key(std::shared_ptr<Node> x, K& k) {
    int i = 0;
    while (i < x->n_ && x->key_[i] < k) {
        ++i;
    }
    return i;
}

/*!
 * @brief This method removes a designated key-value pair from a leaf.
 * @param x is pointer to the leaf.
 * @param i is index of the key.
 * @return true if succeeded, false otherwise.
 */
template<typename K, typename V>
bool BTree<K, V>::_remove_from_leaf(std::shared_ptr<Node> x, int i) {
    for (int j = i; j < x->n_ - 1; ++j) {
        x->key_[j] = x->key_[j + 1];  // Overwrite the key.
        x->val_[j] = x->val_[j + 1];  // Overwrite the key.
    }
    x->n_--;
    return true;
}

/*!
 * @brief This method removes a designated key-value pair from an internal node.
 * @param x is pointer to the internal node.
 * @param i is index of the key.
 * @return true if succeeded, false otherwise.
 */
template<typename K, typename V>
bool BTree<K, V>::_remove_from_non_leaf(std::shared_ptr<Node>& x, int i) {
    K k = x->key_[i];
    auto l = x->c_[i];
    auto r = x->c_[i + 1];
    if (l->n_ > t - 1) {
        auto p = _get_pred(x, i);
        x->key_[i] = p.first;
        x->val_[i] = p.second;
        return _remove_node(l, p.first);
    }
    if (r->n_ > t - 1) {
        auto s = _get_succ(x, i);
        x->key_[i] = s.first;
        x->val_[i] = s.second;
        return _remove_node(r, s.first);
    }
    _merge(x, i);
    if (0 == x->n_) {
        x = x->c_[0];
    }
    return _remove_node(x, k);
}
