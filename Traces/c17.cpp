#include "../cflobdd_int.h"
#include "../cflobdd_top_node_int.h"

using namespace std;
using namespace CFL_OBDD;
static void c17(){
// iscas85: c17 -----------------------------------------------------
CFLOBDD gat1, gat2, gat3, gat6, gat7, gat10, gat11,
        gat16, gat19, gat22, gat23;

gat1 = MkProjection(0);
gat2 = MkProjection(1);
gat3 = MkProjection(2);
gat6 = MkProjection(3);
gat7 = MkProjection(4);
gat10 = MkNand(gat1, gat3);
gat11 = MkNand(gat3, gat6);
gat16 = MkNand(gat2, gat11);
gat19 = MkNand(gat11, gat7);
gat22 = MkNand(gat10, gat16);
gat23 = MkNand(gat16, gat19);

unsigned int nodeCount, edgeCount;
cout << "CFLOBDD sizes" << endl;
GroupCountNodesAndEdgesStart(nodeCount, edgeCount);
gat22.GroupCountNodesAndEdges(nodeCount, edgeCount);
gat23.GroupCountNodesAndEdges(nodeCount, edgeCount);
GroupCountNodesAndEdgesEnd();
cout << nodeCount << ", " << edgeCount << endl;

// if MaxLevel is 3 or 4, test all assignments
if (CFLOBDD::maxLevel == 3 || CFLOBDD::maxLevel == 4) {

  cout << "Testing all assignments" << endl;

  unsigned int size = 1 << CFLOBDD::maxLevel;
  SH_OBDD::Assignment a(size);
  bool val22, val23;
  unsigned long int range = 1UL << size;
  for (unsigned long int i = 0UL; i < range; i++) {
    unsigned long int mask = 1UL;
    for (int j = size - 1; j >= 0; j--) {
      a[j] = (i & mask);
      mask = mask << 1;
    }
    val22 = gat22.Evaluate(a);
    val23 = gat23.Evaluate(a);
  
    bool temp1, temp2, temp3, temp6, temp7, temp10, temp11, temp16, temp19, temp22, temp23;
  
    temp1 = (bool)a[0];
    temp2 = (bool)a[1];
    temp3 = (bool)a[2];
    temp6 = (bool)a[3];
    temp7 = (bool)a[4];
    temp10 = !(temp1 && temp3);
    temp11 = !(temp3 && temp6);
    temp16 = !(temp2 && temp11);
    temp19 = !(temp11 && temp7);
    temp22 = !(temp10 && temp16);
    temp23 = !(temp16 && temp19);
  
    if (val22 != temp22) {
      cerr << "val22: " << val22 << " != " << temp22 << endl;
    }
    if (val23 != temp23) {
      cerr << "val23: " << val23 << " != " << temp23 << endl;
    }
  }
}
else {
  cout << "Cannot test all assignments: maxLevel must be 3 or 4" << endl;
}
}
