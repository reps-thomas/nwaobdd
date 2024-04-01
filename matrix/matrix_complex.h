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

namespace mp = boost::multiprecision;

namespace NWA_OBDD {

	typedef mp::cpp_complex_100 BIG_COMPLEX_FLOAT;
	//typedef mp::number<mp::cpp_dec_float<200> > BIG_FLOAT;
	typedef NWAOBDD<BIG_COMPLEX_FLOAT> NWAOBDD_COMPLEX_BIG;

	namespace MatrixComplex {
		// Initialization routine
		extern void Matrix1234Initializer();

		NWAOBDD_COMPLEX_BIG MkId(unsigned int i);
		NWAOBDD_COMPLEX_BIG MkNegation(unsigned int i);

		NWAOBDD_COMPLEX_BIG MkWalsh(unsigned int i);
		NWAOBDD_COMPLEX_BIG MkPauliY(unsigned int i);
		NWAOBDD_COMPLEX_BIG MkPauliZ(unsigned int i);
		NWAOBDD_COMPLEX_BIG MkS(unsigned int i);
		NWAOBDD_COMPLEX_BIG MkPhaseShift(unsigned int i, double theta);

		NWAOBDD_COMPLEX_BIG MkCNot(unsigned int level, unsigned int n, long int controller, long int controlled);
		NWAOBDD_COMPLEX_BIG MkCCNot(unsigned int level, unsigned int n, long int controller1, long int controller2, long int controlled);
		NWAOBDD_COMPLEX_BIG MkSwap(unsigned level, long int c1, long int c2);
		NWAOBDD_COMPLEX_BIG MkiSwap(unsigned level, long int c1, long int c2);
		NWAOBDD_COMPLEX_BIG MkCSwap(unsigned level, long int c1, long int x1, long int x2);
		NWAOBDD_COMPLEX_BIG MkCP(unsigned int level, long int c1, long int c2, double theta);

		NWAOBDD_COMPLEX_BIG MkRestrict(unsigned int level, std::string s); 

		NWAOBDD_COMPLEX_BIG MatrixMultiply(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2);
		
		NWAOBDD_COMPLEX_BIG MatrixShiftToAConnection(NWAOBDD_COMPLEX_BIG c);
		NWAOBDD_COMPLEX_BIG MatrixShiftToBConnection(NWAOBDD_COMPLEX_BIG c);
		NWAOBDD_COMPLEX_BIG KroneckerProduct(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2);

		void DumpMatrix(NWAOBDD_COMPLEX_BIG n);
	}
}

#endif

