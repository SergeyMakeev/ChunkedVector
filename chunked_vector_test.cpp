#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "chunked_vector/chunked_vector.h"
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace dod;

// Test fixture for basic types
class ChunkedVectorTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test fixture for custom types
struct TestObject {
    int value;
    static int constructor_calls;
    static int destructor_calls;
    static int copy_calls;
    static int move_calls;

    TestObject() : value(0) { ++constructor_calls; }
    explicit TestObject(int v) : value(v) { ++constructor_calls; }
    TestObject(const TestObject& other) : value(other.value) { ++copy_calls; }
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
    ~TestObject() { ++destructor_calls; }
    
    bool operator==(const TestObject& other) const { return value == other.value; }
    bool operator!=(const TestObject& other) const { return value != other.value; }
    bool operator<(const TestObject& other) const { return value < other.value; }
};

int TestObject::constructor_calls = 0;
int TestObject::destructor_calls = 0;
int TestObject::copy_calls = 0;
int TestObject::move_calls = 0;

class ChunkedVectorCustomTypeTest : public ::testing::Test {
protected:
    void SetUp() override {
        TestObject::constructor_calls = 0;
        TestObject::destructor_calls = 0;
        TestObject::copy_calls = 0;
        TestObject::move_calls = 0;
    }
    void TearDown() override {}
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(ChunkedVectorTest, DefaultConstructor) {
    chunked_vector<int> vec;
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 0);
}

TEST_F(ChunkedVectorTest, CountConstructor) {
    chunked_vector<int> vec(5);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), 5);
    EXPECT_GE(vec.capacity(), 5);
    
    for (size_t i = 0; i < vec.size(); ++i) {
        EXPECT_EQ(vec[i], 0);  // Default constructed
    }
}

TEST_F(ChunkedVectorTest, CountValueConstructor) {
    chunked_vector<int> vec(5, 42);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), 5);
    EXPECT_GE(vec.capacity(), 5);
    
    for (size_t i = 0; i < vec.size(); ++i) {
        EXPECT_EQ(vec[i], 42);
    }
}

TEST_F(ChunkedVectorTest, CopyConstructor) {
    chunked_vector<int> original;
    for (int i = 0; i < 5; ++i) {
        original.push_back(i + 1);
    }
    
    chunked_vector<int> copy(original);
    EXPECT_EQ(copy.size(), original.size());
    
    for (size_t i = 0; i < original.size(); ++i) {
        EXPECT_EQ(copy[i], original[i]);
    }
}

TEST_F(ChunkedVectorTest, MoveConstructor) {
    chunked_vector<int> original;
    for (int i = 0; i < 5; ++i) {
        original.push_back(i);
    }
    size_t original_size = original.size();
    
    chunked_vector<int> moved(std::move(original));
    EXPECT_EQ(moved.size(), original_size);
    EXPECT_TRUE(original.empty());  // Original should be empty after move
    
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(moved[i], i);
    }
}

TEST_F(ChunkedVectorTest, InitializerListConstructor) {
    // Test with empty initializer list
    chunked_vector<int> empty_vec{};
    EXPECT_TRUE(empty_vec.empty());
    EXPECT_EQ(empty_vec.size(), 0);
    
    // Test with single element
    chunked_vector<int> single_vec{42};
    EXPECT_EQ(single_vec.size(), 1);
    EXPECT_EQ(single_vec[0], 42);
    
    // Test with multiple elements
    chunked_vector<int> multi_vec{1, 2, 3, 4, 5};
    EXPECT_EQ(multi_vec.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(multi_vec[i], i + 1);
    }
    
    // Test with string type
    chunked_vector<std::string> string_vec{"hello", "world", "test"};
    EXPECT_EQ(string_vec.size(), 3);
    EXPECT_EQ(string_vec[0], "hello");
    EXPECT_EQ(string_vec[1], "world");
    EXPECT_EQ(string_vec[2], "test");
    
    // Test with many elements (spanning multiple pages)
    chunked_vector<int, 4> large_vec{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    EXPECT_EQ(large_vec.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(large_vec[i], i + 1);
    }
}

// ============================================================================
// Assignment Tests
// ============================================================================

TEST_F(ChunkedVectorTest, CopyAssignment) {
    chunked_vector<int> original;
    for (int i = 0; i < 5; ++i) {
        original.push_back(i);
    }
    
    chunked_vector<int> copy;
    copy.push_back(999);  // Make sure copy isn't empty
    copy = original;
    
    EXPECT_EQ(copy.size(), original.size());
    for (size_t i = 0; i < original.size(); ++i) {
        EXPECT_EQ(copy[i], original[i]);
    }
}

TEST_F(ChunkedVectorTest, MoveAssignment) {
    chunked_vector<int> original;
    for (int i = 0; i < 5; ++i) {
        original.push_back(i);
    }
    size_t original_size = original.size();
    
    chunked_vector<int> moved;
    moved.push_back(999);  // Make sure moved isn't empty
    moved = std::move(original);
    
    EXPECT_EQ(moved.size(), original_size);
    EXPECT_TRUE(original.empty());
    
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(moved[i], i);
    }
}

TEST_F(ChunkedVectorTest, SelfAssignment) {
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    
    vec = vec;  // Self assignment
    EXPECT_EQ(vec.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(vec[i], i);
    }
}

TEST_F(ChunkedVectorTest, InitializerListAssignment) {
    chunked_vector<int> vec;
    
    // Initial values
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i * 10);
    }
    EXPECT_EQ(vec.size(), 5);
    
    // Assign empty initializer list
    vec = {};
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    
    // Assign single element
    vec = {42};
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 42);
    
    // Assign multiple elements
    vec = {10, 20, 30, 40, 50};
    EXPECT_EQ(vec.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(vec[i], (i + 1) * 10);
    }
    
    // Assign to larger size (growing)
    vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    EXPECT_EQ(vec.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(vec[i], i + 1);
    }
    
    // Assign to smaller size (shrinking)
    vec = {100, 200};
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], 100);
    EXPECT_EQ(vec[1], 200);
    
    // Test with string type
    chunked_vector<std::string> string_vec{"old", "values"};
    string_vec = {"new", "string", "values", "here"};
    EXPECT_EQ(string_vec.size(), 4);
    EXPECT_EQ(string_vec[0], "new");
    EXPECT_EQ(string_vec[1], "string");
    EXPECT_EQ(string_vec[2], "values");
    EXPECT_EQ(string_vec[3], "here");
}

// ============================================================================
// Element Access Tests
// ============================================================================

TEST_F(ChunkedVectorTest, IndexOperator) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 2);
    }
    
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(vec[i], i * 2);
    }
    
    // Test modification
    vec[5] = 999;
    EXPECT_EQ(vec[5], 999);
}

TEST_F(ChunkedVectorTest, IndexOperatorConst) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 2);
    }
    
    const auto& const_vec = vec;
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(const_vec[i], i * 2);
    }
}

TEST_F(ChunkedVectorTest, AtMethod) {
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(vec.at(i), i);
    }
    
    EXPECT_THROW(vec.at(5), std::out_of_range);
    EXPECT_THROW(vec.at(100), std::out_of_range);
}

TEST_F(ChunkedVectorTest, AtMethodConst) {
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    
    const auto& const_vec = vec;
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(const_vec.at(i), i);
    }
    
    EXPECT_THROW(const_vec.at(5), std::out_of_range);
}

TEST_F(ChunkedVectorTest, FrontAndBack) {
    chunked_vector<int> vec;
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);
    
    EXPECT_EQ(vec.front(), 10);
    EXPECT_EQ(vec.back(), 30);
    
    vec.front() = 100;
    vec.back() = 300;
    
    EXPECT_EQ(vec.front(), 100);
    EXPECT_EQ(vec.back(), 300);
}

TEST_F(ChunkedVectorTest, FrontAndBackConst) {
    chunked_vector<int> vec;
    vec.push_back(10);
    vec.push_back(20);
    vec.push_back(30);
    
    const auto& const_vec = vec;
    EXPECT_EQ(const_vec.front(), 10);
    EXPECT_EQ(const_vec.back(), 30);
}

// ============================================================================
// Iterator Tests
// ============================================================================

TEST_F(ChunkedVectorTest, BasicIterators) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    int expected = 0;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        EXPECT_EQ(*it, expected++);
    }
    EXPECT_EQ(expected, 10);
}

TEST_F(ChunkedVectorTest, ConstIterators) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    const auto& const_vec = vec;
    int expected = 0;
    for (auto it = const_vec.begin(); it != const_vec.end(); ++it) {
        EXPECT_EQ(*it, expected++);
    }
    
    expected = 0;
    for (auto it = const_vec.cbegin(); it != const_vec.cend(); ++it) {
        EXPECT_EQ(*it, expected++);
    }
}

TEST_F(ChunkedVectorTest, IteratorArithmetic) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 10);
    }
    
    auto it = vec.begin();
    EXPECT_EQ(*it, 0);
    
    it += 3;
    EXPECT_EQ(*it, 30);
    
    it -= 1;
    EXPECT_EQ(*it, 20);
    
    auto it2 = it + 2;
    EXPECT_EQ(*it2, 40);
    
    auto it3 = it2 - 1;
    EXPECT_EQ(*it3, 30);
    
    EXPECT_EQ(it2 - it, 2);
}

TEST_F(ChunkedVectorTest, IteratorComparisons) {
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    
    auto it1 = vec.begin();
    auto it2 = vec.begin() + 1;
    auto it3 = vec.end();
    
    EXPECT_TRUE(it1 == vec.begin());
    EXPECT_TRUE(it1 != it2);
    EXPECT_TRUE(it1 < it2);
    EXPECT_TRUE(it1 <= it2);
    EXPECT_TRUE(it2 > it1);
    EXPECT_TRUE(it2 >= it1);
    EXPECT_TRUE(it2 < it3);
}

TEST_F(ChunkedVectorTest, IteratorBracketOperator) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 2);
    }
    
    auto it = vec.begin() + 3;
    EXPECT_EQ(it[0], 6);
    EXPECT_EQ(it[1], 8);
    EXPECT_EQ(it[2], 10);
    EXPECT_EQ(it[-1], 4);
}

// ============================================================================
// Capacity Tests
// ============================================================================

TEST_F(ChunkedVectorTest, EmptyAndSize) {
    chunked_vector<int> vec;
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    
    vec.push_back(1);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), 1);
    
    vec.push_back(2);
    EXPECT_EQ(vec.size(), 2);
    
    vec.pop_back();
    EXPECT_EQ(vec.size(), 1);
    
    vec.clear();
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
}

TEST_F(ChunkedVectorTest, Capacity) {
    chunked_vector<int> vec;
    EXPECT_EQ(vec.capacity(), 0);
    
    vec.push_back(1);
    EXPECT_GE(vec.capacity(), 1);
    
    size_t initial_capacity = vec.capacity();
    size_t original_size = vec.size();
    
    // Add elements up to but not exceeding the current capacity
    for (size_t i = original_size; i < initial_capacity && i < original_size + 10; ++i) {
        vec.push_back(static_cast<int>(i));
    }
    // Should still be within initial capacity (or it may have grown if we exceeded it)
    EXPECT_GE(vec.capacity(), initial_capacity);
}

TEST_F(ChunkedVectorTest, Reserve) {
    chunked_vector<int> vec;
    vec.reserve(100);
    EXPECT_GE(vec.capacity(), 100);
    EXPECT_EQ(vec.size(), 0);
    
    // Adding elements shouldn't change capacity if within reserved space
    size_t reserved_capacity = vec.capacity();
    for (int i = 0; i < 50; ++i) {
        vec.push_back(i);
    }
    EXPECT_EQ(vec.capacity(), reserved_capacity);
    
    // Reserve with smaller size shouldn't change capacity
    vec.reserve(50);
    EXPECT_EQ(vec.capacity(), reserved_capacity);
}

TEST_F(ChunkedVectorTest, ShrinkToFit) {
    chunked_vector<int> vec;
    vec.reserve(chunked_vector<int>::page_size() * 2); // Reserve multiple pages
    
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    size_t capacity_before = vec.capacity();
    EXPECT_GE(capacity_before, chunked_vector<int>::page_size() * 2);
    
    vec.shrink_to_fit();
    // After shrink_to_fit, capacity should be at least the size, but may not be dramatically smaller
    // due to page-based allocation. The important thing is it shouldn't be larger than before.
    EXPECT_LE(vec.capacity(), capacity_before);
    EXPECT_GE(vec.capacity(), vec.size());
    
    // Elements should still be accessible
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(vec[i], i);
    }
}

// ============================================================================
// Modifier Tests
// ============================================================================

TEST_F(ChunkedVectorTest, Clear) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    EXPECT_EQ(vec.size(), 10);
    vec.clear();
    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());
    
    // Should be able to add elements after clear
    vec.push_back(42);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 42);
}

TEST_F(ChunkedVectorTest, PushBack) {
    chunked_vector<int> vec;
    
    // Test copy push_back
    int value = 42;
    vec.push_back(value);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 42);
    
    // Test move push_back
    vec.push_back(std::move(value));
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[1], 42);
}

TEST_F(ChunkedVectorTest, EmplaceBack) {
    chunked_vector<std::string> vec;
    
    vec.emplace_back("Hello");
    vec.emplace_back("World");
    vec.emplace_back(5, 'x');  // string(5, 'x')
    
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], "Hello");
    EXPECT_EQ(vec[1], "World");
    EXPECT_EQ(vec[2], "xxxxx");
}

TEST_F(ChunkedVectorTest, PopBack) {
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec.back(), 4);
    
    vec.pop_back();
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(vec.back(), 3);
    
    vec.pop_back();
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec.back(), 2);
}

TEST_F(ChunkedVectorTest, ResizeGrow) {
    chunked_vector<int> vec;
    vec.resize(5);
    
    EXPECT_EQ(vec.size(), 5);
    for (size_t i = 0; i < vec.size(); ++i) {
        EXPECT_EQ(vec[i], 0);  // Default constructed
    }
}

TEST_F(ChunkedVectorTest, ResizeGrowWithValue) {
    chunked_vector<int> vec;
    vec.resize(5, 42);
    
    EXPECT_EQ(vec.size(), 5);
    for (size_t i = 0; i < vec.size(); ++i) {
        EXPECT_EQ(vec[i], 42);
    }
}

TEST_F(ChunkedVectorTest, ResizeShrink) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    vec.resize(5);
    EXPECT_EQ(vec.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(vec[i], i);
    }
}

// ============================================================================
// Page Size and Large Container Tests
// ============================================================================

TEST_F(ChunkedVectorTest, PageSize) {
    chunked_vector<int, 16> vec;
    EXPECT_EQ(vec.page_size(), 16);
    
    chunked_vector<int, 1024> vec2;
    EXPECT_EQ(vec2.page_size(), 1024);
}

TEST_F(ChunkedVectorTest, SpanMultiplePages) {
    constexpr size_t PAGE_SIZE = 8;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Add elements spanning multiple pages
    const int count = PAGE_SIZE * 3 + 5;  // 3 full pages + 5 elements
    for (int i = 0; i < count; ++i) {
        vec.push_back(i);
    }
    
    EXPECT_EQ(vec.size(), count);
    EXPECT_GE(vec.capacity(), count);
    
    // Verify all elements are correct
    for (int i = 0; i < count; ++i) {
        EXPECT_EQ(vec[i], i);
    }
}

TEST_F(ChunkedVectorTest, PageBoundaryOperations) {
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Fill exactly one page
    for (int i = 0; i < PAGE_SIZE; ++i) {
        vec.push_back(i);
    }
    
    EXPECT_EQ(vec.size(), PAGE_SIZE);
    
    // Add one more element (should trigger new page allocation)
    vec.push_back(PAGE_SIZE);
    EXPECT_EQ(vec.size(), PAGE_SIZE + 1);
    EXPECT_EQ(vec[PAGE_SIZE], PAGE_SIZE);
    
    // Remove elements back to page boundary
    vec.pop_back();
    EXPECT_EQ(vec.size(), PAGE_SIZE);
    
    for (size_t i = 0; i < PAGE_SIZE; ++i) {
        EXPECT_EQ(vec[i], static_cast<int>(i));
    }
}

// ============================================================================
// Custom Type Tests
// ============================================================================

TEST_F(ChunkedVectorCustomTypeTest, CustomTypeConstructorDestructor) {
    {
        chunked_vector<TestObject> vec;
        vec.emplace_back(1);
        vec.emplace_back(2);
        vec.emplace_back(3);
        
        EXPECT_EQ(TestObject::constructor_calls, 3);
        EXPECT_EQ(TestObject::destructor_calls, 0);
    }
    // Destructor should be called when vector goes out of scope
    EXPECT_EQ(TestObject::destructor_calls, 3);
}

TEST_F(ChunkedVectorCustomTypeTest, CustomTypeCopyMove) {
    chunked_vector<TestObject> vec;
    
    TestObject obj1(42);
    vec.push_back(obj1);  // Copy
    EXPECT_GT(TestObject::copy_calls, 0);
    
    vec.push_back(TestObject(43));  // Move
    EXPECT_GT(TestObject::move_calls, 0);
    
    EXPECT_EQ(vec[0].value, 42);
    EXPECT_EQ(vec[1].value, 43);
}

TEST_F(ChunkedVectorCustomTypeTest, InitializerListCustomType) {
    {
        // Test initializer_list constructor with custom type
        chunked_vector<TestObject> vec{TestObject(10), TestObject(20), TestObject(30)};
        
        EXPECT_EQ(vec.size(), 3);
        EXPECT_EQ(vec[0].value, 10);
        EXPECT_EQ(vec[1].value, 20);
        EXPECT_EQ(vec[2].value, 30);
        
        // Check constructor calls (3 for initializer list + moves/copies for insertion)
        EXPECT_GT(TestObject::constructor_calls, 0);
        
        // Test initializer_list assignment
        vec = {TestObject(100), TestObject(200)};
        EXPECT_EQ(vec.size(), 2);
        EXPECT_EQ(vec[0].value, 100);
        EXPECT_EQ(vec[1].value, 200);
        
        // Test empty initializer list assignment
        vec = {};
        EXPECT_TRUE(vec.empty());
        EXPECT_EQ(vec.size(), 0);
    }
    
    // After scope, all objects should be destroyed
    // Note: The exact counts depend on optimization and move semantics,
    // but destructor calls should match constructor calls eventually
    EXPECT_GT(TestObject::destructor_calls, 0);
}

// ============================================================================
// Range-based loops and STL algorithm compatibility
// ============================================================================

TEST_F(ChunkedVectorTest, RangeBasedLoop) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 2);
    }
    
    int expected = 0;
    for (const auto& value : vec) {
        EXPECT_EQ(value, expected * 2);
        ++expected;
    }
    EXPECT_EQ(expected, 10);
}

TEST_F(ChunkedVectorTest, STLAlgorithmCompatibility) {
    chunked_vector<int> vec;
    for (int i = 1; i <= 10; ++i) {
        vec.push_back(i);
    }
    
    // Test std::find
    auto it = std::find(vec.begin(), vec.end(), 5);
    EXPECT_NE(it, vec.end());
    EXPECT_EQ(*it, 5);
    
    // Test std::accumulate
    int sum = std::accumulate(vec.begin(), vec.end(), 0);
    EXPECT_EQ(sum, 55);  // 1+2+...+10 = 55
    
    // Test std::sort
    std::reverse(vec.begin(), vec.end());
    EXPECT_EQ(vec[0], 10);
    EXPECT_EQ(vec[9], 1);
    
    std::sort(vec.begin(), vec.end());
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(vec[i], i + 1);
    }
}

// ============================================================================
// Edge Cases and Exception Safety
// ============================================================================

TEST_F(ChunkedVectorTest, EmptyVectorOperations) {
    chunked_vector<int> vec;
    
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.begin(), vec.end());
    EXPECT_EQ(vec.cbegin(), vec.cend());
}

TEST_F(ChunkedVectorTest, SingleElementOperations) {
    chunked_vector<int> vec;
    vec.push_back(42);
    
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec.front(), 42);
    EXPECT_EQ(vec.back(), 42);
    EXPECT_EQ(vec[0], 42);
    EXPECT_EQ(vec.at(0), 42);
    
    auto it = vec.begin();
    EXPECT_EQ(*it, 42);
    ++it;
    EXPECT_EQ(it, vec.end());
}

// ============================================================================
// Additional Coverage Tests
// ============================================================================

TEST_F(ChunkedVectorTest, EmptyFunctionComprehensive) {
    chunked_vector<int> vec;
    
    // Initially empty
    EXPECT_TRUE(vec.empty());
    
    // Add single element - no longer empty
    vec.push_back(42);
    EXPECT_FALSE(vec.empty());
    
    // Add more elements - still not empty
    vec.push_back(43);
    vec.push_back(44);
    EXPECT_FALSE(vec.empty());
    
    // Remove all but one - still not empty
    vec.pop_back();
    vec.pop_back();
    EXPECT_FALSE(vec.empty());
    
    // Remove last element - now empty
    vec.pop_back();
    EXPECT_TRUE(vec.empty());
    
    // Add element again - not empty
    vec.push_back(100);
    EXPECT_FALSE(vec.empty());
    
    // Clear - empty again
    vec.clear();
    EXPECT_TRUE(vec.empty());
    
    // Resize to non-zero - not empty
    vec.resize(5);
    EXPECT_FALSE(vec.empty());
    
    // Resize to zero - empty again
    vec.resize(0);
    EXPECT_TRUE(vec.empty());
}

TEST_F(ChunkedVectorTest, ResizeSmallerEdgeCases) {
    chunked_vector<int, 4> vec;  // Small page size for testing
    
    // Fill multiple pages
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 10);
    }
    EXPECT_EQ(vec.size(), 10);
    
    // Resize to exactly one page boundary
    vec.resize(4);
    EXPECT_EQ(vec.size(), 4);
    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(vec[i], i * 10);
    }
    
    // Resize to middle of a page
    vec.resize(2);
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[1], 10);
    
    // Resize to one element
    vec.resize(1);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 0);
    
    // Resize to zero
    vec.resize(0);
    EXPECT_EQ(vec.size(), 0);
    EXPECT_TRUE(vec.empty());
    
    // Resize from zero to non-zero
    vec.resize(3, 999);
    EXPECT_EQ(vec.size(), 3);
    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(vec[i], 999);
    }
    
    // Resize smaller with value parameter (should not use the value)
    vec.resize(1, 888);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 999);  // Original value preserved
}

TEST_F(ChunkedVectorTest, EnsurePageCapacityEarlyExit) {
    chunked_vector<int, 8> vec;  // Use smaller page size for predictable behavior
    
    // Reserve initial capacity
    vec.reserve(16);  // This should allocate 2 pages (16/8 = 2)
    size_t initial_capacity = vec.capacity();
    EXPECT_GE(initial_capacity, 16);
    
    // Add some elements but stay within capacity
    for (int i = 0; i < 8; ++i) {
        vec.push_back(i);
    }
    
    // Reserve same or smaller capacity - should trigger early exit in ensure_page_capacity
    vec.reserve(16);
    EXPECT_EQ(vec.capacity(), initial_capacity);  // Capacity should not change
    
    vec.reserve(8);   // Smaller than current capacity
    EXPECT_EQ(vec.capacity(), initial_capacity);  // Capacity should not change
    
    vec.reserve(4);   // Much smaller than current capacity
    EXPECT_EQ(vec.capacity(), initial_capacity);  // Capacity should not change
    
    // Verify elements are still intact
    for (int i = 0; i < 8; ++i) {
        EXPECT_EQ(vec[i], i);
    }
    
    // Reserve exact current capacity
    vec.reserve(initial_capacity);
    EXPECT_EQ(vec.capacity(), initial_capacity);  // Should trigger early exit
    
    // Add more elements to test the boundary
    for (int i = 8; i < 16; ++i) {
        vec.push_back(i);
    }
    
    // Reserve current size - should early exit
    vec.reserve(vec.size());
    EXPECT_EQ(vec.capacity(), initial_capacity);
    
    // Verify all elements
    for (int i = 0; i < 16; ++i) {
        EXPECT_EQ(vec[i], i);
    }
}

TEST_F(ChunkedVectorTest, EnsurePageCapacityEarlyExitEmptyVector) {
    chunked_vector<int> vec;
    
    // Empty vector with zero capacity
    EXPECT_EQ(vec.capacity(), 0);
    
    // Reserve zero capacity on empty vector - should early exit
    vec.reserve(0);
    EXPECT_EQ(vec.capacity(), 0);
    
    // Verify vector is still empty and functional
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    
    // Should still be able to add elements normally
    vec.push_back(42);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 42);
}

// ============================================================================
// Erase Tests
// ============================================================================

TEST_F(ChunkedVectorTest, EraseSingleElement) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    // Erase element at index 3 (value 3)
    auto it = vec.begin() + 3;
    auto result_it = vec.erase(it);
    
    EXPECT_EQ(vec.size(), 9);
    EXPECT_EQ(*result_it, 4);  // Should point to element that moved into position 3
    EXPECT_EQ(result_it - vec.begin(), 3);  // Should be at index 3
    
    // Verify all remaining elements
    std::vector<int> expected = {0, 1, 2, 4, 5, 6, 7, 8, 9};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseFirstElement) {
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i + 10);
    }
    
    auto result_it = vec.erase(vec.begin());
    
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(*result_it, 11);
    EXPECT_EQ(result_it, vec.begin());
    
    std::vector<int> expected = {11, 12, 13, 14};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseLastElement) {
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i + 10);
    }
    
    auto result_it = vec.erase(vec.end() - 1);
    
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(result_it, vec.end());
    
    std::vector<int> expected = {10, 11, 12, 13};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseSingleElementVector) {
    chunked_vector<int> vec;
    vec.push_back(42);
    
    auto result_it = vec.erase(vec.begin());
    
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(result_it, vec.end());
}

TEST_F(ChunkedVectorTest, EraseRange) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    // Erase elements 2, 3, 4 (range [2, 5))
    auto first = vec.begin() + 2;
    auto last = vec.begin() + 5;
    auto result_it = vec.erase(first, last);
    
    EXPECT_EQ(vec.size(), 7);
    EXPECT_EQ(*result_it, 5);  // Should point to element that moved into position 2
    EXPECT_EQ(result_it - vec.begin(), 2);
    
    std::vector<int> expected = {0, 1, 5, 6, 7, 8, 9};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseRangeFromBeginning) {
    chunked_vector<int> vec;
    for (int i = 0; i < 8; ++i) {
        vec.push_back(i + 10);
    }
    
    // Erase first 3 elements
    auto result_it = vec.erase(vec.begin(), vec.begin() + 3);
    
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(*result_it, 13);
    EXPECT_EQ(result_it, vec.begin());
    
    std::vector<int> expected = {13, 14, 15, 16, 17};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseRangeToEnd) {
    chunked_vector<int> vec;
    for (int i = 0; i < 8; ++i) {
        vec.push_back(i + 10);
    }
    
    // Erase last 3 elements
    auto result_it = vec.erase(vec.end() - 3, vec.end());
    
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(result_it, vec.end());
    
    std::vector<int> expected = {10, 11, 12, 13, 14};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseEmptyRange) {
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    
    auto it = vec.begin() + 2;
    auto result_it = vec.erase(it, it);  // Empty range
    
    EXPECT_EQ(vec.size(), 5);  // No change
    EXPECT_EQ(result_it, it);  // Should return the same iterator
    
    // Verify all elements are unchanged
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(vec[i], i);
    }
}

TEST_F(ChunkedVectorTest, EraseEntireVector) {
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    
    auto result_it = vec.erase(vec.begin(), vec.end());
    
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(result_it, vec.end());
    EXPECT_EQ(result_it, vec.begin());  // begin() == end() for empty vector
}

TEST_F(ChunkedVectorTest, EraseAcrossPageBoundaries) {
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Fill multiple pages
    for (int i = 0; i < 12; ++i) {  // 3 full pages
        vec.push_back(i);
    }
    
    // Erase elements 2-8 (spanning multiple pages)
    auto result_it = vec.erase(vec.begin() + 2, vec.begin() + 9);
    
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(*result_it, 9);
    
    std::vector<int> expected = {0, 1, 9, 10, 11};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

// ============================================================================
// Erase Unsorted Tests
// ============================================================================

TEST_F(ChunkedVectorTest, EraseUnsortedMiddleElement) {
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    // Erase element at index 3 (value 3)
    auto it = vec.begin() + 3;
    auto result_it = vec.erase_unsorted(it);
    
    EXPECT_EQ(vec.size(), 9);
    EXPECT_EQ(*result_it, 9);  // Last element (9) should now be at position 3
    EXPECT_EQ(result_it - vec.begin(), 3);
    
    // Check that the last element was moved to position 3
    EXPECT_EQ(vec[3], 9);
    
    // Check other elements are unchanged
    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[1], 1);
    EXPECT_EQ(vec[2], 2);
    EXPECT_EQ(vec[4], 4);
    EXPECT_EQ(vec[5], 5);
    EXPECT_EQ(vec[6], 6);
    EXPECT_EQ(vec[7], 7);
    EXPECT_EQ(vec[8], 8);
}

TEST_F(ChunkedVectorTest, EraseUnsortedFirstElement) {
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i + 10);
    }
    
    auto result_it = vec.erase_unsorted(vec.begin());
    
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(*result_it, 14);  // Last element (14) should now be at position 0
    EXPECT_EQ(result_it, vec.begin());
    
    // Check that last element was moved to first position
    EXPECT_EQ(vec[0], 14);
    EXPECT_EQ(vec[1], 11);
    EXPECT_EQ(vec[2], 12);
    EXPECT_EQ(vec[3], 13);
}

TEST_F(ChunkedVectorTest, EraseUnsortedLastElement) {
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i + 10);
    }
    
    auto result_it = vec.erase_unsorted(vec.end() - 1);
    
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(result_it, vec.end());  // Should return end() when erasing last element
    
    // All other elements should be unchanged
    std::vector<int> expected = {10, 11, 12, 13};
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseUnsortedSingleElementVector) {
    chunked_vector<int> vec;
    vec.push_back(42);
    
    auto result_it = vec.erase_unsorted(vec.begin());
    
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(result_it, vec.end());
}

TEST_F(ChunkedVectorTest, EraseUnsortedAcrossPages) {
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Fill multiple pages
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 10);
    }
    
    // Erase element from middle of first page
    auto result_it = vec.erase_unsorted(vec.begin() + 1);
    
    EXPECT_EQ(vec.size(), 9);
    EXPECT_EQ(*result_it, 90);  // Last element (90) should be at position 1
    
    // Check the swap occurred
    EXPECT_EQ(vec[0], 0);    // Unchanged
    EXPECT_EQ(vec[1], 90);   // Last element moved here
    EXPECT_EQ(vec[2], 20);   // Unchanged
    EXPECT_EQ(vec[3], 30);   // Unchanged
    // ... rest should be unchanged except last is gone
}

// ============================================================================
// Erase Custom Type Tests
// ============================================================================

TEST_F(ChunkedVectorCustomTypeTest, EraseCustomType) {
    chunked_vector<TestObject> vec;
    for (int i = 0; i < 5; ++i) {
        vec.emplace_back(i * 10);
    }
    
    //int initial_constructors = TestObject::constructor_calls;
    int initial_destructors = TestObject::destructor_calls;
    
    // Erase middle element
    auto result_it = vec.erase(vec.begin() + 2);
    
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(result_it->value, 30);  // Element that moved into position 2
    
    // Check that destructor was called for the erased element
    // and for temporary objects during the move operations
    EXPECT_GT(TestObject::destructor_calls, initial_destructors);
    
    // Check remaining elements
    EXPECT_EQ(vec[0].value, 0);
    EXPECT_EQ(vec[1].value, 10);
    EXPECT_EQ(vec[2].value, 30);
    EXPECT_EQ(vec[3].value, 40);
}

TEST_F(ChunkedVectorCustomTypeTest, EraseUnsortedCustomType) {
    chunked_vector<TestObject> vec;
    for (int i = 0; i < 5; ++i) {
        vec.emplace_back(i * 10);
    }
    
    int initial_destructors = TestObject::destructor_calls;
    
    // Erase middle element with unsorted erase
    auto result_it = vec.erase_unsorted(vec.begin() + 1);
    
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(result_it->value, 40);  // Last element should be at position 1
    
    // Check that destructor was called
    EXPECT_GT(TestObject::destructor_calls, initial_destructors);
    
    // Check that last element was moved to position 1
    EXPECT_EQ(vec[0].value, 0);
    EXPECT_EQ(vec[1].value, 40);  // Last element moved here
    EXPECT_EQ(vec[2].value, 20);
    EXPECT_EQ(vec[3].value, 30);
}

// ============================================================================
// STL Algorithm Compatibility with Erase
// ============================================================================

TEST_F(ChunkedVectorTest, EraseWithSTLAlgorithms) {
    chunked_vector<int> vec;
    for (int i = 0; i < 20; ++i) {
        vec.push_back(i);
    }
    
    // Use erase-remove idiom to remove all even numbers
    auto new_end = std::remove_if(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; });
    vec.erase(new_end, vec.end());
    
    // Should contain only odd numbers
    EXPECT_EQ(vec.size(), 10);
    for (size_t i = 0; i < vec.size(); ++i) {
        EXPECT_EQ(vec[i], static_cast<int>(2 * i + 1));  // 1, 3, 5, 7, ...
    }
}

// Main function is not needed as we use gtest_main 