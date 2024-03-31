#ifndef MATRIX_COMPLEX_NWA_TOP
#define MATRIX_COMPLEX_NWA_TOP

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

	namespace MatrixComplex {

		// Initialization routine
		extern void Matrix1234InitializerTop();

		extern void MatrixPrintRowMajorInterleavedTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n, std::ostream & out);
		extern void MatrixPrintRowMajorTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n, std::ostream & out);


		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkIdTop(unsigned int i); // Representation of identity relation
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkWalshTop(unsigned int i); // Representation of Walsh matrix
		// extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkInverseReedMullerInterleavedTop(unsigned int i); // Representation of Inverse Reed-Muller matrix
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkNegationTop(unsigned int i);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkPauliYTop(unsigned int i);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkPauliZTop(unsigned int i);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkSTop(unsigned int i);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MkPhaseShiftTop(unsigned int i, double theta);

		NWAOBDDTopNodeComplexFloatBoostRefPtr MkCNotTop(unsigned int level, unsigned int n, long int controller, long int controlled);
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkCCNotTop(unsigned int level, unsigned int n, long int controller1, long int controller2, long int controlled);
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkSwapTop(unsigned int level, long int i, long int j);
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkiSwapTop(unsigned int level, long int i, long int j);
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkCSwapTop(unsigned int level, long int c, long int i, long int j);
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkCPTop(unsigned int level, long int i, long int j, double theta);
		
		NWAOBDDTopNodeComplexFloatBoostRefPtr MkRestrictTop(unsigned int level, std::string s);

		
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixMultiplyTop(NWAOBDDTopNodeComplexFloatBoostRefPtr c1, NWAOBDDTopNodeComplexFloatBoostRefPtr c2);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixShiftToAConnectionTop(NWAOBDDTopNodeComplexFloatBoostRefPtr c);
		extern NWAOBDDTopNodeComplexFloatBoostRefPtr MatrixShiftToBConnectionTop(NWAOBDDTopNodeComplexFloatBoostRefPtr c);
		
		void DumpMatrixTop(NWAOBDDTopNodeComplexFloatBoostRefPtr n);

	}
}

#endif

