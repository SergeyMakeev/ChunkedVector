# chunked_vector

[![CI](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/ci.yml)
[![License](https://img.shields.io/github/license/SergeyMakeev/chunked_vector)](LICENSE)
[![Language](https://img.shields.io/github/languages/top/SergeyMakeev/chunked_vector)](https://github.com/SergeyMakeev/chunked_vector)

A high-performance, header-only C++17 chunked vector implementation with comprehensive testing.

## Features

- **Header-only library** - Easy to integrate into your projects
- **C++17 compatible** - Modern C++ with backward compatibility
- **Cross-platform** - Builds and runs on Windows, Linux, and macOS
- **Comprehensive testing** - Extensive test suite with GoogleTest
- **Memory efficient** - Chunked memory allocation strategy
- **STL compatible** - Works with standard algorithms and iterators

## Build Status

Each badge above represents a different aspect of our continuous integration:

- **Linux GCC**: Tests with GCC 11 (Release & Debug builds)
- **Linux Clang**: Tests with Clang 14 (Release & Debug builds)  
- **macOS**: Tests on macOS 13 and latest (Release & Debug builds)
- **Windows**: Tests with MSVC 2022 and 2019 (Release & Debug builds)
- **Coverage**: Code coverage analysis with gcov/lcov
- **Sanitizers**: Memory safety checks (AddressSanitizer, UndefinedBehaviorSanitizer, ThreadSanitizer)

## Quick Start

```cpp
#include "chunked_vector/chunked_vector.h"

int main() {
    dod::chunked_vector<int> vec;
    
    // Add some elements
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    
    // Use like std::vector
    for (const auto& item : vec) {
        std::cout << item << " ";
    }
    
    return 0;
}
```

## Building and Testing

### Prerequisites

- CMake 3.14 or higher
- C++17 compatible compiler
- Git (for GoogleTest dependency)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/[username]/chunked_vector.git
cd chunked_vector

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run tests
cd build
ctest --output-on-failure
```

### Running Tests Directly

```bash
# From build directory
./chunked_vector_tests
```

## Continuous Integration

This project uses GitHub Actions with separate workflows for different testing aspects:

- **Platform-specific workflows**: Separate workflows for Linux (GCC/Clang), macOS, and Windows
- **Multiple compilers**: GCC 11, Clang 14, MSVC 2019/2022
- **Build configurations**: Both Release and Debug builds for all platforms
- **Code coverage**: Dedicated workflow using gcov/lcov with Codecov integration
- **Memory safety**: Separate sanitizer workflow with AddressSanitizer, UndefinedBehaviorSanitizer, and ThreadSanitizer
- **Comprehensive testing**: GoogleTest framework with extensive test coverage across all configurations

## License

This project is licensed under the terms specified in the [LICENSE](LICENSE) file.
