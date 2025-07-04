@echo off
setlocal enabledelayedexpansion

echo Testing chunked_vector iterator debugging at different levels...
echo.

REM Test with debug level 0 (disabled)
echo ============================================================
echo Testing with CHUNKED_VEC_ITERATOR_DEBUG_LEVEL=0 (Release)
echo ============================================================
mkdir build_debug0 2>nul
cd build_debug0
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=0"
cmake --build . --config Release
if exist Release\chunked_vector_tests.exe (
    echo Running tests with iterator debugging disabled...
    Release\chunked_vector_tests.exe --gtest_filter="*IteratorDebug*"
) else if exist chunked_vector_tests.exe (
    echo Running tests with iterator debugging disabled...
    chunked_vector_tests.exe --gtest_filter="*IteratorDebug*"
) else (
    echo Build failed for debug level 0
)
cd ..

echo.
echo.

REM Test with debug level 1 (enabled)
echo ============================================================
echo Testing with CHUNKED_VEC_ITERATOR_DEBUG_LEVEL=1 (Debug)
echo ============================================================
mkdir build_debug1 2>nul
cd build_debug1
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="/DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=1"
cmake --build . --config Debug
if exist Debug\chunked_vector_tests.exe (
    echo Running tests with iterator debugging enabled...
    Debug\chunked_vector_tests.exe --gtest_filter="*IteratorDebug*"
) else if exist chunked_vector_tests.exe (
    echo Running tests with iterator debugging enabled...
    chunked_vector_tests.exe --gtest_filter="*IteratorDebug*"
) else (
    echo Build failed for debug level 1
)
cd ..

echo.
echo.

REM Test with debug level 2 (maximum debugging)
echo ============================================================
echo Testing with CHUNKED_VEC_ITERATOR_DEBUG_LEVEL=2 (Maximum)
echo ============================================================
mkdir build_debug2 2>nul
cd build_debug2
cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="/DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=2"
cmake --build . --config Debug
if exist Debug\chunked_vector_tests.exe (
    echo Running tests with maximum iterator debugging...
    Debug\chunked_vector_tests.exe --gtest_filter="*IteratorDebug*"
) else if exist chunked_vector_tests.exe (
    echo Running tests with maximum iterator debugging...
    chunked_vector_tests.exe --gtest_filter="*IteratorDebug*"
) else (
    echo Build failed for debug level 2
)
cd ..

echo.
echo ============================================================
echo Iterator debugging tests completed!
echo ============================================================

pause 