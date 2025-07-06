#include "test_iterator_debug_assertions.h"
#include "chunked_vector/chunked_vector.h"
#include "test_common.h"
#include <algorithm>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <numeric>
#include <string>
#include <vector>

using namespace dod;

// Test fixture for basic types
class ChunkedVectorTest : public ::testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
};

// TestObject is now provided by test_common.h

class ChunkedVectorCustomTypeTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
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

TEST_F(ChunkedVectorTest, DefaultConstructor)
{
    chunked_vector<int> vec;
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.capacity(), 0);
}

TEST_F(ChunkedVectorTest, CountConstructor)
{
    chunked_vector<int> vec(5);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), 5);
    EXPECT_GE(vec.capacity(), 5);

    for (size_t i = 0; i < vec.size(); ++i)
    {
        EXPECT_EQ(vec[i], 0); // Default constructed
    }
}

TEST_F(ChunkedVectorTest, CountValueConstructor)
{
    chunked_vector<int> vec(5, 42);
    EXPECT_FALSE(vec.empty());
    EXPECT_EQ(vec.size(), 5);
    EXPECT_GE(vec.capacity(), 5);

    for (size_t i = 0; i < vec.size(); ++i)
    {
        EXPECT_EQ(vec[i], 42);
    }
}

TEST_F(ChunkedVectorTest, CopyConstructor)
{
    chunked_vector<int> original;
    for (int i = 0; i < 5; ++i)
    {
        original.push_back(i + 1);
    }

    chunked_vector<int> copy(original);
    EXPECT_EQ(copy.size(), original.size());

    for (size_t i = 0; i < original.size(); ++i)
    {
        EXPECT_EQ(copy[i], original[i]);
    }
}

TEST_F(ChunkedVectorTest, MoveConstructor)
{
    chunked_vector<int> original;
    for (int i = 0; i < 5; ++i)
    {
        original.push_back(i);
    }
    size_t original_size = original.size();

    chunked_vector<int> moved(std::move(original));
    EXPECT_EQ(moved.size(), original_size);
    EXPECT_TRUE(original.empty()); // Original should be empty after move

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(moved[i], i);
    }
}

TEST_F(ChunkedVectorTest, InitializerListConstructor)
{
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
    for (int i = 0; i < 5; ++i)
    {
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
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(large_vec[i], i + 1);
    }
}

// ============================================================================
// Assignment Tests
// ============================================================================

TEST_F(ChunkedVectorTest, CopyAssignment)
{
    chunked_vector<int> original;
    for (int i = 0; i < 5; ++i)
    {
        original.push_back(i);
    }

    chunked_vector<int> copy;
    copy.push_back(999); // Make sure copy isn't empty
    copy = original;

    EXPECT_EQ(copy.size(), original.size());
    for (size_t i = 0; i < original.size(); ++i)
    {
        EXPECT_EQ(copy[i], original[i]);
    }
}

TEST_F(ChunkedVectorTest, MoveAssignment)
{
    chunked_vector<int> original;
    for (int i = 0; i < 5; ++i)
    {
        original.push_back(i);
    }
    size_t original_size = original.size();

    chunked_vector<int> moved;
    moved.push_back(999); // Make sure moved isn't empty
    moved = std::move(original);

    EXPECT_EQ(moved.size(), original_size);
    EXPECT_TRUE(original.empty());

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(moved[i], i);
    }
}

TEST_F(ChunkedVectorTest, SelfAssignment)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i);
    }

    vec = vec; // Self assignment
    EXPECT_EQ(vec.size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }
}

TEST_F(ChunkedVectorTest, InitializerListAssignment)
{
    chunked_vector<int> vec;

    // Initial values
    for (int i = 0; i < 5; ++i)
    {
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
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(vec[i], (i + 1) * 10);
    }

    // Assign to larger size (growing)
    vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    EXPECT_EQ(vec.size(), 10);
    for (int i = 0; i < 10; ++i)
    {
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

TEST_F(ChunkedVectorTest, IndexOperator)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i * 2);
    }

    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(vec[i], i * 2);
    }

    // Test modification
    vec[5] = 999;
    EXPECT_EQ(vec[5], 999);
}

TEST_F(ChunkedVectorTest, IndexOperatorConst)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i * 2);
    }

    const auto& const_vec = vec;
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(const_vec[i], i * 2);
    }
}

TEST_F(ChunkedVectorTest, AtMethod)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i);
    }

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(vec.at(i), i);
    }

    EXPECT_THROW(CHUNKED_VEC_MAYBE_UNUSED(vec.at(5)), std::out_of_range);
    EXPECT_THROW(CHUNKED_VEC_MAYBE_UNUSED(vec.at(100)), std::out_of_range);
}

TEST_F(ChunkedVectorTest, AtMethodConst)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i);
    }

    const auto& const_vec = vec;
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(const_vec.at(i), i);
    }

    EXPECT_THROW(CHUNKED_VEC_MAYBE_UNUSED(const_vec.at(5)), std::out_of_range);
}

TEST_F(ChunkedVectorTest, FrontAndBack)
{
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

TEST_F(ChunkedVectorTest, FrontAndBackConst)
{
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

TEST_F(ChunkedVectorTest, BasicIterators)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i);
    }

    int expected = 0;
    for (auto it = vec.begin(); it != vec.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
    EXPECT_EQ(expected, 10);
}

TEST_F(ChunkedVectorTest, ConstIterators)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i);
    }

    const auto& const_vec = vec;
    int expected = 0;
    for (auto it = const_vec.begin(); it != const_vec.end(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }

    expected = 0;
    for (auto it = const_vec.cbegin(); it != const_vec.cend(); ++it)
    {
        EXPECT_EQ(*it, expected++);
    }
}

TEST_F(ChunkedVectorTest, ForwardIteratorIncrement)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i * 10);
    }

    auto it = vec.begin();
    EXPECT_EQ(*it, 0);

    // Test pre-increment
    ++it;
    EXPECT_EQ(*it, 10);

    ++it;
    EXPECT_EQ(*it, 20);

    // Test post-increment
    auto old_it = it++;
    EXPECT_EQ(*old_it, 20);
    EXPECT_EQ(*it, 30);
}

TEST_F(ChunkedVectorTest, ForwardIteratorComparisons)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i);
    }

    auto it1 = vec.begin();
    auto it2 = vec.begin();
    ++it2; // Advance to second element
    auto it3 = vec.end();

    // Forward iterators support equality comparison
    EXPECT_TRUE(it1 == vec.begin());
    EXPECT_TRUE(it1 != it2);
    EXPECT_TRUE(it2 != it3);
    EXPECT_FALSE(it1 == it2);
    EXPECT_FALSE(it2 == it3);

    // Test comparison with different iterator instances pointing to same position
    auto it4 = vec.begin();
    EXPECT_TRUE(it1 == it4);
    EXPECT_FALSE(it1 != it4);
}

// ============================================================================
// Capacity Tests
// ============================================================================

TEST_F(ChunkedVectorTest, EmptyAndSize)
{
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

TEST_F(ChunkedVectorTest, Capacity)
{
    chunked_vector<int> vec;
    EXPECT_EQ(vec.capacity(), 0);

    vec.push_back(1);
    EXPECT_GE(vec.capacity(), 1);

    size_t initial_capacity = vec.capacity();
    size_t original_size = vec.size();

    // Add elements up to but not exceeding the current capacity
    for (size_t i = original_size; i < initial_capacity && i < original_size + 10; ++i)
    {
        vec.push_back(static_cast<int>(i));
    }
    // Should still be within initial capacity (or it may have grown if we exceeded it)
    EXPECT_GE(vec.capacity(), initial_capacity);
}

TEST_F(ChunkedVectorTest, Reserve)
{
    chunked_vector<int> vec;
    vec.reserve(100);
    EXPECT_GE(vec.capacity(), 100);
    EXPECT_EQ(vec.size(), 0);

    // Adding elements shouldn't change capacity if within reserved space
    size_t reserved_capacity = vec.capacity();
    for (int i = 0; i < 50; ++i)
    {
        vec.push_back(i);
    }
    EXPECT_EQ(vec.capacity(), reserved_capacity);

    // Reserve with smaller size shouldn't change capacity
    vec.reserve(50);
    EXPECT_EQ(vec.capacity(), reserved_capacity);
}

TEST_F(ChunkedVectorTest, ShrinkToFit)
{
    chunked_vector<int> vec;
    vec.reserve(chunked_vector<int>::page_size() * 2); // Reserve multiple pages

    for (int i = 0; i < 10; ++i)
    {
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
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }
}

// ============================================================================
// Modifier Tests
// ============================================================================

TEST_F(ChunkedVectorTest, Clear)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i)
    {
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

TEST_F(ChunkedVectorTest, PushBack)
{
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

TEST_F(ChunkedVectorTest, EmplaceBack)
{
    chunked_vector<std::string> vec;

    vec.emplace_back("Hello");
    vec.emplace_back("World");
    vec.emplace_back(5, 'x'); // string(5, 'x')

    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], "Hello");
    EXPECT_EQ(vec[1], "World");
    EXPECT_EQ(vec[2], "xxxxx");
}

TEST_F(ChunkedVectorTest, PopBack)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i)
    {
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

TEST_F(ChunkedVectorTest, ResizeGrow)
{
    chunked_vector<int> vec;
    vec.resize(5);

    EXPECT_EQ(vec.size(), 5);
    for (size_t i = 0; i < vec.size(); ++i)
    {
        EXPECT_EQ(vec[i], 0); // Default constructed
    }
}

TEST_F(ChunkedVectorTest, ResizeGrowWithValue)
{
    chunked_vector<int> vec;
    vec.resize(5, 42);

    EXPECT_EQ(vec.size(), 5);
    for (size_t i = 0; i < vec.size(); ++i)
    {
        EXPECT_EQ(vec[i], 42);
    }
}

TEST_F(ChunkedVectorTest, ResizeShrink)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i);
    }

    vec.resize(5);
    EXPECT_EQ(vec.size(), 5);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }
}

// ============================================================================
// Page Size and Large Container Tests
// ============================================================================

TEST_F(ChunkedVectorTest, PageSize)
{
    chunked_vector<int, 16> vec;
    EXPECT_EQ(vec.page_size(), 16);

    chunked_vector<int, 1024> vec2;
    EXPECT_EQ(vec2.page_size(), 1024);
}

TEST_F(ChunkedVectorTest, SpanMultiplePages)
{
    constexpr size_t PAGE_SIZE = 8;
    chunked_vector<int, PAGE_SIZE> vec;

    // Add elements spanning multiple pages
    const int count = PAGE_SIZE * 3 + 5; // 3 full pages + 5 elements
    for (int i = 0; i < count; ++i)
    {
        vec.push_back(i);
    }

    EXPECT_EQ(vec.size(), count);
    EXPECT_GE(vec.capacity(), count);

    // Verify all elements are correct
    for (int i = 0; i < count; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }
}

TEST_F(ChunkedVectorTest, PageBoundaryOperations)
{
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<int, PAGE_SIZE> vec;

    // Fill exactly one page
    for (int i = 0; i < PAGE_SIZE; ++i)
    {
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

    for (size_t i = 0; i < PAGE_SIZE; ++i)
    {
        EXPECT_EQ(vec[i], static_cast<int>(i));
    }
}

// ============================================================================
// Custom Type Tests
// ============================================================================

TEST_F(ChunkedVectorCustomTypeTest, CustomTypeConstructorDestructor)
{
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

TEST_F(ChunkedVectorCustomTypeTest, CustomTypeCopyMove)
{
    chunked_vector<TestObject> vec;

    TestObject obj1(42);
    vec.push_back(obj1); // Copy
    EXPECT_GT(TestObject::copy_calls, 0);

    vec.push_back(TestObject(43)); // Move
    EXPECT_GT(TestObject::move_calls, 0);

    EXPECT_EQ(vec[0].value, 42);
    EXPECT_EQ(vec[1].value, 43);
}

TEST_F(ChunkedVectorCustomTypeTest, InitializerListCustomType)
{
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

TEST_F(ChunkedVectorTest, RangeBasedLoop)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i * 2);
    }

    int expected = 0;
    for (const auto& value : vec)
    {
        EXPECT_EQ(value, expected * 2);
        ++expected;
    }
    EXPECT_EQ(expected, 10);
}

TEST_F(ChunkedVectorTest, STLAlgorithmCompatibility)
{
    chunked_vector<int> vec;
    for (int i = 1; i <= 10; ++i)
    {
        vec.push_back(i);
    }

    // Test std::find
    auto it = std::find(vec.begin(), vec.end(), 5);
    EXPECT_NE(it, vec.end());
    EXPECT_EQ(*it, 5);

    // Test std::accumulate
    int sum = std::accumulate(vec.begin(), vec.end(), 0);
    EXPECT_EQ(sum, 55); // 1+2+...+10 = 55

    // Test std::count
    ptrdiff_t count_even = std::count_if(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; });
    EXPECT_EQ(count_even, 5); // 2, 4, 6, 8, 10

    // Test std::all_of, std::any_of, std::none_of
    bool all_positive = std::all_of(vec.begin(), vec.end(), [](int x) { return x > 0; });
    EXPECT_TRUE(all_positive);

    bool any_greater_than_5 = std::any_of(vec.begin(), vec.end(), [](int x) { return x > 5; });
    EXPECT_TRUE(any_greater_than_5);

    bool none_negative = std::none_of(vec.begin(), vec.end(), [](int x) { return x < 0; });
    EXPECT_TRUE(none_negative);

    // Test std::for_each
    int multiplied_sum = 0;
    std::for_each(vec.begin(), vec.end(), [&multiplied_sum](int x) { multiplied_sum += x * 2; });
    EXPECT_EQ(multiplied_sum, 110); // (1+2+...+10) * 2 = 55 * 2
}

// ============================================================================
// Edge Cases and Exception Safety
// ============================================================================

TEST_F(ChunkedVectorTest, EmptyVectorOperations)
{
    chunked_vector<int> vec;

    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(vec.begin(), vec.end());
    EXPECT_EQ(vec.cbegin(), vec.cend());
}

TEST_F(ChunkedVectorTest, SingleElementOperations)
{
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

TEST_F(ChunkedVectorTest, EmptyFunctionComprehensive)
{
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

TEST_F(ChunkedVectorTest, ResizeSmallerEdgeCases)
{
    chunked_vector<int, 4> vec; // Small page size for testing

    // Fill multiple pages
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i * 10);
    }
    EXPECT_EQ(vec.size(), 10);

    // Resize to exactly one page boundary
    vec.resize(4);
    EXPECT_EQ(vec.size(), 4);
    for (int i = 0; i < 4; ++i)
    {
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
    for (int i = 0; i < 3; ++i)
    {
        EXPECT_EQ(vec[i], 999);
    }

    // Resize smaller with value parameter (should not use the value)
    vec.resize(1, 888);
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 999); // Original value preserved
}

TEST_F(ChunkedVectorTest, EnsurePageCapacityEarlyExit)
{
    chunked_vector<int, 8> vec; // Use smaller page size for predictable behavior

    // Reserve initial capacity
    vec.reserve(16); // This should allocate 2 pages (16/8 = 2)
    size_t initial_capacity = vec.capacity();
    EXPECT_GE(initial_capacity, 16);

    // Add some elements but stay within capacity
    for (int i = 0; i < 8; ++i)
    {
        vec.push_back(i);
    }

    // Reserve same or smaller capacity - should trigger early exit in ensure_page_capacity
    vec.reserve(16);
    EXPECT_EQ(vec.capacity(), initial_capacity); // Capacity should not change

    vec.reserve(8);                              // Smaller than current capacity
    EXPECT_EQ(vec.capacity(), initial_capacity); // Capacity should not change

    vec.reserve(4);                              // Much smaller than current capacity
    EXPECT_EQ(vec.capacity(), initial_capacity); // Capacity should not change

    // Verify elements are still intact
    for (int i = 0; i < 8; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }

    // Reserve exact current capacity
    vec.reserve(initial_capacity);
    EXPECT_EQ(vec.capacity(), initial_capacity); // Should trigger early exit

    // Add more elements to test the boundary
    for (int i = 8; i < 16; ++i)
    {
        vec.push_back(i);
    }

    // Reserve current size - should early exit
    vec.reserve(vec.size());
    EXPECT_EQ(vec.capacity(), initial_capacity);

    // Verify all elements
    for (int i = 0; i < 16; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }
}

TEST_F(ChunkedVectorTest, EnsurePageCapacityEarlyExitEmptyVector)
{
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

// Helper function to advance forward iterator by n steps
template <typename Iterator> Iterator advance_iterator(Iterator it, int n)
{
    for (int i = 0; i < n; ++i)
    {
        ++it;
    }
    return it;
}

// ============================================================================
// Erase Tests
// ============================================================================

TEST_F(ChunkedVectorTest, EraseSingleElement)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i);
    }

    // Erase element at index 3 (value 3)
    auto it = advance_iterator(vec.begin(), 3);
    auto result_it = vec.erase(it);

    EXPECT_EQ(vec.size(), 9);
    EXPECT_EQ(*result_it, 4); // Should point to element that moved into position 3

    // Verify result iterator points to the correct position by checking it equals the expected iterator
    auto expected_it = advance_iterator(vec.begin(), 3);
    EXPECT_EQ(result_it, expected_it);

    // Verify all remaining elements
    std::vector<int> expected = {0, 1, 2, 4, 5, 6, 7, 8, 9};
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseFirstElement)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i + 10);
    }

    auto result_it = vec.erase(vec.begin());

    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(*result_it, 11);
    EXPECT_EQ(result_it, vec.begin());

    std::vector<int> expected = {11, 12, 13, 14};
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseLastElement)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i + 10);
    }

    // Get iterator to last element (one before end)
    auto last_element_it = advance_iterator(vec.begin(), 4);
    auto result_it = vec.erase(last_element_it);

    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(result_it, vec.end());

    std::vector<int> expected = {10, 11, 12, 13};
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseSingleElementVector)
{
    chunked_vector<int> vec;
    vec.push_back(42);

    auto result_it = vec.erase(vec.begin());

    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(result_it, vec.end());
}

TEST_F(ChunkedVectorTest, EraseRange)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i);
    }

    // Erase elements 2, 3, 4 (range [2, 5))
    auto first = advance_iterator(vec.begin(), 2);
    auto last = advance_iterator(vec.begin(), 5);
    auto result_it = vec.erase(first, last);

    EXPECT_EQ(vec.size(), 7);
    EXPECT_EQ(*result_it, 5); // Should point to element that moved into position 2

    // Verify result iterator points to the correct position
    auto expected_it = advance_iterator(vec.begin(), 2);
    EXPECT_EQ(result_it, expected_it);

    std::vector<int> expected = {0, 1, 5, 6, 7, 8, 9};
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseRangeFromBeginning)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 8; ++i)
    {
        vec.push_back(i + 10);
    }

    // Erase first 3 elements
    auto last = advance_iterator(vec.begin(), 3);
    auto result_it = vec.erase(vec.begin(), last);

    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(*result_it, 13);
    EXPECT_EQ(result_it, vec.begin());

    std::vector<int> expected = {13, 14, 15, 16, 17};
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseRangeToEnd)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 8; ++i)
    {
        vec.push_back(i + 10);
    }

    // Erase last 3 elements (from index 5 to end)
    auto first = advance_iterator(vec.begin(), 5);
    auto result_it = vec.erase(first, vec.end());

    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(result_it, vec.end());

    std::vector<int> expected = {10, 11, 12, 13, 14};
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseEmptyRange)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i);
    }

    auto it = advance_iterator(vec.begin(), 2);
    auto result_it = vec.erase(it, it); // Empty range

    EXPECT_EQ(vec.size(), 5); // No change
    EXPECT_EQ(result_it, it); // Should return the same iterator

    // Verify all elements are unchanged
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }
}

TEST_F(ChunkedVectorTest, EraseEntireVector)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i);
    }

    auto result_it = vec.erase(vec.begin(), vec.end());

    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(result_it, vec.end());
    EXPECT_EQ(result_it, vec.begin()); // begin() == end() for empty vector
}

TEST_F(ChunkedVectorTest, EraseAcrossPageBoundaries)
{
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<int, PAGE_SIZE> vec;

    // Fill multiple pages
    for (int i = 0; i < 12; ++i)
    { // 3 full pages
        vec.push_back(i);
    }

    // Erase elements 2-8 (spanning multiple pages)
    auto first = advance_iterator(vec.begin(), 2);
    auto last = advance_iterator(vec.begin(), 9);
    auto result_it = vec.erase(first, last);

    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(*result_it, 9);

    std::vector<int> expected = {0, 1, 9, 10, 11};
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

// ============================================================================
// Erase Unsorted Tests
// ============================================================================

TEST_F(ChunkedVectorTest, EraseUnsortedMiddleElement)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i);
    }

    // Erase element at index 3 (value 3)
    auto it = advance_iterator(vec.begin(), 3);
    auto result_it = vec.erase_unsorted(it);

    EXPECT_EQ(vec.size(), 9);
    EXPECT_EQ(*result_it, 9); // Last element (9) should now be at position 3

    // Verify result iterator points to the correct position
    auto expected_it = advance_iterator(vec.begin(), 3);
    EXPECT_EQ(result_it, expected_it);

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

TEST_F(ChunkedVectorTest, EraseUnsortedFirstElement)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i + 10);
    }

    auto result_it = vec.erase_unsorted(vec.begin());

    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(*result_it, 14); // Last element (14) should now be at position 0
    EXPECT_EQ(result_it, vec.begin());

    // Check that last element was moved to first position
    EXPECT_EQ(vec[0], 14);
    EXPECT_EQ(vec[1], 11);
    EXPECT_EQ(vec[2], 12);
    EXPECT_EQ(vec[3], 13);
}

TEST_F(ChunkedVectorTest, EraseUnsortedLastElement)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.push_back(i + 10);
    }

    // Get iterator to last element (one before end)
    auto last_element_it = advance_iterator(vec.begin(), 4);
    auto result_it = vec.erase_unsorted(last_element_it);

    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(result_it, vec.end()); // Should return end() when erasing last element

    // All other elements should be unchanged
    std::vector<int> expected = {10, 11, 12, 13};
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(vec[i], expected[i]);
    }
}

TEST_F(ChunkedVectorTest, EraseUnsortedSingleElementVector)
{
    chunked_vector<int> vec;
    vec.push_back(42);

    auto result_it = vec.erase_unsorted(vec.begin());

    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    EXPECT_EQ(result_it, vec.end());
}

TEST_F(ChunkedVectorTest, EraseUnsortedAcrossPages)
{
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<int, PAGE_SIZE> vec;

    // Fill multiple pages
    for (int i = 0; i < 10; ++i)
    {
        vec.push_back(i * 10);
    }

    // Erase element from middle of first page
    auto it = advance_iterator(vec.begin(), 1);
    auto result_it = vec.erase_unsorted(it);

    EXPECT_EQ(vec.size(), 9);
    EXPECT_EQ(*result_it, 90); // Last element (90) should be at position 1

    // Check the swap occurred
    EXPECT_EQ(vec[0], 0);  // Unchanged
    EXPECT_EQ(vec[1], 90); // Last element moved here
    EXPECT_EQ(vec[2], 20); // Unchanged
    EXPECT_EQ(vec[3], 30); // Unchanged
    // ... rest should be unchanged except last is gone
}

// ============================================================================
// Erase Custom Type Tests
// ============================================================================

TEST_F(ChunkedVectorCustomTypeTest, EraseCustomType)
{
    chunked_vector<TestObject> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.emplace_back(i * 10);
    }

    // int initial_constructors = TestObject::constructor_calls;
    int initial_destructors = TestObject::destructor_calls;

    // Erase middle element
    auto it = advance_iterator(vec.begin(), 2);
    auto result_it = vec.erase(it);

    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(result_it->value, 30); // Element that moved into position 2

    // Check that destructor was called for the erased element
    // and for temporary objects during the move operations
    EXPECT_GT(TestObject::destructor_calls, initial_destructors);

    // Check remaining elements
    EXPECT_EQ(vec[0].value, 0);
    EXPECT_EQ(vec[1].value, 10);
    EXPECT_EQ(vec[2].value, 30);
    EXPECT_EQ(vec[3].value, 40);
}

TEST_F(ChunkedVectorCustomTypeTest, EraseUnsortedCustomType)
{
    chunked_vector<TestObject> vec;
    for (int i = 0; i < 5; ++i)
    {
        vec.emplace_back(i * 10);
    }

    int initial_destructors = TestObject::destructor_calls;

    // Erase middle element with unsorted erase
    auto it = advance_iterator(vec.begin(), 1);
    auto result_it = vec.erase_unsorted(it);

    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(result_it->value, 40); // Last element should be at position 1

    // Check that destructor was called
    EXPECT_GT(TestObject::destructor_calls, initial_destructors);

    // Check that last element was moved to position 1
    EXPECT_EQ(vec[0].value, 0);
    EXPECT_EQ(vec[1].value, 40); // Last element moved here
    EXPECT_EQ(vec[2].value, 20);
    EXPECT_EQ(vec[3].value, 30);
}

// ============================================================================
// STL Algorithm Compatibility with Erase
// ============================================================================

TEST_F(ChunkedVectorTest, EraseWithSTLAlgorithms)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 20; ++i)
    {
        vec.push_back(i);
    }

    // Use erase-remove idiom to remove all even numbers
    auto new_end = std::remove_if(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; });
    vec.erase(new_end, vec.end());

    // Should contain only odd numbers
    EXPECT_EQ(vec.size(), 10);
    for (size_t i = 0; i < vec.size(); ++i)
    {
        EXPECT_EQ(vec[i], static_cast<int>(2 * i + 1)); // 1, 3, 5, 7, ...
    }
}

// ============================================================================
// Additional Coverage Tests for 100% Coverage
// ============================================================================

TEST_F(ChunkedVectorTest, ShrinkToFitEmptyVectorEdgeCase)
{
    // This test covers the redundant assignment line in shrink_to_fit
    // when pages_needed == 0 && m_size == 0
    chunked_vector<int> vec;

    // Vector is empty, so pages_needed will be 0 and m_size is 0
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);

    // This should hit the condition where pages_needed == 0 && m_size == 0
    // which triggers the redundant assignment: pages_needed = 0;
    vec.shrink_to_fit();

    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
}

TEST_F(ChunkedVectorTest, EnsurePageCapacityGrowthFactorInsufficient)
{
    // This test covers the case where the growth factor (capacity + capacity/2)
    // is insufficient and we need to set new_page_capacity = pages_needed

    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<int, PAGE_SIZE> vec;

    // First, establish some initial capacity
    vec.reserve(PAGE_SIZE * 8); // Reserve 8 pages
    size_t initial_capacity = vec.capacity();
    EXPECT_GE(initial_capacity, PAGE_SIZE * 8);

    // Add some elements to use the current capacity
    for (int i = 0; i < PAGE_SIZE * 6; ++i)
    {
        vec.push_back(i);
    }

    // Now request a capacity that's much larger than the growth factor would provide
    // Growth factor is: current_capacity + (current_capacity >> 1) = 8 + 4 = 12 pages
    // But we'll request much more than that to force the line: new_page_capacity = pages_needed;
    size_t huge_capacity = PAGE_SIZE * 100; // Request 100 pages
    vec.reserve(huge_capacity);

    EXPECT_GE(vec.capacity(), huge_capacity);

    // Verify elements are still intact
    for (int i = 0; i < PAGE_SIZE * 6; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }
}

TEST_F(ChunkedVectorTest, EnsurePageCapacityWithExistingPages)
{
    // This test covers the copy loop and free operations in ensure_page_capacity
    // We need to trigger page array reallocation when there are existing pages

    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<int, PAGE_SIZE> vec;

    // Start with an empty vector and add elements to create some pages
    for (int i = 0; i < PAGE_SIZE * 3; ++i)
    { // Fill 3 pages
        vec.push_back(i * 10);
    }

    size_t initial_capacity = vec.capacity();
    EXPECT_EQ(initial_capacity, PAGE_SIZE * 3);

    // Now trigger a reallocation that will copy existing pages
    // This should hit the copy loop: new_pages[i] = m_pages[i];
    // and the free operation: CHUNKED_VEC_FREE(m_pages);
    vec.reserve(PAGE_SIZE * 20); // Request much more capacity

    EXPECT_GE(vec.capacity(), PAGE_SIZE * 20);

    // Verify all existing elements are still intact after reallocation
    for (int i = 0; i < PAGE_SIZE * 3; ++i)
    {
        EXPECT_EQ(vec[i], i * 10);
    }

    // Add more elements to verify the new capacity works
    for (int i = PAGE_SIZE * 3; i < PAGE_SIZE * 5; ++i)
    {
        vec.push_back(i * 10);
    }

    // Verify all elements including new ones
    for (int i = 0; i < PAGE_SIZE * 5; ++i)
    {
        EXPECT_EQ(vec[i], i * 10);
    }
}

TEST_F(ChunkedVectorTest, PageArrayReallocationFromNonEmpty)
{
    // Another test to ensure we hit the page copying and freeing code paths
    constexpr size_t PAGE_SIZE = 8;
    chunked_vector<int, PAGE_SIZE> vec;

    // Create a vector with some content
    for (int i = 0; i < PAGE_SIZE + 1; ++i)
    { // Force allocation of 2 pages
        vec.push_back(i);
    }

    EXPECT_EQ(vec.size(), PAGE_SIZE + 1);
    size_t original_capacity = vec.capacity();

    // Force reallocation by requesting much larger capacity
    // This will definitely trigger the page copying logic
    vec.reserve(PAGE_SIZE * 50);

    EXPECT_GT(vec.capacity(), original_capacity);

    // Verify data integrity
    for (int i = 0; i < PAGE_SIZE + 1; ++i)
    {
        EXPECT_EQ(vec[i], i);
    }
}

TEST_F(ChunkedVectorTest, AddMultipleElements)
{
    constexpr size_t size = 100000;
    chunked_vector<int> vec;
    for (size_t i = 0; i < size; ++i)
    {
        vec.push_back(int(i));
    }

    EXPECT_EQ(vec.size(), size);

    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(vec[i], int(i));
    }
}

// ============================================================================
// Container Equivalence Tests
// ============================================================================

// Test fixture for comparing chunked_vector with std::vector behavior
class ContainerEquivalenceTest : public ::testing::Test
{
  protected:
    void SetUp() override {}
    void TearDown() override {}
    
    template<typename T>
    void test_containers_equivalent_push_back(size_t size) {
        std::vector<T> std_vec;
        chunked_vector<T> chunked_vec;
        
        test_push_back(std_vec, size);
        test_push_back(chunked_vec, size);
        
        EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
        EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
    }
    
    template<typename T>
    void test_containers_equivalent_sequential_access() {
        std::vector<T> std_vec;
        chunked_vector<T> chunked_vec;
        
        test_sequential_access(std_vec);
        test_sequential_access(chunked_vec);
        
        EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
        EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
    }
    
    template<typename T>
    void test_containers_equivalent_random_access() {
        std::vector<T> std_vec;
        chunked_vector<T> chunked_vec;
        
        test_random_access(std_vec);
        test_random_access(chunked_vec);
        
        EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
        EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
    }
    
    template<typename T>
    void test_containers_equivalent_iterator_traversal() {
        std::vector<T> std_vec;
        chunked_vector<T> chunked_vec;
        
        test_iterator_traversal(std_vec);
        test_iterator_traversal(chunked_vec);
        
        EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
        EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
    }
    
    template<typename T>
    void test_containers_equivalent_range_based_loop() {
        std::vector<T> std_vec;
        chunked_vector<T> chunked_vec;
        
        test_range_based_loop(std_vec);
        test_range_based_loop(chunked_vec);
        
        EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
        EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
    }
    
    template<typename T>
    void test_containers_equivalent_reserve_performance() {
        std::vector<T> std_vec;
        chunked_vector<T> chunked_vec;
        
        test_reserve_performance(std_vec);
        test_reserve_performance(chunked_vec);
        
        EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
        EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
    }
    
    template<typename T>
    void test_containers_equivalent_construct_with_size() {
        std::vector<T> std_vec;
        chunked_vector<T> chunked_vec;
        
        test_construct_with_size(std_vec, SMALL_SIZE, T(42));
        test_construct_with_size(chunked_vec, SMALL_SIZE, T(42));
        
        EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
        EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
    }
    
    template<typename T>
    void test_containers_equivalent_construct_and_fill() {
        std::vector<T> std_vec;
        chunked_vector<T> chunked_vec;
        
        test_construct_and_fill(std_vec);
        test_construct_and_fill(chunked_vec);
        
        EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
        EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
    }
    
    template<typename T>
    void test_containers_equivalent_copy_operations() {
        std::vector<T> std_source;
        chunked_vector<T> chunked_source;
        
        // Create identical source containers
        test_construct_and_fill(std_source);
        test_construct_and_fill(chunked_source);
        
        EXPECT_TRUE(containers_equal(std_source, chunked_source));
        
        // Test copy constructor
        std::vector<T> std_copy;
        chunked_vector<T> chunked_copy;
        
        test_copy_constructor(std_copy, std_source);
        test_copy_constructor(chunked_copy, chunked_source);
        
        EXPECT_TRUE(containers_equal(std_copy, chunked_copy));
        EXPECT_TRUE(containers_equal_iterators(std_copy, chunked_copy));
        
        // Test copy assignment
        std::vector<T> std_assign;
        chunked_vector<T> chunked_assign;
        
        test_copy_assignment(std_assign, std_source);
        test_copy_assignment(chunked_assign, chunked_source);
        
        EXPECT_TRUE(containers_equal(std_assign, chunked_assign));
        EXPECT_TRUE(containers_equal_iterators(std_assign, chunked_assign));
    }
    
    template<typename T>
    void test_containers_equivalent_resize_operations() {
        // Test resize grow
        std::vector<T> std_vec_grow;
        chunked_vector<T> chunked_vec_grow;
        
        test_resize_grow(std_vec_grow);
        test_resize_grow(chunked_vec_grow);
        
        EXPECT_TRUE(containers_equal(std_vec_grow, chunked_vec_grow));
        EXPECT_TRUE(containers_equal_iterators(std_vec_grow, chunked_vec_grow));
        
        // Test resize shrink
        std::vector<T> std_vec_shrink;
        chunked_vector<T> chunked_vec_shrink;
        
        test_resize_shrink(std_vec_shrink);
        test_resize_shrink(chunked_vec_shrink);
        
        EXPECT_TRUE(containers_equal(std_vec_shrink, chunked_vec_shrink));
        EXPECT_TRUE(containers_equal_iterators(std_vec_shrink, chunked_vec_shrink));
    }
    
    template<typename T>
    void test_containers_equivalent_mixed_operations() {
        std::vector<T> std_vec;
        chunked_vector<T> chunked_vec;
        
        test_mixed_operations(std_vec);
        test_mixed_operations(chunked_vec);
        
        EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
        EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
    }
    
    template<typename T>
    void test_containers_equivalent_stl_algorithms() {
        std::vector<T> std_vec;
        chunked_vector<T> chunked_vec;
        
        test_std_algorithm_find(std_vec);
        test_std_algorithm_find(chunked_vec);
        
        EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
        EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
        
        // Test accumulate with fresh containers
        std::vector<T> std_vec2;
        chunked_vector<T> chunked_vec2;
        
        test_std_algorithm_accumulate(std_vec2);
        test_std_algorithm_accumulate(chunked_vec2);
        
        EXPECT_TRUE(containers_equal(std_vec2, chunked_vec2));
        EXPECT_TRUE(containers_equal_iterators(std_vec2, chunked_vec2));
    }
    
    template<typename T>
    void test_containers_equivalent_page_boundary_access() {
        std::vector<T> std_vec;
        chunked_vector<T> chunked_vec;
        
        test_page_boundary_access(std_vec);
        test_page_boundary_access(chunked_vec);
        
        EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
        EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
    }
};

// Test equivalence with TestObject
TEST_F(ContainerEquivalenceTest, PushBackEquivalence_TestObject) {
    test_containers_equivalent_push_back<TestObject>(SMALL_SIZE);
    test_containers_equivalent_push_back<TestObject>(1000);
}

TEST_F(ContainerEquivalenceTest, PushBackEquivalence_Float) {
    test_containers_equivalent_push_back<float>(SMALL_SIZE);
    test_containers_equivalent_push_back<float>(1000);
}

TEST_F(ContainerEquivalenceTest, SequentialAccessEquivalence_TestObject) {
    test_containers_equivalent_sequential_access<TestObject>();
}

TEST_F(ContainerEquivalenceTest, SequentialAccessEquivalence_Float) {
    test_containers_equivalent_sequential_access<float>();
}

TEST_F(ContainerEquivalenceTest, RandomAccessEquivalence_TestObject) {
    test_containers_equivalent_random_access<TestObject>();
}

TEST_F(ContainerEquivalenceTest, RandomAccessEquivalence_Float) {
    test_containers_equivalent_random_access<float>();
}

TEST_F(ContainerEquivalenceTest, IteratorTraversalEquivalence_TestObject) {
    test_containers_equivalent_iterator_traversal<TestObject>();
}

TEST_F(ContainerEquivalenceTest, IteratorTraversalEquivalence_Float) {
    test_containers_equivalent_iterator_traversal<float>();
}

TEST_F(ContainerEquivalenceTest, RangeBasedLoopEquivalence_TestObject) {
    test_containers_equivalent_range_based_loop<TestObject>();
}

TEST_F(ContainerEquivalenceTest, RangeBasedLoopEquivalence_Float) {
    test_containers_equivalent_range_based_loop<float>();
}

TEST_F(ContainerEquivalenceTest, ReservePerformanceEquivalence_TestObject) {
    test_containers_equivalent_reserve_performance<TestObject>();
}

TEST_F(ContainerEquivalenceTest, ReservePerformanceEquivalence_Float) {
    test_containers_equivalent_reserve_performance<float>();
}

TEST_F(ContainerEquivalenceTest, ConstructWithSizeEquivalence_TestObject) {
    test_containers_equivalent_construct_with_size<TestObject>();
}

TEST_F(ContainerEquivalenceTest, ConstructWithSizeEquivalence_Float) {
    test_containers_equivalent_construct_with_size<float>();
}

TEST_F(ContainerEquivalenceTest, ConstructAndFillEquivalence_TestObject) {
    test_containers_equivalent_construct_and_fill<TestObject>();
}

TEST_F(ContainerEquivalenceTest, ConstructAndFillEquivalence_Float) {
    test_containers_equivalent_construct_and_fill<float>();
}

TEST_F(ContainerEquivalenceTest, CopyOperationsEquivalence_TestObject) {
    test_containers_equivalent_copy_operations<TestObject>();
}

TEST_F(ContainerEquivalenceTest, CopyOperationsEquivalence_Float) {
    test_containers_equivalent_copy_operations<float>();
}

TEST_F(ContainerEquivalenceTest, ResizeOperationsEquivalence_TestObject) {
    test_containers_equivalent_resize_operations<TestObject>();
}

TEST_F(ContainerEquivalenceTest, ResizeOperationsEquivalence_Float) {
    test_containers_equivalent_resize_operations<float>();
}

TEST_F(ContainerEquivalenceTest, MixedOperationsEquivalence_TestObject) {
    test_containers_equivalent_mixed_operations<TestObject>();
}

TEST_F(ContainerEquivalenceTest, MixedOperationsEquivalence_Float) {
    test_containers_equivalent_mixed_operations<float>();
}

TEST_F(ContainerEquivalenceTest, STLAlgorithmsEquivalence_TestObject) {
    test_containers_equivalent_stl_algorithms<TestObject>();
}

TEST_F(ContainerEquivalenceTest, STLAlgorithmsEquivalence_Float) {
    test_containers_equivalent_stl_algorithms<float>();
}

TEST_F(ContainerEquivalenceTest, PageBoundaryAccessEquivalence_TestObject) {
    test_containers_equivalent_page_boundary_access<TestObject>();
}

TEST_F(ContainerEquivalenceTest, PageBoundaryAccessEquivalence_Float) {
    test_containers_equivalent_page_boundary_access<float>();
}

TEST_F(ContainerEquivalenceTest, LargeObjectsEquivalence) {
    std::vector<LargeObject> std_vec;
    chunked_vector<LargeObject> chunked_vec;
    
    test_large_objects(std_vec);
    test_large_objects(chunked_vec);
    
    EXPECT_TRUE(containers_equal(std_vec, chunked_vec));
    EXPECT_TRUE(containers_equal_iterators(std_vec, chunked_vec));
}

// Additional edge case tests for equivalence
TEST_F(ContainerEquivalenceTest, EdgeCasesEquivalence) {
    // Test empty containers
    std::vector<int> std_empty;
    chunked_vector<int> chunked_empty;
    EXPECT_TRUE(containers_equal(std_empty, chunked_empty));
    EXPECT_TRUE(containers_equal_iterators(std_empty, chunked_empty));
    
    // Test single element
    std::vector<int> std_single{42};
    chunked_vector<int> chunked_single{42};
    EXPECT_TRUE(containers_equal(std_single, chunked_single));
    EXPECT_TRUE(containers_equal_iterators(std_single, chunked_single));
    
    // Test with different page sizes
    chunked_vector<int, 4> small_page_vec;
    std::vector<int> std_vec;
    
    for (int i = 0; i < 20; ++i) {
        small_page_vec.push_back(i);
        std_vec.push_back(i);
    }
    
    EXPECT_TRUE(containers_equal(std_vec, small_page_vec));
    EXPECT_TRUE(containers_equal_iterators(std_vec, small_page_vec));
}

// Test that sizes also remain equivalent during operations
TEST_F(ContainerEquivalenceTest, SizeEquivalenceTracking) {
    std::vector<TestObject> std_vec;
    chunked_vector<TestObject> chunked_vec;
    
    // Initial state
    EXPECT_EQ(std_vec.size(), chunked_vec.size());
    EXPECT_EQ(std_vec.empty(), chunked_vec.empty());
    
    // After adding elements
    test_push_back(std_vec, 100);
    test_push_back(chunked_vec, 100);
    
    EXPECT_EQ(std_vec.size(), chunked_vec.size());
    EXPECT_EQ(std_vec.empty(), chunked_vec.empty());
    
    // After resize
    std_vec.resize(50);
    chunked_vec.resize(50);
    
    EXPECT_EQ(std_vec.size(), chunked_vec.size());
    EXPECT_EQ(std_vec.empty(), chunked_vec.empty());
    
    // After clear
    std_vec.clear();
    chunked_vec.clear();
    
    EXPECT_EQ(std_vec.size(), chunked_vec.size());
    EXPECT_EQ(std_vec.empty(), chunked_vec.empty());
}

// Main function is not needed as we use gtest_main

// ============================================================================
// Page-by-Page Optimization Coverage Tests
// ============================================================================

class PageByPageOptimizationTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        TestObject::constructor_calls = 0;
        TestObject::destructor_calls = 0;
        TestObject::copy_calls = 0;
        TestObject::move_calls = 0;
    }
    void TearDown() override {}
};

// Test optimized clear() with non-trivial types across multiple pages
TEST_F(PageByPageOptimizationTest, ClearNonTrivialMultiplePages)
{
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<TestObject, PAGE_SIZE> vec;
    
    // Fill multiple pages with non-trivial objects
    const size_t num_elements = PAGE_SIZE * 3 + 2; // 3 full pages + 2 elements
    for (size_t i = 0; i < num_elements; ++i)
    {
        vec.emplace_back(static_cast<int>(i * 10));
    }
    
    EXPECT_EQ(vec.size(), num_elements);
    int destructors_before = TestObject::destructor_calls;
    
    // Clear should use page-by-page optimization
    vec.clear();
    
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
    
    // Should have called destructors for all elements
    EXPECT_EQ(TestObject::destructor_calls - destructors_before, static_cast<int>(num_elements));
}

// Test optimized clear() with trivial types (should not call destructors)
TEST_F(PageByPageOptimizationTest, ClearTrivialMultiplePages)
{
    constexpr size_t PAGE_SIZE = 8;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Fill multiple pages
    const size_t num_elements = PAGE_SIZE * 5 + 3;
    for (size_t i = 0; i < num_elements; ++i)
    {
        vec.push_back(static_cast<int>(i));
    }
    
    EXPECT_EQ(vec.size(), num_elements);
    
    // Clear should use optimized path for trivial types (no destructor calls)
    vec.clear();
    
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.size(), 0);
}

// Test optimized resize shrink with non-trivial types across multiple pages
TEST_F(PageByPageOptimizationTest, ResizeShrinkNonTrivialMultiplePages)
{
    constexpr size_t PAGE_SIZE = 6;
    chunked_vector<TestObject, PAGE_SIZE> vec;
    
    // Fill multiple pages
    const size_t initial_size = PAGE_SIZE * 4 + 1; // 4 full pages + 1 element
    for (size_t i = 0; i < initial_size; ++i)
    {
        vec.emplace_back(static_cast<int>(i * 5));
    }
    
    EXPECT_EQ(vec.size(), initial_size);
    int destructors_before = TestObject::destructor_calls;
    
    // Resize to smaller size (should trigger page-by-page destruction)
    const size_t new_size = PAGE_SIZE + 3; // 1 full page + 3 elements
    vec.resize(new_size);
    
    EXPECT_EQ(vec.size(), new_size);
    
    // Should have called destructors for removed elements
    size_t destroyed_elements = initial_size - new_size;
    EXPECT_EQ(TestObject::destructor_calls - destructors_before, static_cast<int>(destroyed_elements));
    
    // Verify remaining elements are correct
    for (size_t i = 0; i < new_size; ++i)
    {
        EXPECT_EQ(vec[i].value, static_cast<int>(i * 5));
    }
}

// Test optimized resize shrink with value parameter
TEST_F(PageByPageOptimizationTest, ResizeShrinkWithValueNonTrivialMultiplePages)
{
    constexpr size_t PAGE_SIZE = 5;
    chunked_vector<TestObject, PAGE_SIZE> vec;
    
    // Fill multiple pages
    const size_t initial_size = PAGE_SIZE * 3 + 4;
    for (size_t i = 0; i < initial_size; ++i)
    {
        vec.emplace_back(static_cast<int>(i * 7));
    }
    
    int destructors_before = TestObject::destructor_calls;
    
    // Resize to smaller size with value (should only destroy, not use the value)
    const size_t new_size = PAGE_SIZE * 2;
    TestObject unused_value(999);
    vec.resize(new_size, unused_value);
    
    EXPECT_EQ(vec.size(), new_size);
    
    // Should have destroyed the excess elements
    size_t destroyed_elements = initial_size - new_size;
    EXPECT_GT(TestObject::destructor_calls - destructors_before, static_cast<int>(destroyed_elements - 1));
    
    // Verify remaining elements are original values (not the resize value)
    for (size_t i = 0; i < new_size; ++i)
    {
        EXPECT_EQ(vec[i].value, static_cast<int>(i * 7));
    }
}

// Test optimized bulk_copy_from with trivial types (should use memcpy)
TEST_F(PageByPageOptimizationTest, BulkCopyTrivialTypes)
{
    constexpr size_t PAGE_SIZE = 8;
    chunked_vector<int, PAGE_SIZE> source;
    
    // Fill source with multiple pages
    const size_t num_elements = PAGE_SIZE * 3 + 5;
    for (size_t i = 0; i < num_elements; ++i)
    {
        source.push_back(static_cast<int>(i * 2));
    }
    
    // Copy should use optimized bulk_copy_from with memcpy
    chunked_vector<int, PAGE_SIZE> dest;
    dest = source;
    
    EXPECT_EQ(dest.size(), source.size());
    
    // Verify all elements copied correctly
    for (size_t i = 0; i < num_elements; ++i)
    {
        EXPECT_EQ(dest[i], source[i]);
        EXPECT_EQ(dest[i], static_cast<int>(i * 2));
    }
}

// Test optimized copy assignment with non-trivial types (page-by-page access)
TEST_F(PageByPageOptimizationTest, CopyAssignmentNonTrivialTypes)
{
    constexpr size_t PAGE_SIZE = 6;
    chunked_vector<TestObject, PAGE_SIZE> source;
    
    // Fill source with multiple pages
    const size_t num_elements = PAGE_SIZE * 4 + 2;
    for (size_t i = 0; i < num_elements; ++i)
    {
        source.emplace_back(static_cast<int>(i * 3));
    }
    
    // Copy assignment should use page-by-page optimization for non-trivial types
    chunked_vector<TestObject, PAGE_SIZE> dest;
    dest = source;
    
    EXPECT_EQ(dest.size(), source.size());
    
    // Verify all elements copied correctly
    for (size_t i = 0; i < num_elements; ++i)
    {
        EXPECT_EQ(dest[i].value, source[i].value);
        EXPECT_EQ(dest[i].value, static_cast<int>(i * 3));
    }
}

// Test erase single element with page-by-page move optimization
TEST_F(PageByPageOptimizationTest, EraseSingleElementMultiplePages)
{
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<TestObject, PAGE_SIZE> vec;
    
    // Fill multiple pages
    const size_t num_elements = PAGE_SIZE * 5;
    for (size_t i = 0; i < num_elements; ++i)
    {
        vec.emplace_back(static_cast<int>(i * 10));
    }
    
    // Erase element from middle of second page
    size_t erase_index = PAGE_SIZE + 1;
    auto it = vec.begin();
    std::advance(it, erase_index);
    
    int destructors_before = TestObject::destructor_calls;
    auto result_it = vec.erase(it);
    
    EXPECT_EQ(vec.size(), num_elements - 1);
    
    // Should have called destructors during the move operations
    EXPECT_GT(TestObject::destructor_calls - destructors_before, 0);
    
    // Verify elements shifted correctly
    for (size_t i = 0; i < erase_index; ++i)
    {
        EXPECT_EQ(vec[i].value, static_cast<int>(i * 10));
    }
    for (size_t i = erase_index; i < vec.size(); ++i)
    {
        EXPECT_EQ(vec[i].value, static_cast<int>((i + 1) * 10));
    }
}

// Test erase range with page-by-page optimization
TEST_F(PageByPageOptimizationTest, EraseRangeMultiplePages)
{
    constexpr size_t PAGE_SIZE = 5;
    chunked_vector<TestObject, PAGE_SIZE> vec;
    
    // Fill multiple pages
    const size_t num_elements = PAGE_SIZE * 6;
    for (size_t i = 0; i < num_elements; ++i)
    {
        vec.emplace_back(static_cast<int>(i * 5));
    }
    
    // Erase range spanning multiple pages
    size_t first_index = PAGE_SIZE + 2;     // Middle of second page
    size_t last_index = PAGE_SIZE * 4 + 1;  // Middle of fifth page
    
    auto first_it = vec.begin();
    auto last_it = vec.begin();
    std::advance(first_it, first_index);
    std::advance(last_it, last_index);
    
    int destructors_before = TestObject::destructor_calls;
    auto result_it = vec.erase(first_it, last_it);
    
    size_t expected_size = num_elements - (last_index - first_index);
    EXPECT_EQ(vec.size(), expected_size);
    
    // Should have called destructors for both erased and moved elements
    EXPECT_GT(TestObject::destructor_calls - destructors_before, 0);
    
    // Verify elements before erase range are unchanged
    for (size_t i = 0; i < first_index; ++i)
    {
        EXPECT_EQ(vec[i].value, static_cast<int>(i * 5));
    }
    
    // Verify elements after erase range shifted correctly
    for (size_t i = first_index; i < vec.size(); ++i)
    {
        size_t original_index = i + (last_index - first_index);
        EXPECT_EQ(vec[i].value, static_cast<int>(original_index * 5));
    }
}

// Test edge case: resize shrink to exactly page boundary
TEST_F(PageByPageOptimizationTest, ResizeShrinkToPageBoundary)
{
    constexpr size_t PAGE_SIZE = 8;
    chunked_vector<TestObject, PAGE_SIZE> vec;
    
    // Fill exactly 3 pages plus some extra
    const size_t initial_size = PAGE_SIZE * 3 + 5;
    for (size_t i = 0; i < initial_size; ++i)
    {
        vec.emplace_back(static_cast<int>(i));
    }
    
    int destructors_before = TestObject::destructor_calls;
    
    // Resize to exactly a page boundary
    vec.resize(PAGE_SIZE * 2);
    
    EXPECT_EQ(vec.size(), PAGE_SIZE * 2);
    
    // Should have destroyed the excess elements
    EXPECT_GT(TestObject::destructor_calls - destructors_before, 0);
    
    // Verify remaining elements
    for (size_t i = 0; i < PAGE_SIZE * 2; ++i)
    {
        EXPECT_EQ(vec[i].value, static_cast<int>(i));
    }
}

// Test edge case: clear with partial last page
TEST_F(PageByPageOptimizationTest, ClearWithPartialLastPage)
{
    constexpr size_t PAGE_SIZE = 6;
    chunked_vector<TestObject, PAGE_SIZE> vec;
    
    // Fill pages with partial last page
    const size_t num_elements = PAGE_SIZE * 2 + 3; // 2 full pages + 3 elements
    for (size_t i = 0; i < num_elements; ++i)
    {
        vec.emplace_back(static_cast<int>(i * 7));
    }
    
    int destructors_before = TestObject::destructor_calls;
    
    vec.clear();
    
    EXPECT_TRUE(vec.empty());
    
    // Should have destroyed all elements
    EXPECT_EQ(TestObject::destructor_calls - destructors_before, static_cast<int>(num_elements));
}

// Test edge case: resize shrink with partial pages
TEST_F(PageByPageOptimizationTest, ResizeShrinkPartialPages)
{
    constexpr size_t PAGE_SIZE = 7;
    chunked_vector<TestObject, PAGE_SIZE> vec;
    
    // Fill with non-aligned sizes
    const size_t initial_size = PAGE_SIZE * 2 + 4;
    for (size_t i = 0; i < initial_size; ++i)
    {
        vec.emplace_back(static_cast<int>(i * 9));
    }
    
    int destructors_before = TestObject::destructor_calls;
    
    // Resize to middle of first page
    const size_t new_size = 3;
    vec.resize(new_size);
    
    EXPECT_EQ(vec.size(), new_size);
    
    // Should have destroyed many elements
    size_t destroyed = initial_size - new_size;
    EXPECT_EQ(TestObject::destructor_calls - destructors_before, static_cast<int>(destroyed));
    
    // Verify remaining elements
    for (size_t i = 0; i < new_size; ++i)
    {
        EXPECT_EQ(vec[i].value, static_cast<int>(i * 9));
    }
}

// Test copy assignment with existing content (should clear first)
TEST_F(PageByPageOptimizationTest, CopyAssignmentWithExistingContent)
{
    constexpr size_t PAGE_SIZE = 5;
    
    // Create source vector
    chunked_vector<TestObject, PAGE_SIZE> source;
    for (size_t i = 0; i < PAGE_SIZE * 2; ++i)
    {
        source.emplace_back(static_cast<int>(i * 10));
    }
    
    // Create destination with different content
    chunked_vector<TestObject, PAGE_SIZE> dest;
    for (size_t i = 0; i < PAGE_SIZE * 3; ++i)
    {
        dest.emplace_back(static_cast<int>(i * 100));
    }
    
    EXPECT_EQ(dest.size(), PAGE_SIZE * 3);
    
    // Assignment should clear existing content and copy new content
    dest = source;
    
    EXPECT_EQ(dest.size(), source.size());
    
    // Verify all elements match source
    for (size_t i = 0; i < source.size(); ++i)
    {
        EXPECT_EQ(dest[i].value, source[i].value);
        EXPECT_EQ(dest[i].value, static_cast<int>(i * 10));
    }
}

// Test bulk_copy_from with exact page boundaries
TEST_F(PageByPageOptimizationTest, BulkCopyExactPageBoundaries)
{
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<int, PAGE_SIZE> source;
    
    // Fill exactly 5 pages
    const size_t num_elements = PAGE_SIZE * 5;
    for (size_t i = 0; i < num_elements; ++i)
    {
        source.push_back(static_cast<int>(i * 3));
    }
    
    chunked_vector<int, PAGE_SIZE> dest;
    dest = source;
    
    EXPECT_EQ(dest.size(), num_elements);
    
    // Verify all elements
    for (size_t i = 0; i < num_elements; ++i)
    {
        EXPECT_EQ(dest[i], static_cast<int>(i * 3));
    }
}

// Test erase with trivial types (should not call destructors but still optimize)
TEST_F(PageByPageOptimizationTest, EraseTrivialTypesMultiplePages)
{
    constexpr size_t PAGE_SIZE = 6;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Fill multiple pages
    const size_t num_elements = PAGE_SIZE * 4;
    for (size_t i = 0; i < num_elements; ++i)
    {
        vec.push_back(static_cast<int>(i));
    }
    
    // Erase range in middle
    auto first = vec.begin();
    auto last = vec.begin();
    std::advance(first, PAGE_SIZE + 2);
    std::advance(last, PAGE_SIZE * 3 - 1);
    
    vec.erase(first, last);
    
    size_t expected_size = num_elements - (PAGE_SIZE * 2 - 3);
    EXPECT_EQ(vec.size(), expected_size);
    
    // Verify elements are correct (no destructor tracking for trivial types)
    for (size_t i = 0; i < PAGE_SIZE + 2; ++i)
    {
        EXPECT_EQ(vec[i], static_cast<int>(i));
    }
}

// Test geometric growth calculation coverage
TEST_F(PageByPageOptimizationTest, GeometricGrowthCalculation)
{
    constexpr size_t PAGE_SIZE = 8;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Start with small capacity
    vec.reserve(PAGE_SIZE * 2);
    
    // Add elements to trigger multiple growth cycles
    for (size_t i = 0; i < PAGE_SIZE * 16; ++i)
    {
        vec.push_back(static_cast<int>(i));
    }
    
    EXPECT_EQ(vec.size(), PAGE_SIZE * 16);
    
    // Verify all elements
    for (size_t i = 0; i < PAGE_SIZE * 16; ++i)
    {
        EXPECT_EQ(vec[i], static_cast<int>(i));
    }
}

// Test max_page_capacity limits
TEST_F(PageByPageOptimizationTest, MaxPageCapacityLimits)
{
    chunked_vector<int> vec;
    
    // Test max_size() which uses max_page_capacity()
    size_t max_size = vec.max_size();
    EXPECT_GT(max_size, 0);
    
    // The actual value depends on the system, but should be reasonable
    EXPECT_GT(max_size, 1000000); // Should support at least a million elements
}

// Test edge case with single page operations
TEST_F(PageByPageOptimizationTest, SinglePageOperations)
{
    constexpr size_t PAGE_SIZE = 10;
    chunked_vector<TestObject, PAGE_SIZE> vec;
    
    // Fill less than one page
    for (size_t i = 0; i < 7; ++i)
    {
        vec.emplace_back(static_cast<int>(i));
    }
    
    int destructors_before = TestObject::destructor_calls;
    
    // Clear should still work with page-by-page logic for single page
    vec.clear();
    
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(TestObject::destructor_calls - destructors_before, 7);
}

// Test ensure_capacity_for_one_more with exact page boundary
TEST_F(PageByPageOptimizationTest, EnsureCapacityPageBoundary)
{
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Fill exactly to page boundary
    for (size_t i = 0; i < PAGE_SIZE; ++i)
    {
        vec.push_back(static_cast<int>(i));
    }
    
    EXPECT_EQ(vec.size(), PAGE_SIZE);
    
    // Adding one more should trigger new page allocation
    vec.push_back(static_cast<int>(PAGE_SIZE));
    
    EXPECT_EQ(vec.size(), PAGE_SIZE + 1);
    EXPECT_EQ(vec[PAGE_SIZE], static_cast<int>(PAGE_SIZE));
}

// Test allocate_page assertion coverage
TEST_F(PageByPageOptimizationTest, AllocatePageUpdateCount)
{
    constexpr size_t PAGE_SIZE = 8;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Reserve capacity but don't fill
    vec.reserve(PAGE_SIZE * 3);
    
    // Add elements one by one to trigger individual page allocations
    for (size_t i = 0; i < PAGE_SIZE * 2 + 3; ++i)
    {
        vec.push_back(static_cast<int>(i));
    }
    
    EXPECT_EQ(vec.size(), PAGE_SIZE * 2 + 3);
    
    // Verify all elements
    for (size_t i = 0; i < vec.size(); ++i)
    {
        EXPECT_EQ(vec[i], static_cast<int>(i));
    }
}

// ============================================================================
// Additional Coverage Tests for Edge Cases
// ============================================================================

// Test trivial vs non-trivial type optimizations in resize
TEST_F(PageByPageOptimizationTest, ResizeTrivialVsNonTrivialTypes)
{
    constexpr size_t PAGE_SIZE = 4;
    
    // Test with trivial types (should not call destructors)
    chunked_vector<int, PAGE_SIZE> trivial_vec;
    for (size_t i = 0; i < PAGE_SIZE * 3; ++i)
    {
        trivial_vec.push_back(static_cast<int>(i));
    }
    
    // Resize smaller - should just change size for trivial types
    trivial_vec.resize(PAGE_SIZE);
    EXPECT_EQ(trivial_vec.size(), PAGE_SIZE);
    
    // Test with non-trivial types (should call destructors)
    chunked_vector<TestObject, PAGE_SIZE> nontrivial_vec;
    for (size_t i = 0; i < PAGE_SIZE * 3; ++i)
    {
        nontrivial_vec.emplace_back(static_cast<int>(i));
    }
    
    int destructors_before = TestObject::destructor_calls;
    nontrivial_vec.resize(PAGE_SIZE);
    EXPECT_EQ(nontrivial_vec.size(), PAGE_SIZE);
    
    // Should have called destructors for non-trivial types
    EXPECT_GT(TestObject::destructor_calls - destructors_before, 0);
}

// Test bulk construction optimizations
TEST_F(PageByPageOptimizationTest, BulkConstructionOptimizations)
{
    constexpr size_t PAGE_SIZE = 6;
    
    // Test bulk_construct_default with trivial arithmetic types
    chunked_vector<int, PAGE_SIZE> int_vec;
    int_vec.resize(PAGE_SIZE * 2 + 3); // Should use memset optimization
    
    EXPECT_EQ(int_vec.size(), PAGE_SIZE * 2 + 3);
    for (size_t i = 0; i < int_vec.size(); ++i)
    {
        EXPECT_EQ(int_vec[i], 0); // Should be zero-initialized
    }
    
    // Test bulk_construct_default with trivial pointer types
    chunked_vector<void*, PAGE_SIZE> ptr_vec;
    ptr_vec.resize(PAGE_SIZE + 2); // Should use memset optimization
    
    EXPECT_EQ(ptr_vec.size(), PAGE_SIZE + 2);
    for (size_t i = 0; i < ptr_vec.size(); ++i)
    {
        EXPECT_EQ(ptr_vec[i], nullptr); // Should be zero-initialized
    }
    
    // Test bulk_construct_with_value with trivial types
    chunked_vector<double, PAGE_SIZE> double_vec;
    double_vec.resize(PAGE_SIZE * 2, 3.14); // Should use assignment optimization
    
    EXPECT_EQ(double_vec.size(), PAGE_SIZE * 2);
    for (size_t i = 0; i < double_vec.size(); ++i)
    {
        EXPECT_DOUBLE_EQ(double_vec[i], 3.14);
    }
}

// Test edge cases with empty containers
TEST_F(PageByPageOptimizationTest, EmptyContainerEdgeCases)
{
    chunked_vector<TestObject> vec;
    
    // Clear empty container
    vec.clear();
    EXPECT_TRUE(vec.empty());
    
    // Resize empty container to zero
    vec.resize(0);
    EXPECT_TRUE(vec.empty());
    
    // Copy assignment from empty to empty
    chunked_vector<TestObject> vec2;
    vec2 = vec;
    EXPECT_TRUE(vec2.empty());
    
    // Test copy assignment from empty to non-empty
    vec2.emplace_back(42);
    EXPECT_FALSE(vec2.empty());
    
    vec2 = vec; // Assign empty to non-empty
    EXPECT_TRUE(vec2.empty());
}

// Test iterator edge cases and caching
TEST_F(PageByPageOptimizationTest, IteratorEdgeCases)
{
    constexpr size_t PAGE_SIZE = 3;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Test iterator on empty container
    EXPECT_EQ(vec.begin(), vec.end());
    
    // Add elements to span multiple pages
    for (int i = 0; i < PAGE_SIZE * 2 + 1; ++i)
    {
        vec.push_back(i * 10);
    }
    
    // Test iterator increment across page boundaries
    auto it = vec.begin();
    for (int i = 0; i < PAGE_SIZE * 2 + 1; ++i)
    {
        EXPECT_EQ(*it, i * 10);
        if (i < PAGE_SIZE * 2)
        {
            ++it;
        }
    }
    
    // Test post-increment
    it = vec.begin();
    auto old_it = it++;
    EXPECT_EQ(*old_it, 0);
    EXPECT_EQ(*it, 10);
    
    // Test const iterator conversion
    const auto& const_vec = vec;
    auto const_it = const_vec.begin();
    chunked_vector<int, PAGE_SIZE>::const_iterator const_it2 = vec.begin(); // Non-const to const
    EXPECT_EQ(const_it, const_it2);
}

// Test power-of-2 vs non-power-of-2 page sizes
TEST_F(PageByPageOptimizationTest, PageSizeOptimizations)
{
    // Test power-of-2 page size (should use bit operations)
    chunked_vector<int, 8> power_of_2_vec; // 8 is power of 2
    for (size_t i = 0; i < 20; ++i)
    {
        power_of_2_vec.push_back(static_cast<int>(i));
    }
    
    EXPECT_EQ(power_of_2_vec.size(), 20);
    for (size_t i = 0; i < 20; ++i)
    {
        EXPECT_EQ(power_of_2_vec[i], static_cast<int>(i));
    }
    
    // Test non-power-of-2 page size (should use division/modulo)
    chunked_vector<int, 7> non_power_of_2_vec; // 7 is not power of 2
    for (size_t i = 0; i < 20; ++i)
    {
        non_power_of_2_vec.push_back(static_cast<int>(i * 2));
    }
    
    EXPECT_EQ(non_power_of_2_vec.size(), 20);
    for (size_t i = 0; i < 20; ++i)
    {
        EXPECT_EQ(non_power_of_2_vec[i], static_cast<int>(i * 2));
    }
}

// Test pop_back with trivial vs non-trivial types
TEST_F(PageByPageOptimizationTest, PopBackOptimizations)
{
    constexpr size_t PAGE_SIZE = 4;
    
    // Test pop_back with trivial types (should not call destructor)
    chunked_vector<int, PAGE_SIZE> trivial_vec;
    for (int i = 0; i < 10; ++i)
    {
        trivial_vec.push_back(i);
    }
    
    trivial_vec.pop_back();
    EXPECT_EQ(trivial_vec.size(), 9);
    EXPECT_EQ(trivial_vec.back(), 8);
    
    // Test pop_back with non-trivial types (should call destructor)
    chunked_vector<TestObject, PAGE_SIZE> nontrivial_vec;
    for (int i = 0; i < 10; ++i)
    {
        nontrivial_vec.emplace_back(i);
    }
    
    int destructors_before = TestObject::destructor_calls;
    nontrivial_vec.pop_back();
    EXPECT_EQ(nontrivial_vec.size(), 9);
    
    // Should have called destructor for non-trivial type
    EXPECT_GT(TestObject::destructor_calls - destructors_before, 0);
}

// Test deallocate_page_array coverage
TEST_F(PageByPageOptimizationTest, DeallocatePageArrayCoverage)
{
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Allocate multiple pages
    for (size_t i = 0; i < PAGE_SIZE * 3; ++i)
    {
        vec.push_back(static_cast<int>(i));
    }
    
    EXPECT_GT(vec.capacity(), 0);
    
    // Move assignment should deallocate the old page array
    chunked_vector<int, PAGE_SIZE> vec2;
    vec2.push_back(999);
    
    vec2 = std::move(vec);
    
    // Original vector should be empty, moved-to vector should have the data
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec2.size(), PAGE_SIZE * 3);
    
    // Destructor will also call deallocate_page_array
}

// Test shrink_to_fit with various scenarios
TEST_F(PageByPageOptimizationTest, ShrinkToFitScenarios)
{
    constexpr size_t PAGE_SIZE = 8;
    
    // Test shrink_to_fit with empty vector
    chunked_vector<int, PAGE_SIZE> empty_vec;
    empty_vec.shrink_to_fit();
    EXPECT_EQ(empty_vec.capacity(), 0);
    
    // Test shrink_to_fit with exactly full pages
    chunked_vector<int, PAGE_SIZE> full_pages_vec;
    for (size_t i = 0; i < PAGE_SIZE * 2; ++i)
    {
        full_pages_vec.push_back(static_cast<int>(i));
    }
    
    // Reserve extra capacity
    full_pages_vec.reserve(PAGE_SIZE * 5);
    size_t capacity_before = full_pages_vec.capacity();
    
    full_pages_vec.shrink_to_fit();
    
    EXPECT_LE(full_pages_vec.capacity(), capacity_before);
    EXPECT_GE(full_pages_vec.capacity(), full_pages_vec.size());
    
    // Verify elements are intact
    for (size_t i = 0; i < PAGE_SIZE * 2; ++i)
    {
        EXPECT_EQ(full_pages_vec[i], static_cast<int>(i));
    }
}

// Test emplace_back return value and reference
TEST_F(PageByPageOptimizationTest, EmplaceBackReturnValue)
{
    chunked_vector<TestObject> vec;
    
    // Test that emplace_back returns a reference to the constructed object
    auto& ref = vec.emplace_back(42);
    EXPECT_EQ(ref.value, 42);
    EXPECT_EQ(&ref, &vec.back()); // Should be same object
    
    // Modify through returned reference
    ref.value = 100;
    EXPECT_EQ(vec.back().value, 100);
}

// Test calculate_page_growth with edge cases
TEST_F(PageByPageOptimizationTest, CalculatePageGrowthEdgeCases)
{
    constexpr size_t PAGE_SIZE = 16;
    chunked_vector<int, PAGE_SIZE> vec;
    
    // Start with zero capacity (should start with exactly what's needed)
    EXPECT_EQ(vec.capacity(), 0);
    
    // Reserve 1 page - should get exactly 1 page for initial allocation
    vec.reserve(PAGE_SIZE);
    EXPECT_GE(vec.capacity(), PAGE_SIZE);
    
    // Add elements to trigger geometric growth
    size_t initial_capacity = vec.capacity();
    
    // Fill current capacity
    for (size_t i = 0; i < initial_capacity; ++i)
    {
        vec.push_back(static_cast<int>(i));
    }
    
    // Add one more to trigger growth
    vec.push_back(static_cast<int>(initial_capacity));
    
    // Should have grown geometrically
    EXPECT_GT(vec.capacity(), initial_capacity);
}

// Test the safe_alignment_of template
TEST_F(PageByPageOptimizationTest, SafeAlignmentTest)
{
    // Test that safe_alignment_of works correctly
    // This is mostly a compile-time test, but we can verify basic functionality
    
    chunked_vector<char> char_vec;     // Small alignment
    chunked_vector<double> double_vec; // Larger alignment
    chunked_vector<int*> ptr_vec;      // Pointer alignment
    
    // Add elements to ensure allocation works with proper alignment
    char_vec.push_back('a');
    double_vec.push_back(3.14);
    ptr_vec.push_back(nullptr);
    
    EXPECT_EQ(char_vec.size(), 1);
    EXPECT_EQ(double_vec.size(), 1);
    EXPECT_EQ(ptr_vec.size(), 1);
    
    EXPECT_EQ(char_vec[0], 'a');
    EXPECT_DOUBLE_EQ(double_vec[0], 3.14);
    EXPECT_EQ(ptr_vec[0], nullptr);
}

// Test count_trailing_zeros function
TEST_F(PageByPageOptimizationTest, CountTrailingZerosTest)
{
    // Test with power-of-2 page sizes to ensure bit operations work
    chunked_vector<int, 1> vec1;     // 2^0
    chunked_vector<int, 2> vec2;     // 2^1  
    chunked_vector<int, 4> vec4;     // 2^2
    chunked_vector<int, 8> vec8;     // 2^3
    chunked_vector<int, 16> vec16;   // 2^4
    chunked_vector<int, 32> vec32;   // 2^5
    
    // Add elements across page boundaries to test index calculations
    for (int i = 0; i < 10; ++i)
    {
        vec1.push_back(i);
        vec2.push_back(i);
        vec4.push_back(i);
        vec8.push_back(i);
        vec16.push_back(i);
        vec32.push_back(i);
    }
    
    // Verify all work correctly
    EXPECT_EQ(vec1.size(), 10);
    EXPECT_EQ(vec2.size(), 10);
    EXPECT_EQ(vec4.size(), 10);
    EXPECT_EQ(vec8.size(), 10);
    EXPECT_EQ(vec16.size(), 10);
    EXPECT_EQ(vec32.size(), 10);
    
    // Verify element access across pages
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(vec1[i], i);
        EXPECT_EQ(vec2[i], i);
        EXPECT_EQ(vec4[i], i);
        EXPECT_EQ(vec8[i], i);
        EXPECT_EQ(vec16[i], i);
        EXPECT_EQ(vec32[i], i);
    }
}

// Test bulk operations with empty ranges
TEST_F(PageByPageOptimizationTest, BulkOperationsEmptyRanges)
{
    constexpr size_t PAGE_SIZE = 4;
    chunked_vector<TestObject, PAGE_SIZE> vec;
    
    // Test bulk_construct_default with empty range
    vec.resize(5);
    size_t old_size = vec.size();
    vec.resize(5); // Same size - should not construct anything
    EXPECT_EQ(vec.size(), old_size);
    
    // Test bulk_construct_with_value with empty range  
    vec.resize(5, TestObject(999)); // Same size - should not construct anything
    EXPECT_EQ(vec.size(), old_size);
}

// Test various iterator comparison scenarios
TEST_F(PageByPageOptimizationTest, IteratorComparisonScenarios)
{
    constexpr size_t PAGE_SIZE = 3;
    chunked_vector<int, PAGE_SIZE> vec;
    
    for (int i = 0; i < PAGE_SIZE * 2; ++i)
    {
        vec.push_back(i);
    }
    
    auto it1 = vec.begin();
    auto it2 = vec.begin();
    auto it3 = vec.end();
    
    // Test various comparison combinations
    EXPECT_TRUE(it1 == it2);
    EXPECT_FALSE(it1 != it2);
    EXPECT_TRUE(it1 != it3);
    EXPECT_FALSE(it1 == it3);
    
    // Test after increment
    ++it2;
    EXPECT_FALSE(it1 == it2);
    EXPECT_TRUE(it1 != it2);
    
    // Test iterator copy constructor
    auto it4(it1);
    EXPECT_TRUE(it1 == it4);
    EXPECT_FALSE(it1 != it4);
}