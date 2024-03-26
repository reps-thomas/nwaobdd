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

static void test_kronecker_product1() {
    auto f = MatrixComplex::MkId(0);
    auto g = MatrixComplex::MkNegation(0);
    auto fg = MatrixComplex::KroneckerProduct(f, g);
    MatrixComplex::DumpMatrix(fg);
    auto gf = MatrixComplex::KroneckerProduct(g, f);
    MatrixComplex::DumpMatrix(gf);

    auto f1 = MatrixComplex::MkId(1);
    auto g1 = MatrixComplex::MkNegation(1);
    auto fg1 = MatrixComplex::KroneckerProduct(f1, g1);
    MatrixComplex::DumpMatrix(fg1);
    auto gf1 = MatrixComplex::KroneckerProduct(g1, f1);
    MatrixComplex::DumpMatrix(gf1);
}

static void test_kronecker_product2() {
    auto f = MatrixComplex::MkPauliY(0);
    auto g = MatrixComplex::MkPauliZ(0);
    auto fg = MatrixComplex::KroneckerProduct(f, g);
    MatrixComplex::DumpMatrix(fg);
    auto gf = MatrixComplex::KroneckerProduct(g, f);
    MatrixComplex::DumpMatrix(gf);
}

static void test_id() {
    auto f0 = MatrixComplex::MkId(0);
    printf("*** %d\n", MatrixComplex::GetLevel(f0));
    MatrixComplex::DumpMatrix(f0);
    auto f1 = MatrixComplex::MkId(1);
    printf("*** %d\n", MatrixComplex::GetLevel(f1));
    MatrixComplex::DumpMatrix(f1);
    auto f2 = MatrixComplex::MkId(2);
    printf("*** %d\n", MatrixComplex::GetLevel(f2));
    MatrixComplex::DumpMatrix(f2);
}

static void test_negation() {
    auto f0 = MatrixComplex::MkNegation(0);
    printf("*** %d\n", MatrixComplex::GetLevel(f0));
    MatrixComplex::DumpMatrix(f0);
    auto f1 = MatrixComplex::MkNegation(1);
    printf("*** %d\n", MatrixComplex::GetLevel(f1));
    MatrixComplex::DumpMatrix(f1);
    auto f2 = MatrixComplex::MkNegation(2);
    printf("*** %d\n", MatrixComplex::GetLevel(f2));
    MatrixComplex::DumpMatrix(f2);
}

static void test_walsh() {
    auto f0 = MatrixComplex::MkWalsh(0);
    printf("*** %d\n", MatrixComplex::GetLevel(f0));
    MatrixComplex::DumpMatrix(f0);
    auto f1 = MatrixComplex::MkWalsh(1);
    printf("*** %d\n", MatrixComplex::GetLevel(f1));
}

static void test_pauliy() {
    auto f0 = MatrixComplex::MkPauliY(0);
    printf("*** %d\n", MatrixComplex::GetLevel(f0));
    MatrixComplex::DumpMatrix(f0);
    auto f1 = MatrixComplex::MkPauliY(1);
    printf("*** %d\n", MatrixComplex::GetLevel(f1));
}

static void test_pauliz() {
    auto f0 = MatrixComplex::MkPauliZ(0);
    printf("*** %d\n", MatrixComplex::GetLevel(f0));
    MatrixComplex::DumpMatrix(f0);
    auto f1 = MatrixComplex::MkPauliZ(1);
    printf("*** %d\n", MatrixComplex::GetLevel(f1));
}

static void test_s() {
    auto f = MatrixComplex::MkS(0);
    MatrixComplex::DumpMatrix(f);
    auto g = MatrixComplex::MkPhaseShift(0, 0.5);
    MatrixComplex::DumpMatrix(g);
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
    
    // test_kronecker_product1();
    test_s();

    std::cout << "Finishing\n";
}


