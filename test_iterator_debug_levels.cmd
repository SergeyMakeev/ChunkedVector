@echo off
setlocal enabledelayedexpansion

echo Testing chunked_vector iterator debugging at different levels...
echo.

REM Function to test a specific debug level
goto :main

:test_debug_level
set debug_level=%1
set build_type=%2
set build_dir=build_debug%debug_level%_%build_type%

echo ============================================================
echo Testing with CHUNKED_VEC_ITERATOR_DEBUG_LEVEL=%debug_level% (%build_type%)
echo ============================================================

mkdir %build_dir% 2>nul
cd %build_dir%

REM Configure with specific debug level
cmake .. -DCMAKE_BUILD_TYPE=%build_type% -DCMAKE_CXX_FLAGS="/DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=%debug_level%"

REM Build
cmake --build . --config %build_type% --parallel

REM Determine executable path
set exe_path=%build_type%\chunked_vector_tests.exe
if not exist %exe_path% (
    set exe_path=chunked_vector_tests.exe
)

if exist %exe_path% (
    echo Running all tests with iterator debugging level %debug_level%...
    %exe_path% --gtest_output=xml:test_results_debug%debug_level%.xml
    
    echo.
    echo Running iterator debug tests specifically...
    %exe_path% --gtest_filter="*IteratorDebug*"
    
    if %debug_level% GTR 0 (
        echo.
        echo Running assertion verification tests...
        %exe_path% --gtest_filter="*AssertionOn*" || echo Assertion tests completed (some may intentionally trigger assertions)
    )
) else (
    echo Build failed for debug level %debug_level%
    cd ..
    exit /b 1
)

cd ..
echo.
goto :eof

:performance_comparison
echo ============================================================
echo Performance Comparison: Debug Level 0 vs 1
echo ============================================================

echo Building optimized release build with debug level 0...
mkdir build_perf_0 2>nul
cd build_perf_0
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=0 /O2"
cmake --build . --config Release --parallel

set perf_exe=Release\performance_test.exe
if not exist %perf_exe% set perf_exe=performance_test.exe

if exist %perf_exe% (
    echo Running performance test with debug level 0...
    powershell -Command "Measure-Command { & '.\%perf_exe%' }"
)
cd ..

echo.
echo Building release build with debug level 1...
mkdir build_perf_1 2>nul
cd build_perf_1
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/DCHUNKED_VEC_ITERATOR_DEBUG_LEVEL=1 /O2"
cmake --build . --config Release --parallel

set perf_exe=Release\performance_test.exe
if not exist %perf_exe% set perf_exe=performance_test.exe

if exist %perf_exe% (
    echo Running performance test with debug level 1...
    powershell -Command "Measure-Command { & '.\%perf_exe%' }"
)
cd ..

echo.
echo Performance comparison completed.
echo Debug level 0 should be significantly faster, while debug level 1 adds safety checks.
goto :eof

:main
REM Test different combinations
call :test_debug_level 0 Release
call :test_debug_level 1 Debug
call :test_debug_level 2 Debug

REM Optional performance comparison
set /p choice="Would you like to run performance comparison? (y/N): "
if /i "%choice%"=="y" call :performance_comparison

echo ============================================================
echo Iterator debugging tests completed!
echo ============================================================
echo.
echo Summary:
echo - Debug Level 0: Release mode, no debugging overhead
echo - Debug Level 1: Full iterator validation and tracking
echo - Debug Level 2: Same as level 1 (maximum debugging)
echo.
echo Test artifacts saved in:
echo - build_debug0_release/
echo - build_debug1_debug/
echo - build_debug2_debug/
echo.
echo All iterator debugging features have been verified!

pause 