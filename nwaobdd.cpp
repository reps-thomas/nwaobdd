//
//	Copyright (c) 1999 Thomas W. Reps
//	All Rights Reserved.
//
//	This software is furnished under a license and may be used and
//	copied only in accordance with the terms of such license and the
//	inclusion of the above copyright notice.  This software or any
//	other copies thereof or any derivative works may not be provided
//	or otherwise made available to any other person.  Title to and
//	ownership of the software and any derivative works is retained
//	by Thomas W. Reps.
//
//	THIS IMPLEMENTATION MAY HAVE BUGS, SOME OF WHICH MAY HAVE SERIOUS
//	CONSEQUENCES.  THOMAS W. REPS PROVIDES THIS SOFTWARE IN ITS "AS IS"
//	CONDITION, AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
//	BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//	AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
//	THOMAS W. REPS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
//	TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include "nwaobdd.h"
#include "nwaobdd_node.h"
#include "nwaobdd_top_node.h"
// #include "nwaobdd_top_node_impl.cpp"
#include "assignment.h"
#include "bool_op.h"
#include "nwaobdd_int.h"
#include <boost/multiprecision/cpp_complex.hpp>


using namespace NWA_OBDD;

//********************************************************************
// NWAOBDD
//********************************************************************

template<typename T>
unsigned int const NWAOBDD<T>::maxLevel = NWAOBDDMaxLevel;

// Constructors/Destructor -------------------------------------------

// Default constructor
template <>
NWAOBDD<int>::NWAOBDD()
  : root(MkTrueTop())
{
}

template <typename T>
NWAOBDD<T>::NWAOBDD(typename NWAOBDDTopNode<T>::NWAOBDDTopNodeTRefPtr n)
{
	root = n;
}

// Copy constructor
template <typename T>
NWAOBDD<T>::NWAOBDD(const NWAOBDD<T> &c)
{
  root = c.root;
}

template <typename T>
NWAOBDD<T>::~NWAOBDD()
{
}

// Operations -----------------------------------------------------

// Evaluate
//    Return the value of the Boolean function under the given assignment
template <typename T>
T NWAOBDD<T>::Evaluate(SH_OBDD::Assignment &assignment)
{
  return root->Evaluate(assignment);
}

template <typename T>
T NWAOBDD<T>::Evaluate(SH_OBDD::Assignment &&assignment)
{ // rvalue-ref version, to pass compiling
  return root->Evaluate(assignment);
}

// PrintYieldSemantic
//
// print the yield of the NWAOBDD (i.e., the leaves of 0's and 1's
// in "left-to-right order").
//
template <typename T>
void NWAOBDD<T>::PrintYieldSemantic(std::ostream & out)
{
  if (NWAOBDDMaxLevel == 1 || NWAOBDDMaxLevel == 2) {
    unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDDMaxLevel + 2)) - (unsigned int)4));
    SH_OBDD::Assignment a(size);
    T b;
    unsigned long int range = 1UL << size;
    for (unsigned long int i = 0UL; i < range; i++) {
      unsigned long int mask = 1UL;
      for (int j = size - 1; j >= 0; j--) {
        a[j] = (i & mask);
        mask = mask << 1;
      }
      b = Evaluate(a);
      out << (b?1:0);
    }
    out << std::endl;
  }
  else {
    std::cerr << "Cannot test all assignments: maxLevel must be 3 or 4" << std::endl;
  }
}

// PrintYield
//
// print the yield of the NWAOBDD (i.e., the leaves of 0's and 1's
// in "left-to-right order").
//
template <typename T>
void NWAOBDD<T>::PrintYield(std::ostream * out)
{
  root->PrintYield(out);
}

// Satisfaction Operations ------------------------------------

// FindOneSatisfyingAssignment
//
// If a satisfying assignment exists, allocate and place such an
//    assignment in variable "assignment" and return true.
// Otherwise return false.
//
// Running time: Linear in the number of variables
//

template<typename T>
void NWAOBDD<T>::DumpValueTuple() {
  root -> DumpValueTuple();
}
template<typename T>
void NWAOBDD<T>::DumpPathCountings() {
  root -> DumpPathCountings();
}

template <typename T>
bool NWAOBDD<T>::FindOneSatisfyingAssignment(SH_OBDD::Assignment * &assignment)
{
  return root->FindOneSatisfyingAssignment(assignment);
}

// Hash
template <typename T>
unsigned int NWAOBDD<T>::Hash(unsigned int modsize)
{
  return root->Hash(modsize);
}

// Overloaded !=
template <typename T>
bool NWAOBDD<T>::operator!= (const NWAOBDD & C) const
{
  return *root != *C.root;
}

// Overloaded ==
template <typename T>
bool NWAOBDD<T>::operator== (const NWAOBDD & C) const
{
  return *root == *C.root;
}

// Overloaded assignment
template <typename T>
NWAOBDD<T>& NWAOBDD<T>::operator= (const NWAOBDD<T> &c)
{
  if (this != &c)      // don't assign to self!
  {
    root = c.root;
  }
  return *this;    	
}

// print
template <typename T>
std::ostream& NWAOBDD<T>::print(std::ostream & out) const
{
  out << *root << std::endl;
  return out;
}

template <typename T>
bool NWAOBDD<T>::DependsOn(int i) const
{
  // horrible performance!
  // if (MkRestrict(*this, i, false) == *this) {
  //   assert(MkRestrict(*this, i, true) == *this); // or this algorithm is bad
  //   return false;
  // }
  // assert(MkRestrict(*this, i, true) != *this);
  // return true;
}

template <typename T>
bool NWAOBDD<T>::IsPositiveCube() const
{
  // return IsPositiveCubeInt(0);
}

template <typename T>
bool NWAOBDD<T>::IsPositiveCubeInt(int least) const
{
  // base case:
  // if (*this == MkFalse()) return false;
  // if (*this == MkTrue()) return true;

  // // recursive step:
  // unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDDMaxLevel + 2)) - (unsigned int)4));
  // for (int xi = least; xi < size; xi++) {
  //   if (DependsOn(xi)) {
  //     if (MkRestrict(*this, xi, false)
  //     != MkFalse())
  //   return false;
  //     if (!MkRestrict(*this, xi, true).IsPositiveCubeInt(xi+1))
  //   return false;
  //   }
  // }
  return true;
}

template <typename T>
bool NWAOBDD<T>::SupportSetIs(const apvector<int> &ss) const
{
  // assert(&ss != NULL);
  // apvector<int> *truess = GetSupportSet();
  // bool rc = (ss == *truess);
  // delete truess;
  // return rc;
}

template <typename T>
apvector<int> *NWAOBDD<T>::GetSupportSet() const
  // Returns a new, sorted-in-increasing-order array
  // containing the support set (the set of VarNums
  // corresponding to indices of projection functions upon which bdd
  // depends -- for (x0+x1*x3), it would be [0, 1, 3]).
  //
  // Delete it when you are done.
{
  // Decent algorithm: find the index associated with each fork node on EVERY path.  kind of ugly, but better than this quickie:

  // apvector<int> *vec = new apvector<int>;
  // unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDDMaxLevel + 2)) - (unsigned int)4));
  // for (int i = 0; i < size; i++) {
  //   if (DependsOn(i)) {
  //     vec->AddToEnd(i);
  //   }
  // }
  // assert(vec != NULL);
  // return vec;

  /*
  init vector
  for xi in ( x0 to highest projection function xn (inclusive) ) {
    if (bdd.DependsOn(xi)) {
      add xi to vector
    }
  }
  return vec;
  */
}


namespace NWA_OBDD {

template <typename T>
std::ostream& operator<< (std::ostream & out, const NWAOBDD<T> &d)
{
  d.print(out);
  return(out);
}

}
// Create a representation of the multiplication relation for natural numbers
// with numBits bits: { (xi yi zi _)* | vec{x} * vec{y} = vec{z} }
// We create entries only for products without overflow, which includes
// products of the form x * 1 and 1 * y.
// The current method is brute force.
// As an optimization, we only create entries for x * y where x >= y.



//********************************************************************
// Operations to gather statistics
//********************************************************************

// Statistics on Connections ----------------------------------

template <typename T>
void NWAOBDD<T>::DumpConnections(std::ostream & out /* = std::cout */)
{
  Hashset<NWAOBDDNode> *visited = new Hashset<NWAOBDDNode>;
  root->DumpConnections(visited, out);
  delete visited;
}

template <typename T>
Hashset<NWAOBDDNode> *NWAOBDD<T>::visitedDuringGroupDumpConnections = NULL;

namespace NWA_OBDD {

template <typename T>
void GroupDumpConnectionsStart()
{
  NWAOBDD<T>::visitedDuringGroupDumpConnections = new Hashset<NWAOBDDNode>;
}

template <typename T>
void GroupDumpConnectionsEnd()
{
  delete NWAOBDD<T>::visitedDuringGroupDumpConnections;
  NWAOBDD<T>::visitedDuringGroupDumpConnections = NULL;
}

template <typename T>
void NWAOBDD<T>::GroupDumpConnections(std::ostream & out /* = std::cout */)
{
  root->DumpConnections(NWAOBDD<T>::visitedDuringGroupDumpConnections, out);
}
}
// Statistics on size ----------------------------------------------------

template <typename T>
void NWAOBDD<T>::CountNodesAndEdges(unsigned int &nodeCount, unsigned int &edgeCount)
{
  Hashset<NWAOBDDNode> *visitedNodes = new Hashset<NWAOBDDNode>;
  Hashset<ReturnMapBody<intpair>> *visitedEdges = new Hashset<ReturnMapBody<intpair>>;
  nodeCount = 0;
  edgeCount = 0;
  root->CountNodesAndEdges(visitedNodes, visitedEdges, nodeCount, edgeCount);
  delete visitedNodes;
  delete visitedEdges;
}

template <typename T>
Hashset<NWAOBDDNode> *NWAOBDD<T>::visitedNodesDuringGroupCountNodesAndEdges = NULL;

template <typename T>
Hashset<ReturnMapBody<intpair>> *NWAOBDD<T>::visitedEdgesDuringGroupCountNodesAndEdges = NULL;

namespace NWA_OBDD {

template <typename T>
void GroupCountNodesAndEdgesStart(unsigned int &nodeCount, unsigned int &edgeCount)
{
  nodeCount = 0;
  edgeCount = 0;
  NWAOBDD<T>::visitedNodesDuringGroupCountNodesAndEdges = new Hashset<NWAOBDDNode>;
  NWAOBDD<T>::visitedEdgesDuringGroupCountNodesAndEdges = new Hashset<ReturnMapBody<intpair>>;
}

template <typename T>
void GroupCountNodesAndEdgesEnd()
{
  delete NWAOBDD<T>::visitedNodesDuringGroupCountNodesAndEdges;
  delete NWAOBDD<T>::visitedEdgesDuringGroupCountNodesAndEdges;
  NWAOBDD<T>::visitedNodesDuringGroupCountNodesAndEdges = NULL;
  NWAOBDD<T>::visitedEdgesDuringGroupCountNodesAndEdges = NULL;
}
}

template <typename T>
void NWAOBDD<T>::GroupCountNodesAndEdges(unsigned int &nodeCount, unsigned int &edgeCount)
{
  root->CountNodesAndEdges(NWAOBDD<T>::visitedNodesDuringGroupCountNodesAndEdges,
                           NWAOBDD<T>::visitedEdgesDuringGroupCountNodesAndEdges,
                           nodeCount, edgeCount
                          );
}


namespace NWA_OBDD {
	template class NWAOBDD<int>;
	template std::ostream& operator<< (std::ostream & out, const NWAOBDD<int> &d);



namespace mp = boost::multiprecision;
typedef mp::cpp_complex_100 BIG_COMPLEX_FLOAT;

	template class NWAOBDD<BIG_COMPLEX_FLOAT>;
	template std::ostream& operator<< (std::ostream & out, const NWAOBDD<BIG_COMPLEX_FLOAT> &d);
};