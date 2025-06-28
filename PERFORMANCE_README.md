# Performance Comparison Tests: chunked_vector vs std::vector

This document describes the comprehensive performance comparison tests between `dod::chunked_vector` and `std::vector` using the ubench framework.

## Building and Running

### Prerequisites
- CMake 3.14 or higher
- C++17 compatible compiler
- The ubench.h header file (included in the repository)

### Building
```bash
mkdir build && cd build
cmake ..
make performance_test  # or use your build system
```

### Running Tests
```bash
./performance_test
```

### Command Line Options
- `--help` - Show help message
- `--filter=<pattern>` - Run only tests matching the pattern (e.g., `--filter=push_back`)
- `--list-benchmarks` - List all available benchmarks
- `--output=<file.csv>` - Output results to CSV file
- `--confidence=<percent>` - Set confidence interval threshold (default: 2.5%)

## Test Categories

### 1. Push Back Performance Tests
**Purpose**: Compare the cost of adding elements to the containers.

- `push_back_small` - Add 1,000 elements
- `push_back_medium` - Add 100,000 elements  
- `push_back_large` - Add 1,000,000 elements

**Expected Results**: 
- `std::vector` may be faster for small sizes due to contiguous memory
- `chunked_vector` should show more consistent performance across sizes due to avoiding large reallocations

### 2. Sequential Access Performance Tests
**Purpose**: Measure the cost of accessing elements in order.

- `sequential_access` - Sum all elements in a 100,000 element container

**Expected Results**:
- `std::vector` should be faster due to better cache locality
- `chunked_vector` performance depends on page size and cache behavior

### 3. Random Access Performance Tests
**Purpose**: Measure the cost of accessing elements randomly.

- `random_access` - Perform 10,000 random accesses on a 100,000 element container

**Expected Results**:
- Similar performance for both containers for true random access
- `chunked_vector` may have slightly higher overhead due to page/element index calculation

### 4. Iterator Performance Tests
**Purpose**: Compare iteration performance.

- `iterator_traversal` - Traverse using iterators
- `range_based_loop` - Traverse using range-based for loops

**Expected Results**:
- `std::vector` should be faster due to simpler pointer arithmetic
- `chunked_vector` has more complex iterator implementation

### 5. Memory Allocation Performance Tests
**Purpose**: Compare memory allocation strategies.

- `reserve_performance` - Pre-allocate space then fill 1,000,000 elements

**Expected Results**:
- `std::vector` does one large allocation
- `chunked_vector` allocates multiple pages - may be faster for very large sizes

### 6. Construction Performance Tests
**Purpose**: Compare different construction methods.

- `construct_with_size` - Create container with 100,000 elements and default value
- `construct_and_fill` - Create empty container, resize, then fill

**Expected Results**:
- Similar performance for most cases
- `chunked_vector` may avoid large memory allocations

### 7. Copy Performance Tests
**Purpose**: Compare copying overhead.

- `copy_constructor` - Copy construct a 100,000 element container
- `copy_assignment` - Copy assign a 100,000 element container

**Expected Results**:
- `std::vector` may be faster due to single memcpy operation
- `chunked_vector` needs to copy multiple pages

### 8. Resize Performance Tests
**Purpose**: Compare resizing operations.

- `resize_grow` - Grow from 1,000 to 100,000 elements
- `resize_shrink` - Shrink from 1,000,000 to 100,000 elements

**Expected Results**:
- `chunked_vector` should handle growing better (no large reallocations)
- Shrinking may be similar for both

### 9. Mixed Operations Performance Tests
**Purpose**: Simulate real-world usage patterns.

- `mixed_operations` - Combination of push_back, random access, and resize operations

**Expected Results**:
- Shows overall performance in realistic scenarios
- `chunked_vector` may perform better with less memory fragmentation

### 10. Large Object Performance Tests
**Purpose**: Test with objects that are expensive to copy/move.

- `large_objects` - Use 264-byte objects to test allocation and access patterns

**Expected Results**:
- Larger objects amplify the differences between containers
- `chunked_vector` may show better memory usage patterns

### 11. STL Algorithm Performance Tests
**Purpose**: Test compatibility and performance with standard algorithms.

- `std_algorithm_find` - Use std::find on 100,000 elements
- `std_algorithm_sort` - Use std::sort on 1,000 elements

**Expected Results**:
- Both containers should work equally well with STL algorithms
- Performance differences reflect underlying access patterns

### 12. Memory Pattern Tests
**Purpose**: Test specific memory access patterns relevant to chunked_vector.

- `page_boundary_access` - Access elements at page boundaries

**Expected Results**:
- `chunked_vector` design specifically optimized for this pattern
- May show advantages of the chunked approach

## Key Performance Characteristics

### std::vector Advantages
- **Cache Locality**: Contiguous memory layout provides excellent cache performance
- **Simple Access**: Direct pointer arithmetic for element access
- **Copy Performance**: Single memory block copy operations
- **Iterator Speed**: Simple pointer-based iterators

### chunked_vector Advantages
- **Memory Allocation**: Avoids large reallocations that can cause memory fragmentation
- **Stable Pointers**: Elements don't move when container grows (within same page)
- **Memory Usage**: Can deallocate unused pages, potentially using less memory
- **Large Container Handling**: Better suited for very large containers

### Trade-offs
- **Small Containers**: `std::vector` typically faster due to lower overhead
- **Large Containers**: `chunked_vector` may be faster due to allocation strategy
- **Access Patterns**: Sequential access favors `std::vector`, scattered access more similar
- **Memory Pressure**: `chunked_vector` may perform better under memory pressure

## Interpreting Results

### Reading ubench Output
```
[       OK ] push_back_medium.std_vector (mean 45.234ms, confidence interval +- 1.234%)
[       OK ] push_back_medium.chunked_vector (mean 52.567ms, confidence interval +- 2.001%)
```

- **Mean Time**: Average execution time
- **Confidence Interval**: Measure of result stability (lower is better)
- **Test Passes**: Confidence interval must be below threshold (default 2.5%)

### What to Look For
1. **Absolute Performance**: Which container is faster for your use case
2. **Scalability**: How performance changes with data size
3. **Consistency**: Lower confidence intervals indicate more predictable performance
4. **Use Case Relevance**: Focus on tests that match your application's usage patterns

## Customizing Tests

### Modifying Test Sizes
Edit the constants in `performance_test.cpp`:
```cpp
constexpr size_t SMALL_SIZE = 1000;     // Adjust for different test sizes
constexpr size_t MEDIUM_SIZE = 100000;
constexpr size_t LARGE_SIZE = 1000000;
```

### Adding New Tests
Follow the ubench pattern:
```cpp
UBENCH(test_category, test_name) {
    // Your test code here
    do_not_optimize(result);  // Prevent compiler optimization
}
```

### Running Specific Tests
```bash
./performance_test --filter=push_back    # Run only push_back tests
./performance_test --filter=std_vector   # Run only std::vector tests
./performance_test --filter=chunked      # Run only chunked_vector tests
```

## Contributing

When adding new performance tests:
1. Follow the existing naming conventions
2. Include both `std_vector` and `chunked_vector` variants
3. Use `do_not_optimize()` to prevent unwanted optimizations
4. Document the purpose and expected results
5. Test with different data sizes when relevant

## Notes

- Results may vary significantly between different systems, compilers, and compiler settings
- Always run tests in Release mode with optimizations enabled
- Consider running multiple times and averaging results for important decisions
- Memory pressure and system load can affect results
- The `--confidence` parameter can be adjusted for more/less strict requirements 