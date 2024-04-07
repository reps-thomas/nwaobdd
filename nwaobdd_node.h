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
#define PATH_COUNTING_ENABLED 1

namespace NWA_OBDD {
// Node classes declared in this file --------------------------------
template<typename T> class NWAOBDDTopNode;
class NWAOBDDNode;
class NWAOBDDInternalNode;   //  : public NWAOBDDNode
class NWAOBDDEpsilonNode;   //  : public NWAOBDDLeafNode
class NWAOBDDNodeHandle;
 }

#include <iostream>
#include <fstream>
#include "infra/list_T.h"
#include "infra/list_TPtr.h"
#include "infra/intpair.h"
#include "infra/conscell.h"
#include "assignment.h"
#include "bool_op.h"
#include "return_map_T.h"
#include "reduction_map.h"
#include "traverse_state_nwa.h"
#include "infra/hash.h"
#include "infra/hashset.h"
#include "infra/ref_ptr.h"
#include "connection.h"
#include "unordered_map"
#include <unordered_set>



namespace NWA_OBDD {
typedef ref_ptr<NWAOBDDTopNode<int>> NWAOBDDTopNodeRefPtr;


//********************************************************************
// NWAOBDDNodeHandle
//********************************************************************

struct NWAOBDDNodeStarHash {
  size_t operator() (const NWAOBDDNode* n) const;
};

struct NWAOBDDNodeStarEq {
  bool operator() (const NWAOBDDNode* n1, const NWAOBDDNode* n2) const;
};

class NWAOBDDNodeHandle {
#define NWAOBDD_NODE_HANDLE_GUARD
 public:
  NWAOBDDNodeHandle();                                        // Default constructor
  NWAOBDDNodeHandle(NWAOBDDNode *n);                          // Constructor
  NWAOBDDNodeHandle(const NWAOBDDNodeHandle &nh);              // Copy constructor
  ~NWAOBDDNodeHandle();                                       // Destructor
  unsigned int Hash(unsigned int modsize);
  bool operator!= (const NWAOBDDNodeHandle &nh) const;              // Overloaded !=
  bool operator== (const NWAOBDDNodeHandle &nh) const;              // Overloaded ==
  NWAOBDDNodeHandle & operator= (const NWAOBDDNodeHandle &nh); // assignment

  // Distinguished NWAOBDDNodeHandles -----------------------
     public:
      static NWAOBDDNodeHandle *NoDistinctionNode;     // NWAOBDDNodeHandle NoDistinctionNode[maxLevel+1]
      static NWAOBDDNodeHandle NWAOBDDEpsilonNodeHandle;
      static void InitNoDistinctionTable();

  NWAOBDDNode *handleContents;

 // Table of canonical nodes -------------------------
    public:
    //  static Hashset<NWAOBDDNode> *canonicalNodeTable;
     static std::unordered_set<NWAOBDDNode*, NWAOBDDNodeStarHash, NWAOBDDNodeStarEq> canonicalNodeTableLeveled[26];
     void Canonicalize();

 // Reduce and its associated cache ---------------
    public:
     NWAOBDDNodeHandle Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits, bool forceReduce = false);
	 static NWAOBDDTopNodeRefPtr SchemaAdjust(NWAOBDDNodeHandle n, int exit, int s[4], int offset);
   // added to pass compilation
   static NWAOBDDTopNodeRefPtr SchemaAdjust(NWAOBDDNodeHandle *n, int exit, int s[4], int offset);
   static NWAOBDDTopNodeRefPtr PathSummary(NWAOBDDNodeHandle n, int exit, int offset);
   // added to pass compilation
   static NWAOBDDTopNodeRefPtr PathSummary(NWAOBDDNodeHandle *n, int exit, int offset);
     static void InitReduceCache();
     static void DisposeOfReduceCache();

 public:
	 std::ostream& print(std::ostream & out = std::cout) const;
   friend std::ostream& operator<< (std::ostream & out, const NWAOBDDNodeHandle &d);
};

} // namespace NWAOBDDs

#include "cross_product_nwa.h"

namespace NWA_OBDD {
// Other classes and types declared in this file ---------------------
typedef List<intpair> RestrictMap;
typedef ListIterator<intpair> RestrictMapIterator;

typedef ConnectionT<ReturnMapHandle<intpair> > Connection;
class NWAReduceKey;

// Auxiliary data -----------------------------------------------

// Note: If NWAOBDDMaxLevel >= 27, allocating an Assignment will cause
//       virtual memory to be exceeded (on velveeta).
static unsigned int NWAOBDDMaxLevel = 25;
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
  long long id;
  static unsigned int const maxLevel;
#ifdef PATH_COUNTING_ENABLED
  long double *numPathsToExit = nullptr;       // unsigned int numPathsToExit[numExits]
  bool isNumPathsMemAllocated;
#endif
  virtual void FillSatisfyingAssignment(unsigned int i, SH_OBDD::Assignment &assignment, unsigned int &index) = 0;
  virtual int Traverse(SH_OBDD::AssignmentIterator &ai) = 0;
  virtual NWAOBDDNodeHandle Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits, bool forceReduce) = 0;
  virtual unsigned int Hash(unsigned int modsize) const = 0;
  virtual void DumpConnections(Hashset<NWAOBDDNode> *visited, std::ostream & out = std::cout) = 0;
  virtual void CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *visitedEdges, unsigned int &nodeCount, unsigned int &edgeCount) = 0;
  virtual bool operator!= (const NWAOBDDNode & n) const = 0;  // Overloaded !=
  virtual bool operator== (const NWAOBDDNode & n) const = 0;  // Overloaded ==
  virtual void IncrRef() = 0;
  virtual void DecrRef() = 0;
  const unsigned int Level() const { return level; }
  unsigned int GetRefCount(){ return refCount; }
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
  // friend NWAOBDDNodeHandle TripleProduct(NWAOBDDInternalNode *n1, NWAOBDDInternalNode *n2, NWAOBDDInternalNode *n3, TripleProductMapHandle &tripleProductMap);
  // friend bool NWAOBDDTopNode::EvaluateIteratively(SH_OBDD::Assignment &assignment);
  // friend void NWAOBDDTopNode::PrintYieldAux(std::ostream * out, List<ConsCell<TraverseState> *> &T, ConsCell<TraverseState> *S);
  friend NWAOBDDNodeHandle *InitNoDistinctionTable();
  friend NWAOBDDNodeHandle MkDistinction(unsigned int level, unsigned int i);
  friend NWAOBDDNodeHandle MkIdRelationInterleaved(unsigned int level, int idtype);
  friend NWAOBDDNodeHandle MkAdditionInterleaved(unsigned int level, bool carry);
  friend NWAOBDDNodeHandle MkDetensorConstraintInterleaved(unsigned int level);
  // friend NWAOBDDNodeHandle MkParity(unsigned int level);
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
  long double *numPathsToMiddle = nullptr;
  std::unordered_map<ReductionMapHandle, NWAOBDDNodeHandle, RedMapHash> * reduceCache = nullptr;
  void FillSatisfyingAssignment(unsigned int i, SH_OBDD::Assignment &assignment, unsigned int &index);
  int Traverse(SH_OBDD::AssignmentIterator &ai);
  NWAOBDDNodeHandle Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits, bool forceReduce = false);
  static NWAOBDDTopNodeRefPtr SchemaAdjust(NWAOBDDInternalNode * n, int exit, int s[4], int offset);
  static NWAOBDDTopNodeRefPtr PathSummary(NWAOBDDInternalNode * n, int exit, int offset);
  unsigned int Hash(unsigned int modsize) const;
  void DumpConnections(Hashset<NWAOBDDNode> *visited, std::ostream & out = std::cout);
  void CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *visitedEdges, unsigned int &nodeCount, unsigned int &edgeCount);
  bool operator!= (const NWAOBDDNode & n) const;        // Overloaded !=
  bool operator== (const NWAOBDDNode & n) const;        // Overloaded ==
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
NWAOBDDNodeHandle MkParity(unsigned int level);
NWAOBDDNodeHandle MkDistinction(unsigned int level, unsigned int i);
NWAOBDDNodeHandle MkIdRelationNested(unsigned int level);
//********************************************************************

//********************************************************************
// NWAOBDDEpsilonNode
//********************************************************************

class NWAOBDDEpsilonNode : public NWAOBDDNode {
 public:
  NWAOBDDEpsilonNode();                   // Constructor
  ~NWAOBDDEpsilonNode();                  // Destructor
  NWAOBDD_NODEKIND NodeKind() const { return NWAOBDD_EPSILON; }
  void FillSatisfyingAssignment(unsigned int i, SH_OBDD::Assignment &assignment, unsigned int &index);
  int Traverse(SH_OBDD::AssignmentIterator &ai);
  NWAOBDDNodeHandle Reduce(ReductionMapHandle redMapHandle, unsigned int replacementNumExits, bool forceReduce = false);
  unsigned int Hash(unsigned int modsize) const;
  bool operator!= (const NWAOBDDNode & n) const;        // Overloaded !=
  bool operator== (const NWAOBDDNode & n) const;        // Overloaded ==
  void DumpConnections(Hashset<NWAOBDDNode> *visited, std::ostream & out = std::cout);
  void CountNodesAndEdges(Hashset<NWAOBDDNode> *visitedNodes, Hashset<ReturnMapBody<intpair>> *visitedEdges, unsigned int &nodeCount, unsigned int &edgeCount);
  void IncrRef();
  void DecrRef();

 public:
	 std::ostream& print(std::ostream & out = std::cout) const;

 private:
  NWAOBDDEpsilonNode(const NWAOBDDEpsilonNode &n);   // Copy constructor (hidden)
  NWAOBDDEpsilonNode& operator= (const NWAOBDDEpsilonNode &n); // Overloaded = (hidden)
};



struct NWAOBDDNodeHandleHash {
  size_t operator ()(const NWAOBDDNodeHandle &nh) const;
};


 } // namespace NWA_OBDD

namespace NWA_OBDD {
  void GroupDumpConnectionsStart();
  void GroupDumpConnectionsEnd();

  void GroupCountNodesAndEdgesStart(unsigned int &nodeCount, unsigned int &edgeCount);
  void GroupCountNodesAndEdgesEnd();
}


#endif