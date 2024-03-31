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

#include <cassert>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include "../nwaobdd_node.h"
#include "../nwaobdd_top_node.h"
// #include "NWAobdd_top_node_int.h"
// #include "matrix1234_node.h"
#include "matrix_complex_top_node.h"
#include "matrix_complex.h"

namespace NWA_OBDD {

	namespace MatrixComplex {
		void Matrix1234Initializer() {
			Matrix1234InitializerTop();
		}
		void DumpMatrix(NWAOBDD_COMPLEX_BIG n) {
			DumpMatrixTop(n.root);
		}
		NWAOBDD_COMPLEX_BIG MatrixShiftToAConnection(NWAOBDD_COMPLEX_BIG c) {
			return NWAOBDD_COMPLEX_BIG(MatrixShiftToAConnectionTop(c.root));
		}
		NWAOBDD_COMPLEX_BIG MatrixShiftToBConnection(NWAOBDD_COMPLEX_BIG c) {
			return NWAOBDD_COMPLEX_BIG(MatrixShiftToBConnectionTop(c.root));
		}
		NWAOBDD_COMPLEX_BIG KroneckerProduct(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2) { 
			// XZ: this corresponds to "KroneckerProduct2Vocs in NWAOBDD code"
			NWAOBDD_COMPLEX_BIG m1_A = MatrixShiftToAConnection(m1);
			NWAOBDD_COMPLEX_BIG m2_B = MatrixShiftToBConnection(m2);
			NWAOBDD_COMPLEX_BIG c = m1_A * m2_B;
			return c;
		}
	}

	namespace MatrixComplex { // the matrix-multiplication
		NWAOBDD_COMPLEX_BIG MatrixMultiply(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2) {
			// XZ: this corresponds to MatrixMultiplyV4WIthInfo
			assert(m1.root -> level == m2.root -> level);
			// assert(m1.root -> level >= 2); 
			// XZ: why this
			return NWAOBDD_COMPLEX_BIG(MatrixMultiplyTop(m1.root, m2.root));
		}
	}
	
	namespace MatrixComplex { // the creation of matrices
		NWAOBDD_COMPLEX_BIG MkId(unsigned int level) {
			return NWAOBDD_COMPLEX_BIG(MkIdTop(level));
		}
		NWAOBDD_COMPLEX_BIG MkNegation(unsigned int level) {
			return NWAOBDD_COMPLEX_BIG(MkNegationTop(level));
		}

		NWAOBDD_COMPLEX_BIG MkWalsh(unsigned int level) {
			return NWAOBDD_COMPLEX_BIG(MkWalshTop(level));
		}
		NWAOBDD_COMPLEX_BIG MkPauliY(unsigned int level) {
			return NWAOBDD_COMPLEX_BIG(MkPauliYTop(level));
		}
		NWAOBDD_COMPLEX_BIG MkPauliZ(unsigned int level) {
			return NWAOBDD_COMPLEX_BIG(MkPauliZTop(level));
		}
		NWAOBDD_COMPLEX_BIG MkS(unsigned int level) {
			return NWAOBDD_COMPLEX_BIG(MkSTop(level));
		}
		NWAOBDD_COMPLEX_BIG MkPhaseShift(unsigned int level, double theta) {
			return NWAOBDD_COMPLEX_BIG(MkPhaseShiftTop(level, theta));
		}

		NWAOBDD_COMPLEX_BIG MkCNot(unsigned int level, unsigned int n, long int controller, long int controlled) {
			return NWAOBDD_COMPLEX_BIG(MkCNotTop(level, n, controller, controlled));
		}
		NWAOBDD_COMPLEX_BIG MkCCNot(unsigned int level, unsigned int n, long int controller1, long int controller2, long int controlled) {
			return NWAOBDD_COMPLEX_BIG(MkCCNotTop(level, n, controller1, controller2, controlled));
		}
		NWAOBDD_COMPLEX_BIG MkSwap(unsigned int level, long c1, long c2) {
			return NWAOBDD_COMPLEX_BIG(MkSwapTop(level, c1, c2));
		}
		NWAOBDD_COMPLEX_BIG MkiSwap(unsigned int level, long c1, long c2) {
			return NWAOBDD_COMPLEX_BIG(MkiSwapTop(level, c1, c2));
		}
		NWAOBDD_COMPLEX_BIG MkCSwap(unsigned int level, long int c1, long int x1, long int x2) {
			return NWAOBDD_COMPLEX_BIG(MkCSwapTop(level, c1, x2, x2));
		}
		NWAOBDD_COMPLEX_BIG MkCP(unsigned int level, long c1, long c2, double theta) {
			return NWAOBDD_COMPLEX_BIG(MkCPTop(level, c1, c2, theta));
		}

		NWAOBDD_COMPLEX_BIG MkRestrict(unsigned int level, std::string s) {
			// return NWAOBDD_COMPLEX_BIG(MkRestrictTop(level, s));
		}
		
	}
}

