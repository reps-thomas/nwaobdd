#include "nwaobdd_int.h"
#include <iostream>
#include <fstream>

namespace NWA_OBDD{
// NWAOBDD-creation operations --------------------------------------

// Create representation of \x.true
NWAOBDD<int> MkTrue()
{
  return NWAOBDD<int>(MkTrueTop());
}

// Create representation of \x.false
NWAOBDD<int> MkFalse()
{
  return NWAOBDD<int>(MkFalseTop());
}

// Create representation of \x.x_i
NWAOBDD<int> MkProjection(unsigned int i)
{
  assert(i < ((unsigned int)((((unsigned int)1) << (NWAOBDDMaxLevel + 2)) - (unsigned int)4)));
  return NWAOBDD<int>(MkDistinction(i));
}

NWAOBDD<int> MkCompose(NWAOBDD<int> f, int i, NWAOBDD<int> g)
{
#ifdef NDEBUG
  return  MkComposeTop(f.root, i, g.root);
#else
  NWAOBDD<int> comp = MkComposeTop(f.root, i, g.root);
  assert(comp.root->count == 1);
  return comp;
#endif
}

// Create representation of identity relation
NWAOBDD<int> MkIdRelationInterleaved()
{
  return NWAOBDD<int>(MkIdRelationInterleavedTop());
}

NWAOBDD<int> MkIdRelationNested()
{
	return NWAOBDD<int>(MkIdRelationNestedTop());
}

// MkXBit, MkYBit, MkZBit -----------------------------------------------
//
// MISSING: introduce a level of indirection so that the bits can be
// in a different order than interleaved.
static NWAOBDD<int> MkXBit(unsigned int j) {
	return MkProjection(j * 4);
}

static NWAOBDD<int> MkYBit(unsigned int j) {
	return MkProjection(j * 4 + 1);
}

#define EVEN(x) (((x)/2) * 2 == (x))

static NWAOBDD<int> MkZBit(unsigned int j) {
//	return MkProjection((j/2) * 4 + 2 + (EVEN(j) ? 0 : 1));
	return MkProjection(j * 4 + 2);
}

// Create a representation of the addition relation for natural numbers
// { (xi yi zi _)* | vec{x} * vec{y} = vec{z} }.
// We create entries only for sums where the highest bit of vec{x} and vec{y}
// are 0.
NWAOBDD<int> MkAdditionNested()
{
	//ETTODO
	NWAOBDD<int> quasiAns = NWAOBDD<int>(MkAdditionNestedTop());

	// Restrict the highest bits of x and y to 0
	NWAOBDD xRestriction = MkNot(MkXBit(1 << ((NWAOBDDMaxLevel - 2) - 1)));
	NWAOBDD yRestriction = MkNot(MkYBit(1 << ((NWAOBDDMaxLevel - 2) - 1)));
    return MkAnd(quasiAns, MkAnd(xRestriction, yRestriction));
}

// Create a representation of the addition relation for natural numbers
// { (xi yi zi _)* | vec{x} * vec{y} = vec{z} } by brute force.
// We create entries only for sums where the highest bit of vec{x} and vec{y}
// are 0.
NWAOBDD<int> MkAdditionInterleavedBruteForce() {
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
NWAOBDD<int> MkX(unsigned int i) {
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
NWAOBDD<int> MkY(unsigned int i) {
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
NWAOBDD<int> MkZ(unsigned int i) {
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

NWAOBDD<int> MkMultiplicationInterleavedBruteForce() {
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

} // namespace NWA_OBDD

static int HighOrderBitPosition(unsigned int z) {
	int pos = -1;  
	while (z > 0) {
		z = z >> 1;
		pos++;
	}
	return pos;
}

namespace NWA_OBDD {
//
// HighOrderBitPosition
//
// Find the position of the high-order bit of z.
//


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



}
namespace NWA_OBDD {
    bool FactorZ(NWAOBDD<int> R, unsigned int z, NWAOBDD<int> &f1, NWAOBDD<int> &f2, unsigned int &v1, unsigned int &v2) {
	assert(NWAOBDDMaxLevel >= 2);
	unsigned int numBits = 1 << (NWAOBDDMaxLevel - 2);   // i.e., 2**(maxLevel-2)
	
	NWAOBDD<int> False = MkFalse();
	NWAOBDD<int> temp;
	NWAOBDD<int> Z = MkZ(z);
	NWAOBDD<int> curRel = MkAnd(R, Z);  // assert that the product is z
	NWAOBDD<int> g1 = MkTrue();         // g1 initially unconstrained
	NWAOBDD<int> g2 = g1;               // g2 initially unconstrained
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
		NWAOBDD<int> curXTrue = MkXBit(j);
		NWAOBDD<int> curXFalse = MkNot(curXTrue);
		NWAOBDD<int> curYTrue = MkYBit(j);
		NWAOBDD<int> curYFalse = MkNot(curYTrue);
		g1 = MkAnd(g1, curXFalse);
		g2 = MkAnd(g2, curYFalse);
		w1 = w1 & ~((unsigned int)(1 << j));
		w2 = w2 & ~((unsigned int)(1 << j));
	}

	// Search for the values of the rest of the bits, from high-order to low-order
	for (unsigned int j = 0; j <= hobp; j++) {
		unsigned int i = hobp - j;
		NWAOBDD<int> curXTrue = MkXBit(i);
		NWAOBDD<int> curXFalse = MkNot(curXTrue);
		NWAOBDD<int> curYTrue = MkYBit(i);
		NWAOBDD<int> curYFalse = MkNot(curYTrue);
		
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
	NWAOBDD<int> XOne = MkX(1);
	NWAOBDD<int> YOne = MkY(1);
	return (g1 != XOne && g2 != YOne);  // Return true if neither factor is 1
}

} // namespace NWA_OBDD


namespace NWA_OBDD {

// Create representation of detensor-constraint relation
NWAOBDD<int> MkDetensorConstraintInterleaved()
{
  return NWAOBDD<int>(MkDetensorConstraintInterleavedTop());
}

// Create representation of parity function
NWAOBDD<int> MkParity()
{
  return NWAOBDD<int>(MkParityTop());
}

// Create representation of the Walsh matrix W(2**(i-1))
// [i.e., a matrix of size 2**(2**(i-1))) x 2**(2**(i-1)))]
// with interleaved indexing of components: that is, input
// (x0,y0,x1,y1,...,xN,yN) yields W[(x0,x1,...,xN)][(y0,y1,...,yN)]
NWAOBDD<int> MkWalshInterleaved(unsigned int i)
{
  assert(i <= NWAOBDDMaxLevel);
  return NWAOBDD<int>(MkWalshInterleavedTop(i));
}

// Create representation of the Inverse Reed-Muller matrix IRM(2**(i-1))
// [i.e., a matrix of size 2**(2**(i-1))) x 2**(2**(i-1)))]
// with interleaved indexing of components: that is, input
// (x0,y0,x1,y1,...,xN,yN) yields IRM[(x0,x1,...,xN)][(y0,y1,...,yN)]
NWAOBDD<int> MkInverseReedMullerInterleaved(unsigned int i)
{
  assert(i <= NWAOBDDMaxLevel);
  return NWAOBDD<int>(MkInverseReedMullerInterleavedTop(i));
}

// Create the representation of a step function
NWAOBDD<int> MkStepUpOneFourth()
{
  assert(NWAOBDDMaxLevel >= 1);
  return NWAOBDD<int>(MkStepUpOneFourthTop());
}

// Create the representation of a step function
NWAOBDD<int> MkStepDownOneFourth()
{
  assert(NWAOBDDMaxLevel >= 1);
  return NWAOBDD<int>(MkStepDownOneFourthTop());
}

#ifdef ARBITRARY_STEP_FUNCTIONS
// Create the representation of a step function
NWAOBDD<int> MkStepUp(unsigned int i)
{
  assert(NWAOBDDMaxLevel <= 5);
  return NWAOBDD<int>(MkStepUpTop(i));
}

// Create the representation of a step function
NWAOBDD<int> MkStepDown(unsigned int i)
{
  assert(NWAOBDDMaxLevel <= 5);
  return NWAOBDD<int>(MkStepDownTop(i));
}

// Create the representation of an up-pulse function
NWAOBDD<int> MkPulseUp(unsigned int i, unsigned int j)
{
  assert(i < j);
  return MkAnd(MkStepUp(i), MkStepDown(j));
}

// Create the representation of a down-pulse function
NWAOBDD<int> MkPulseDown(unsigned int i, unsigned int j)
{
  assert(i < j);
  return MkOr(MkStepDown(i), MkStepUp(j));
}

#endif

// Unary operations on NWAOBDDs --------------------------------------

// Implements \f.!f
NWAOBDD<int> MkNot(NWAOBDD<int> f)
{
  return NWAOBDD<int>(MkNot(f.root));
}

// Binary operations on NWAOBDDs --------------------------------------

// \f.\g.(f && g)
NWAOBDD<int> MkAnd(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkAnd(f.root, g.root));
}

// \f.\g.!(f && g)
NWAOBDD<int> MkNand(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkNand(f.root, g.root));
}

// \f.\g.(f || g)
NWAOBDD<int> MkOr(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkOr(f.root, g.root));
}

// \f.\g.!(f || g)
NWAOBDD<int> MkNor(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkNor(f.root, g.root));
}

// \f.\g.(f == g)
NWAOBDD<int> MkIff(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkIff(f.root, g.root));
}

// \f.\g.(f != g)
NWAOBDD<int> MkExclusiveOr(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkExclusiveOr(f.root, g.root));
}

// \f.\g.(!f || g)
NWAOBDD<int> MkImplies(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkImplies(f.root, g.root));
}

// \f.\g.(f && !g)
NWAOBDD<int> MkMinus(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkMinus(f.root, g.root));
}

// \f.\g.(!g || f)
NWAOBDD<int> MkQuotient(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkQuotient(f.root, g.root));
}

// \f.\g.(g && !f)
NWAOBDD<int> MkNotQuotient(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkNotQuotient(f.root, g.root));
}

// \f.\g.f
NWAOBDD<int> MkFirst(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkFirst(f.root, g.root));
}

// \f.\g.!f
NWAOBDD<int> MkNotFirst(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkNotFirst(f.root, g.root));
}

// \f.\g.g
NWAOBDD<int> MkSecond(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkSecond(f.root, g.root));
}

// \f.\g.!g
NWAOBDD<int> MkNotSecond(NWAOBDD<int> f, NWAOBDD<int> g)
{
  return NWAOBDD<int>(MkNotSecond(f.root, g.root));
}

// Ternary operations on NWAOBDDs --------------------------------------

// \a.\b.\c.(a && b) || (!a && c)
NWAOBDD<int> MkIfThenElse(NWAOBDD<int> f, NWAOBDD<int> g, NWAOBDD<int> h)
{
  return NWAOBDD<int>(MkOr(MkAnd(f, g), MkAnd(MkNot(f), h)));
}

// \a.\b.\c.(b && !a) || (c && !a) || (b && c)
NWAOBDD<int> MkNegMajority(NWAOBDD<int> f, NWAOBDD<int> g, NWAOBDD<int> h)
{
	auto r1 = NWAOBDD<int>(MkAnd(g, MkNot(f)));
	auto r2 = NWAOBDD<int>(MkAnd(h, MkNot(f)));
	auto r3 = NWAOBDD<int>(MkAnd(g, h));
  	return MkOr(r1, MkOr(r2, r3));
}

// \f. f | (x_i = val)
NWAOBDD<int> MkRestrict(NWAOBDD<int> f, unsigned int i, bool val)
{
  return NWAOBDD<int>(MkRestrict(f.root, i, val));
}

// \f. exists x_i : f
NWAOBDD<int> MkExists(NWAOBDD<int> f, unsigned int i)
{
  return NWAOBDD<int>(MkExists(f.root, i));
}

NWAOBDD<int> SchemaAdjust(NWAOBDD<int> f, int s[4])
{
  return NWAOBDD<int>(MkSchemaAdjust(f.root, s));
}

NWAOBDD<int> PathSummary(NWAOBDD<int> f)
{
	return NWAOBDD<int>(MkPathSummary(f.root));
}


// \f. forall x_i : f
NWAOBDD<int> MkForall(NWAOBDD<int> f, unsigned int i)
{
  return NWAOBDD<int>(MkForall(f.root, i));
}
}

