#include "ubench.h"
#include "chunked_vector/chunked_vector.h"
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <chrono>

using namespace dod;

// Custom test object with non-trivial constructor/destructor for fair comparison
class TestObject {
private:
    int value_;
    static size_t construction_count_;
    static size_t destruction_count_;
    
public:
    TestObject() : value_(0) {
        ++construction_count_;
    }
    
    TestObject(int val) : value_(val) {
        ++construction_count_;
    }
    
    TestObject(size_t val) : value_(static_cast<int>(val)) {
        ++construction_count_;
    }
    
    TestObject(const TestObject& other) : value_(other.value_) {
        ++construction_count_;
    }
    
    TestObject& operator=(const TestObject& other) {
        if (this != &other) {
            value_ = other.value_;
        }
        return *this;
    }
    
    TestObject& operator=(int val) {
        value_ = val;
        return *this;
    }
    
    TestObject& operator=(size_t val) {
        value_ = static_cast<int>(val);
        return *this;
    }
    
    ~TestObject() {
        ++destruction_count_;
    }
    
    // Conversion operators and arithmetic for compatibility with tests
    operator int() const { return value_; }
    
    TestObject operator+(const TestObject& other) const {
        return TestObject(value_ + other.value_);
    }
    
    TestObject& operator+=(const TestObject& other) {
        value_ += other.value_;
        return *this;
    }
    
    // Volatile-compatible addition operator for benchmarking
    TestObject& operator+=(const TestObject& other) volatile {
        const_cast<TestObject*>(this)->value_ += other.value_;
        return const_cast<TestObject&>(*this);
    }
    
    bool operator==(const TestObject& other) const {
        return value_ == other.value_;
    }
    
    bool operator!=(const TestObject& other) const {
        return value_ != other.value_;
    }
    
    int get_value() const { return value_; }
    
    static size_t get_construction_count() { return construction_count_; }
    static size_t get_destruction_count() { return destruction_count_; }
    static void reset_counters() { construction_count_ = 0; destruction_count_ = 0; }
};

// Static member definitions
size_t TestObject::construction_count_ = 0;
size_t TestObject::destruction_count_ = 0;

// Test data sizes
constexpr size_t SMALL_SIZE = 1000;
constexpr size_t MEDIUM_SIZE = 100000;
constexpr size_t LARGE_SIZE = 1000000;

// Helper to prevent compiler optimizations
template<typename T>
void do_not_optimize(T&& value) {
    UBENCH_DO_NOTHING(const_cast<void*>(static_cast<const volatile void*>(&value)));
}

// =============================================================================
// Template Test Functions
// =============================================================================

template<typename Container>
void test_push_back(size_t size) {
    Container vec;
    for (size_t i = 0; i < size; ++i) {
        vec.push_back(typename Container::value_type(i));
    }
    do_not_optimize(vec);
}

template<typename Container>
void test_sequential_access() {
    Container vec;
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    volatile typename Container::value_type sum = typename Container::value_type(0);
    for (size_t i = 0; i < vec.size(); ++i) {
        sum += vec[i];
    }
    do_not_optimize(sum);
}

template<typename Container>
void test_random_access() {
    Container vec;
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    std::mt19937 gen(42);
    std::uniform_int_distribution<size_t> dis(0, vec.size() - 1);
    
    volatile typename Container::value_type sum = typename Container::value_type(0);
    for (int i = 0; i < 10000; ++i) {
        size_t idx = dis(gen);
        sum += vec[idx];
    }
    do_not_optimize(sum);
}

template<typename Container>
void test_iterator_traversal() {
    Container vec;
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    volatile typename Container::value_type sum = typename Container::value_type(0);
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        sum += *it;
    }
    do_not_optimize(sum);
}

template<typename Container>
void test_range_based_loop() {
    Container vec;
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    volatile typename Container::value_type sum = typename Container::value_type(0);
    for (const auto& value : vec) {
        sum += value;
    }
    do_not_optimize(sum);
}

template<typename Container>
void test_reserve_performance() {
    Container vec;
    vec.reserve(LARGE_SIZE);
    for (size_t i = 0; i < LARGE_SIZE; ++i) {
        vec.push_back(typename Container::value_type(i));
    }
    do_not_optimize(vec);
}

template<typename Container>
void test_construct_with_size() {
    Container vec(MEDIUM_SIZE, typename Container::value_type(42));
    do_not_optimize(vec);
}

template<typename Container>
void test_construct_and_fill() {
    Container vec;
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < vec.size(); ++i) {
        vec[i] = typename Container::value_type(i * 2);
    }
    do_not_optimize(vec);
}

template<typename Container>
void test_copy_constructor() {
    Container original;
    original.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        original[i] = typename Container::value_type(i);
    }
    
    Container copy(original);
    do_not_optimize(copy);
}

template<typename Container>
void test_copy_assignment() {
    Container original;
    original.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        original[i] = typename Container::value_type(i);
    }
    
    Container copy;
    copy = original;
    do_not_optimize(copy);
}

template<typename Container>
void test_resize_grow() {
    Container vec(SMALL_SIZE, typename Container::value_type(1));
    vec.resize(MEDIUM_SIZE, typename Container::value_type(2));
    do_not_optimize(vec);
}

template<typename Container>
void test_resize_shrink() {
    Container vec(LARGE_SIZE, typename Container::value_type(1));
    vec.resize(MEDIUM_SIZE);
    do_not_optimize(vec);
}

template<typename Container>
void test_mixed_operations() {
    Container vec;
    
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
        volatile typename Container::value_type sum = typename Container::value_type(0);
        for (int i = 0; i < 50; ++i) {
            sum += vec[dis(gen)];
        }
        
        // Remove some elements
        if (vec.size() > 50) {
            vec.resize(vec.size() - 50);
        }
    }
    
    do_not_optimize(vec);
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
};

template<typename Container>
void test_large_objects() {
    Container vec;
    for (int i = 0; i < 10000; ++i) {
        vec.emplace_back(i);
    }
    
    volatile int sum = 0;
    for (const auto& obj : vec) {
        sum += static_cast<int>(obj);
    }
    do_not_optimize(sum);
    do_not_optimize(vec);
}

template<typename Container>
void test_std_algorithm_find() {
    Container vec;
    vec.resize(MEDIUM_SIZE);
    for (size_t i = 0; i < MEDIUM_SIZE; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    volatile int found_count = 0;
    for (int target = 0; target < 1000; ++target) {
        auto it = std::find(vec.begin(), vec.end(), typename Container::value_type(target));
        if (it != vec.end()) {
            found_count++;
        }
    }
    do_not_optimize(found_count);
}

template<typename Container>
void test_std_algorithm_accumulate() {
    Container vec;
    vec.resize(SMALL_SIZE);
    for (size_t i = 0; i < SMALL_SIZE; ++i) {
        vec[i] = typename Container::value_type(i + 1);
    }
    
    volatile auto sum = std::accumulate(vec.begin(), vec.end(), typename Container::value_type(0));
    do_not_optimize(sum);
}

template<typename Container>
void test_page_boundary_access() {
    static constexpr size_t page_size = 1024; // Default page size for chunked_vector
    Container vec;
    vec.resize(page_size * 4);
    for (size_t i = 0; i < page_size * 4; ++i) {
        vec[i] = typename Container::value_type(i);
    }
    
    volatile typename Container::value_type sum = typename Container::value_type(0);
    // Access elements at page boundaries
    for (size_t i = 0; i < 4; ++i) {
        size_t page_start = i * page_size;
        size_t page_end = page_start + page_size - 1;
        
        for (size_t j = 0; j < 100; ++j) {
            sum += vec[page_start];
            sum += vec[page_end];
        }
    }
    do_not_optimize(sum);
}

// =============================================================================
// Benchmark Instantiations
// =============================================================================

// Push Back Performance Tests
UBENCH(push_back_small, std_vector) {
    test_push_back<std::vector<TestObject>>(SMALL_SIZE);
}

UBENCH(push_back_small, chunked_vector) {
    test_push_back<chunked_vector<TestObject>>(SMALL_SIZE);
}

UBENCH(push_back_medium, std_vector) {
    test_push_back<std::vector<TestObject>>(MEDIUM_SIZE);
}

UBENCH(push_back_medium, chunked_vector) {
    test_push_back<chunked_vector<TestObject>>(MEDIUM_SIZE);
}

UBENCH(push_back_large, std_vector) {
    test_push_back<std::vector<TestObject>>(LARGE_SIZE);
}

UBENCH(push_back_large, chunked_vector) {
    test_push_back<chunked_vector<TestObject>>(LARGE_SIZE);
}

// Sequential Access Performance Tests
UBENCH(sequential_access, std_vector) {
    test_sequential_access<std::vector<TestObject>>();
}

UBENCH(sequential_access, chunked_vector) {
    test_sequential_access<chunked_vector<TestObject>>();
}

// Random Access Performance Tests
UBENCH(random_access, std_vector) {
    test_random_access<std::vector<TestObject>>();
}

UBENCH(random_access, chunked_vector) {
    test_random_access<chunked_vector<TestObject>>();
}

// Iterator Performance Tests
UBENCH(iterator_traversal, std_vector) {
    test_iterator_traversal<std::vector<TestObject>>();
}

UBENCH(iterator_traversal, chunked_vector) {
    test_iterator_traversal<chunked_vector<TestObject>>();
}

UBENCH(range_based_loop, std_vector) {
    test_range_based_loop<std::vector<TestObject>>();
}

UBENCH(range_based_loop, chunked_vector) {
    test_range_based_loop<chunked_vector<TestObject>>();
}

// Memory Allocation Performance Tests
UBENCH(reserve_performance, std_vector) {
    test_reserve_performance<std::vector<TestObject>>();
}

UBENCH(reserve_performance, chunked_vector) {
    test_reserve_performance<chunked_vector<TestObject>>();
}

// Construction Performance Tests
UBENCH(construct_with_size, std_vector) {
    test_construct_with_size<std::vector<TestObject>>();
}

UBENCH(construct_with_size, chunked_vector) {
    test_construct_with_size<chunked_vector<TestObject>>();
}

UBENCH(construct_and_fill, std_vector) {
    test_construct_and_fill<std::vector<TestObject>>();
}

UBENCH(construct_and_fill, chunked_vector) {
    test_construct_and_fill<chunked_vector<TestObject>>();
}

// Copy Performance Tests
UBENCH(copy_constructor, std_vector) {
    test_copy_constructor<std::vector<TestObject>>();
}

UBENCH(copy_constructor, chunked_vector) {
    test_copy_constructor<chunked_vector<TestObject>>();
}

UBENCH(copy_assignment, std_vector) {
    test_copy_assignment<std::vector<TestObject>>();
}

UBENCH(copy_assignment, chunked_vector) {
    test_copy_assignment<chunked_vector<TestObject>>();
}

// Resize Performance Tests
UBENCH(resize_grow, std_vector) {
    test_resize_grow<std::vector<TestObject>>();
}

UBENCH(resize_grow, chunked_vector) {
    test_resize_grow<chunked_vector<TestObject>>();
}

UBENCH(resize_shrink, std_vector) {
    test_resize_shrink<std::vector<TestObject>>();
}

UBENCH(resize_shrink, chunked_vector) {
    test_resize_shrink<chunked_vector<TestObject>>();
}

// Mixed Operations Performance Tests
UBENCH(mixed_operations, std_vector) {
    test_mixed_operations<std::vector<TestObject>>();
}

UBENCH(mixed_operations, chunked_vector) {
    test_mixed_operations<chunked_vector<TestObject>>();
}

// Large Object Performance Tests
UBENCH(large_objects, std_vector) {
    test_large_objects<std::vector<LargeObject>>();
}

UBENCH(large_objects, chunked_vector) {
    test_large_objects<chunked_vector<LargeObject>>();
}

// STL Algorithm Performance Tests
UBENCH(std_algorithm_find, std_vector) {
    test_std_algorithm_find<std::vector<TestObject>>();
}

UBENCH(std_algorithm_find, chunked_vector) {
    test_std_algorithm_find<chunked_vector<TestObject>>();
}

UBENCH(std_algorithm_accumulate, std_vector) {
    test_std_algorithm_accumulate<std::vector<TestObject>>();
}

UBENCH(std_algorithm_accumulate, chunked_vector) {
    test_std_algorithm_accumulate<chunked_vector<TestObject>>();
}

// Memory Pattern Tests
UBENCH(page_boundary_access, std_vector) {
    test_page_boundary_access<std::vector<TestObject>>();
}

UBENCH(page_boundary_access, chunked_vector) {
    test_page_boundary_access<chunked_vector<TestObject>>();
}

UBENCH_MAIN(); 