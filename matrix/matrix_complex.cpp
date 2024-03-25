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
// #include "cflobdd_top_node_int.h"
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
		NWAOBDD_COMPLEX_BIG MatrixShiftLevel0(NWAOBDD_COMPLEX_BIG c) {
			return NWAOBDD_COMPLEX_BIG(MatrixShiftLevel0Top(c.root));
		}
		NWAOBDD_COMPLEX_BIG KroneckerProduct(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2) { 
			// XZ: this corresponds to "KroneckerProduct2Vocs in NWAOBDD code"
			assert(GetLevel(m1) == GetLevel(m2));
			if(GetLevel(m1) == 0) {
				NWAOBDD_COMPLEX_BIG m2_B = MatrixShiftLevel0(m2);
				NWAOBDD_COMPLEX_BIG c = m1 * m2_B;
				return c;
			}
			else {
				NWAOBDD_COMPLEX_BIG m1_A = MatrixShiftToAConnection(m1);
				NWAOBDD_COMPLEX_BIG m2_B = MatrixShiftToBConnection(m2);
				NWAOBDD_COMPLEX_BIG c = m1_A * m2_B;
				return c;
			}
		}
		unsigned GetLevel(NWAOBDD_COMPLEX_BIG n) {
			return GetLevelTop(n.root);
		}
	}
	
	namespace MatrixComplex {
		NWAOBDD_COMPLEX_BIG MkId(unsigned int i) {
			return NWAOBDD_COMPLEX_BIG(MkIdTop(i));
		}
		NWAOBDD_COMPLEX_BIG MkNegation(unsigned int i) {
			return NWAOBDD_COMPLEX_BIG(MkNegationTop(i));
		}
		NWAOBDD_COMPLEX_BIG MkWalsh(unsigned int i) {
			return NWAOBDD_COMPLEX_BIG(MkWalshTop(i));
		}
		NWAOBDD_COMPLEX_BIG MkPauliY(unsigned int i) {
			return NWAOBDD_COMPLEX_BIG(MkPauliYTop(i));
		}
		NWAOBDD_COMPLEX_BIG MkPauliZ(unsigned int i) {
			return NWAOBDD_COMPLEX_BIG(MkPauliZTop(i));
		}
	



		
		
	}
}

