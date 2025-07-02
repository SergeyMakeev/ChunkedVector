#pragma once

#include "chunked_vector/chunked_vector.h"
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <chrono>

using namespace dod;

// Macro to suppress unused variable warnings in benchmark functions
#define UNUSED(x) (void)(x)

// Test data sizes
constexpr size_t SMALL_SIZE = 1000;
constexpr size_t MEDIUM_SIZE = 100000;
constexpr size_t LARGE_SIZE = 1000000;

// Custom test object with non-trivial constructor/destructor for fair comparison
struct TestObject {
    int value;
    static int constructor_calls;
    static int destructor_calls;
    static int copy_calls;
    static int move_calls;

    TestObject() : value(0) {
        ++constructor_calls;
    }
    
    explicit TestObject(int v) : value(v) {
        ++constructor_calls;
    }
    
    TestObject(size_t val) : value(static_cast<int>(val)) {
        ++constructor_calls;
    }
    
    TestObject(const TestObject& other) : value(other.value) {
        ++copy_calls;
    }
    
    TestObject(TestObject&& other) noexcept : value(other.value) {
        other.value = -1;
        ++move_calls;
    }
    
    TestObject& operator=(const TestObject& other) {
        if (this != &other) {
            value = other.value;
            ++copy_calls;
        }
        return *this;
    }
    
    TestObject& operator=(TestObject&& other) noexcept {
        if (this != &other) {
            value = other.value;
            other.value = -1;
            ++move_calls;
        }
        return *this;
    }
    
    TestObject& operator=(int val) {
        value = val;
        return *this;
    }
    
    TestObject& operator=(size_t val) {
        value = static_cast<int>(val);
        return *this;
    }
    
    ~TestObject() { 
        ++destructor_calls; 
    }
    
    // Conversion operators and arithmetic for compatibility with tests
    operator int() const { return value; }
    
    TestObject operator+(const TestObject& other) const {
        return TestObject(value + other.value);
    }
    
    TestObject& operator+=(const TestObject& other) {
        value += other.value;
        return *this;
    }
    
    // Volatile-compatible addition operator for benchmarking
    TestObject& operator+=(const TestObject& other) volatile {
        const_cast<TestObject*>(this)->value += other.value;
        return const_cast<TestObject&>(*this);
    }
    
    bool operator==(const TestObject& other) const {
        return value == other.value;
    }
    
    bool operator!=(const TestObject& other) const {
        return value != other.value;
    }
    
    bool operator<(const TestObject& other) const {
        return value < other.value;
    }
    
    int get_value() const { return value; }
};

// Static member definitions
int TestObject::constructor_calls = 0;
int TestObject::destructor_calls = 0;
int TestObject::copy_calls = 0;
int TestObject::move_calls = 0;

// =============================================================================
// Template Test Functions
// =============================================================================

template<typename Container>
void test_push_back(Container& vec, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        vec.push_back(typename Container::value_type(i));
    }
}

template<typename Container>
void test_sequential_access(Container& vec) {
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    typename Container::value_type sum = typename Container::value_type(0);
    for (size_t i = 0; i < vec.size(); ++i) {
        sum += vec[i];
    }
    UNUSED(sum);
}

template<typename Container>
void test_random_access(Container& vec) {
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    std::mt19937 gen(42);
    std::uniform_int_distribution<size_t> dis(0, vec.size() - 1);
    
    typename Container::value_type sum = typename Container::value_type(0);
    for (int i = 0; i < 10000; ++i) {
        size_t idx = dis(gen);
        sum += vec[idx];
    }
    UNUSED(sum);
}

template<typename Container>
void test_iterator_traversal(Container& vec) {
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    typename Container::value_type sum = typename Container::value_type(0);
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        sum += *it;
    }
    UNUSED(sum);
}

template<typename Container>
void test_range_based_loop(Container& vec) {
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    typename Container::value_type sum = typename Container::value_type(0);
    for (const auto& value : vec) {
        sum += value;
    }
    UNUSED(sum);
}

template<typename Container>
void test_reserve_performance(Container& vec) {
    vec.reserve(LARGE_SIZE);
    for (size_t i = 0; i < LARGE_SIZE; ++i) {
        vec.push_back(typename Container::value_type(i));
    }
}

template<typename Container>
void test_construct_with_size(Container& vec, size_t size, const typename Container::value_type& value) {
    vec = Container(size, value);
}

template<typename Container>
void test_construct_and_fill(Container& vec) {
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] = typename Container::value_type(i * 2);
    }
}

template<typename Container>
void test_copy_constructor(Container& vec, const Container& source) {
    vec = Container(source);
}

template<typename Container>
void test_copy_assignment(Container& vec, const Container& source) {
    vec = source;
}

template<typename Container>
void test_resize_grow(Container& vec) {
    vec = Container(SMALL_SIZE, typename Container::value_type(1));
    vec.resize(MEDIUM_SIZE, typename Container::value_type(2));
}

template<typename Container>
void test_resize_shrink(Container& vec) {
    vec = Container(LARGE_SIZE, typename Container::value_type(1));
    vec.resize(MEDIUM_SIZE);
}

template<typename Container>
void test_mixed_operations(Container& vec) {
    // Fill with initial data
    for (size_t i = 0; i < SMALL_SIZE; ++i) {
        vec.push_back(typename Container::value_type(i));
    }
    
    // Mix of operations
    for (int round = 0; round < 10; ++round) {
        // Add elements
        for (size_t i = 0; i < 100; ++i) {
            vec.push_back(typename Container::value_type(i + round * 100));
        }
        
        // Access elements randomly
        std::mt19937 gen(42 + round);
        std::uniform_int_distribution<size_t> dis(0, vec.size() - 1);
        typename Container::value_type sum = typename Container::value_type(0);
        for (int i = 0; i < 50; ++i) {
            sum += vec[dis(gen)];
        }
        UNUSED(sum);
        
        // Remove some elements
        if (vec.size() > 50) {
            vec.resize(vec.size() - 50);
        }
    }
}

// Large Object for testing
struct LargeObject {
    char data[256];
    int id;
    double value;
    
    LargeObject() : id(0), value(0.0) {
        std::fill(std::begin(data), std::end(data), 0);
    }
    
    explicit LargeObject(int i) : id(i), value(i * 1.5) {
        std::fill(std::begin(data), std::end(data), static_cast<char>(i % 256));
    }
    
    // Assignment operator for compatibility
    LargeObject& operator=(int i) {
        id = i;
        value = i * 1.5;
        std::fill(std::begin(data), std::end(data), static_cast<char>(i % 256));
        return *this;
    }
    
    // Addition operator for sum operations
    LargeObject operator+(const LargeObject& other) const {
        LargeObject result;
        result.id = id + other.id;
        result.value = value + other.value;
        return result;
    }
    
    // Conversion to int for sum operations
    operator int() const { return id; }
    
    bool operator==(const LargeObject& other) const {
        return id == other.id && value == other.value;
    }
};

template<typename Container>
void test_large_objects(Container& vec) {
    for (int i = 0; i < 10000; ++i) {
        vec.emplace_back(i);
    }
    
    int sum = 0;
    for (const auto& obj : vec) {
        sum += static_cast<int>(obj);
    }
    UNUSED(sum);
}

template<typename Container>
void test_std_algorithm_find(Container& vec) {
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    int found_count = 0;
    for (int target = 0; target < 1000; ++target) {
        auto it = std::find(vec.begin(), vec.end(), typename Container::value_type(target));
        if (it != vec.end()) {
            found_count++;
        }
    }
    UNUSED(found_count);
}

template<typename Container>
void test_std_algorithm_accumulate(Container& vec) {
    vec.resize(SMALL_SIZE);
    for (size_t i = 0; i < SMALL_SIZE; ++i) {
        vec[i] = typename Container::value_type(i + 1);
    }
    
    auto sum = std::accumulate(vec.begin(), vec.end(), typename Container::value_type(0));
    UNUSED(sum);
}

template<typename Container>
void test_page_boundary_access(Container& vec) {
    static constexpr size_t page_size = 1024; // Default page size for chunked_vector
    vec.resize(page_size * 4);
    for (size_t i = 0; i < page_size * 4; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    typename Container::value_type sum = typename Container::value_type(0);
    // Access elements at page boundaries
    for (size_t i = 0; i < 4; ++i) {
        size_t page_start = i * page_size;
        size_t page_end = page_start + page_size - 1;
        
        for (size_t j = 0; j < 100; ++j) {
            sum += vec[page_start];
            sum += vec[page_end];
        }
    }
    UNUSED(sum);
}

// Helper function to compare two containers element by element
template<typename Container1, typename Container2>
bool containers_equal(const Container1& c1, const Container2& c2) {
    if (c1.size() != c2.size()) {
        return false;
    }
    
    for (size_t i = 0; i < c1.size(); ++i) {
        if (!(c1[i] == c2[i])) {
            return false;
        }
    }
    
    return true;
}

// Helper function to compare containers using iterators
template<typename Container1, typename Container2>
bool containers_equal_iterators(const Container1& c1, const Container2& c2) {
    if (c1.size() != c2.size()) {
        return false;
    }
    
    auto it1 = c1.begin();
    auto it2 = c2.begin();
    
    while (it1 != c1.end() && it2 != c2.end()) {
        if (!(*it1 == *it2)) {
            return false;
        }
        ++it1;
        ++it2;
    }
    
    return it1 == c1.end() && it2 == c2.end();
} 