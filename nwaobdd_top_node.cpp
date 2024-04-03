#include "nwaobdd_top_node.h"

namespace NWA_OBDD {



enum IDTYPE {MATCH, PREV0, PREV1};
NWAOBDDNodeHandle MkIdRelationInterleaved(unsigned int level, IDTYPE idtype)
{
  NWAOBDDInternalNode *n;

  if (level == 0) {
    return NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle;
  }
  else if (level == 1) {
      n = new NWAOBDDInternalNode(level);
      if(idtype == MATCH){
          ReturnMapHandle<intpair> m01, m10, m11;
          m01.AddToEnd(intpair(0,1));
		  m01.Canonicalize();

          m10.AddToEnd(intpair(1,0));
		  m10.Canonicalize();

		  m11.AddToEnd(intpair(1,1));
		  m11.Canonicalize();

          NWAOBDDNodeHandle temp = MkIdRelationInterleaved(level-1, idtype);
          n->AConnection[0] = Connection(temp,m01);
          n->AConnection[1] = Connection(temp,m10);

          n->numBConnections = 2;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];
          n->BConnection[0][0] = Connection(temp,m01);
          n->BConnection[1][0] = Connection(temp,m10);
          n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m11);
          n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m11);
          n->numExits = 2;
      }
      else if (idtype == PREV0)
      {
		  ReturnMapHandle<intpair> m01, m22;
          m01.AddToEnd(intpair(0,1));
		  m01.Canonicalize();

          m22.AddToEnd(intpair(2,2));
		  m22.Canonicalize();

          NWAOBDDNodeHandle temp = MkIdRelationInterleaved(level-1, idtype);

          n->AConnection[0] = Connection(temp,m01);
          n->AConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m22);

          n->numBConnections = 3;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];
          n->BConnection[0][0] = Connection(temp,m01);
          n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m22);
          n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m22);
          n->BConnection[1][1] = Connection(temp, m01);
          n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m22);
          n->BConnection[1][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m22);
          n->numExits = 3;
      } else {
          ReturnMapHandle<intpair> m00, m12;
          m00.AddToEnd(intpair(0,0));
		  m00.Canonicalize();

          m12.AddToEnd(intpair(1,2));
          m12.Canonicalize();

          NWAOBDDNodeHandle temp = MkIdRelationInterleaved(level-1, idtype);

          n->AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
          n->AConnection[1] = Connection(temp,m12);

          n->numBConnections = 3;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];
          n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
          n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
          n->BConnection[0][1] = Connection(temp, m12);
          n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
          n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1], m00);
          n->BConnection[1][2] = Connection(temp, m12);
          n->numExits = 3;
      }
  }
  else {  // Create an appropriate NWAOBDDInternalNode
      n = new NWAOBDDInternalNode(level);
      if (idtype == MATCH){
          ReturnMapHandle<intpair> m01_10_11,m11_01_10;
          m01_10_11.AddToEnd(intpair(0,1));
          m01_10_11.AddToEnd(intpair(1,0));
          m01_10_11.AddToEnd(intpair(1,1));
		  m01_10_11.Canonicalize();

		  m11_01_10.AddToEnd(intpair(1,1));
          m11_01_10.AddToEnd(intpair(0,1));
          m11_01_10.AddToEnd(intpair(1,0));
          m11_01_10.Canonicalize();

          NWAOBDDNodeHandle temp0 = MkIdRelationInterleaved(level-1, PREV0);
          NWAOBDDNodeHandle temp1 = MkIdRelationInterleaved(level-1, PREV1);

          n->AConnection[0] = Connection(temp0,m01_10_11);
          n->AConnection[1] = Connection(temp1,m11_01_10);

		  ReturnMapHandle<intpair> m11;
		  m11.AddToEnd(intpair(1,1));
		  m11.Canonicalize();

          n->numBConnections = 2;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];
          n->BConnection[0][0] = Connection(temp0,m01_10_11);
          n->BConnection[1][0] = Connection(temp1,m11_01_10);
          n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
          n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
          n->numExits = 2;
      } else if (idtype == PREV0)
      {
          ReturnMapHandle<intpair> m01_22,m22;

          m01_22.AddToEnd(intpair(0,1));
		  m01_22.AddToEnd(intpair(2,2));
          m01_22.Canonicalize();
          
		  m22.AddToEnd(intpair(2,2));
          m22.Canonicalize();

          NWAOBDDNodeHandle temp = MkIdRelationInterleaved(level-1, MATCH);

          n->AConnection[0] = Connection(temp,m01_22);
          n->AConnection[1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m22);

          n->numBConnections = 3;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];
          n->BConnection[0][0] = Connection(temp, m01_22);
          n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m22);
          n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m22);
          n->BConnection[1][1] = Connection(temp, m01_22);
          n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m22);
          n->BConnection[1][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m22);
          n->numExits = 3;
      } else {
          ReturnMapHandle<intpair> m00, m12_00;
          m00.AddToEnd(intpair(0,0));
		  m00.Canonicalize();

		  m12_00.AddToEnd(intpair(1,2));
		  m12_00.AddToEnd(intpair(0,0));
          m12_00.Canonicalize();

          NWAOBDDNodeHandle temp = MkIdRelationInterleaved(level-1, MATCH);

          n->AConnection[0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
          n->AConnection[1] = Connection(temp,m12_00);

          n->numBConnections = 3;
          n->BConnection[0] = new Connection[n->numBConnections];
          n->BConnection[1] = new Connection[n->numBConnections];

          n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
          n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
          n->BConnection[0][1] = Connection(temp,m12_00);
          n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
          n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
          n->BConnection[1][2] = Connection(temp,m12_00);
          n->numExits = 3;
      }
  }
  // n->numExits = 2;
// #ifdef PATH_COUNTING_ENABLED
//   n->InstallPathCounts();
// #endif
  return NWAOBDDNodeHandle(n);
} // MkIdRelationInterleaved


NWAOBDDNodeHandle MkAdditionNested(unsigned int level, bool carry)
{
    NWAOBDDInternalNode *n;

    assert(level >= 1);

    if(level == 1){
        n = new NWAOBDDInternalNode(level);
        ReturnMapHandle<intpair> m00,m01,m11,m12,m22;

		m00.AddToEnd(intpair(0,0));
		m00.Canonicalize();

        m01.AddToEnd(intpair(0,1));
        m01.Canonicalize();

		m11.AddToEnd(intpair(1,1));
		m11.Canonicalize();

        m12.AddToEnd(intpair(1,2));
        m12.Canonicalize();

        m22.AddToEnd(intpair(2,2));
        m22.Canonicalize();

        
        if(!carry) {
        // 3-return points for A's 0,1,2 - 00, 01/10, 11
        // Return Map for 0-AConnection: <0,1>
        // Return Map for 1-AConnection: <1,2>
            n->AConnection[0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m01);
            n->AConnection[1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m12);
    
            n->numBConnections = 3;
            n->BConnection[0] = new Connection[n->numBConnections];
            n->BConnection[1] = new Connection[n->numBConnections];

        // 3-return points for B's 0,1,2 -- True, False, True + Carry
        // Return Map for B0's 0-Connection: <0,0>
        // Return Map for B0's 1-Connection: <1,1>
        // Return Map for B1's 0-Connection: <1,1>
        // Return Map for B1's 1-Connection: <0,0>
        // Return Map for B2's 0-Connection: <2,2>
        // Return Map for B2's 1-Connection: <1,1>
            n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m00);
            n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m11);
            n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m11);
            n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m00);
            n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m22);
            n->BConnection[1][2] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m11);
            n->numExits = 3;
        } else { // carry = true
            //3 return points for the A-Connections 0, 1, 2 -- 00+1, 01/10 + 1, 11 +1
            //Return Map for A's 0-Connection: 0:0, 1:1
            //Return Map for A's 1-Connection: 0:1, 1:2
            n->AConnection[0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m01);
            n->AConnection[1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m12);

            n->numBConnections = 3;
            n->BConnection[0] = new Connection[n->numBConnections];
            n->BConnection[1] = new Connection[n->numBConnections];

            //There are 3 B-return points 0,1,2 -- F,T,C
            //Return Map for B0's 0 Connection: 0:0, 1:0
            //Return Map for B0's 1 Connection: 0:1, 1:1
            //Return Map for B1's 0-Connection: 0:2, 1:2
            //Return Map for B1's 1-Connection: 0:0, 1:0
            //Return Map for B2's 0-Connection: 0:0, 1:0
            //Return Map for B2's 1-Connection: 0:2, 1:2
            n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m00);
            n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m11);
            n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m22);
            n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m00);
            n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m00);
            n->BConnection[1][2] = Connection(NWAOBDDNodeHandle::NWAOBDDEpsilonNodeHandle, m22);
        }
    } else //level >=2
    {
        n = new NWAOBDDInternalNode(level); 
        ReturnMapHandle<intpair> m11, m00, m00_11_22, m00_12_23, m01_22_13, m00_23_34, m11_00_22, m13_22_34; 
        
        m11.AddToEnd(intpair(1,1));
        m11.Canonicalize();

		m00.AddToEnd(intpair(0, 0));
		m00.Canonicalize();

        m00_11_22.AddToEnd(intpair(0,0));
        m00_11_22.AddToEnd(intpair(1,1));
        m00_11_22.AddToEnd(intpair(2,2));
        m00_11_22.Canonicalize();

        m00_12_23.AddToEnd(intpair(0,0));
        m00_12_23.AddToEnd(intpair(1,2));
        m00_12_23.AddToEnd(intpair(2,3));
        m00_12_23.Canonicalize();

        m01_22_13.AddToEnd(intpair(0,1));
        m01_22_13.AddToEnd(intpair(2,2));
        m01_22_13.AddToEnd(intpair(1,3));
        m01_22_13.Canonicalize();

		m00_23_34.AddToEnd(intpair(0,0));
        m00_23_34.AddToEnd(intpair(2,3));
        m00_23_34.AddToEnd(intpair(3,4));
        m00_23_34.Canonicalize();

        m11_00_22.AddToEnd(intpair(1,1));
        m11_00_22.AddToEnd(intpair(0,0));
        m11_00_22.AddToEnd(intpair(2,2));
        m11_00_22.Canonicalize();

        m13_22_34.AddToEnd(intpair(1,3));
        m13_22_34.AddToEnd(intpair(2,2));
        m13_22_34.AddToEnd(intpair(3,4));
        m13_22_34.Canonicalize();

        NWAOBDDNodeHandle c0 = MkAdditionNested(level-1, false);
        NWAOBDDNodeHandle c1 = MkAdditionNested(level-1, true);
        if (!carry)
        {
            // A-Connections are (i-1) no carries
            // Their returns are T,F,c
            // There are 5 in between points
            // A0's Return Map is <0,1> <2,2> <1,3>
            // A1's Return Map is <1,3> <2,2> <3,4>
            n->AConnection[0] = Connection(c0,m01_22_13);
            n->AConnection[1] = Connection(c0,m13_22_34);

            n->numBConnections = 5;
            n->BConnection[0] = new Connection[n->numBConnections];
            n->BConnection[1] = new Connection[n->numBConnections];

            //There are 5 B-Connections 0: 0, 1:1, 2:false, 3:0/Carry 4: 1/carry
            n->BConnection[0][0] = Connection(c0, m00_11_22);
            n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[0][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[1][1] = Connection(c0, m00_11_22);
            n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[1][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[0][3] = Connection(c1, m11_00_22);
            n->BConnection[1][3] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[0][4] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m11);
            n->BConnection[1][4] = Connection(c1, m11_00_22);
        } else { //carry = true
            n->AConnection[0] = Connection(c1,m00_12_23);
            n->AConnection[1] = Connection(c1,m00_23_34);
            
            n->numBConnections = 5;
            n->BConnection[0] = new Connection[n->numBConnections];
            n->BConnection[1] = new Connection[n->numBConnections];

            n->BConnection[0][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[1][0] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[0][1] = Connection(c0,m11_00_22);
            n->BConnection[1][1] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[0][2] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[1][2] = Connection(c0,m11_00_22);
            n->BConnection[0][3] = Connection(c1,m00_11_22);
            n->BConnection[1][3] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[0][4] = Connection(NWAOBDDNodeHandle::NoDistinctionNode[level-1],m00);
            n->BConnection[1][4] = Connection(c1,m00_11_22);
        }
    }
    n->numExits = 3;
// #ifdef PATH_COUNTING_ENABLED
//     n->InstallPathCounts();
// #endif
  return NWAOBDDNodeHandle(n);
}


//********************************************************************
// NWAOBDDTopNode
//********************************************************************

// Initializations of static members ---------------------------------

template<typename T>
unsigned int const NWAOBDDTopNode<T>::maxLevel = NWAOBDDMaxLevel;

template<typename T>
Hashset<NWAOBDDTopNode<T>> *NWAOBDDTopNode<T>::computedCache = new Hashset<NWAOBDDTopNode>(10000);


// Constructors/Destructor -------------------------------------------
template<typename T>
NWAOBDDTopNode<T>::NWAOBDDTopNode(NWAOBDDNode *n, ReturnMapHandle<T>(&mapHandle))
{
	rootConnection = ConnectionT(n, mapHandle);
  level = n->level;
}

template<typename T>
NWAOBDDTopNode<T>::NWAOBDDTopNode(NWAOBDDNodeHandle &nodeHandle, ReturnMapHandle<T>(&mapHandle)) 
{
	rootConnection = ConnectionT(nodeHandle, mapHandle);
  level = nodeHandle.handleContents->level;
}

template<typename T>
NWAOBDDTopNode<T>::~NWAOBDDTopNode()
{
}

template<typename T>
void NWAOBDDTopNode<T>::DeallocateMemory()
{
	// NWAOBDDTopNode::~NWAOBDDTopNode();
}

// Evaluate
//    Return the value of the Boolean function under the given assignment
template<typename T>
T NWAOBDDTopNode<T>::Evaluate(SH_OBDD::Assignment &assignment)
{
  SH_OBDD::AssignmentIterator ai(assignment);
  int i = rootConnection.entryPointHandle->handleContents->Traverse(ai);
  T ans = rootConnection.returnMapHandle.Lookup(i);
  return ans;
}


//ETTODO IterativeEvaluation
// EvaluateIteratively
//    Return the value of the Boolean function under the given assignment
template<typename T>
T NWAOBDDTopNode<T>::EvaluateIteratively(SH_OBDD::Assignment &assignment)
{
  // return true; //ETTODO
    SH_OBDD::AssignmentIterator ai(assignment);
  int i = rootConnection.entryPointHandle->handleContents->Traverse(ai);
  T ans = rootConnection.returnMapHandle.Lookup(i);
  return ans;
  /*
  AssignmentIterator ai(assignment);

  unsigned int exitIndex = 0;
  ConsCell<TraverseState> *S = NULL;
  TraverseState ts;
  bool ans;

  S = new ConsCell<TraverseState>(TraverseState(rootConnection.entryPointHandle->handleContents,FirstVisit), S);
  while (S != NULL) {
    ts = S->Item();
    S = S->Next();
    if (ts.node->NodeKind() == NWAOBDD_EPSILON) {
      ai.Next();
      exitIndex = 0;
    }
    else if (ts.node->NodeKind() == NWAOBDD_FORK) {
      bool val = ai.Current();
      ai.Next();
      exitIndex = (int)val;
    }
    else {  // Must be a NWAOBDDInternalNode
      NWAOBDDInternalNode *n = (NWAOBDDInternalNode *)ts.node;

      if (ts.visitState == FirstVisit) {
        S = new ConsCell<TraverseState>(TraverseState(n, SecondVisit), S);
        S = new ConsCell<TraverseState>(TraverseState(n->AConnection.entryPointHandle->handleContents, FirstVisit), S);
      }
      else if (ts.visitState == SecondVisit) {
        int i = n->AConnection.returnMapHandle.Lookup(exitIndex);
        S = new ConsCell<TraverseState>(TraverseState(n, ThirdVisit, i), S);
        S = new ConsCell<TraverseState>(TraverseState(n->BConnection[i].entryPointHandle->handleContents, FirstVisit), S);
      }
      else {  // if (ts.visitState == ThirdVisit)
        exitIndex = n->BConnection[ts.index].returnMapHandle.Lookup(exitIndex);
      }
    }
  }
  ans = rootConnection.returnMapHandle.Lookup(exitIndex);
  return ans;*/
}

// PrintYield -----------------------------------------------------

//ETTODO PrintYield
// PrintYieldAux
template<typename T>
void NWAOBDDTopNode<T>::PrintYieldAux(std::ostream * out, List<ConsCell<TraverseState> *> &L, ConsCell<TraverseState> *S)
{//ETTODO
  unsigned int exitIndex = 0;
  TraverseState ts;
    T ans;

  while (S != NULL) {
    ts = S->Item();
    S = S->Next();
    if (ts.node->NodeKind() == NWAOBDD_EPSILON) {
        exitIndex = 0;
    }
    else {  // Must be a NWAOBDDInternalNode
      NWAOBDDInternalNode *n = (NWAOBDDInternalNode *)ts.node;

      if (ts.visitState == FirstVisit) {
		  L.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node,RestartFirst), S));
        S = new ConsCell<TraverseState>(TraverseState(n, SecondVisit, 0, 0), S);
        S = new ConsCell<TraverseState>(TraverseState(n->AConnection[0].entryPointHandle->handleContents, FirstVisit), S);
      } else if (ts.visitState == RestartFirst) {
        S = new ConsCell<TraverseState>(TraverseState(n, SecondVisit, 0, 1), S);
        S = new ConsCell<TraverseState>(TraverseState(n->AConnection[1].entryPointHandle->handleContents, FirstVisit), S);
	  } else if (ts.visitState == SecondVisit) {
		  L.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node,SecondVisitOne, 0, ts.val1, 0, exitIndex), S));
          int i = n->AConnection[ts.val1].returnMapHandle.Lookup(exitIndex).First();
		  L.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node,RestartSecond, i, ts.val1, 0), S));
        S = new ConsCell<TraverseState>(TraverseState(n, ThirdVisit, i, ts.val1, 0), S);
        S = new ConsCell<TraverseState>(TraverseState(n->BConnection[0][i].entryPointHandle->handleContents, FirstVisit), S);
      } else if (ts.visitState == RestartSecond) {
		  S = new ConsCell<TraverseState>(TraverseState(n, ThirdVisit, ts.index, ts.val1, 1), S);
        S = new ConsCell<TraverseState>(TraverseState(n->BConnection[1][ts.index].entryPointHandle->handleContents, FirstVisit), S);
	  } else if (ts.visitState == SecondVisitOne) {
		  int i = n->AConnection[ts.val1].returnMapHandle.Lookup(ts.exitIndex).Second();
		  L.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node,RestartSecond, i, ts.val1, 0), S));
		  S = new ConsCell<TraverseState>(TraverseState(n, ThirdVisit, i, ts.val1, 0), S);
		  S = new ConsCell<TraverseState>(TraverseState(n->BConnection[0][i].entryPointHandle->handleContents, FirstVisit), S);
	  } else if (ts.visitState == ThirdVisit) {
		  L.AddToFront(new ConsCell<TraverseState>(TraverseState(ts.node,RestartThird, ts.index, 0, ts.val2, exitIndex), S));
		  exitIndex = n->BConnection[ts.val2][ts.index].returnMapHandle.Lookup(exitIndex).First();
	  } else {  // if (ts.visitState == RestartThird)
        exitIndex = n->BConnection[ts.val2][ts.index].returnMapHandle.Lookup(ts.exitIndex).Second();
      }
    }
  }
  ans = rootConnection.returnMapHandle.Lookup(exitIndex);
  if (out != NULL) *out << ans;
}

//ETTODO PrintYield
// PrintYield
//
// print the yield of the NWAOBDDTopNode (i.e., the leaves of 0's and 1's
// in "left-to-right order").
//
template<typename T>
void NWAOBDDTopNode<T>::PrintYield(std::ostream * out)
{
  ConsCell<TraverseState> *S = NULL;   // Traversal stack
  List<ConsCell<TraverseState> *> L;   // Snapshot stack

  S = new ConsCell<TraverseState>(TraverseState(rootConnection.entryPointHandle->handleContents,FirstVisit), S);
  PrintYieldAux(out, L, S);
  while (!L.IsEmpty()) {
    S = L.RemoveFirst();
    PrintYieldAux(out, L, S);
  }
}

template<typename T>
void NWAOBDDTopNode<T>::DumpPathCountings() {
  NWAOBDDNode* n = rootConnection.entryPointHandle->handleContents;
  for(unsigned i = 0; i < n -> numExits; ++i)
    printf("%.0lf ", pow(2, n->numPathsToExit[i]) + 1e-7);
  printf("\n");
}

template<typename T>
void NWAOBDDTopNode<T>::DumpValueTuple() {
  ReturnMapHandle<T> rmh = rootConnection.returnMapHandle;
  for(unsigned i = 0; i < rmh.Size(); ++i)
    std::cout << rmh[i] << ' ';
  std::cout << '\n';
}

// Satisfaction Operations ------------------------------------

// FindOneSatisfyingAssignment
//
// If a satisfying assignment exists, allocate and place such an
//    assignment in variable "assignment" and return true.
// Otherwise return false.
//
// Running time: Linear in the number of variables

template<typename T> // XZ: this was commented out for compilation
bool NWAOBDDTopNode<T>::FindOneSatisfyingAssignment(SH_OBDD::Assignment * &assignment)
{
  for (unsigned int i = 0; i < rootConnection.entryPointHandle->handleContents->numExits; i++) {
    T k = rootConnection.returnMapHandle.Lookup(i);
    if (k) {  // A satisfying assignment must exist
      unsigned int size = ((unsigned int)((((unsigned int)1) << (level + 2)) - (unsigned int)4));
      assignment = new SH_OBDD::Assignment(size);
      rootConnection.entryPointHandle->handleContents->FillSatisfyingAssignment(i, *assignment, size);
      return true;
    }
  }
  return false;
}

template<typename T>
void NWAOBDDTopNode<T>::DumpConnections(Hashset<NWAOBDDNode> *visited, std::ostream & out /* = std::cout */)
{
  rootConnection.entryPointHandle->handleContents->DumpConnections(visited, out);
  out << rootConnection << std::endl;
}

//ETTODO CountNodesAndEdges
template<typename T>
void NWAOBDDTopNode<T>::CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *visitedEdges, unsigned int &nodeCount, unsigned int &edgeCount)
{//ETTODO
  rootConnection.entryPointHandle->handleContents->CountNodesAndEdges(visitedNodes, visitedEdges, nodeCount, edgeCount);
  return;
  // if (visitedEdges->Lookup(rootConnection.returnMapHandle.mapContents) == NULL) {
  //   visitedEdges->Insert(rootConnection.returnMapHandle.mapContents);
  //   edgeCount += rootConnection.returnMapHandle.Size();
  // }
}

// Hash
template<typename T>
unsigned int NWAOBDDTopNode<T>::Hash(unsigned int modsize)
{
  return rootConnection.Hash(modsize);
}

// Overloaded !=
template<typename T>
bool NWAOBDDTopNode<T>::operator!= (const NWAOBDDTopNode & C)
{
  return rootConnection != C.rootConnection;
}

// Overloaded ==
template<typename T>
bool NWAOBDDTopNode<T>::operator== (const NWAOBDDTopNode & C)
{
  return rootConnection == C.rootConnection;
}

// print
template<typename T>
std::ostream& NWAOBDDTopNode<T>::print(std::ostream & out) const
{
  out << rootConnection.entryPointHandle << std::endl;
  out << rootConnection.returnMapHandle << std::endl;
  return out;
}

}


namespace NWA_OBDD {

void setMaxLevel(unsigned int level)
{
	NWAOBDDMaxLevel = level;
}

template<typename T>
std::ostream& operator<< (std::ostream & out, const NWAOBDDTopNode<T> &d)
{
  d.print(out);
  return(out);
}

// NWAOBDDTopNode-creation operations --------------------------------------

// Create representation of \x.true
NWAOBDDTopNodeRefPtr MkTrueTop()
{
  NWAOBDDTopNodeRefPtr v;
  ReturnMapHandle<int> m;

  m.AddToEnd(1);  // Map the one exit of the body to T
  m.Canonicalize();
  
  v = new NWAOBDDTopNode<int>(NWAOBDDNodeHandle::NoDistinctionNode[NWAOBDDMaxLevel], m);
  return v;
}

// Create representation of \x.false
NWAOBDDTopNodeRefPtr MkFalseTop()
{
  NWAOBDDTopNodeRefPtr v;
  ReturnMapHandle<int> m;

  m.AddToEnd(0);  // Map the one exit of the body to F
  m.Canonicalize();

  v = new NWAOBDDTopNode<int>(NWAOBDDNodeHandle::NoDistinctionNode[NWAOBDDMaxLevel], m);
  return v;
}

// Create representation of \x.x_i
NWAOBDDTopNodeRefPtr MkDistinction(unsigned int i)
{
  NWAOBDDTopNodeRefPtr v;
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle<int> m;

  assert(i < ((((unsigned int)1) << (NWAOBDDMaxLevel + 2)) - (unsigned int)4));   // i.e., i < 2**maxLevel

  tempHandle = MkDistinction(NWAOBDDMaxLevel, i);
  m.AddToEnd(0);
  m.AddToEnd(1);
  m.Canonicalize();

  v = new NWAOBDDTopNode<int>(tempHandle, m);
  return v;
}

// Create representation of identity relation (with interleaved variable order).
// That is, input (x0,y0,x1,y1,...,xN,yN) yield Id[(x0,x1,...,xN)][(y0,y1,...,yN)]
// which equals 1 iff xi == yi, for 0 <= i <= N.
NWAOBDDTopNodeRefPtr MkIdRelationInterleavedTop()
{
  NWAOBDDTopNodeRefPtr v;
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle<int> m;

  tempHandle = MkIdRelationInterleaved(NWAOBDDMaxLevel, MATCH);
  m.AddToEnd(1);
  m.AddToEnd(0);
  m.Canonicalize();

  v = new NWAOBDDTopNode<int>(tempHandle, m);
  return v;
}

NWAOBDDTopNodeRefPtr MkIdRelationNestedTop()
{
  NWAOBDDTopNodeRefPtr v;
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle<int> m;

  tempHandle = MkIdRelationNested(NWAOBDDMaxLevel);
  m.AddToEnd(1);
  m.AddToEnd(0);
  m.Canonicalize();

  v = new NWAOBDDTopNode<int>(tempHandle, m);
  return v;
}

// Create representation of addition relation with interleaved variables
// { (xi yi zi _)* | vec{x} + vec{y} = vec{z} }
NWAOBDDTopNodeRefPtr MkAdditionNestedTop()
{
  NWAOBDDTopNodeRefPtr v;
  NWAOBDDNodeHandle n;
  ReturnMapHandle<int> m10;

  n = MkAdditionNested(NWAOBDDMaxLevel, false);

  //ETTODO FixReductionMap
  // Reduce n by mapping the "carry=0" and "carry=1" exits to accept
     m10.AddToEnd(1);
     m10.AddToEnd(0);
     m10.Canonicalize();
     ReductionMapHandle reductionMapHandle;
     reductionMapHandle.AddToEnd(0);
     reductionMapHandle.AddToEnd(1);
     reductionMapHandle.AddToEnd(0);
     //     NWAOBDDNodeHandle::InitReduceCache();
     NWAOBDDNodeHandle reduced_n = n.Reduce(reductionMapHandle, m10.Size());
     //     NWAOBDDNodeHandle::DisposeOfReduceCache();


  // Create and return NWAOBDDTopNode
     v = new NWAOBDDTopNode<int>(reduced_n, m10);
     return(v);
}


//ETTODO Detensor
// Create representation of (W,X,Y,Z) s.t. X==Y with interleaved variables
NWAOBDDTopNodeRefPtr MkDetensorConstraintInterleavedTop()
{
  NWAOBDDTopNodeRefPtr v;/*ETTODO 
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle m;

  tempHandle = MkDetensorConstraintInterleaved(NWAOBDDMaxLevel);
  m.AddToEnd(0);
  m.AddToEnd(1);
  m.Canonicalize();
  v = new NWAOBDDTopNode(tempHandle, m);*/
  return v;
}

// Create representation of parity function
NWAOBDDTopNodeRefPtr MkParityTop()
{
  NWAOBDDTopNodeRefPtr v; 
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle<int> m;

  tempHandle = MkParity(NWAOBDDMaxLevel);
  m.AddToEnd(0);
  m.AddToEnd(1);
  m.Canonicalize();

  v = new NWAOBDDTopNode<int>(tempHandle, m);
  return v;
}

//ETTODO Walsh

// Create representation of the Walsh matrix W(2**(i-1))
// [i.e., a matrix of size 2**(2**(i-1))) x 2**(2**(i-1)))]
// with interleaved indexing of components: that is, input
// (x0,y0,x1,y1,...,xN,yN) yields W[(x0,x1,...,xN)][(y0,y1,...,yN)]
NWAOBDDTopNodeRefPtr MkWalshInterleavedTop(unsigned int i)
{
  NWAOBDDTopNodeRefPtr v;/*ETTODO 
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle m;

  assert(i <= NWAOBDDMaxLevel);

  tempHandle = MkWalshInterleavedNode(i);
  m.AddToEnd(1);
  m.AddToEnd(-1);
  m.Canonicalize();
  v = new NWAOBDDTopNode(tempHandle, m);*/
  return v;
}

//ETTODO ReedMuller
// Create representation of the Inverse Reed-Muller matrix IRM(2**(i-1))
// [i.e., a matrix of size 2**(2**(i-1))) x 2**(2**(i-1)))]
// with interleaved indexing of components: that is, input
// (x0,y0,x1,y1,...,xN,yN) yields IRM[(x0,x1,...,xN)][(y0,y1,...,yN)]
NWAOBDDTopNodeRefPtr MkInverseReedMullerInterleavedTop(unsigned int i)
{
  NWAOBDDTopNodeRefPtr v;/*ETTODO 
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle m;

  assert(i <= NWAOBDDMaxLevel);

  tempHandle = MkInverseReedMullerInterleavedNode(i);
  m.AddToEnd(1);
  m.AddToEnd(0);
  m.AddToEnd(2);
  m.Canonicalize();
  v = new NWAOBDDTopNode(tempHandle, m);*/
  return v;
}

//ETTODO StepFunctions
// Create representation of step function
NWAOBDDTopNodeRefPtr MkStepUpOneFourthTop()
{
  NWAOBDDTopNodeRefPtr v;/*ETTODO
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle m;

  assert(NWAOBDDMaxLevel >= 1);
  tempHandle = MkStepOneFourth(NWAOBDDMaxLevel);
  m.AddToEnd(0);
  m.AddToEnd(1);
  m.Canonicalize();
  v = new NWAOBDDTopNode(tempHandle, m);*/
  return v;
}

// Create representation of step function
NWAOBDDTopNodeRefPtr MkStepDownOneFourthTop()
{
  return MkNot(MkStepUpOneFourthTop());
}

//ETTODO Step Functions
#ifdef ARBITRARY_STEP_FUNCTIONS
// Create representation of step function
NWAOBDDTopNodeRefPtr MkStepUpTop(unsigned int i)
{
  NWAOBDDTopNodeRefPtr v;
  NWAOBDDNodeHandle tempHandle;
  ReturnMapHandle m;

  assert(NWAOBDDMaxLevel <= 5);
  tempHandle = MkStepNode(NWAOBDDMaxLevel, i, 0, (1 << (1 << NWAOBDDMaxLevel)) - i);
  if (i != 0) {
    m.AddToEnd(0);
  }
  if (i != (1 << (1 << NWAOBDDMaxLevel))) {
    m.AddToEnd(1);
  }
  m.Canonicalize();
  v = new NWAOBDDTopNode(tempHandle, m);
  return v;
}

// Create representation of step function
NWAOBDDTopNodeRefPtr MkStepDownTop(unsigned int i)
{
  return MkNot(MkStepUpTop(i));
}
#endif

// Unary operations on NWAOBDDTopNodes --------------------------------------

// Implements \f.!f
NWAOBDDTopNodeRefPtr MkNot(NWAOBDDTopNodeRefPtr f)
{
  NWAOBDDTopNodeRefPtr answer;
  ReturnMapHandle<int> m = f->rootConnection.returnMapHandle.Complement();
  answer = new NWAOBDDTopNode<int>(*(f->rootConnection.entryPointHandle), m);
  return answer;
}

// Binary operations on NWAOBDDTopNodes ------------------------------------

// \f.\g.(f && g)
NWAOBDDTopNodeRefPtr MkAnd(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce<int>(f, g, andOp);
}

// \f.\g.!(f && g)
NWAOBDDTopNodeRefPtr MkNand(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce<int>(f, g, nandOp);
}

// \f.\g.(f || g)
NWAOBDDTopNodeRefPtr MkOr(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce<int>(f, g, orOp);
}

// \f.\g.!(f || g)
NWAOBDDTopNodeRefPtr MkNor(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce<int>(f, g, norOp);
}

// \f.\g.(f == g)
NWAOBDDTopNodeRefPtr MkIff(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce<int>(f, g, iffOp);
}

// \f.\g.(f != g)
NWAOBDDTopNodeRefPtr MkExclusiveOr(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce<int>(f, g, exclusiveOrOp);
}

// \f.\g.(!f || g)
NWAOBDDTopNodeRefPtr MkImplies(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce<int>(f, g, impliesOp);
}

// \f.\g.(f && !g)
NWAOBDDTopNodeRefPtr MkMinus(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce<int>(f, g, minusOp);
}

// \f.\g.(!g || f)
NWAOBDDTopNodeRefPtr MkQuotient(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce<int>(f, g, quotientOp);
}

// \f.\g.(g && !f)
NWAOBDDTopNodeRefPtr MkNotQuotient(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g)
{
  return ApplyAndReduce<int>(f, g, notQuotientOp);
}

// \f.\g.f
NWAOBDDTopNodeRefPtr MkFirst(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr )
{
  return f;
}

// \f.\g.!f
NWAOBDDTopNodeRefPtr MkNotFirst(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr )
{
  return MkNot(f);
}

// \f.\g.g
NWAOBDDTopNodeRefPtr MkSecond(NWAOBDDTopNodeRefPtr , NWAOBDDTopNodeRefPtr g)
{
  return g;
}

// \f.\g.!g
NWAOBDDTopNodeRefPtr MkNotSecond(NWAOBDDTopNodeRefPtr , NWAOBDDTopNodeRefPtr g)
{
  return MkNot(g);
}

// N-ary operations on NWAOBDDTopNodes -----------------------------------

// \f1. ... \fk.(f1 && ... && fk)
// NWAOBDDTopNodeRefPtr  MkAnd(int N, ...)
// {
//   NWAOBDDTopNodeRefPtr  temp;

//   assert(N >= 2);
//   va_list ap;
//   va_start(ap, N);
//   temp = va_arg(ap, NWAOBDDTopNodeRefPtr );
//   for (int k = 1; k < N; k++) {
//     temp = MkAnd(temp, va_arg(ap, NWAOBDDTopNodeRefPtr ));
//   }
//   va_end(ap);
//   return temp;
// }

// // \f1. ... \fk.!(f1 && ... && fk)
// NWAOBDDTopNodeRefPtr  MkNand(int N, ...)
// {
//   NWAOBDDTopNodeRefPtr  temp;

//   assert(N >= 2);
//   va_list ap;
//   va_start(ap, N);
//   temp = va_arg(ap, NWAOBDDTopNodeRefPtr );
//   for (int k = 1; k < N; k++) {
//     temp = MkAnd(temp, va_arg(ap, NWAOBDDTopNodeRefPtr ));
//   }
//   va_end(ap);
//   return MkNot(temp);
// }

// // \f1. ... \fk.(f1 || ... || fk)
// NWAOBDDTopNodeRefPtr  MkOr(int N, ...)
// {
//   NWAOBDDTopNodeRefPtr  temp;

//   assert(N >= 2);
//   va_list ap;
//   va_start(ap, N);
//   temp = va_arg(ap, NWAOBDDTopNodeRefPtr );
//   for (int k = 1; k < N; k++) {
//     temp = MkOr(temp, va_arg(ap, NWAOBDDTopNodeRefPtr ));
//   }
//   va_end(ap);
//   return temp;
// }

// // \f1. ... \fk.!(f1 || ... || fk)
// NWAOBDDTopNodeRefPtr  MkNor(int N, ...)
// {
//   NWAOBDDTopNodeRefPtr  temp;

//   assert(N >= 2);
//   va_list ap;
//   va_start(ap, N);
//   temp = va_arg(ap, NWAOBDDTopNodeRefPtr );
//   for (int k = 1; k < N; k++) {
//     temp = MkOr(temp, va_arg(ap, NWAOBDDTopNodeRefPtr ));
//   }
//   va_end(ap);
//   return MkNot(temp);
// }

// Ternary operations on NWAOBDDTopNodes ------------------------------------

// \a.\b.\c.(a && b) || (!a && c)
NWAOBDDTopNodeRefPtr MkIfThenElse(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g, NWAOBDDTopNodeRefPtr h)
{
  return MkOr(MkAnd(f, g), MkAnd( MkNot(f), h));
}

// \a.\b.\c.(b && !a) || (c && !a) || (b && c)
NWAOBDDTopNodeRefPtr MkNegMajority(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g, NWAOBDDTopNodeRefPtr h)
{
  return MkOr( MkAnd(g, MkNot(h)), MkOr(MkAnd(h, MkNot(f)), MkAnd(g,h) ) );
}

// Create representation of \f . exists x_i : f
NWAOBDDTopNodeRefPtr MkExists(NWAOBDDTopNodeRefPtr f, unsigned int i)
{
  NWAOBDDTopNodeRefPtr tempTrue = MkRestrict(f, i, true);
  NWAOBDDTopNodeRefPtr tempFalse = MkRestrict(f, i, false);
  return MkOr(tempTrue, tempFalse);
}

// Create representation of \f . forall x_i : f
NWAOBDDTopNodeRefPtr MkForall(NWAOBDDTopNodeRefPtr f, unsigned int i)
{
  NWAOBDDTopNodeRefPtr tempTrue = MkRestrict(f, i, true);
  NWAOBDDTopNodeRefPtr tempFalse = MkRestrict(f, i, false);
  return MkAnd(tempTrue, tempFalse);
}

//ETTODO count++ count--?
NWAOBDDTopNodeRefPtr MkComposeTop(NWAOBDDTopNodeRefPtr f, int i, NWAOBDDTopNodeRefPtr g)              // \f. f | x_i = g
{
	  // DLC inefficient

  // Simple but slow method, see Bryant 1986 _GBAfBFM_:
  
  // f|x_i=g is g*(f|x_i=1) + (!g)*(f|x_i=0)

  NWAOBDDTopNodeRefPtr f__x_i_1 = MkRestrict(f, i, true);
  NWAOBDDTopNodeRefPtr f__x_i_0 = MkRestrict(f, i, false);
  NWAOBDDTopNodeRefPtr not_g = MkNot(g);


  NWAOBDDTopNodeRefPtr left_product = MkAnd(g, f__x_i_1);


  NWAOBDDTopNodeRefPtr right_product = MkAnd(not_g, f__x_i_0);

  NWAOBDDTopNodeRefPtr composition = MkOr(left_product, right_product);
 
  return composition;
}

// Restrict -----------------------------------------------------------

NWAOBDDNodeHandle Restrict(NWAOBDDNodeHandle g, unsigned int i, bool val,
                           ReturnMapHandle<int> &MapHandle
                          )
{
    NWAOBDDNodeHandle answer;
  
    if (g.handleContents->NodeKind() == NWAOBDD_INTERNAL) {
      answer = Restrict((NWAOBDDInternalNode *)g.handleContents, i, val,
                        MapHandle
                       );
    }
    else { /* g.handleContents->NodeKind() == NWAOBDD_EPSILON */
      MapHandle.AddToEnd(0);
      MapHandle.Canonicalize();
      answer = g;
    }
    return answer;
}

NWAOBDDNodeHandle Restrict(NWAOBDDInternalNode *g, unsigned int i, bool val,
                           ReturnMapHandle<int> &MapHandle
                          )
{

  if (g == NWAOBDDNodeHandle::NoDistinctionNode[g->level].handleContents) {
    MapHandle.AddToEnd(0);
    MapHandle.Canonicalize();
    return NWAOBDDNodeHandle(g);
  }

  unsigned int size = ((unsigned int)((((unsigned int)1) << (g->level + 2)) - (unsigned int)4));
  ReturnMapHandle<intpair> AMap;
  NWAOBDDInternalNode *n = new NWAOBDDInternalNode(g->level);
  unsigned int curExit = 0;

  if (i == 0)  { //i is outbound a variable
	  n->AConnection[0].entryPointHandle = g->AConnection[val].entryPointHandle;
	  n->AConnection[1].entryPointHandle = g->AConnection[val].entryPointHandle;
	  
	  int max = 0;
	  //iterator through the entries in the 0 A Connection's Return Map
	  AMap = g->AConnection[val].returnMapHandle;
	  ReturnMapHandle<int> BList;
	  ReturnMapHandle<intpair> ARet;
	  
	  unsigned aMapSize = AMap.mapContents->mapArray.size();
	  for (unsigned sAIT = 0; sAIT < aMapSize; sAIT++)
	  {
		  intpair b = AMap.mapContents->mapArray[sAIT];
		  int b0 = b.First();
		  int b1 = b.Second();
		  int index0, index1;
		  if (BList.Member(b0)) {
			  index0 = BList.LookupInv(b0);
		  }
		  else {
			  BList.AddToEnd(b0);
			  index0 = max;
			  max++;
		  }
		  if (BList.Member(b1)) {
			  index1 = BList.LookupInv(b0);
		  }
		  else {
			  BList.AddToEnd(b1);
			  index1 = max;
			  max++;
		  }
		  ARet.AddToEnd(intpair(index0, index1));
	  }

	  n->BConnection[0] = new Connection[max]; //May be reduced later
	  n->BConnection[1] = new Connection[max]; //May be reduced later
	  
	  ARet.Canonicalize();
      n->AConnection[0].returnMapHandle = ARet;
	  n->AConnection[1].returnMapHandle = ARet;
	  n->numBConnections = max;
	  for (unsigned sAI = 0; sAI < max; sAI++)
	  {
		  int midB = BList.Lookup(sAI);
		  n->BConnection[0][sAI].entryPointHandle = g->BConnection[0][midB].entryPointHandle;
		  n->BConnection[1][sAI].entryPointHandle = g->BConnection[1][midB].entryPointHandle;
		  for (unsigned int l = 0; l < 2; l++)
		  {
			  ReturnMapHandle<intpair> BMap = g->BConnection[l][midB].returnMapHandle;
			  unsigned bMapSize = BMap.mapContents->mapArray.size();
			  for (unsigned sBI = 0; sBI < bMapSize; sBI++)
			  {
				  intpair c = BMap.mapContents->mapArray[sBI];
				  int c0 = c.First();
				  int c1 = c.Second();

				  int endIndex0;
				  int endIndex1;

				  if (MapHandle.Member(c0))
				  {
					  endIndex0 = MapHandle.LookupInv(c0);
				  } else {
					  MapHandle.AddToEnd(c0);
					  endIndex0 = curExit;
						  curExit++;
				  }

				  if (MapHandle.Member(c1))
				  {
					  endIndex1 = MapHandle.LookupInv(c1);
				  } else {
					  MapHandle.AddToEnd(c1);
					  endIndex1 = curExit;
					  curExit++;
				  }

				  n->BConnection[l][sAI].returnMapHandle.AddToEnd(intpair(endIndex0,endIndex1));
			  }
			  n->BConnection[l][sAI].returnMapHandle.Canonicalize();
		  }
	  }

  }
  else if (i == (size / 2 - 1)) { //The AConnections remain the same, but the 0 and 1 return values for the maps become the same
	  ReturnMapHandle<intpair> AMap0 = g->AConnection[0].returnMapHandle;
	  ReturnMapHandle<intpair> AMap1 = g->AConnection[1].returnMapHandle;

	  ReturnMapHandle<int> BList;
	  ReturnMapHandle<intpair> ARet0;
	  ReturnMapHandle<intpair> ARet1;

	  unsigned aMapSize = AMap0.mapContents->mapArray.size();
	  int max = 0;
	  int bret;
	  for (unsigned sAIT = 0; sAIT < aMapSize; sAIT++)
	  {
		  intpair b = AMap0.mapContents->mapArray[sAIT];
		  if (val)
		  {
			  bret = b.Second();
		  }
		  else
		  {
			  bret = b.First();
		  }
		  int index;
		  if (BList.Member(bret)) {
			  index = BList.LookupInv(bret);
		  }
		  else {
			  BList.AddToEnd(bret);
			  index = max;
			  max++;
		  }
		  ARet0.AddToEnd(intpair(index, index));
	  }

	  aMapSize = AMap1.mapContents->mapArray.size();
	  for (unsigned sAIT2 = 0; sAIT2 < aMapSize; sAIT2++)
	  {
		  intpair b = AMap1.mapContents->mapArray[sAIT2];
		  if (val)
		  {
			  bret = b.Second();
		  }
		  else
		  {
			  bret = b.First();
		  }
		  int index;
		  if (BList.Member(bret)) {
			  index = BList.LookupInv(bret);
		  }
		  else {
			  BList.AddToEnd(bret);
			  index = max;
			  max++;
		  }
		  ARet1.AddToEnd(intpair(index, index));
	  }


	  ARet0.Canonicalize();
	  ARet1.Canonicalize();
	  n->AConnection[0].entryPointHandle = g->AConnection[0].entryPointHandle;
	  n->AConnection[1].entryPointHandle = g->AConnection[1].entryPointHandle;
	  n->AConnection[0].returnMapHandle = ARet0;
	  n->AConnection[1].returnMapHandle = ARet1;

	  n->BConnection[0] = new Connection[max];
	  n->BConnection[1] = new Connection[max];
	  n->numBConnections = max;

	  for (unsigned sAI = 0; sAI < max; sAI++)
	  {
		  int midB = BList.Lookup(sAI);
		  n->BConnection[0][sAI].entryPointHandle = g->BConnection[0][midB].entryPointHandle;
		  n->BConnection[1][sAI].entryPointHandle = g->BConnection[1][midB].entryPointHandle;
		  for (unsigned int l = 0; l < 2; l++)
		  {
			  ReturnMapHandle<intpair> BMap = g->BConnection[l][midB].returnMapHandle;
			  unsigned bMapSize = BMap.mapContents->mapArray.size();
			  for (unsigned sBI = 0; sBI < bMapSize; sBI++)
			  {
				  intpair c = BMap.mapContents->mapArray[sBI];
				  int c0 = c.First();
				  int c1 = c.Second();

				  int endIndex0;
				  int endIndex1;

				  if (MapHandle.Member(c0))
				  {
					  endIndex0 = MapHandle.LookupInv(c0);
				  }
				  else {
					  MapHandle.AddToEnd(c0);
					  endIndex0 = curExit;
					  curExit++;
				  }

				  if (MapHandle.Member(c1))
				  {
					  endIndex1 = MapHandle.LookupInv(c1);
				  }
				  else {
					  MapHandle.AddToEnd(c1);
					  endIndex1 = curExit;
					  curExit++;
				  }

				  n->BConnection[l][sAI].returnMapHandle.AddToEnd(intpair(endIndex0, endIndex1));
			  }
			  n->BConnection[l][sAI].returnMapHandle.Canonicalize();
		  }
	  }
  } else if (i == size/2) {  //The outbound bvariable is restricticted to val
	  n->AConnection[0] = g->AConnection[0];
	  n->AConnection[1] = g->AConnection[1];

	  n->numBConnections = 0;

	  n->BConnection[0] = new Connection[g->numBConnections];
	  n->BConnection[1] = new Connection[g->numBConnections];
	  ReductionMapHandle AReductionMapHandle;

	  for (unsigned int j = 0; j < g->numBConnections; j++)
	  {
		  NWAOBDDNodeHandle *m = g->BConnection[val][j].entryPointHandle;
		  ReturnMapHandle<intpair> inducedReturnMapHandleB;
		  ReturnMapHandle<intpair> BMap = g->BConnection[val][j].returnMapHandle;
		  unsigned bMapSize = BMap.mapContents->mapArray.size();
		  for (unsigned sBI = 0; sBI < bMapSize; sBI++)
		  {
			  intpair c = BMap.mapContents->mapArray[sBI];
			  int c0 = c.First();
			  int c1 = c.Second();
			  int index0,index1;
			  if (MapHandle.Member(c0)){
				  index0 = MapHandle.LookupInv(c0);
			  } else {
				  MapHandle.AddToEnd(c0);
				  index0 = curExit;
				  curExit++;
			  }

			  if (MapHandle.Member(c1)){
				  index1 = MapHandle.LookupInv(c1);
			  } else {
				  MapHandle.AddToEnd(c1);
				  index1 = curExit;
				  curExit++;
			  }
			  inducedReturnMapHandleB.AddToEnd(intpair(index0, index1));
		  }
		  inducedReturnMapHandleB.Canonicalize();
		  Connection candidate0(*m, inducedReturnMapHandleB);
		  Connection candidate1(*m, inducedReturnMapHandleB);
		  unsigned int position = n->InsertBConnection(n->numBConnections, candidate0, candidate1);
		  AReductionMapHandle.AddToEnd(position);
	  }
	  AReductionMapHandle.Canonicalize();
	  if (n->numBConnections < g->numBConnections) {  // Shorten
		  Connection *temp0 = n->BConnection[0];
		  Connection *temp1 = n->BConnection[1];
		  n->BConnection[0] = new Connection[n->numBConnections];
		  n->BConnection[1] = new Connection[n->numBConnections];
		  for (unsigned int k = 0; k < n->numBConnections; k++) {
			  n->BConnection[0][k] = temp0[k];
			  n->BConnection[1][k] = temp1[k];
		  }
		  delete[] temp0;
		  delete[] temp1;
	  }
	  ReductionMapHandle inducedA0ReductionMapHandle;
	  ReductionMapHandle inducedA1ReductionMapHandle;
	  ReturnMapHandle<intpair> inducedA0ReturnMap;
	  ReturnMapHandle<intpair> inducedA1ReturnMap;
	  ReturnMapHandle<intpair> reducedAReturnMap = n->AConnection[0].returnMapHandle.Compose(AReductionMapHandle);
	  reducedAReturnMap.InducedReductionAndReturnMap(inducedA0ReductionMapHandle, inducedA0ReturnMap);
	  NWAOBDDNodeHandle tempHandle = n->AConnection[0].entryPointHandle->Reduce(inducedA0ReductionMapHandle, inducedA0ReturnMap.Size());
	  n->AConnection[0] = Connection(tempHandle, inducedA0ReturnMap);

	  reducedAReturnMap = n->AConnection[1].returnMapHandle.Compose(AReductionMapHandle);
	  reducedAReturnMap.InducedReductionAndReturnMap(inducedA1ReductionMapHandle, inducedA1ReturnMap);
	  NWAOBDDNodeHandle tempHandle1 = n->AConnection[1].entryPointHandle->Reduce(inducedA1ReductionMapHandle, inducedA1ReturnMap.Size());
	  n->AConnection[1] = Connection(tempHandle1, inducedA1ReturnMap);
  } else if (i == size - 1) { //The B-Return is restricted to Val
	  n->AConnection[0] = g->AConnection[0];
	  n->AConnection[1] = g->AConnection[1];

	  n->numBConnections = 0;

	  n->BConnection[0] = new Connection[g->numBConnections];
	  n->BConnection[1] = new Connection[g->numBConnections];

	  ReductionMapHandle AReductionMapHandle;

	  for (unsigned int l = 0; l < g->numBConnections; l++)
	  {
			  NWAOBDDNodeHandle *m0 = g->BConnection[0][l].entryPointHandle;
			  NWAOBDDNodeHandle *m1 = g->BConnection[1][l].entryPointHandle;
			  ReturnMapHandle<intpair> BMap0 = g->BConnection[0][l].returnMapHandle;
			  ReturnMapHandle<intpair> BMap1 = g->BConnection[1][l].returnMapHandle; 
			  ReturnMapHandle<intpair> inducedReturnMapHandleB0, inducedReturnMapHandleB1;
			  unsigned bMapSize0 = BMap0.mapContents->mapArray.size();
			  unsigned bMapSize1 = BMap1.mapContents->mapArray.size();
			  for (unsigned sBI = 0; sBI < bMapSize0; sBI++)
			  {
				  int curValue;
				  if (val == 0)
				  {
					  curValue = BMap0.mapContents->mapArray[sBI].First();
				  } else {
					  curValue = BMap0.mapContents->mapArray[sBI].Second();
				  }
				  if (MapHandle.Member(curValue))  {
					  int index = MapHandle.LookupInv(curValue);
					  inducedReturnMapHandleB0.AddToEnd(intpair(index, index));
				  } else {
					  MapHandle.AddToEnd(curValue);
					  inducedReturnMapHandleB0.AddToEnd(intpair(curExit, curExit));
					  curExit++;
				  }
			  }
			  for (unsigned sBIN = 0; sBIN < bMapSize1; sBIN++)
			  {
				  int curValue;
				  if (val == 0)
				  {
					  curValue = BMap1.mapContents->mapArray[sBIN].First();
				  }
				  else {
					  curValue = BMap1.mapContents->mapArray[sBIN].Second();
				  }
				  if (MapHandle.Member(curValue))  {
					  int index = MapHandle.LookupInv(curValue);
					  inducedReturnMapHandleB1.AddToEnd(intpair(index, index));
				  }
				  else {
					  MapHandle.AddToEnd(curValue);
					  inducedReturnMapHandleB1.AddToEnd(intpair(curExit, curExit));
					  curExit++;
				  }
			  }
			  inducedReturnMapHandleB0.Canonicalize();
			  inducedReturnMapHandleB1.Canonicalize();
			  Connection candidate0(*m0, inducedReturnMapHandleB0);
			  Connection candidate1(*m1, inducedReturnMapHandleB1);
			  unsigned int position = n->InsertBConnection(n->numBConnections, candidate0, candidate1);
			  AReductionMapHandle.AddToEnd(position);
	  }

	  AReductionMapHandle.Canonicalize();
	  if (n->numBConnections < g->numBConnections) {  // Shorten
		  Connection *temp0 = n->BConnection[0];
		  Connection *temp1 = n->BConnection[1];
		  n->BConnection[0] = new Connection[n->numBConnections];
		  n->BConnection[1] = new Connection[n->numBConnections];
		  for (unsigned int j = 0; j < n->numBConnections; j++) {
			  n->BConnection[0][j] = temp0[j];
			  n->BConnection[1][j] = temp1[j];
		  }
		  delete[] temp0;
		  delete[] temp1;
	  }

	  ReductionMapHandle inducedA0ReductionMapHandle;
	  ReductionMapHandle inducedA1ReductionMapHandle;
	  ReturnMapHandle<intpair> inducedA0ReturnMap;
	  ReturnMapHandle<intpair> inducedA1ReturnMap;
	  ReturnMapHandle<intpair> reducedAReturnMap = n->AConnection[0].returnMapHandle.Compose(AReductionMapHandle);
	  reducedAReturnMap.InducedReductionAndReturnMap(inducedA0ReductionMapHandle, inducedA0ReturnMap);
	  NWAOBDDNodeHandle tempHandle = n->AConnection[0].entryPointHandle->Reduce(inducedA0ReductionMapHandle, inducedA0ReturnMap.Size());
	  n->AConnection[0] = Connection(tempHandle, inducedA0ReturnMap);

	  reducedAReturnMap = n->AConnection[1].returnMapHandle.Compose(AReductionMapHandle);
	  reducedAReturnMap.InducedReductionAndReturnMap(inducedA1ReductionMapHandle, inducedA1ReturnMap);
	  NWAOBDDNodeHandle tempHandle1 = n->AConnection[1].entryPointHandle->Reduce(inducedA1ReductionMapHandle, inducedA1ReturnMap.Size());
	  n->AConnection[1] = Connection(tempHandle1, inducedA1ReturnMap);
  } else if (i > (size/2)) { //The var is located in the B-Connection
	  n->AConnection[0] = g->AConnection[0];
	  n->AConnection[1] = g->AConnection[1];
	  n->numBConnections = 0;
	  n->BConnection[0] = new Connection[g->numBConnections];
	  n->BConnection[1] = new Connection[g->numBConnections];
	  ReductionMapHandle AReductionMapHandle;
	  for (unsigned int l = 0; l < g->numBConnections; l++)
	  {
		  ReturnMapHandle<int> BRedMap0;
		  ReturnMapHandle<int> BRedMap1;
		  ReturnMapHandle<intpair> InducedReturnMapB0, InducedReturnMapB1;
		  NWAOBDDNodeHandle m0 = Restrict(*(g->BConnection[0][l].entryPointHandle), i - (size / 2) - 1, val, BRedMap0);
		  NWAOBDDNodeHandle m1 = Restrict(*(g->BConnection[1][l].entryPointHandle), i - (size / 2) - 1, val, BRedMap1);
		  for (unsigned int j = 0; j < BRedMap0.Size(); j++)
		  {
			  int retPoint = BRedMap0.Lookup(j);
			  int c0 = g->BConnection[0][l].returnMapHandle.Lookup(retPoint).First();
			  int c1 = g->BConnection[0][l].returnMapHandle.Lookup(retPoint).Second();
			  int index0, index1;
			  if (MapHandle.Member(c0)) {
				  index0 = MapHandle.LookupInv(c0);
			  }
			  else {
				  index0 = curExit;
				  MapHandle.AddToEnd(c0);
				  curExit++;
			  }
			  if (MapHandle.Member(c1)) {
				  index1 = MapHandle.LookupInv(c1);
			  }
			  else {
				  index1 = curExit;
				  MapHandle.AddToEnd(c1);
				  curExit++;
			  }

			  InducedReturnMapB0.AddToEnd(intpair(index0, index1));
		  }

		  for (unsigned int j = 0; j < BRedMap1.Size(); j++)
		  {
			  int retPoint = BRedMap1.Lookup(j);
			  int c0 = g->BConnection[1][l].returnMapHandle.Lookup(retPoint).First();
			  int c1 = g->BConnection[1][l].returnMapHandle.Lookup(retPoint).Second();
			  int index0, index1;
			  if (MapHandle.Member(c0)) {
				  index0 = MapHandle.LookupInv(c0);
			  }
			  else {
				  index0 = curExit;
				  MapHandle.AddToEnd(c0);
				  curExit++;
			  }
			  if (MapHandle.Member(c1)) {
				  index1 = MapHandle.LookupInv(c1);
			  }
			  else {
				  index1 = curExit;
				  MapHandle.AddToEnd(c1);
				  curExit++;
			  }

			  InducedReturnMapB1.AddToEnd(intpair(index0, index1));
		  }

		  InducedReturnMapB0.Canonicalize();
		  InducedReturnMapB1.Canonicalize();
		  Connection candidate0(m0, InducedReturnMapB0);
		  Connection candidate1(m1, InducedReturnMapB1);
		  unsigned int position = n->InsertBConnection(n->numBConnections, candidate0, candidate1);
		  AReductionMapHandle.AddToEnd(position);

	  }
			  AReductionMapHandle.Canonicalize();
			  if (n->numBConnections < g->numBConnections) {  // Shorten
				  Connection *temp0 = n->BConnection[0];
				  Connection *temp1 = n->BConnection[1];
				  n->BConnection[0] = new Connection[n->numBConnections];
				  n->BConnection[1] = new Connection[n->numBConnections];
				  for (unsigned int j = 0; j < n->numBConnections; j++) {
					  n->BConnection[0][j] = temp0[j];
					  n->BConnection[1][j] = temp1[j];
				  }
				  delete[] temp0;
				  delete[] temp1;
			  }

			  ReductionMapHandle inducedA0ReductionMapHandle;
			  ReductionMapHandle inducedA1ReductionMapHandle;
			  ReturnMapHandle<intpair> inducedA0ReturnMap;
			  ReturnMapHandle<intpair> inducedA1ReturnMap;
			  ReturnMapHandle<intpair> reducedAReturnMap = n->AConnection[0].returnMapHandle.Compose(AReductionMapHandle);
			  reducedAReturnMap.InducedReductionAndReturnMap(inducedA0ReductionMapHandle, inducedA0ReturnMap);
			  NWAOBDDNodeHandle tempHandle = n->AConnection[0].entryPointHandle->Reduce(inducedA0ReductionMapHandle, inducedA0ReturnMap.Size());
			  n->AConnection[0] = Connection(tempHandle, inducedA0ReturnMap);

			  reducedAReturnMap = n->AConnection[1].returnMapHandle.Compose(AReductionMapHandle);
			  reducedAReturnMap.InducedReductionAndReturnMap(inducedA1ReductionMapHandle, inducedA1ReturnMap);
			  NWAOBDDNodeHandle tempHandle1 = n->AConnection[1].entryPointHandle->Reduce(inducedA1ReductionMapHandle, inducedA1ReturnMap.Size());
			  n->AConnection[1] = Connection(tempHandle1, inducedA1ReturnMap);  
  } else { //The var is located in the A-Connections

	  ReturnMapHandle<int> BList;
	  int numB = 0;
	  curExit = 0;
	  ReturnMapHandle<int> AMap0,AMap1;
      NWAOBDDNodeHandle A0_entry = Restrict(*(g->AConnection[0].entryPointHandle), i - 1, val, AMap0);
		  // n->AConnection[0].entryPointHandle = 
      ReturnMapHandle<intpair>A0_ret;
		  for (unsigned int j = 0; j < AMap0.Size(); j++)
		  {
			  int retPoint = AMap0.Lookup(j);
			  int b0 = g->AConnection[0].returnMapHandle.Lookup(retPoint).First();
			  int b1 = g->AConnection[0].returnMapHandle.Lookup(retPoint).Second();
			  int index0, index1;
			  if (BList.Member(b0))
			  {
				  index0 = BList.LookupInv(b0);
			  }
			  else {
				  index0 = numB;
				  BList.AddToEnd(b0);
				  numB++;
			  } if (BList.Member(b1)){
				  index1 = BList.LookupInv(b1);
			  }
			  else {
				  index1 = numB;
				  BList.AddToEnd(b1);
				  numB++;
			  }
			  A0_ret.AddToEnd(intpair(index0, index1));
		  }
      n->AConnection[0] = Connection(A0_entry, A0_ret);
		  // n->AConnection[1].entryPointHandle = Restrict(g->AConnection[1].entryPointHandle, i - 1, val, AMap1);
		  NWAOBDDNodeHandle A1_entry = Restrict(*(g->AConnection[1].entryPointHandle), i - 1, val, AMap1);
      ReturnMapHandle<intpair> A1_ret;
      for (unsigned int j = 0; j < AMap1.Size(); j++)
		  {
			  int retPoint = AMap1.Lookup(j);
			  int b0 = g->AConnection[1].returnMapHandle.Lookup(retPoint).First();
			  int b1 = g->AConnection[1].returnMapHandle.Lookup(retPoint).Second();
			  int index0, index1;
			  if (BList.Member(b0))
			  {
				  index0 = BList.LookupInv(b0);
			  }
			  else {
				  index0 = numB;
				  BList.AddToEnd(b0);
				  numB++;
			  } if (BList.Member(b1)){
				  index1 = BList.LookupInv(b1);
			  }
			  else {
				  index1 = numB;
				  BList.AddToEnd(b1);
				  numB++;
			  }
			  n->AConnection[1].returnMapHandle.AddToEnd(intpair(index0, index1));
		  }
      n -> AConnection[1] = Connection(A1_entry, A1_ret);
		n->AConnection[0].returnMapHandle.Canonicalize();
		n->AConnection[1].returnMapHandle.Canonicalize();

		n->numBConnections = numB;
		n->BConnection[0] = new Connection[n->numBConnections];
		n->BConnection[1] = new Connection[n->numBConnections];
		for (unsigned fin = 0; fin < numB; fin++)
		{
		  int midB = BList.Lookup(fin);
		  n->BConnection[0][fin].entryPointHandle = g->BConnection[0][midB].entryPointHandle;
		  n->BConnection[1][fin].entryPointHandle = g->BConnection[1][midB].entryPointHandle;
		  for (unsigned int l = 0; l < 2; l++)
		  {
			  ReturnMapHandle<intpair> BMap = g->BConnection[l][midB].returnMapHandle;
			  unsigned bMapSize = BMap.mapContents->mapArray.size();
			  for (unsigned sBI = 0; sBI < bMapSize; sBI++)
			  {
				  intpair c = BMap.mapContents->mapArray[sBI];
				  int c0 = c.First();
				  int c1 = c.Second();

				  int endIndex0;
				  int endIndex1;

				  if (MapHandle.Member(c0))
				  {
					  endIndex0 = MapHandle.LookupInv(c0);
				  }
				  else {
					  MapHandle.AddToEnd(c0);
					  endIndex0 = curExit;
					  curExit++;
				  }

				  if (MapHandle.Member(c1))
				  {
					  endIndex1 = MapHandle.LookupInv(c1);
				  }
				  else {
					  MapHandle.AddToEnd(c1);
					  endIndex1 = curExit;
					  curExit++;
				  }

				  n->BConnection[l][fin].returnMapHandle.AddToEnd(intpair(endIndex0, endIndex1));
			  }
			  n->BConnection[l][fin].returnMapHandle.Canonicalize();
		  }
		}
  } 
  n->numExits = curExit;
  return NWAOBDDNodeHandle(n);
}
}
namespace NWA_OBDD {

NWAOBDDTopNodeRefPtr MkSchemaAdjust(NWAOBDDTopNodeRefPtr n, int s[])
{
	NWAOBDDTopNodeRefPtr g = NWAOBDDNodeHandle::SchemaAdjust(n->rootConnection.entryPointHandle, 1, s, 0);
	return g;
}

NWAOBDDTopNodeRefPtr MkPathSummary(NWAOBDDTopNodeRefPtr n)
{
	NWAOBDDTopNodeRefPtr g = NWAOBDDNodeHandle::PathSummary(n->rootConnection.entryPointHandle, 1, 0);
	return g;
}

template<typename T>
ref_ptr<NWAOBDDTopNode<T>> MkRestrict(ref_ptr<NWAOBDDTopNode<T>> n, unsigned int i, bool val)
{
  ReturnMapHandle<int> MapHandle;
  NWAOBDDNodeHandle g = Restrict(*(n->rootConnection.entryPointHandle), i, val,
                                 MapHandle);
  g.Canonicalize();

  // Create returnMapHandle from MapHandle
     ReturnMapHandle<int> returnMapHandle;
	 unsigned mapSize = MapHandle.mapContents->mapArray.size();
     for (unsigned i = 0; i <mapSize; i++) {
		 int d = MapHandle.mapContents->mapArray[i];
       int c = n->rootConnection.returnMapHandle.Lookup(d);
       returnMapHandle.AddToEnd(c);
     }
     returnMapHandle.Canonicalize();

  // Create and return NWAOBDDTopNode

     return(new NWAOBDDTopNode<int>(g, returnMapHandle));
}


template <typename T>
typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr
// ApplyAndReduce -----------------------------------------------------
ApplyAndReduce(typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr n1,
            typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr n2,
            BoolOp op
            )
{
  // Perform 2-way cross product of n1 and n2
     PairProductMapHandle MapHandle;
     NWAOBDDNodeHandle n = PairProduct(*(n1->rootConnection.entryPointHandle),
                                       *(n2->rootConnection.entryPointHandle),
                                       MapHandle);
  // Create returnMapHandle from MapHandle: Fold the pairs in MapHandle by applying
  // [n1->rootConnection.returnMapHandle, n2->rootConnection.returnMapHandle]
  // (component-wise) to each pair.
     ReturnMapHandle<int> returnMapHandle;
    boost::unordered_map<int, unsigned int> reductionMap;
    ReductionMapHandle reductionMapHandle;
    unsigned int iterator = 0;
    //while (!MapIterator.AtEnd()) {
    while (iterator < MapHandle.Size()){
        int c1, c2;
        int first, second;
        //first = MapIterator.Current().First();
        //second = MapIterator.Current().Second();
        first = MapHandle[iterator].First();
        second = MapHandle[iterator].Second();
        c1 = n1->rootConnection.returnMapHandle.Lookup(first);
        c2 = n2->rootConnection.returnMapHandle.Lookup(second);
        int val = op[c1][c2];
        unsigned int k = returnMapHandle.Size();
        for ( k = 0; k < returnMapHandle.Size(); k++)
        {
            if (returnMapHandle[k] == val)
            {
                break;
            }
        }
        if (k < returnMapHandle.Size())
        {
            reductionMapHandle.AddToEnd(k);
        }
        else
        {
            returnMapHandle.AddToEnd(val);
            reductionMapHandle.AddToEnd(returnMapHandle.Size() - 1);
        }
        iterator++;
    }
    returnMapHandle.Canonicalize();
    reductionMapHandle.Canonicalize();

    NWAOBDDNodeHandle reduced_n = n.Reduce(reductionMapHandle, returnMapHandle.Size());
    return(new NWAOBDDTopNode<int>(reduced_n, returnMapHandle));
}



template <typename T>
typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr
ApplyAndReduce(typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr n1,
            typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr n2,
            T(*func)(T, T)
            )
{
  // Perform 2-way cross product of n1 and n2
     PairProductMapHandle MapHandle;
     NWAOBDDNodeHandle n = PairProduct(*(n1->rootConnection.entryPointHandle),
                                       *(n2->rootConnection.entryPointHandle),
                                       MapHandle);
  // Create returnMapHandle from MapHandle: Fold the pairs in MapHandle by applying
  // [n1->rootConnection.returnMapHandle, n2->rootConnection.returnMapHandle]
  // (component-wise) to each pair.
    ReturnMapHandle<T> returnMapHandle;
    boost::unordered_map<T, unsigned int> reductionMap;
    ReductionMapHandle reductionMapHandle;
    unsigned int iterator = 0;
    //while (!MapIterator.AtEnd()) {
    while (iterator < MapHandle.Size()){
        T c1, c2;
        int first, second;
        //first = MapIterator.Current().First();
        //second = MapIterator.Current().Second();
        first = MapHandle[iterator].First();
        second = MapHandle[iterator].Second();
        c1 = n1->rootConnection.returnMapHandle.Lookup(first);
        c2 = n2->rootConnection.returnMapHandle.Lookup(second);
        T val = (*func)(c1, c2);
        unsigned int k = returnMapHandle.Size();
        for ( k = 0; k < returnMapHandle.Size(); k++)
        {
            if (returnMapHandle[k] == val)
            {
                break;
            }
        }
        if (k < returnMapHandle.Size())
        {
            reductionMapHandle.AddToEnd(k);
        }
        else
        {
            returnMapHandle.AddToEnd(val);
            reductionMapHandle.AddToEnd(returnMapHandle.Size() - 1);
        }
        iterator++;
    }
    returnMapHandle.Canonicalize();
    reductionMapHandle.Canonicalize();

    NWAOBDDNodeHandle reduced_n = n.Reduce(reductionMapHandle, returnMapHandle.Size());
    return(new NWAOBDDTopNode<T>(reduced_n, returnMapHandle));
}


// Pointwise multiplication: \f.\g.(f * g) ---------------------------------------
template <typename T>
typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr
MkTimesTopNode(typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr f,
            typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr g
            )
{
    return ApplyAndReduce<T>(f, g, TimesFunc);
}

// Pointwise multiplication: \f.\g.(f + g) ---------------------------------------
template <typename T>
typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr
MkPlusTopNode(typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr f,
            typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr g
            )
{
    return ApplyAndReduce<T>(f, g, PlusFunc);
}


}
