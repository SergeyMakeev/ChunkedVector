// Include custom assertion handler before chunked_vector header
#include "test_iterator_debug_assertions.h"
#include "chunked_vector/chunked_vector.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <algorithm>
#include <numeric>
#include <memory>

using namespace dod;

// Test fixture for iterator debugging
class ChunkedVectorIteratorDebugTest : public ::testing::Test
{
  protected:
    void SetUp() override {
        // Reset assertion state before each test
        assertion_guard.reset(new test_assertions::AssertionStateGuard());
    }
    
    void TearDown() override {
        // Clean up assertion guard
        assertion_guard.reset();
    }
    
    // Helper to check if assertion was triggered
    bool was_assertion_triggered() const {
        return assertion_guard && assertion_guard->was_assertion_triggered();
    }
    
    const std::string& get_last_assertion_expression() const {
        static const std::string empty;
        return assertion_guard ? assertion_guard->get_last_expression() : empty;
    }
    
  private:
    std::unique_ptr<test_assertions::AssertionStateGuard> assertion_guard;
};

// =============================================================================
// Iterator Debugging Level Tests
// =============================================================================

TEST_F(ChunkedVectorIteratorDebugTest, IteratorDebugLevelCompileTimeCheck)
{
    // This test verifies that the debug level is properly configured
    chunked_vector<int> vec;
    vec.push_back(1);
    auto it = vec.begin();
    
    // These should work regardless of debug level
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(it, vec.end());
}

TEST_F(ChunkedVectorIteratorDebugTest, CustomAssertionMechanismTest)
{
    // Test our custom assertion mechanism directly
    EXPECT_FALSE(was_assertion_triggered());
    
    // This should NOT trigger an assertion
    CHUNKED_VEC_ASSERT(true && "This should not trigger");
    EXPECT_FALSE(was_assertion_triggered());
    
    // This SHOULD trigger an assertion and throw an exception
    bool exception_caught = false;
    std::string exception_message;
    try {
        CHUNKED_VEC_ASSERT(false && "Test assertion message");
        FAIL() << "Expected assertion exception was not thrown";
    } catch (const test_assertions::AssertionException& e) {
        exception_caught = true;
        exception_message = e.get_expression();
        EXPECT_EQ(exception_message, "false && \"Test assertion message\"");
    }
    
    EXPECT_TRUE(exception_caught) << "Custom assertion exception should have been thrown";
    EXPECT_TRUE(was_assertion_triggered()) << "Assertion state should be triggered";
}

#if CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0

// =============================================================================
// Iterator Validation Tests (Debug Mode Only)
// =============================================================================

TEST_F(ChunkedVectorIteratorDebugTest, ValidIteratorAccess)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    auto it = vec.begin();
    
    // Valid iterator access should work
    EXPECT_EQ(*it, 0);
    EXPECT_EQ(&(*it), &vec[0]);
    
    ++it;
    EXPECT_EQ(*it, 1);
    
    // Test that we can iterate to the last element
    auto last_it = vec.begin();
    std::advance(last_it, 9);
    EXPECT_EQ(*last_it, 9);
}

TEST_F(ChunkedVectorIteratorDebugTest, IteratorInvalidationAfterPushBack)
{
    chunked_vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    
    auto it = vec.begin();
    EXPECT_EQ(*it, 1);
    
    // Iterator should be invalidated when adding elements that might cause reallocation
    // Force a page allocation
    for (int i = 0; i < static_cast<int>(vec.page_size()) + 10; ++i) {
        vec.push_back(i + 10);
    }
    
    // The iterator is now invalid due to potential page reallocation
    // Note: In debug mode, accessing invalid iterators should trigger assertions
    // We can't easily test assertion failures in unit tests, but we can verify
    // that new iterators work correctly
    auto new_it = vec.begin();
    EXPECT_EQ(*new_it, 1);
}

TEST_F(ChunkedVectorIteratorDebugTest, IteratorInvalidationAfterPopBack)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    auto it = vec.begin();
    std::advance(it, 8); // Point to element at index 8
    EXPECT_EQ(*it, 8);
    
    // Pop the last element
    vec.pop_back();
    
    // The iterator pointing to element 8 should still be valid
    // Create a new iterator to verify the container state
    auto new_it = vec.begin();
    std::advance(new_it, 8);
    EXPECT_EQ(*new_it, 8);
    EXPECT_EQ(vec.size(), 9);
}

TEST_F(ChunkedVectorIteratorDebugTest, IteratorInvalidationAfterClear)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    auto it = vec.begin();
    auto end_it = vec.end();
    
    EXPECT_EQ(*it, 0);
    EXPECT_NE(it, end_it);
    
    // Clear should invalidate all iterators
    vec.clear();
    
    // Verify container is empty and new iterators work
    EXPECT_TRUE(vec.empty());
    EXPECT_EQ(vec.begin(), vec.end());
}

TEST_F(ChunkedVectorIteratorDebugTest, IteratorInvalidationAfterResize)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    auto it = vec.begin();
    auto mid_it = vec.begin();
    std::advance(mid_it, 5);
    
    EXPECT_EQ(*it, 0);
    EXPECT_EQ(*mid_it, 5);
    
    // Resize smaller - should invalidate iterators beyond new size
    vec.resize(7);
    
    // Verify new state
    EXPECT_EQ(vec.size(), 7);
    auto new_it = vec.begin();
    EXPECT_EQ(*new_it, 0);
    
    auto new_end_it = vec.begin();
    std::advance(new_end_it, 6);
    EXPECT_EQ(*new_end_it, 6);
    
    // Resize larger
    vec.resize(15, 99);
    EXPECT_EQ(vec.size(), 15);
    
    new_it = vec.begin();
    std::advance(new_it, 10);
    EXPECT_EQ(*new_it, 99);
}

TEST_F(ChunkedVectorIteratorDebugTest, IteratorInvalidationAfterErase)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    auto it = vec.begin();
    std::advance(it, 5); // Point to element 5
    EXPECT_EQ(*it, 5);
    
    auto erase_it = vec.begin();
    std::advance(erase_it, 3); // Point to element 3
    
    // Erase element 3
    auto returned_it = vec.erase(erase_it);
    
    // Returned iterator should point to element that was at position 4 (now at position 3)
    EXPECT_EQ(*returned_it, 4);
    EXPECT_EQ(vec.size(), 9);
    
    // Verify elements shifted correctly
    auto check_it = vec.begin();
    for (int expected = 0; expected < 3; ++expected) {
        EXPECT_EQ(*check_it, expected);
        ++check_it;
    }
    // Skip the erased element (3)
    for (int expected = 4; expected < 10; ++expected) {
        EXPECT_EQ(*check_it, expected);
        ++check_it;
    }
}

TEST_F(ChunkedVectorIteratorDebugTest, IteratorInvalidationAfterEraseRange)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    auto first = vec.begin();
    std::advance(first, 3); // Position 3
    auto last = vec.begin();
    std::advance(last, 7);  // Position 7
    
    // Erase range [3, 7) - elements 3, 4, 5, 6
    auto returned_it = vec.erase(first, last);
    
    // Should point to element that was at position 7 (now at position 3)
    EXPECT_EQ(*returned_it, 7);
    EXPECT_EQ(vec.size(), 6);
    
    // Verify remaining elements: 0, 1, 2, 7, 8, 9
    std::vector<int> expected = {0, 1, 2, 7, 8, 9};
    auto it = vec.begin();
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(*it, expected[i]);
        ++it;
    }
}

TEST_F(ChunkedVectorIteratorDebugTest, IteratorInvalidationAfterMoveAssignment)
{
    chunked_vector<int> vec1;
    for (int i = 0; i < 5; ++i) {
        vec1.push_back(i);
    }
    
    chunked_vector<int> vec2;
    for (int i = 10; i < 15; ++i) {
        vec2.push_back(i);
    }
    
    auto it1 = vec1.begin();
    auto it2 = vec2.begin();
    
    EXPECT_EQ(*it1, 0);
    EXPECT_EQ(*it2, 10);
    
    // Move assignment should invalidate iterators from both containers
    vec2 = std::move(vec1);
    
    // vec1 should be empty now
    EXPECT_TRUE(vec1.empty());
    EXPECT_EQ(vec1.begin(), vec1.end());
    
    // vec2 should have vec1's old contents
    auto new_it = vec2.begin();
    for (int expected = 0; expected < 5; ++expected) {
        EXPECT_EQ(*new_it, expected);
        ++new_it;
    }
}

TEST_F(ChunkedVectorIteratorDebugTest, IteratorInvalidationAfterCopyAssignment)
{
    chunked_vector<int> vec1;
    for (int i = 0; i < 5; ++i) {
        vec1.push_back(i);
    }
    
    chunked_vector<int> vec2;
    for (int i = 10; i < 15; ++i) {
        vec2.push_back(i);
    }
    
    auto it2 = vec2.begin();
    EXPECT_EQ(*it2, 10);
    
    // Copy assignment should invalidate iterators from destination container
    vec2 = vec1;
    
    // vec1 should remain unchanged
    auto it1 = vec1.begin();
    EXPECT_EQ(*it1, 0);
    
    // vec2 should have vec1's contents
    auto new_it = vec2.begin();
    for (int expected = 0; expected < 5; ++expected) {
        EXPECT_EQ(*new_it, expected);
        ++new_it;
    }
}

// =============================================================================
// Iterator Range Verification Tests
// =============================================================================

TEST_F(ChunkedVectorIteratorDebugTest, ValidIteratorRange)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    auto first = vec.begin();
    auto last = vec.end();
    
    // Valid range operations should work with STL algorithms
    auto count = std::distance(first, last);
    EXPECT_EQ(count, 10);
    
    // Partial range
    auto mid = vec.begin();
    std::advance(mid, 5);
    auto count_first_half = std::distance(first, mid);
    auto count_second_half = std::distance(mid, last);
    EXPECT_EQ(count_first_half, 5);
    EXPECT_EQ(count_second_half, 5);
    
    // Empty range (same iterator)
    auto empty_count = std::distance(first, first);
    EXPECT_EQ(empty_count, 0);
}

TEST_F(ChunkedVectorIteratorDebugTest, IteratorFromSameContainer)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    
    auto it1 = vec.begin();
    auto it2 = vec.begin();
    std::advance(it2, 2);
    auto it3 = vec.end();
    
    // All iterators from same container
    EXPECT_EQ(it1, vec.begin());
    EXPECT_NE(it1, it2);
    EXPECT_NE(it2, it3);
    
    // Test iterator advancement and comparison
    ++it1;
    ++it1;
    EXPECT_EQ(it1, it2);
}

// =============================================================================
// Iterator Conversion Tests
// =============================================================================

TEST_F(ChunkedVectorIteratorDebugTest, ConstIteratorConversion)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 5; ++i) {
        vec.push_back(i);
    }
    
    // Non-const to const iterator conversion
    auto it = vec.begin();
    auto const_it = vec.cbegin();
    
    EXPECT_EQ(*it, *const_it);
    
    // Assignment from non-const to const
    chunked_vector<int>::const_iterator converted_it = it;
    EXPECT_EQ(*converted_it, *it);
    
    // Verify const iterator behavior
    const auto& const_vec = vec;
    auto const_begin = const_vec.begin();
    auto const_end = const_vec.end();
    
    int sum = 0;
    for (auto ci = const_begin; ci != const_end; ++ci) {
        sum += *ci;
    }
    EXPECT_EQ(sum, 0 + 1 + 2 + 3 + 4);
}

// =============================================================================
// Generation Tracking Tests
// =============================================================================

TEST_F(ChunkedVectorIteratorDebugTest, GenerationTracking)
{
    chunked_vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    
    auto it1 = vec.begin();
    EXPECT_EQ(*it1, 1);
    
    // Create another iterator
    auto it2 = vec.begin();
    ++it2;
    EXPECT_EQ(*it2, 2);
    
    // Both iterators should be valid
    EXPECT_EQ(*it1, 1);
    EXPECT_EQ(*it2, 2);
    
    // Modify container to change generation
    vec.clear();
    
    // Create new iterators after modification
    vec.push_back(10);
    vec.push_back(20);
    
    auto new_it = vec.begin();
    EXPECT_EQ(*new_it, 10);
}

// =============================================================================
// Page Boundary Iterator Tests
// =============================================================================

TEST_F(ChunkedVectorIteratorDebugTest, IteratorAcrossPageBoundaries)
{
    // Use small page size to test page boundary behavior
    chunked_vector<int, 4> vec; // 4 elements per page
    
    // Fill multiple pages
    for (int i = 0; i < 12; ++i) {
        vec.push_back(i);
    }
    
    auto it = vec.begin();
    
    // Iterate across page boundaries
    for (int expected = 0; expected < 12; ++expected) {
        EXPECT_EQ(*it, expected);
        ++it;
    }
    
    EXPECT_EQ(it, vec.end());
    
    // Test iteration from beginning again
    auto start_it = vec.begin();
    auto end_it = vec.end();
    int count = 0;
    for (auto test_it = start_it; test_it != end_it; ++test_it) {
        EXPECT_EQ(*test_it, count);
        ++count;
    }
    EXPECT_EQ(count, 12);
}

TEST_F(ChunkedVectorIteratorDebugTest, IteratorInvalidationAtPageBoundary)
{
    chunked_vector<int, 4> vec; // 4 elements per page
    
    // Fill exactly one page
    for (int i = 0; i < 4; ++i) {
        vec.push_back(i);
    }
    
    auto it = vec.begin();
    std::advance(it, 3); // Point to last element (3)
    EXPECT_EQ(*it, 3);
    
    // Add one more element (should allocate new page)
    vec.push_back(4);
    
    // Original iterator should still be valid (pointing to same element)
    auto new_it = vec.begin();
    std::advance(new_it, 3);
    EXPECT_EQ(*new_it, 3);
    
    // Check new element
    auto last_it = vec.begin();
    std::advance(last_it, 4);
    EXPECT_EQ(*last_it, 4);
}

// =============================================================================
// Erase Operation Iterator Tests
// =============================================================================

TEST_F(ChunkedVectorIteratorDebugTest, EraseUnsortedIteratorBehavior)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    auto it = vec.begin();
    std::advance(it, 3); // Point to element 3
    EXPECT_EQ(*it, 3);
    
    // Erase unsorted (swaps with last element)
    auto returned_it = vec.erase_unsorted(it);
    
    // Should point to the swapped element (originally element 9)
    EXPECT_EQ(*returned_it, 9);
    EXPECT_EQ(vec.size(), 9);
    
    // Verify the element was replaced
    auto check_it = vec.begin();
    std::advance(check_it, 3);
    EXPECT_EQ(*check_it, 9);
}

// =============================================================================
// Assertion Verification Tests (Debug Mode Only)
// =============================================================================

TEST_F(ChunkedVectorIteratorDebugTest, AssertionOnOutOfRangeAccess)
{
    chunked_vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    
    // Valid access should not trigger assertion
    auto it = vec.begin();
    EXPECT_EQ(*it, 1);
    EXPECT_FALSE(was_assertion_triggered());
    
    ++it;
    EXPECT_EQ(*it, 2);
    EXPECT_FALSE(was_assertion_triggered());
    
    // Access beyond end should trigger assertion
    ++it; // Now points to end()
    
    bool assertion_caught = false;
    std::string assertion_message;
    try {
        *it;  // This should trigger assertion: "Iterator out of range"
        FAIL() << "Expected assertion exception was not thrown";
    } catch (const test_assertions::AssertionException& e) {
        assertion_caught = true;
        assertion_message = e.get_expression();
        EXPECT_TRUE(assertion_message.find("out of range") != std::string::npos ||
                   assertion_message.find("index") != std::string::npos)
            << "Unexpected assertion message: " << assertion_message;
    }
    
    EXPECT_TRUE(assertion_caught) << "Assertion exception should have been thrown";
    EXPECT_TRUE(was_assertion_triggered()) << "Assertion state should be triggered";
}

TEST_F(ChunkedVectorIteratorDebugTest, AssertionOnInvalidatedIteratorAccess)
{
    chunked_vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    
    auto it = vec.begin();
    EXPECT_EQ(*it, 1);
    EXPECT_FALSE(was_assertion_triggered());
    
    // Clear invalidates all iterators
    vec.clear();
    
    // Accessing invalidated iterator should trigger assertion
    bool assertion_caught = false;
    std::string assertion_message;
    try {
        *it;  // This should trigger assertion: "Iterator has been invalidated"
        FAIL() << "Expected assertion exception was not thrown";
    } catch (const test_assertions::AssertionException& e) {
        assertion_caught = true;
        assertion_message = e.get_expression();
        EXPECT_TRUE(assertion_message.find("invalidated") != std::string::npos ||
                   assertion_message.find("generation") != std::string::npos)
            << "Unexpected assertion message: " << assertion_message;
    }
    
    EXPECT_TRUE(assertion_caught) << "Assertion exception should have been thrown";
    EXPECT_TRUE(was_assertion_triggered()) << "Assertion state should be triggered";
}

TEST_F(ChunkedVectorIteratorDebugTest, AssertionOnIteratorFromDifferentContainer)
{
    chunked_vector<int> vec1 = {1, 2, 3};
    chunked_vector<int> vec2 = {4, 5, 6};
    
    auto it1 = vec1.begin();
    auto it2 = vec2.begin();
    
    // Using iterator from different container should trigger assertion
    bool assertion_caught = false;
    std::string assertion_message;
    try {
        vec1.erase(it2);  // This should trigger assertion: "Iterator from different container"
        FAIL() << "Expected assertion exception was not thrown";
    } catch (const test_assertions::AssertionException& e) {
        assertion_caught = true;
        assertion_message = e.get_expression();
        EXPECT_TRUE(assertion_message.find("container") != std::string::npos ||
                   assertion_message.find("different") != std::string::npos)
            << "Unexpected assertion message: " << assertion_message;
    }
    
    EXPECT_TRUE(assertion_caught) << "Assertion exception should have been thrown";
    EXPECT_TRUE(was_assertion_triggered()) << "Assertion state should be triggered";
}

TEST_F(ChunkedVectorIteratorDebugTest, AssertionOnInvalidIteratorRange)
{
    chunked_vector<int> vec = {1, 2, 3, 4, 5};
    
    auto first = vec.begin();
    std::advance(first, 3);  // Position 3
    auto last = vec.begin();
    std::advance(last, 1);   // Position 1
    
    // Invalid range (first > last) should trigger assertion
    bool assertion_caught = false;
    std::string assertion_message;
    try {
        vec.erase(first, last);  // This should trigger assertion: "Invalid iterator range"
        FAIL() << "Expected assertion exception was not thrown";
    } catch (const test_assertions::AssertionException& e) {
        assertion_caught = true;
        assertion_message = e.get_expression();
        EXPECT_TRUE(assertion_message.find("range") != std::string::npos ||
                   assertion_message.find("index") != std::string::npos)
            << "Unexpected assertion message: " << assertion_message;
    }
    
    EXPECT_TRUE(assertion_caught) << "Assertion exception should have been thrown";
    EXPECT_TRUE(was_assertion_triggered()) << "Assertion state should be triggered";
}

TEST_F(ChunkedVectorIteratorDebugTest, AssertionOnIteratorAfterResize)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    auto it = vec.begin();
    std::advance(it, 8); // Point to element 8
    EXPECT_EQ(*it, 8);
    EXPECT_FALSE(was_assertion_triggered());
    
    // Resize to smaller size, invalidating iterator
    vec.resize(5);
    
    // Using iterator that's now beyond valid range should trigger assertion
    bool assertion_caught = false;
    std::string assertion_message;
    try {
        *it;  // This should trigger assertion since iterator is beyond new size
        FAIL() << "Expected assertion exception was not thrown";
    } catch (const test_assertions::AssertionException& e) {
        assertion_caught = true;
        assertion_message = e.get_expression();
        // Iterator beyond valid range after resize
        EXPECT_TRUE(assertion_message.find("out of range") != std::string::npos ||
                   assertion_message.find("index") != std::string::npos ||
                   assertion_message.find("invalidated") != std::string::npos)
            << "Unexpected assertion message: " << assertion_message;
    }
    
    EXPECT_TRUE(assertion_caught) << "Assertion exception should have been thrown";
    EXPECT_TRUE(was_assertion_triggered()) << "Assertion state should be triggered";
}

TEST_F(ChunkedVectorIteratorDebugTest, AssertionOnIteratorAfterMoveAssignment)
{
    chunked_vector<int> vec1 = {1, 2, 3};
    chunked_vector<int> vec2 = {4, 5, 6};
    
    auto it1 = vec1.begin();
    auto it2 = vec2.begin();
    
    EXPECT_EQ(*it1, 1);
    EXPECT_EQ(*it2, 4);
    EXPECT_FALSE(was_assertion_triggered());
    
    // Move assignment invalidates iterators from both containers
    vec2 = std::move(vec1);
    
    // Using invalidated iterator should trigger assertion
    bool assertion_caught = false;
    std::string assertion_message;
    try {
        *it1;  // This should trigger assertion since iterator was invalidated
        FAIL() << "Expected assertion exception was not thrown";
    } catch (const test_assertions::AssertionException& e) {
        assertion_caught = true;
        assertion_message = e.get_expression();
        EXPECT_TRUE(assertion_message.find("invalidated") != std::string::npos ||
                   assertion_message.find("generation") != std::string::npos ||
                   assertion_message.find("out of range") != std::string::npos)
            << "Unexpected assertion message: " << assertion_message;
    }
    
    EXPECT_TRUE(assertion_caught) << "Assertion exception should have been thrown";
    EXPECT_TRUE(was_assertion_triggered()) << "Assertion state should be triggered";
}

#else // CHUNKED_VEC_ITERATOR_DEBUG_LEVEL == 0

// =============================================================================
// Release Mode Tests (Debug Disabled)
// =============================================================================

TEST_F(ChunkedVectorIteratorDebugTest, NoDebugOverhead)
{
    // In release mode, iterators should work normally without debug overhead
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    auto it = vec.begin();
    EXPECT_EQ(*it, 0);
    
    // Even after modifications, iterators should work (though they may be invalid)
    vec.clear();
    vec.push_back(100);
    
    auto new_it = vec.begin();
    EXPECT_EQ(*new_it, 100);
}

#endif // CHUNKED_VEC_ITERATOR_DEBUG_LEVEL > 0

// =============================================================================
// Cross-Debug-Level Tests (Always Run)
// =============================================================================

TEST_F(ChunkedVectorIteratorDebugTest, BasicIteratorFunctionality)
{
    chunked_vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * 2);
    }
    
    // Test basic iterator operations
    auto it = vec.begin();
    EXPECT_EQ(*it, 0);
    
    ++it;
    EXPECT_EQ(*it, 2);
    
    auto it2 = it++;
    EXPECT_EQ(*it2, 2);  // Post-increment returns old value
    EXPECT_EQ(*it, 4);   // it now points to next element
    
    // Test equality/inequality
    auto another_it = vec.begin();
    std::advance(another_it, 2);
    EXPECT_EQ(it, another_it);
    
    ++it;
    EXPECT_NE(it, another_it);
}

TEST_F(ChunkedVectorIteratorDebugTest, STLAlgorithmCompatibility)
{
    chunked_vector<int> vec;
    for (int i = 1; i <= 10; ++i) {
        vec.push_back(i);
    }
    
    // Test with std::find
    auto found = std::find(vec.begin(), vec.end(), 5);
    EXPECT_NE(found, vec.end());
    EXPECT_EQ(*found, 5);
    
    // Test with std::count
    auto count = std::count_if(vec.begin(), vec.end(), [](int x) { return x % 2 == 0; });
    EXPECT_EQ(count, 5); // Even numbers: 2, 4, 6, 8, 10
    
    // Test with std::accumulate
    auto sum = std::accumulate(vec.begin(), vec.end(), 0);
    EXPECT_EQ(sum, 55); // Sum of 1..10
} 