#ifndef MATRIX_COMPLEX_NWA
#define MATRIX_COMPLEX_NWA

//
//    Copyright (c) 2017, 2018 Thomas W. Reps
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
#include <complex>
#include <boost/multiprecision/cpp_complex.hpp>
#include "../nwaobdd.h"
#include "../fourier_semiring.h"

namespace mp = boost::multiprecision;

namespace NWA_OBDD {

	typedef mp::cpp_complex_100 BIG_COMPLEX_FLOAT;
	//typedef mp::number<mp::cpp_dec_float<200> > BIG_FLOAT;
	typedef NWAOBDD<BIG_COMPLEX_FLOAT> NWAOBDD_COMPLEX_BIG;
	typedef NWAOBDD<fourierSemiring> NWAOBDD_FOURIER;

	namespace MatrixComplex {
		// Initialization routine
		extern void Matrix1234Initializer();

		extern void MatrixPrintRowMajorInterleaved(NWAOBDD_COMPLEX_BIG c, std::ostream & out);
		extern void MatrixPrintRowMajor(NWAOBDD_COMPLEX_BIG c, std::ostream & out);


		extern NWAOBDD_COMPLEX_BIG MkId(unsigned int i); // Representation of identity relation
		extern NWAOBDD_COMPLEX_BIG MkWalsh(unsigned int i);              // Representation of Walsh matrix
		extern NWAOBDD_COMPLEX_BIG MkNegation(unsigned int i);
		extern NWAOBDD_COMPLEX_BIG MkPauliY(unsigned int i);
		extern NWAOBDD_COMPLEX_BIG MkPauliZ(unsigned int i);
		// extern NWAOBDD_COMPLEX_BIG MkInverseReedMullerInterleaved(unsigned int i);  // Representation of Inverse Reed-Muller matrix
		// extern NWAOBDD_COMPLEX_BIG MkExchangeInterleaved(unsigned int i); // Representation of exchange matrix

		extern NWAOBDD_COMPLEX_BIG MkS(unsigned int i);
		extern NWAOBDD_COMPLEX_BIG MkPhaseShift(unsigned int i, double theta);

		extern NWAOBDD_COMPLEX_BIG MkRestrictMatrix(unsigned int level, std::string s); 

		// Matrix-related operations (on matrices with room for two extra vocabularies) ------------
		extern NWAOBDD_COMPLEX_BIG MkWalshVoc13(unsigned int i);                    // Create representation of Walsh matrix with room for two extra vocabularies
		extern NWAOBDD_COMPLEX_BIG MkWalshVoc12(unsigned int i);                    // Create representation of Walsh matrix with room for two extra vocabularies
		extern NWAOBDD_COMPLEX_BIG MatrixShiftVocs13To24(NWAOBDD_COMPLEX_BIG c);                // Vocabulary shift in a matrix
		extern NWAOBDD_COMPLEX_BIG MatrixShiftVocs12To34(NWAOBDD_COMPLEX_BIG c);                // Vocabulary shift in a matrix
		extern NWAOBDD_COMPLEX_BIG MatrixShiftVoc43(NWAOBDD_COMPLEX_BIG c);                     // Vocabulary shift in a matrix
		extern NWAOBDD_COMPLEX_BIG MatrixShiftVoc42(NWAOBDD_COMPLEX_BIG c);                     // Vocabulary shift in a matrix
		extern NWAOBDD_COMPLEX_BIG MkDetensorConstraintInterleaved(unsigned int i); // Create representation of a matrix in which vocabularies 2 and 3 are constrained to be equal: (W,X,Y,Z) s.t. X==Y with interleaved variables
		// extern NWAOBDD_COMPLEX_BIG MatrixProjectVoc23(NWAOBDD_COMPLEX_BIG c);                   // Vocabulary projection
		extern NWAOBDD_COMPLEX_BIG MatrixDetensor(NWAOBDD_COMPLEX_BIG k);                       // Detensor of a 4-vocabulary matrix
		// extern NWAOBDD_COMPLEX_BIG MatrixMultiply(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2);          // Matrix multiplication
		extern NWAOBDD_COMPLEX_BIG MatrixMultiplyV4(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2);          // Matrix multiplication
		extern NWAOBDD_COMPLEX_BIG MatrixMultiplyV4WithInfo(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2);          // Matrix multiplication

		// Discrete Fourier Transform, and subroutines
		extern NWAOBDD_COMPLEX_BIG MkFourierMatrixInterleaved(unsigned int i);      // Create representation of the DFT matrix
		extern NWAOBDD_COMPLEX_BIG MkFourierMatrixInterleavedV4(unsigned int i);      // Create representation of the DFT matrix
		extern NWAOBDD_COMPLEX_BIG MkFourierMatrixInterleavedV4WithInfo(unsigned int i);      // Create representation of the DFT matrix
		extern NWAOBDD_COMPLEX_BIG MkNWAOBDDMatrixEqVoc14(unsigned int i);          // Create representation of a matrix in which vocabularies 1 and 4 are constrained to be equal: (W,X,Y,Z) s.t. W==Z with interleaved variables
		extern NWAOBDD_COMPLEX_BIG MkFourierDiagonalComponent(unsigned int i);
		extern NWAOBDD_COMPLEX_BIG PromoteInterleavedTo12(NWAOBDD_COMPLEX_BIG c);
		extern NWAOBDD_COMPLEX_BIG Demote12ToInterleaved(NWAOBDD_COMPLEX_BIG c);
		extern NWAOBDD_COMPLEX_BIG ConvertToComplex(NWAOBDD_FOURIER c);
		extern NWAOBDD_COMPLEX_BIG MkCNOTInterleaved(unsigned int i);
		extern NWAOBDD_COMPLEX_BIG MkCPGate(unsigned int i, long int c1, long int c2, double theta);
		extern NWAOBDD_COMPLEX_BIG MkSwapGate(unsigned int i, long int c1, long int c2);
		extern NWAOBDD_COMPLEX_BIG MkiSwapGate(unsigned int i, long int c1, long int c2);
		extern NWAOBDD_COMPLEX_BIG MkCSwapGate(unsigned int i, long int c1, long int x1, long int x2);
		extern NWAOBDD_COMPLEX_BIG MkCNOT(unsigned int level, unsigned int n, long int controller, long int controlled); // Representation of CNOT matrix with index1 as controller and index2 as controlled bits
		extern NWAOBDD_COMPLEX_BIG MkCCNOT(unsigned int level, unsigned int n, long int controller1, long int controller2, long int controlled);
		extern NWAOBDD_COMPLEX_BIG MkMCX(unsigned int level, unsigned int n, std::vector<long int>& controllers, long int controlled);
		extern NWAOBDD_COMPLEX_BIG MkCCP(unsigned int level, unsigned int n, long int controller1, long int controller2, long int controlled, double theta);

		extern NWAOBDD_COMPLEX_BIG MatrixShiftToAConnection(NWAOBDD_COMPLEX_BIG c);
		extern NWAOBDD_COMPLEX_BIG MatrixShiftToBConnection(NWAOBDD_COMPLEX_BIG c);
		NWAOBDD_COMPLEX_BIG MatrixShiftLevel0(NWAOBDD_COMPLEX_BIG c);
		extern NWAOBDD_COMPLEX_BIG KroneckerProduct(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2);
		
		void DumpMatrix(NWAOBDD_COMPLEX_BIG n);
		unsigned GetLevel(NWAOBDD_COMPLEX_BIG n);
	}
}

#endif

