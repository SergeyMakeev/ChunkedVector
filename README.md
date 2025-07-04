# chunked_vector

[![License](https://img.shields.io/github/license/SergeyMakeev/chunked_vector)](LICENSE)
[![Language](https://img.shields.io/github/languages/top/SergeyMakeev/chunked_vector)](https://github.com/SergeyMakeev/chunked_vector)
[![codecov](https://codecov.io/gh/SergeyMakeev/chunked_vector/graph/badge.svg?token=8B80XIGDVT)](https://codecov.io/gh/SergeyMakeev/chunked_vector)


A high-performance, header-only C++17 chunked vector implementation with comprehensive testing.

## Features

- **Header-only library** - Easy to integrate into your projects
- **C++17 compatible** - Modern C++ with backward compatibility
- **Cross-platform** - Builds and runs on Windows, Linux, and macOS
- **Comprehensive testing** - Extensive test suite with GoogleTest
- **Memory efficient** - Chunked memory allocation strategy
- **STL compatible** - Works with standard algorithms and iterators

## Build Status

Each badge above represents a different aspect of our continuous integration:

[![Linux GCC](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/linux-gcc.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/linux-gcc.yml)  
[![Linux Clang](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/linux-clang.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/linux-clang.yml)  
[![macOS](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/macos.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/macos.yml)  
[![Windows](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/windows.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/windows.yml)  
[![Coverage](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/coverage.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/coverage.yml)  
[![Sanitizers](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/sanitizers.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/sanitizers.yml)  

## Quick Start

```cpp
#include "chunked_vector/chunked_vector.h"

int main() {
    dod::chunked_vector<int> vec;
    
    // Add some elements
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    // Use like std::vector
    for (const auto& item : vec) {
        std::cout << item << " ";
    }
    
    return 0;
}
```

## Overview

The `chunked_vector` is a high-performance container that provides a `std::vector`-like interface while using a chunked memory allocation strategy. Instead of storing all elements in a single contiguous memory block, it organizes data into fixed-size pages (chunks), providing several advantages:

- **Reduced memory fragmentation** - Allocates memory in fixed-size chunks
- **Stable element addresses** - Elements don't move during growth (except during explicit operations)
- **Efficient growth** - No need to copy entire contents when expanding
- **Cache-friendly access** - Elements within a page are contiguous
- **Configurable page size** - Tune for specific use cases

## Template Parameters

```cpp
template <typename T, size_t PAGE_SIZE = 1024>
class chunked_vector;
```

- **`T`** - The type of elements stored in the vector
- **`PAGE_SIZE`** - Number of elements per page (default: 1024)
  - Must be greater than 0
  - Power-of-2 values are optimized for better performance
  - Recommended values: 256, 512, 1024, 2048, 4096

## API Documentation

### Member Types

```cpp
using value_type = T;
using reference = T&;
using const_reference = const T&;
using pointer = T*;
using const_pointer = const T*;
using size_type = std::size_t;
using difference_type = std::ptrdiff_t;
using iterator = basic_iterator<T>;
using const_iterator = basic_iterator<const T>;
```

### Constructors

```cpp
// Default constructor
chunked_vector() noexcept;

// Fill constructor
explicit chunked_vector(size_type count);
chunked_vector(size_type count, const T& value);

// Initializer list constructor
chunked_vector(std::initializer_list<T> init);

// Copy constructor
chunked_vector(const chunked_vector& other);

// Move constructor
chunked_vector(chunked_vector&& other) noexcept;
```

### Element Access

```cpp
// Unchecked access
reference operator[](size_type pos);
const_reference operator[](size_type pos) const;

// Checked access (throws std::out_of_range)
reference at(size_type pos);
const_reference at(size_type pos) const;

// First and last elements
reference front();
const_reference front() const;
reference back();
const_reference back() const;
```

### Iterators

```cpp
iterator begin() noexcept;
const_iterator begin() const noexcept;
const_iterator cbegin() const noexcept;

iterator end() noexcept;
const_iterator end() const noexcept;
const_iterator cend() const noexcept;
```

### Capacity

```cpp
bool empty() const noexcept;
size_type size() const noexcept;
size_type capacity() const noexcept;
size_type max_size() const noexcept;

void reserve(size_type new_capacity);
void shrink_to_fit();

static constexpr size_t page_size();
```

### Modifiers

```cpp
void clear() noexcept;

void push_back(const T& value);
void push_back(T&& value);

template<typename... Args>
reference emplace_back(Args&&... args);

void pop_back();

void resize(size_type count);
void resize(size_type count, const T& value);

// Erase operations
iterator erase(const_iterator pos);
iterator erase(const_iterator first, const_iterator last);
iterator erase_unsorted(const_iterator pos);  // Fast unordered erase
```

### Assignment

```cpp
chunked_vector& operator=(const chunked_vector& other);
chunked_vector& operator=(chunked_vector&& other) noexcept;
chunked_vector& operator=(std::initializer_list<T> init);
```

## Usage Examples

### Basic Usage

```cpp
#include "chunked_vector/chunked_vector.h"
#include <iostream>

int main() {
    // Create a chunked_vector with default page size
    dod::chunked_vector<int> vec;
    
    // Add elements
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);
    
    // Access elements
    std::cout << "First: " << vec.front() << std::endl;
    std::cout << "Last: " << vec.back() << std::endl;
    std::cout << "Size: " << vec.size() << std::endl;
    
    // Iterate
    for (const auto& value : vec) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### Custom Page Size

```cpp
// Use smaller page size for memory-constrained environments
dod::chunked_vector<int, 256> small_vec;

// Use larger page size for better cache performance
dod::chunked_vector<int, 4096> large_vec;

// Power-of-2 page sizes are optimized
dod::chunked_vector<int, 1024> optimized_vec;
```

### Working with Custom Types

```cpp
struct Point {
    float x, y, z;
    
    Point(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    
    Point(const Point&) = default;
    Point& operator=(const Point&) = default;
    
    Point(Point&&) = default;
    Point& operator=(Point&&) = default;
};

int main() {
    dod::chunked_vector<Point> points;
    
    // Add points using different methods
    points.push_back({1.0f, 2.0f, 3.0f});
    points.emplace_back(4.0f, 5.0f, 6.0f);
    
    // Reserve space to avoid reallocations
    points.reserve(1000);
    
    // Resize with default-constructed elements
    points.resize(10);
    
    // Resize with specific value
    points.resize(20, Point{1.0f, 1.0f, 1.0f});
    
    return 0;
}
```

### STL Algorithm Compatibility

```cpp
#include <algorithm>
#include <numeric>

int main() {
    dod::chunked_vector<int> vec = {5, 2, 8, 1, 9, 3};
    
    // Sort elements
    std::sort(vec.begin(), vec.end());
    
    // Find elements
    auto it = std::find(vec.begin(), vec.end(), 8);
    if (it != vec.end()) {
        std::cout << "Found 8 at position " << std::distance(vec.begin(), it) << std::endl;
    }
    
    // Calculate sum
    int sum = std::accumulate(vec.begin(), vec.end(), 0);
    std::cout << "Sum: " << sum << std::endl;
    
    return 0;
}
```

### Performance-Optimized Operations

```cpp
int main() {
    dod::chunked_vector<int> vec;
    
    // Reserve capacity to avoid multiple reallocations
    vec.reserve(10000);
    
    // Use emplace_back for in-place construction
    for (int i = 0; i < 10000; ++i) {
        vec.emplace_back(i);
    }
    
    // Use erase_unsorted for fast removal when order doesn't matter
    auto it = vec.begin() + 100;
    vec.erase_unsorted(it);  // O(1) instead of O(n)
    
    // Shrink to fit to reclaim unused memory
    vec.shrink_to_fit();
    
    return 0;
}
```

## Performance Characteristics

### Time Complexity

| Operation | Time Complexity | Notes |
|-----------|-----------------|-------|
| `operator[]`, `at()` | O(1) | Optimized for power-of-2 page sizes |
| `front()`, `back()` | O(1) | Direct access |
| `push_back()`, `emplace_back()` | O(1) amortized | May allocate new page |
| `pop_back()` | O(1) | No reallocation |
| `insert()` | O(n) | Elements need to be shifted |
| `erase()` | O(n) | Elements need to be shifted |
| `erase_unsorted()` | O(1) | Fast unordered removal |
| `clear()` | O(n) for non-trivial types, O(1) for trivial | Destructor calls |
| `resize()` | O(k) where k is the difference | Construction/destruction |
| Iterator increment | O(1) | Cached page access |

### Space Complexity

- **Memory overhead**: `sizeof(T*) * number_of_pages + small_constant`
- **Memory efficiency**: ~99% for large containers (overhead becomes negligible)
- **Fragmentation**: Minimal due to fixed-size page allocation

### Optimizations

- **Trivial types**: Optimized bulk operations using `memcpy` and `memset`
- **Power-of-2 page sizes**: Bit operations instead of division/modulo
- **Iterator caching**: Reduces page lookup overhead during iteration
- **Geometric growth**: Page array grows similar to `std::vector`

## Configuration Options

### Custom Memory Allocators

```cpp
// Define custom allocator macros before including the header
#define CHUNKED_VEC_ALLOC(size, alignment) my_aligned_alloc(size, alignment)
#define CHUNKED_VEC_FREE(ptr) my_free(ptr)
#include "chunked_vector/chunked_vector.h"
```

### Custom Assertions

```cpp
// Define custom assertion macro
#define CHUNKED_VEC_ASSERT(expr) my_assert(expr)
#include "chunked_vector/chunked_vector.h"
```

### Inline Control

```cpp
// Override inlining behavior
#define CHUNKED_VEC_INLINE __forceinline
#include "chunked_vector/chunked_vector.h"
```

## Memory Management

### Page Allocation Strategy

1. **Initial state**: No pages allocated
2. **Growth**: Pages allocated on-demand during `push_back()` or `reserve()`
3. **Page array growth**: Geometric growth (1.5x) similar to `std::vector`
4. **Alignment**: Respects type alignment requirements (minimum `alignof(void*)`)

### Memory Layout

```
Page Array:     [Page0*] [Page1*] [Page2*] [Page3*] ...
                    |        |        |        |
                    v        v        v        v
Pages:          [elem0...] [elem1024...] [elem2048...] [elem3072...]
```

### Platform-Specific Optimizations

- **Windows**: Uses `_mm_malloc()` and `_mm_free()` for aligned allocation
- **POSIX**: Uses `aligned_alloc()` and `free()`
- **macOS**: Ensures minimum alignment of `alignof(void*)` for compatibility

## Best Practices

1. **Choose appropriate page size**: 
   - Smaller for memory-constrained environments
   - Larger for better cache performance
   - Power-of-2 values for optimal performance

2. **Reserve capacity**: Use `reserve()` when the final size is known

3. **Use `emplace_back()`**: More efficient than `push_back()` for complex types

4. **Consider `erase_unsorted()`**: Much faster when element order doesn't matter

5. **Profile your use case**: Test different page sizes to find the optimal value

## Building and Usage

### Prerequisites

- CMake 3.14 or higher
- C++17 compatible compiler
- Git (for GoogleTest dependency)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/SergeyMakeev/chunked_vector.git
cd chunked_vector

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run tests
cd build
ctest --output-on-failure
```

### Usage

```
# Add the chunked_vector library
add_subdirectory("path_to_chunked_vector_repo/chunked_vector")

# use chunked_vector library
target_link_libraries(my_app_name PRIVATE chunked_vector)
```


## License

This project is licensed under the terms specified in the [LICENSE](LICENSE) file.
