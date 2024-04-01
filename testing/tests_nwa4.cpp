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


using namespace NWA_OBDD::MatrixComplex;
using namespace NWA_OBDD::VectorComplex;

static void test_cnot() {
    auto f = MkCNot(2, 4, 0, 1);
    DumpMatrix(f);

    auto g = MkCNot(2, 4, 1, 0);
    DumpMatrix(g);
}


