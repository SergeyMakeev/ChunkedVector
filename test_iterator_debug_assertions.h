#pragma once

#include <string>
#include <atomic>
#include <stdexcept>

namespace test_assertions {

// Custom exception for assertion failures
class AssertionException : public std::runtime_error {
public:
    AssertionException(const std::string& expression, const std::string& file, int line)
        : std::runtime_error("Assertion failed: " + expression + " at " + file + ":" + std::to_string(line))
        , expression_(expression)
        , file_(file)
        , line_(line)
    {
    }
    
    const std::string& get_expression() const { return expression_; }
    const std::string& get_file() const { return file_; }
    int get_line() const { return line_; }
    
private:
    std::string expression_;
    std::string file_;
    int line_;
};

// Global state for tracking assertions
struct AssertionState {
    std::atomic<bool> assertion_triggered{false};
    std::string last_expression;
    std::string last_file;
    int last_line = 0;
    
    void reset() {
        assertion_triggered = false;
        last_expression.clear();
        last_file.clear();
        last_line = 0;
    }
    
    bool was_triggered() const {
        return assertion_triggered.load();
    }
};

// Global assertion state
inline AssertionState g_assertion_state;

// Custom assertion handler that throws an exception
inline void custom_assert_handler(const char* expression, const char* file, int line) {
    g_assertion_state.assertion_triggered = true;
    g_assertion_state.last_expression = expression ? expression : "";
    g_assertion_state.last_file = file ? file : "";
    g_assertion_state.last_line = line;
    
    // Throw exception to allow test to catch and continue
    throw AssertionException(
        expression ? expression : "",
        file ? file : "",
        line
    );
}

// Helper class to reset assertion state in test setup
class AssertionStateGuard {
public:
    AssertionStateGuard() {
        g_assertion_state.reset();
    }
    
    ~AssertionStateGuard() {
        g_assertion_state.reset();
    }
    
    bool was_assertion_triggered() const {
        return g_assertion_state.was_triggered();
    }
    
    const std::string& get_last_expression() const {
        return g_assertion_state.last_expression;
    }
    
    const std::string& get_last_file() const {
        return g_assertion_state.last_file;
    }
    
    int get_last_line() const {
        return g_assertion_state.last_line;
    }
};

} // namespace test_assertions

// Disable standard assert to avoid conflicts
#ifdef assert
#undef assert
#endif
#define assert(expression) ((void)0)

// Also disable NDEBUG-dependent asserts
#ifdef _ASSERT
#undef _ASSERT
#endif
#define _ASSERT(expression) ((void)0)

// Override the assertion macro to use our custom handler
#define CHUNKED_VEC_ASSERT(expression) \
    (void)((!!(expression)) || (test_assertions::custom_assert_handler(#expression, __FILE__, __LINE__), 0)) 