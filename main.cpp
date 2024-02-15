//
//    Copyright (c) 1999 Thomas W. Reps
//    All Rights Reserved.
//
//    This software is furnished under a license and may be used and
//    copied only in accordance with the terms of such license and the
//    inclusion of the above copyright notice.  This software or any
//    other copies thereof or any derivative works may not be provided
//    or otherwise made available to any other person.  Title to and
//    ownership of the software and any derivative works is retained
//    by Thomas W. Reps.
//
//    THIS IMPLEMENTATION MAY HAVE BUGS, SOME OF WHICH MAY HAVE SERIOUS
//    CONSEQUENCES.  THOMAS W. REPS PROVIDES THIS SOFTWARE IN ITS "AS IS"
//    CONDITION, AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
//    BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
//    THOMAS W. REPS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//    PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//    LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//    NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include "testing/tests_nwa.h"
#include <iomanip>
// #include "memory_check.h"
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
// #include <crtdbg.h>

#ifdef _DEBUG
	#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#else
	#define DBG_NEW new
#endif

static long seed_value = 27;

// MkXBit, MkYBit, MkZBit duplicated from cflobdd.cpp -----------------------------------------------
//
// MISSING: introduce a level of indirection so that the bits can be
// in a different order than interleaved.
/*static CFLOBDD MkXBit(unsigned int j) {
    return MkProjection(j * 4);
}

static CFLOBDD MkYBit(unsigned int j) {
    return MkProjection(j * 4 + 1);
}

static CFLOBDD MkZBit(unsigned int j) {
    return MkProjection(j * 4 + 2);
}

/*
int main (int argc, char * argv[])
{
	freopen( "nwaobddErr.txt", "w", stderr );
	freopen( "nwaobddOut.txt", "w", stdout );
	srand(seed_value);
    NWAOBDDNodeHandle::InitNoDistinctionTable();

	Tests::testTopNodes();
	Tests::testSatisfyingAssignments();
	Tests::testMkIdRelationInterleaved();
	Tests::testMkIdRelationNested();
	Tests::testParity();

  return 0;

}*/


class B{
	int x;
};

class A{
public:
	B* b;
};

extern unsigned long long checker;

int main(int argc, char * argv[])
{
    // printf("Program Starts\n");
	//_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
	//freopen("memory_report.txt", "w", stdout);
	//_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	//_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDOUT);
	//_RPT0(_CRT_ERROR, "1st message\n");
	//_CrtMemState ms = { 0 };
	//_CrtMemCheckpoint(&ms);
	// Check for missing arg
	if (argc < 1){
		std::cerr << "usage: " << argv[0] << " ..." << std::endl;
		return(1);
	}
	// Redirect stdout and stderr
	// freopen("cflobddErr.txt", "w", stderr);
	// freopen("cflobddOut.txt", "w", stdout);
	/*std::cout << std::fixed << std::showpoint;
	std::cout << std::setprecision(2);*/
	// Supply a default argument for when invoking from Windows (e.g., for debugging)
	if (argc == 1) {
		std::string default_string = "And";
		NWA_OBDD::NWATests::runTests(default_string.c_str());
	}
	else {
		if (argc == 3){
			NWA_OBDD::NWATests::runTests(argv[1], atoi(argv[2]));
        }
        else if (argc == 4) {
            NWA_OBDD::NWATests::runTests(argv[1], atoi(argv[2]), atoi(argv[3]));
        }
		else{
			NWA_OBDD::NWATests::runTests(argv[1]);
		}
	}
    printf("%llu\n", checker);
	/*
	char* a = new char[10];
	char* b = (char *)malloc(20);
	_CrtDumpMemoryLeaks();
	_CrtMemDumpAllObjectsSince(&ms);
	*/
	/*	srand(seed_value);
		bool otherTest;
		int startVal = 2;
		*/
	//	std::string t1 = argv[1];
	/*	if (t1 == "cflobdd") {
			bool otherTest = CFL_OBDD::CFLTests::runTests(argv, &startVal, argc);
			if (otherTest)
			NWA_OBDD::NWATests::runTests(argv, &startVal, argc);
			} else if (t1 == "nwaobdd") {
			otherTest = NWA_OBDD::NWATests::runTests(argv, &startVal, argc);
			if (otherTest)
			CFL_OBDD::CFLTests::runTests(argv, &startVal, argc);
			} else if (t1 == "both") {
			int s1 = 2;
			int s2 = 2;
			CFL_OBDD::CFLTests::runTests(argv, &s1, argc);
			NWA_OBDD::NWATests::runTests(argv, &s2, argc);
			} else {
			std::cerr << "Error, do not recognize argument " << argv[0] << " first argument must be 'cflobdd', 'nwaobdd', or 'both'" << std::endl;
			}*/
}

/*int main(int argc, char * argv[])
{
  // Check for missing arg; open output file
  //if (argc < 1 || argc >= 2){
  //  std::cerr << "usage: " << argv[0] << std::endl;
  //  return(1);
  //}

	freopen( "cflobddErr.txt", "w", stderr );
	freopen( "cflobddOut.txt", "w", stdout );

     srand(seed_value);
     CFLOBDDNodeHandle::InitNoDistinctionTable();
     CFLOBDDNodeHandle::InitReduceCache();
     InitPairProductCache();
     InitTripleProductCache();

     // Tests based on symbolicUnsigned --------------------------------------------

     symbolicUnsigned one(4,1);
     symbolicUnsigned two = one + one;
     symbolicUnsigned ans = two - one;
      std::cout << "ans = " << ans << std::endl << std::endl;

     symbolicUnsigned four = two * two;
       std::cout << "four = " << four << std::endl << std::endl;



      symbolicUnsigned x(8, 13);
        std::cout << "x = " << x << std::endl << std::endl;
      symbolicUnsigned y(8, 17);
        std::cout << "y = " << y << std::endl << std::endl;
      symbolicUnsigned z = x * y;
       std::cout << "z = " << z << std::endl << std::endl;

	   //ETTODO figure out what this is supposed to be
//     symbolicUnsigned a(8, symbolicUnsigned::ID_VOC1);
//     symbolicUnsigned b(8, symbolicUnsigned::ID_VOC2);
//     symbolicUnsigned c = a * b;

//     std::cout << "a = " << a << std::endl << std::endl;
//      std::cout << "b = " << b << std::endl << std::endl;
//     c = a + b;
//     std::cout << "c = " << c << std::endl;


	 
     for (int i = 0; i < argc; i++)
     {
         if (std::string(argv[i]) == "inverseReedMuller")
             Tests::testInverseReedMull();
         else if (std::string(argv[i]) == "walsh")
             Tests::testWalsh();
         else if (std::string(argv[i]) == "stepFunctions")
             Tests::testStepFunction();
         else if (std::string(argv[i]) == "pulseFunctions")
             Tests::testPulseFunction();
         else if (std::string(argv[i]) == "iscas85")
             Tests::testIscas85();
         else if (std::string(argv[i]) == "permute")
             Tests::testPermute();
         else if (std::string(argv[i]) == "topNodes")
             Tests::testTopNodes();
         else if (std::string(argv[i]) == "canonical")
             Tests::testCanonicalness();
         else if (std::string(argv[i]) == "allAssignments")
             Tests::testAllAssignments();
         else if (std::string(argv[i]) == "3Tests")
             Tests::testThree();
         else if (std::string(argv[i]) == "idRelation")
             Tests::testMkIdRelationInterleaved();
         else if (std::string(argv[i]) == "parity")
             Tests::testParity();
         else if (std::string(argv[i]) == "test1")
             Tests::test1();
         else if (std::string(argv[i]) == "test2")
             Tests::test2();
         else if (std::string(argv[i]) == "maxLevel")
             Tests::testMaxLevelGreaterThan10();
         else {
             Tests::testInverseReedMull();
             Tests::testWalsh();
             Tests::testStepFunction();
             Tests::testPulseFunction();
             //Tests::testIscas85();
             Tests::testPermute();
             Tests::testTopNodes();
             Tests::testCanonicalness();
             Tests::testAllAssignments();
             Tests::testThree();
             Tests::testMkIdRelationInterleaved();
             Tests::testMkDetensorConstraintInterleaved();
			 Tests::testMkCFLOBDDMatrixEqVoc14();
             Tests::testParity();
             Tests::test1();
             Tests::test2();
             Tests::testMaxLevelGreaterThan10();
         }
	 }

     // Show alpha01 + beta01 = (alpha+beta)10
     // Should succeed for (maxLevel >= 3)
     CFLOBDD X = MkAnd(MkNot(MkXBit(1)), MkXBit(0));          // X := "01"
     CFLOBDD Y = MkAnd(MkNot(MkYBit(1)), MkYBit(0));          // Y := "01"
     CFLOBDD Z = MkAnd(MkZBit(1), MkNot(MkZBit(0)));          // Z := "10"
     CFLOBDD R = MkAnd(MkAnd(MkAnd(X,Y),Z),AdditionRel);
//     CFLOBDD R = MkAnd(X,MkAnd(Y,MkAnd(Z,AdditionRel)));
     CFLOBDD TT = MkTrue();
     if (R == TT)
          std::cout << "R == TT" << std::endl;
     else
         std::cout << "R != TT" << std::endl;


     
     // Should succeed for 2-bit numbers (maxLevel == 3)
     X = MkAnd(MkNot(MkProjection(4)), MkProjection(0));          // X := "01"
     Y = MkAnd(MkNot(MkProjection(5)), MkProjection(1));          // Y := "01"
     Z = MkAnd(MkProjection(6), MkNot(MkProjection(2)));          // Z := "10"
     R = MkAnd(X,MkAnd(Y,MkAnd(Z,AdditionRel)));
     R.PrintYield(&std::cout);
     std::cout << std::endl;
     R.CountNodesAndEdges(nodeCount, edgeCount);
     std::cout << nodeCount << ", " << edgeCount << std::endl;
//     std::cout << "NumSatisfyingAssignments: " << R.NumSatisfyingAssignments() << std::endl;


/*
     // Should succeed for 32-bit numbers (maxLevel == 7)
     CFLOBDD X = MkX(13);
     CFLOBDD Y = MkY(13);
     CFLOBDD Z = MkZ(26);
     CFLOBDD R = MkAnd(X,MkAnd(Y,MkAnd(Z,AdditionRel)));
//     R.PrintYield(std::cout);
     std::cout << std::endl;
     R.CountNodesAndEdges(nodeCount, edgeCount);
     std::cout << nodeCount << ", " << edgeCount << std::endl;
     std::cout << "NumSatisfyingAssignments: " << R.NumSatisfyingAssignments() << std::endl;
*/

	 /*
     // Tests based on the multiplication relation --------------------------------
     CFLOBDD MultRel = MkMultiplicationInterleavedBruteForce();
//     unsigned int nodeCount, edgeCount;
     MultRel.CountNodesAndEdges(nodeCount, edgeCount);
     std::cout << nodeCount << ", " << edgeCount << std::endl;
//     std::cout << "NumSatisfyingAssignments: " << MultRel.NumSatisfyingAssignments() << std::endl;

     factoringTest(0, MultRel, 1);
     factoringTest(1, MultRel, 4);
      factoringTest(2, MultRel, 6);
      factoringTest(3, MultRel, 12);
     factoringTest(4, MultRel, 13);
     factoringTest(5, MultRel, 15);
     factoringTest(6, MultRel, 35);
      factoringTest(7, MultRel, 221);    // 13 x 17
      factoringTest(8, MultRel, 41291);  // 157 x 263

	  */

/*  DisposeOfTripleProductCache();
  DisposeOfPairProductCache();
  CFLOBDDNodeHandle::DisposeOfReduceCache();
  return 0;
}*/
