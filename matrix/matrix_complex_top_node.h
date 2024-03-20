#ifndef MATRIX1234_COMPLEX_FLOAT_BOOST_TOP_NODE_GUARD
#define MATRIX1234_COMPLEX_FLOAT_BOOST_TOP_NODE_GUARD

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
#include "matrix_complex.h"
#include "../return_map_T.h"
namespace NWA_OBDD {
	typedef ReturnMapBody<BIG_COMPLEX_FLOAT> ComplexFloatBoostReturnMapBody;
	typedef ReturnMapHandle<BIG_COMPLEX_FLOAT> ComplexFloatBoostReturnMapHandle;
}
#include "../connection.h"
namespace NWA_OBDD {
	typedef ConnectionT<ComplexFloatBoostReturnMapHandle> ComplexFloatBoostConnection;
}

namespace NWA_OBDD {

	typedef NWAOBDDTopNode<BIG_COMPLEX_FLOAT> NWAOBDDTopNodeComplexFloatBoost;
	typedef NWAOBDDTopNode<BIG_COMPLEX_FLOAT>::NWAOBDDTopNodeTRefPtr NWAOBDDTopNodeComplexFloatBoostRefPtr;
	typedef NWAOBDDTopNode<fourierSemiring>::NWAOBDDTopNodeTRefPtr NWAOBDDTopNodeFourierRefPtr;

	namespace Matrix1234ComplexFloatBoost {

		// Initialization routine
		extern void Matrix1234InitializerTop();

		extern void MatrixPrintRowMajorInterleavedTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n, std::ostream & out);
		extern void MatrixPrintRowMajorTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n, std::ostream & out);


		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkIdRelationInterleavedTop(unsigned int i); // Representation of identity relation
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkWalshInterleavedTop(unsigned int i); // Representation of Walsh matrix
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkInverseReedMullerInterleavedTop(unsigned int i); // Representation of Inverse Reed-Muller matrix
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkNegationMatrixInterleavedTop(unsigned int i);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkPauliYMatrixInterleavedTop(unsigned int i);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkPauliZMatrixInterleavedTop(unsigned int i);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkSGateInterleavedTop(unsigned int i);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkPhaseShiftGateInterleavedTop(unsigned int i, double theta);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkExchangeInterleavedTop(unsigned int i); // Representation of exchange matrix

		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkRestrictTop(unsigned int level, std::string s);

		// Matrix-related operations (on matrices with room for two extra vocabularies) ------------
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkWalshVoc13Top(unsigned int i); // Representation of Walsh matrix with room for two additional vocabularies
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkWalshVoc12Top(unsigned int i); // Representation of Walsh matrix with room for two additional vocabularies
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixShiftVocs13To24Top(NWAOBDDTopNodeComplexFloatBoostRefPtr n); // Vocabulary shift
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixShiftVocs12To34Top(NWAOBDDTopNodeComplexFloatBoostRefPtr n); // Vocabulary shift
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixShiftVoc43Top(NWAOBDDTopNodeComplexFloatBoostRefPtr n); // Vocabulary shift
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixShiftVoc42Top(NWAOBDDTopNodeComplexFloatBoostRefPtr n); // Vocabulary shift
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkDetensorConstraintInterleavedTop(unsigned int i);
		// extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixProjectVoc23Top(NWAOBDDTopNodeComplexFloatBoostRefPtr n); // Vocabulary projection
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixMultiplyV4TopNode(NWAOBDDTopNodeComplexFloatBoostRefPtr c1, NWAOBDDTopNodeComplexFloatBoostRefPtr c2);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixMultiplyV4WithInfoTopNode(NWAOBDDTopNodeComplexFloatBoostRefPtr c1, NWAOBDDTopNodeComplexFloatBoostRefPtr c2);

		// Subroutines for Discrete Fourier Transform
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkNWAOBDDMatrixEqVoc14Top(unsigned int i);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkFourierDiagonalComponentTop(unsigned int i);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr PromoteInterleavedTo12Top(NWAOBDDTopNodeComplexFloatBoostRefPtr c);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr Demote12ToInterleavedTop(NWAOBDDTopNodeComplexFloatBoostRefPtr c);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr ConvertToComplexTop(NWAOBDDTopNodeFourierRefPtr c);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkCNOTInterleavedTop(unsigned int i);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkCPGateTop(unsigned int level, long int i, long int j, double theta);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkSwapGateTop(unsigned int level, long int i, long int j);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkiSwapGateTop(unsigned int level, long int i, long int j);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkCSwapGateTop(unsigned int level, long int c, long int i, long int j);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkCNOTTopNode(unsigned int level, unsigned int n, long int controller, long int controlled);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkCCNOTTopNode(unsigned int level, unsigned int n, long int controller1, long int controller2, long int controlled);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkMCXTopNode(unsigned int level, unsigned int n, std::vector<long int>& controllers, long int controlled);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkCCPTopNode(unsigned int level, unsigned int n, long int controller1, long int controller2, long int controlled, double theta);

		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixShiftToAConnectionTop(NWAOBDDTopNodeComplexFloatBoostRefPtr c);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixShiftToBConnectionTop(NWAOBDDTopNodeComplexFloatBoostRefPtr c);

	}
}

#endif

