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

using namespace NWA_OBDD;
using namespace SH_OBDD;


void NWATests::RunAllTests() {

    std::cout << "Starting to Run All Tests:\n";

    NWAOBDDNodeHandle::InitNoDistinctionTable();
    NWAOBDDNodeHandle::InitReduceCache();
    VectorComplex::VectorInitializer();
    InitPairProductCache();
	  InitPathSummaryCache();
	  InitPairProductMapCaches();

    srand(time(0));
    // testStepFunction(); 
    // testIscas85();
    // test4();
    // test_mk_basis_vector();
    test_path_sampling2();
    // testAnd();
    // testSatisfyingAssignments();
    // test_demorgans();
    // test_Addition();
    // test1();
    // test2();
    // testAllAssignments();
    // testMkIdRelationInterleaved();
    std::cout << "Finishing\n";
}


