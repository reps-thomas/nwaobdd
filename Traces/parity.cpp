// parity -----------------------------------------------------
#include "../cflobdd_int.h"
#include "../cflobdd_top_node_int.h"

using namespace std;

static void iscasParity(){

// if MaxLevel is 4, run test
if (CFLOBDD::maxLevel == 4) {

  CFLOBDD parity16 = MkParity();
  CFLOBDD x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10,
          x11, x12, x13, x14, x15;

  x0 = MkProjection(0);
  x1 = MkProjection(1);
  x2 = MkProjection(2);
  x3 = MkProjection(3);
  x4 = MkProjection(4);
  x5 = MkProjection(5);
  x6 = MkProjection(6);
  x7 = MkProjection(7);
  x8 = MkProjection(8);
  x9 = MkProjection(9);
  x10 = MkProjection(10);
  x11 = MkProjection(11);
  x12 = MkProjection(12);
  x13 = MkProjection(13);
  x14 = MkProjection(14);
  x15 = MkProjection(15);
  
  CFLOBDD x0_1 = MkExclusiveOr(x0, x1);
  CFLOBDD x2_3 = MkExclusiveOr(x2, x3);
  CFLOBDD x4_5 = MkExclusiveOr(x4, x5);
  CFLOBDD x6_7 = MkExclusiveOr(x6, x7);
  CFLOBDD x8_9 = MkExclusiveOr(x8, x9);
  CFLOBDD x10_11 = MkExclusiveOr(x10, x11);
  CFLOBDD x12_13 = MkExclusiveOr(x12, x13);
  CFLOBDD x14_15 = MkExclusiveOr(x14, x15);
  
  CFLOBDD x0_3 = MkExclusiveOr(x0_1, x2_3);
  CFLOBDD x4_7 = MkExclusiveOr(x4_5, x6_7);
  CFLOBDD x8_11 = MkExclusiveOr(x8_9, x10_11);
  CFLOBDD x12_15 = MkExclusiveOr(x12_13, x14_15);
  
  CFLOBDD x0_7 = MkExclusiveOr(x0_3, x4_7);
  CFLOBDD x8_15 = MkExclusiveOr(x8_11, x12_15);
  
  CFLOBDD x0_15 = MkExclusiveOr(x0_7, x8_15);
  
  if (x0_15 == parity16) {
    cout << "x0_15 == parity16" << endl;
  }
  else {
    cout << "x0_15 != parity16" << endl;
  }
  
  unsigned int nodeCount, edgeCount;
  unsigned int returnEdgesCount, returnEdgesObjCount;
  x0_15.CountNodesAndEdges(nodeCount, edgeCount, returnEdgesCount, returnEdgesObjCount);
  cout << nodeCount << ", " << edgeCount << endl;
  
  cout << "Testing all assignments" << endl;

  unsigned int size = 1 << CFLOBDD::maxLevel;
  Assignment a(size);
  bool val0_15;
  unsigned long int range = 1UL << size;
  for (unsigned long int i = 0UL; i < range; i++) {
    unsigned long int mask = 1UL;
    bool bb = false;
    for (int j = size - 1; j >= 0; j--) {
      a[j] = (i & mask);
      bb ^= a[j];
      mask = mask << 1;
    }
    val0_15 = x0_15.Evaluate(a);
    if (val0_15 != bb) {
      cerr << a << ": " << val0_15 << endl;
    }
  }
}
else {
  cout << "Cannot run test: maxLevel must be 4" << endl;
}
}
