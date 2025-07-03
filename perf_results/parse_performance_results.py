#!/usr/bin/env python3
"""
Performance Results Parser

This script parses performance test results from multiple files and generates
a CSV file where:
- Each row represents a different platform (file)
- Each column represents a different test
- Additional columns show the difference between std_vector and chunked_vector
"""

import os
import re
import csv
import argparse
from pathlib import Path
from typing import Dict, List, Tuple, Optional


def parse_time_value(time_str: str) -> float:
    """
    Parse time string and convert to microseconds for consistent comparison.
    
    Args:
        time_str: Time string like "2.651us", "496.994us", "3.334ms"
    
    Returns:
        Time value in microseconds
    """
    # Remove any whitespace
    time_str = time_str.strip()
    
    # Extract numeric value and unit
    match = re.match(r'([0-9.]+)(us|ms|s)', time_str)
    if not match:
        raise ValueError(f"Could not parse time value: {time_str}")
    
    value = float(match.group(1))
    unit = match.group(2)
    
    # Convert to microseconds
    if unit == 'us':
        return value
    elif unit == 'ms':
        return value * 1000
    elif unit == 's':
        return value * 1000000
    else:
        raise ValueError(f"Unknown time unit: {unit}")


def parse_performance_file(file_path: str) -> Dict[str, float]:
    """
    Parse a single performance result file.
    
    Args:
        file_path: Path to the performance result file
    
    Returns:
        Dictionary mapping test names to execution times (in microseconds)
    """
    results = {}
    
    with open(file_path, 'r') as file:
        for line in file:
            line = line.strip()
            
            # Look for result lines with format: [       OK ] test_name (mean X.XXXus, ...)
            match = re.match(r'\[\s+OK\s+\]\s+([^(]+)\s+\(mean\s+([^,]+),', line)
            if match:
                test_name = match.group(1).strip()
                time_str = match.group(2).strip()
                
                try:
                    time_value = parse_time_value(time_str)
                    results[test_name] = time_value
                except ValueError as e:
                    print(f"Warning: Could not parse time for {test_name}: {e}")
    
    return results


def extract_platform_name(filename: str) -> str:
    """
    Extract platform name from filename.
    
    Args:
        filename: Performance result filename
    
    Returns:
        Platform name (e.g., "windows-msvc-2022")
    """
    # Remove "performance_results_" prefix and ".txt" suffix
    if filename.startswith("performance_results_"):
        filename = filename[len("performance_results_"):]
    if filename.endswith(".txt"):
        filename = filename[:-4]
    
    return filename


def get_base_test_name(test_name: str) -> str:
    """
    Extract base test name without .std_vector or .chunked_vector suffix.
    
    Args:
        test_name: Full test name like "push_back_small.std_vector"
    
    Returns:
        Base test name like "push_back_small"
    """
    if test_name.endswith('.std_vector'):
        return test_name[:-len('.std_vector')]
    elif test_name.endswith('.chunked_vector'):
        return test_name[:-len('.chunked_vector')]
    else:
        return test_name


def calculate_ratio(std_vector_time: float, chunked_vector_time: float) -> float:
    """
    Calculate ratio of std_vector time to chunked_vector time.
    
    Args:
        std_vector_time: Baseline time (std_vector)
        chunked_vector_time: Comparison time (chunked_vector)
    
    Returns:
        Ratio (std_vector_time / chunked_vector_time)
        Values > 1 mean chunked_vector is faster
        Values < 1 mean std_vector is faster
    """
    if chunked_vector_time == 0:
        return float('inf') if std_vector_time > 0 else 0.0
    
    return std_vector_time / chunked_vector_time


def main():
    parser = argparse.ArgumentParser(description='Parse performance test results and generate CSV')
    parser.add_argument('--input-dir', '-i', default='.', 
                       help='Directory containing performance result files (default: current directory)')
    parser.add_argument('--output', '-o', default='performance_comparison.csv',
                       help='Output CSV file (default: performance_comparison.csv)')
    
    args = parser.parse_args()
    
    input_dir = Path(args.input_dir)
    
    if not input_dir.exists():
        print(f"Error: Directory {input_dir} does not exist")
        return 1
    
    # Find all performance result files
    result_files = list(input_dir.glob("performance_results_*.txt"))
    
    if not result_files:
        print(f"Error: No performance result files found in {input_dir}")
        return 1
    
    print(f"Found {len(result_files)} performance result files")
    
    # Parse all files
    all_platforms_data = {}
    all_test_names = set()
    
    for file_path in result_files:
        platform_name = extract_platform_name(file_path.name)
        print(f"Processing {platform_name}...")
        
        try:
            results = parse_performance_file(str(file_path))
            all_platforms_data[platform_name] = results
            
            # Track all test names for consistent columns
            all_test_names.update(results.keys())
            
            print(f"  Found {len(results)} test results")
            
        except Exception as e:
            print(f"Error processing {file_path}: {e}")
            continue
    
    # Extract unique base test names
    base_test_names = set()
    for test_name in all_test_names:
        base_test_names.add(get_base_test_name(test_name))
    
    base_test_names = sorted(base_test_names)
    
    print(f"Found {len(base_test_names)} unique base tests")
    
    # Prepare CSV columns - transposed format (tests as rows, platforms as columns)
    csv_columns = ['Test']
    
    # Add columns for each platform
    sorted_platforms = sorted(all_platforms_data.keys())
    for platform_name in sorted_platforms:
        csv_columns.extend([
            f"{platform_name}.std_vector (us)",
            f"{platform_name}.chunked_vector (us)",
            f"{platform_name}.ratio"
        ])
    
    # Generate CSV data - transposed format
    csv_data = []
    
    for base_test in base_test_names:
        row = [base_test]
        
        for platform_name in sorted_platforms:
            platform_data = all_platforms_data[platform_name]
            
            std_vector_name = f"{base_test}.std_vector"
            chunked_vector_name = f"{base_test}.chunked_vector"
            
            std_vector_time = platform_data.get(std_vector_name, None)
            chunked_vector_time = platform_data.get(chunked_vector_name, None)
            
            # Add std_vector time
            row.append(f"{std_vector_time:.3f}" if std_vector_time is not None else "")
            
            # Add chunked_vector time
            row.append(f"{chunked_vector_time:.3f}" if chunked_vector_time is not None else "")
            
            # Calculate and add ratio
            if std_vector_time is not None and chunked_vector_time is not None:
                ratio = calculate_ratio(std_vector_time, chunked_vector_time)
                row.append(f"{ratio:.2f}")
            else:
                row.append("")
        
        csv_data.append(row)
    
    # Write CSV file
    output_path = Path(args.output)
    
    try:
        with open(output_path, 'w', newline='', encoding='utf-8') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(csv_columns)
            writer.writerows(csv_data)
        
        print(f"Successfully generated {output_path}")
        print(f"CSV contains {len(csv_data)} tests and {len(sorted_platforms)} platforms")
        
    except Exception as e:
        print(f"Error writing CSV file: {e}")
        return 1
    
    return 0


if __name__ == "__main__":
    exit(main()) 