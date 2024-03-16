#include"nwaobdd.h"

namespace NWA_OBDD {

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