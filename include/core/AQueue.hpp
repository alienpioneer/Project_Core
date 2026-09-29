/**
 * @file AQueue.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once
#include <array>
#include <utility>
#include <type_traits>

/**
 * @page AQueuePage AQueue Class
 *
 * @brief Fixed-capacity FIFO queue with static storage.
 *
 * AQueue is a lightweight queue container implemented as a ring buffer with
 * compile-time capacity. It performs no dynamic memory allocation and is
 * intended for deterministic systems such as embedded or real-time software.
 *
 * Elements are inserted at the tail using push() and accessed at the head
 * using get(). Removal of the front element is performed explicitly with
 * pop(). The container follows semantics similar to std::queue but with a
 * fixed capacity.
 *
 * @section characteristics Characteristics
 *
 * - FIFO (First-In First-Out) ordering
 * - Fixed capacity determined at compile time
 * - Static storage using std::array
 * - No heap allocation
 * - Constant time operations
 *
 * @section operations Operations
 *
 * | Operation | Description |
 * |----------|-------------|
 * | push()   | Inserts an element at the back of the queue |
 * | get()    | Returns a reference to the front element |
 * | pop()    | Removes the front element |
 * | empty()  | Returns true if the queue contains no elements |
 * | full()   | Returns true if the queue reached its capacity |
 *
 * Typical usage:
 *
 * @code
 * AQueue<int, 8> q;
 *
 * q.push(10);
 * q.push(20);
 *
 * int v = q.get();   // 10
 * q.pop();
 * @endcode
 *
 * @section thread_safety Thread Safety
 *
 * AQueue is not thread-safe. External synchronization must be provided when
 * the queue is accessed from multiple threads.
 *
 * @section complexity Complexity
 *
 * All operations are O(1).
 *
 * @section constraints Constraints
 *
 * - The queue capacity is fixed at compile time.
 * - Calling get() requires the queue to be non-empty.
 * - push() fails and returns false when the queue is full.
 * 
 * @section Thread safety:
 * - Not thread-safe by itself
 * - Must be protected by external synchronization when used across threads
 *
 * @section implementation Implementation Notes
 *
 * - Implemented as a ringbuffer
 * - Capacity is fixed at compile time
 * - push() reports overflow by returning false
 * - use get() to get the queue front()
 * - Intended for deterministic producer/consumer designs
 * 
 * @section References
 * - Core::AQueue
 */

namespace Core
{
    
template<class T, std::size_t N>
class AQueue 
{
public:
    AQueue()=default;
    
    // No copy and no move for the queue
    AQueue(const AQueue&) = delete;
    AQueue& operator=(const AQueue&) = delete;
    AQueue(AQueue&&) = delete;
    AQueue& operator=(AQueue&&) = delete;

    /**
     * @brief Push into the queue
     * 
     * @param v_in 
     * @return true if operation successful
     */
    bool push(const T& v_in) noexcept(std::is_nothrow_copy_assignable<T>::value)
    {
        if (count_ == N)
        {
            return false;
        }
        buffer_[tail_] = v_in;
        tail_ = (tail_ + 1U) % N;
        ++count_;
        return true;
    }

    /**
     * @brief Push rvalue into the queue
     * 
     * @param v_in 
     * @return true if operation successful 
     */
    bool push(T&& v_in) noexcept(std::is_nothrow_move_assignable<T>::value)
    {
        if (count_ == N)
        {
            return false;
        }
        buffer_[tail_] = std::move(v_in);
        tail_ = (tail_ + 1U) % N;
        ++count_;
        return true;
    }

    /**
     * @brief Pop the head from the queue
     * 
     * @return true if operation successful
     */
    bool pop() noexcept
    {
        if (count_ == 0U) 
        {
            return false;
        }
        head_ = (head_ + 1U) % N;
        --count_;
        return true;
    }

    /**
     * @brief Get the queue head
     * 
     * @return T& 
     */
    T& get() noexcept
    {
        // precondition: !empty()
        return buffer_[head_];
    }

    /**
     * @brief Get the queue head
     * 
     * @return const T& 
     */
    const T& get() const noexcept
    {
        // precondition: !empty()
        return buffer_[head_];
    }

    /**
     * @brief Check if the queue is empty
     * 
     * @return bool 
     */
    bool empty() const noexcept 
    { 
        return count_ == 0U; 
    }

    /**
     * @brief Check if the queue is full
     * 
     * @return bool 
     */
    bool full()  const noexcept
    { 
        return count_ == N;
    }

    /**
     * @brief Get the queue current size
     * 
     * @return std::size_t 
     */
    std::size_t size() const noexcept
    {
        return count_;
    }

private:
    std::array<T, N> buffer_{};
    std::size_t head_{0U};
    std::size_t tail_{0U};
    std::size_t count_{0U};
};

} // namespace Core