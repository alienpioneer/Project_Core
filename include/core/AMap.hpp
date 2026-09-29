/**
 * @file AMap.hpp
 * @author Alexandru ALEXANDRESCU
 * All rights reserved.
 * 
 */

#pragma once

#include <array>
#include <cstddef>
#include <cassert>
#include <utility>

/**
 * @page AMapPage AMap - Fixed-capacity associative container (no dynamic allocation)
 *
 * @details
 * AMap is a lightweight key-value container designed for environments where:
 * - dynamic allocation is restricted or forbidden
 * - deterministic memory usage is required
 * - small datasets are expected
 *
 * Internally, it uses a contiguous fixed-size array with linear lookup.
 *
 * @warning
 * This container tries to be compatible with std::unordered_map, but does NOT behave EXACTLY like std::unordered_map or std::unordered_map since :
 * - Capacity is fixed at compile time
 * - Insertions fail when full
 * - Lookup is O(N)
 * - No iterator safety guarantees (invalidated on erase)
 *
 * @section allocation Allocation Considerations
 *
 * AMap stores its elements inline:
 * @code
 * std::array<std::pair<K,V>, N>
 * @endcode
 *
 * This means:
 * - Memory is allocated on the stack when declared locally
 * - Large N values can easily overflow the stack
 *
 * @subsection nested_maps Nested Maps WARNING
 *
 * @warning 
 * Using nested AMap multiplies memory:
 * @code
 * AMap<std::string, AMap<std::string, std::string, 1000>, 1000>
 * @endcode
 *
 * This creates:
 * - 1000 sections
 * - each containing 1000 entries
 * → ~1,000,000 elements
 *
 * This will almost certainly cause stack overflow.
 *
 * @section rec Recommendation
 * - Keep capacities small
 * - Use different limits for outer/inner maps
 * - Use indirection (e.g. std::unique_ptr) for nested structures
 *
 * @section usage Usage Model
 *
 * Explicit, failure-aware API:
 * - insert() → insert if space available
 * - find() → pointer-based lookup
 * - contains() → existence check
 *
 * No exceptions, no hidden allocations.
 */

namespace Core
{

/**
 * @brief Fixed-capacity associative container
 * 
 * @tparam K Key type
 * @tparam V Value type
 * @tparam N Maximum number of elements
 */
template<typename K, typename V, std::size_t N>
class AMap
{
public:
    static_assert(N > 0, "FixedMap capacity must be > 0");
    // static_assert(std::is_trivially_constructible_v<value_type>, "NOT TRIVIAL");

    /**
     * @brief Stored key-value pair type
     */
    using value_type     = std::pair<K, V>;

    /**
     * @brief Iterator type
     */
    using iterator       = typename std::array<value_type, N>::iterator;

    /**
     * @brief Const iterator type
     */
    using const_iterator = typename std::array<value_type, N>::const_iterator;

    static constexpr std::size_t npos = static_cast<std::size_t>(-1);

    // ================= lookup =================

    /**
     * @brief Check if key exists
     *
     * @param key Key to search
     * @return true if found, false otherwise
     */
    template <typename KeyLike, typename = decltype(std::declval<K>() == std::declval<KeyLike>())>
    bool contains(const KeyLike& key) const noexcept
    {
        return findIndex(key) != npos;
    }

    /**
     * @brief Find element by key
     *
     * @param key Key to search
     * @return Iterator to element or end() if not found
     */
    template <typename KeyLike, typename = decltype(std::declval<K>() == std::declval<KeyLike>())>
    iterator find(const KeyLike& key) noexcept
    {
        size_t idx = findIndex(key);
        return (idx == npos) ? end() : (begin() + idx);
    }

    /**
     * @brief Find element by key (const)
     *
     * @param key Key to search
     * @return Const iterator to element or end() if not found
     */
    template <typename KeyLike, typename = decltype(std::declval<K>() == std::declval<KeyLike>())>
    const_iterator find(const KeyLike& key) const noexcept
    {
        size_t idx = findIndex(key);
        return (idx == npos) ? end() : (begin() + idx);
    }
    
    // ================= insert / emplace =================

    /**
     * @brief Insert key-value pair
     *
     * @param value Pair to insert
     * @return {iterator, true} if inserted, {iterator, false} if key exists or full
     *
     * @note Fails if capacity reached
     */
    std::pair<iterator, bool> insert(const value_type& value)
    {
        size_t idx = findIndex(value.first);

        if (idx != npos)
            return { m_data.begin() + idx, false };

        if (m_size >= N)
            return { end(), false };

        m_data[m_size] = value;
        return { m_data.begin() + m_size++, true };
    }

    /**
     * @brief In-place construction of element
     *
     * @tparam Args Constructor arguments for value_type
     * @param args Arguments forwarded to value_type constructor
     * @return {iterator, true} if inserted, {iterator, false} otherwise
     *
     * @note No allocation; fails if full or duplicate key
     */
    template<typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args)
    {
        if (m_size >= N)
        {
            return { end(), false };
        }
            
        value_type tmp(std::forward<Args>(args)...);

        size_t idx = findIndex(tmp.first);

        if (idx != npos)
        {
            return { m_data.begin() + idx, false };
        }
            
        m_data[m_size] = std::move(tmp);

        auto res = std::make_pair<iterator, bool>(m_data.begin() + m_size, true);

        ++m_size;

        return res;
    }

    // ================= erase =================

    /**
     * @brief Erase element by key
     *
     * @param key Key to remove
     * @return true if removed, false if not found
     *
     * @note Does not preserve order
     */
    bool erase(const K& key) noexcept
    {
        std::size_t idx = findIndex(key);
        if (idx == npos)
            return false;

        // swap with last (O(1), order not preserved)
        m_data[idx] = std::move(m_data[m_size - 1]);
        --m_size;
        return true;
    }

    /**
     * @brief Erase element by const iterator
     */
    iterator erase(const_iterator pos) noexcept
    {
        if (pos == cend())
            return end();

        size_t idx = static_cast<size_t>(pos - cbegin());

        if (idx >= m_size)
            return end();

        m_data[idx] = std::move(m_data[m_size - 1]);
        --m_size;

        if (idx >= m_size)
            return end();

        return begin() + idx;
    }

    /**
     * @brief Erase element by iterator
     *
     * @param pos Iterator to element
     * @return Iterator to next element or end()
     *
     * @note Iterator must be valid and dereferenceable
     * @note Does not preserve order
     */
    iterator erase(iterator pos) noexcept
    {
        if (pos == end())
            return end();

        size_t idx = static_cast<size_t>(pos - begin());

        if (idx >= m_size) // defensive (covers corrupted iterator too)
            return end();

        m_data[idx] = std::move(m_data[m_size - 1]);
        --m_size;

        if (idx >= m_size)
            return end();

        return begin() + idx;
    }

    // ================= operators ==============

    /**
     * @brief Access or insert element
     *
     * @param key Key to access
     * @return Reference to value
     *
     * @note Inserts default value if key not found
     * @warning If container is full, returns fallback reference (implementation-defined)
     */
    V& operator[](const K& key) noexcept
    {
        size_t idx = findIndex(key);

        if (idx != npos)
        {
            return m_data[idx].second;
        }
            
        if (m_size < N)
        {
            m_data[m_size] = value_type{key, V{}};
            return m_data[m_size++].second;
        }

        assert(false && "AMap::operator[] — map is full, insertion impossible");
        // Fallback to safety on release !
        static V dummy{};
        return dummy;
    }

    /**
     * @brief Access element (const)
     *
     * @param key Key to access
     * @return Reference to value or dummy if not found
     */
    const V& operator[](const K& key) const noexcept
    {
        size_t idx = findIndex(key);

        if (idx != npos)
        {
            return m_data[idx].second;
        }

        assert(m_size < N && "AMap::operator[] — map is full, insertion impossible");

        // Fallback to safety on release !
        static const V dummy{};
        return dummy;
    }

    /**
     * @brief STL compatibility method
     * 
     * @param key Key to access
     * @return V& Reference to value or dummy if not found
     */
    V& at(const K& key) noexcept
    {
        size_t idx = findIndex(key);

        if (idx != npos)
        {
            return m_data[idx].second;
        }
            
        if (m_size < N)
        {
            m_data[m_size] = value_type{key, V{}};
            return m_data[m_size++].second;
        }

        assert(false && "AMap::operator[] — map is full, insertion impossible");
        // Fallback to safety on release !
        static V dummy{};
        return dummy;
    }

    /**
     * @brief STL compatibility method
     * 
     * @param key Key to access
     * @return V& Reference to value or dummy if not found
     */
    const V& at(const K& key) const noexcept
    {
        size_t idx = findIndex(key);

        if (idx != npos)
        {
            return m_data[idx].second;
        }

        assert(m_size < N && "AMap::operator[] — map is full, insertion impossible");

        // Fallback to safety on release !
        static const V dummy{};
        return dummy;
    }

    // ================= iterators =================

    /**
     * @brief Iterator to first element
     */
    iterator begin() noexcept { return m_data.begin(); }

    /**
     * @brief Iterator past last element
     */
    iterator end() noexcept { return m_data.begin() + m_size; }

    /**
     * @brief Const iterator to first element
     */
    const_iterator begin() const noexcept { return m_data.begin(); }

    /**
     * @brief Const iterator past last element
     */
    const_iterator end() const noexcept { return m_data.begin() + m_size; }

    /**
     * @brief Const begin iterator
     */
    const_iterator cbegin() const noexcept { return m_data.cbegin(); }

    /**
     * @brief Const end iterator
     */
    const_iterator cend() const noexcept { return m_data.cbegin() + m_size; }

    // ================= capacity =================

    /**
     * @brief Current number of elements
     */
    std::size_t size() const noexcept { return m_size; }

    /**
     * @brief Maximum capacity
     */
    static constexpr std::size_t capacity() noexcept { return N; }

    /**
     * @brief Check if container is empty
     */
    bool empty() const noexcept { return m_size == 0; }

    /**
     * @brief Check if container is full
     */
    bool full() const noexcept { return m_size == N; }

private:
    /**
     * @brief Heterogeneous lookup (no key construction)
     *
     * @tparam KeyLike Compatible key type
     * @param key Key-like object
     * @return Iterator to element or end()
     */
    template<class KeyLike, typename = decltype(std::declval<K>() == std::declval<KeyLike>())>
    std::size_t findIndex(const KeyLike& key) const noexcept
    {
        for (std::size_t i = 0; i < m_size; ++i)
        {
            if (m_data[i].first == key)
            {
                return i;
            }   
        }
        return npos;
    }

    std::array<value_type, N> m_data{};
    std::size_t m_size{};
};

}// namespace Core
