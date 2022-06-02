/*!
 * @brief This file contains the implementation of class fibonacciHeap.
 */
#include "fibonacciHeap.h"

/*!
 * @brief This no-args constructor creates a new Fibonacci heap
 * with the transparent comparator.
 */
template<typename T, typename Comp>
FibonacciHeap<T, Comp>::FibonacciHeap() : FibonacciHeap(std::less<T>()) {}

/*!
 * @brief This constructor accepts a customized comparator and constructs a new heap.
 * @tparam T is type of keys in nodes.
 * @tparam Comp is the type of comparator.
 * @param comp is the comparator.
 */
template<typename T, typename Comp>
FibonacciHeap<T, Comp>::FibonacciHeap(Comp comp) : comp_(comp) {}

/*!
 * @brief This constructor constructs a new node with given key.
 * @param k is the key of the new node.
 */
template<typename T, typename Comp>
FibonacciHeap<T, Comp>::FibonacciNode::FibonacciNode(T k) : key(std::move(k)) {}

/*!
 * @brief This destructor deallocates all nodes within the Fibonacci heap.
 */
template<typename T, typename Comp>
FibonacciHeap<T, Comp>::~FibonacciHeap() {
    DeallocateTree(min);  // Deallocate all nodes.
}

// APIs.
/*!
 * @brief This method pushes the given object onto the heap.
 * @param k is the input object.
 * @return pointer to the inserted node.
 */
template<typename T, typename Comp>
typename FibonacciHeap<T, Comp>::FibonacciNode* FibonacciHeap<T, Comp>::push(T k) {
    auto x = new FibonacciNode(std::move(k));
    FibHeapInsert(x);
    return x;
}

/*!
 * @brief This method pops the top object off the heap and deallocate it.
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::pop() {
    if (empty()) { return; }
    delete FibHeapExtractMin();
}

/*!
 * @brief This function checks if the heap is empty.
 * @return true if empty, false otherwise.
 */
template<typename T, typename Comp>
bool FibonacciHeap<T, Comp>::empty() const {
    return n == 0;
}

/*!
 * @brief This method returns number of objects in the heap.
 * @return number of objects in the heap.
 */
template<typename T, typename Comp>
unsigned FibonacciHeap<T, Comp>::size() const {
    return n;
}

/*!
 * @brief This method returns the object on top of the heap.
 * @return the top object.
 */
template<typename T, typename Comp>
T& FibonacciHeap<T, Comp>::top() const {
    return min->key;
}

/*!
 * @brief This function merges two Fibonacci heaps.
 * @param H1 is pointer to a Fibonacci heap.
 * @param H2 is pointer to another Fibonacci heap.
 * @return pointer to the resulting heap.
 */
template<typename T, typename Comp>
FibonacciHeap<T, Comp>* FibonacciHeap<T, Comp>::FibHeapUnion(FibonacciHeap* H1, FibonacciHeap* H2) {
    auto H = new FibonacciHeap;
    H->min = H1->min;  // Set H1 the root list.
    if (H->min != nullptr && H2->min != nullptr) {
        concatLists(H->min, H2->min);  // Concatenate the root lists.
    }
    if (H1->min == nullptr || (H2->min != nullptr && H1->comp_(H2->min->key, H1->min->key))) {
        H->min = H2->min;   // Update H.min if necessary.
    }
    H->n = H1->n + H2->n;  // Update total number of nodes.
    return H;
}

// Private methods.
/*!
 * @brief This function deallocates the whole tree `x`.
 * @param x is the root of the tree to deallocate.
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::DeallocateTree(FibonacciNode* x) {
    if (!x) { return; }
    auto iter1 = x; // Used to traverse the siblings.
    do {  // Iterate through all siblings.
        auto iter2 = iter1;
        iter1 = iter1->right;
        DeallocateTree(iter2->child);
        delete iter2;
    } while (iter1 != x);
}

/*!
 * @brief This method inserts a new node into the Fibonacci heap.
 * @param x is pointer to the new node.
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::FibHeapInsert(FibonacciHeap::FibonacciNode* x) {
    // Set node properties.
    x->degree = 0;
    x->p = nullptr;
    x->child = nullptr;
    x->mark = false;
    if (!min) {  // Create a root list containing just x.
        x->left = x->right = x;
        min = x;
    } else {  // Insert x into the root list.
        insertListNode(x, min);
        if (comp_(x->key, min->key)) { min = x; }
    }
    n++;  // Update number of nodes.
}

/*!
 * @brief This method consolidates trees in the Fibonacci heap.
 * @sideeffects The root list contains roots with unique degrees after execution.
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::Consolidate() {
    auto x{min};  // `x` is always in the root list.
    const int MAX_DEGREE =
        static_cast<int>(log(static_cast<double>(n)) / log((1 + sqrt(static_cast<double>(5))) / 2));
    std::vector<FibonacciNode*> A(MAX_DEGREE + 1, nullptr);  // Define vector A filled with null pointers.
    while (true) { // Iterate through the root list until
        int d = x->degree;
        while (A[d]) {  // Root with degree `d` already exists in the root list.
            auto y = A[d];
            if (x == y) { goto EXIT; }
            if (comp_(y->key, x->key)) { std::swap(x, y); }
            FibHeapLink(y, x);
            A[d] = nullptr;  // Trees linked and eliminated.
            d++;  // Continue to check the next possible degree.
        }
        A[d] = x;
        x = x->right;
    }
    EXIT:
    min = x;
    for (auto root : A) {  // For each unique root in list A.
        if (root) {  // If root with current degree exists...
            if (comp_(root->key, min->key)) { min = root; }  // Update min if necessary.
        }
    }
}

/*!
 * @brief This method extracts the minimum root from the Fibonacci heap.
 * @return pointer to the extracted node.
 */
template<typename T, typename Comp>
typename FibonacciHeap<T, Comp>::FibonacciNode* FibonacciHeap<T, Comp>::FibHeapExtractMin() {
    FibonacciHeap::FibonacciNode* z{min};
    if (z) {  // No operation needed if root list is empty.
        // Clear the children's parent pointers.
        clearParentListNode(z->child);
        // Add all children to the root list.
        concatLists(z, z->child);
        // Remove z (H.min) from the root list.
        removeListNode(z);
        // z is the last root and z has no children?
        if (z == z->right) {
            min = nullptr;  // The heap is empty.
        } else {
            min = z->right;  // Point `min` to its sibling.
            Consolidate();
        }
        n--;  // Update number of nodes in the heap.
    }
    return z;
}

/*!
 * @brief This method links one tree to another.
 * @param y is pointer to the root with larger key.
 * @param x is pointer to the root with smaller key.
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::FibHeapLink(FibonacciNode* y, FibonacciNode* x) {
    // Remove y from the root list of the heap.
    removeListNode(y);
    // Make `y` a child of `x`, incrementing x.degree.
    if (x->child) {
        insertListNode(y, x->child);
    } else {  // x did not have a child.
        x->child = y->left = y->right = y;
    }
    y->p = x;
    x->degree++;
    y->mark = false;  // Clear the mark of `y`.
}

/*!
 * @brief This function cuts node `x` from the child list of node `y`.
 * @param x is a child of y.
 * @param y is the parent of x.
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::Cut(FibonacciHeap::FibonacciNode* x, FibonacciHeap::FibonacciNode* y) {
    // Is `x` the only child of `y`?
    if (x->right == x) {
        y->child = nullptr;
    } else {
        // Remove x from the child list.
        removeListNode(x);
        // Update the child pointer of `y` if necessary.
        if (y->child == x) {
            y->child = x->right;
        }
    }
    // Decrement y.degree.
    y->degree--;
    // Add x to the root list of the heap.
    insertListNode(x, min);
    // Set relevant fields of x.
    x->p = nullptr;
    x->mark = false;
}

/*!
 * @brief This function recursively check a node that has just lost a child and perform cutting if necessary.
 * @param y is the node which just lost a child.
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::CascadingCut(FibonacciHeap::FibonacciNode* y) {
    FibonacciNode* z{y->p};
    if (z) {  // If `y` is not the root.
        if (!y->mark) {  // Mark `y` if previously unmarked.
            y->mark = true;
        } else {
            Cut(y, z);  // Cut `y` if already marked.
            CascadingCut(z);  // Call recursively on `y`'s parent.
        }
    }
}

/*!
 * @brief This function decrease the key value of a given node.
 * @param x is the pointer to the node.
 * @param k is the new key value (must be no larger than the current key).
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::FibHeapDecreaseKey(FibonacciHeap::FibonacciNode* x, T k) {
    // Check if new key is valid.
    if (comp_(x->key, k)) {
        std::cerr << "Error: New key is greater than the current key!" << std::endl;
        return;
    }
    // Update current key.
    x->key = std::move(k);
    FibonacciNode* y{x->p};
    // If x is not a root?
    if (y && comp_(x->key, y->key)) {
        Cut(x, y);  // Cut `x` from subtree `y`.
        CascadingCut(y);  // Recursively check and cut `y`.
    }
    // Update `min` pointer if necessary.
    if (comp_(x->key, min->key)) {
        min = x;
    }
}

// Private helper functions.
/*!
 * @brief This function concatenates two circular, doubly linked lists.
 * @param list1 is pointer to list 1.
 * @param list2 is pointer to list 2.
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::concatLists(FibonacciNode*& list1, FibonacciNode*& list2) {
    if (!list1 || !list2) { return; }
    list2->left->right = list1->right;
    list1->right->left = list2->left;
    list2->left = list1;
    list1->right = list2;
}

/*!
 * @brief This function inserts one node into a doubly linked lists.
 * @param node is pointer to the node to insert.
 * @param list is pointer to the list.
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::insertListNode(FibonacciNode*& node, FibonacciNode*& list) {
    if (!node) { return; }
    if (!list) {
        list = node;
        return;
    }
    list->right->left = node;
    node->right = list->right;
    list->right = node;
    node->left = list;
}

/*!
 * @brief This function removes a specified node from the list.
 * @param node is pointer to the node.
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::removeListNode(FibonacciNode* node) {
    if (node->right == node) { return; }
    node->left->right = node->right;
    node->right->left = node->left;
}

/*!
 * @brief This function clears the parent pointer of a given list of children.
 * @param node is pointer to one of the children.
 */
template<typename T, typename Comp>
void FibonacciHeap<T, Comp>::clearParentListNode(FibonacciNode* node) {
    if (!node) { return; }
    auto iter{node};
    do {
        iter->p = nullptr;
        iter = iter->right;
    } while (iter != node);
}

/*!
 * @brief This method returns the pointer to the node with the minimum key value.
 * @return pointer to the smallest node.
 */
template<typename T, typename Comp>
typename FibonacciHeap<T, Comp>::FibonacciNode* FibonacciHeap<T, Comp>::topNode() const {
    return min;
}
