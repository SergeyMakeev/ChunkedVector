#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#define CHUNKED_VEC_INLINE inline

// TODO (detailed desc): you could override memory allocator by defining CHUNKED_VEC_ALLOC/CHUNKED_VEC_FREE macroses
#if !defined(CHUNKED_VEC_ALLOC) || !defined(CHUNKED_VEC_FREE)

#if defined(_WIN32)
// Windows
#include <xmmintrin.h>
#define CHUNKED_VEC_ALLOC(sizeInBytes, alignment) _mm_malloc(sizeInBytes, alignment)
#define CHUNKED_VEC_FREE(ptr) _mm_free(ptr)
#else
// Posix
#include <stdlib.h>
// Helper to round up size to be a multiple of alignment (required by aligned_alloc)
inline size_t round_up_to_alignment(size_t size, size_t alignment) noexcept {
    return ((size + alignment - 1) / alignment) * alignment;
}
#define CHUNKED_VEC_ALLOC(sizeInBytes, alignment) aligned_alloc(alignment, round_up_to_alignment(sizeInBytes, alignment))
#define CHUNKED_VEC_FREE(ptr) free(ptr)
#endif

#endif

// extern void _onAssertionFailed(const char* expression, const char* srcFile, unsigned int srcLine);
//#define CHUNKED_VEC_ASSERT(expression) (void)((!!(expression)) || (_onAssertionFailed(#expression, __FILE__, (unsigned int)(__LINE__)), 0))

// TODO (detailed desc): you could override asserts by defining CHUNKED_VEC_ASSERT macro
#if !defined(CHUNKED_VEC_ASSERT)
#include <assert.h>
#define CHUNKED_VEC_ASSERT(expression) assert(expression)
#endif

// Iterator debugging support similar to Microsoft STL
// Users can override CHUNKED_VEC_ITERATOR_DEBUG_LEVEL to control iterator debugging
#if !defined(CHUNKED_VEC_ITERATOR_DEBUG_LEVEL)
    #if defined(_ITERATOR_DEBUG_LEVEL)
        #define CHUNKED_VEC_ITERATOR_DEBUG_LEVEL _ITERATOR_DEBUG_LEVEL
    #else
        #if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
            #define CHUNKED_VEC_ITERATOR_DEBUG_LEVEL 2
        #else
            #define CHUNKED_VEC_ITERATOR_DEBUG_LEVEL 0
        #endif
    #endif
#endif

#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
    #define CHUNKED_VEC_VERIFY_ITERATOR(iter) (iter)._verify_valid(__FILE__, __LINE__)
    #define CHUNKED_VEC_VERIFY_ITERATOR_RANGE(first, last) do { \
        CHUNKED_VEC_VERIFY_ITERATOR(first); \
        CHUNKED_VEC_VERIFY_ITERATOR(last); \
        CHUNKED_VEC_ASSERT((first).m_container == (last).m_container && "Iterators from different containers"); \
        CHUNKED_VEC_ASSERT((first).m_index <= (last).m_index && "Invalid iterator range"); \
    } while(0)
#else
    #define CHUNKED_VEC_VERIFY_ITERATOR(iter) ((void)0)
    #define CHUNKED_VEC_VERIFY_ITERATOR_RANGE(first, last) ((void)0)
#endif


namespace dod
{
template <typename U, typename... Args> static U* construct(void* ptr, Args&&... args)
{
    U* p = new (ptr) U(std::forward<Args>(args)...);
    return std::launder(p);
}

template <typename U> static void destruct(U* ptr) { ptr->~U(); }

template <typename U, typename... Args> static U* reconstruct(U* ptr, Args&&... args)
{
    ptr->~U();
    U* p = new (ptr) U(std::forward<Args>(args)...);
    return std::launder(p);
}

// Helper to ensure alignment is at least alignof(void*) for compatibility with macOS.
// macOS does not support alignments smaller than alignof(void*) in aligned_alloc,
// so we need to ensure the alignment parameter is always at least that size.
// For types with larger natural alignment, we still use their natural alignment.
template <typename T>
inline constexpr size_t safe_alignment_of = alignof(T) < alignof(void*) ? alignof(void*) : alignof(T);

}

namespace dod
{

/// @brief A chunked vector implementation that stores elements in fixed-size pages.
/// @details This container provides O(1) random access while avoiding the memory
/// fragmentation issues of std::vector when dealing with large amounts of data.
/// Elements are stored in pages of fixed size, allowing for efficient memory usage
/// and avoiding large contiguous memory allocations.
///
/// @tparam T The type of elements stored in the vector
/// @tparam PAGE_SIZE The number of elements per page (default: 1024)
///
/// Key features:
/// - O(1) random access via operator[] and at()
/// - Efficient memory usage with page-based allocation
/// - Iterator debugging support (similar to MSVC STL)
/// - Optimized operations for trivial types
/// - Custom allocator support via macros
template <typename T, size_t PAGE_SIZE = 1024> class chunked_vector
{
  public:
    using value_type = T;
    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    static_assert(PAGE_SIZE > 0, "PAGE_SIZE must be greater than 0");

    template <typename ValueType> class basic_iterator;
    using iterator = basic_iterator<T>;
    using const_iterator = basic_iterator<const T>;

    /// @brief Returns the page size used by this container
    static constexpr size_t page_size() { return PAGE_SIZE; }

    chunked_vector() noexcept
        : m_pages(nullptr)
        , m_page_count(0)
        , m_page_capacity(0)
        , m_size(0)
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        , m_iterator_list(nullptr)
#endif
    {
    }

    explicit chunked_vector(size_type count)
        : m_pages(nullptr)
        , m_page_count(0)
        , m_page_capacity(0)
        , m_size(0)
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        , m_iterator_list(nullptr)
#endif
    {
        resize(count);
    }

    chunked_vector(size_type count, const T& value)
        : m_pages(nullptr)
        , m_page_count(0)
        , m_page_capacity(0)
        , m_size(0)
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        , m_iterator_list(nullptr)
#endif
    {
        resize(count, value);
    }

    chunked_vector(std::initializer_list<T> init)
        : m_pages(nullptr)
        , m_page_count(0)
        , m_page_capacity(0)
        , m_size(0)
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        , m_iterator_list(nullptr)
#endif
    {
        reserve(init.size());
        for (const auto& item : init)
        {
            push_back(item);
        }
    }

    chunked_vector(const chunked_vector& other)
        : m_pages(nullptr)
        , m_page_count(0)
        , m_page_capacity(0)
        , m_size(0)
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        , m_iterator_list(nullptr)
#endif
    {
        *this = other;
    }

    chunked_vector(chunked_vector&& other) noexcept
        : m_pages(other.m_pages)
        , m_page_count(other.m_page_count)
        , m_page_capacity(other.m_page_capacity)
        , m_size(other.m_size)
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        , m_iterator_list(nullptr)
#endif
    {
        other.m_pages = nullptr;
        other.m_page_count = 0;
        other.m_page_capacity = 0;
        other.m_size = 0;
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        // Invalidate all iterators from both containers during move
        other._invalidate_all_iterators();
#endif
    }

    ~chunked_vector()
    {
        clear();
        deallocate_page_array();
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        _invalidate_all_iterators();
#endif
    }

    chunked_vector& operator=(const chunked_vector& other)
    {
        if (this != &other)
        {
            clear();
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            _invalidate_all_iterators();
#endif
            reserve(other.m_size);
            
            if constexpr (std::is_trivially_copyable_v<T>)
            {
                // For trivial types, use optimized bulk copy
                bulk_copy_from(other);
            }
            else
            {
                // For non-trivial types, use page-by-page copy to avoid redundant index calculations
                size_type remaining_elements = other.m_size;
                for (size_type page_idx = 0; page_idx < other.m_page_count && remaining_elements > 0; ++page_idx)
                {
                    size_type elements_in_this_page = std::min(remaining_elements, PAGE_SIZE);
                    const T* src_page = other.m_pages[page_idx];
                    
                    for (size_type elem_idx = 0; elem_idx < elements_in_this_page; ++elem_idx)
                    {
                        push_back(src_page[elem_idx]);
                    }
                    
                    remaining_elements -= elements_in_this_page;
                }
            }
        }
        return *this;
    }

    chunked_vector& operator=(chunked_vector&& other) noexcept
    {
        if (this != &other)
        {
            clear();
            deallocate_page_array();
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            _invalidate_all_iterators();
#endif

            m_pages = other.m_pages;
            m_page_count = other.m_page_count;
            m_page_capacity = other.m_page_capacity;
            m_size = other.m_size;

            other.m_pages = nullptr;
            other.m_page_count = 0;
            other.m_page_capacity = 0;
            other.m_size = 0;
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            // Invalidate all iterators from the source container during move
            other._invalidate_all_iterators();
#endif
        }
        return *this;
    }

    chunked_vector& operator=(std::initializer_list<T> init)
    {
        clear();
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        _invalidate_all_iterators();
#endif
        reserve(init.size());
        for (const auto& item : init)
        {
            push_back(item);
        }
        return *this;
    }

    CHUNKED_VEC_INLINE reference operator[](size_type pos)
    {
        CHUNKED_VEC_ASSERT(pos < m_size && "Index out of range");
        auto [page_idx, elem_idx] = get_page_and_element_indices(pos);
        return m_pages[page_idx][elem_idx];
    }

    CHUNKED_VEC_INLINE const_reference operator[](size_type pos) const
    {
        CHUNKED_VEC_ASSERT(pos < m_size && "Index out of range");
        auto [page_idx, elem_idx] = get_page_and_element_indices(pos);
        return m_pages[page_idx][elem_idx];
    }

    CHUNKED_VEC_INLINE reference at(size_type pos)
    {
        if (pos >= m_size)
        {
            throw std::out_of_range("chunked_vector::at: index out of range");
        }
        return (*this)[pos];
    }

    CHUNKED_VEC_INLINE const_reference at(size_type pos) const
    {
        if (pos >= m_size)
        {
            throw std::out_of_range("chunked_vector::at: index out of range");
        }
        return (*this)[pos];
    }

    CHUNKED_VEC_INLINE reference front()
    {
        CHUNKED_VEC_ASSERT(m_size > 0 && "Cannot access front of empty chunked_vector");
        return m_pages[0][0];
    }

    CHUNKED_VEC_INLINE const_reference front() const
    {
        CHUNKED_VEC_ASSERT(m_size > 0 && "Cannot access front of empty chunked_vector");
        return m_pages[0][0];
    }

    CHUNKED_VEC_INLINE reference back()
    {
        CHUNKED_VEC_ASSERT(m_size > 0 && "Cannot access back of empty chunked_vector");
        auto [last_page, last_elem] = get_page_and_element_indices(m_size - 1);
        return m_pages[last_page][last_elem];
    }

    CHUNKED_VEC_INLINE const_reference back() const
    {
        CHUNKED_VEC_ASSERT(m_size > 0 && "Cannot access back of empty chunked_vector");
        auto [last_page, last_elem] = get_page_and_element_indices(m_size - 1);
        return m_pages[last_page][last_elem];
    }

    CHUNKED_VEC_INLINE iterator begin() noexcept { return iterator(this, 0); }
    CHUNKED_VEC_INLINE const_iterator begin() const noexcept { return const_iterator(this, 0); }
    CHUNKED_VEC_INLINE const_iterator cbegin() const noexcept { return const_iterator(this, 0); }

    CHUNKED_VEC_INLINE iterator end() noexcept { return iterator(this, m_size); }
    CHUNKED_VEC_INLINE const_iterator end() const noexcept { return const_iterator(this, m_size); }
    CHUNKED_VEC_INLINE const_iterator cend() const noexcept { return const_iterator(this, m_size); }

    CHUNKED_VEC_INLINE bool empty() const noexcept { return m_size == 0; }
    CHUNKED_VEC_INLINE size_type size() const noexcept { return m_size; }
    CHUNKED_VEC_INLINE size_type capacity() const noexcept { return m_page_count * PAGE_SIZE; }
    CHUNKED_VEC_INLINE size_type max_size() const noexcept { return max_page_capacity() * PAGE_SIZE; }

    CHUNKED_VEC_INLINE void reserve(size_type new_capacity)
    {
        if (new_capacity <= capacity())
        {
            return;
        }

        size_type pages_needed = (new_capacity + PAGE_SIZE - 1) / PAGE_SIZE;
        ensure_page_capacity(pages_needed);

        for (size_type i = m_page_count; i < pages_needed; ++i)
        {
            allocate_page(i);
        }
    }

    void shrink_to_fit()
    {
        size_type pages_needed = (m_size + PAGE_SIZE - 1) / PAGE_SIZE;
        if (pages_needed == 0 && m_size == 0)
        {
            pages_needed = 0;
        }

        for (size_type i = pages_needed; i < m_page_count; ++i)
        {
            deallocate_page(i);
        }
        m_page_count = pages_needed;
    }

    CHUNKED_VEC_INLINE void clear() noexcept
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            // Only call destructors for non-trivial types
            // Optimize by iterating over pages first, then elements within each page
            size_type remaining_elements = m_size;
            for (size_type page_idx = 0; page_idx < m_page_count && remaining_elements > 0; ++page_idx)
            {
                size_type elements_in_this_page = std::min(remaining_elements, PAGE_SIZE);
                T* page = m_pages[page_idx];
                
                for (size_type elem_idx = 0; elem_idx < elements_in_this_page; ++elem_idx)
                {
                    dod::destruct(&page[elem_idx]);
                }
                
                remaining_elements -= elements_in_this_page;
            }
        }
        // For trivial types, just reset the size
        m_size = 0;
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        _invalidate_all_iterators();
#endif
    }

    CHUNKED_VEC_INLINE void push_back(const T& value)
    {
        emplace_back(value);
    }

    CHUNKED_VEC_INLINE void push_back(T&& value)
    {
        emplace_back(std::move(value));
    }

    template <typename... Args> CHUNKED_VEC_INLINE reference emplace_back(Args&&... args)
    {
        ensure_capacity_for_one_more();

        auto [page_idx, elem_idx] = get_page_and_element_indices(m_size);

        T* ptr = dod::construct<T>(&m_pages[page_idx][elem_idx], std::forward<Args>(args)...);
        ++m_size;
        return *ptr;
    }

    CHUNKED_VEC_INLINE void pop_back()
    {
        CHUNKED_VEC_ASSERT(m_size > 0 && "Cannot pop from empty chunked_vector");
        --m_size;
        
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            // Only call destructor for non-trivial types
            auto [page_idx, elem_idx] = get_page_and_element_indices(m_size);
            dod::destruct(&m_pages[page_idx][elem_idx]);
        }
        // For trivial types, no destructor call needed
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        _invalidate_iterators_at_or_after(m_size);
#endif
    }

    void resize(size_type count)
    {
        if (count < m_size)
        {
            shrink_to_size(count);
        }
        else if (count > m_size)
        {
            expand_to_size(count);
        }
        m_size = count;
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        _invalidate_iterators_at_or_after(std::min(m_size, count));
#endif
    }

    void resize(size_type count, const T& value)
    {
        if (count < m_size)
        {
            shrink_to_size(count);
        }
        else if (count > m_size)
        {
            expand_to_size(count, value);
        }
        m_size = count;
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        _invalidate_iterators_at_or_after(std::min(m_size, count));
#endif
    }

    iterator erase(const_iterator pos)
    {
        CHUNKED_VEC_VERIFY_ITERATOR(pos);
        CHUNKED_VEC_ASSERT(pos.m_container == this && "Iterator from different container");
        CHUNKED_VEC_ASSERT(pos.m_index < m_size && "Iterator out of range");
        
        size_type erase_idx = pos.m_index;
        
        // Destroy the element at the erase position
        auto [page_idx, elem_idx] = get_page_and_element_indices(erase_idx);
        dod::destruct(&m_pages[page_idx][elem_idx]);
        
        // Move all elements after the erase position one position forward using page-by-page iteration
        size_type elements_to_move = m_size - 1 - erase_idx;
        size_type src_idx = erase_idx + 1;
        size_type dst_idx = erase_idx;
        
        while (elements_to_move > 0)
        {
            size_type src_page = src_idx / PAGE_SIZE;
            size_type src_elem = src_idx % PAGE_SIZE;
            size_type dst_page = dst_idx / PAGE_SIZE;
            size_type dst_elem = dst_idx % PAGE_SIZE;
            
            size_type src_elements_in_page = PAGE_SIZE - src_elem;
            size_type dst_elements_in_page = PAGE_SIZE - dst_elem;
            size_type elements_to_move_in_batch = std::min({elements_to_move, src_elements_in_page, dst_elements_in_page});
            
            T* src_page_ptr = m_pages[src_page];
            T* dst_page_ptr = m_pages[dst_page];
            
            for (size_type i = 0; i < elements_to_move_in_batch; ++i)
            {
                // Move construct at destination and destruct source
                dod::construct<T>(&dst_page_ptr[dst_elem + i], std::move(src_page_ptr[src_elem + i]));
                dod::destruct(&src_page_ptr[src_elem + i]);
            }
            
            src_idx += elements_to_move_in_batch;
            dst_idx += elements_to_move_in_batch;
            elements_to_move -= elements_to_move_in_batch;
        }
        
        --m_size;
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        _invalidate_iterators_at_or_after(erase_idx);
#endif
        return iterator(this, erase_idx);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        CHUNKED_VEC_VERIFY_ITERATOR_RANGE(first, last);
        CHUNKED_VEC_ASSERT(last.m_index <= m_size && "Iterator out of range");
        
        if (first == last)
        {
            return iterator(this, first.m_index);
        }
        
        size_type first_idx = first.m_index;
        size_type last_idx = last.m_index;
        size_type erase_count = last_idx - first_idx;
        
        // Destroy elements in the range [first, last) using page-by-page iteration
        size_type destroy_idx = first_idx;
        size_type elements_to_destroy = erase_count;
        
        while (elements_to_destroy > 0)
        {
            size_type page_idx = destroy_idx / PAGE_SIZE;
            size_type start_elem_idx = destroy_idx % PAGE_SIZE;
            size_type elements_in_page = PAGE_SIZE - start_elem_idx;
            size_type elements_to_destroy_in_page = std::min(elements_to_destroy, elements_in_page);
            
            T* page = m_pages[page_idx];
            for (size_type elem_idx = start_elem_idx; elem_idx < start_elem_idx + elements_to_destroy_in_page; ++elem_idx)
            {
                dod::destruct(&page[elem_idx]);
            }
            
            destroy_idx += elements_to_destroy_in_page;
            elements_to_destroy -= elements_to_destroy_in_page;
        }
        
        // Move elements after last to fill the gap using page-by-page iteration
        size_type elements_to_move = m_size - last_idx;
        size_type src_idx = last_idx;
        size_type dst_idx = first_idx;
        
        while (elements_to_move > 0)
        {
            size_type src_page = src_idx / PAGE_SIZE;
            size_type src_elem = src_idx % PAGE_SIZE;
            size_type dst_page = dst_idx / PAGE_SIZE;
            size_type dst_elem = dst_idx % PAGE_SIZE;
            
            size_type src_elements_in_page = PAGE_SIZE - src_elem;
            size_type dst_elements_in_page = PAGE_SIZE - dst_elem;
            size_type elements_to_move_in_batch = std::min({elements_to_move, src_elements_in_page, dst_elements_in_page});
            
            T* src_page_ptr = m_pages[src_page];
            T* dst_page_ptr = m_pages[dst_page];
            
            for (size_type i = 0; i < elements_to_move_in_batch; ++i)
            {
                // Move construct at destination and destruct source
                dod::construct<T>(&dst_page_ptr[dst_elem + i], std::move(src_page_ptr[src_elem + i]));
                dod::destruct(&src_page_ptr[src_elem + i]);
            }
            
            src_idx += elements_to_move_in_batch;
            dst_idx += elements_to_move_in_batch;
            elements_to_move -= elements_to_move_in_batch;
        }
        
        m_size -= erase_count;
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        _invalidate_iterators_at_or_after(first_idx);
#endif
        return iterator(this, first_idx);
    }

    iterator erase_unsorted(const_iterator pos)
    {
        CHUNKED_VEC_VERIFY_ITERATOR(pos);
        CHUNKED_VEC_ASSERT(pos.m_container == this && "Iterator from different container");
        CHUNKED_VEC_ASSERT(pos.m_index < m_size && "Iterator out of range");
        
        size_type erase_idx = pos.m_index;
        
        if (erase_idx == m_size - 1)
        {
            // If erasing the last element, just pop_back
            pop_back();
            return end();
        }
        
        // Replace the element to be erased with the last element
        size_type erase_page = erase_idx / PAGE_SIZE;
        size_type erase_elem = erase_idx % PAGE_SIZE;
        size_type last_page = (m_size - 1) / PAGE_SIZE;
        size_type last_elem = (m_size - 1) % PAGE_SIZE;
        
        // Destroy the element to be erased
        dod::destruct(&m_pages[erase_page][erase_elem]);
        
        // Move the last element to the erased position
        dod::construct<T>(&m_pages[erase_page][erase_elem], std::move(m_pages[last_page][last_elem]));
        
        // Destroy the last element (which was moved)
        dod::destruct(&m_pages[last_page][last_elem]);
        
        --m_size;
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        _invalidate_iterators_at_or_after(erase_idx);
#endif
        return iterator(this, erase_idx);
    }

  private:
    T** m_pages;
    size_type m_page_count;
    size_type m_page_capacity;
    size_type m_size;
    
    // Constants for better readability
    static constexpr size_type SAFETY_MARGIN = 16;
    static constexpr size_type GROWTH_FACTOR_NUMERATOR = 3;
    static constexpr size_type GROWTH_FACTOR_DENOMINATOR = 2;
    
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
    // Forward declaration for iterator debugging
    struct iterator_node
    {
        const chunked_vector* container;
        size_type index;
        iterator_node* next;
        
        iterator_node() : container(nullptr), index(0), next(nullptr) {}
    };
    
    // Iterator debugging support similar to Microsoft STL
    mutable iterator_node* m_iterator_list;
    
    // Iterator debugging support methods
    void _invalidate_all_iterators() const noexcept
    {
        auto* node = m_iterator_list;
        m_iterator_list = nullptr;            // list becomes empty immediately
        while (node) {
            auto* next = node->next;          // store before we overwrite
            node->container = nullptr;
            node->next      = nullptr;        // unlink
            node            = next;
        }
    }
    
    void _invalidate_iterators_at_or_after(size_type pos) const noexcept
    {
        auto** pprev = &m_iterator_list;
        while (auto* curr = *pprev) {
            if (curr->index >= pos) {         // invalidate + unlink
                *pprev          = curr->next; // cut from list
                curr->container = nullptr;
                curr->next      = nullptr;
            } else {
                pprev = &curr->next;          // keep, advance
            }
        }
    }
    
    void _adopt_iterator(iterator_node* node) const noexcept
    {
        node->container = this;
        node->next = m_iterator_list;
        m_iterator_list = node;
    }
    
    void _orphan_iterator(iterator_node* node) const noexcept
    {
        auto** current = &m_iterator_list;
        while (*current)
        {
            if (*current == node)
            {
                *current = (*current)->next;
                node->next = nullptr;
                node->container = nullptr;
                break;
            }
            current = &(*current)->next;
        }
    }
#endif

    /// @brief Helper function to count trailing zeros (C++17 compatible)
    /// @param value The value to count trailing zeros for
    /// @return Number of trailing zeros, or 0 if value is 0
    static constexpr size_type count_trailing_zeros(size_type value) noexcept
    {
        if (value == 0) return 0;
        
        size_type count = 0;
        while ((value & 1) == 0) {
            value >>= 1;
            ++count;
        }
        return count;
    }

    /// @brief Calculate page and element indices from linear position
    /// @param pos Linear position in the container
    /// @return Pair of (page_index, element_index_within_page)
    CHUNKED_VEC_INLINE std::pair<size_type, size_type> get_page_and_element_indices(size_type pos) const noexcept
    {
        // Optimize for power-of-2 page sizes using bit operations
        if constexpr ((PAGE_SIZE & (PAGE_SIZE - 1)) == 0) {
            // PAGE_SIZE is a power of 2, use fast bit operations
            constexpr size_type PAGE_SIZE_BITS = count_trailing_zeros(PAGE_SIZE);
            return {pos >> PAGE_SIZE_BITS, pos & (PAGE_SIZE - 1)};
        } else {
            // Use regular division and modulo for non-power-of-2 sizes
            return {pos / PAGE_SIZE, pos % PAGE_SIZE};
        }
    }

    /// @brief Get the maximum number of pages that can be allocated
    CHUNKED_VEC_INLINE size_type max_page_capacity() const noexcept
    {
        // Maximum size_type value divided by pointer size, with safety margin
        constexpr size_type MAX_POINTERS = std::numeric_limits<size_type>::max() / sizeof(T*);
        // Leave room for allocation headers and alignment
        return MAX_POINTERS > SAFETY_MARGIN ? MAX_POINTERS - SAFETY_MARGIN : MAX_POINTERS;
    }

    /// @brief Calculate geometric growth similar to std::vector, but for pages
    /// @param pages_needed Minimum number of pages required
    /// @return New page capacity using geometric growth (1.5x)
    CHUNKED_VEC_INLINE size_type calculate_page_growth(size_type pages_needed) const
    {
        const size_type old_capacity = m_page_capacity;
        const size_type max_capacity = max_page_capacity();

        // Handle overflow case: if old_capacity > max - old_capacity/2
        if (old_capacity > max_capacity - old_capacity / GROWTH_FACTOR_DENOMINATOR)
        {
            return max_capacity; // geometric growth would overflow
        }

        // Calculate geometric growth: old_capacity * 1.5 (3/2 growth factor)
        const size_type geometric = old_capacity + old_capacity / GROWTH_FACTOR_DENOMINATOR;

        if (geometric < pages_needed)
        {
            return pages_needed; // geometric growth would be insufficient
        }

        return geometric; // geometric growth is sufficient
    }

    CHUNKED_VEC_INLINE void ensure_capacity_for_one_more()
    {
        size_type page_idx = m_size / PAGE_SIZE;
        if (page_idx >= m_page_count)
        {
            ensure_page_capacity(page_idx + 1);
            allocate_page(page_idx);
        }
    }

    CHUNKED_VEC_INLINE void ensure_page_capacity(size_type pages_needed)
    {
        if (pages_needed <= m_page_capacity)
        {
            return;
        }

        size_type new_page_capacity;
        
        if (m_page_capacity == 0)
        {
            // Start with exactly what's needed, but at least 1 page
            new_page_capacity = pages_needed > 0 ? pages_needed : 1;
        }
        else
        {
            // Use geometric growth calculation similar to std::vector
            new_page_capacity = calculate_page_growth(pages_needed);
        }

        T** new_pages = static_cast<T**>(CHUNKED_VEC_ALLOC(new_page_capacity * sizeof(T*), safe_alignment_of<T*>));
        
        for (size_type i = 0; i < m_page_count; ++i)
        {
            new_pages[i] = m_pages[i];
        }

        for (size_type i = m_page_count; i < new_page_capacity; ++i)
        {
            new_pages[i] = nullptr;
        }

        if (m_pages)
        {
            CHUNKED_VEC_FREE(m_pages);
        }

        m_pages = new_pages;
        m_page_capacity = new_page_capacity;
    }

    CHUNKED_VEC_INLINE void allocate_page(size_type page_idx)
    {
        CHUNKED_VEC_ASSERT(page_idx < m_page_capacity && "Page index out of capacity");
        CHUNKED_VEC_ASSERT(m_pages[page_idx] == nullptr && "Page already allocated");
        
        m_pages[page_idx] = static_cast<T*>(CHUNKED_VEC_ALLOC(PAGE_SIZE * sizeof(T), safe_alignment_of<T>));
        if (page_idx >= m_page_count)
        {
            m_page_count = page_idx + 1;
        }
    }

    void deallocate_page(size_type page_idx)
    {
        CHUNKED_VEC_ASSERT(page_idx < m_page_count && "Page index out of range");
        if (m_pages[page_idx])
        {
            CHUNKED_VEC_FREE(m_pages[page_idx]);
            m_pages[page_idx] = nullptr;
        }
    }

    void deallocate_page_array()
    {
        if (m_pages)
        {
            for (size_type i = 0; i < m_page_count; ++i)
            {
                if (m_pages[i])
                {
                    CHUNKED_VEC_FREE(m_pages[i]);
                }
            }
            CHUNKED_VEC_FREE(m_pages);
            m_pages = nullptr;
        }
        m_page_capacity = 0;
        m_page_count = 0;
    }

    // Optimized bulk construction with the same value
    void bulk_construct_with_value(size_type start_idx, size_type end_idx, const T& value)
    {
        if (start_idx >= end_idx) return;
        
        size_type current_idx = start_idx;
        
        while (current_idx < end_idx)
        {
            size_type page_idx = current_idx / PAGE_SIZE;
            size_type start_elem_idx = current_idx % PAGE_SIZE;
            size_type elements_remaining = end_idx - current_idx;
            size_type elements_in_page = PAGE_SIZE - start_elem_idx;
            size_type elements_to_construct = std::min(elements_remaining, elements_in_page);
            
            // Construct elements in this page segment
            T* page_ptr = m_pages[page_idx];
            
            if constexpr (std::is_trivially_copyable_v<T> && std::is_trivially_constructible_v<T>)
            {
                // For trivial types, use optimized bulk operations
                for (size_type i = 0; i < elements_to_construct; ++i)
                {
                    page_ptr[start_elem_idx + i] = value;
                }
            }
            else
            {
                // For non-trivial types, use placement new
                for (size_type i = 0; i < elements_to_construct; ++i)
                {
                    dod::construct<T>(&page_ptr[start_elem_idx + i], value);
                }
            }
            
            current_idx += elements_to_construct;
        }
    }

    // Optimized bulk construction with default constructor
    void bulk_construct_default(size_type start_idx, size_type end_idx)
    {
        if (start_idx >= end_idx) return;
        
        size_type current_idx = start_idx;
        
        while (current_idx < end_idx)
        {
            size_type page_idx = current_idx / PAGE_SIZE;
            size_type start_elem_idx = current_idx % PAGE_SIZE;
            size_type elements_remaining = end_idx - current_idx;
            size_type elements_in_page = PAGE_SIZE - start_elem_idx;
            size_type elements_to_construct = std::min(elements_remaining, elements_in_page);
            
            // Construct elements in this page segment
            T* page_ptr = m_pages[page_idx];
            
            if constexpr (std::is_trivially_default_constructible_v<T> && std::is_trivially_copyable_v<T>)
            {
                // For trivial types, use memset if default construction is zero-initialization
                if constexpr (std::is_arithmetic_v<T> || std::is_pointer_v<T>)
                {
                    std::memset(&page_ptr[start_elem_idx], 0, elements_to_construct * sizeof(T));
                }
                else
                {
                    // For other trivial types, use assignment
                    T default_value{};
                    for (size_type i = 0; i < elements_to_construct; ++i)
                    {
                        page_ptr[start_elem_idx + i] = default_value;
                    }
                }
            }
            else
            {
                // For non-trivial types, use placement new
                for (size_type i = 0; i < elements_to_construct; ++i)
                {
                    dod::construct<T>(&page_ptr[start_elem_idx + i]);
                }
            }
            
            current_idx += elements_to_construct;
        }
    }

    // Optimized bulk copy from another chunked_vector (for trivial types)
    void bulk_copy_from(const chunked_vector& other)
    {
        static_assert(std::is_trivially_copyable_v<T>, "bulk_copy_from should only be called for trivial types");
        
        // Optimize by iterating over pages first, then elements within each page
        size_type remaining_elements = other.m_size;
        size_type current_idx = 0;
        
        for (size_type page_idx = 0; page_idx < other.m_page_count && remaining_elements > 0; ++page_idx)
        {
            size_type elements_in_this_page = std::min(remaining_elements, PAGE_SIZE);
            T* dst_page = m_pages[page_idx];
            const T* src_page = other.m_pages[page_idx];
            
            // Copy elements within this page using memcpy for better performance
            std::memcpy(dst_page, src_page, elements_in_this_page * sizeof(T));
            
            remaining_elements -= elements_in_this_page;
            current_idx += elements_in_this_page;
        }
        m_size = other.m_size;
    }

    // Helper function to shrink container to specified size
    void shrink_to_size(size_type new_size)
    {
        if constexpr (!std::is_trivially_destructible_v<T>)
        {
            // Only call destructors for non-trivial types
            // Optimize by iterating over pages, starting from the element at 'new_size'
            size_type start_idx = new_size;
            size_type elements_to_destroy = m_size - new_size;
            
            while (elements_to_destroy > 0)
            {
                size_type page_idx = start_idx / PAGE_SIZE;
                size_type start_elem_idx = start_idx % PAGE_SIZE;
                size_type elements_in_page = PAGE_SIZE - start_elem_idx;
                size_type elements_to_destroy_in_page = std::min(elements_to_destroy, elements_in_page);
                
                T* page = m_pages[page_idx];
                for (size_type elem_idx = start_elem_idx; elem_idx < start_elem_idx + elements_to_destroy_in_page; ++elem_idx)
                {
                    dod::destruct(&page[elem_idx]);
                }
                
                start_idx += elements_to_destroy_in_page;
                elements_to_destroy -= elements_to_destroy_in_page;
            }
        }
        // For trivial types, no destructor calls needed
    }

    // Helper function to expand container to specified size with default values
    void expand_to_size(size_type new_size)
    {
        reserve(new_size);
        bulk_construct_default(m_size, new_size);
    }

    // Helper function to expand container to specified size with given value
    void expand_to_size(size_type new_size, const T& value)
    {
        reserve(new_size);
        bulk_construct_with_value(m_size, new_size, value);
    }

  public:
    template <typename ValueType> class basic_iterator
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        : public iterator_node
#endif
    {
      public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::remove_cv_t<ValueType>;
        using difference_type = std::ptrdiff_t;
        using pointer = ValueType*;
        using reference = ValueType&;

        template <typename> friend class basic_iterator;
        friend class chunked_vector;

        basic_iterator() noexcept
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            : iterator_node()
            , m_container(nullptr)
#else
            : m_container(nullptr)
#endif
            , m_index(0)
            , m_current_page(nullptr)
            , m_page_element_index(0)
        {
        }

        basic_iterator(const chunked_vector* container, size_type index) noexcept
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            : iterator_node()
            , m_container(container)
#else
            : m_container(container)
#endif
            , m_index(index)
        {
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            this->index = index;
            if (container)
            {
                container->_adopt_iterator(this);
            }
#endif
            update_page_cache();
        }

        basic_iterator(const basic_iterator& other) noexcept
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            : iterator_node()
            , m_container(other.m_container)
#else
            : m_container(other.m_container)
#endif
            , m_index(other.m_index)
        {
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            this->index = other.index;
            if (m_container)
            {
                m_container->_adopt_iterator(this);
            }
#endif
            update_page_cache();
        }

        template <typename U, typename = std::enable_if_t<std::is_const_v<ValueType> && !std::is_const_v<U>>>
        basic_iterator(const basic_iterator<U>& other) noexcept
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            : iterator_node()
            , m_container(other.m_container)
#else
            : m_container(other.m_container)
#endif
            , m_index(other.m_index)
        {
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            this->index = other.index;
            if (m_container)
            {
                m_container->_adopt_iterator(this);
            }
#endif
            update_page_cache();
        }

        basic_iterator(basic_iterator&& other) noexcept
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            : iterator_node()
            , m_container(other.m_container)
#else
            : m_container(other.m_container)
#endif
            , m_index(other.m_index)
        {
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            this->index = other.index;
            if (m_container)
            {
                m_container->_adopt_iterator(this);
                // Properly orphan the other iterator before moving
                m_container->_orphan_iterator(&other);
            }
#endif
            other.m_container = nullptr;
            other.m_index = 0;
            update_page_cache();
        }

        ~basic_iterator()
        {
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            if (m_container)
            {
                m_container->_orphan_iterator(this);
            }
#endif
        }

        basic_iterator& operator=(const basic_iterator& other)
        {
            if (this != &other)
            {
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
                if (m_container)
                {
                    m_container->_orphan_iterator(this);
                }
#endif
                m_container = other.m_container;
                m_index = other.m_index;
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
                this->index = other.index;
                if (m_container)
                {
                    m_container->_adopt_iterator(this);
                }
#endif
                update_page_cache();
            }
            return *this;
        }

        basic_iterator& operator=(basic_iterator&& other) noexcept
        {
            if (this != &other)
            {
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
                if (m_container)
                {
                    m_container->_orphan_iterator(this);
                }
#endif
                m_container = other.m_container;
                m_index = other.m_index;
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
                this->index = other.index;
                if (m_container)
                {
                    m_container->_adopt_iterator(this);
                    // Properly orphan the other iterator before moving
                    m_container->_orphan_iterator(&other);
                }
#endif
                other.m_container = nullptr;
                other.m_index = 0;
                update_page_cache();
            }
            return *this;
        }

        reference operator*() const
        {
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            _verify_valid(__FILE__, __LINE__);
#endif
            CHUNKED_VEC_ASSERT(m_container && m_index < m_container->size() && "Iterator out of range");
            CHUNKED_VEC_ASSERT(m_current_page && "Invalid page cache");
            return m_current_page[m_page_element_index];
        }

        pointer operator->() const
        {
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            _verify_valid(__FILE__, __LINE__);
#endif
            CHUNKED_VEC_ASSERT(m_container && m_index < m_container->size() && "Iterator out of range");
            CHUNKED_VEC_ASSERT(m_current_page && "Invalid page cache");
            return &m_current_page[m_page_element_index];
        }

        CHUNKED_VEC_INLINE basic_iterator& operator++()
        {
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            _verify_valid(__FILE__, __LINE__);
#endif
            ++m_index;
            ++m_page_element_index;
            
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
            this->index = m_index;
#endif
            
            // Only update page cache if we've crossed a page boundary or reached the end
            if (m_page_element_index >= PAGE_SIZE) {
                update_page_cache();
            } else if (m_index >= m_container->size()) {
                // Reached the end, invalidate cache
                m_current_page = nullptr;
                m_page_element_index = 0;
            }
            
            return *this;
        }

        CHUNKED_VEC_INLINE basic_iterator operator++(int)
        {
            basic_iterator temp = *this;
            ++(*this);
            return temp;
        }

        bool operator==(const basic_iterator& other) const noexcept
        {
            return m_container == other.m_container && m_index == other.m_index;
        }

        bool operator!=(const basic_iterator& other) const noexcept
        {
            return !(*this == other);
        }

      private:
        const chunked_vector* m_container;
        size_type m_index;
        ValueType* m_current_page;
        size_type m_page_element_index;
        
#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0
        void _verify_valid(const char* file, int line) const
        {
            if (!m_container)
            {
                CHUNKED_VEC_ASSERT(false && "Iterator is not associated with a container");
                return;
            }
            
            if (this->container != m_container)
            {
                CHUNKED_VEC_ASSERT(false && "Iterator has been invalidated");
                return;
            }
            
            if (m_index > m_container->size())
            {
                CHUNKED_VEC_ASSERT(false && "Iterator is out of range");
                return;
            }
            
            (void)file; // Suppress unused parameter warning
            (void)line; // Suppress unused parameter warning
        }
#endif

        CHUNKED_VEC_INLINE void update_page_cache()
        {
            if (!m_container || m_index >= m_container->size())
            {
                m_current_page = nullptr;
                m_page_element_index = 0;
                return;
            }

            auto [page_idx, elem_idx] = m_container->get_page_and_element_indices(m_index);
            m_page_element_index = elem_idx;
            
            if constexpr (std::is_const_v<ValueType>) {
                m_current_page = m_container->m_pages[page_idx];
            } else {
                m_current_page = const_cast<chunked_vector*>(m_container)->m_pages[page_idx];
            }
        }
    };
};



} // namespace dod 