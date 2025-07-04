#!/bin/bash

echo "Testing chunked_vector iterator debugging at different levels..."
echo

# Test with debug level 0 (disabled)
echo "============================================================"
echo "Testing with CHUNKED_VEC_ITERATOR_DEBUG_LEVEL=0 (Release)"
echo "============================================================"
mkdir -p build_debug0
cd build_debug0
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=0"
cmake --build .
if [ -x "./chunked_vector_tests" ]; then
    echo "Running tests with iterator debugging disabled..."
    ./chunked_vector_tests --gtest_filter="*IteratorDebug*"
else
    echo "Build failed for debug level 0"
fi
cd ..

echo
echo

# Test with debug level 1 (enabled)
echo "============================================================"
echo "Testing with CHUNKED_VEC_ITERATOR_DEBUG_LEVEL=1 (Debug)"
echo "============================================================"
mkdir -p build_debug1
cd build_debug1
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=1"
cmake --build .
if [ -x "./chunked_vector_tests" ]; then
    echo "Running tests with iterator debugging enabled..."
    ./chunked_vector_tests --gtest_filter="*IteratorDebug*"
else
    echo "Build failed for debug level 1"
fi
cd ..

echo
echo

# Test with debug level 2 (maximum debugging)
echo "============================================================"
echo "Testing with CHUNKED_VEC_ITERATOR_DEBUG_LEVEL=2 (Maximum)"
echo "============================================================"
mkdir -p build_debug2
cd build_debug2
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=2"
cmake --build .
if [ -x "./chunked_vector_tests" ]; then
    echo "Running tests with maximum iterator debugging..."
    ./chunked_vector_tests --gtest_filter="*IteratorDebug*"
else
    echo "Build failed for debug level 2"
fi
cd ..

echo
echo "============================================================"
echo "Iterator debugging tests completed!"
echo "============================================================" 