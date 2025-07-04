#pragma once

#include <stdexcept>
#include <string>

namespace test_assertions {

// Simple exception for assertion failures to be used with EXPECT_THROW
class AssertionException : public std::runtime_error {
public:
    AssertionException(const std::string& message) : std::runtime_error(message) {}
};

} // namespace test_assertions

// Override the assertion macro to throw an exception for testing
#define CHUNKED_VEC_ASSERT(expression) \
    ((void)((expression) || (throw test_assertions::AssertionException("Assertion failed: " #expression), 0))) 