# Performance Testing

This directory contains tools for analyzing performance test results from the `chunked_vector` library.

## Overview

The performance testing infrastructure allows you to:
- Run performance benchmarks comparing `chunked_vector` with `std::vector`
- Parse results from multiple platforms
- Generate comparative analysis reports
- Track performance across different configurations

## Running Performance Tests

### Local Testing

```bash
# Build the performance test
cmake --build build --config Release

# Run performance tests
./build/performance_test
```

### CI/CD Integration

Performance tests are automatically run in CI/CD pipelines and artifacts are uploaded. You can download these artifacts and place them in this directory for analysis.

## File Structure

- `parse_performance_results.py` - Script to parse and analyze performance results
- `README.md` - This documentation file

## Expected Input Files

Place performance result files in this directory with the naming convention:
```
performance_results_<platform>.txt
```

Examples:
- `performance_results_windows-msvc-2022.txt`
- `performance_results_linux-gcc-11.txt`
- `performance_results_macos-clang-14.txt`

## Using the Parser

### Basic Usage

```bash
# Parse all performance files in the current directory
python parse_performance_results.py

# Specify input directory
python parse_performance_results.py --input-dir /path/to/results

# Specify output file
python parse_performance_results.py --output my_analysis.csv
```

### Output Format

The parser generates a CSV file with:
- **Rows**: Individual performance tests
- **Columns**: Platform-specific results
- **Metrics**: Execution times and performance ratios

### Example Output

```csv
Test,platform1.std_vector (us),platform1.chunked_vector (us),platform1.ratio,platform2.std_vector (us),platform2.chunked_vector (us),platform2.ratio
push_back_small,45.2,42.1,1.07,48.3,44.2,1.09
push_back_medium,1250.8,1180.3,1.06,1320.5,1245.1,1.06
sequential_access,890.2,920.1,0.97,905.3,935.2,0.97
```

## Performance Metrics

### Ratio Interpretation

- **Ratio > 1.0**: `chunked_vector` is faster
- **Ratio < 1.0**: `std::vector` is faster
- **Ratio = 1.0**: Equal performance

### Test Categories

The performance suite includes tests for:

1. **Push Back Operations**
   - Small, medium, and large containers
   - Different element types (trivial vs non-trivial)

2. **Access Patterns**
   - Sequential access
   - Random access
   - Iterator traversal
   - Range-based loops

3. **Construction and Assignment**
   - Copy construction
   - Move construction
   - Copy assignment
   - Move assignment

4. **Memory Management**
   - Reserve operations
   - Resize operations
   - Page boundary access

5. **STL Algorithm Compatibility**
   - `std::find`
   - `std::accumulate`
   - Other standard algorithms

## Analyzing Results

### Performance Trends

Look for patterns in the results:
- **Consistent ratios** across platforms indicate reliable performance characteristics
- **Platform-specific variations** may indicate compiler or hardware differences
- **Test-specific patterns** help identify optimization opportunities

### Key Metrics

1. **Push Back Performance**: Critical for dynamic growth scenarios
2. **Access Performance**: Important for read-heavy workloads
3. **Memory Efficiency**: Compare memory usage patterns
4. **STL Compatibility**: Ensure no performance regression with standard algorithms

## Troubleshooting

### Common Issues

1. **No files found**: Ensure performance result files follow the naming convention
2. **Parse errors**: Check that result files contain the expected format
3. **Missing tests**: Some platforms may not run all tests due to resource constraints

### File Format

Performance result files should contain lines like:
```
[       OK ] push_back_small.std_vector (mean 45.2us, ...)
[       OK ] push_back_small.chunked_vector (mean 42.1us, ...)
```

### Debugging

Enable verbose output:
```bash
python parse_performance_results.py --verbose
```

## Integration with CI/CD

### GitHub Actions

The project includes GitHub Actions workflows that:
1. Run performance tests on multiple platforms
2. Upload results as artifacts
3. Generate performance reports

### Custom Integration

To integrate with your own CI/CD:

1. Run the performance test: `./performance_test`
2. Save output to a file: `./performance_test > performance_results_<platform>.txt`
3. Use the parser to analyze results

## Contributing

When adding new performance tests:

1. Add the test to `performance_test.cpp`
2. Update the parser if new test categories are added
3. Document the test purpose and expected behavior
4. Consider adding tests for edge cases and different data types