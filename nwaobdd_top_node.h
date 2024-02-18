#pragma once

#include "nwaobdd_node.h"
#include "cstdarg"

//********************************************************************
// NWAOBDDBaseNode
//********************************************************************

namespace NWA_OBDD {
typedef ref_ptr<NWAOBDDTopNode> NWAOBDDTopNodeRefPtr;


//********************************************************************
// NWAOBDDTopNode
//********************************************************************


class NWAOBDDTopNode
{
  friend NWAOBDDTopNodeRefPtr MkRestrict(NWAOBDDTopNodeRefPtr f, unsigned int i, bool val);  // \f. f | (x_i = val)
  friend NWAOBDDTopNodeRefPtr ApplyAndReduce(NWAOBDDTopNodeRefPtr n1, NWAOBDDTopNodeRefPtr n2, BoolOp op);
  friend NWAOBDDTopNodeRefPtr ApplyAndReduce(NWAOBDDTopNodeRefPtr n1, NWAOBDDTopNodeRefPtr n2, NWAOBDDTopNodeRefPtr n3, BoolOp3 op);
 public:
  NWAOBDDTopNode(NWAOBDDNode *n, ReturnMapHandle<intpair> (&mapHandle));                // Constructor
  NWAOBDDTopNode(NWAOBDDNodeHandle &nodeHandle, ReturnMapHandle<intpair> (&mapHandle)); // Constructor
  ~NWAOBDDTopNode();                                   // Destructor
  void DeallocateMemory();
  bool Evaluate(SH_OBDD::Assignment &assignment);             // Evaluate a Boolean function (recursive)
  bool EvaluateIteratively(SH_OBDD::Assignment &assignment);  // Evaluate a Boolean function (iterative)
  void PrintYield(std::ostream * out);
  static unsigned const int maxLevel;
  unsigned int level;
#ifdef PATH_COUNTING_ENABLED
  unsigned int NumSatisfyingAssignments();
#endif
  bool FindOneSatisfyingAssignment(SH_OBDD::Assignment * &assignment);
  unsigned int Hash(unsigned int modsize);
  void DumpConnections(Hashset<NWAOBDDNode> *visited, std::ostream & out = std::cout);
  void CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *visitedEdges, unsigned int &nodeCount, unsigned int &edgeCount);
  bool operator!= (const NWAOBDDTopNode & C);          // Overloaded !=
  bool operator== (const NWAOBDDTopNode & C);          // Overloaded ==
  Connection rootConnection;                           // A single Connection; TWR: Only need one between BaseNode and TopNode
  RefCounter count;                                    // TWR: Dangerous because NWAOBDDBaseNode has a RefCounter, too

 private:
  static Hashset<NWAOBDDTopNode> *computedCache;       // TEMPORARY: should be HashCache
  NWAOBDDTopNode();                                    // Default constructor (hidden)
  NWAOBDDTopNode(const NWAOBDDTopNode &n);             // Copy constructor (hidden)
  NWAOBDDTopNode& operator= (const NWAOBDDTopNode &n); // Overloaded = (hidden)
 public:
  void PrintYieldAux(std::ostream * out, List<ConsCell<TraverseState> *> &T, ConsCell<TraverseState> *S);
  std::ostream& print(std::ostream & out = std::cout) const;
};

// NWAOBDDTopNode-creation operations --------------------------------------
NWAOBDDTopNodeRefPtr MkTrueTop();                    // Representation of \x.true
NWAOBDDTopNodeRefPtr MkFalseTop();                   // Representation of \x.false
NWAOBDDTopNodeRefPtr MkDistinction(unsigned int i);  // Representation of \x.x_i
NWAOBDDTopNodeRefPtr MkIdRelationInterleavedTop();   // Representation of identity relation
NWAOBDDTopNodeRefPtr MkIdRelationNestedTop();
NWAOBDDTopNodeRefPtr MkAdditionNestedTop();     // Representation of addition relation
NWAOBDDTopNodeRefPtr MkDetensorConstraintInterleavedTop();   // Representation of (W,X,Y,Z) s.t. X==Y with interleaved variables
NWAOBDDTopNodeRefPtr MkParityTop();                  // Representation of parity function
NWAOBDDTopNodeRefPtr MkWalshInterleavedTop(unsigned int i); // Representation of Walsh matrix
NWAOBDDTopNodeRefPtr MkInverseReedMullerInterleavedTop(unsigned int i); // Representation of Inverse Reed-Muller matrix
NWAOBDDTopNodeRefPtr MkStepUpOneFourthTop();         // Representation of step function
NWAOBDDTopNodeRefPtr MkStepDownOneFourthTop();       // Representation of step function
#ifdef ARBITRARY_STEP_FUNCTIONS
  NWAOBDDTopNodeRefPtr MkStepUpTop(unsigned int i);    // Representation of step function
  NWAOBDDTopNodeRefPtr MkStepDownTop(unsigned int i);  // Representation of step function
#endif

NWAOBDDTopNodeRefPtr GetWeight(int s[], int vals[], int vLocs[], int c);

// Unary operations on NWAOBDDTopNodes --------------------------------------
NWAOBDDTopNodeRefPtr MkNot(NWAOBDDTopNodeRefPtr f);               // \f.!f

// Binary operations on NWAOBDDTopNodes --------------------------------------
NWAOBDDTopNodeRefPtr MkAnd(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);          // \f.\g.(f && g)
NWAOBDDTopNodeRefPtr MkNand(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);         // \f.\g.!(f && g)
NWAOBDDTopNodeRefPtr MkOr(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);           // \f.\g.(f || g)
NWAOBDDTopNodeRefPtr MkNor(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);          // \f.\g.!(f || g)
NWAOBDDTopNodeRefPtr MkIff(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);          // \f.\g.(f == g)
NWAOBDDTopNodeRefPtr MkExclusiveOr(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);  // \f.\g.(f != g)
NWAOBDDTopNodeRefPtr MkImplies(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);      // \f.\g.(!f || g)
NWAOBDDTopNodeRefPtr MkMinus(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);        // \f.\g.(f && !g)
NWAOBDDTopNodeRefPtr MkQuotient(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);     // \f.\g.(!g || f)
NWAOBDDTopNodeRefPtr MkNotQuotient(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);  // \f.\g.(g && !f)
NWAOBDDTopNodeRefPtr MkFirst(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);        // \f.\g.f
NWAOBDDTopNodeRefPtr MkNotFirst(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);     // \f.\g.!f
NWAOBDDTopNodeRefPtr MkSecond(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);       // \f.\g.g
NWAOBDDTopNodeRefPtr MkNotSecond(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g);    // \f.\g.!g

// N-ary operations on NWAOBDDTopNodes --------------------------------------
NWAOBDDTopNodeRefPtr MkAnd(int N, ...);      // \f1. ... \fk.(f1 && ... && fk)
NWAOBDDTopNodeRefPtr MkNand(int N, ...);     // \f1. ... \fk.!(f1 && ... && fk)
NWAOBDDTopNodeRefPtr MkOr(int N, ...);       // \f1. ... \fk.(f1 || ... || fk)
NWAOBDDTopNodeRefPtr MkNor(int N, ...);      // \f1. ... \fk.!(f1 || ... || fk)

// Ternary operations on NWAOBDDTopNodes ------------------------------------

// \a.\b.\c.(a && b) || (!a && c)
NWAOBDDTopNodeRefPtr MkIfThenElse(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g, NWAOBDDTopNodeRefPtr h);

// \a.\b.\c.(b && !a) || (c && !a) || (b && c)
NWAOBDDTopNodeRefPtr MkNegMajority(NWAOBDDTopNodeRefPtr f, NWAOBDDTopNodeRefPtr g, NWAOBDDTopNodeRefPtr h);

NWAOBDDTopNodeRefPtr MkExists(NWAOBDDTopNodeRefPtr f, unsigned int i);              // \f. exists x_i : f
NWAOBDDTopNodeRefPtr MkForall(NWAOBDDTopNodeRefPtr f, unsigned int i);              // \f. forall x_i : f
NWAOBDDTopNodeRefPtr MkSchemaAdjust(NWAOBDDTopNodeRefPtr g, int s[4]);
NWAOBDDTopNodeRefPtr MkPathSummary(NWAOBDDTopNodeRefPtr g);
NWAOBDDTopNodeRefPtr MkComposeTop(NWAOBDDTopNodeRefPtr f, int i, NWAOBDDTopNodeRefPtr g);              // \f. f | x_i = g

} // namespace NWA_OBDD