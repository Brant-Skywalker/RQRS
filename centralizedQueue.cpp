/*!
 * @brief This file implements all class methods of @em CentralizedQueue.
 */
#include "centralizedQueue.h"

/*!
 * @brief This constructor accepts a customized comparator.
 * @tparam T is type of objects contained in the priority queue.
 * @tparam Comp is type of comparator.
 * @param comp_ is the customized comparator.
 */
template<typename T, typename Comp, typename KeyHasher>
CentralizedQueue<T, Comp, KeyHasher>::CentralizedQueue(Comp comp_)
    : Heap(comp_) {}

/*!
 * @brief This method decreases a key and update the multimap and the heap.
 * @param x is pointer to the node to be decreased.
 * @param k is the new key (pass by value).
 */
template<typename T, typename Comp, typename KeyHasher>
void CentralizedQueue<T, Comp, KeyHasher>::decreaseKey(Node* x, T k) {
    multimap_.erase(x->key); // Remove the record with the key.
    multimap_.insert({k, x});  // Insert the new pair into the map.
    Heap::FibHeapDecreaseKey(x, std::move(k));  // Perform heap operation.
}

/*!
 * @brief This method pushes a new object onto the priority queue.
 * @param k is the new key (pass by value).
 */
template<typename T, typename Comp, typename KeyHasher>
typename CentralizedQueue<T, Comp, KeyHasher>::Node* CentralizedQueue<T, Comp, KeyHasher>::push(T k) {
    auto x = Heap::push(k);
    multimap_.insert({std::move(k), x});  // Bug fixed here. Always remember not to use something already moved.
    return x;
}

/*!
 * @brief This method finds the object with the given key on the multimap.
 * @param k is the key to look for.
 * @return an iterator to the desired key-node pair.
 */
template<typename T, typename Comp, typename KeyHasher>
typename CentralizedQueue<T, Comp, KeyHasher>::Iterator
CentralizedQueue<T, Comp, KeyHasher>::find(const T& k) {
    return multimap_.find(k);
}

/*!
 * @brief This function finds an node with the specified key in the multimap.
 * @param found is flag indicates success or failure of the search operation.
 * @param k is reference to the desired key object.
 * @return an iterator to the desired key-node pair.
 */
template<typename T, typename Comp, typename KeyHasher>
typename CentralizedQueue<T, Comp, KeyHasher>::Iterator
CentralizedQueue<T, Comp, KeyHasher>::find(bool& found, const T& k) {
    auto iter = multimap_.find(k);
    found = iter != multimap_.end();
    return iter;
}

/*!
 * @brief This method removes the object with the highest priority from the priority queue.
 */
template<typename T, typename Comp, typename KeyHasher>
void CentralizedQueue<T, Comp, KeyHasher>::pop() {
    if (Heap::empty()) { return; }
    auto x = Heap::FibHeapExtractMin();  // Remove the node from the heap.
    auto range = multimap_.equal_range(x->key);  // Locate the range with the same key.
    auto iter = std::find_if(range.first, range.second, [x](const std::pair<T, Node*>& ele) -> bool {
        return ele.second == x;
    });  // Find the first element with the given key.
    if (iter != range.second) {  // Key found!
        multimap_.erase(iter);  // Remove from the multimap.
    } else {
        std::cerr << "Key " << x->key << " not found in multimap." << std::endl;
    }
}

/*!
 * @brief This method finds a node with the given key.
 * @param k is the key to find.
 * @return pointer to the node.
 */
template<typename T, typename Comp, typename KeyHasher>
typename CentralizedQueue<T, Comp, KeyHasher>::Node* CentralizedQueue<T, Comp, KeyHasher>::findNode(const T& k) {
    Iterator iter = find(k);
    return iter->second;
}

/*!
 * @brief This functions searches the entire multimap_ for a key-node pair whose key satisfies the input predicate.
 * @param found is flag indicating success or failure of searching operation.
 * @param pred is a predicate which compares keys (e.g. a lambda expression).
 * @return an iterator to the desired key-node pair.
 */
template<typename T, typename Comp, typename KeyHasher>
typename CentralizedQueue<T, Comp, KeyHasher>::Iterator
CentralizedQueue<T, Comp, KeyHasher>::find_if(bool& found,
                                              const std::function<bool(const std::pair<T, Node*>&)>& pred) {
    auto iter = std::find_if(multimap_.begin(), multimap_.end(), pred);
    found = iter != multimap_.end();
    return iter;
}

/*!
 * @brief This function gets pointers to all objects in the multimap.
 * @return a vector of pointers to the keys.
 */
template<typename T, typename Comp, typename KeyHasher>
std::vector<const T*> CentralizedQueue<T, Comp, KeyHasher>::get_ptrs() const {
    std::vector<const T*> vec;
    for (auto iter = multimap_.begin(); iter != multimap_.end(); ++iter) {
        vec.template emplace_back(&iter->second->key);
    }  // Get a vector of pointers to all keys in the multimap.
    return vec;
}
