/*!
 * @brief This file contains the class definition of <em>FibonacciHeap</em>.
 */
#ifndef CS225_SP22_C1_FIBONACCIHEAP_H_
#define CS225_SP22_C1_FIBONACCIHEAP_H_

#include <iostream>
#include <utility>
#include <functional>
#include <cmath>
#include <algorithm>
#include <vector>

/*!
 * @brief This class declaration declares a CLRS-style Fibonacci heap data structure.
 * @tparam T is type of keys.
 * @tparam Comp is comparator (functor with overloaded () operator).
 */
template<typename T, typename Comp = std::less<T>>  // Transparent comparator.
class FibonacciHeap {
protected:
    // Fields.
    class FibonacciNode {
    public:
        FibonacciNode() = delete;  // No-args constructor explicitly removed.
        explicit FibonacciNode(T k);
        FibonacciNode(const FibonacciNode& node) = default;
        FibonacciNode& operator=(const FibonacciNode& node) = default;
        FibonacciNode(FibonacciNode&& node) noexcept = default;  // Move constructor.
        FibonacciNode& operator=(FibonacciNode&& node) noexcept = default;  // Overloaded move assignment operator.
        virtual ~FibonacciNode() = default;
        T key{NULL};
        bool mark{false};
        FibonacciNode* p{nullptr};  // Parent pointer.
        FibonacciNode* left{this};  // Initialized to point to the node itself.
        FibonacciNode* right{this};  // Initialized to point to the node itself.
        FibonacciNode* child{nullptr};  // Child list pointer.
        int degree{-1};
    };
    int n{0};
    FibonacciNode* min{nullptr};
    Comp comp_;  // Function object for comparison.

public:
    // Constructors and destructors.
    FibonacciHeap();
    explicit FibonacciHeap(Comp comp);
    FibonacciHeap(const FibonacciHeap& fibonacci_heap) = default;
    FibonacciHeap& operator=(const FibonacciHeap& fibonacci_heap) = default;
    FibonacciHeap(FibonacciHeap&& fibonacci_heap) noexcept = default;  // Move constructor.
    FibonacciHeap& operator=(FibonacciHeap&& fibonacci_heap) noexcept = default;
    virtual ~FibonacciHeap();

    // APIs.
    virtual FibonacciNode* push(T k);
    virtual void pop();
    [[nodiscard]] virtual bool empty() const;
    [[nodiscard]] virtual unsigned size() const;
    [[nodiscard]] virtual T& top() const;
    virtual FibonacciNode* topNode() const;

    // Static functions.
    static FibonacciHeap* FibHeapUnion(FibonacciHeap* H1, FibonacciHeap* H2);

protected:
    // Inline helper functions.
    static inline void concatLists(FibonacciNode*& list1, FibonacciNode*& list2);
    static inline void removeListNode(FibonacciNode* node);
    static inline void insertListNode(FibonacciNode*& node, FibonacciNode*& list);
    static inline void clearParentListNode(FibonacciNode* node);

    // Private methods.
    void FibHeapInsert(FibonacciNode* x);
    FibonacciNode* FibHeapExtractMin();
    void FibHeapLink(FibonacciNode* y, FibonacciNode* x);
    void FibHeapDecreaseKey(FibonacciNode* x, T k);
    void Consolidate();
    void Cut(FibonacciNode* x, FibonacciNode* y);
    void CascadingCut(FibonacciNode* y);
    void DeallocateTree(FibonacciNode* x);
};

#endif //CS225_SP22_C1_FIBONACCIHEAP_H_
