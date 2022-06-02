/*!
 * @brief This file contains the declaration of class <em>CentralizedQueue</em>.
 */
#ifndef CS225_SP22_C1_CENTRALIZEDQUEUE_H_
#define CS225_SP22_C1_CENTRALIZEDQUEUE_H_

#include "fibonacciHeap.h"
#include "fibonacciHeap.cpp"
#include <unordered_map>

/*!
 * @brief This class implements a priority queue with a <em>Fibonacci heap</em>.
 * search operations accelerated to <em>constant time</em> complexity with std::unordered_map.
 * @tparam T is type of keys.
 * @tparam Comp is type of comparators (functor with overloaded () operator).
 * @tparam Hasher is the hashing function of the keys.
 */
template<typename T, typename Comp = std::less<T>, typename Hasher = std::hash<T>>
class CentralizedQueue : public FibonacciHeap<T, Comp> {
public:
    // Setting alias.
    using Heap = FibonacciHeap<T, Comp>;
    using Node = typename Heap::FibonacciNode;
    using Iterator = typename std::unordered_map<T, Node*, Hasher>::iterator;

private:
    // Fields.
    std::unordered_multimap<T, Node*, Hasher> multimap_;  // Protected!

public:
    // Constructors and destructor.
    CentralizedQueue() = default;  // Uses default comparator from namespace `std`.
    explicit CentralizedQueue(Comp comp_);  // Pass in customized comparator.
    CentralizedQueue(const CentralizedQueue& centralized_queue) = default;
    CentralizedQueue& operator=(const CentralizedQueue& centralized_queue) = default;
    CentralizedQueue(CentralizedQueue&& centralized_queue) noexcept = default;
    CentralizedQueue& operator=(CentralizedQueue&& centralized_queue) noexcept = default;
    virtual ~CentralizedQueue() = default;

    // APIs.
    Node* push(T k) override;
    void pop() override;
    void decreaseKey(Node* x, T k);
    [[nodiscard]] Iterator find(const T& k);
    [[nodiscard]] Iterator find(bool& found, const T& k);  // Overloaded find with boolean flag.
    [[nodiscard]] Iterator find_if(bool& found, const std::function<bool(const std::pair<T, Node*>&)>& pred);
    [[nodiscard]] Node* findNode(const T& k);  // Debugging purposes only.
    [[nodiscard]] std::vector<const T*> get_ptrs() const;
};

#endif //CS225_SP22_C1_CENTRALIZEDQUEUE_H_
