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
#include "nwaobdd_top_node_impl.cpp"
#include "assignment.h"
#include "bool_op.h"

using namespace NWA_OBDD;

//********************************************************************
// NWAOBDD
//********************************************************************

unsigned int const NWAOBDD::maxLevel = NWAOBDDMaxLevel;

// Constructors/Destructor -------------------------------------------

// Default constructor
NWAOBDD::NWAOBDD()
  : root(MkTrueTop())
{
}

NWAOBDD::NWAOBDD(NWAOBDDTopNodeRefPtr n)
  : root(n)
{
}

// Copy constructor
NWAOBDD::NWAOBDD(const NWAOBDD &c)
{
  root = c.root;
}

NWAOBDD::~NWAOBDD()
{
}

// Operations -----------------------------------------------------

// Evaluate
//    Return the value of the Boolean function under the given assignment
bool NWAOBDD::Evaluate(SH_OBDD::Assignment &assignment)
{
  return root->Evaluate(assignment);
}


bool NWAOBDD::Evaluate(SH_OBDD::Assignment &&assignment)
{ // rvalue-ref version, to pass compiling
  return root->Evaluate(assignment);
}

// PrintYieldSemantic
//
// print the yield of the NWAOBDD (i.e., the leaves of 0's and 1's
// in "left-to-right order").
//
void NWAOBDD::PrintYieldSemantic(std::ostream & out)
{
  if (NWAOBDDMaxLevel == 1 || NWAOBDDMaxLevel == 2) {
    unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDDMaxLevel + 2)) - (unsigned int)4));
    SH_OBDD::Assignment a(size);
    bool b;
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
void NWAOBDD::PrintYield(std::ostream * out)
{
  root->PrintYield(out);
}

// Satisfaction Operations ------------------------------------

#ifdef PATH_COUNTING_ENABLED
// NumSatisfyingAssignments
//
// Return the number of satisfying assignments
//
// Running time: Linear in the size of the NWAOBDD
//
unsigned int NWAOBDD::NumSatisfyingAssignments()
{
  return root->NumSatisfyingAssignments();
}
#endif

// FindOneSatisfyingAssignment
//
// If a satisfying assignment exists, allocate and place such an
//    assignment in variable "assignment" and return true.
// Otherwise return false.
//
// Running time: Linear in the number of variables
//
bool NWAOBDD::FindOneSatisfyingAssignment(SH_OBDD::Assignment * &assignment)
{
  return root->FindOneSatisfyingAssignment(assignment);
}

// Hash
unsigned int NWAOBDD::Hash(unsigned int modsize)
{
  return root->Hash(modsize);
}

// Overloaded !=
bool NWAOBDD::operator!= (const NWAOBDD & C) const
{
  return *root != *C.root;
}

// Overloaded ==
bool NWAOBDD::operator== (const NWAOBDD & C) const
{
  return *root == *C.root;
}

// Overloaded assignment
NWAOBDD & NWAOBDD::operator= (const NWAOBDD &c)
{
  if (this != &c)      // don't assign to self!
  {
    root = c.root;
  }
  return *this;    	
}

// print
std::ostream& NWAOBDD::print(std::ostream & out) const
{
  out << *root << std::endl;
  return out;
}

bool NWAOBDD::DependsOn(int i) const
{
  // horrible performance!
  if (MkRestrict(*this, i, false) == *this) {
    assert(MkRestrict(*this, i, true) == *this); // or this algorithm is bad
    return false;
  }
  assert(MkRestrict(*this, i, true) != *this);
  return true;
}

bool NWAOBDD::IsPositiveCube() const
{
  return IsPositiveCubeInt(0);
}

bool NWAOBDD::IsPositiveCubeInt(int least) const
{
  // base case:
  if (*this == MkFalse()) return false;
  if (*this == MkTrue()) return true;

  // recursive step:
  unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDDMaxLevel + 2)) - (unsigned int)4));
  for (int xi = least; xi < size; xi++) {
    if (DependsOn(xi)) {
      if (MkRestrict(*this, xi, false)
      != MkFalse())
    return false;
      if (!MkRestrict(*this, xi, true).IsPositiveCubeInt(xi+1))
    return false;
    }
  }
  return true;
}

bool NWAOBDD::SupportSetIs(const apvector<int> &ss) const
{
  assert(&ss != NULL);
  apvector<int> *truess = GetSupportSet();
  bool rc = (ss == *truess);
  delete truess;
  return rc;
}

apvector<int> *NWAOBDD::GetSupportSet() const
  // Returns a new, sorted-in-increasing-order array
  // containing the support set (the set of VarNums
  // corresponding to indices of projection functions upon which bdd
  // depends -- for (x0+x1*x3), it would be [0, 1, 3]).
  //
  // Delete it when you are done.
{
  // Decent algorithm: find the index associated with each fork node on EVERY path.  kind of ugly, but better than this quickie:

  apvector<int> *vec = new apvector<int>;
  unsigned int size = ((unsigned int)((((unsigned int)1) << (NWAOBDDMaxLevel + 2)) - (unsigned int)4));
  for (int i = 0; i < size; i++) {
    if (DependsOn(i)) {
      vec->AddToEnd(i);
    }
  }
  assert(vec != NULL);
  return vec;

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

std::ostream& operator<< (std::ostream & out, const NWAOBDD &d)
{
  d.print(out);
  return(out);
}

// NWAOBDD-creation operations --------------------------------------

// Create representation of \x.true
NWAOBDD MkTrue()
{
  return NWAOBDD(MkTrueTop());
}

// Create representation of \x.false
NWAOBDD MkFalse()
{
  return NWAOBDD(MkFalseTop());
}

// Create representation of \x.x_i
NWAOBDD MkProjection(unsigned int i)
{
  assert(i < ((unsigned int)((((unsigned int)1) << (NWAOBDDMaxLevel + 2)) - (unsigned int)4)));
  return NWAOBDD(MkDistinction(i));
}

NWAOBDD MkCompose(NWAOBDD f, int i, NWAOBDD g)
{
#ifdef NDEBUG
  return  MkComposeTop(f.root, i, g.root);
#else
  NWAOBDD comp = MkComposeTop(f.root, i, g.root);
  assert(comp.root->count == 1);
  return comp;
#endif
}

// Create representation of identity relation
NWAOBDD MkIdRelationInterleaved()
{
  return NWAOBDD(MkIdRelationInterleavedTop());
}

NWAOBDD MkIdRelationNested()
{
	return NWAOBDD(MkIdRelationNestedTop());
}

// MkXBit, MkYBit, MkZBit -----------------------------------------------
//
// MISSING: introduce a level of indirection so that the bits can be
// in a different order than interleaved.
static NWAOBDD MkXBit(unsigned int j) {
	return MkProjection(j * 4);
}

static NWAOBDD MkYBit(unsigned int j) {
	return MkProjection(j * 4 + 1);
}

#define EVEN(x) (((x)/2) * 2 == (x))

static NWAOBDD MkZBit(unsigned int j) {
//	return MkProjection((j/2) * 4 + 2 + (EVEN(j) ? 0 : 1));
	return MkProjection(j * 4 + 2);
}

// Create a representation of the addition relation for natural numbers
// { (xi yi zi _)* | vec{x} * vec{y} = vec{z} }.
// We create entries only for sums where the highest bit of vec{x} and vec{y}
// are 0.
NWAOBDD MkAdditionNested()
{
	//ETTODO
	NWAOBDD quasiAns = NWAOBDD(MkAdditionNestedTop());

	// Restrict the highest bits of x and y to 0
	NWAOBDD xRestriction = MkNot(MkXBit(1 << ((NWAOBDDMaxLevel - 2) - 1)));
	NWAOBDD yRestriction = MkNot(MkYBit(1 << ((NWAOBDDMaxLevel - 2) - 1)));
    return MkAnd(quasiAns, MkAnd(xRestriction, yRestriction));
}

// Create a representation of the addition relation for natural numbers
// { (xi yi zi _)* | vec{x} * vec{y} = vec{z} } by brute force.
// We create entries only for sums where the highest bit of vec{x} and vec{y}
// are 0.
NWAOBDD MkAdditionInterleavedBruteForce() {
	NWAOBDD ans = MkFalse();
	if (2 <= NWAOBDDMaxLevel && NWAOBDDMaxLevel <= 7) {
		unsigned int numBits = 1 << (NWAOBDDMaxLevel - 2);   // i.e., 2**(maxLevel-2)
		for (unsigned int i = 0; i < ((unsigned int)(1 << (numBits - 1))); i++) {
			NWAOBDD X = MkX(i);
			for (unsigned int j = 0; j < ((unsigned int)(1 << (numBits - 1))); j++) {
				NWAOBDD Y = MkY(j);
				NWAOBDD Z = MkZ(i+j);
				ans = MkOr(ans, MkAnd(X,MkAnd(Y,Z)));
			}
		}
	}
	else {
		std::cerr << "Cannot use MkAdditionInterleavedBruteForce: maxLevel must be in the range [2..7]" << std::endl;
	}
	return ans;
}

// Bits of i -> vec{x} for addition relation
NWAOBDD MkX(unsigned int i) {
 	NWAOBDD ans = MkTrue();
	if (2 <= NWAOBDDMaxLevel && NWAOBDDMaxLevel <= 7) {
		unsigned int bits = 1 << (NWAOBDDMaxLevel - 2);   // i.e., 2**(maxLevel-2)
		unsigned int mask = 1;
		for (unsigned int j = 0; j < bits; j++) {
			if (mask & i) ans = MkAnd(ans, MkXBit(j));
			else          ans = MkAnd(ans, MkNot(MkXBit(j)));
			mask = mask << 1;
		}
	}
	else {
		std::cerr << "Cannot use MkX: maxLevel must be in the range [2..7]" << std::endl;
	}
	return ans;
}

// Bits of i -> vec{y} for addition relation
NWAOBDD MkY(unsigned int i) {
 	NWAOBDD ans = MkTrue();
	if (2 <= NWAOBDDMaxLevel && NWAOBDDMaxLevel <= 7) {
		unsigned int bits = 1 << (NWAOBDDMaxLevel - 2);   // i.e., 2**(maxLevel-2)
		unsigned int mask = 1;
		for (unsigned int j = 0; j < bits; j++) {
			if (mask & i) ans = MkAnd(ans, MkYBit(j));
			else          ans = MkAnd(ans, MkNot(MkYBit(j)));
			mask = mask << 1;
		}
	}
	else {
		std::cerr << "Cannot use MkY: maxLevel must be in the range [2..7]" << std::endl;
	}
	return ans;
}

// Bits of i -> vec{z} for addition relation
NWAOBDD MkZ(unsigned int i) {
 	NWAOBDD ans = MkTrue();
	if (2 <= NWAOBDDMaxLevel && NWAOBDDMaxLevel <= 7) {
		unsigned int bits = 1 << (NWAOBDDMaxLevel - 2);   // i.e., 2**(maxLevel-2)
		unsigned int mask = 1;
		for (unsigned int j = 0; j < bits; j++) {
			if (mask & i) ans = MkAnd(ans, MkZBit(j));
			else          ans = MkAnd(ans, MkNot(MkZBit(j)));
			mask = mask << 1;
		}
	}
	else {
		std::cerr << "Cannot use MkZ: maxLevel must be in the range [2..7]" << std::endl;
	}
	return ans;
}
}
// Create a representation of the multiplication relation for natural numbers
// with numBits bits: { (xi yi zi _)* | vec{x} * vec{y} = vec{z} }
// We create entries only for products without overflow, which includes
// products of the form x * 1 and 1 * y.
// The current method is brute force.
// As an optimization, we only create entries for x * y where x >= y.
NWAOBDD MkMultiplicationInterleavedBruteForce() {
	NWAOBDD ans = MkFalse();
	if (2 <= NWAOBDDMaxLevel && NWAOBDDMaxLevel <= 7) {
		unsigned int numBits = 1 << (NWAOBDDMaxLevel - 2);   // i.e., 2**(maxLevel-2)

		NWAOBDD ZeroZ = MkZ(0);

		// Add all products of the form 0 * y = 0
//		std::cout << "Add all products of the form 0 * y = 0" << std::endl;
//		NWAOBDD ZeroXZ = MkAnd(MkX(0), ZeroZ);
//		for (unsigned int j = 0; j < ((unsigned int)(1 << numBits)); j++) {
//			NWAOBDD Y = MkY(j);
//			ans = MkOr(ans, MkAnd(ZeroXZ,Y));
//		}

		// Add all products of the form x * 0 = 0 -- i.e., x is unconstrained
		std::cout << "Add all products of the form x * 0 = 0" << std::endl;
		ans = MkOr(ans, MkAnd(MkY(0), ZeroZ));
/*		NWAOBDD ZeroYZ = MkAnd(MkY(0), ZeroZ);
		for (unsigned int j = 0; j < ((unsigned int)(1 << numBits)); j++) {
			NWAOBDD X = MkX(j);
			ans = MkOr(ans, MkAnd(X, ZeroYZ));
		}
*/
		//		// Add all products of the form 1 * y = y, y in [1 .. 2^numBits - 1]
//		std::cout << "Add all products of the form 1 * y = y, y in [1 .. 2^numBits - 1]" << std::endl;
//		NWAOBDD OneX = MkX(1);
//		for (unsigned int j = 1; j < ((unsigned int)(1 << numBits)); j++) {
//			NWAOBDD Y = MkY(j);
//			NWAOBDD Z = MkZ(j);
//			ans = MkOr(ans, MkAnd(OneX,MkAnd(Y,Z)));
//		}

		// Add all products of the form x * 1 = x, x in [1 .. 2^numBits - 1]
		std::cout << "Add all products of the form x * 1 = x, x in [1 .. 2^numBits - 1]" << std::endl;
		NWAOBDD XeqZ = MkTrue();
		for (unsigned int b = 0; b < numBits; b++) {
			NWAOBDD Xb = MkXBit(b);
			NWAOBDD Zb = MkZBit(b);
			XeqZ = MkAnd(XeqZ, MkIff(Xb, Zb));
		}
		ans = MkOr(ans, MkAnd(MkY(1), XeqZ));

/*		// Add all products of the form x * 1 = x, x in [1 .. 2^numBits - 1]
		std::cout << "Add all products of the form x * 1 = x, x in [1 .. 2^numBits - 1]" << std::endl;
		NWAOBDD OneY = MkY(1);
		for (unsigned int i = 1; i < ((unsigned int)(1 << numBits)); i++) {
			NWAOBDD X = MkX(i);
			NWAOBDD Z = MkZ(i);
			ans = MkOr(ans, MkAnd(X,MkAnd(OneY,Z)));
		}
		ans = MkOr(ans, MkAnd(MkX(0), MkAnd(OneY, MkZ(0))));
*/
		// Add all products of all other forms that cannot overflow: i,j in [2 .. 2^numBits - 1]
		std::cout << "Add all products of all other forms that cannot overflow: i,j in [2 .. 2^numBits - 1]" << std::endl;
		for (unsigned long long i = 2ULL; i < ((unsigned long long)(1 << numBits))/2ULL; i++) {
			std::cout << "i = " << i << std::endl;
			NWAOBDD X = MkX(i);
//			for (unsigned long long j = 2ULL; j < ((unsigned long long)(1 << numBits))/2ULL; j++) {
			for (unsigned long long j = 2ULL; j <= i; j++) {
				unsigned long long k = i*j;
				if (k < ((unsigned long long)(1 << numBits))) { // no overflow
					NWAOBDD Y = MkY(j);
					NWAOBDD Z = MkZ(k);
					ans = MkOr(ans, MkAnd(X,MkAnd(Y,Z)));
//					std::cout << " No overflow on [" << i << ", " << j << ", " << k << "]" << std::endl;
				}
//				else
//					std::cout << "Overflow on (" << i << ", " << j << ", " << k << ")" << std::endl;
			}
		}
	}
	else {
		std::cerr << "Cannot use MkMultiplicationInterleavedBruteForce: maxLevel must be in the range [2..7]" << std::endl;
	}
	return ans;
}



namespace NWA_OBDD {
//
// HighOrderBitPosition
//
// Find the position of the high-order bit of z.
//
static int HighOrderBitPosition(unsigned int z) {
	int pos = -1;  
	while (z > 0) {
		z = z >> 1;
		pos++;
	}
	return pos;
}

//
// Factor
//
// Return true if z has a non-trivial factorization w.r.t. relation R,
// in which case the factors are returned in f1 and f2 (as NWAOBDDs)
// and v1 and v2 (as unsigned ints).
//
// The search for factors of z is biased away from 1 and z.
//     Search from high-order bits to low-order bits
//     Let k denote the position of the high-order bit of z.
//     For there to be a non-trivial factoring of z, the position
//        of the high-order bit of each factor is strictly less than k;
//        i.e., the k-th bit of both factors must be 0.
//        Consequently, we try the 0,0 case first
//
bool FactorZ(NWAOBDD R, unsigned int z, NWAOBDD &f1, NWAOBDD &f2, unsigned int &v1, unsigned int &v2) {
	assert(NWAOBDDMaxLevel >= 2);
	unsigned int numBits = 1 << (NWAOBDDMaxLevel - 2);   // i.e., 2**(maxLevel-2)
	
	NWAOBDD False = MkFalse();
	NWAOBDD temp;
	NWAOBDD Z = MkZ(z);
	NWAOBDD curRel = MkAnd(R, Z);  // assert that the product is z
	NWAOBDD g1 = MkTrue();         // g1 initially unconstrained
	NWAOBDD g2 = g1;               // g2 initially unconstrained
	unsigned int w1 = 0;
	unsigned int w2 = 0;

	// Loop through the bit positions; find bit values for g1 and g2
	int hobp = HighOrderBitPosition(z);
	if (4*hobp >= (1 << NWAOBDDMaxLevel)) {
		f1 = MkX(0);
		f2 = MkY(0);
		v1 = 0;
		v2 = 0;
		return false;
	}

	// Zero out the high-order bits
	for (unsigned int j = hobp+1; j < numBits; j++) {
		NWAOBDD curXTrue = MkXBit(j);
		NWAOBDD curXFalse = MkNot(curXTrue);
		NWAOBDD curYTrue = MkYBit(j);
		NWAOBDD curYFalse = MkNot(curYTrue);
		g1 = MkAnd(g1, curXFalse);
		g2 = MkAnd(g2, curYFalse);
		w1 = w1 & ~((unsigned int)(1 << j));
		w2 = w2 & ~((unsigned int)(1 << j));
	}

	// Search for the values of the rest of the bits, from high-order to low-order
	for (unsigned int j = 0; j <= hobp; j++) {
		unsigned int i = hobp - j;
		NWAOBDD curXTrue = MkXBit(i);
		NWAOBDD curXFalse = MkNot(curXTrue);
		NWAOBDD curYTrue = MkYBit(i);
		NWAOBDD curYFalse = MkNot(curYTrue);
		
		// One of the following four possibilities must be true
		// 0,0 case performed first
		temp = MkAnd(curRel, MkAnd(curXFalse, curYFalse));
		if (temp != False) {
			curRel = temp;
			g1 = MkAnd(g1, curXFalse);
			g2 = MkAnd(g2, curYFalse);
			w1 = w1 & ~((unsigned int)(1 << i));
			w2 = w2 & ~((unsigned int)(1 << i));
			continue;
		}

		// 1,0
		temp = MkAnd(curRel, MkAnd(curXTrue, curYFalse));
		if (temp != False) {
			curRel = temp;
			g1 = MkAnd(g1, curXTrue);
			g2 = MkAnd(g2, curYFalse);
			w1 = w1 | ((unsigned int)(1 << i));
			w2 = w2 & ~((unsigned int)(1 << i));
			continue;
		}
		// 0,1
		temp = MkAnd(curRel, MkAnd(curXFalse, curYTrue));
		if (temp != False) {
			curRel = temp;
			g1 = MkAnd(g1, curXFalse);
			g2 = MkAnd(g2, curYTrue);
			w1 = w1 & ~((unsigned int)(1 << i));
			w2 = w2 | ((unsigned int)(1 << i));
			continue;
		}
		// 1,1
		temp = MkAnd(curRel, MkAnd(curXTrue, curYTrue));
		if (temp != False) {
			curRel = temp;
			g1 = MkAnd(g1, curXTrue);
			g2 = MkAnd(g2, curYTrue);
			w1 = w1 | ((unsigned int)(1 << i));
			w2 = w2 | ((unsigned int)(1 << i));
			continue;
		}
		assert(false);
	}
	f1 = g1;
	f2 = g2;
	v1 = w1;
	v2 = w2;
	NWAOBDD XOne = MkX(1);
	NWAOBDD YOne = MkY(1);
	return (g1 != XOne && g2 != YOne);  // Return true if neither factor is 1
}


}


namespace NWA_OBDD {

// Create representation of detensor-constraint relation
NWAOBDD MkDetensorConstraintInterleaved()
{
  return NWAOBDD(MkDetensorConstraintInterleavedTop());
}

// Create representation of parity function
NWAOBDD MkParity()
{
  return NWAOBDD(MkParityTop());
}

// Create representation of the Walsh matrix W(2**(i-1))
// [i.e., a matrix of size 2**(2**(i-1))) x 2**(2**(i-1)))]
// with interleaved indexing of components: that is, input
// (x0,y0,x1,y1,...,xN,yN) yields W[(x0,x1,...,xN)][(y0,y1,...,yN)]
NWAOBDD MkWalshInterleaved(unsigned int i)
{
  assert(i <= NWAOBDDMaxLevel);
  return NWAOBDD(MkWalshInterleavedTop(i));
}

// Create representation of the Inverse Reed-Muller matrix IRM(2**(i-1))
// [i.e., a matrix of size 2**(2**(i-1))) x 2**(2**(i-1)))]
// with interleaved indexing of components: that is, input
// (x0,y0,x1,y1,...,xN,yN) yields IRM[(x0,x1,...,xN)][(y0,y1,...,yN)]
NWAOBDD MkInverseReedMullerInterleaved(unsigned int i)
{
  assert(i <= NWAOBDDMaxLevel);
  return NWAOBDD(MkInverseReedMullerInterleavedTop(i));
}

// Create the representation of a step function
NWAOBDD MkStepUpOneFourth()
{
  assert(NWAOBDDMaxLevel >= 1);
  return NWAOBDD(MkStepUpOneFourthTop());
}

// Create the representation of a step function
NWAOBDD MkStepDownOneFourth()
{
  assert(NWAOBDDMaxLevel >= 1);
  return NWAOBDD(MkStepDownOneFourthTop());
}

#ifdef ARBITRARY_STEP_FUNCTIONS
// Create the representation of a step function
NWAOBDD MkStepUp(unsigned int i)
{
  assert(NWAOBDDMaxLevel <= 5);
  return NWAOBDD(MkStepUpTop(i));
}

// Create the representation of a step function
NWAOBDD MkStepDown(unsigned int i)
{
  assert(NWAOBDDMaxLevel <= 5);
  return NWAOBDD(MkStepDownTop(i));
}

// Create the representation of an up-pulse function
NWAOBDD MkPulseUp(unsigned int i, unsigned int j)
{
  assert(i < j);
  return MkAnd(MkStepUp(i), MkStepDown(j));
}

// Create the representation of a down-pulse function
NWAOBDD MkPulseDown(unsigned int i, unsigned int j)
{
  assert(i < j);
  return MkOr(MkStepDown(i), MkStepUp(j));
}

#endif

// Unary operations on NWAOBDDs --------------------------------------

// Implements \f.!f
NWAOBDD MkNot(NWAOBDD f)
{
  return NWAOBDD(MkNot(f.root));
}

// Binary operations on NWAOBDDs --------------------------------------

// \f.\g.(f && g)
NWAOBDD MkAnd(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkAnd(f.root, g.root));
}

// \f.\g.!(f && g)
NWAOBDD MkNand(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkNand(f.root, g.root));
}

// \f.\g.(f || g)
NWAOBDD MkOr(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkOr(f.root, g.root));
}

// \f.\g.!(f || g)
NWAOBDD MkNor(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkNor(f.root, g.root));
}

// \f.\g.(f == g)
NWAOBDD MkIff(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkIff(f.root, g.root));
}

// \f.\g.(f != g)
NWAOBDD MkExclusiveOr(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkExclusiveOr(f.root, g.root));
}

// \f.\g.(!f || g)
NWAOBDD MkImplies(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkImplies(f.root, g.root));
}

// \f.\g.(f && !g)
NWAOBDD MkMinus(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkMinus(f.root, g.root));
}

// \f.\g.(!g || f)
NWAOBDD MkQuotient(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkQuotient(f.root, g.root));
}

// \f.\g.(g && !f)
NWAOBDD MkNotQuotient(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkNotQuotient(f.root, g.root));
}

// \f.\g.f
NWAOBDD MkFirst(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkFirst(f.root, g.root));
}

// \f.\g.!f
NWAOBDD MkNotFirst(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkNotFirst(f.root, g.root));
}

// \f.\g.g
NWAOBDD MkSecond(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkSecond(f.root, g.root));
}

// \f.\g.!g
NWAOBDD MkNotSecond(NWAOBDD f, NWAOBDD g)
{
  return NWAOBDD(MkNotSecond(f.root, g.root));
}

// Ternary operations on NWAOBDDs --------------------------------------

// \a.\b.\c.(a && b) || (!a && c)
NWAOBDD MkIfThenElse(NWAOBDD f, NWAOBDD g, NWAOBDD h)
{
  return NWAOBDD(MkIfThenElse(f.root, g.root, h.root));
}

// \a.\b.\c.(b && !a) || (c && !a) || (b && c)
NWAOBDD MkNegMajority(NWAOBDD f, NWAOBDD g, NWAOBDD h)
{
  return NWAOBDD(MkNegMajority(f.root, g.root, h.root));
}

// \f. f | (x_i = val)
NWAOBDD MkRestrict(NWAOBDD f, unsigned int i, bool val)
{
  return NWAOBDD(MkRestrict(f.root, i, val));
}

// \f. exists x_i : f
NWAOBDD MkExists(NWAOBDD f, unsigned int i)
{
  return NWAOBDD(MkExists(f.root, i));
}

NWAOBDD SchemaAdjust(NWAOBDD f, int s[4])
{
  return NWAOBDD(MkSchemaAdjust(f.root, s));
}

NWAOBDD PathSummary(NWAOBDD f)
{
	return NWAOBDD(MkPathSummary(f.root));
}


// \f. forall x_i : f
NWAOBDD MkForall(NWAOBDD f, unsigned int i)
{
  return NWAOBDD(MkForall(f.root, i));
}
}
//********************************************************************
// Operations to gather statistics
//********************************************************************

// Statistics on Connections ----------------------------------

void NWAOBDD::DumpConnections(std::ostream & out /* = std::cout */)
{
  Hashset<NWAOBDDNode> *visited = new Hashset<NWAOBDDNode>;
  root->DumpConnections(visited, out);
  delete visited;
}

Hashset<NWAOBDDNode> *NWAOBDD::visitedDuringGroupDumpConnections = NULL;
namespace NWA_OBDD {
void GroupDumpConnectionsStart()
{
  NWAOBDD::visitedDuringGroupDumpConnections = new Hashset<NWAOBDDNode>;
}

void GroupDumpConnectionsEnd()
{
  delete NWAOBDD::visitedDuringGroupDumpConnections;
  NWAOBDD::visitedDuringGroupDumpConnections = NULL;
}

void NWAOBDD::GroupDumpConnections(std::ostream & out /* = std::cout */)
{
  root->DumpConnections(NWAOBDD::visitedDuringGroupDumpConnections, out);
}
}
// Statistics on size ----------------------------------------------------

void NWAOBDD::CountNodesAndEdges(unsigned int &nodeCount, unsigned int &edgeCount)
{
  Hashset<NWAOBDDNode> *visitedNodes = new Hashset<NWAOBDDNode>;
  Hashset<ReturnMapBody<intpair>> *visitedEdges = new Hashset<ReturnMapBody<intpair>>;
  nodeCount = 0;
  edgeCount = 0;
  root->CountNodesAndEdges(visitedNodes, visitedEdges, nodeCount, edgeCount);
  delete visitedNodes;
  delete visitedEdges;
}

Hashset<NWAOBDDNode> *NWAOBDD::visitedNodesDuringGroupCountNodesAndEdges = NULL;
Hashset<ReturnMapBody<intpair>> *NWAOBDD::visitedEdgesDuringGroupCountNodesAndEdges = NULL;

namespace NWA_OBDD {
void GroupCountNodesAndEdgesStart(unsigned int &nodeCount, unsigned int &edgeCount)
{
  nodeCount = 0;
  edgeCount = 0;
  NWAOBDD::visitedNodesDuringGroupCountNodesAndEdges = new Hashset<NWAOBDDNode>;
  NWAOBDD::visitedEdgesDuringGroupCountNodesAndEdges = new Hashset<ReturnMapBody<intpair>>;
}

void GroupCountNodesAndEdgesEnd()
{
  delete NWAOBDD::visitedNodesDuringGroupCountNodesAndEdges;
  delete NWAOBDD::visitedEdgesDuringGroupCountNodesAndEdges;
  NWAOBDD::visitedNodesDuringGroupCountNodesAndEdges = NULL;
  NWAOBDD::visitedEdgesDuringGroupCountNodesAndEdges = NULL;
}
}

void NWAOBDD::GroupCountNodesAndEdges(unsigned int &nodeCount, unsigned int &edgeCount)
{
  root->CountNodesAndEdges(NWAOBDD::visitedNodesDuringGroupCountNodesAndEdges,
                           NWAOBDD::visitedEdgesDuringGroupCountNodesAndEdges,
                           nodeCount, edgeCount
                          );
}