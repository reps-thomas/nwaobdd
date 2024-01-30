#ifndef NWAOBDD_NODE_GUARD
#define NWAOBDD_NODE_GUARD

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


// Configuration flags --------------------------------------------
//#define PATH_COUNTING_ENABLED 1

namespace NWA_OBDD {
// Node classes declared in this file --------------------------------
class NWAOBDDTopNode;
class NWAOBDDBaseNode;
class NWAOBDDNode;
class NWAOBDDInternalNode;   //  : public NWAOBDDNode
class NWAOBDDLeafNode;       //  : public NWAOBDDNode
class NWAOBDDEpsilonNode;   //  : public NWAOBDDLeafNode
class NWAOBDDNodeHandle;
 }

#include <iostream>
#include <fstream>
#include "list_T.h"
#include "list_TPtr.h"
#include "intpair.h"
#include "conscell.h"
#include "assignment.h"
#include "bool_op.h"
#include "return_map_T.h"
#include "reduction_map.h"
#include "traverse_state_nwa.h"
#include "hash.h"
#include "hashset.h"
#include "ref_ptr.h"

namespace NWA_OBDD {
typedef ref_ptr<NWAOBDDTopNode> NWAOBDDTopNodeRefPtr;
typedef ref_ptr<NWAOBDDBaseNode> NWAOBDDBaseNodeRefPtr;

//********************************************************************
// NWAOBDDNodeHandle
//********************************************************************

class NWAOBDDNodeHandle {
#define NWAOBDD_NODE_HANDLE_GUARD
 public:
  NWAOBDDNodeHandle();                                        // Default constructor
  NWAOBDDNodeHandle(NWAOBDDNode *n);                          // Constructor
  NWAOBDDNodeHandle(const NWAOBDDNodeHandle &nh);              // Copy constructor
  ~NWAOBDDNodeHandle();                                       // Destructor
  unsigned int Hash(unsigned int modsize);
  bool operator!= (const NWAOBDDNodeHandle &nh);              // Overloaded !=
  bool operator== (const NWAOBDDNodeHandle &nh);              // Overloaded ==
  NWAOBDDNodeHandle & operator= (const NWAOBDDNodeHandle &nh); // assignment

  // Distinguished NWAOBDDNodeHandles -----------------------
     public:
      static NWAOBDDNodeHandle *NoDistinctionNode;     // NWAOBDDNodeHandle NoDistinctionNode[maxLevel+1]
      static NWAOBDDNodeHandle NWAOBDDEpsilonNodeHandle;
      static void InitNoDistinctionTable();

  NWAOBDDNode *handleContents;

 // Table of canonical nodes -------------------------
    public:
     static Hashset<NWAOBDDNode> *canonicalNodeTable;
     void Canonicalize();

 // Reduce and its associated cache ---------------
    public:
     NWAOBDDNodeHandle Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits);
	 static NWAOBDDTopNodeRefPtr SchemaAdjust(NWAOBDDNodeHandle n, int exit, int s[4], int offset);
	 static NWAOBDDTopNodeRefPtr PathSummary(NWAOBDDNodeHandle n, int exit, int offset);
     static void InitReduceCache();
     static void DisposeOfReduceCache();

 public:
	 std::ostream& print(std::ostream & out = std::cout) const;
};

std::ostream& operator<< (std::ostream & out, const NWAOBDDNodeHandle &d);
 }

#include "cross_product_nwa.h"

namespace NWA_OBDD {
// Other classes and types declared in this file ---------------------
typedef List<intpair> RestrictMap;
typedef ListIterator<intpair> RestrictMapIterator;

class Connection;
class NWAReduceKey;

// Auxiliary data -----------------------------------------------

// Note: If NWAOBDDMaxLevel >= 27, allocating an Assignment will cause
//       virtual memory to be exceeded (on velveeta).
static unsigned int NWAOBDDMaxLevel = 3;
void setMaxLevel(unsigned int level);

//********************************************************************
// NWAReduceKey
//********************************************************************

class NWAReduceKey {

 public:
  NWAReduceKey(NWAOBDDNodeHandle nodeHandle, ReductionMapHandle redMap); // Constructor
  unsigned int Hash(unsigned int modsize);
  NWAReduceKey& operator= (const NWAReduceKey& p);  // Overloaded assignment
  bool operator!= (const NWAReduceKey& p);        // Overloaded !=
  bool operator== (const NWAReduceKey& p);        // Overloaded ==
  NWAOBDDNodeHandle NodeHandle() const { return nodeHandle; }      // Access function
  ReductionMapHandle RedMapHandle() const { return redMapHandle; } // Access function
  std::ostream& print(std::ostream & out) const;

 private:
  NWAOBDDNodeHandle nodeHandle;
  ReductionMapHandle redMapHandle;
  NWAReduceKey();                                 // Default constructor (hidden)
};

std::ostream& operator<< (std::ostream & out, const NWAReduceKey &p);

//********************************************************************
// Connection
//********************************************************************

class Connection {
 public:
  Connection();                                  // Default constructor
  Connection(NWAOBDDNode *entryPoint, ReturnMapHandle<intpair> (&returnMapHandle));
  Connection(NWAOBDDNodeHandle &entryPointHandle, ReturnMapHandle<intpair> (&returnMapHandle));
  ~Connection();                                 // Destructor
  unsigned int Hash(unsigned int modsize);
  Connection& operator= (const Connection &C);   // Overloaded =
  bool operator!= (const Connection & C);        // Overloaded !=
  bool operator== (const Connection & C);        // Overloaded ==

  NWAOBDDNodeHandle entryPointHandle;
#define compile_hack = 1
  ReturnMapHandle<intpair> returnMapHandle;
#define compile_hack = 0
 public:
	 std::ostream& print(std::ostream & out = std::cout) const;
};

std::ostream& operator<< (std::ostream & out, const Connection &c);



//********************************************************************
// NWAOBDDBaseNode
//********************************************************************



class NWAOBDDBaseNode {
	friend NWAOBDDBaseNodeRefPtr MkRestrict(NWAOBDDBaseNodeRefPtr f, unsigned int i, bool val);  // \f. f | (x_i = val)
	friend NWAOBDDBaseNodeRefPtr ApplyAndReduce(NWAOBDDBaseNodeRefPtr n1, NWAOBDDBaseNodeRefPtr n2, BoolOp op);
	friend NWAOBDDBaseNodeRefPtr ApplyAndReduce(NWAOBDDBaseNodeRefPtr n1, NWAOBDDBaseNodeRefPtr n2, NWAOBDDBaseNodeRefPtr n3, BoolOp3 op);
public:
	NWAOBDDBaseNode(NWAOBDDNode *n, ReturnMapHandle<intpair>(&mapHandle));                // Constructor
	NWAOBDDBaseNode(NWAOBDDNodeHandle &nodeHandle, ReturnMapHandle<intpair>(&mapHandle)); // Constructor
	~NWAOBDDBaseNode();                                   // Destructor
	void DeallocateMemory();
	unsigned int level;
	void PrintYield(std::ostream * out);
	unsigned int Hash(unsigned int modsize);
	bool operator!= (const NWAOBDDBaseNode & C);          // Overloaded !=
	bool operator== (const NWAOBDDBaseNode & C);          // Overloaded ==
	Connection rootConnection;                           // A single Connection
	RefCounter count;

private:
	NWAOBDDBaseNode();                                    // Default constructor (hidden)
	NWAOBDDBaseNode(const NWAOBDDBaseNode &n);             // Copy constructor (hidden)
	NWAOBDDBaseNode& operator= (const NWAOBDDBaseNode &n); // Overloaded = (hidden)
public:
	void PrintYieldAux(std::ostream * out, List<ConsCell<TraverseState> *> &T, ConsCell<TraverseState> *S);
	std::ostream& print(std::ostream & out = std::cout) const;
};

// Unary operations on NWAOBDDBaseNodes --------------------------------------
NWAOBDDBaseNodeRefPtr MkNot(NWAOBDDBaseNodeRefPtr f);               // \f.!f

// Binary operations on NWAOBDDBaseNodes --------------------------------------
NWAOBDDBaseNodeRefPtr MkAnd(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);          // \f.\g.(f && g)
NWAOBDDBaseNodeRefPtr MkNand(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);         // \f.\g.!(f && g)
NWAOBDDBaseNodeRefPtr MkOr(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);           // \f.\g.(f || g)
NWAOBDDBaseNodeRefPtr MkNor(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);          // \f.\g.!(f || g)
NWAOBDDBaseNodeRefPtr MkIff(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);          // \f.\g.(f == g)
NWAOBDDBaseNodeRefPtr MkExclusiveOr(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);  // \f.\g.(f != g)
NWAOBDDBaseNodeRefPtr MkImplies(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);      // \f.\g.(!f || g)
NWAOBDDBaseNodeRefPtr MkMinus(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);        // \f.\g.(f && !g)
NWAOBDDBaseNodeRefPtr MkQuotient(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);     // \f.\g.(!g || f)
NWAOBDDBaseNodeRefPtr MkNotQuotient(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);  // \f.\g.(g && !f)
NWAOBDDBaseNodeRefPtr MkFirst(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);        // \f.\g.f
NWAOBDDBaseNodeRefPtr MkNotFirst(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);     // \f.\g.!f
NWAOBDDBaseNodeRefPtr MkSecond(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);       // \f.\g.g
NWAOBDDBaseNodeRefPtr MkNotSecond(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g);    // \f.\g.!g

// Ternary operations on NWAOBDDBaseNodes ------------------------------------

// \a.\b.\c.(a && b) || (!a && c)
NWAOBDDBaseNodeRefPtr MkIfThenElse(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g, NWAOBDDBaseNodeRefPtr h);

// \a.\b.\c.(b && !a) || (c && !a) || (b && c)
NWAOBDDBaseNodeRefPtr MkNegMajority(NWAOBDDBaseNodeRefPtr f, NWAOBDDBaseNodeRefPtr g, NWAOBDDBaseNodeRefPtr h);

NWAOBDDBaseNodeRefPtr MkExists(NWAOBDDBaseNodeRefPtr f, unsigned int i);              // \f. exists x_i : f
NWAOBDDBaseNodeRefPtr MkForall(NWAOBDDBaseNodeRefPtr f, unsigned int i);              // \f. forall x_i : f
NWAOBDDBaseNodeRefPtr MkComposeBase(NWAOBDDBaseNodeRefPtr f, int i, NWAOBDDBaseNodeRefPtr g);              // \f. f | x_i = g


//********************************************************************
// NWAOBDDTopNode
//********************************************************************


class NWAOBDDTopNode: public NWAOBDDBaseNode
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

//********************************************************************
// NWAOBDDNode
//********************************************************************

class PathSummaryKey {

 public:
  PathSummaryKey(NWAOBDDNodeHandle nodeHandle, int exit, int offset); // Constructor
  unsigned int Hash(unsigned int modsize);
  PathSummaryKey& operator= (const PathSummaryKey& p);  // Overloaded assignment
  bool operator!= (const PathSummaryKey& p);        // Overloaded !=
  bool operator== (const PathSummaryKey& p);        // Overloaded ==
  NWAOBDDNodeHandle NodeHandle() const { return nodeHandle; }      // Access function
  int Exit() const { return exit; }      // Access function
  std::ostream& print(std::ostream & out) const;

 private:
  NWAOBDDNodeHandle nodeHandle;
  int exit;
  int offset;
  PathSummaryKey();                                 // Default constructor (hidden)
};

void InitPathSummaryCache();
void DisposeOfPathSummaryCache();

enum NWAOBDD_NODEKIND { NWAOBDD_INTERNAL, NWAOBDD_EPSILON };

class NWAOBDDNode {
  friend void NWAOBDDNodeHandle::Canonicalize();
  friend void NWAOBDDNodeHandle::InitNoDistinctionTable();
 public:
  NWAOBDDNode();                       // Constructor
  NWAOBDDNode(const unsigned int l);   // Constructor
  virtual ~NWAOBDDNode();              // Destructor
  virtual NWAOBDD_NODEKIND NodeKind() const = 0;
  unsigned int numExits;
  static unsigned int const maxLevel;
#ifdef PATH_COUNTING_ENABLED
  unsigned int *numPathsToExit;       // unsigned int numPathsToExit[numExits]
#endif
  virtual void FillSatisfyingAssignment(unsigned int i, SH_OBDD::Assignment &assignment, unsigned int &index) = 0;
  virtual int Traverse(SH_OBDD::AssignmentIterator &ai) = 0;
  virtual NWAOBDDNodeHandle Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits) = 0;
  virtual unsigned int Hash(unsigned int modsize) = 0;
  virtual void DumpConnections(Hashset<NWAOBDDNode> *visited, std::ostream & out = std::cout) = 0;
  virtual void CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *visitedEdges, unsigned int &nodeCount, unsigned int &edgeCount) = 0;
  virtual bool operator!= (const NWAOBDDNode & n) = 0;  // Overloaded !=
  virtual bool operator== (const NWAOBDDNode & n) = 0;  // Overloaded ==
  virtual void IncrRef() = 0;
  virtual void DecrRef() = 0;
  const unsigned int Level() const { return level; }
 public:
	 virtual std::ostream& print(std::ostream & out = std::cout) const = 0;
  const unsigned int level;
 protected:
  unsigned int refCount;
  bool isCanonical;              // Is this NWAOBDDNode in canonicalNodeTable?
};

std::ostream& operator<< (std::ostream & out, const NWAOBDDNode &n);

//********************************************************************
// NWAOBDDInternalNode
//********************************************************************

class NWAOBDDInternalNode : public NWAOBDDNode {
  friend void NWAOBDDNodeHandle::InitNoDistinctionTable();
  friend NWAOBDDNodeHandle Restrict(NWAOBDDInternalNode *g, unsigned int i, bool val, ReturnMapHandle<int> &MapHandle);
  friend NWAOBDDNodeHandle PairProduct(NWAOBDDInternalNode *n1, NWAOBDDInternalNode *n2, PairProductMapHandle &pairProductMap);
  friend NWAOBDDNodeHandle TripleProduct(NWAOBDDInternalNode *n1, NWAOBDDInternalNode *n2, NWAOBDDInternalNode *n3, TripleProductMapHandle &tripleProductMap);
  friend bool NWAOBDDTopNode::EvaluateIteratively(SH_OBDD::Assignment &assignment);
  friend void NWAOBDDTopNode::PrintYieldAux(std::ostream * out, List<ConsCell<TraverseState> *> &T, ConsCell<TraverseState> *S);
  friend NWAOBDDNodeHandle *InitNoDistinctionTable();
  friend NWAOBDDNodeHandle MkDistinction(unsigned int level, unsigned int i);
  friend NWAOBDDNodeHandle MkIdRelationInterleaved(unsigned int level, int idtype);
  friend NWAOBDDNodeHandle MkAdditionInterleaved(unsigned int level, bool carry);
  friend NWAOBDDNodeHandle MkDetensorConstraintInterleaved(unsigned int level);
  friend NWAOBDDNodeHandle MkParity(unsigned int level);
  friend NWAOBDDNodeHandle MkWalshInterleavedNode(unsigned int i);
  friend NWAOBDDNodeHandle MkInverseReedMullerInterleavedNode(unsigned int i);
  friend NWAOBDDNodeHandle MkStepOneFourth(unsigned int level);
  friend NWAOBDDNodeHandle Restrict(NWAOBDDNodeHandle g, unsigned int i, bool val, ReturnMapHandle<int> &MapHandle);
#ifdef ARBITRARY_STEP_FUNCTIONS
  friend NWAOBDDNodeHandle MkStepNode(unsigned int level, unsigned int left, unsigned int middle, unsigned int right);
#endif

 public:
  NWAOBDDInternalNode(const unsigned int l);   // Constructor
  ~NWAOBDDInternalNode();                      // Destructor
  NWAOBDD_NODEKIND NodeKind() const { return NWAOBDD_INTERNAL; }
  void FillSatisfyingAssignment(unsigned int i, SH_OBDD::Assignment &assignment, unsigned int &index);
  int Traverse(SH_OBDD::AssignmentIterator &ai);
  NWAOBDDNodeHandle Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits);
  static NWAOBDDTopNodeRefPtr SchemaAdjust(NWAOBDDInternalNode * n, int exit, int s[4], int offset);
  static NWAOBDDTopNodeRefPtr PathSummary(NWAOBDDInternalNode * n, int exit, int offset);
  unsigned int Hash(unsigned int modsize);
  void DumpConnections(Hashset<NWAOBDDNode> *visited, std::ostream & out = std::cout);
  void CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *visitedEdges, unsigned int &nodeCount, unsigned int &edgeCount);
  bool operator!= (const NWAOBDDNode & n);        // Overloaded !=
  bool operator== (const NWAOBDDNode & n);        // Overloaded ==
  void IncrRef();
  void DecrRef();

 public:
	 std::ostream& print(std::ostream & out = std::cout) const;
  Connection AConnection[2];              // A single Connection
  unsigned int numBConnections;
  Connection *BConnection[2];             // Connection BConnection[numBConnections];
  unsigned int InsertBConnection(unsigned int &j, Connection &c0, Connection &c1);
#ifdef PATH_COUNTING_ENABLED
  void InstallPathCounts();
#endif

 private:
  NWAOBDDInternalNode();                                         // Default constructor (hidden)
  NWAOBDDInternalNode(const NWAOBDDInternalNode &n);             // Copy constructor (hidden)
  NWAOBDDInternalNode& operator= (const NWAOBDDInternalNode &n); // Overloaded = (hidden)
};

//********************************************************************

std::ostream& operator<< (std::ostream & out, const NWAOBDDTopNode &d);

//********************************************************************
// NWAOBDDLeafNode
//********************************************************************

class NWAOBDDLeafNode : public NWAOBDDNode {
 public:
  NWAOBDDLeafNode();                   // Constructor
  virtual ~NWAOBDDLeafNode();          // Destructor
  virtual NWAOBDD_NODEKIND NodeKind() const = 0;
  virtual void FillSatisfyingAssignment(unsigned int i, SH_OBDD::Assignment &assignment, unsigned int &index) = 0;
  virtual int Traverse(SH_OBDD::AssignmentIterator &ai) = 0;
  virtual NWAOBDDNodeHandle Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits) = 0;
  virtual unsigned int Hash(unsigned int modsize) = 0;
  void DumpConnections(Hashset<NWAOBDDNode> *visited, std::ostream & out = std::cout);
  void CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *visitedEdges, unsigned int &nodeCount, unsigned int &edgeCount);
  virtual bool operator!= (const NWAOBDDNode & n) = 0;  // Overloaded !=
  virtual bool operator== (const NWAOBDDNode & n) = 0;  // Overloaded ==
  void IncrRef();
  void DecrRef();

 public:
	 virtual std::ostream& print(std::ostream & out = std::cout) const = 0;
};

//********************************************************************
// NWAOBDDEpsilonNode
//********************************************************************

class NWAOBDDEpsilonNode : public NWAOBDDLeafNode {
 public:
  NWAOBDDEpsilonNode();                   // Constructor
  ~NWAOBDDEpsilonNode();                  // Destructor
  NWAOBDD_NODEKIND NodeKind() const { return NWAOBDD_EPSILON; }
  void FillSatisfyingAssignment(unsigned int i, SH_OBDD::Assignment &assignment, unsigned int &index);
  int Traverse(SH_OBDD::AssignmentIterator &ai);
  NWAOBDDNodeHandle Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits);
  unsigned int Hash(unsigned int modsize);
  bool operator!= (const NWAOBDDNode & n);        // Overloaded !=
  bool operator== (const NWAOBDDNode & n);        // Overloaded ==

 public:
	 std::ostream& print(std::ostream & out = std::cout) const;

 private:
  NWAOBDDEpsilonNode(const NWAOBDDEpsilonNode &n);   // Copy constructor (hidden)
  NWAOBDDEpsilonNode& operator= (const NWAOBDDEpsilonNode &n); // Overloaded = (hidden)
};




 } // namespace NWA_OBDD

namespace NWA_OBDD {
  void GroupDumpConnectionsStart();
  void GroupDumpConnectionsEnd();

  void GroupCountNodesAndEdgesStart(unsigned int &nodeCount, unsigned int &edgeCount);
  void GroupCountNodesAndEdgesEnd();
}


#endif