#pragma once

#include <cassert>
#include <cstddef>
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
#define CHUNKED_VEC_ALLOC(sizeInBytes, alignment) aligned_alloc(alignment, sizeInBytes)
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

    static constexpr size_t page_size() { return PAGE_SIZE; }

    chunked_vector() noexcept
        : m_pages(nullptr)
        , m_page_count(0)
        , m_page_capacity(0)
        , m_size(0)
    {
    }

    explicit chunked_vector(size_type count)
        : m_pages(nullptr)
        , m_page_count(0)
        , m_page_capacity(0)
        , m_size(0)
    {
        resize(count);
    }

    chunked_vector(size_type count, const T& value)
        : m_pages(nullptr)
        , m_page_count(0)
        , m_page_capacity(0)
        , m_size(0)
    {
        resize(count, value);
    }

    chunked_vector(std::initializer_list<T> init)
        : m_pages(nullptr)
        , m_page_count(0)
        , m_page_capacity(0)
        , m_size(0)
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
    {
        *this = other;
    }

    chunked_vector(chunked_vector&& other) noexcept
        : m_pages(other.m_pages)
        , m_page_count(other.m_page_count)
        , m_page_capacity(other.m_page_capacity)
        , m_size(other.m_size)
    {
        other.m_pages = nullptr;
        other.m_page_count = 0;
        other.m_page_capacity = 0;
        other.m_size = 0;
    }

    ~chunked_vector()
    {
        clear();
        deallocate_page_array();
    }

    chunked_vector& operator=(const chunked_vector& other)
    {
        if (this != &other)
        {
            clear();
            reserve(other.m_size);
            for (size_type i = 0; i < other.m_size; ++i)
            {
                push_back(other[i]);
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

            m_pages = other.m_pages;
            m_page_count = other.m_page_count;
            m_page_capacity = other.m_page_capacity;
            m_size = other.m_size;

            other.m_pages = nullptr;
            other.m_page_count = 0;
            other.m_page_capacity = 0;
            other.m_size = 0;
        }
        return *this;
    }

    chunked_vector& operator=(std::initializer_list<T> init)
    {
        clear();
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

    reference at(size_type pos)
    {
        if (pos >= m_size)
        {
            throw std::out_of_range("chunked_vector::at: index out of range");
        }
        return (*this)[pos];
    }

    const_reference at(size_type pos) const
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

    void reserve(size_type new_capacity)
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

    void clear() noexcept
    {
        for (size_type i = 0; i < m_size; ++i)
        {
            auto [page_idx, elem_idx] = get_page_and_element_indices(i);
            dod::destruct(&m_pages[page_idx][elem_idx]);
        }
        m_size = 0;
    }

    void push_back(const T& value)
    {
        emplace_back(value);
    }

    void push_back(T&& value)
    {
        emplace_back(std::move(value));
    }

    template <typename... Args> reference emplace_back(Args&&... args)
    {
        ensure_capacity_for_one_more();

        auto [page_idx, elem_idx] = get_page_and_element_indices(m_size);

        T* ptr = dod::construct<T>(&m_pages[page_idx][elem_idx], std::forward<Args>(args)...);
        ++m_size;
        return *ptr;
    }

    void pop_back()
    {
        CHUNKED_VEC_ASSERT(m_size > 0 && "Cannot pop from empty chunked_vector");
        --m_size;
        auto [page_idx, elem_idx] = get_page_and_element_indices(m_size);
        dod::destruct(&m_pages[page_idx][elem_idx]);
    }

    void resize(size_type count)
    {
        if (count < m_size)
        {
            for (size_type i = count; i < m_size; ++i)
            {
                auto [page_idx, elem_idx] = get_page_and_element_indices(i);
                dod::destruct(&m_pages[page_idx][elem_idx]);
            }
        }
        else if (count > m_size)
        {
            reserve(count);
            bulk_construct_default(m_size, count);
        }
        m_size = count;
    }

    void resize(size_type count, const T& value)
    {
        if (count < m_size)
        {
            for (size_type i = count; i < m_size; ++i)
            {
                auto [page_idx, elem_idx] = get_page_and_element_indices(i);
                dod::destruct(&m_pages[page_idx][elem_idx]);
            }
        }
        else if (count > m_size)
        {
            reserve(count);
            bulk_construct_with_value(m_size, count, value);
        }
        m_size = count;
    }

    iterator erase(const_iterator pos)
    {
        CHUNKED_VEC_ASSERT(pos.m_container == this && "Iterator from different container");
        CHUNKED_VEC_ASSERT(pos.m_index < m_size && "Iterator out of range");
        
        size_type erase_idx = pos.m_index;
        
        // Destroy the element at the erase position
        auto [page_idx, elem_idx] = get_page_and_element_indices(erase_idx);
        dod::destruct(&m_pages[page_idx][elem_idx]);
        
        // Move all elements after the erase position one position forward
        for (size_type i = erase_idx; i < m_size - 1; ++i)
        {
            size_type src_page = (i + 1) / PAGE_SIZE;
            size_type src_elem = (i + 1) % PAGE_SIZE;
            size_type dst_page = i / PAGE_SIZE;
            size_type dst_elem = i % PAGE_SIZE;
            
            // Move construct at destination and destruct source
            dod::construct<T>(&m_pages[dst_page][dst_elem], std::move(m_pages[src_page][src_elem]));
            dod::destruct(&m_pages[src_page][src_elem]);
        }
        
        --m_size;
        return iterator(this, erase_idx);
    }

    iterator erase(const_iterator first, const_iterator last)
    {
        CHUNKED_VEC_ASSERT(first.m_container == this && last.m_container == this && "Iterators from different container");
        CHUNKED_VEC_ASSERT(first.m_index <= last.m_index && "Invalid iterator range");
        CHUNKED_VEC_ASSERT(last.m_index <= m_size && "Iterator out of range");
        
        if (first == last)
        {
            return iterator(this, first.m_index);
        }
        
        size_type first_idx = first.m_index;
        size_type last_idx = last.m_index;
        size_type erase_count = last_idx - first_idx;
        
        // Destroy elements in the range [first, last)
        for (size_type i = first_idx; i < last_idx; ++i)
        {
            size_type page_idx = i / PAGE_SIZE;
            size_type elem_idx = i % PAGE_SIZE;
            dod::destruct(&m_pages[page_idx][elem_idx]);
        }
        
        // Move elements after last to fill the gap
        for (size_type i = last_idx; i < m_size; ++i)
        {
            size_type src_page = i / PAGE_SIZE;
            size_type src_elem = i % PAGE_SIZE;
            size_type dst_idx = i - erase_count;
            size_type dst_page = dst_idx / PAGE_SIZE;
            size_type dst_elem = dst_idx % PAGE_SIZE;
            
            // Move construct at destination and destruct source
            dod::construct<T>(&m_pages[dst_page][dst_elem], std::move(m_pages[src_page][src_elem]));
            dod::destruct(&m_pages[src_page][src_elem]);
        }
        
        m_size -= erase_count;
        return iterator(this, first_idx);
    }

    iterator erase_unsorted(const_iterator pos)
    {
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
        return iterator(this, erase_idx);
    }

  private:
    T** m_pages;
    size_type m_page_count;
    size_type m_page_capacity;
    size_type m_size;

    // Get the maximum number of pages that can be allocated
    size_type max_page_capacity() const noexcept
    {
        // Maximum size_type value divided by pointer size, with some safety margin
        const size_type max_pointers = std::numeric_limits<size_type>::max() / sizeof(T*);
        // Leave some room for allocation headers and alignment
        return max_pointers > 16 ? max_pointers - 16 : max_pointers;
    }

    // Calculate geometric growth similar to std::vector, but for pages
    size_type calculate_page_growth(size_type pages_needed) const
    {
        const size_type old_capacity = m_page_capacity;
        const size_type max_capacity = max_page_capacity();

        // Handle overflow case: if old_capacity > max - old_capacity/2
        if (old_capacity > max_capacity - old_capacity / 2)
        {
            return max_capacity; // geometric growth would overflow
        }

        // Calculate geometric growth: old_capacity + old_capacity/2 (1.5x growth)
        const size_type geometric = old_capacity + old_capacity / 2;

        if (geometric < pages_needed)
        {
            return pages_needed; // geometric growth would be insufficient
        }

        return geometric; // geometric growth is sufficient
    }

    void ensure_capacity_for_one_more()
    {
        size_type page_idx = m_size / PAGE_SIZE;
        if (page_idx >= m_page_count)
        {
            ensure_page_capacity(page_idx + 1);
            allocate_page(page_idx);
        }
    }

    void ensure_page_capacity(size_type pages_needed)
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

    void allocate_page(size_type page_idx)
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

    // Helper function to calculate page and element indices from linear position
    CHUNKED_VEC_INLINE std::pair<size_type, size_type> get_page_and_element_indices(size_type pos) const noexcept
    {
        return {pos / PAGE_SIZE, pos % PAGE_SIZE};
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
            for (size_type i = 0; i < elements_to_construct; ++i)
            {
                dod::construct<T>(&page_ptr[start_elem_idx + i], value);
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
            for (size_type i = 0; i < elements_to_construct; ++i)
            {
                dod::construct<T>(&page_ptr[start_elem_idx + i]);
            }
            
            current_idx += elements_to_construct;
        }
    }

  public:
    template <typename ValueType> class basic_iterator
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
            : m_container(nullptr)
            , m_index(0)
            , m_current_page(nullptr)
            , m_page_element_index(0)
        {
        }

        basic_iterator(const chunked_vector* container, size_type index) noexcept
            : m_container(container)
            , m_index(index)
        {
            update_page_cache();
        }

        template <typename U, typename = std::enable_if_t<std::is_const_v<ValueType> && !std::is_const_v<U>>>
        basic_iterator(const basic_iterator<U>& other) noexcept
            : m_container(other.m_container)
            , m_index(other.m_index)
        {
            update_page_cache();
        }

        reference operator*() const
        {
            CHUNKED_VEC_ASSERT(m_container && m_index < m_container->size() && "Iterator out of range");
            CHUNKED_VEC_ASSERT(m_current_page && "Invalid page cache");
            return m_current_page[m_page_element_index];
        }

        pointer operator->() const
        {
            CHUNKED_VEC_ASSERT(m_container && m_index < m_container->size() && "Iterator out of range");
            CHUNKED_VEC_ASSERT(m_current_page && "Invalid page cache");
            return &m_current_page[m_page_element_index];
        }

        basic_iterator& operator++()
        {
            ++m_index;
            ++m_page_element_index;
            if (m_page_element_index >= PAGE_SIZE || m_index >= m_container->size())
            {
                update_page_cache();
            }
            return *this;
        }

        basic_iterator operator++(int)
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

        void update_page_cache()
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