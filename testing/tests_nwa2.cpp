#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <string>
#include "../nwaobdd_top_node.h"
#include "../nwaobdd_node.h"
#include "../nwaobdd.h"
#include "../assignment.h" 
#include "tests_nwa.h"
#include "../matrix/matrix_complex.h"

using namespace NWA_OBDD;
using namespace SH_OBDD;

void NWATests::test_mkID_matrix1() {
    auto f = MatrixComplex::MkIdRelationInterleaved(1);
    MatrixComplex::DumpMatrix(f);

    auto g = MatrixComplex::MkIdRelationInterleaved(2);
    MatrixComplex::DumpMatrix(g);

    auto h = MatrixComplex::MkIdRelationInterleaved(3);
    MatrixComplex::DumpMatrix(h);
}

NWAOBDD_COMPLEX_BIG make_all_c_matrix(unsigned level, BIG_COMPLEX_FLOAT c) {
    assert(level >= 1);
    NWAOBDDNodeHandle root = NWAOBDDNodeHandle::NoDistinctionNode[level];
    ReturnMapHandle<BIG_COMPLEX_FLOAT> r;
    r.AddToEnd(c);
    auto root_conn = new NWAOBDDTopNode<BIG_COMPLEX_FLOAT>(root, r);
    return NWAOBDD_COMPLEX_BIG(root_conn);
}

void NWATests::test_mkID_matrix2() {
    auto f = make_all_c_matrix(1, 2);
    auto g = MatrixComplex::MkIdRelationInterleaved(1);
    auto h = MatrixComplex::KroneckerProduct(g, f);
    MatrixComplex::DumpMatrix(h);
}

void test_dump_vector() {
    auto acc = VectorComplex::MkBasisVector(2, "00000000");
    for(unsigned i = 1; i < 256; ++i) {
        auto f = VectorComplex::MkBasisVector(2, i);
        for(unsigned j = 0; j <= i; ++j) {
            acc = acc + f;
        }
    }
    VectorComplex::DumpVector(acc);
}
void NWATests::RunAllTests() {

    std::cout << "Starting to Run All Tests:\n";

    NWAOBDDNodeHandle::InitNoDistinctionTable();
    NWAOBDDNodeHandle::InitReduceCache();
    VectorComplex::VectorInitializer();
    InitPairProductCache();
	  InitPathSummaryCache();
	  InitPairProductMapCaches();

    srand(time(0));
    
    test_mkID_matrix2();
    // test_dump_vector();

    std::cout << "Finishing\n";
}


