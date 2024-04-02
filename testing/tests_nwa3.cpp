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

template<unsigned siz>
struct Matrix {
    unsigned a[siz][siz];
    Matrix() {
        memset(a, 0, sizeof(a));
        for(unsigned i = 0; i < siz; ++i) 
            a[i][i] = 1.0;
    }
    Matrix(unsigned all_c) {
        for(unsigned i = 0; i < siz; ++i)
            for(unsigned j = 0; j < siz; ++j)
                a[i][j] = all_c;
    }
    void dump() {
        printf("[\n");
        for(unsigned i = 0; i < siz; ++i) {
            printf("  ");
            for(unsigned j = 0; j < siz; ++j)
                printf("%2d ", a[i][j]);
            printf("\n");
        }
        printf("]\n");
    }
    void gen() {
        for(unsigned i = 0; i < siz; ++i)
            for(unsigned j = 0; j < siz; ++j)
                a[i][j] = rand() % 3;
        a[0][0]++;
    }
};

template<unsigned siz>
Matrix<siz>  mult(Matrix<siz> p, Matrix<siz> q) {
    Matrix<siz>r(0);
    for(unsigned i = 0; i < siz; ++i)
        for(unsigned j = 0; j < siz; ++j)
            for(unsigned k = 0; k < siz; ++k)
                r.a[i][j] += p.a[i][k] * q.a[k][j];
    return r;
}
using namespace NWA_OBDD::MatrixComplex;
using namespace NWA_OBDD::VectorComplex;



NWAOBDD_COMPLEX_BIG mk44matrix(Matrix<4> m) {
    NWAOBDD_COMPLEX_BIG basis[16];
    for(unsigned i = 0; i < 16; ++i) 
        basis[i] = MkBasisVector(2, i);
    
    NWAOBDD_COMPLEX_BIG r = MkBasisVector(2, 0);
    assert(m.a[0][0] > 0); m.a[0][0]--;

    while(m.a[0][0]--) r = r + basis[0];
    while(m.a[0][1]--) r = r + basis[2];
    while(m.a[0][2]--) r = r + basis[8];
    while(m.a[0][3]--) r = r + basis[10];

    while(m.a[1][0]--) r = r + basis[1];
    while(m.a[1][1]--) r = r + basis[3];
    while(m.a[1][2]--) r = r + basis[9];
    while(m.a[1][3]--) r = r + basis[11];

    while(m.a[2][0]--) r = r + basis[4];
    while(m.a[2][1]--) r = r + basis[6];
    while(m.a[2][2]--) r = r + basis[12];
    while(m.a[2][3]--) r = r + basis[14];

    while(m.a[3][0]--) r = r + basis[5];
    while(m.a[3][1]--) r = r + basis[7];
    while(m.a[3][2]--) r = r + basis[13];
    while(m.a[3][3]--) r = r + basis[15];

    return r;
}

static void test_multiply() {
    Matrix<4>m1, m2, r;
    m1.gen(), m2.gen();
    r = mult(m1, m2);
    // printf("----multiplicants----\n");
    // m1.dump(), m2.dump();
    auto f1 = mk44matrix(m1);
    auto f2 = mk44matrix(m2);
    auto h = MatrixMultiply(f1, f2);
    // printf("-------result-------\n");
    // r.dump(); DumpMatrix(h);
}


void NWATests::RunAllTests() {

    std::cout << "Starting to Run All Tests:\n";

    NWAOBDDNodeHandle::InitNoDistinctionTable();
    NWAOBDDNodeHandle::InitReduceCache();
    VectorComplex::VectorInitializer();
    InitPairProductCache();
	InitPathSummaryCache();

    srand(time(0));
    for(unsigned i = 0; i < 30000; ++i)
        test_multiply();
}
