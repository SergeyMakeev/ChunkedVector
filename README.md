# chunked_vector

[![License](https://img.shields.io/github/license/SergeyMakeev/chunked_vector)](LICENSE)
[![Language](https://img.shields.io/github/languages/top/SergeyMakeev/chunked_vector)](https://github.com/SergeyMakeev/chunked_vector)
[![codecov](https://codecov.io/gh/SergeyMakeev/chunked_vector/graph/badge.svg?token=8B80XIGDVT)](https://codecov.io/gh/SergeyMakeev/chunked_vector)


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

[![Linux GCC](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/linux-gcc.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/linux-gcc.yml)  
[![Linux Clang](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/linux-clang.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/linux-clang.yml)  
[![macOS](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/macos.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/macos.yml)  
[![Windows](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/windows.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/windows.yml)  
[![Coverage](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/coverage.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/coverage.yml)  
[![Sanitizers](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/sanitizers.yml/badge.svg?branch=main)](https://github.com/SergeyMakeev/chunked_vector/actions/workflows/sanitizers.yml)  

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

## Building and Usage

### Prerequisites

- CMake 3.14 or higher
- C++17 compatible compiler
- Git (for GoogleTest dependency)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/SergeyMakeev/chunked_vector.git
cd chunked_vector

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Run tests
cd build
ctest --output-on-failure
```

### Usage

```
# Add the chunked_vector library
add_subdirectory("path_to_chunked_vector_repo/chunked_vector")

# use chunked_vector library
target_link_libraries(my_app_name PRIVATE chunked_vector)
```


## License

This project is licensed under the terms specified in the [LICENSE](LICENSE) file.
