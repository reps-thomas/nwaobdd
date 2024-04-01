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


static void test_mk_basis_vector_new() {
    for(unsigned index = 0; index < 4; ++index) {
        auto f = VectorComplex::MkBasisVector(1, index);
        VectorComplex::DumpVector(f);
    }
    for(unsigned index = 0; index < 16; ++index) {
        auto f = VectorComplex::MkBasisVector(2, index);
        VectorComplex::DumpVector(f);
    }
    // for(unsigned index = 0; index < 256; ++index) {
    //     auto f = VectorComplex::MkBasisVector(3, index);
    //     VectorComplex::DumpVector(f);
    // }
}

static void test_path_sampling_new() {
    auto f1 = VectorComplex::MkBasisVector(1, 0);
    auto f2 = VectorComplex::MkBasisVector(1, 1);
    auto f3 = VectorComplex::MkBasisVector(1, 2);
    auto h = f1 + (f2 + f2 + f2) + (f3 + f3 + f3 + f3 + f3 + f3);
    std::string s;
    int occur[3] = {0, 0, 0};
    for(unsigned i = 0; i < 10000; ++i) {
        s = VectorComplex::Sampling(h, false);
        if(s == "00") occur[0]++;
        else if(s == "10") occur[1]++;
        else if(s == "01") occur[2]++;
        else abort();
    }
    printf("%d %d %d\n", occur[0], occur[1], occur[2]);

    f1 = VectorComplex::MkBasisVector(2, "0101");
    f2 = VectorComplex::MkBasisVector(2, "1010");
    f3 = VectorComplex::MkBasisVector(2, "0110");
    h = (f1 + f1) + (f2 + f2 + f2) + (f3 + f3 + f3 + f3 + f3);
    occur[0] = occur[1] = occur[2] = 0;
    for(unsigned i = 0; i < 10000; ++i) {
        s = VectorComplex::Sampling(h, true);
        if(s == "00") occur[0]++;
        else if(s == "11") occur[1]++;
        else if(s == "01") occur[2]++;
        else abort();
    }
    printf("%d %d %d\n", occur[0], occur[1], occur[2]);

    f1 = VectorComplex::MkBasisVector(3, "01010101");
    f2 = VectorComplex::MkBasisVector(3, "10101010");
    f3 = VectorComplex::MkBasisVector(3, "01100110");
    h = (f1) + (f2 + f2 + f2) + (f3 + f3 + f3 + f3 + f3 + f3);
    occur[0] = occur[1] = occur[2] = 0;
    for(unsigned i = 0; i < 10000; ++i) {
        s = VectorComplex::Sampling(h, true);
        if(s == "0000") occur[0]++;
        else if(s == "1111") occur[1]++;
        else if(s == "0101") occur[2]++;
        else abort();
    }
    printf("%d %d %d\n", occur[0], occur[1], occur[2]);
}

static void test_id() {
    auto f0 = MatrixComplex::MkId(1);
    MatrixComplex::DumpMatrix(f0);
    auto f1 = MatrixComplex::MkId(2);
    MatrixComplex::DumpMatrix(f1);
    auto f2 = MatrixComplex::MkId(3);
    MatrixComplex::DumpMatrix(f2);
}

static void test_negation() {
    auto f0 = MatrixComplex::MkNegation(1);
    MatrixComplex::DumpMatrix(f0);
    auto f1 = MatrixComplex::MkNegation(2);
    MatrixComplex::DumpMatrix(f1);
    auto f2 = MatrixComplex::MkNegation(3);
    MatrixComplex::DumpMatrix(f2);
}

static void test_walsh() {
    auto f0 = MatrixComplex::MkWalsh(1);
    MatrixComplex::DumpMatrix(f0);
    auto f1 = MatrixComplex::MkWalsh(2);
}

static void test_pauliy() {
    auto f0 = MatrixComplex::MkPauliY(1);
    MatrixComplex::DumpMatrix(f0);
    auto f1 = MatrixComplex::MkPauliY(2);
}

static void test_pauliz() {
    auto f0 = MatrixComplex::MkPauliZ(1);
    MatrixComplex::DumpMatrix(f0);
    auto f1 = MatrixComplex::MkPauliZ(2);
}

static void test_s() {
    auto f = MatrixComplex::MkS(0);
    MatrixComplex::DumpMatrix(f);
    auto g = MatrixComplex::MkPhaseShift(0, 0.25);
    MatrixComplex::DumpMatrix(g);
}

static void test_kronecker_product1() {
    auto f = MatrixComplex::MkId(1);
    auto g = MatrixComplex::MkNegation(1);
    auto fg = MatrixComplex::KroneckerProduct(f, g);
    MatrixComplex::DumpMatrix(fg);
    auto gf = MatrixComplex::KroneckerProduct(g, f);
    MatrixComplex::DumpMatrix(gf);

    auto f1 = MatrixComplex::MkId(2);
    auto g1 = MatrixComplex::MkNegation(2);
    auto fg1 = MatrixComplex::KroneckerProduct(f1, g1);
    MatrixComplex::DumpMatrix(fg1);
    auto gf1 = MatrixComplex::KroneckerProduct(g1, f1);
    MatrixComplex::DumpMatrix(gf1);
}

static void test_kronecker_product2() {
    auto f = MatrixComplex::MkPauliY(1);
    auto g = MatrixComplex::MkPauliZ(1);
    auto fg = MatrixComplex::KroneckerProduct(f, g);
    MatrixComplex::DumpMatrix(fg);
    auto gf = MatrixComplex::KroneckerProduct(g, f);
    MatrixComplex::DumpMatrix(gf);
}
static NWAOBDD_COMPLEX_BIG mk22matrix(unsigned a0, unsigned a1, unsigned a2, unsigned a3) {
    NWAOBDD_COMPLEX_BIG r = VectorComplex::MkBasisVector(1, 0); 
    if(a0-- == 0) {
        printf("a0 should not be 0\n");
        return r;
    }
    auto m0 = VectorComplex::MkBasisVector(1, 0);
    auto m1 = VectorComplex::MkBasisVector(1, 2);
    auto m2 = VectorComplex::MkBasisVector(1, 1);
    auto m3 = VectorComplex::MkBasisVector(1, 3);
    while(a0--) r = r + m0;
    while(a1--) r = r + m1;
    while(a2--) r = r + m2;
    while(a3--) r = r + m3;
    MatrixComplex::DumpMatrix(r);
    return r;
}
static void test_matrix_multiply() {
    int a0, a1, a2, a3;
    scanf("%d %d %d %d", &a0, &a1, &a2, &a3);
    auto f1 = mk22matrix(a0, a1, a2, a3);
    scanf("%d %d %d %d", &a0, &a1, &a2, &a3);
    auto f2 = mk22matrix(a0, a1, a2, a3);
    auto h = MatrixComplex::MatrixMultiply(f1, f2);
    MatrixComplex::DumpMatrix(h);
}

static void test_matrix_multiply_random() {
    int a0, a1, a2, a3;
    a0 = rand() % 5 + 1;
    a1 = rand() % 5;
    a2 = rand() % 5;
    a3 = rand() % 5;
    auto f1 = mk22matrix(a0, a1, a2, a3);
    int b0, b1, b2, b3;
    b0 = rand() % 5 + 1;
    b1 = rand() % 5;
    b2 = rand() % 5;
    b3 = rand() % 5;
    auto f2 = mk22matrix(b0, b1, b2, b3);
    auto h = MatrixComplex::MatrixMultiply(f1, f2);
    printf("Result:\n");
    MatrixComplex::DumpMatrix(h);
    printf("%d %d\n%d %d\n", a0 * b0 + a1 * b2, a0 * b1 + a1 * b3, a2 * b0 + a3 * b2, a2 * b1 + a3 * b3);
}





