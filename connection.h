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


namespace NWA_OBDD {

// Node classes declared in this file --------------------------------
class NWAOBDDTopNode;
class NWAOBDDBaseNode;
class NWAOBDDNode;
class NWAOBDDInternalNode;   //  : public NWAOBDDNode
class NWAOBDDLeafNode;       //  : public NWAOBDDNode
class NWAOBDDEpsilonNode;   //  : public NWAOBDDLeafNode
class NWAOBDDNodeHandle;

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
   friend std::ostream& operator<< (std::ostream & out, const NWAOBDDNodeHandle &d);
};

//********************************************************************
// Connection
//********************************************************************

class Connection {
 public:
  Connection();                                  // Default constructor
  Connection(NWAOBDDNode *entryPoint, ReturnMapHandle<intpair> (&returnMapHandle));
  Connection(NWAOBDDNodeHandle &entryPointHandle, ReturnMapHandle<intpair> (&returnMapHandle));
  Connection(NWAOBDDNodeHandle &&entryPointHandle, ReturnMapHandle<intpair> (&returnMapHandle));
  ~Connection();                                 // Destructor
  unsigned int Hash(unsigned int modsize);
  Connection& operator= (const Connection &C);   // Overloaded =
  bool operator!= (const Connection & C);        // Overloaded !=
  bool operator== (const Connection & C);        // Overloaded ==

  NWAOBDDNodeHandle entryPointHandle;
// #define compile_hack = 1
  ReturnMapHandle<intpair> returnMapHandle;
// #define compile_hack = 0
 public:
	std::ostream& print(std::ostream & out = std::cout) const;
   friend std::ostream& operator<< (std::ostream & out, const Connection &c);
};

} // namespace NWA_OBDD