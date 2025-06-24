cmake_minimum_required(VERSION 3.14)
project(chunked_vector_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()
include(GoogleTest)

# Add the chunked_vector subdirectory first
add_subdirectory("${PROJECT_SOURCE_DIR}/chunked_vector")

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add chunked vector test executable
add_executable(chunked_vector_tests
  chunked_vector_test.cpp
)

# Link Google Test and chunked_vector
target_link_libraries(chunked_vector_tests 
  gtest_main
  gmock_main
  chunked_vector
)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT chunked_vector_tests)

# For Windows MSVC
if(MSVC)
  target_compile_options(chunked_vector_tests PRIVATE /W4 /permissive-)
else()
  # For clang/gcc
  target_compile_options(chunked_vector_tests PRIVATE -Wall -Wextra -pedantic)
endif()

# Discover tests - use POST_BUILD mode to avoid CI issues across all platforms
gtest_discover_tests(chunked_vector_tests PROPERTIES DISCOVERY_MODE POST_BUILD) 