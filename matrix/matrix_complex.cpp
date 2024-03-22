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

		void Matrix1234Initializer()
		{
			Matrix1234InitializerTop();
		}

		// Create representation of identity relation
		NWAOBDD_COMPLEX_BIG MkIdRelationInterleaved(unsigned int i)
		{
			return NWAOBDD_COMPLEX_BIG(MkIdRelationInterleavedTop(i));
		}

		NWAOBDD_COMPLEX_BIG MkFourierMatrixInterleavedV4(unsigned int i)
		{
			// if (i >= 5){
			// 	DisposeOfPairProductCache();
			// 	InitPairProductCache();
			// }
			// assert(1 <= i && i < (NWAOBDD_COMPLEX_BIG::maxLevel - 1));  // Need two extra levels: +1 for Kronecker Product; +1 for matrix multiplication

			// if (i == 1) {
			// 	return MkWalshInterleaved(1);                                                          // Level 1, interleaved
			// }
			// else {  // Need to promote all matrices to vocs 1,2 to allow final matrix multiplication


			// 	NWAOBDD_COMPLEX_BIG FourierIMinusOne = PromoteInterleavedTo12(MkFourierMatrixInterleavedV4(i - 1));  // Level i, Vocs 1,2

			// 	std::cout << "---FourierIMinusOne------------------------------------------------------------" << std::endl;
			// 	//std::cout << FourierIMinusOne << std::endl;
			// 	//FourierIMinusOne.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	std::cout << "---Level i: " << i << " ------------------------------------------------------------" << std::endl;


			// 	NWAOBDD_COMPLEX_BIG Id = PromoteInterleavedTo12(MkIdRelationInterleaved(i - 1));                   // Level i-1, Vocs 1

			// 	std::cout << "---Id------------------------------------------------------------" << std::endl;
			// 	//std::cout << Id << std::endl;
			// 	//Id.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;


			// 	NWAOBDD_COMPLEX_BIG term1 = KroneckerProduct(FourierIMinusOne, Id);        // Level i+1, Vocs 1,2

			// 	std::cout << "---term1------------------------------------------------------------" << std::endl;
			// 	//std::cout << term1 << std::endl;
			// 	//term1.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	NWAOBDD_COMPLEX_BIG term2 = MkFourierDiagonalComponent(i);                 // Level i+1, Vocs 1,2

			// 	std::cout << "---term2------------------------------------------------------------" << std::endl;
			// 	//std::cout << term2 << std::endl;
			// 	//term2.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	NWAOBDD_COMPLEX_BIG term3 = KroneckerProduct(Id, FourierIMinusOne);        // Level i+1, Vocs 1,2

			// 	std::cout << "---term3------------------------------------------------------------" << std::endl;
			// 	//std::cout << term3 << std::endl;
			// 	//term3.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	NWAOBDD_COMPLEX_BIG term4 = MkNWAOBDDMatrixEqVoc14(i) * MkDetensorConstraintInterleaved(i);                     // Level i+1, Vocs 1,2

			// 	std::cout << "---term4------------------------------------------------------------" << std::endl;
			// 	//std::cout << term4 << std::endl;
			// 	//term4.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;
			// 	//printMemory();

			// 	NWAOBDD_COMPLEX_BIG temp12 = MatrixMultiplyV4(term1, term2);                                          // Level i+1, Vocs 1,2

			// 	std::cout << "---temp12------------------------------------------------------------" << std::endl;
			// 	//std::cout << temp12 << std::endl;
			// 	//temp12.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	NWAOBDD_COMPLEX_BIG temp34 = MatrixMultiplyV4(term3, term4);                                          // Level i+1, Vocs 1,2
				
			// 	std::cout << "---temp34------------------------------------------------------------" << std::endl;
			// 	//std::cout << temp34 << std::endl;
			// 	//temp34.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;


			// 	NWAOBDD_COMPLEX_BIG temp1234 = MatrixMultiplyV4(temp12, temp34);                                          // Level i+1, Vocs 1,2

			// 	std::cout << "---temp1234------------------------------------------------------------" << std::endl;
			// 	//std::cout << temp1234 << std::endl;
			// 	//temp1234.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	return temp1234;
			// }
		}

		NWAOBDD_COMPLEX_BIG MkFourierMatrixInterleavedV4WithInfo(unsigned int i)
		{
			// if (i >= 5){
			// 	DisposeOfPairProductCache();
			// 	InitPairProductCache();
			// }
			// assert(1 <= i && i < (NWAOBDD_COMPLEX_BIG::maxLevel - 1));  // Need two extra levels: +1 for Kronecker Product; +1 for matrix multiplication

			// if (i == 1) {
			// 	return MkWalshInterleaved(1);                                                          // Level 1, interleaved
			// }
			// else {  // Need to promote all matrices to vocs 1,2 to allow final matrix multiplication


			// 	NWAOBDD_COMPLEX_BIG FourierIMinusOne = PromoteInterleavedTo12(MkFourierMatrixInterleavedV4(i - 1));  // Level i, Vocs 1,2

			// 	std::cout << "---FourierIMinusOne------------------------------------------------------------" << std::endl;
			// 	//std::cout << FourierIMinusOne << std::endl;
			// 	//FourierIMinusOne.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	std::cout << "---Level i: " << i << " ------------------------------------------------------------" << std::endl;


			// 	NWAOBDD_COMPLEX_BIG Id = PromoteInterleavedTo12(MkIdRelationInterleaved(i - 1));                   // Level i-1, Vocs 1

			// 	std::cout << "---Id------------------------------------------------------------" << std::endl;
			// 	//std::cout << Id << std::endl;
			// 	//Id.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;


			// 	NWAOBDD_COMPLEX_BIG term1 = KroneckerProduct(FourierIMinusOne, Id);        // Level i+1, Vocs 1,2

			// 	std::cout << "---term1------------------------------------------------------------" << std::endl;
			// 	//std::cout << term1 << std::endl;
			// 	//term1.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	NWAOBDD_COMPLEX_BIG term2 = MkFourierDiagonalComponent(i);                 // Level i+1, Vocs 1,2

			// 	std::cout << "---term2------------------------------------------------------------" << std::endl;
			// 	//std::cout << term2 << std::endl;
			// 	//term2.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	NWAOBDD_COMPLEX_BIG term3 = KroneckerProduct(Id, FourierIMinusOne);        // Level i+1, Vocs 1,2

			// 	std::cout << "---term3------------------------------------------------------------" << std::endl;
			// 	//std::cout << term3 << std::endl;
			// 	//term3.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	NWAOBDD_COMPLEX_BIG term4 = MkNWAOBDDMatrixEqVoc14(i) * MkDetensorConstraintInterleaved(i);                     // Level i+1, Vocs 1,2

			// 	std::cout << "---term4------------------------------------------------------------" << std::endl;
			// 	//std::cout << term4 << std::endl;
			// 	//term4.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;
			// 	//printMemory();

			// 	NWAOBDD_COMPLEX_BIG temp12 = MatrixMultiplyV4WithInfo(term1, term2);                                          // Level i+1, Vocs 1,2

			// 	std::cout << "---temp12------------------------------------------------------------" << std::endl;
			// 	//std::cout << temp12 << std::endl;
			// 	//temp12.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	NWAOBDD_COMPLEX_BIG temp34 = MatrixMultiplyV4WithInfo(term3, term4);                                          // Level i+1, Vocs 1,2

			// 	std::cout << "---temp34------------------------------------------------------------" << std::endl;
			// 	//std::cout << temp34 << std::endl;
			// 	//temp34.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;



			// 	NWAOBDD_COMPLEX_BIG temp1234 = MatrixMultiplyV4WithInfo(temp12, temp34);                                          // Level i+1, Vocs 1,2

			// 	std::cout << "---temp1234------------------------------------------------------------" << std::endl;
			// 	//std::cout << temp1234 << std::endl;
			// 	//temp1234.PrintYield(&std::cout);
			// 	//std::cout << "----------------------------------------------------------------" << std::endl;

			// 	return temp1234;
			// }
		}

		void DumpMatrix(NWAOBDD_COMPLEX_BIG n) {
			DumpMatrixTop(n.root);
		}

		NWAOBDD_COMPLEX_BIG MatrixShiftToAConnection(NWAOBDD_COMPLEX_BIG c)
		{
			return NWAOBDD_COMPLEX_BIG(MatrixShiftToAConnectionTop(c.root));
		}

		NWAOBDD_COMPLEX_BIG MatrixShiftToBConnection(NWAOBDD_COMPLEX_BIG c)
		{
			return NWAOBDD_COMPLEX_BIG(MatrixShiftToBConnectionTop(c.root));
		}

		NWAOBDD_COMPLEX_BIG KroneckerProduct(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2) { 
			// XZ: this corresponds to "KroneckerProduct2Vocs in NWAOBDD code"
			assert(m1.root->level == m2.root->level);
			NWAOBDD_COMPLEX_BIG m1_A = MatrixShiftToAConnection(m1);
			NWAOBDD_COMPLEX_BIG m2_B = MatrixShiftToBConnection(m2);
			NWAOBDD_COMPLEX_BIG c = m1_A * m2_B;
			return c;
		}

	}
}

