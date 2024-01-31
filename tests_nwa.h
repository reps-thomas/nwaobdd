#include "assignment.h"
#include "nwaobdd.h"

namespace NWA_OBDD {
class NWATests
{
    public:
        static void testInverseReedMull();
        static void testWalsh();
        static void testStepFunction();
        static void testPulseFunction();
        static void testIscas85();
        static void testPermute();
        static void testTopNodes();
        static void testCanonicalness();
        static void testAnd();
        static void testSatisfyingAssignments();
        static void testAllAssignments();
        static void test3();
        static void testMkDetensorConstraintInterleaved();
		static void testMkCFLOBDDMatrixEqVoc14();
        static void testMkIdRelationInterleaved();
		static void testMkIdRelationNested();
        static void testParity();
        static void test1();
        static void test2();
        static void testMaxLevelGreaterThan10();
        static int test_demorgans(void);
        static int test_irm4(void);
        static int test_arbitrary_step_functions(unsigned int mlev);
        static int test_restrict_exists_and_forall(void);
		static int test_restrict_exists_and_forall_deeper(void);
        static void factoringTest(unsigned int testNo, NWA_OBDD::NWAOBDD rel, unsigned int product);
		static void ApplyAndReduceUnitTests();
		static void testWeights();
		static void testSchema();
		static void testAddition();
		static bool runTests(const char * argv, int start = 0, int size = 0);
};
}