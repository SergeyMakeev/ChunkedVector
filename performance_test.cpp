#include "ubench.h"
#include "test_common.h"

// Helper to prevent compiler optimizations
template<typename T>
void do_not_optimize(T&& value) {
    UBENCH_DO_NOTHING(const_cast<void*>(static_cast<const volatile void*>(&value)));
}

// =============================================================================
// Performance Test Function Wrappers
// =============================================================================

template<typename Container>
void perf_test_push_back(size_t size) {
    Container vec;
    test_push_back(vec, size);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_sequential_access() {
    Container vec;
    test_sequential_access(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_random_access() {
    Container vec;
    test_random_access(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_iterator_traversal() {
    Container vec;
    test_iterator_traversal(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_range_based_loop() {
    Container vec;
    test_range_based_loop(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_reserve_performance() {
    Container vec;
    test_reserve_performance(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_construct_with_size() {
    Container vec;
    test_construct_with_size(vec, MEDIUM_SIZE, typename Container::value_type(42));
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_construct_and_fill() {
    Container vec;
    test_construct_and_fill(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_copy_constructor() {
    Container original;
    test_construct_and_fill(original);
    Container copy;
    test_copy_constructor(copy, original);
    do_not_optimize(copy);
}

template<typename Container>
void perf_test_copy_assignment() {
    Container original;
    test_construct_and_fill(original);
    Container copy;
    test_copy_assignment(copy, original);
    do_not_optimize(copy);
}

template<typename Container>
void perf_test_resize_grow() {
    Container vec;
    test_resize_grow(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_resize_shrink() {
    Container vec;
    test_resize_shrink(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_mixed_operations() {
    Container vec;
    test_mixed_operations(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_large_objects() {
    Container vec;
    test_large_objects(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_std_algorithm_find() {
    Container vec;
    test_std_algorithm_find(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_std_algorithm_accumulate() {
    Container vec;
    test_std_algorithm_accumulate(vec);
    do_not_optimize(vec);
}

template<typename Container>
void perf_test_page_boundary_access() {
    Container vec;
    test_page_boundary_access(vec);
    do_not_optimize(vec);
}

// =============================================================================
// Benchmark Instantiations
// =============================================================================

// Push Back Performance Tests - TestObject
UBENCH(push_back_small_testobject, std_vector) {
    perf_test_push_back<std::vector<TestObject>>(SMALL_SIZE);
}

UBENCH(push_back_small_testobject, chunked_vector) {
    perf_test_push_back<chunked_vector<TestObject>>(SMALL_SIZE);
}

UBENCH(push_back_medium_testobject, std_vector) {
    perf_test_push_back<std::vector<TestObject>>(MEDIUM_SIZE);
}

UBENCH(push_back_medium_testobject, chunked_vector) {
    perf_test_push_back<chunked_vector<TestObject>>(MEDIUM_SIZE);
}

UBENCH(push_back_large_testobject, std_vector) {
    perf_test_push_back<std::vector<TestObject>>(LARGE_SIZE);
}

UBENCH(push_back_large_testobject, chunked_vector) {
    perf_test_push_back<chunked_vector<TestObject>>(LARGE_SIZE);
}

// Push Back Performance Tests - float
UBENCH(push_back_small_float, std_vector) {
    perf_test_push_back<std::vector<float>>(SMALL_SIZE);
}

UBENCH(push_back_small_float, chunked_vector) {
    perf_test_push_back<chunked_vector<float>>(SMALL_SIZE);
}

UBENCH(push_back_medium_float, std_vector) {
    perf_test_push_back<std::vector<float>>(MEDIUM_SIZE);
}

UBENCH(push_back_medium_float, chunked_vector) {
    perf_test_push_back<chunked_vector<float>>(MEDIUM_SIZE);
}

UBENCH(push_back_large_float, std_vector) {
    perf_test_push_back<std::vector<float>>(LARGE_SIZE);
}

UBENCH(push_back_large_float, chunked_vector) {
    perf_test_push_back<chunked_vector<float>>(LARGE_SIZE);
}

// Sequential Access Performance Tests - TestObject
UBENCH(sequential_access_testobject, std_vector) {
    perf_test_sequential_access<std::vector<TestObject>>();
}

UBENCH(sequential_access_testobject, chunked_vector) {
    perf_test_sequential_access<chunked_vector<TestObject>>();
}

// Sequential Access Performance Tests - float
UBENCH(sequential_access_float, std_vector) {
    perf_test_sequential_access<std::vector<float>>();
}

UBENCH(sequential_access_float, chunked_vector) {
    perf_test_sequential_access<chunked_vector<float>>();
}

// Random Access Performance Tests - TestObject
UBENCH(random_access_testobject, std_vector) {
    perf_test_random_access<std::vector<TestObject>>();
}

UBENCH(random_access_testobject, chunked_vector) {
    perf_test_random_access<chunked_vector<TestObject>>();
}

// Random Access Performance Tests - float
UBENCH(random_access_float, std_vector) {
    perf_test_random_access<std::vector<float>>();
}

UBENCH(random_access_float, chunked_vector) {
    perf_test_random_access<chunked_vector<float>>();
}

// Iterator Performance Tests - TestObject
UBENCH(iterator_traversal_testobject, std_vector) {
    perf_test_iterator_traversal<std::vector<TestObject>>();
}

UBENCH(iterator_traversal_testobject, chunked_vector) {
    perf_test_iterator_traversal<chunked_vector<TestObject>>();
}

UBENCH(range_based_loop_testobject, std_vector) {
    perf_test_range_based_loop<std::vector<TestObject>>();
}

UBENCH(range_based_loop_testobject, chunked_vector) {
    perf_test_range_based_loop<chunked_vector<TestObject>>();
}

// Iterator Performance Tests - float
UBENCH(iterator_traversal_float, std_vector) {
    perf_test_iterator_traversal<std::vector<float>>();
}

UBENCH(iterator_traversal_float, chunked_vector) {
    perf_test_iterator_traversal<chunked_vector<float>>();
}

UBENCH(range_based_loop_float, std_vector) {
    perf_test_range_based_loop<std::vector<float>>();
}

UBENCH(range_based_loop_float, chunked_vector) {
    perf_test_range_based_loop<chunked_vector<float>>();
}

// Memory Allocation Performance Tests - TestObject
UBENCH(reserve_performance_testobject, std_vector) {
    perf_test_reserve_performance<std::vector<TestObject>>();
}

UBENCH(reserve_performance_testobject, chunked_vector) {
    perf_test_reserve_performance<chunked_vector<TestObject>>();
}

// Memory Allocation Performance Tests - float
UBENCH(reserve_performance_float, std_vector) {
    perf_test_reserve_performance<std::vector<float>>();
}

UBENCH(reserve_performance_float, chunked_vector) {
    perf_test_reserve_performance<chunked_vector<float>>();
}

// Construction Performance Tests - TestObject
UBENCH(construct_with_size_testobject, std_vector) {
    perf_test_construct_with_size<std::vector<TestObject>>();
}

UBENCH(construct_with_size_testobject, chunked_vector) {
    perf_test_construct_with_size<chunked_vector<TestObject>>();
}

UBENCH(construct_and_fill_testobject, std_vector) {
    perf_test_construct_and_fill<std::vector<TestObject>>();
}

UBENCH(construct_and_fill_testobject, chunked_vector) {
    perf_test_construct_and_fill<chunked_vector<TestObject>>();
}

// Construction Performance Tests - float
UBENCH(construct_with_size_float, std_vector) {
    perf_test_construct_with_size<std::vector<float>>();
}

UBENCH(construct_with_size_float, chunked_vector) {
    perf_test_construct_with_size<chunked_vector<float>>();
}

UBENCH(construct_and_fill_float, std_vector) {
    perf_test_construct_and_fill<std::vector<float>>();
}

UBENCH(construct_and_fill_float, chunked_vector) {
    perf_test_construct_and_fill<chunked_vector<float>>();
}

// Copy Performance Tests - TestObject
UBENCH(copy_constructor_testobject, std_vector) {
    perf_test_copy_constructor<std::vector<TestObject>>();
}

UBENCH(copy_constructor_testobject, chunked_vector) {
    perf_test_copy_constructor<chunked_vector<TestObject>>();
}

UBENCH(copy_assignment_testobject, std_vector) {
    perf_test_copy_assignment<std::vector<TestObject>>();
}

UBENCH(copy_assignment_testobject, chunked_vector) {
    perf_test_copy_assignment<chunked_vector<TestObject>>();
}

// Copy Performance Tests - float
UBENCH(copy_constructor_float, std_vector) {
    perf_test_copy_constructor<std::vector<float>>();
}

UBENCH(copy_constructor_float, chunked_vector) {
    perf_test_copy_constructor<chunked_vector<float>>();
}

UBENCH(copy_assignment_float, std_vector) {
    perf_test_copy_assignment<std::vector<float>>();
}

UBENCH(copy_assignment_float, chunked_vector) {
    perf_test_copy_assignment<chunked_vector<float>>();
}

// Resize Performance Tests - TestObject
UBENCH(resize_grow_testobject, std_vector) {
    perf_test_resize_grow<std::vector<TestObject>>();
}

UBENCH(resize_grow_testobject, chunked_vector) {
    perf_test_resize_grow<chunked_vector<TestObject>>();
}

UBENCH(resize_shrink_testobject, std_vector) {
    perf_test_resize_shrink<std::vector<TestObject>>();
}

UBENCH(resize_shrink_testobject, chunked_vector) {
    perf_test_resize_shrink<chunked_vector<TestObject>>();
}

// Resize Performance Tests - float
UBENCH(resize_grow_float, std_vector) {
    perf_test_resize_grow<std::vector<float>>();
}

UBENCH(resize_grow_float, chunked_vector) {
    perf_test_resize_grow<chunked_vector<float>>();
}

UBENCH(resize_shrink_float, std_vector) {
    perf_test_resize_shrink<std::vector<float>>();
}

UBENCH(resize_shrink_float, chunked_vector) {
    perf_test_resize_shrink<chunked_vector<float>>();
}

// Mixed Operations Performance Tests - TestObject
UBENCH(mixed_operations_testobject, std_vector) {
    perf_test_mixed_operations<std::vector<TestObject>>();
}

UBENCH(mixed_operations_testobject, chunked_vector) {
    perf_test_mixed_operations<chunked_vector<TestObject>>();
}

// Mixed Operations Performance Tests - float
UBENCH(mixed_operations_float, std_vector) {
    perf_test_mixed_operations<std::vector<float>>();
}

UBENCH(mixed_operations_float, chunked_vector) {
    perf_test_mixed_operations<chunked_vector<float>>();
}

// Large Object Performance Tests
UBENCH(large_objects, std_vector) {
    perf_test_large_objects<std::vector<LargeObject>>();
}

UBENCH(large_objects, chunked_vector) {
    perf_test_large_objects<chunked_vector<LargeObject>>();
}

// STL Algorithm Performance Tests - TestObject
UBENCH(std_algorithm_find_testobject, std_vector) {
    perf_test_std_algorithm_find<std::vector<TestObject>>();
}

UBENCH(std_algorithm_find_testobject, chunked_vector) {
    perf_test_std_algorithm_find<chunked_vector<TestObject>>();
}

UBENCH(std_algorithm_accumulate_testobject, std_vector) {
    perf_test_std_algorithm_accumulate<std::vector<TestObject>>();
}

UBENCH(std_algorithm_accumulate_testobject, chunked_vector) {
    perf_test_std_algorithm_accumulate<chunked_vector<TestObject>>();
}

// STL Algorithm Performance Tests - float
UBENCH(std_algorithm_find_float, std_vector) {
    perf_test_std_algorithm_find<std::vector<float>>();
}

UBENCH(std_algorithm_find_float, chunked_vector) {
    perf_test_std_algorithm_find<chunked_vector<float>>();
}

UBENCH(std_algorithm_accumulate_float, std_vector) {
    perf_test_std_algorithm_accumulate<std::vector<float>>();
}

UBENCH(std_algorithm_accumulate_float, chunked_vector) {
    perf_test_std_algorithm_accumulate<chunked_vector<float>>();
}

// Memory Pattern Tests - TestObject
UBENCH(page_boundary_access_testobject, std_vector) {
    perf_test_page_boundary_access<std::vector<TestObject>>();
}

UBENCH(page_boundary_access_testobject, chunked_vector) {
    perf_test_page_boundary_access<chunked_vector<TestObject>>();
}

// Memory Pattern Tests - float
UBENCH(page_boundary_access_float, std_vector) {
    perf_test_page_boundary_access<std::vector<float>>();
}

UBENCH(page_boundary_access_float, chunked_vector) {
    perf_test_page_boundary_access<chunked_vector<float>>();
}

UBENCH_MAIN(); 