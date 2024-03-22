#ifndef VECTOR_COMPLEX_FLOAT_BOOST_GUARD
#define VECTOR_COMPLEX_FLOAT_BOOST_GUARD


#include <iostream>
#include <string>
#include <fstream>
#include <complex>
#include <boost/multiprecision/cpp_complex.hpp>
#include "../nwaobdd.h"

namespace mp = boost::multiprecision;

namespace NWA_OBDD {
	typedef mp::cpp_complex_100 BIG_COMPLEX_FLOAT;
	typedef NWAOBDD<BIG_COMPLEX_FLOAT> NWAOBDD_COMPLEX_BIG;
	//typedef CFLOBDD_T<BIG_FLOAT> NWAOBDD_COMPLEX_BIG;

	namespace VectorComplex {
		// Initialization routine
		extern void VectorInitializer();

		extern NWAOBDD_COMPLEX_BIG MkBasisVector(unsigned int level, unsigned int index);         // Representation of basis vector with index i
		extern NWAOBDD_COMPLEX_BIG MkBasisVector(unsigned int level, std::string s); // index i represented as string
		// extern NWAOBDD_COMPLEX_BIG VectorToMatrixInterleaved(NWAOBDD_COMPLEX_BIG c); // Convert vector to matrix with variables interleaved
		//extern NWAOBDD_COMPLEX_BIG MatrixToVector(NWAOBDD_COMPLEX_BIG c); // Convert vector to matrix with variables interleaved
		// extern NWAOBDD_COMPLEX_BIG MkColumn1Matrix(unsigned int level);
		// extern NWAOBDD_COMPLEX_BIG MkVectorWithVoc12(NWAOBDD_COMPLEX_BIG c); // convert the vector into another vector with extra volcabularies
		// extern NWAOBDD_COMPLEX_BIG KroneckerProduct(NWAOBDD_COMPLEX_BIG m1, NWAOBDD_COMPLEX_BIG m2);
		// extern NWAOBDD_COMPLEX_BIG VectorShiftVocs1To2(NWAOBDD_COMPLEX_BIG m1);
		// extern NWAOBDD_COMPLEX_BIG VectorPadWithZeros(NWAOBDD_COMPLEX_BIG c, unsigned int level);
		// extern NWAOBDD_COMPLEX_BIG NoDistinctionNode(unsigned int level, BIG_COMPLEX_FLOAT val);
		//#ifdef PATH_COUNTING_ENABLED
		extern std::string Sampling(NWAOBDD_COMPLEX_BIG c, bool voctwo);
		// extern std::string SamplingV2(NWAOBDD_COMPLEX_BIG c);
		//#endif
		extern NWAOBDD_COMPLEX_BIG VectorWithAmplitude(NWAOBDD_COMPLEX_BIG c);
		// extern void VectorPrintColumnMajor(NWAOBDD_COMPLEX_BIG c, std::ostream & out);
		// extern void VectorPrintColumnMajorInterleaved(NWAOBDD_COMPLEX_BIG c, std::ostream & out);
		extern long double getNonZeroProbability(NWAOBDD_COMPLEX_BIG n);
		extern unsigned long long int GetPathCount(NWAOBDD_COMPLEX_BIG n, long double p);
		void DumpVector(NWAOBDD<BIG_COMPLEX_FLOAT> n);
	}
}

#endif

