
cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 NEW)
project(NWAOBDD VERSION 1.0)

set(path_to_boost "/home/xusheng_zhi/Desktop/boost")

add_executable(nwaobdd.out 
    assignment.cpp
    bool_op.cpp
    cross_product_nwa.cpp
    main.cpp
    nwaobdd.cpp
    nwaobdd_node.cpp
    reduction_map.cpp
    return_map_impl.cpp
    traverse_state_nwa.cpp
    testing/tests_nwa.cpp
    connection.cpp
    connectionT_impl.cpp
    nwaobdd_top_node.cpp
)
target_compile_options(nwaobdd.out PRIVATE -w -lm -std=c++2a -O3 -I${path_to_boost})
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
target_compile_options(infra PRIVATE -w -std=c++2a -O3)