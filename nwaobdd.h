#ifndef NWAOBDD_GUARD
#define NWAOBDD_GUARD


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

#include <iostream>
#include <fstream>
#include "nwaobdd_node.h"
#include "nwaobdd_top_node.h"
#include "assignment.h"
#include "bool_op.h"

namespace NWA_OBDD {

// Node classes declared in this file --------------------------------
template<typename T>class NWAOBDD;

//********************************************************************
// NWAOBDD
//********************************************************************

// Representation of a Boolean function
template <typename T>
class NWAOBDD {
  friend void GroupCountNodesAndEdgesStart(unsigned int &nodeCount, unsigned int &edgeCount);
  friend void GroupCountNodesAndEdgesEnd();
  friend void GroupDumpConnectionsStart();
  friend void GroupDumpConnectionsEnd();
 public:
  NWAOBDD();                                    // Default constructor (rep. of \a.true)
  NWAOBDD(typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr n);              // Constructor
  NWAOBDD(const NWAOBDD &d);                    // Copy constructor
  ~NWAOBDD();                                   // Destructor
  static unsigned int const maxLevel;
  T Evaluate(SH_OBDD::Assignment &assignment);        // Evaluate a Boolean function
  T Evaluate(SH_OBDD::Assignment &&assignment);        // rvalue-ref version to pass comiling
  void PrintYield(std::ostream * out);               // print the yield of the "tree"
  void PrintYieldSemantic(std::ostream & out);       // print the yield of the "tree"
#ifdef PATH_COUNTING_ENABLED
  unsigned int NumSatisfyingAssignments();      // Return number of satisfying assignments
#endif
  bool FindOneSatisfyingAssignment(SH_OBDD::Assignment * &assignment);
                                                // Find a satisfying assignment
  unsigned int Hash(unsigned int modsize);
  bool operator!= (const NWAOBDD & C) const;          // Overloaded !=
  bool operator== (const NWAOBDD & C) const;          // Overloaded ==
  NWAOBDD & operator= (const NWAOBDD &c);       // assignment
  ref_ptr<NWAOBDDTopNode<T>> root;
  bool DependsOn(int i) const;
  bool IsPositiveCube() const;
  bool IsPositiveCubeInt(int least) const;
  bool SupportSetIs(const apvector<int> &ss) const;
  apvector<int> *GetSupportSet() const;


  void DumpConnections(std::ostream & out = std::cout);
  void GroupDumpConnections(std::ostream & out = std::cout);
  void CountNodesAndEdges(unsigned int &nodeCount, unsigned int &edgeCount);
  void GroupCountNodesAndEdges(unsigned int &nodeCount, unsigned int &edgeCount);

 public:
	 std::ostream& print(std::ostream & out = std::cout) const;
  static Hashset<NWAOBDDNode> *visitedNodesDuringGroupCountNodesAndEdges;
  static Hashset<ReturnMapBody<intpair>> *visitedEdgesDuringGroupCountNodesAndEdges;
  static Hashset<NWAOBDDNode> *visitedDuringGroupDumpConnections;
};

template <typename T>
std::ostream& operator<< (std::ostream & out, const NWAOBDD<T> &d);

// NWAOBDD-creation operations --------------------------------------
NWAOBDD<int> MkTrue();                             // Representation of \x.true
NWAOBDD<int> MkFalse();                            // Representation of \x.false
NWAOBDD<int> MkProjection(unsigned int i);         // Representation of \x.x_i
NWAOBDD<int> MkIdRelationNested();
NWAOBDD<int> MkIdRelationInterleaved();            // Representation of identity relation
NWAOBDD<int> MkAdditionNested();                   // Representation of addition relation { (xi yi zi _)* | vec{x} + vec{y} = vec{z} }
NWAOBDD<int> MkAdditionInterleavedBruteForce();    // Representation of addition relation { (xi yi zi _)* | vec{x} + vec{y} = vec{z} }, created by brute force
NWAOBDD<int> MkX(unsigned int i);                  // Bits of i -> vec{x} for addition relation
NWAOBDD<int> MkY(unsigned int i);                  // Bits of i -> vec{y} for addition relation
NWAOBDD<int> MkZ(unsigned int i);                  // Bits of i -> vec{z} for addition relation
NWAOBDD<int> MkMultiplicationInterleavedBruteForce(); // Representation of multiplication relation { (xi yi zi _)* | vec{x} * vec{y} = vec{z} }
bool FactorZ(NWAOBDD<int> R, unsigned int z, NWAOBDD<int> &f1, NWAOBDD<int> &f2, unsigned int &v1, unsigned int &v2);   // Return true if z has a non-trivial factorization
NWAOBDD<int> MkDetensorConstraintInterleaved();    // Representation of (W,X,Y,Z) s.t. X==Y with interleaved variables
NWAOBDD<int> MkParity();                           // Representation of parity function
NWAOBDD<int> MkWalshInterleaved(unsigned int i);   // Representation of Walsh matrix
NWAOBDD<int> MkInverseReedMullerInterleaved(unsigned int i);   // Representation of Inverse Reed-Muller matrix
NWAOBDD<int> MkStepUpOneFourth();                  // Representation of step function
NWAOBDD<int> MkStepDownOneFourth();                // Representation of step function
#ifdef ARBITRARY_STEP_FUNCTIONS
  NWAOBDD MkStepUp(unsigned int i);           // Representation of step function
  NWAOBDD MkStepDown(unsigned int i);         // Representation of step function
  NWAOBDD MkPulseUp(unsigned int i, unsigned int j);
                                              // Representation of pulse function
  NWAOBDD MkPulseDown(unsigned int i, unsigned int j);
                                              // Representation of step function
#endif

// Unary operations on NWAOBDDs --------------------------------------
NWAOBDD<int> MkNot(NWAOBDD<int> f);                     // \f.!f

// Binary operations on NWAOBDDs --------------------------------------
NWAOBDD<int> MkAnd(NWAOBDD<int> f, NWAOBDD<int> g);          // \f.\g.(f && g)
NWAOBDD<int> MkNand(NWAOBDD<int> f, NWAOBDD<int> g);         // \f.\g.!(f && g)
NWAOBDD<int> MkOr(NWAOBDD<int> f, NWAOBDD<int> g);           // \f.\g.(f || g)
NWAOBDD<int> MkNor(NWAOBDD<int> f, NWAOBDD<int> g);          // \f.\g.!(f || g)
NWAOBDD<int> MkIff(NWAOBDD<int> f, NWAOBDD<int> g);          // \f.\g.(f == g)
NWAOBDD<int>MkExclusiveOr(NWAOBDD<int>f, NWAOBDD<int>g);  // \f.\g.(f != g)
NWAOBDD<int>MkImplies(NWAOBDD<int>f, NWAOBDD<int>g);      // \f.\g.(!f || g)
NWAOBDD<int>MkMinus(NWAOBDD<int>f, NWAOBDD<int>g);        // \f.\g.(f && !g)
NWAOBDD<int>MkQuotient(NWAOBDD<int>f, NWAOBDD<int>g);     // \f.\g.(!g || f)
NWAOBDD<int>MkNotQuotient(NWAOBDD<int>f, NWAOBDD<int>g);  // \f.\g.(g && !f)
NWAOBDD<int>MkFirst(NWAOBDD<int>f, NWAOBDD<int>g);        // \f.\g.f
NWAOBDD<int>MkNotFirst(NWAOBDD<int>f, NWAOBDD<int>g);     // \f.\g.!f
NWAOBDD<int>MkSecond(NWAOBDD<int>f, NWAOBDD<int>g);       // \f.\g.g
NWAOBDD<int>MkNotSecond(NWAOBDD<int>f, NWAOBDD<int>g);    // \f.\g.!g

// Ternary operations on NWAOBDDs --------------------------------------
NWAOBDD<int>MkIfThenElse(NWAOBDD<int>f, NWAOBDD<int>g, NWAOBDD<int>h);  // \a.\b.\c.(a && b) || (!a && c)
NWAOBDD<int>MkNegMajority(NWAOBDD<int>f, NWAOBDD<int>g, NWAOBDD<int>h); // \a.\b.\c.(b && !a) || (c && !a) || (b && c)

NWAOBDD<int>MkRestrict(NWAOBDD<int>f, unsigned int i, bool val);  // \f. f | (x_i = val)
NWAOBDD<int>MkExists(NWAOBDD<int>f, unsigned int i);              // \f. exists x_i : f
NWAOBDD<int>MkForall(NWAOBDD<int>f, unsigned int i);              // \f. forall x_i : f
NWAOBDD<int>SchemaAdjust(NWAOBDD<int>f, int s[4]);
NWAOBDD<int>PathSummary(NWAOBDD<int>f);
NWAOBDD<int>MkCompose(NWAOBDD<int>f, int i, NWAOBDD<int>g);  // \f. f | (x_i = g)

} // namespace NWA_OBDD

#endif
