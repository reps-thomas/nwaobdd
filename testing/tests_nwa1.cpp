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

static void factoringTest(unsigned int testNo, NWAOBDD<int> rel, unsigned int product) {
    NWAOBDD<int> f1, f2;
    unsigned int v1, v2;
    bool testResult = FactorZ(rel, product, f1, f2, v1, v2);
    std::cout << "Test " << testNo << ": factor " << product << ": " << testResult << std::endl;
    std::cout << v1 << " x " << v2 << " = " << product << std::endl;
}

void NWATests::testTopNodes(){
	std::cout << "Test of TopNodes --------------------------------------" << std::endl;
  // Create all of the possible projection NWAOBDDTopNodes
     NWAOBDD<int> F;
     for (unsigned int i = 0; i < (unsigned int)(1 << NWAOBDD<int>::maxLevel); i++) {
       F = MkProjection(i);
     }
}

static bool say_is_active = true;
#define ACTIVATE_SAY() say_is_active = true
#define DEACTIVATE_SAY() say_is_active = false
#define SAY_IS_ACTIVE() say_is_active
#define SAY(s) if (SAY_IS_ACTIVE()) std::cout << s

#define OUTPUT_STREAM() (SAY_IS_ACTIVE() ? (&std::cout) : ((std::ostream* )NULL))

#ifdef NDEBUG
#define VERIFY(s) if (!(s)) { std::cout << "\nTEST FAILED: '" #s "'\n"; exit(2); }
#else
#ifdef NWAOBDD_DEVELOPMENT
#define VERIFY(s) if (!(s)) { std::cout << "\nTEST FAILED: '" #s "'\n"; abort(); exit(2); }
#else // NWAOBDD_DEVELOPMENT
#define VERIFY(s) if (!(s)) { std::cout << "\nTEST FAILED: '" #s "'\n"; exit(2); }
#endif // NWAOBDD_DEVELOPMENT
#endif

#define TEST_PASSED 0
#define TEST_NOT_RUN 77
#define TEST_FAILED 1

void NWATests::testSatisfyingAssignments(){
	std::cout << "  testing FindOneSatisfyingAssignment ------------" << std::endl;
  // generating formulas like "x[0] /\ ~x[1] /\ ...", 
  // so that we can check the solution easily

  unsigned num_of_vars = (1 << (NWAOBDD<int>::maxLevel + 2)) - 4;
  bool *target = new bool [num_of_vars];
  for(unsigned i = 0; i < num_of_vars; ++i)
    target[i] = rand() & 1;
  
  NWAOBDD<int> F; // constant true
  for(unsigned i = 0; i < num_of_vars; ++i) {
    NWAOBDD<int>G = MkProjection(i);
    if(target[i])
      F = MkAnd(F, G);
    else {
      G = MkNot(G);
      F = MkAnd(F, G);
    }
  }
  Assignment *assignmentPtr;
  assert(F.FindOneSatisfyingAssignment(assignmentPtr));
  for(unsigned i = 0; i < num_of_vars; ++i) 
    assert( (assignmentPtr -> operator[](i)) == target[i] );
  // assignmentPtr -> print(); std::cout << "\n";
  delete [] target;
}

int NWATests::test_restrict_exists_and_forall(void)
{
  int jk;

  NWAOBDD<int> F, G, H, I;
  NWAOBDD<int> J, K, L, M;
  NWAOBDD<int> N, O, P, Q;
#define WORLDC(func) func

  if (NWAOBDD<int>::maxLevel > 1) {
    // DLC use a manager for this test.
    SAY("MaxLevel must be no more than 1 for this test to be meaniningful to you.");
    return TEST_NOT_RUN;
  }
  SAY("Test 1: ---------------------------------" << std::endl);
  F = WORLDC(MkProjection)(0);
  VERIFY(!F.DependsOn(1));
  VERIFY(F.DependsOn(0));
  G = WORLDC(MkProjection)(1);
  H = WORLDC(MkProjection)(2);
  I = WORLDC(MkAnd)(F, WORLDC(MkAnd)(G, H));
  SAY("I = (x0 & x1 & x2)" << std::endl);
  VERIFY(!I.DependsOn(3));
  for (int jj = 0; jj < 3; jj++)
    VERIFY(I.DependsOn(jj));
  VERIFY(F.IsPositiveCube());
  VERIFY(G.IsPositiveCube());
  VERIFY(H.IsPositiveCube());
  VERIFY(I.IsPositiveCube());
  apvector<int> ss(3);
  for (jk = 0; jk < 3; jk++)
    ss[jk] = jk;
  {
    NWAOBDD<int> tmp = WORLDC(MkOr)(WORLDC(MkAnd)(F, G), H); // x0*x1 + x2
    for (jk = 0; jk < 3; jk++)
      VERIFY(tmp.DependsOn(jk));
    VERIFY(!tmp.IsPositiveCube());
    VERIFY(tmp.SupportSetIs(ss));
  }
  VERIFY(WORLDC(MkAnd)(F, G).IsPositiveCube());
  VERIFY(WORLDC(MkAnd)(F, H).IsPositiveCube());
  VERIFY(WORLDC(MkAnd)(G, H).IsPositiveCube());
  VERIFY(WORLDC(MkAnd)(F, WORLDC(MkAnd)(G, H)).IsPositiveCube());
  VERIFY(I.SupportSetIs(ss));

  I.PrintYield(OUTPUT_STREAM());

#define X0 01
#define X1 02
#define X2 04
#define X3 010
#define X4 020
#define X5 040
#define X6 0100
#define X7 0200

  VERIFY(I.Evaluate(Assignment(0.0, true, true, true, false)) == true);
  VERIFY(I.Evaluate(Assignment(0.0, true, true, false, false)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, true, false, true, false)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, true, false, false, false)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, false, true, true, false)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, false, true, false, false)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, false, false, true, false)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, false, false, false, false)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, true, true, true, true)) == true);
  VERIFY(I.Evaluate(Assignment(0.0, true, true, false, true)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, true, false, true, true)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, true, false, false, true)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, false, true, true, true)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, false, true, false, true)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, false, false, true, true)) == false);
  VERIFY(I.Evaluate(Assignment(0.0, false, false, false, true)) == false);
  VERIFY(I.Evaluate(Assignment(4, 0)) == false);
  VERIFY(I.Evaluate(Assignment(4, X0)) == false);
  VERIFY(I.Evaluate(Assignment(4, X2)) == false);
  VERIFY(I.Evaluate(Assignment(4, X1)) == false);
  VERIFY(I.Evaluate(Assignment(4, X1|X2)) == false);
  VERIFY(I.Evaluate(Assignment(4, X0|X1)) == false);
  VERIFY(I.Evaluate(Assignment(4, X0|X2)) == false);
  VERIFY(I.Evaluate(Assignment(4, X0|X1|X2)) == true);

  SAY(std::endl << std::endl);
  SAY("I Restricted such that x1 <- true === I[true/x1] <Expected: (x0 & x2)>" << std::endl);
  J = WORLDC(MkRestrict)(I, 1, true);
  J.PrintYield(OUTPUT_STREAM());
  SAY(std::endl << std::endl);

  VERIFY(J.Evaluate(Assignment(4, 0)) == false);
  VERIFY(J.Evaluate(Assignment(4, X0)) == false);
  VERIFY(J.Evaluate(Assignment(4, X2)) == false);
  VERIFY(J.Evaluate(Assignment(4, X1)) == false);
  VERIFY(J.Evaluate(Assignment(4, X1|X2)) == false);
  VERIFY(J.Evaluate(Assignment(4, X0|X1)) == false);
  VERIFY(J.Evaluate(Assignment(4, X0|X2)) == true);
  VERIFY(J.Evaluate(Assignment(4, X0|X1|X2)) == true);


  SAY("I[false/x1] <Expected: false>" << std::endl);
  K = WORLDC(MkRestrict)(I, 1, false);
  K.PrintYield(OUTPUT_STREAM());
  SAY(std::endl << std::endl);
  SAY("Test whether False == I[false/x1] <Expected answer: equal>" << std::endl);
  NWAOBDD<int> falseflob = WORLDC(MkFalse)();
  if (K != falseflob) {
    SAY("K != MkFalse()" << std::endl);
    SAY(*K.root << std::endl);
  }
  else
    SAY("K == MkFalse()" << std::endl);
  VERIFY(K == falseflob);
  SAY(std::endl);



  SAY("Exists x1.I === I[true/x1] v I[false/x1] === (x0 & x2) v (false) === x0 & x2" << std::endl);
  L = WORLDC(MkExists)(I, 1);
  L.PrintYield(OUTPUT_STREAM());
  VERIFY(L == J);
  SAY(std::endl << std::endl);


  return TEST_PASSED;
}


int NWATests::test_restrict_exists_and_forall_deeper(void)
{
	NWAOBDD<int> F, G, H, I;
	NWAOBDD<int> J, K, L, M;
	NWAOBDD<int> N, O, P, Q;
  SAY("Test 2: ---------------------------------" << std::endl);

  F = WORLDC(MkProjection)(1);
  G = WORLDC(MkProjection)(4);
  H = WORLDC(MkProjection)(7);
  I = WORLDC(MkAnd)(F, WORLDC(MkAnd)(G, H));
  SAY("I = (x1 & x4 & x7)" << std::endl);
  I.PrintYield(OUTPUT_STREAM());
  VERIFY(I.Evaluate(Assignment(12, X1|X4|X7)) == true);
  VERIFY(I.Evaluate(Assignment(12, X1|X4)) == false);
  VERIFY(I.Evaluate(Assignment(12, X1|X7)) == false);
  VERIFY(I.Evaluate(Assignment(12, X7 | X4)) == false);
  VERIFY(I.Evaluate(Assignment(12, X1)) == false);
  VERIFY(I.Evaluate(Assignment(12, X4)) == false);
  VERIFY(I.Evaluate(Assignment(12, X7)) == false);
  VERIFY(I.Evaluate(Assignment(12, 0)) == false);
  SAY(std::endl << std::endl);


  SAY("I[true/x4] == (x1 & x7)" << std::endl);
  J = WORLDC(MkRestrict)(I, 4, true);
  J.PrintYield(OUTPUT_STREAM());
  VERIFY(J.Evaluate(Assignment(12, X1 | X4 | X7)) == true);
  VERIFY(J.Evaluate(Assignment(12, X1 | X4)) == false);
  VERIFY(J.Evaluate(Assignment(12, X1 | X7)) == true);
  VERIFY(J.Evaluate(Assignment(12, X7 | X4)) == false);
  VERIFY(J.Evaluate(Assignment(12, X1)) == false);
  VERIFY(J.Evaluate(Assignment(12, X4)) == false);
  VERIFY(J.Evaluate(Assignment(12, X7)) == false);
  VERIFY(J.Evaluate(Assignment(12, 0)) == false);
  SAY(std::endl << std::endl);


  SAY("I[false/x4]" << std::endl);
  K = WORLDC(MkRestrict)(I, 4, false);
  K.PrintYield(OUTPUT_STREAM());
  SAY(std::endl << std::endl);


  SAY("Test whether False == I[true/x4] <Expected answer: equal>" << std::endl);
  if (K != WORLDC(MkFalse)()) {
    SAY("K != MkFalse()" << std::endl);
    SAY(*K.root << std::endl);
  }
  else
    SAY("K == MkFalse()" << std::endl);
  VERIFY(K == WORLDC(MkFalse)());
  SAY(std::endl);


  SAY("Exists x4.I === (I[true/x4] v I[false/x4]) === (x1&x7 v false) === (x1 & x7)" << std::endl);
  L = WORLDC(MkExists)(I, 4);
  L.PrintYield(OUTPUT_STREAM());
  VERIFY(J == L);
  SAY(std::endl << std::endl);





  SAY("Test 3: ---------------------------------" << std::endl);

  F = WORLDC(MkProjection)(1);
  G = WORLDC(MkProjection)(4);
  H = WORLDC(MkProjection)(7);
  I = WORLDC(MkAnd)(F, WORLDC(MkAnd)(G, H));
  J = WORLDC(MkNot)(F);
  K = WORLDC(MkProjection)(5);
  L = WORLDC(MkAnd)(J,K);
  M = WORLDC(MkOr)(I,L);
  SAY("M = (x1 & x4 & x7) | (!x1 & x5)" << std::endl);
  M.PrintYield(OUTPUT_STREAM());
  VERIFY(M.Evaluate(Assignment(12, X1 | X4 | X5 | X7)) == true);
  VERIFY(M.Evaluate(Assignment(12, X1 | X4 | X5)) == false);
  VERIFY(M.Evaluate(Assignment(12, X1 | X4 | X7)) == true);
  VERIFY(M.Evaluate(Assignment(12, X1 | X5 | X7)) == false);
  VERIFY(M.Evaluate(Assignment(12, X4 | X5 | X7)) == true);
  VERIFY(M.Evaluate(Assignment(12, X1 | X4)) == false);
  VERIFY(M.Evaluate(Assignment(12, X1 | X5)) == false);
  VERIFY(M.Evaluate(Assignment(12, X1 | X7)) == false);
  VERIFY(M.Evaluate(Assignment(12, X4 | X5)) == true);
  VERIFY(M.Evaluate(Assignment(12, X4 | X7)) == false);
  VERIFY(M.Evaluate(Assignment(12, X5 | X7)) == true);
  VERIFY(M.Evaluate(Assignment(12, X1)) == false);
  VERIFY(M.Evaluate(Assignment(12, X4)) == false);
  VERIFY(M.Evaluate(Assignment(12, X5)) == true);
  VERIFY(M.Evaluate(Assignment(12, X7)) == false);
  VERIFY(M.Evaluate(Assignment(12, 0)) == false);
  SAY(std::endl << std::endl);


  SAY("M[true/x4] === ((x1 & x7) | (!x1 & x5))" << std::endl);
  N = WORLDC(MkRestrict)(M, 4, true);
  N.PrintYield(OUTPUT_STREAM());
  VERIFY(N.Evaluate(Assignment(12, X1 | X4 | X5 | X7)) == true);
  VERIFY(N.Evaluate(Assignment(12, X1 | X4 | X5)) == false);
  VERIFY(N.Evaluate(Assignment(12, X1 | X4 | X7)) == true);
  VERIFY(N.Evaluate(Assignment(12, X1 | X5 | X7)) == true);
  VERIFY(N.Evaluate(Assignment(12, X4 | X5 | X7)) == true);
  VERIFY(N.Evaluate(Assignment(12, X1 | X4)) == false);
  VERIFY(N.Evaluate(Assignment(12, X1 | X5)) == false);
  VERIFY(N.Evaluate(Assignment(12, X1 | X7)) == true);
  VERIFY(N.Evaluate(Assignment(12, X4 | X5)) == true);
  VERIFY(N.Evaluate(Assignment(12, X4 | X7)) == false);
  VERIFY(N.Evaluate(Assignment(12, X5 | X7)) == true);
  VERIFY(N.Evaluate(Assignment(12, X1)) == false);
  VERIFY(N.Evaluate(Assignment(12, X4)) == false);
  VERIFY(N.Evaluate(Assignment(12, X5)) == true);
  VERIFY(N.Evaluate(Assignment(12, X7)) == false);
  VERIFY(N.Evaluate(Assignment(12, 0)) == false);
  SAY(std::endl << std::endl);


  SAY("M[false/x4] === !x1 & x5" << std::endl);
  O = WORLDC(MkRestrict)(M, 4, false);
  O.PrintYield(OUTPUT_STREAM());
  VERIFY(O.Evaluate(Assignment(12, X1 | X4 | X5 | X7)) == false);
  VERIFY(O.Evaluate(Assignment(12, X1 | X4 | X5)) == false);
  VERIFY(O.Evaluate(Assignment(12, X1 | X4 | X7)) == false);
  VERIFY(O.Evaluate(Assignment(12, X1 | X5 | X7)) == false);
  VERIFY(O.Evaluate(Assignment(12, X4 | X5 | X7)) == true);
  VERIFY(O.Evaluate(Assignment(12, X1 | X4)) == false);
  VERIFY(O.Evaluate(Assignment(12, X1 | X5)) == false);
  VERIFY(O.Evaluate(Assignment(12, X1 | X7)) == false);
  VERIFY(O.Evaluate(Assignment(12, X4 | X5)) == true);
  VERIFY(O.Evaluate(Assignment(12, X4 | X7)) == false);
  VERIFY(O.Evaluate(Assignment(12, X5 | X7)) == true);
  VERIFY(O.Evaluate(Assignment(12, X1)) == false);
  VERIFY(O.Evaluate(Assignment(12, X4)) == false);
  VERIFY(O.Evaluate(Assignment(12, X5)) == true);
  VERIFY(O.Evaluate(Assignment(12, X7)) == false);
  VERIFY(O.Evaluate(Assignment(12, 0)) == false);
  SAY(std::endl << std::endl);


  SAY("Exists x4.M === M[true/x4] v M[false/x4] === ((x1 & x7) | (!x1 & x5)) v (!x1 & x5) === (x1 & x7) | (!x1 & x5) === M[true/x4]" << std::endl);
  P = WORLDC(MkExists)(M, 4);
  P.PrintYield(OUTPUT_STREAM());
  VERIFY(P == N);
  SAY(std::endl << std::endl);


  SAY("Forall x4.M === M[true/x4] & M[false/x4] === ((x1 & x7) | (!x1 & x5)) & (!x1 & x5) === !x1 & x5 === M[false/x4]" << std::endl);
  Q = WORLDC(MkForall)(M, 4);
  Q.PrintYield(OUTPUT_STREAM());
  VERIFY(Q == O);
  SAY(std::endl << std::endl);


  SAY("Testing function composition." << std::endl);
  {
    F = WORLDC(MkProjection)(1);
    G = WORLDC(MkProjection)(4);
    H = WORLDC(MkProjection)(7);
    K = WORLDC(MkProjection)(5);
    L = WORLDC(MkProjection)(2);

    NWAOBDD<int> tmp = WORLDC(MkOr)(WORLDC(MkAnd)(F, G), H);  // x1*x4+x7
    NWAOBDD<int> tmp2 = WORLDC(MkOr)(WORLDC(MkAnd)(tmp, G), H);  // (x1*x4+x7)*x4+x7
    VERIFY(MkCompose(tmp, 1, tmp) == tmp2);
    VERIFY(MkCompose(tmp, 1, K) == WORLDC(MkOr)(WORLDC(MkAnd)(K, G), H)); // x5*x4+x7
    VERIFY(MkCompose(tmp, 1, WORLDC(MkFalse)()) == H); // 0*x4+x7 === x7
    VERIFY(MkCompose(tmp, 7, WORLDC(MkTrue)()) == WORLDC(MkTrue)());
    VERIFY(MkCompose(tmp, 7, L) == WORLDC(MkOr)(WORLDC(MkAnd)(F, G), L));
    VERIFY(MkCompose(tmp, 2, WORLDC(MkTrue)()) == tmp);
  }

#ifdef USE_WORLDS
#ifdef KEEP_MEMUSE
  SAY("F.MemUse() is " << F.MemUse() << std::endl);
  SAY("G.MemUse() is " << G.MemUse() << std::endl);
  SAY("H.MemUse() is " << H.MemUse() << std::endl);
  SAY("I.MemUse() is " << I.MemUse() << std::endl);
  SAY("J.MemUse() is " << J.MemUse() << std::endl);
  SAY("K.MemUse() is " << K.MemUse() << std::endl);
  SAY("L.MemUse() is " << L.MemUse() << std::endl);
  SAY("M.MemUse() is " << M.MemUse() << std::endl);
  SAY("N.MemUse() is " << N.MemUse() << std::endl);
  SAY("O.MemUse() is " << O.MemUse() << std::endl);
  SAY("P.MemUse() is " << P.MemUse() << std::endl);
  SAY("Q.MemUse() is " << Q.MemUse() << std::endl);
#endif // KEEP_MEMUSE
  SAY("Ending testWorld: \n" << testWorld << std::endl);
#endif*/
  
  return TEST_PASSED;
}

void NWATests::testMkIdRelationInterleaved()
{
// Test of MkIdRelationInterleaved function ----------------------------------
  std::cout << "Test of MkIdRelationInterleaved function --------------------------------------" << std::endl;
  NWAOBDD<int> F = MkIdRelationInterleaved();
  F.DumpValueTuple();
  F.DumpPathCountings();

  // if MaxLevel is 3 or 4, test all assignments
  // if (NWAOBDD<int>::maxLevel == 1 || NWAOBDD<int>::maxLevel == 2) {

    std::cout << "Testing all assignments" << std::endl;

    unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDD<int>::maxLevel + 2)) - (unsigned int)4));
    Assignment a(size);
    bool b;
    unsigned long int range = 1UL << size;
    for (unsigned long int i = 0UL; i < range; i++) { // for each assignment
      unsigned long int mask = 1UL;
      bool bb = true;
      for (int j = size - 1; j >= 0; j--) {  // for each variable
		unsigned long int temp = i & mask;
        a[j] = (temp != 0);
        if ((j/2)*2 == j) { // j is even
            if (a[j+1] != a[j]) {  // mismatch for interleaved ordering
                bb = false;
            }
        }
        mask = mask << 1;
      }
      // bb == true iff a represents a diagonal element for the interleaved ordering
      b = F.Evaluate(a);
      if (b != bb) {
		  std::cout << i << ": " << b << std::endl; //ETTODO -Fix
		  break;
      }
    }
  std::cout << "Id-Interleaved passed.\n";
  // }
  // else {
  //   std::cout << "Cannot test all assignments: maxLevel must be 1 or 2" << std::endl;
  // }
}

void NWATests::testMkIdRelationNested()
{
// Test of MkIdRelationNested function ----------------------------------
  std::cout << "Test of MkIdRelationNested function --------------------------------------" << std::endl;
  NWAOBDD<int> F = MkIdRelationNested();

  // if MaxLevel is 2 or 3, test all assignments
  if (NWAOBDD<int>::maxLevel == 1 || NWAOBDD<int>::maxLevel == 2) {

    std::cout << "Testing all assignments" << std::endl;

    unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDD<int>::maxLevel + 2)) - (unsigned int)4));
    Assignment a(size);
    bool b;
    unsigned long int range = 1UL << size;
    for (unsigned long int i = 0UL; i < range; i++) { // for each assignment
      unsigned long int mask = 1UL;
      bool bb = true;
      for (int j = size - 1; j >= 0; j--) {  // for each variable
		unsigned long int temp = i & mask;
        a[j] = (temp != 0);
		if (NWAOBDD<int>::maxLevel == 2) {
			if ((j == 0) || (j == 6)){
				if (a[j+5] != a[j]){
					bb = false;
				}
			}
			if ((j == 1) || (j == 3) || (j == 7) || (j == 9)) {
				if (a[j+1] != a[j]){
					bb = false;
				}
			}
		}
		if (NWAOBDD<int>::maxLevel == 1) {
			if ((j/2)*2 == j) { // j is even
				if (a[j+1] != a[j]) {  // mismatch for interleaved ordering
					bb = false;
				}
			}
		}
        mask = mask << 1;
      }
      // bb == true iff a represents a diagonal element for the interleaved ordering
      b = F.Evaluate(a);
      // std::cout << a << ": " << b << std::endl;
      if (b != bb) {
        std::cerr << "ERROR: " << i << std::endl; //ETTODO -Fix
      }
    }
  }
  else {
    std::cout << "Cannot test all assignments: maxLevel must be 1 or 2" << std::endl;
  }
}

void NWATests::testParity()
{
  // Test of parity function ----------------------------------
  std::cout << "Test of parity function --------------------------------------" << std::endl;
  NWAOBDD<int> F = MkParity();

  // doing some random tests
  unsigned num_of_vars = (1 << (NWAOBDD<int>::maxLevel + 2)) - 4;
  Assignment sample(num_of_vars);
  for(int tim = 0; tim < 20000; ++tim) {
    bool res = false;
    for(unsigned i = 0; i < num_of_vars; ++i) {
      bool b = rand() & 1;
      sample[i] = b;
      res ^= b;
    }
    assert(F.Evaluate(sample) == res);
  }
}


void NWATests::testAnd(){
  std::cout << "Testing MkAnd" << std::endl;
  NWAOBDD<int> F, G, H;
  F = MkProjection(0);
  std::cout << F << std::endl;
  std::cout << "----------------------------------------------------------------" << std::endl;
  G = MkProjection(1);
  std::cout << G << std::endl;
  std::cout << "----------------------------------------------------------------" << std::endl;
  H = F * G;
  std::cout << "----------------------------------------------------------------" << std::endl;
  std::cout << H << std::endl;
  std::cout << "----------------------------------------------------------------" << std::endl;
  H.DumpPathCountings();
}

void NWATests::ApplyAndReduceUnitTests(){
  std::cout << "----------------------------------------------------------------" << std::endl;
  std::cout << "-- Testing Pair Product --" << std::endl;

  NWAOBDD<int> F,G;
  F = MkParity();
  G = MkIdRelationNested();

  PairProductMapHandle testMap;
  NWAOBDDNodeHandle n = PairProduct(*(F.root->rootConnection.entryPointHandle),
                                       *(G.root->rootConnection.entryPointHandle),
                                       testMap);

  std::cout << "--------- Resulting NWAOBDD ------" << std::endl;
  n.print();

  std::cout << "---- Resulting PairProduct Map ---" << std::endl;
  std::cout << *testMap.mapContents << std::endl;

  std::cout << "--- Testing InducedReductionAndReturnMap ----" << std::endl;

  ReductionMapHandle inducedReductionMapHandle;
  ReturnMapHandle<intpair> inducedReturnMap;

  ReturnMapHandle<intpair> returnMapHandle;

  returnMapHandle.AddToEnd(intpair(0,0));
  returnMapHandle.AddToEnd(intpair(0,0));
  returnMapHandle.AddToEnd(intpair(1,1));
  returnMapHandle.AddToEnd(intpair(0,0));
  returnMapHandle.AddToEnd(intpair(1,1));
  
  returnMapHandle.InducedReductionAndReturnMap(inducedReductionMapHandle, inducedReturnMap);

  std::cout << "--- Resulting ReductionMap ---" << std::endl;
  std::cout << *inducedReductionMapHandle.mapContents << std::endl;
  std::cout << "--- Resulting ReturnMap ---" << std::endl;
  std::cout << *inducedReturnMap.mapContents << std::endl;
}

void NWATests::test1(){
  NWAOBDD<int> F, G, H, I;
  F = MkProjection(3);
  G = MkProjection(7);
  H = MkProjection(5);
  I = MkIfThenElse(F, G, H);

  // if MaxLevel is 3 or 4, test all assignments
  // if (NWAOBDD<int>::maxLevel == 1 || NWAOBDD<int>::maxLevel == 2) {

    std::cout << "Testing all assignments" << std::endl;

    unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDD<int>::maxLevel + 2)) - (unsigned int)4));
    Assignment a(size);
    bool b;
    unsigned long int range = 1UL << size;
    for (unsigned long int i = 0UL; i < range; i++) {
      unsigned long int mask = 1UL;
      for (int j = size - 1; j >= 0; j--) {
        a[j] = (i & mask);
        mask = mask << 1;
      }
    if(i % 100000 == 0)
	    std::cout << "i: " << i << std::endl;
    b = I.Evaluate(a);
    assert(b == ((a[3] && a[7]) || (!a[3] && a[5])) );
    }
  // }
  // else {
  //   std::cout << "Cannot test all assignments: maxLevel must be 1 or 2" << std::endl;
  // }
}

void NWATests::test2(){
  NWAOBDD<int> F, G, H, I;
  F = MkProjection(3);
  G = MkProjection(7);
  H = MkProjection(5);
  I = MkNegMajority(F, G, H);

  // if MaxLevel is 3 or 4, test all assignments


    std::cout << "Testing all assignments" << std::endl;

    unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDD<int>::maxLevel + 2)) - (unsigned int)4));
    Assignment a(size);
    bool b;
    unsigned long int range = 1UL << size;
    for (unsigned long int i = 0UL; i < range; i++) {
      unsigned long int mask = 1UL;
      for (int j = size - 1; j >= 0; j--) {
        a[j] = (i & mask);
        mask = mask << 1;
      }
      b = I.Evaluate(a);
      assert(b == ((a[7] && !a[3]) || (a[5] && !a[3]) || (a[7] && a[5])) );
    }
  std::cout << "test2 passed.\n";
}

void NWATests::test3(){
  NWAOBDD<int> F, G, H, I, J, K, L, M, N, O, P, Q;

  std::cout << "Test 1: ---------------------------------" << std::endl;
  F = MkProjection(0);
  G = MkProjection(1);
  H = MkProjection(2);
  I = MkAnd(F, MkAnd(G, H));
  std::cout << "(x0 & x1 & x2)" << std::endl;
  // I.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;
  std::cout << "Restrict, x1 <- true" << std::endl;
  J = MkRestrict(I, 1, true);
  // J.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;
  std::cout << "Restrict, x1 <- false" << std::endl;
  K = MkRestrict(I, 1, false);
  // K.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;
  std::cout << "Test whether False == Restrict, x1 <- false [Expected answer: K == MkFalse()]" << std::endl;
  if (K != MkFalse()) {
    std::cout << "K != MkFalse()" << std::endl;
    std::cout << *K.root << std::endl;
  }
  else
    std::cout << "K == MkFalse()" << std::endl;
  std::cout << std::endl;
  std::cout << "Exists x1" << std::endl;
  L = MkExists(I, 1);
  // L.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;

  std::cout << "Test 2: ---------------------------------" << std::endl;

  F = MkProjection(1);
  G = MkProjection(4);
  H = MkProjection(7);
  I = MkAnd( MkAnd(F, G), H);
  std::cout << "(x1 & x4 & x7)" << std::endl;
  // I.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;
  std::cout << "Restrict, x4 <- true" << std::endl;
  J = MkRestrict(I, 4, true);
  // J.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;
  std::cout << "Restrict, x4 <- false" << std::endl;
  K = MkRestrict(I, 4, false);
  // K.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;
  std::cout << "Test whether False == Restrict, x4 <- false [Expected answer: K == MkFalse()]" << std::endl;
  if (K != MkFalse()) {
    std::cout << "K != MkFalse()" << std::endl;
    std::cout << *K.root << std::endl;
  }
  else
    std::cout << "K == MkFalse()" << std::endl;
  std::cout << std::endl;
  std::cout << "Exists x4" << std::endl;
  L = MkExists(I, 4);
  // L.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;

  std::cout << "Test 3: ---------------------------------" << std::endl;

  F = MkProjection(1);
  G = MkProjection(4);
  H = MkProjection(7);
  I = MkAnd(MkAnd(F, G), H);
  J = MkNot(F);
  K = MkProjection(5);
  L = MkAnd(J,K);
  M = MkOr(I,L);
  std::cout << "(x1 & x4 & x7) | (!x1 & x5)" << std::endl;
  // M.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;
  std::cout << "Restrict, x4 <- true" << std::endl;
  N = MkRestrict(M, 4, true);
  // N.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;
  std::cout << "Restrict, x4 <- false" << std::endl;
  O = MkRestrict(M, 4, false);
  // O.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;
  std::cout << "Exists x4" << std::endl;
  P = MkExists(M, 4);
  // P.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;
  std::cout << "Forall x4" << std::endl;
  Q = MkForall(M, 4);
  // Q.PrintYield(&std::cout);
  std::cout << std::endl << std::endl;
}


void NWATests::test4() {
  NWAOBDD<int> F1 = MkProjection(7);
  NWAOBDD<int> F2 = MkProjection(8);
  NWAOBDD<int> F3 = MkProjection(10);
  NWAOBDD<int> F4 = MkProjection(11);
  auto F5 = MkProjection(13);
  auto F6 = MkProjection(18);
  NWAOBDD<int> r = (F1 + F2 + F3) * (F4 + F5 + F6);
  r.DumpValueTuple();
  r.DumpPathCountings();
}

void NWATests::testAllAssignments(){
  NWAOBDD<int> F, G, H, I;
  F = MkProjection(3);
  // std::cout << "F: " << std::endl;
  // std::cout << F << std::endl;
  G = MkProjection(7);
  // std::cout << "G: " << std::endl;
  // std::cout << G << std::endl;
  H = MkExclusiveOr(F, G);
  I = MkNot(H);

  // if MaxLevel is 3 or 4, test all assignments

    std::cout << "Testing all assignments" << std::endl;

    unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDD<int>::maxLevel + 2)) - (unsigned int)4));
    Assignment a(size);
    bool b;
    unsigned long int range = 1UL << size;
    for (unsigned long int i = 0UL; i < range; i++) {
      unsigned long int mask = 1UL;
      for (int j = size - 1; j >= 0; j--) {
        a[j] = (i & mask);
        mask = mask << 1;
      }
      b = I.Evaluate(a);
      // std::cout << a << ": " << b << std::endl;
      assert(b == !(a[3] != a[7]));
    }
    std::cout << "testAllAssignments finished.\n";
}

#include "c6288-10_nwa.cpp"
void NWATests::testIscas85()
{
//    c17();
//    c432();
//    iscasProduct();
//    iscasProduct2();
//    iscasParity();
//    dlx1_c();
    std::clock_t start;
    double duration;
    start = std::clock();
    c6288_10_nwa();
	//NWAOBDDNodeHandle::canonicalNodeTable->PrintTable();    duration = ( std::clock() - start )/(double) CLOCKS_PER_SEC;
    //std::cout<<"c6288_8: " << duration << '\n';
//    dartes();  //FIXME:  Compiler limit: too many exception handler states in function dartes
}

void NWATests::testSchema() {
	NWAOBDD<int> n = MkAdditionNested();
	int s[4];
	s[0] = 5;
	s[1] = 5;
	s[2] = 1;
	s[3] = 0;

	NWAOBDD<int> t = SchemaAdjust(n,s);

	std::cout << t << std::endl;

}

//Testing method for the weight code
void NWATests::testWeights(){

	NWAOBDD<int> n = MkIdRelationNested();
	int s[4];
	s[0] = 0;
	s[1] = 2;
	s[2] = 5;
	s[3] = 1;

	int vals[4];
	vals[0] = 1;
	vals[1] = 1;
	vals[2] = 0;
	vals[3] = 0;


	int vLocs[4];
	vLocs[0] = 0;
	vLocs[1] = 5;
	vLocs[2] = 6;
	vLocs[3] = 11;

	NWAOBDD<int> t = GetWeight(s,vals,vLocs,0);
	std::cout << t << std::endl;
}



void NWATests::testCanonicalness(){
  // DeMorgan's law as a test of canonicalness ------------------
     std::cout << "Test of Canonicalness" << std::endl;
     NWAOBDD<int> F, G, H, I, J, K;
     F = MkProjection(3);
     G = MkProjection(7);
     H = MkNand(F, G);
     I = MkNot(F);
     J = MkNot(G);
     K = MkOr(I, J);
   
     std::cout << (H == K) << std::endl;
   
    //  unsigned int nodeCount, edgeCount;
    //  F.CountNodesAndEdges(nodeCount, edgeCount);
    //  std::cout << nodeCount << ", " << edgeCount << std::endl;
    //  G.CountNodesAndEdges(nodeCount, edgeCount);
    //  std::cout << nodeCount << ", " << edgeCount << std::endl;
    //  H.CountNodesAndEdges(nodeCount, edgeCount);
    //  std::cout << nodeCount << ", " << edgeCount << std::endl;
    //  I.CountNodesAndEdges(nodeCount, edgeCount);
    //  std::cout << nodeCount << ", " << edgeCount << std::endl;
    //  J.CountNodesAndEdges(nodeCount, edgeCount);
    //  std::cout << nodeCount << ", " << edgeCount << std::endl;
    //  K.CountNodesAndEdges(nodeCount, edgeCount);
    //  std::cout << nodeCount << ", " << edgeCount << std::endl;
}

#define VERIFY_EDGECOUNT(ec, maxec) VERIFY((ec) <= (maxec))

int NWATests::test_demorgans(void)
{
#ifdef USE_WORLDS
  World testWorld1;

  CFLOBDD F(&testWorld1);
#define WORLDCX3(func) testWorld1.func
#else // USE_WORLDS
  NWAOBDD<int> F;
#define WORLDCX3(func) func
#endif // USE_WORLDS
#ifdef USE_WORLDS
  World testWorld1;
  CFLOBDD F(&testWorld1), G(&testWorld1), H(&testWorld1), I(&testWorld1);
  CFLOBDD J(&testWorld1), K(&testWorld1);
#else
  NWAOBDD<int> G, H, I, J, K;
#endif

  // DeMorgan's law as a test of canonicalness ------------------
  SAY("Test of DeMorgan's law" << std::endl);
  F = WORLDCX3(MkProjection)(3);
  G = WORLDCX3(MkProjection)(7);
  H = WORLDCX3(MkNand)(F, G);
  I = WORLDCX3(MkNot)(F);
  J = WORLDCX3(MkNot)(G);
  K = WORLDCX3(MkOr)(I, J);

  SAY((H == K) << " should be non-zero" << std::endl);
  VERIFY(H == K);
  H.DumpValueTuple();
  H.DumpPathCountings();

 /* unsigned int nodeCount, edgeCount;
  F.CountNodesAndEdges(nodeCount, edgeCount);
  SAY(nodeCount << ", " << edgeCount << std::endl);
  VERIFY(nodeCount == 7); VERIFY_EDGECOUNT(edgeCount, 15);
  G.CountNodesAndEdges(nodeCount, edgeCount);
  SAY(nodeCount << ", " << edgeCount << std::endl);
  VERIFY(nodeCount == 7); VERIFY_EDGECOUNT(edgeCount, 13);
  H.CountNodesAndEdges(nodeCount, edgeCount);
  SAY(nodeCount << ", " << edgeCount << std::endl);
  VERIFY(nodeCount == 7); VERIFY_EDGECOUNT(edgeCount, 16);
  I.CountNodesAndEdges(nodeCount, edgeCount);
  SAY(nodeCount << ", " << edgeCount << std::endl);
  VERIFY(nodeCount == 7); VERIFY_EDGECOUNT(edgeCount, 17);
  J.CountNodesAndEdges(nodeCount, edgeCount);
  SAY(nodeCount << ", " << edgeCount << std::endl);
  VERIFY(nodeCount == 7); VERIFY_EDGECOUNT(edgeCount, 15);
  K.CountNodesAndEdges(nodeCount, edgeCount);
  SAY(nodeCount << ", " << edgeCount << std::endl);
  VERIFY(nodeCount == 7); VERIFY_EDGECOUNT(edgeCount, 16);*/ //ETTODO Fix CountNodesAndEdges
  return TEST_PASSED;
}

void NWATests::test_Addition()
{
	std::clock_t start;
	double duration;
	double duration2;
	start = std::clock();
	//NWAOBDDNodeHandle::canonicalNodeTable->PrintTable();    duration = ( std::clock() - start )/(double) CLOCKS_PER_SEC;
	//std::cout<<"c6288_8: " << duration << '\n';
	NWAOBDD<int> AdditionRel = MkAdditionNestedTop();
	duration = (std::clock() - start) / (double)CLOCKS_PER_SEC;
	// Show alpha01 + beta01 = (alpha+beta)10
	// Should succeed for (maxLevel >= 3)
	unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDD<int>::maxLevel + 2)) - (unsigned int)4));
	Assignment A(size);
	for (unsigned i = 0; i < NWAOBDDMaxLevel - 2; i++)
	{
		A[i] = 0;
	}
	A[NWAOBDDMaxLevel - 2] = 0;
	A[NWAOBDDMaxLevel - 1] = 1;
	A[NWAOBDDMaxLevel] = 1;
	A[NWAOBDDMaxLevel + 1] = 0;
	A[NWAOBDDMaxLevel + 2] = 0;
	A[NWAOBDDMaxLevel + 3] = 0;
	A[NWAOBDDMaxLevel + 4] = 1;
	for (unsigned j = NWAOBDDMaxLevel + 5; j < size; j++)
	{
		A[j] = 0;
	}

	start = std::clock();
	bool result = AdditionRel.Evaluate(A);
	duration2 = (std::clock() - start) / (double)CLOCKS_PER_SEC;

	if (result)
	{
		std::cout << "YAY!"  << std::endl;
	}
	else
	{
		std::cout << "No!" << std::endl;
	}

	std::cout << "Build Duration: " << duration << "\n";
	std::cout << "Operation Duration: " << duration2 << "\n";
}


void NWATests::testStepFunction()
{
	std::cout << "Test of Step Function --------------------------------------" << std::endl;
     NWAOBDD<int> F = MkStepUpOneFourth();
     Assignment *assignmentPtr;
     if (F.FindOneSatisfyingAssignment(assignmentPtr)) {
       //std::cout << *assignmentPtr << std::endl; ETTODO -Fix
       bool b = F.Evaluate(*assignmentPtr);
       std::cout << "Value = " << b << std::endl;
       delete assignmentPtr;
     }
     else {
       std::cout << "No satisfying assignment exists" << std::endl;
     }
     
  // Tests of step-function creation operations ------------------------
#ifdef ARBITRARY_STEP_FUNCTIONS
     if (CFLOBDD::maxLevel <= 4) {
       CFLOBDD F;
       std::cout << "Testing all " << (1 << (1 << CFLOBDD::maxLevel)) + 1 << " step functions" << std::endl;
       for (int i = 0; i <= (1 << (1 << CFLOBDD::maxLevel)); i++) {
         unsigned int nodeCount, edgeCount;
         F = MkStepUp(i);
         F.PrintYield(std::cout);
         std::cout << std::endl;
         F.CountNodesAndEdges(nodeCount, edgeCount);
         std::cout << nodeCount << ", " << edgeCount << std::endl;
       }
     }
     else {
       std::cout << "Cannot test all step functions: maxLevel must be <= 4" << std::endl;
     }
#endif
}
//ETTODO SizeOfHaarHiddenBit - to get to testpermute

//ETTODO CountNodesandEdges

//ETTODO ISCAS stuff for NWAs

//ETTODO Arbitrary Step Functions

//ETTODO Pulse Functions
bool NWATests::runTests(const char * argv, int start, int size){
    NWAOBDDNodeHandle::InitNoDistinctionTable();
     NWAOBDDNodeHandle::InitReduceCache();
     InitPairProductCache();
	 InitPathSummaryCache();
	 InitPairProductMapCaches();

//	freopen( "nwaobddErr.txt", "w", stderr );
//	freopen( "nwaobddOut.txt", "w", stdout );
/*	while(*start < size)
	{
		std::string curTest = argv[*start];
		if (curTest == "TopNode") {
			NWATests::testTopNodes(); 
		} else if (curTest == "SatisfyingAssignment") {
			NWATests::testSatisfyingAssignments();  
		} else if (curTest == "InterleavedId") { 
			NWATests::testMkIdRelationInterleaved(); 
		} else if (curTest == "NestedId") {
			NWATests::testMkIdRelationNested(); 
		} else if (curTest == "Parity") {
			NWATests::testParity(); 
		} else if (curTest == "And") {
		    NWATests::testAnd(); 
		} else if (curTest == "ARUnit") 
			NWATests::ApplyAndReduceUnitTests(); 
		} else if (curTest == "RestrictExists"){ 
			NWATests::test_restrict_exists_and_forall(); 
		} else if (curTest == "RestricExistsDeeper") {
			NWATests::test_restrict_exists_and_forall_deeper(); 
		} else if (curTest == "Test1") {
			NWATests::test1();
		} else if (curTest == "Test2") {
			NWATests::test2();
		} else if (curTest == "Test3") {
			NWATests::test3();
		} else if (curTest == "AllAssign") {
			NWATests::testAllAssignments(); 
		} else if (curTest == "Canonicalness") { 
			NWATests::testCanonicalness();  
		} else if (curTest == "DeMorgans") {
			NWATests::test_demorgans(); 
		} else if (curTest == "WeightGen") {
			NWATests::testWeights();
		} else if (curTest == "Schema") {
			NWATests::testSchema();
		} else if (curTest == "All") {
			NWATests::testAddition();*/
			NWATests::testIscas85();/*
			NWATests::testTopNodes();
			NWATests::testSatisfyingAssignments();
			NWATests::testMkIdRelationInterleaved();
			NWATests::testMkIdRelationNested(); 
			NWATests::testParity();
			NWATests::testAnd();
		    NWATests::testAddition();
			NWATests::test_restrict_exists_and_forall();
			//NWATests::test1();  Need ternary Apply&Reduce for this to work
			//NWATests::test2();  Need ternary Apply&Reduce
			//NWATests::test3();  Need ternary Apply&Reduce
			NWATests::testAllAssignments();/*
			NWATests::testCanonicalness();
			NWATests::test_demorgans();
			NWATests::testStepFunction();
		} else if (curTest == "cflobdd") {
			(*start)++;
			  DisposeOfPairProductCache();
  NWAOBDDNodeHandle::DisposeOfReduceCache();
			return true;
		} else {
			std::cout << "Unrecognized test name: " << curTest << std::endl;
		}
		(*start)++;
	}*/
	  DisposeOfPairProductMapCaches();
	  DisposeOfPairProductCache();
	  DisposeOfPathSummaryCache();
  NWAOBDDNodeHandle::DisposeOfReduceCache();
	return false;
}

void NWATests::test_mk_basis_vector() {
  // NWAOBDD<BIG_COMPLEX_FLOAT>F0 = VectorComplex::MkBasisVector(2, "00000000");
  // F0.DumpValueTuple();
  // F0.DumpPathCountings();
  auto f = VectorComplex::MkBasisVector(3, 12);
  auto g = VectorComplex::MkBasisVector(3, 12);
  auto h = f + g;
  h.DumpValueTuple();
  h.DumpPathCountings();

}

void NWATests::test_path_sampling1() {
  // XZ: This appears slower than what I expected.
  auto f1 = VectorComplex::MkBasisVector(1, 7);
  auto f2 = VectorComplex::MkBasisVector(1, 11);
  auto f3 = VectorComplex::MkBasisVector(1, 3);
  auto h = f1 + (f2 + f2 + f2) + (f3 + f3 + f3 + f3 + f3 + f3);
  std::string s;
  int occur[3] = {0, 0, 0};
  for(unsigned i = 0; i < 10000; ++i) {
    s = VectorComplex::Sampling(h, true);
    if(s == "1100") occur[0]++;
    else if(s == "1110") occur[1]++;
    else if(s == "1101") occur[2]++;
    else abort();
  }
  printf("%d %d %d\n", occur[0], occur[1], occur[2]);
}

void NWATests::test_path_sampling2() {
  // XZ: This appears slower than what I expected.
  auto f1 = VectorComplex::MkBasisVector(3, "0000000000000000");
  auto f2 = VectorComplex::MkBasisVector(3, "1111111111111111");
  auto f3 = VectorComplex::MkBasisVector(3, "0101010101010101");
  auto h = f1 + f2 + f3 + f2 + f3;
  VectorComplex::DumpVector(h);
  std::string s;
  int occur[3] = {0, 0, 0};
  for(unsigned i = 0; i < 10000; ++i) {
    s = VectorComplex::Sampling(h, true);
    if(s == "0000000000000000") occur[0]++;
    else if(s == "1111111111111111") occur[1]++;
    else if(s == "0101010101010101") occur[2]++;
    else abort();
  }
  printf("%d %d %d\n", occur[0], occur[1], occur[2]);
}
