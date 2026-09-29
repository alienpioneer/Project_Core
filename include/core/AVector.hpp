/**
 * @file AVector.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once
#include <array>
#include <type_traits>
#include <utility>

/**
 * @page AVectorPage AVector
 * 
 * @brief Lightweight fixed-capacity container
 *
 * @section avector_overview Overview
 *
 * `Core::AVector<T, N>` is a lightweight fixed-capacity container similar to
 * `std::vector`, but without dynamic memory allocation. Storage is backed by
 * `std::array<T, N>` and the number of active elements is tracked separately.
 *
 * The container is designed for deterministic systems where heap allocations
 * are undesirable or forbidden (embedded systems, real-time components, or
 * performance-critical code paths).
 *
 * Characteristics:
 * - No dynamic allocation
 * - Compile-time capacity
 * - Contiguous memory layout
 * - STL-like API for iteration and element access
 *
 *
 * @section avector_template Template Parameters
 *
 * - `T` : Type of stored elements.
 * - `N` : Maximum number of elements the container can hold.
 *
 *
 * @section avector_storage Storage Model
 *
 * Internally the container stores:
 *
 * - `std::array<T, N>` : physical storage
 * - `size_` : number of currently used elements
 *
 * Elements are always stored contiguously and iterators are raw pointers.
 *
 *
 * @section avector_capacity Capacity
 *
 * The capacity is fixed at compile time.
 *
 * | Function | Description |
 * |--------|-------------|
 * | `size()` | Current number of stored elements |
 * | `capacity()` | Maximum number of elements (`N`) |
 * | `empty()` | Returns true if container contains no elements |
 *
 *
 * @section avector_modification Modification Operations
 *
 * The container supports common vector-like modifications:
 *
 * - `push_back()` : append element if capacity allows
 * - `pop_back()` : remove last element
 * - `resize()` : grow or shrink logical size
 * - `erase()` : remove elements by index, iterator, or iterator range
 * - `clear()` : reset container size to zero
 *
 * Insertions beyond capacity are ignored.
 *
 *
 * @section avector_iteration Iteration
 *
 * Iterators are raw pointers and follow standard C++ iterator semantics:
 *
 * - `begin()`, `end()`
 * - `cbegin()`, `cend()`
 *
 * Example:
 *
 * @code
 * Core::AVector<int, 16> v;
 *
 * v.push_back(1);
 * v.push_back(2);
 *
 * for (auto& x : v)
 * {
 *     std::cout << x << std::endl;
 * }
 * @endcode
 *
 *
 * @section avector_complexity Complexity
 *
 * | Operation | Complexity |
 * |----------|-----------|
 * | `push_back` | O(1) |
 * | `pop_back` | O(1) |
 * | `erase(index)` | O(n) |
 * | `erase(range)` | O(n) |
 * | `resize` | O(n) |
 *
 *
 * @section avector_usecases Typical Use Cases
 *
 * - Embedded systems without heap allocation
 * - Real-time software requiring deterministic memory usage
 * - Performance-critical code paths
 * - Temporary containers with known maximum size
 *
 *
 * @section References
 * - Core::AVector
 *
 */

namespace Core
{

template<typename T, std::size_t N>
class AVector
{
public:
    AVector() = default;

    /// If count is equal to the current size, does nothing.
    /// If the current size is greater than count, the container is reduced to its first count elements.
    /// If the current size is less than count, then additional copies of T() are appended.
    void resize( std::size_t count ) noexcept
    {
        if(count == size_)
        {
            return;
        }

        if (count < size_)
        {
            size_= count;
        }
        else // count > size_
        {
            T* start_it = begin()+size_;
            T* end_it = start_it+(count-size_);

            std::size_t cap = data_.max_size();

            if (count >= cap)
            {
                end_it = start_it+(cap-size_);
            }
            
            while (start_it != end_it) 
            {
                *start_it=T();
                start_it++;
                size_++;
            }
        }
    }

    /**
     * @brief Add an element to the end by copy
     * 
     * @param item The element to copy into the container
     * 
     * @pre size() < N
     * @post If size() was < N before the call, size() increases by 1
     * 
     * @note If the container is full (size() == N), this operation has no effect
     * @note This function is noexcept if T's copy assignment is noexcept
     */
    void push_back(const T& item) noexcept(std::is_nothrow_copy_assignable_v<T>)
    {
        if (size_ < N)
        {
            data_[size_++] = item;
        } 
    }

    /**
     * @brief Add an rvalue element to the end by copy
     * 
     * @param item The element to copy into the container
     * 
     * @pre size() < N
     * @post If size() was < N before the call, size() increases by 1
     * 
     * @note If the container is full (size() == N), this operation has no effect
     * @note This function is noexcept if T's copy assignment is noexcept
     */
    void push_back(T&& item) noexcept(std::is_nothrow_move_assignable_v<T>)
    {
        if (size_ < N)
        {
            data_[size_++] = std::move(item);
        }
    }

    /**
     * @brief Erase element at given index
     * 
     * @param index The position of the element to remove
     * 
     * @pre index < size()
     * @post If index was valid, size() decreases by 1
     * 
     * Elements after the erased position are shifted left.
     * If index is out of bounds, this operation has no effect.
     */
    void erase(std::size_t index)
    {
        if (index < size_)
        {
            auto it = begin()+index;

            while (it != end()-1) 
            {
                *it=*(it+1);
                it++;
            }
            --size_;
        }
    }

    /**
     * @brief Erase element at iterator position
     * 
     * @param iterator_pos Iterator pointing to the element to erase
     * @return Iterator to the element following the erased element, or end() if erasing the last element
     * 
     * @pre iterator_pos must be a valid iterator in [begin(), end())
     * @post size() decreases by 1
     * 
     * Elements after the erased position are shifted left.
     */
    T* erase(T* iterator_pos)
    {
        static_assert(std::is_pointer_v<T*>, "erase(T* iterator_pos) T* must be a pointer type");

        if (iterator_pos >= begin() && iterator_pos < end())
        {
            auto it = iterator_pos;

            while (it != end()-1) 
            {
                *it=*(it+1);
                it++;
            }
            --size_;

            return iterator_pos;
        }

        return end();
    }

    /**
     * @brief Erase element at iterator position
     * 
     * @param iterator_pos Iterator pointing to the element to erase
     * @return Const Iterator to the element following the erased element, or end() if erasing the last element
     * 
     * @pre iterator_pos must be a valid iterator in [begin(), end())
     * @post size() decreases by 1
     * 
     * Elements after the erased position are shifted left.
     */
    T* erase(const T* iterator_pos)
    {
        static_assert(std::is_pointer_v<const T*>, "erase(cons T* iterator_pos) T* must be a pointer type");
        return erase(const_cast<T*>(iterator_pos));
    }

    /**
     * @brief  Removes the elements in the range [begin_pos, end_pos) - end_pos not included !
     * 
     * @param begin_pos Iterator to the first element to erase
     * @param end_pos Iterator to one past the last element to erase
     * @return Iterator to the element following the last erased element. 
     * This allows you to chain erase operations or continue iterating -- standard C++ iterator erase semantics
     * 
     * @pre begin_pos <= end_pos
     * @pre begin_pos and end_pos must be valid iterators in [begin(), end()]
     * @post size() decreases by (end_pos - begin_pos)
     * 
     * Erases all elements in the range [begin_pos, end_pos).
     * If begin_pos == end_pos, no elements are erased.
     */
    T* erase(T* begin_pos, T* end_pos)
    {
        static_assert(std::is_pointer_v<T*>, "erase(T* begin_pos, T* end_pos) T* must be a pointer type");

        if(begin_pos == end_pos && begin_pos >= begin())
        {
            return begin_pos;
        }
        else if (begin_pos < end_pos && begin_pos >= begin() && end_pos <= end())
        {
            T* start_it = begin_pos;
            T* end_it = end_pos;

            while (end_it != end()) 
            {
                *start_it=*end_it;
                start_it++;
                end_it++;
            }
            size_ -= (end_pos-begin_pos);
            return begin_pos;
        }

        return end();
    }

    /**
     * @brief  Removes the elements in the range [begin_pos, end_pos) - end_pos not included !
     * 
     * @param begin_pos Const Iterator to the first element to erase
     * @param end_pos Const Iterator to one past the last element to erase
     * @return Iterator to the element following the last erased element. 
     * This allows you to chain erase operations or continue iterating -- standard C++ iterator erase semantics
     * 
     * @pre begin_pos <= end_pos
     * @pre begin_pos and end_pos must be valid iterators in [begin(), end()]
     * @post size() decreases by (end_pos - begin_pos)
     * 
     * Erases all elements in the range [begin_pos, end_pos).
     * If begin_pos == end_pos, no elements are erased.
     */
    T* erase(const T* begin_pos, const T* end_pos)
    {
        static_assert(std::is_pointer_v<const T*>, "erase(T* begin_pos, T* end_pos) T* must be a pointer type");
        return erase(const_cast<T*>(begin_pos), const_cast<T*>(end_pos));
    }

    /**
     * @brief Removes the last element from the container.
     *
     * Decreases the logical size of the container by one.  
     * The underlying storage is not modified, only the element count changes.
     *
     * @pre The container must not be empty.
     *
     * @complexity O(1)
     */
    void pop_back() noexcept
    {
        if(size_ > 0)
        {
            --size_;
        }
    }
    
    /**
     * @brief Returns a reference to the last element.
     *
     * Provides direct access to the last element currently stored in the container.
     *
     * @return Reference to the last element.
     *
     * @pre The container must not be empty.
     *
     * @complexity O(1)
     */
    T& back() noexcept
    {
         return data_[size_-1]; 
    }
    
    /**
     * @brief Returns a const reference to the last element.
     *
     * Provides read-only access to the last element currently stored in the container.
     *
     * @return Const reference to the last element.
     *
     * @pre The container must not be empty.
     *
     * @complexity O(1)
     */
    const T& back() const noexcept
    {
         return data_[size_-1]; 
    }

    /**
     * @brief Access element at the specified index.
     *
     * Provides direct unchecked access to an element stored in the container.
     *
     * @param index Position of the element.
     * @return Reference to the element at the given index.
     *
     * @pre `index < size()`
     *
     * @complexity O(1)
     */
    T& operator[](std::size_t index)
    {
         return data_[index]; 
    }

    /**
     * @brief Access element at the specified index (const overload).
     *
     * Provides read-only unchecked access to an element stored in the container.
     *
     * @param index Position of the element.
     * @return Const reference to the element at the given index.
     *
     * @pre `index < size()`
     *
     * @complexity O(1)
     */
    const T& operator[](std::size_t index) const 
    {
        return data_[index]; 
    }
    
    /**
     * @brief Get iterator to beginning
     * @return Pointer to the first element
     */
    T* begin() 
    { 
        return data_.data(); 
    }

    /**
     * @brief Get iterator to end
     * @return Pointer to one past the last element
     */
    T* end() 
    { 
        return data_.data() + size_; 
    }

    /**
     * @brief Get iterator to beginning
     * @return Pointer to the first element
     */
    const T* cbegin() const 
    { 
        return data_.data(); 
    }

    /**
     * @brief Get iterator to end
     * @return Pointer to one past the last element
     */
    const T* cend() const 
    { 
        return data_.data() + size_; 
    }
    
    /**
     * @brief Get current number of elements
     * @return The number of elements currently in the container
     */
    std::size_t size() const 
    { 
        return size_; 
    }

    /**
     * @brief Check if container is empty
     * @return true if size() == 0, false otherwise
     */
    bool empty() const 
    {
        return size_ == 0; 
    }

    /**
     * @brief Remove(invalidate) all elements
     * @post size() == 0
     * @post empty() == true
     */
    void clear() 
    {
        size_ = 0; 
    }

    std::size_t capacity() const
    {
        return data_.max_size();
    }

    T* data() noexcept
    {
        return data_.data();
    }

    const T* data() const noexcept
    {
        return data_.data();
    }

private:
    std::array<T, N> data_;
    std::size_t size_{0};
};

}// namespace Core
