/*!
 * @brief This file contains the class definition of the queue based on std::deque.
 */
#ifndef CS225_SP22_C1_QUEUE_H_
#define CS225_SP22_C1_QUEUE_H_

#include <queue>
#include <utility>
#include <functional>

template<typename T, typename Container = std::deque<T>>
class Queue {
public:
    // Constructors and destructor.
    Queue();  // No-args constructor.
    Queue(const Queue& queue) = default;
    Queue& operator=(const Queue& queue) = default;
    Queue(Queue&& queue) noexcept = default;
    Queue& operator=(Queue&& queue) noexcept = default;
    explicit Queue(const Container& container);
    explicit Queue(Container&& container);
    virtual ~Queue() = default;

    // APIs.
    [[nodiscard]] bool empty() const;
    [[nodiscard]] unsigned size() const;
    [[nodiscard]] std::vector<const T*> get_ptrs() const;
    const T& front() const;
    const T& back() const;
    void push(const T& v);
    void push(T&& v);
    template<typename... Args>
    decltype(auto) emplace(Args&& ... args); // Variadic.
    void pop();
    void swap(Queue& queue) noexcept;
    void erase(typename Container::iterator& iter);
    typename Container::iterator find(bool& found, const T& k);
    typename Container::iterator find_if(bool& found, const std::function<bool(const T&)>& pred);

private:
    Container container_;
};

#endif //CS225_SP22_C1_QUEUE_H_
