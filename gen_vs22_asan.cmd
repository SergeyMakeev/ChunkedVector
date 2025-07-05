@echo off
set builddir=build2022_asan
if not exist %builddir% goto GENERATE
del %builddir% /S /Q
:GENERATE
mkdir %builddir%
cd %builddir%
# MSVC toolset, x64 target
cmake -G "Visual Studio 17 2022" -A x64 ^
      -DCMAKE_C_FLAGS="/fsanitize=address" ^
      -DCMAKE_CXX_FLAGS="/fsanitize=address" ^
      -DCMAKE_LINKER_FLAGS="/fsanitize=address" ^
      ../
cd ..

