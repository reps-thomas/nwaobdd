
cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0048 NEW)
project(NWAOBDD VERSION 1.0)

set(path_to_boost "/home/xusheng_zhi/Desktop/boost")

add_executable(nwaobdd.out 
    main.cpp
    testing/tests_nwa1.cpp
    testing/tests_nwa2.cpp
    testing/tests_nwa3.cpp
    testing/tests_nwa4.cpp
)
target_compile_options(nwaobdd.out PRIVATE -g -w -lm -std=c++2a -O3 -I${path_to_boost})
target_link_libraries(nwaobdd.out PRIVATE infra lib_nwaobdd)

add_library(lib_nwaobdd STATIC
    assignment.cpp
    bool_op.cpp
    connectionT_impl.cpp
    reduction_map.cpp
    return_map_impl.cpp
    nwaobdd_node.cpp
    nwaobdd_top_node_impl.cpp
    traverse_state_nwa.cpp
    nwaobdd.cpp
    nwaobdd_int.cpp
    cross_product_nwa.cpp

    pseudoCFLOBDD.cpp
    
    vector/vector_complex_top_node.cpp 
    vector/vector_complex.cpp
    vector/vector_node.cpp
    
    matrix/matrix_complex.cpp
    matrix/matrix_complex_top_node.cpp 
    matrix/matrix_complex_node.cpp
    matrix/matmult_map.cpp 
    matrix/nwaobdd_top_node_matmult_map.cpp
    matrix/zero_indices_map.cpp
)
target_compile_options(lib_nwaobdd PRIVATE -g -w -lm -std=c++2a -O3 -I${path_to_boost} -fPIC)
target_link_libraries(lib_nwaobdd PRIVATE infra)

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
target_compile_options(infra PRIVATE -w -g -std=c++2a -O3 -fPIC)