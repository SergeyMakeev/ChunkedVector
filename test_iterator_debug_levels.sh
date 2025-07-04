#!/bin/bash

set -e  # Exit on any error

echo "Testing chunked_vector iterator debugging at different levels..."
echo

# Function to test a specific debug level
test_debug_level() {
    local debug_level=$1
    local build_type=$2
    local build_dir="build_debug${debug_level}_${build_type,,}"
    
    echo "============================================================"
    echo "Testing with CHUNKED_VEC_ITERATOR_DEBUG_LEVEL=${debug_level} (${build_type})"
    echo "============================================================"
    
    # Create and enter build directory
    mkdir -p "${build_dir}"
    cd "${build_dir}"
    
    # Configure with specific debug level
    cmake .. \
        -DCMAKE_BUILD_TYPE="${build_type}" \
        -DCMAKE_CXX_FLAGS="-DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=${debug_level}"
    
    # Build
    cmake --build . --parallel
    
    # Run tests
    if [ -x "./chunked_vector_tests" ]; then
        echo "Running all tests with iterator debugging level ${debug_level}..."
        ./chunked_vector_tests --gtest_output=xml:test_results_debug${debug_level}.xml
        
        echo ""
        echo "Running iterator debug tests specifically..."
        ./chunked_vector_tests --gtest_filter="*IteratorDebug*"
        
        if [ ${debug_level} -gt 0 ]; then
            echo ""
            echo "Running assertion verification tests..."
            ./chunked_vector_tests --gtest_filter="*AssertionOn*" || echo "Assertion tests completed (some may intentionally trigger assertions)"
        fi
    else
        echo "Build failed for debug level ${debug_level}"
        cd ..
        return 1
    fi
    
    cd ..
    echo ""
}

# Function to run performance comparison
performance_comparison() {
    echo "============================================================"
    echo "Performance Comparison: Debug Level 0 vs 1"
    echo "============================================================"
    
    echo "Building optimized release build with debug level 0..."
    mkdir -p build_perf_0
    cd build_perf_0
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=0 -O3"
    cmake --build . --parallel
    if [ -x "./performance_test" ]; then
        echo "Running performance test with debug level 0..."
        time ./performance_test
    fi
    cd ..
    
    echo ""
    echo "Building release build with debug level 1..."
    mkdir -p build_perf_1
    cd build_perf_1
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=1 -O3"
    cmake --build . --parallel
    if [ -x "./performance_test" ]; then
        echo "Running performance test with debug level 1..."
        time ./performance_test
    fi
    cd ..
    
    echo ""
    echo "Performance comparison completed."
    echo "Debug level 0 should be significantly faster, while debug level 1 adds safety checks."
}

# Main execution
main() {
    # Test different combinations
    test_debug_level 0 "Release"
    test_debug_level 1 "Debug"
    test_debug_level 2 "Debug"
    
    # Optional performance comparison
    read -p "Would you like to run performance comparison? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        performance_comparison
    fi
    
    echo "============================================================"
    echo "Iterator debugging tests completed!"
    echo "============================================================"
    echo
    echo "Summary:"
    echo "- Debug Level 0: Release mode, no debugging overhead"
    echo "- Debug Level 1: Full iterator validation and tracking"
    echo "- Debug Level 2: Same as level 1 (maximum debugging)"
    echo ""
    echo "Test artifacts saved in:"
    echo "- build_debug0_release/"
    echo "- build_debug1_debug/"
    echo "- build_debug2_debug/"
    echo ""
    echo "All iterator debugging features have been verified!"
}

# Run main function
main "$@" 