/*!
 * @brief This file contains the implementation of the FIFO queue.
 */
#include "queue.h"

/*!
 * @brief No-args constructor.
 * @tparam T is type of the objects in the queue.
 * @tparam Container is type of the container.
 */
template<typename T, typename Container>
Queue<T, Container>::Queue() : container_() {}

/*!
 * @brief This constructor creates a new queue with the given container.
 * @param container is the input container.
 */
template<typename T, typename Container>
Queue<T, Container>::Queue(const Container& container) : container_(container) {}

/*!
 * @brief The move constructor.
 */
template<typename T, typename Container>
Queue<T, Container>::Queue(Container&& container) : container_(std::move(container)) {}

/*!
 * @brief This method checks if the queue is empty.
 * @return true if the queue is empty and false otherwise.
 */
template<typename T, typename Container>
bool Queue<T, Container>::empty() const {
    return container_.empty();
}

/*!
 * @brief This method returns the number of elements in the queue.
 * @return number of elements in the queue.
 */
template<typename T, typename Container>
unsigned Queue<T, Container>::size() const {
    return container_.size();
}

/*!
 * @brief This method returns the first element in the queue.
 * @return a constant reference to the element.
 */
template<typename T, typename Container>
const T& Queue<T, Container>::front() const {
    return container_.front();
}

/*!
 * @brief This method returns the last element in the queue.
 * @tparam T
 * @tparam Container
 * @return
 */
template<typename T, typename Container>
const T& Queue<T, Container>::back() const {
    return container_.back();
}

/*!
 * @brief This method pushes a given object onto the queue.
 * @param v is a reference to the object.
 */
template<typename T, typename Container>
void Queue<T, Container>::push(const T& v) {
    container_.push_back(v);
}

/*!
 * @brief This method pushes a given object onto the queue.
 * @param v is an rvalue reference to the object.
 */
template<typename T, typename Container>
void Queue<T, Container>::push(T&& v) {
    container_.push_back(std::move(v));
}

/*!
 * @brief This method inserts a new element into the container.
 * @param args is the new element.
 * @return an iterator pointing to the emplaced element.
 */
template<typename T, typename Container>
template<typename... Args>
decltype(auto) Queue<T, Container>::emplace(Args&& ... args) {
    return container_.emplace_back(std::forward<Args>(args)...);
}

/*!
 * @brief This method pops the head element off the queue.
 */
template<typename T, typename Container>
void Queue<T, Container>::pop() {
    container_.pop_front();
}

/*!
 * @brief This method swaps two queue.
 * @param queue is the other queue.
 */
template<typename T, typename Container>
void Queue<T, Container>::swap(Queue& queue) noexcept {
    using std::swap;
    swap(container_, queue.container_);
}

/*!
 * @brief This method searches for an element which satisfies the given predicate.
 * @param found is reference to the flag which indicates success or failure of the operation.
 * @param pred is the predicate (lambda expression or functor).
 * @return an iterator to the desired element.
 */
template<typename T, typename Container>
typename Container::iterator Queue<T, Container>::find_if(bool& found, const std::function<bool(const T&)>& pred) {
    auto iter = std::find_if(container_.begin(), container_.end(), pred);
    found = iter != container_.end();
    return iter;
}

/*!
 * @brief This method removes the element pointed-to by the iterator.
 * @param iter is iterator to the position to be erased.
 */
template<typename T, typename Container>
void Queue<T, Container>::erase(typename Container::iterator& iter) {
    container_.erase(iter);
}

/*!
 * @brief This method searches for an element with the input key.
 * @param found is reference to the flag which indicates success or failure of the operation.
 * @param k is reference to the desired key value.
 * @return an iterator to the desired element.
 */
template<typename T, typename Container>
typename Container::iterator Queue<T, Container>::find(bool& found, const T& k) {
    auto iter = std::find(container_.begin(), container_.end(), k);
    found = iter != container_.end();
    return iter;
}

/*!
 * @brief This method returns a vector of pointers to all elements in the queue.
 * @return a vector of pointers to all elements in the queue.
 */
template<typename T, typename Container>
std::vector<const T*> Queue<T, Container>::get_ptrs() const {
    std::vector<const T*> vector;
    for (auto iter = container_.begin(); iter != container_.end(); ++iter) {
        vector.template emplace_back(&*iter);
    }
    return vector;
}
