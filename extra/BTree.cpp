/*!
 * @brief This file contains the implementation of our @em Disk-IO B-Tree.
 */
#include "BTree.h"

/*!
 * @brief This no-arg constructor initializes some fields of the class.
 */
template<typename K, typename V>
BTree<K, V>::Node::Node() : leaf_(false), pos_(-1), n_(0) {}

/*!
 * @brief This overloaded constructor further initialize the other two properties.
 * @param pos is desired file IO position.
 * @param isLeaf is the desired `leaf_` attribute.
 */
template<typename K, typename V>
BTree<K, V>::Node::Node(int pos, bool isLeaf) : Node() { // Delegating constructor.
    pos_ = pos;
    leaf_ = isLeaf;
}

/*!
 * @brief
 */
template<typename K, typename V>
BTree<K, V>::BTree() : root_(-1), next_pos_(0) {
    std::filesystem::remove(PATH);  // C++17.
    (void) fclose(fopen(PATH, "a"));
    fp_ = fopen(PATH, "r+");
}

template<typename K, typename V>
BTree<K, V>::~BTree() {
    (void) fclose(fp_);
    std::filesystem::remove(PATH);  // C++17.
}

/*!
 * @brief
 * @param k
 * @return
 */
template<typename K, typename V>
std::shared_ptr<V> BTree<K, V>::search(K k) {
    auto root = _allocate_node(root_, false);
    _disk_read(root);
    return _search(root, k);
}

/*!
 * @brief
 * @param x
 * @param k
 * @return
 */
template<typename K, typename V>
std::shared_ptr<V> BTree<K, V>::_search(std::shared_ptr<Node>& x, K& k) {
    if (!x) { return std::shared_ptr<V>(); }
    int i = 0;
    while (i < x->n_ && k > x->key_[i]) {
        ++i;
    }
    if (i < x->n_ && k == x->key_[i]) {
        return std::make_shared<V>(x->val_[i]);
    }
    if (x->leaf_) { return std::shared_ptr<V>(); }
    auto child = _allocate_node(x->c_[i], false);
    _disk_read(child);
    return _search(child, k);
}

/*!
 * @brief
 * @param k
 * @param v
 */
template<typename K, typename V>
void BTree<K, V>::insert(K k, V v) {
    if (0 == next_pos_) {
        root_ = 0;
        auto root = _allocate_node(next_pos_++, true);
        root->key_[0] = k;
        root->val_[0] = v;
        root->n_++;
        _disk_write(root);
        return;
    }
    auto r = _allocate_node(root_, false);
    _disk_read(r);
    if (r->n_ != 2 * t - 1) {
        _insert_non_full(r, k, v);
        return;
    }
    auto s = _allocate_node(next_pos_++, false);  // New root.
    s->c_[0] = root_;
    root_ = s->pos_;
    _split_child(s, 0, r);
    int i = (s->key_[0] < k) ? 1 : 0;
    auto ch = _allocate_node(s->c_[i], false);
    _disk_read(ch);
    _insert_non_full(ch, k, v);
}

/*!
 * @brief
 * @param x
 */
template<typename K, typename V>
void BTree<K, V>::_disk_read(std::shared_ptr<Node>& x) {
    int pos = x->pos_;
    (void) fseek(fp_, pos * sizeof(Node), 0);
    fread(x.get(), sizeof(Node), 1, fp_);
}

template<typename K, typename V>
void BTree<K, V>::_disk_write(std::shared_ptr<Node>& x) {
    int pos = x->pos_;
    if (pos == -1) {
        pos = next_pos_++;
    }
    (void) fseek(fp_, pos * sizeof(Node), 0);
    fwrite(x.get(), sizeof(Node), 1, fp_);
}

/*!
 * @brief
 * @param x
 * @param i
 * @param y
 */
template<typename K, typename V>
void BTree<K, V>::_split_child(std::shared_ptr<Node>& x, int i, std::shared_ptr<Node>& y) {
    auto z = _allocate_node(next_pos_++, y->leaf_);
    z->n_ = t - 1;
    for (int j = 0; j < t - 1; ++j) {
        z->key_[j] = y->key_[j + t];
        z->val_[j] = y->val_[j + t];
    }
    if (!y->leaf_) {
        for (int j = 0; j < t; ++j) {
            z->c_[j] = y->c_[j + t];
        }
    }
    y->n_ = t - 1;
    for (int j = x->n_; j >= i + 1; --j) {
        x->c_[j + 1] = x->c_[j];
    }
    x->c_[i + 1] = z->pos_;
    for (int j = x->n_ - 1; j >= i; --j) {
        x->key_[j + 1] = x->key_[j];
        x->val_[j + 1] = x->val_[j];
    }
    x->key_[i] = y->key_[t - 1];
    x->val_[i] = y->val_[t - 1];
    ++x->n_;
    _disk_write(y);
    _disk_write(z);
    _disk_write(x);
}

/*!
 * @brief
 * @param x
 * @param k
 * @param v
 */
template<typename K, typename V>
void BTree<K, V>::_insert_non_full(std::shared_ptr<Node>& x, K& k, V& v) {
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
        _disk_write(x);
    } else {
        while (i >= 0 && k < x->key_[i]) {
            --i;
        }
        auto child = _allocate_node(x->c_[i + 1], false);
        _disk_read(child);
        if (child->n_ == 2 * t - 1) {
            _split_child(x, i + 1, child);
            if (k > x->key_[i + 1]) {
                ++i;
            }
        }
        child->pos_ = x->c_[i + 1];
        _disk_read(child);
        _insert_non_full(child, k, v);
    }
}

/*!
 * @brief
 * @param pos
 * @param isLeaf
 * @return
 */
template<typename K, typename V>
std::shared_ptr<typename BTree<K, V>::Node> BTree<K, V>::_allocate_node(int pos, bool isLeaf) {
    return std::make_shared<Node>(pos, isLeaf);
}

/*!
 * @brief
 * @param key
 * @return
 */
template<typename K, typename V>
bool BTree<K, V>::contains(K k) {
    return std::shared_ptr<V>() != search(k);
}

/*!
 * @brief
 * @param key
 * @return
 */
template<typename K, typename V>
bool BTree<K, V>::remove(K k) {
    auto r = _allocate_node(root_);
    _disk_read(r);
    return _remove_node(r, k);
}

/*!
 * @brief
 * @param x
 * @param i
 * @return
 */
template<typename K, typename V>
std::shared_ptr<typename BTree<K, V>::Node> BTree<K, V>::_get_pred(std::shared_ptr<Node> x, int i) {
    auto curr = _allocate_node(x->c_[i]);
    _disk_read(curr);
    while (!curr->leaf_) {  // Now keep moving to the right most child.
        curr->pos_ = curr->c_[curr->n_];
        _disk_read(curr);
    }
//    return std::make_pair(std::make_shared<K>(curr->key_[curr->n_ - 1]),
//                          std::make_shared<V>(curr->val_[curr->n_ - 1]));
    return curr;
}

/*!
 * @brief
 * @param x
 * @param i
 * @return
 */
template<typename K, typename V>
std::shared_ptr<typename BTree<K, V>::Node> BTree<K, V>::_get_succ(std::shared_ptr<Node> x, int i) {
    auto curr = _allocate_node(x->c_[i + 1]);
    _disk_read(curr);
    while (!curr->leaf_) {
        curr->pos_ = curr->c_[0];
        _disk_read(curr);
    }
//    return std::make_pair(std::make_shared<K>(curr->key_[0]),
//                          std::make_shared<V>(curr->val_[0]));
    return curr;
}

/*!
 * @brief
 * @param x
 * @param i
 */
template<typename K, typename V>
void BTree<K, V>::_borrow_from_prev(std::shared_ptr<Node> x, int i) {
    auto c = _allocate_node(x->c_[i]);
    auto l = _allocate_node(x->c_[i - 1]);
    _disk_read(c);  // Load two children from disk.
    _disk_read(l);
    for (int j = c->n_; j > 0; --j) {  // Move all keys in c to the left by 1.
        c->key_[j] = c->key_[j - 1];
        c->val_[j] = c->val_[j - 1];
    }
    if (!c->leaf_) {
        for (int j = c->n_ + 1; j > 0; --j) {
            c->c_[j] = c->c_[j - 1];
        }
    }
    c->key_[0] = x->key_[i - 1];
    c->val_[0] = x->val_[i - 1];
    if (!c->leaf_) {
        c->c_[0] = l->c_[l->n_];
    }
    x->key_[i - 1] = l->key_[l->n_ - 1];
    x->val_[i - 1] = l->val_[l->n_ - 1];
    c->n_++;
    l->n_--;
    _disk_write(c);
    _disk_write(l);
    _disk_write(x);
}

/*!
 * @brief
 * @param x
 * @param i
 */
template<typename K, typename V>
void BTree<K, V>::_borrow_from_next(std::shared_ptr<Node> x, int i) {
    auto c = _allocate_node(x->c_[i]);
    auto r = _allocate_node(x->c_[i + 1]);
    _disk_read(c);  // Load two children from disk.
    _disk_read(r);
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
    _disk_write(c);
    _disk_write(r);
    _disk_write(x);
}

/*!
 * @brief
 * @param x
 * @param i
 */
template<typename K, typename V>
void BTree<K, V>::_merge(std::shared_ptr<Node> x, int i) {
    auto l = _allocate_node(x->c_[i]);
    auto r = _allocate_node(x->c_[i + 1]);
    _disk_read(l);
    _disk_read(r);
    l->val_[l->n_] = x->val_[i];  // Demote a key from x.
    l->key_[l->n_] = x->key_[i];
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
    for (int j = i; j < x->n_ - 1; ++j) {  // Move keys in x to the left by 1.
        x->key_[j] = x->key_[j + 1];
        x->val_[j] = x->val_[j + 1];
    }
    if (!x->leaf_) {
        for (int j = i + 1; j < x->n_; ++j) {  // Move child pointers in x to the left by 1.
            x->c_[j] = x->c_[j + 1];
        }
    }
    x->n_--;
    _disk_write(l);
    _disk_write(r);
    _disk_write(x);
}

/*!
 * @brief
 * @param x
 * @param i
 */
template<typename K, typename V>
void BTree<K, V>::_fill(std::shared_ptr<Node> x, int i) {
    if (i > 0) {
        auto p = _allocate_node(x->c_[i - 1]);
        _disk_read(p);
        if (p->n_ > t - 1) {
            _borrow_from_prev(x, i);
            return;
        }
    }
    if (i < x->n_) {
        auto n = _allocate_node(x->c_[i + 1]);
        _disk_read(n);
        if (n->n_ > t - 1) {
            _borrow_from_next(x, i);
            return;
        }
    }
    if (i < x->n_) {
        _merge(x, i);
        return;
    }
    _merge(x, i - 1);
}

/*!
 * @brief
 * @param x
 * @param k
 * @return
 */
template<typename K, typename V>
bool BTree<K, V>::_remove_node(std::shared_ptr<Node> x, K k) {
    bool res;
    if (0 == x->n_) {
        if (x->pos_ == root_) {
            root_ = x->c_[0];
        }
        auto temp = _allocate_node(x->c_[0]);
        x.swap(temp);
        _disk_read(x);
    }
    int i = _find_key(x, k);
    if (i < x->n_ && x->key_[i] == k) {  // The key k exists in node x.
        res = x->leaf_ ? _remove_from_leaf(x, i) : _remove_from_non_leaf(x, i);
        _disk_write(x);
        return res;
    }
    if (x->leaf_) {
        return false;
    }
    bool temp = i == x->n_;  // Save current child status.
    auto c = _allocate_node(x->c_[i]);
    _disk_read(c);
    if (c->n_ < t) {
        _fill(x, i);
        _disk_read(c);
    }
    if (temp && i > x->n_) {  // Merge happened!
        auto s = _allocate_node(x->c_[i - 1]);
        _disk_read(s);  // Load the left sibling where k must be.
        res = _remove_node(s, k);
        _disk_write(s);
    } else {
        auto l = _allocate_node(x->c_[i]);
        _disk_read(l);
        res = _remove_node(l, k);
        _disk_write(l);
    }
    return res;
}

/*!
 * @brief
 * @param x
 * @param k
 * @return
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
 * @brief
 * @param x
 * @param i
 * @return
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
 * @brief
 * @param x
 * @param i
 * @return
 */
template<typename K, typename V>
bool BTree<K, V>::_remove_from_non_leaf(std::shared_ptr<Node> x, int i) {
    K k = x->key_[i];
    bool res;
    auto l = _allocate_node(x->c_[i]);
    _disk_read(l);
    auto r = _allocate_node(x->c_[i + 1]);
    _disk_read(r);
    if (l->n_ > t - 1) {
        auto p = _get_pred(x, i);
        _disk_read(p);
        x->key_[i] = p->key_[p->n_ - 1];
        x->val_[i] = p->val_[p->n_ - 1];
        res = _remove_node(l, x->key_[i]);
    } else if (r->n_ > t - 1) {
        auto s = _get_succ(x, i);
        x->key_[i] = s->key_[0];
        x->val_[i] = s->val_[0];
        res = _remove_node(r, x->key_[i]);
    } else {
        _merge(x, i);
        if (0 == x->n_) {
            if (x->pos_ == root_) {
                root_ = x->c_[0];  // Update root pointer if necessary.
            }
            auto temp = _allocate_node(x->c_[0]);
            x.swap(temp);
            _disk_read(x);
        }
        res = _remove_node(x, k);
    }
    return res;
}
