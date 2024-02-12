
cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 NEW)
project(NWAOBDD VERSION 1.0)

set(path_to_boost "/mnt/e/Languages/boost")

add_executable(nwaobdd.out 
    assignment.cpp
    bool_op.cpp
    cmake_install.cmake
    cross_product_nwa.cpp
    main.cpp
    nwaobdd.cpp
    nwaobdd_node.cpp
    reduction_map.cpp
    return_map_specializations.cpp
    traverse_state_nwa.cpp
    testing/tests_nwa.cpp
)
target_compile_options(nwaobdd.out PRIVATE -w -lm -std=c++2a -O3 -I${path_to_boost})
target_link_libraries(nwaobdd.out PRIVATE infra)

add_library(infra STATIC
    infra/apvector_T.h
    infra/conscell.cpp
    infra/conscell.h
    infra/hash.cpp
    infra/hash.h
    infra/hashset.cpp
    infra/hashset.h
    infra/intpair.cpp
    infra/intpair.h
    infra/inttriple.cpp
    infra/inttriple.h
    infra/list_T.h
    infra/list_TPtr.h
    infra/ref_ptr.h
)
target_compile_options(infra PRIVATE -w -std=c++2a -O3)