#include "../assignment.h"
#include "../nwaobdd.h"
#include "../nwaobdd_int.h"
#include "../vector/vector_complex.h"
namespace NWA_OBDD {
class NWATests
{
    public:
        static void testInverseReedMull();
        static void testWalsh();
        static void testStepFunction(); // Fail, "MkStepUpOneFourthTop" not implemented
        static void testPulseFunction();
        static void testIscas85(); // Pass
        static void testPermute();
        static void testTopNodes(); // Pass
        static void testCanonicalness(); // Fail, Problem comes in "CountNodesAndEdges"
        static void testAnd(); // Pass
        static void testSatisfyingAssignments(); // Pass
        static void testAllAssignments(); // Pass
        static void test3(); // Failing, need further checkings
        static void testMkDetensorConstraintInterleaved();
		static void testMkCFLOBDDMatrixEqVoc14();
        static void testMkIdRelationInterleaved(); // Pass
		static void testMkIdRelationNested(); // Fail, might be the problem in the test itself
        static void testParity(); // Pass
        static void test1(); // Pass
        static void test2(); // Pass
        static void test4();
        static void testMaxLevelGreaterThan10();
        static int test_demorgans(void); // Pass
        static int test_irm4(void);
        static int test_arbitrary_step_functions(unsigned int mlev);
        static int test_restrict_exists_and_forall(void);
		static int test_restrict_exists_and_forall_deeper(void);
        static void factoringTest(unsigned int testNo, NWA_OBDD::NWAOBDD<int> rel, unsigned int product);
		static void ApplyAndReduceUnitTests();
		static void testWeights();
		static void testSchema();
		static void test_Addition(); // Pass
		static bool runTests(const char * argv, int start = 0, int size = 0);
        static void RunAllTests();
        static void test_mk_basis_vector();
};
}