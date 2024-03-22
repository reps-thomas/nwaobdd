
cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0048 NEW)
project(NWAOBDD VERSION 1.0)

set(path_to_boost "/home/xusheng_zhi/Desktop/boost")

add_executable(nwaobdd.out 
    assignment.cpp
    bool_op.cpp
    cross_product_nwa.cpp
    main.cpp
    nwaobdd.cpp
    nwaobdd_int.cpp
    nwaobdd_node.cpp
    reduction_map.cpp
    return_map_impl.cpp
    traverse_state_nwa.cpp
    testing/tests_nwa1.cpp
    testing/tests_nwa2.cpp
    # connection.cpp
    connectionT_impl.cpp
    # nwaobdd_top_node.cpp
    fourier_semiring.cpp
    # matrix/matrix_complex.cpp
    nwaobdd_top_node_impl.cpp
    vector/vector_complex_top_node.cpp 
    vector/vector_complex.cpp
    vector/vector_node.cpp
    matrix/matrix_complex.cpp
    matrix/matrix_complex_top_node.cpp 
    matrix/matrix_complex_node.cpp
)
target_compile_options(nwaobdd.out PRIVATE -g -w -lm -std=c++2a -O3 -I${path_to_boost})
target_link_libraries(nwaobdd.out PRIVATE infra)

add_library(infra STATIC
    infra/apvector_T.h
    infra/conscell.cpp
    infra/hash.cpp
    infra/hashset.cpp
    infra/intpair.cpp
    infra/inttriple.cpp
    infra/list_T.h
    infra/list_TPtr.h
    infra/ref_ptr.h
)
target_compile_options(infra PRIVATE -w -g -std=c++2a -O3)