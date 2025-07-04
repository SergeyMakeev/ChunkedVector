# Chunked Vector Iterator Debugging

This document explains the iterator debugging feature in `chunked_vector` and how to use it effectively.

## Overview

The `chunked_vector` library includes iterator debugging support similar to Microsoft STL's `_ITERATOR_DEBUG_LEVEL`. This feature helps detect common iterator-related bugs during development by validating iterator usage and detecting when iterators become invalid.

## Configuration

Iterator debugging is controlled by the `CHUNKED_VEC_ITERATOR_DEBUG_LEVEL` macro:

- **Level 0** (Release): No debugging, minimal overhead
- **Level 1+** (Debug): Full iterator validation and tracking

### Setting the Debug Level

#### Option 1: Compiler Definitions
```bash
# GCC/Clang
g++ -DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=1 your_code.cpp

# MSVC
cl /DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=1 your_code.cpp
```

#### Option 2: CMake
```cmake
target_compile_definitions(your_target PRIVATE CHUNKED_VEC_ITERATOR_DEBUG_LEVEL=1)
```

#### Option 3: Before Including Header
```cpp
#define CHUNKED_VEC_ITERATOR_DEBUG_LEVEL 1
#include "chunked_vector/chunked_vector.h"
```

### Automatic Detection

If not explicitly set, the library automatically detects the debug level:
- Uses Microsoft's `_ITERATOR_DEBUG_LEVEL` if available
- Defaults to 0 (disabled) otherwise

## Features

### 1. Iterator Invalidation Detection

Iterator debugging tracks when iterators become invalid due to container modifications:

```cpp
chunked_vector<int> vec = {1, 2, 3, 4, 5};
auto it = vec.begin();
std::advance(it, 2);  // Points to element 3

vec.clear();          // Invalidates all iterators

// In debug mode, this would trigger an assertion:
// *it;  // ERROR: Iterator has been invalidated
```

### 2. Container Association Verification

Ensures iterators are used with their associated container:

```cpp
chunked_vector<int> vec1 = {1, 2, 3};
chunked_vector<int> vec2 = {4, 5, 6};

auto it1 = vec1.begin();
auto it2 = vec2.begin();

// In debug mode, this would trigger an assertion:
// vec1.erase(it2);  // ERROR: Iterator from different container
```

### 3. Range Validation

Validates iterator ranges in operations:

```cpp
chunked_vector<int> vec = {1, 2, 3, 4, 5};
auto first = vec.begin();
auto last = vec.begin();
std::advance(last, 3);

// Valid range operations work normally
vec.erase(first, last);  // OK

// Invalid ranges would trigger assertions in debug mode
// vec.erase(last, first);  // ERROR: Invalid range (last < first)
```

### 4. Generation Tracking

Each container maintains a generation counter that increments when structural changes occur:

```cpp
chunked_vector<int> vec = {1, 2, 3};
auto it = vec.begin();  // Generation 0

vec.push_back(4);       // Generation incremented to 1

// Iterator from generation 0 is now invalid
// Using 'it' would trigger an assertion in debug mode
```

## Operations That Invalidate Iterators

The following operations invalidate iterators:

### All Iterators
- `clear()`
- `operator=(const chunked_vector&)` (copy assignment)
- `operator=(chunked_vector&&)` (move assignment)
- `operator=(std::initializer_list<T>)`

### Iterators At/After Modification Point
- `pop_back()` (invalidates iterator to last element)
- `resize()` (invalidates iterators beyond new size)
- `erase()` (invalidates iterators at/after erase point)
- `erase_unsorted()` (invalidates iterator to erased element)

### Potentially All Iterators
- `push_back()` / `emplace_back()` (if page reallocation occurs)
- `reserve()` (if page array reallocation occurs)

## Testing Different Debug Levels

The project includes scripts to test iterator debugging at different levels:

### Windows
```cmd
test_iterator_debug_levels.cmd
```

### Unix/Linux/macOS
```bash
chmod +x test_iterator_debug_levels.sh
./test_iterator_debug_levels.sh
```

These scripts build and test the library with debug levels 0, 1, and 2.

## Performance Considerations

- **Debug Level 0**: No overhead, suitable for release builds
- **Debug Level 1+**: Small overhead due to generation tracking and validation
  - Each iterator stores an additional generation counter
  - Each container stores a generation counter
  - Validation checks on iterator operations

### Recommended Usage

```cpp
// Development/Debug builds
#ifdef _DEBUG
    #define CHUNKED_VEC_ITERATOR_DEBUG_LEVEL 1
#else
    #define CHUNKED_VEC_ITERATOR_DEBUG_LEVEL 0
#endif
```

## Example Usage Patterns

### Safe Iterator Usage
```cpp
chunked_vector<int> vec = {1, 2, 3, 4, 5};

// Safe: Use iterators before modifying container
for (auto it = vec.begin(); it != vec.end(); ++it) {
    std::cout << *it << " ";
}

// Safe: Get fresh iterators after modification
vec.push_back(6);
auto new_it = vec.begin();  // Fresh iterator
```

### Unsafe Patterns (Caught by Debug Mode)
```cpp
chunked_vector<int> vec = {1, 2, 3, 4, 5};
auto it = vec.begin();

vec.clear();        // Invalidates 'it'
// *it;             // ERROR: Using invalidated iterator

// Or:
auto first = vec.begin();
vec.push_back(6);   // May invalidate 'first'
// ++first;         // ERROR: Potentially invalidated iterator
```

## Integration with Existing Projects

The iterator debugging feature is designed to be:
- **Zero-overhead when disabled** (debug level 0)
- **Compatible with existing code** (no API changes)
- **Easy to enable/disable** (single macro)

Simply define `CHUNKED_VEC_ITERATOR_DEBUG_LEVEL=1` in debug builds and `=0` in release builds.

## Running Tests

To run the iterator debugging tests:

```bash
# Build the project
cmake --build build --config Debug

# Run only iterator debugging tests
build/Debug/chunked_vector_tests --gtest_filter="*IteratorDebug*"

# Run all tests
build/Debug/chunked_vector_tests
```

## Troubleshooting

### Common Assertion Messages

1. **"Iterator has been invalidated"**
   - The iterator was invalidated by a container modification
   - Get a fresh iterator after the modification

2. **"Iterator from different container"**
   - You're using an iterator with the wrong container
   - Ensure iterators match their source container

3. **"Invalid iterator range"**
   - The range [first, last) is invalid (first > last)
   - Check your iterator arithmetic

### Debugging Tips

1. Enable iterator debugging in development builds
2. Use address sanitizer (`-fsanitize=address`) for additional checks
3. Test with different optimization levels
4. Use static analysis tools to catch iterator issues

## Compatibility

- **C++ Standard**: C++17 or later
- **Compilers**: GCC, Clang, MSVC
- **Platforms**: Windows, Linux, macOS
- **Debug Levels**: Compatible with Microsoft STL debug levels 